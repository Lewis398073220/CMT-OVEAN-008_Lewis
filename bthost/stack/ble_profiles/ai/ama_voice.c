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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if BLE_AMA_VOICE
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
 * AMA CMD PROFILE ATTRIBUTES
 ****************************************************************************************
 */
///Attributes State Machine
enum
{
    AMA_IDX_SVC,

    AMA_IDX_TX_CHAR,
    AMA_IDX_TX_VAL,
    AMA_IDX_TX_NTF_CFG,
    AMA_IDX_RX_CHAR,
    AMA_IDX_RX_VAL,

    AMA_IDX_NB,
};

#define ama_service_uuid_128_content        {0xFB,0x34,0x9B,0x5F,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,0x03,0xFE,0x00,0x00}
#define ama_rx_char_val_uuid_128_content    {0x76,0x30,0xF8,0xDD,0x90,0xA3,0x61,0xAC,0xA7,0x43,0x05,0x30,0x77,0xB1,0x4E,0xF0}
#define ama_tx_char_val_uuid_128_content    {0x0B,0x42,0x82,0x1F,0x64,0x72,0x2F,0x8A,0xB4,0x4B,0x79,0x18,0x5B,0xA0,0xEE,0x2B}

static const uint8_t AMA_SERVICE_UUID_128[GATT_UUID_128_LEN] = ama_service_uuid_128_content;

/// Full AMA SERVER Database Description - Used to add attributes into the database
static const struct gatt_att_desc ama_att_db[AMA_IDX_NB] =
{
    // Service Declaration
    [AMA_IDX_SVC]       =   {ATT_DECL_PRIMARY_SERVICE_UUID, PROP(RD), 0},

    // Command RX Characteristic Declaration
    [AMA_IDX_RX_CHAR]   =   {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Command RX Characteristic Value
    [AMA_IDX_RX_VAL]    =   {ama_rx_char_val_uuid_128_content,
                             PROP(WR) | SEC_LVL(WP, AUTH) | ATT_UUID(128), AI_MAX_LEN},

    // Command TX Characteristic Declaration
    [AMA_IDX_TX_CHAR]   =   {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Command TX Characteristic Value
    [AMA_IDX_TX_VAL]    =   {ama_tx_char_val_uuid_128_content,
                             PROP(N) | PROP(RD) | SEC_LVL(RP, AUTH) | ATT_UUID(128), AI_MAX_LEN},

    // Command TX Characteristic - Client Characteristic Configuration Descriptor
    [AMA_IDX_TX_NTF_CFG] =  {ATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR) | SEC_LVL(NIP, AUTH), 0},
};

static void ama_event_sent_cb(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    // notification or indication has been sent out
    TRACE(0, "%s conidx 0x%x", __func__, conidx);

    ai_event_ind_t *ind = KE_MSG_ALLOC(PRF_AI_TX_DONE_IND,
                                 TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
    ind->conidx = conidx;
    ind->ai_type = SVC_AI_AMA;
    ind->data_len = 0;
    ke_msg_send(ind);
}

static void ama_att_read_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token,
                                         uint16_t hdl, uint16_t offset, uint16_t max_length)
{
    co_buf_t* p_data = NULL;
    uint16_t dataLen = 0;
    uint16_t status = GAP_ERR_NO_ERROR;

    // Get the address of the environment
    PRF_ENV_T(ai) *ama_env = PRF_ENV_GET(AMA, ai);

    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    TRACE(1, "read hdl 0x%x shdl 0x%x", hdl, ama_env->shdl);

    if (hdl == (ama_env->shdl + AMA_IDX_TX_NTF_CFG))
    {
        uint16_t notify_ccc = ama_env->ntfIndEnableFlag[conidx];
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

static void ama_att_event_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token,
                                                uint16_t dummy, uint16_t hdl, uint16_t max_length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

static void ama_att_info_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl)
{
    uint16_t length = 0;
    uint16_t status = GAP_ERR_NO_ERROR;
    
    PRF_ENV_T(ai) *ama_env = PRF_ENV_GET(AMA, ai);

    // check if it's a client configuration char
    if (hdl == ama_env->shdl + AMA_IDX_TX_NTF_CFG)
    {
        // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    }
    else if (hdl == ama_env->shdl + AMA_IDX_RX_VAL)
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

static bool ama_att_has_connected()
{
    // Get the address of the environment
    PRF_ENV_T(ai) *ama_env = PRF_ENV_GET(AMA, ai);
    for(uint8_t i = 0 ; i < BLE_CONNECTION_MAX; i++)
    {
        if(ama_env->ntfIndEnableFlag[i] & (PRF_CLI_START_IND|PRF_CLI_START_NTF))
        {
            return true;
        }
    }
    return false;
}

static void ama_att_set_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                 uint16_t offset, co_buf_t* p_buf)
{
    // Get the address of the environment
    PRF_ENV_T(ai) *ama_env = PRF_ENV_GET(AMA, ai);

    TRACE(4, "%s env 0x%x write handle 0x%x shdl 0x%x conidx 0x%x", 
        __func__, (uint32_t)ama_env, hdl, ama_env->shdl, conidx);

    uint8_t* pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;

    DUMP8("%02x ", pData, dataLen);

    if (ama_env == NULL || \
        (ama_att_has_connected() && (hdl == ama_env->shdl + AMA_IDX_TX_NTF_CFG)) || \
        (hdl != (ama_env->shdl + AMA_IDX_TX_NTF_CFG) \
        && hdl != (ama_env->shdl + AMA_IDX_RX_VAL)))
    {
        // Inform GATT about handling
        gatt_srv_att_val_set_cfm(conidx, user_lid, token, PRF_APP_ERROR);
        return;
    }
    else
    {
        // Inform GATT about handling
        gatt_srv_att_val_set_cfm(conidx, user_lid, token, GAP_ERR_NO_ERROR);
        // TX ccc
        if (hdl == (ama_env->shdl + AMA_IDX_TX_NTF_CFG))
        {
            uint16_t value = 0x0000;

            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));
            TRACE(0, "AMA_IDX_TX_NTF_CFG 0x%x value 0x%x", AMA_IDX_TX_NTF_CFG, value);

            if (value <= (PRF_CLI_START_IND|PRF_CLI_START_NTF))
            {
                ai_event_ind_t *ind = NULL;
                ama_env->ntfIndEnableFlag[conidx] = value;
                if (value == 0)
                {
                    ind = KE_MSG_ALLOC(PRF_AI_DISCONNECT_IND,
                                       TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
                }
                else
                {
                    ind = KE_MSG_ALLOC(PRF_AI_CONNECT_IND,
                                                        TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
                }

                ind->conidx = conidx;
                ind->ai_type = SVC_AI_AMA;
                ind->data_len = 0;
                ke_msg_send(ind);
            }
        }
        // RX data
        else if (hdl == (ama_env->shdl + AMA_IDX_RX_VAL))
        {
            TRACE(0, "AMA_IDX_RX_VAL 0x%x dataLen %d", AMA_IDX_RX_VAL, dataLen);
            DUMP8("0x%x ", pData, dataLen);

            ai_event_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_CMD_RECEIVED_IND,
                                                TASK_APP, PRF_SRC_TASK(AI), ai_event_ind, dataLen);
            ind->conidx = conidx;
            ind->ai_type = SVC_AI_AMA;
            ind->data_len = dataLen;
            memcpy(ind->data, pData, dataLen);
            ke_msg_send(ind);
        }
        else
        {
        }
    }
}

/// Set of callbacks functions for communication with GATT as a GATT User Server
static const gatt_srv_cb_t ama_gatt_srv_cb = {
    .cb_event_sent      = ama_event_sent_cb,
    .cb_att_read_get    = ama_att_read_get_cb,
    .cb_att_event_get   = ama_att_event_get_cb,
    .cb_att_info_get    = ama_att_info_get_cb,
    .cb_att_val_set     = ama_att_set_cb,
};

static int ama_data_send_handler(ke_msg_id_t const msgid,
                                         struct ai_data_send_cfm *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    enum gatt_evt_type evtType = param->gatt_event_type;
    PRF_ENV_T(ai) *ama_env = PRF_ENV_GET(AMA, ai);

    TRACE(1, "ntfIndEnableFlag %d, %d", ama_env->ntfIndEnableFlag[param->conidx],
        1 << (uint8_t)evtType);

    if ((ama_env->ntfIndEnableFlag[param->conidx]) & (1 << (uint8_t)evtType))
    {
        co_buf_t* p_buf = NULL;
        prf_buf_alloc(&p_buf, param->data_len);

        uint8_t* p_data = co_buf_data(p_buf);
        memcpy(p_data, param->data, param->data_len);

        // Dummy parameter provided to GATT
        uint16_t dummy = 0;

        // Inform the GATT that notification must be sent
        uint16_t ret = gatt_srv_event_send(param->conidx, ama_env->srv_user_lid, dummy, evtType,
                            ama_env->shdl + AMA_IDX_TX_VAL, p_buf);

        // Release the buffer
        co_buf_release(p_buf);

        if(ret){
            TRACE(1, "%s[ERROR]ama send fail. err=%x!", __func__, ret);
        }
    }
    else
    {
        TRACE(1, "%s[ERROR]ama send fail. %d, %d!", __func__,
               ama_env->ntfIndEnableFlag[param->conidx], (1 << (uint8_t)evtType));
    }

    return (KE_MSG_CONSUMED);
}

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(ama) { /// AMA ancc_msg_handler_tab
    /// handlers for command from upper layer
    {AMA_DATA_DEND_CFM,       (ke_msg_func_t)ama_data_send_handler},
};

static void ama_task_init(struct ke_task_desc *task_desc, PRF_ENV_T(ai) *ama_env)
{
    TRACE(1, "%s Entry.", __func__);

    task_desc->msg_handler_tab = ama_msg_handler_tab;
    task_desc->msg_cnt = ARRAY_LEN(ama_msg_handler_tab);
    task_desc->state   = &ama_env->state;
    task_desc->idx_max = BLE_CONNECTION_MAX;
}


/**
 ****************************************************************************************
 * @brief Initialization of the SMARTVOICE module.
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
static uint16_t ama_init(prf_data_t* env, uint16_t* start_hdl,uint8_t sec_lvl,
                             uint8_t user_prio, const void* params, const void* p_cb)
{
    uint16_t status = GAP_ERR_NO_ERROR;

    TRACE(0, "ama_init returns %d start handle is %d sec_lvl 0x%x", status, *start_hdl, sec_lvl);

    // Allocate AMA required environment variable
    PRF_ENV_T(ai)* ama_env = (PRF_ENV_T(ai)* ) ke_malloc(sizeof(PRF_ENV_T(ai)), KE_MEM_PROFILE);

    memset((uint8_t *)ama_env, 0, sizeof(PRF_ENV_T(ai)));

    // Register as GATT User Client
    status = gatt_user_srv_register(PREFERRED_BLE_MTU, user_prio, &ama_gatt_srv_cb,
                                    &(ama_env->srv_user_lid));

    //-------------------- allocate memory required for the profile  ---------------------
    if (GAP_ERR_NO_ERROR == status)
    {
        ama_env->shdl     = *start_hdl;
        status = gatt_db_svc_add(ama_env->srv_user_lid, SVC_SEC_LVL(NOT_ENC) | SVC_UUID(128), AMA_SERVICE_UUID_128,
                                   AMA_IDX_NB, NULL, ama_att_db, AMA_IDX_NB,
                                   &ama_env->shdl);
        if(GAP_ERR_NO_ERROR == status)
        {
            *start_hdl = ama_env->shdl;
            // Initialize BUDS environment
            env->p_env = (prf_hdr_t*) ama_env;
            ama_task_init(&(env->desc), ama_env);
            TRACE(1, "%s status %d nb_att %d shdl %d", __func__, status, AMA_IDX_NB, ama_env->shdl);
        }
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
static uint16_t ama_destroy(prf_data_t* p_env, uint8_t reason)
{
    PRF_ENV_T(ai)* ama_env = (PRF_ENV_T(ai)*) p_env->p_env;

    TRACE(0, "%s reason 0x%x", __func__, reason);
    // free profile environment variables
    p_env->p_env = NULL;
    ke_free(ama_env);

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
static void ama_create(prf_data_t* env, uint8_t conidx, bool is_le_con)
{
    TRACE(3,"%s env %p conidx %d", __func__, env, conidx);

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
static void ama_cleanup(prf_data_t* env, uint8_t conidx, uint16_t reason)
{
    TRACE(4,"%s env %p, conidx %d reason %d", __func__, env, conidx, reason);
    PRF_ENV_T(ai)* ama_env = (PRF_ENV_T(ai)*) env->p_env;
    ama_env->ntfIndEnableFlag[conidx] = 0;
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
static void ama_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param)
{
    TRACE(0, "%s", __func__);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// AMA Task interface required by profile manager
const prf_task_cbs_t ama_itf =
{
    ama_init,
    ama_destroy,
    ama_create,
    ama_cleanup,
    ama_upd,
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
const struct prf_task_cbs* ama_prf_itf_get(void)
{
    return &ama_itf;
}

#endif


