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

#if BLE_DMA_VOICE
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "gatt.h"
#include "gatt_msg.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "co_utils.h"
#include "prf_dbg.h"
#include "ai.h"

#define DMA_MAX_LEN                 (244)

///Attributes State Machine
enum
{
    DMA_IDX_SVC,

    DMA_IDX_CMD_TX_CHAR,
    DMA_IDX_CMD_TX_VAL,
    DMA_IDX_CMD_TX_NTF_CFG,

    DMA_IDX_CMD_RX_CHAR,
    DMA_IDX_CMD_RX_VAL,

    DMA_IDX_DATA_TX_CHAR,
    DMA_IDX_DATA_TX_VAL,
    DMA_IDX_DATA_TX_NTF_CFG,

    DMA_IDX_DATA_RX_CHAR,
    DMA_IDX_DATA_RX_VAL,

    DMA_IDX_NB,
};

/*
 * DMA CMD PROFILE ATTRIBUTES
 ****************************************************************************************
 */
#define dma_service_uuid_128_content  			{0xfb,0x34,0x9b,0x5f,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x04,0xfe,0x00,0x00}
#define dma_cmd_tx_char_val_uuid_128_content  	{0x01,0x00,0x82,0x6f,0x63,0x2e,0x74,0x6e,0x69,0x6f,0x70,0x6c,0x65,0x63,0x78,0x65}	
#define dma_cmd_rx_char_val_uuid_128_content  	{0x02,0x00,0x82,0x6f,0x63,0x2e,0x74,0x6e,0x69,0x6f,0x70,0x6c,0x65,0x63,0x78,0x65}
#define dma_data_tx_char_val_uuid_128_content  	{0x0f,0x33,0x84,0x47,0x53,0x9d,0xa1,0xbb,0xd4,0x46,0xc5,0x29,0xc6,0xc9,0x4a,0xb8}//dma tx
#define dma_data_rx_char_val_uuid_128_content   {0xc5,0x3f,0x19,0x93,0xa5,0x98,0xcb,0xb0,0xe0,0x41,0x78,0x0e,0xb9,0x58,0xe7,0xc2}//dam rx


static const uint8_t DMA_SERVICE_UUID_128[GATT_UUID_128_LEN] = dma_service_uuid_128_content;

/// Full DMA SERVER Database Description - Used to add attributes into the database
const struct gatt_att_desc dma_att_db[DMA_IDX_NB] =
{
    // Service Declaration
    [DMA_IDX_SVC]            = {ATT_DECL_PRIMARY_SERVICE_UUID, PROP(RD), 0},

    // Command TX Characteristic Declaration
    [DMA_IDX_CMD_TX_CHAR]    = {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Command TX Characteristic Value
    [DMA_IDX_CMD_TX_VAL]     = {dma_cmd_tx_char_val_uuid_128_content, 
                                PROP(N) | PROP(RD) | SEC_LVL(RP, AUTH) | ATT_UUID(128), DMA_MAX_LEN},
    // Command TX Characteristic - Client Characteristic Configuration Descriptor
    [DMA_IDX_CMD_TX_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG_UUID,
                                PROP(RD) | PROP(WR) | SEC_LVL(NIP, AUTH), 0},

    // Command RX Characteristic Declaration
    [DMA_IDX_CMD_RX_CHAR]     = {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Command RX Characteristic Value
    [DMA_IDX_CMD_RX_VAL]      = {dma_cmd_rx_char_val_uuid_128_content, 
                                 PROP(WR) | SEC_LVL(WP, AUTH) | ATT_UUID(128), DMA_MAX_LEN},

    // Data TX Characteristic Declaration
    [DMA_IDX_DATA_TX_CHAR]    = {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Data TX Characteristic Value
    [DMA_IDX_DATA_TX_VAL]     = {dma_data_tx_char_val_uuid_128_content,
                                 PROP(N) | PROP(RD) | SEC_LVL(RP, AUTH) | ATT_UUID(128), DMA_MAX_LEN},
    // Data TX Characteristic - Client Characteristic Configuration Descriptor
    [DMA_IDX_DATA_TX_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR) | SEC_LVL(NIP, AUTH), 0},

    // Data RX Characteristic Declaration
    [DMA_IDX_DATA_RX_CHAR]    = {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Data RX Characteristic Value
    [DMA_IDX_DATA_RX_VAL]     = {dma_data_rx_char_val_uuid_128_content, 
                                 PROP(WR) | SEC_LVL(WP, AUTH) | ATT_UUID(128), DMA_MAX_LEN},	
};
static void dma_event_sent_cb(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    // notification or indication has been sent out
    TRACE(0, "%s conidx 0x%x", __func__, conidx);

    ai_event_ind_t *ind = KE_MSG_ALLOC(PRF_AI_TX_DONE_IND,
                                 TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
    ind->conidx = conidx;
    ind->ai_type = SVC_AI_DMA;
    ind->data_len = 0;
    ke_msg_send(ind);
}

static void dma_att_read_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                        uint16_t offset, uint16_t max_length)
{
    co_buf_t* p_data = NULL;
    uint16_t dataLen = 0;
    uint16_t status = GAP_ERR_NO_ERROR;

    // Get the address of the environment
    PRF_ENV_T(ai) *dma_env = PRF_ENV_GET(DMA, ai);

    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    TRACE(1, "read hdl 0x%x shdl 0x%x", hdl, dma_env->shdl);

    if (hdl == (dma_env->shdl + DMA_IDX_CMD_TX_NTF_CFG) || hdl == (dma_env->shdl + DMA_IDX_DATA_TX_NTF_CFG))
    {
        uint16_t notify_ccc = dma_env->ntfIndEnableFlag[conidx];
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

static void dma_att_event_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t dummy, uint16_t hdl,
                              uint16_t max_length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

static void dma_att_info_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl)
{
    uint16_t length = 0;
    uint16_t status = GAP_ERR_NO_ERROR;
    
    PRF_ENV_T(ai) *dma_env = PRF_ENV_GET(DMA, ai);

    // check if it's a client configuration char
    if (hdl == dma_env->shdl + DMA_IDX_CMD_TX_NTF_CFG)
    {
        // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    }
    else if (hdl == dma_env->shdl + DMA_IDX_DATA_TX_NTF_CFG)
    {
        // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    }
    else if (hdl == dma_env->shdl + DMA_IDX_CMD_RX_VAL)
    {
        // force length to zero to reject any write starting from something != 0
        length = 0;
        status = GAP_ERR_NO_ERROR;
    }
    else if (hdl == dma_env->shdl + DMA_IDX_DATA_RX_VAL)
    {
        // force length to zero to reject any write starting from something != 0
        length = 0;
        status = GAP_ERR_NO_ERROR;
    }
    // not expected request
    else
    {
        length = 0;
        status = ATT_ERR_WRITE_NOT_PERMITTED;
    }

    TRACE(1, "%s status 0x%x hdl %d", __func__, status, hdl);

    // Send the confirmation
    gatt_srv_att_info_get_cfm(conidx, user_lid, token, status, length);
}

static void dma_att_set_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                          uint16_t offset, co_buf_t* p_buf)
{
    // Get the address of the environment
    PRF_ENV_T(ai) *dma_env = PRF_ENV_GET(DMA, ai);

    uint8_t status = GAP_ERR_NO_ERROR;

    TRACE(4, "%s buds_env 0x%x write handle 0x%x shdl 0x%x conidx 0x%x", 
        __func__, (uint32_t)dma_env, hdl, dma_env->shdl, conidx);

    uint8_t* pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;

    DUMP8("%02x ", pData, dataLen);

    if (dma_env != NULL)
    {
        TRACE(1, "%s", __func__);
        // TX ccc
        if (hdl == (dma_env->shdl + DMA_IDX_DATA_TX_NTF_CFG))
        {
            uint16_t value = 0x0000;

            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));
            TRACE(0, "DMA_IDX_TX_NTF_CFG 0x%x value 0x%x", DMA_IDX_CMD_TX_NTF_CFG, value);

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
                    dma_env->ntfIndEnableFlag[conidx] = value;
                    ind = KE_MSG_ALLOC(PRF_AI_CONNECT_IND,
                                       TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
                }

                ind->conidx = conidx;
                ind->ai_type = SVC_AI_DMA;
                ind->data_len = 0;
                ke_msg_send(ind);
            }
            else 
            {
                status = PRF_APP_ERROR;
            }
        }
        // RX data
        else if (hdl == (dma_env->shdl + DMA_IDX_DATA_RX_VAL))
        {
            TRACE(0, "DMA_IDX_RX_VAL 0x%x dataLen %d", DMA_IDX_DATA_RX_VAL, dataLen);
            DUMP8("0x%x ", pData, dataLen);

            ai_event_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_DATA_RECEIVED_IND,
                                                TASK_APP, PRF_SRC_TASK(AI), ai_event_ind, dataLen);
            ind->conidx = conidx;
            ind->ai_type = SVC_AI_DMA;
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
static const gatt_srv_cb_t dma_gatt_srv_cb = {
    .cb_event_sent      = dma_event_sent_cb,
    .cb_att_read_get    = dma_att_read_get_cb,
    .cb_att_event_get   = dma_att_event_get_cb,
    .cb_att_info_get    = dma_att_info_get_cb,
    .cb_att_val_set     = dma_att_set_cb,
};

static int dma_data_send_handler(ke_msg_id_t const msgid,
                                         struct ai_data_send_cfm *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    enum gatt_evt_type evtType = param->gatt_event_type;

    PRF_ENV_T(ai) *dma_env = PRF_ENV_GET(DMA, ai);

    TRACE(1, "ntfIndEnableFlag %d, %d", dma_env->ntfIndEnableFlag[param->conidx],
        1 << (uint8_t)evtType);

    if ((dma_env->ntfIndEnableFlag[param->conidx]) & (1 << (uint8_t)evtType))
    {
        co_buf_t* p_buf = NULL;
        prf_buf_alloc(&p_buf, param->data_len);

        uint8_t* p_data = co_buf_data(p_buf);
        memcpy(p_data, param->data, param->data_len);

        // Dummy parameter provided to GATT
        uint16_t dummy = 0;

        // Inform the GATT that notification must be sent
        uint16_t ret = gatt_srv_event_send(param->conidx, dma_env->srv_user_lid, dummy, evtType,
                            dma_env->shdl + DMA_IDX_DATA_TX_VAL, p_buf);

        // Release the buffer
        co_buf_release(p_buf);

        if(ret){
            TRACE(1, "%s[ERROR]ama send fail. err=%x!", __func__, ret);
        }
    }
    else
    {
        TRACE(1, "%s[ERROR]ama send fail. %d, %d!", __func__,
               dma_env->ntfIndEnableFlag[param->conidx], (1 << (uint8_t)evtType));
    }

    return (KE_MSG_CONSUMED);
}

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(dma) { /// AKA ancc_msg_handler_tab
    /// handlers for command from upper layer
    {DMA_DATA_DEND_CFM,       (ke_msg_func_t)dma_data_send_handler},
};

static void dma_task_init(struct ke_task_desc *task_desc, PRF_ENV_T(ai) *dma_env)
{
    TRACE(1, "%s Entry.", __func__);

    task_desc->msg_handler_tab = dma_msg_handler_tab;
    task_desc->msg_cnt = ARRAY_LEN(dma_msg_handler_tab);
    task_desc->state = &dma_env->state;
    task_desc->idx_max = BLE_CONNECTION_MAX;
}

/**
 ****************************************************************************************
 * @brief Initialization of the DMA VOICE module.
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
static uint16_t dma_init(prf_data_t* env, uint16_t* start_hdl, 
        uint8_t sec_lvl, uint8_t user_prio, const void* params, const void* p_cb)
{
    uint16_t status = GAP_ERR_NO_ERROR;

    TRACE(0, "ama_init returns %d start handle is %d sec_lvl 0x%x", status, *start_hdl, sec_lvl);

    // Allocate BUDS required environment variable
    PRF_ENV_T(ai)* dma_env =
            (PRF_ENV_T(ai)* ) ke_malloc(sizeof(PRF_ENV_T(ai)), KE_MEM_PROFILE);

    memset((uint8_t *)dma_env, 0, sizeof(PRF_ENV_T(ai)));

    // Register as GATT User Client
    status = gatt_user_srv_register(PREFERRED_BLE_MTU, 0, &dma_gatt_srv_cb,
                                    &(dma_env->srv_user_lid));

    //-------------------- allocate memory required for the profile  ---------------------
    if (GAP_ERR_NO_ERROR == status)
    {
        dma_env->shdl     = *start_hdl;
        status = gatt_db_svc_add(dma_env->srv_user_lid, SVC_SEC_LVL(NOT_ENC) | SVC_UUID(128), DMA_SERVICE_UUID_128,
                                   DMA_IDX_NB, NULL, dma_att_db, DMA_IDX_NB,
                                   &dma_env->shdl);
        if(GAP_ERR_NO_ERROR == status)
        {
            *start_hdl = dma_env->shdl;
            // Initialize BUDS environment
            env->p_env = (prf_hdr_t*) dma_env;
            dma_task_init(&(env->desc), dma_env);
        }

       TRACE(1, "%s status %d nb_att %d shdl %d", __func__, status, DMA_IDX_NB, dma_env->shdl);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the SMARTVOICE module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static uint16_t dma_destroy(prf_data_t* p_env, uint8_t reason)
{
    PRF_ENV_T(ai)* dma_env = (PRF_ENV_T(ai)*) p_env->p_env;

    TRACE(0, "%s reason 0x%x", __func__, reason);
    // free profile environment variables
    p_env->p_env = NULL;
    ke_free(dma_env);

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
static void dma_create(prf_data_t* env, uint8_t conidx, bool flag)
{
    TRACE(3,"%s env %p conidx %d", __func__, env, conidx);
    
#if 0
    PRF_ENV_T(ai)* dma_env = (PRF_ENV_T(ai)*) env->env;
    struct prf_svc dma_svc = {dma_env->shdl, dma_env->shdl + DMA_IDX_NB};
    prf_register_atthdl2gatt(env->env, conidx, &dma_svc);
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
static void dma_cleanup(prf_data_t* env, uint8_t conidx, uint16_t reason)
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
static void dma_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param)
{
    TRACE(0, "%s", __func__);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// AMA Task interface required by profile manager
const prf_task_cbs_t dma_itf =
{
    dma_init,
    dma_destroy,
    dma_create,
    dma_cleanup,
    dma_upd,
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
const struct prf_task_cbs* dma_prf_itf_get(void)
{
    return &dma_itf;
}
#endif


