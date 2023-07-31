/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

/**
 ****************************************************************************************
 * @addtogroup AMSC
 * @{
 ****************************************************************************************
 */
#include "rwip_config.h"

#if BLE_GMA_VOICE

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "gatt.h"
#include "gatt_msg.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "co_utils.h"
#include "prf_dbg.h"
#include "ai.h"

/*
 * GMA VOICE CMD PROFILE ATTRIBUTES
 ****************************************************************************************
 */
#define GMA_GATT_MAX_ATTR_LEN   255

#define GMA_GATT_UUID_SERVICE             0xFEB3
    
#define GMA_GATT_UUID_READ                0xFED4
#define GMA_GATT_UUID_WRITE               0xFED5
#define GMA_GATT_UUID_INDICATE            0xFED6
#define GMA_GATT_UUID_WR_NO_RSP           0xFED7
#define GMA_GATT_UUID_NTF                 0xFED8

#define GATT_DECL_PRIMARY_SERVICE_UUID       0x2800
#define GATT_DECL_CHARACTERISTIC_UUID        0x2803
#define GATT_DESC_CLIENT_CHAR_CFG_UUID       0x2902

///Attributes State Machine
enum {
    GMA_IDX_SVC,    //0

    GMA_IDX_READ_CHAR,   //1
    GMA_IDX_READ_VAL,    //2

    GMA_IDX_WRITE_CHAR,   //3
    GMA_IDX_WRITE_VAL,    //4

    GMA_IDX_INDICATE_CHAR,    //5
    GMA_IDX_INDICATE_VAL,   //6
    GMA_IDX_INDICATE_CFG,    //7

    GMA_IDX_WR_NO_RSP_CHAR,   //8
    GMA_IDX_WR_NO_RSP_VAL,    //9

    GMA_IDX_NTF_CHAR,      //10
    GMA_IDX_NTF_VAL,      //11
    GMA_IDX_NTF_CFG,    //12

    GMA_IDX_NB,

};

/// Full GMA SERVER Database Description - Used to add attributes into the database
const struct gatt_att16_desc gma_att_db[GMA_IDX_NB] = {
    // Service Declaration
    [GMA_IDX_SVC]           =    {GATT_DECL_PRIMARY_SERVICE_UUID, PROP(RD), 0},

    [GMA_IDX_READ_CHAR]     = {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [GMA_IDX_READ_VAL]      = {GMA_GATT_UUID_READ, PROP(RD) | ATT_UUID(16),GMA_GATT_MAX_ATTR_LEN},
  
    [GMA_IDX_WRITE_CHAR]    = {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [GMA_IDX_WRITE_VAL]     = {GMA_GATT_UUID_WRITE,   PROP(RD) | PROP(WR) | ATT_UUID(16), GMA_GATT_MAX_ATTR_LEN},
  
    [GMA_IDX_INDICATE_CHAR] = {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [GMA_IDX_INDICATE_VAL]  = {GMA_GATT_UUID_INDICATE,     PROP(I) | PROP(RD) | ATT_UUID(16),  GMA_GATT_MAX_ATTR_LEN},
    [GMA_IDX_INDICATE_CFG]  = {GATT_DESC_CLIENT_CHAR_CFG_UUID,   PROP(RD) | PROP(WR), 0},

    [GMA_IDX_WR_NO_RSP_CHAR]= {GATT_DECL_CHARACTERISTIC_UUID,    PROP(RD), 0},
    [GMA_IDX_WR_NO_RSP_VAL] = {GMA_GATT_UUID_WR_NO_RSP,    PROP(RD) | PROP(WC) | ATT_UUID(16), GMA_GATT_MAX_ATTR_LEN},

    [GMA_IDX_NTF_CHAR]      = {GATT_DECL_CHARACTERISTIC_UUID,    PROP(RD), 0},
    [GMA_IDX_NTF_VAL]       = {GMA_GATT_UUID_NTF,          PROP(N) | PROP(RD) | ATT_UUID(16),GMA_GATT_MAX_ATTR_LEN},
    [GMA_IDX_NTF_CFG]       = {GATT_DESC_CLIENT_CHAR_CFG_UUID,   PROP(RD) | PROP(WR), 0},
};

__STATIC void gma_event_sent_cb(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    // notification or indication has been sent out
    TRACE(0, "%s conidx 0x%x", __func__, conidx);

    ai_event_ind_t *ind = KE_MSG_ALLOC(PRF_AI_TX_DONE_IND,
                                 TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
    ind->conidx = conidx;
    ind->ai_type = SVC_AI_GMA;
    ind->data_len = 0;
    ke_msg_send(ind);

}

__STATIC void gma_att_read_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl, uint16_t offset,
                             uint16_t max_length)
{
    co_buf_t* p_data = NULL;
    uint16_t dataLen = 0;
    uint16_t status = GAP_ERR_NO_ERROR;

    // Get the address of the environment
    PRF_ENV_T(ai) *gma_env = PRF_ENV_GET(GMA, ai);

    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    TRACE(1, "read hdl 0x%x shdl 0x%x", hdl, gma_env->shdl);

    if (hdl == (gma_env->shdl + GMA_IDX_NTF_CFG))
    {
        uint16_t notify_ccc = gma_env->ntfIndEnableFlag[conidx];
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
    }
    else if (hdl == (gma_env->shdl + GMA_IDX_INDICATE_CFG))
    {
        uint16_t notify_ccc = gma_env->ntfIndEnableFlag[conidx];
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
    }
    else {
        dataLen = 0;
        status = ATT_ERR_REQUEST_NOT_SUPPORTED;
    }    

    gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, dataLen, p_data);

    // Release the buffer
    co_buf_release(p_data);
}

__STATIC void gma_att_event_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t dummy, uint16_t hdl,
                              uint16_t max_length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

__STATIC void gma_att_info_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl)
{
    uint16_t length = 0;
    uint16_t status = GAP_ERR_NO_ERROR;
   
    TRACE(1, "%s status 0x%x hdl %d", __func__, status, hdl);
 
    // Send the confirmation
    gatt_srv_att_info_get_cfm(conidx, user_lid, token, status, length);
}

__STATIC void gma_att_set_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                         uint16_t offset, co_buf_t* p_buf)
{
    // Get the address of the environment
    PRF_ENV_T(ai) *gma_env = PRF_ENV_GET(GMA, ai);
    uint8_t status = GAP_ERR_NO_ERROR;

    TRACE(4, "%s buds_env 0x%x write handle 0x%x shdl 0x%x conidx 0x%x", 
       __func__, (uint32_t)gma_env, hdl, gma_env->shdl, conidx);

    uint8_t* pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;

    DUMP8("%02x ", pData, dataLen);

    if (gma_env != NULL)
    {
        // TX ccc
        if (hdl == (gma_env->shdl + GMA_IDX_NTF_CFG))
        {
           uint16_t value = 0x0000;

           //Extract value before check
           memcpy(&value, pData, sizeof(uint16_t));
           TRACE(0, " GMA_IDX_NTF_CFG value 0x%x",  value);

           if (value <= (PRF_CLI_START_IND|PRF_CLI_START_NTF))
           {
                ai_event_ind_t *ind = NULL;
                if(value == 0)
                {
                    ind = KE_MSG_ALLOC(PRF_AI_DISCONNECT_IND,
                                       TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
                }
                else
                {
                    gma_env->ntfIndEnableFlag[conidx] = value;
                    ind = KE_MSG_ALLOC(PRF_AI_CONNECT_IND,
                                       TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
                }

                ind->conidx = conidx;
                ind->ai_type = SVC_AI_GMA;
                ind->data_len = 0;
                ke_msg_send(ind);
            }
            else 
            {
                status = PRF_APP_ERROR;
            }
        }
        // RX data
        else if ((hdl == (gma_env->shdl + GMA_IDX_WRITE_VAL))
                 ||(hdl == (gma_env->shdl + GMA_IDX_INDICATE_CFG))
                 ||(hdl == (gma_env->shdl + GMA_IDX_WR_NO_RSP_VAL)))
        {
           TRACE(0, "GMA_IDX_WRITE_VAL dataLen %d", dataLen);
           DUMP8("0x%x ", pData, dataLen);
           ai_event_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_CMD_RECEIVED_IND,
                                               TASK_APP, PRF_SRC_TASK(AI), ai_event_ind, dataLen);
           ind->conidx = conidx;
           ind->ai_type = SVC_AI_GMA;
           ind->data_len = dataLen;
           memcpy(ind->data, pData, dataLen);
           ke_msg_send(ind);
        }
        else
        {
           status = PRF_APP_ERROR;
        }
    }

    // Inform GATT about handling
    gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
}

/// Set of callbacks functions for communication with GATT as a GATT User Server
__STATIC const gatt_srv_cb_t gma_gatt_srv_cb = {
    .cb_event_sent      = gma_event_sent_cb,
    .cb_att_read_get    = gma_att_read_get_cb,
    .cb_att_event_get   = gma_att_event_get_cb,
    .cb_att_info_get    = gma_att_info_get_cb,
    .cb_att_val_set     = gma_att_set_cb,
};

static int gma_data_send_handler(ke_msg_id_t const msgid,
                                         struct ai_data_send_cfm *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    uint8_t val_offset = 0;
    enum gatt_evt_type evtType = param->gatt_event_type;

    PRF_ENV_T(ai) *gma_env = PRF_ENV_GET(GMA, ai);

    TRACE(1, "ntfIndEnableFlag %d, %d", gma_env->ntfIndEnableFlag[param->conidx],
        1 << (uint8_t)evtType);

    if ((gma_env->ntfIndEnableFlag[param->conidx]))
    {
        co_buf_t* p_buf = NULL;
        prf_buf_alloc(&p_buf, param->data_len);

        uint8_t* p_data = co_buf_data(p_buf);
        memcpy(p_data, param->data,  param->data_len);

        // Dummy parameter provided to GATT
        uint16_t dummy = 0;

        if (evtType)
        {
            val_offset = GMA_IDX_NTF_VAL;
        }
        else
        {
            val_offset = GMA_IDX_INDICATE_VAL;
        }
        // Inform the GATT that indication must be sent
        uint16_t ret = gatt_srv_event_send(param->conidx, gma_env->srv_user_lid, dummy, evtType,
                            gma_env->shdl + val_offset, p_buf);

        // Release the buffer
        co_buf_release(p_buf);

        if(ret){
            TRACE(1, "%s[ERROR]ama send fail. err=%x!", __func__, ret);
        }
    }
    else
    {
        TRACE(1, "%s[ERROR]ama send fail. %d, %d!", __func__,
               gma_env->ntfIndEnableFlag[param->conidx], (1 << (uint8_t)evtType));
    }

    return (KE_MSG_CONSUMED);
}


/* Default State handlers definition. */
KE_MSG_HANDLER_TAB(gma)
{
    {GMA_DATA_DEND_CFM,      (ke_msg_func_t) gma_data_send_handler},
};

static void gma_task_init(struct ke_task_desc *task_desc, PRF_ENV_T(ai) *gma_env)
{
    task_desc->msg_handler_tab = gma_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(gma_msg_handler_tab);
    task_desc->state           = &gma_env->state;
    task_desc->idx_max         = BLE_CONNECTION_MAX;
}

/**
 ****************************************************************************************
 * @brief Initialization of the GMA module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    env        Collector or Service allocated environment data.
 * @param[in|out] start_hdl  Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task   Application task number.
 * @param[in]     sec_lvl    Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     param      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint16_t gma_init(prf_data_t* env, uint16_t* start_hdl, uint8_t sec_lvl, uint8_t user_prio,
                            const void* params, const void* p_cb)
{
    uint16_t status = GAP_ERR_NO_ERROR;;
    TRACE(0, "gma_init returns %d start handle is %d sec_lvl 0x%x", status, *start_hdl, sec_lvl);

    // Allocate BUDS required environment variable
    PRF_ENV_T(ai)* gma_env =
            (PRF_ENV_T(ai)* ) ke_malloc(sizeof(PRF_ENV_T(ai)), KE_MEM_PROFILE);

    memset((uint8_t *)gma_env, 0, sizeof(PRF_ENV_T(ai)));

    // Register as GATT User Client
    status = gatt_user_srv_register(PREFERRED_BLE_MTU, 0, &gma_gatt_srv_cb,
                                    &(gma_env->srv_user_lid));

     //-------------------- allocate memory required for the profile  ---------------------
    if (GAP_ERR_NO_ERROR == status)
    {
        gma_env->shdl     = *start_hdl;
        status = gatt_db_svc16_add(gma_env->srv_user_lid, SVC_SEC_LVL(NOT_ENC) | SVC_UUID(16), GMA_GATT_UUID_SERVICE,
                                   GMA_IDX_NB, NULL, gma_att_db, GMA_IDX_NB,
                                   &gma_env->shdl);

        if(GAP_ERR_NO_ERROR == status)
        {
            *start_hdl = gma_env->shdl;
            // Initialize BUDS environment
            env->p_env = (prf_hdr_t*) gma_env;
            gma_task_init(&(env->desc), gma_env);
        }

       TRACE(1, "%s status %d nb_att %d shdl %d", __func__, status, GMA_IDX_NB, gma_env->shdl);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the TENCENT_SMARTVOICE module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static uint16_t gma_destroy(prf_data_t* p_env, uint8_t reason)
{
    PRF_ENV_T(ai)* gma_env = (PRF_ENV_T(ai)*) p_env->p_env;

    TRACE(0, "%s reason 0x%x", __func__, reason);
    // free profile environment variables
    p_env->p_env = NULL;
    ke_free(gma_env);

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void gma_create(prf_data_t* env, uint8_t conidx, bool flag)
{
        TRACE(3,"%s env %p conidx %d", __func__, env, conidx);
        
#if 0
    PRF_ENV_T(ai)* gma_env = (PRF_ENV_T(ai)*) env->env;
    struct prf_svc gma_svc = {tencent_env->shdl, gma_env->shdl + GMA_IDX_NB};
    prf_register_atthdl2gatt(env->env, conidx, &gma_svc);
#endif
}

/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void gma_cleanup(prf_data_t* env, uint8_t conidx, uint16_t reason)
{
    TRACE(4,"%s env %p, conidx %d reason %d", __func__, env, conidx, reason);
    /* Nothing to do */
}

/**
 ****************************************************************************************
 * @brief Indicates update of connection parameters
 *
 * @param[in|out]    env          Collector or Service allocated environment data.
 * @param[in]        conidx       Connection index
 * @param[in]        p_con_param  Pointer to new connection parameters information
 ****************************************************************************************
 */
static void gma_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param)
{
    TRACE(0, "%s", __func__);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// GMA Task interface required by profile manager
const prf_task_cbs_t gma_itf = {
    (prf_init_cb) gma_init,
    gma_destroy,
    gma_create,
    gma_cleanup,
    (prf_con_upd_cb)gma_upd,
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
const struct prf_task_cbs* gma_prf_itf_get(void)
{
    return &gma_itf;
}
#endif


