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

#if BLE_CUSTOMIZE_VOICE
#include "gatt.h"
#include "gatt_msg.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "co_utils.h"
#include "prf_dbg.h"
#include "ai.h"

#ifdef __GATT_OVER_BR_EDR__
#include "app_btgatt.h"
#endif


/*
 *VOICE CMD PROFILE ATTRIBUTES
 ****************************************************************************************
 */
#define cus_service_uuid_128_content            {0x00,0x00,0x82,0x6f,0x63,0x2e,0x74,0x6e,0x69,0x6f,0x70,0x6c,0x65,0x63,0x78,0x65}
#define cus_cmd_tx_char_val_uuid_128_content    {0x01,0x00,0x82,0x6f,0x63,0x2e,0x74,0x6e,0x69,0x6f,0x70,0x6c,0x65,0x63,0x78,0x65}	
#define cus_cmd_rx_char_val_uuid_128_content    {0x02,0x00,0x82,0x6f,0x63,0x2e,0x74,0x6e,0x69,0x6f,0x70,0x6c,0x65,0x63,0x78,0x65}
#define cus_data_tx_char_val_uuid_128_content   {0x03,0x00,0x82,0x6f,0x63,0x2e,0x74,0x6e,0x69,0x6f,0x70,0x6c,0x65,0x63,0x78,0x65}	
#define cus_data_rx_char_val_uuid_128_content   {0x04,0x00,0x82,0x6f,0x63,0x2e,0x74,0x6e,0x69,0x6f,0x70,0x6c,0x65,0x63,0x78,0x65}

///Attributes State Machine
enum {
    CUS_VOICE_IDX_SVC,

    CUS_VOICE_IDX_CMD_TX_CHAR,
    CUS_VOICE_IDX_CMD_TX_VAL,
    CUS_VOICE_IDX_CMD_TX_NTF_CFG,

    CUS_VOICE_IDX_CMD_RX_CHAR,
    CUS_VOICE_IDX_CMD_RX_VAL,

    CUS_VOICE_IDX_DATA_TX_CHAR,
    CUS_VOICE_IDX_DATA_TX_VAL,
    CUS_VOICE_IDX_DATA_TX_NTF_CFG,

    CUS_VOICE_IDX_DATA_RX_CHAR,
    CUS_VOICE_IDX_DATA_RX_VAL,

    CUS_VOICE_IDX_NB,
};

static const uint8_t CUS_SERVICE_UUID_128[GATT_UUID_128_LEN] = cus_service_uuid_128_content;

/// Full SMARTVOICE SERVER Database Description - Used to add attributes into the database
const struct gatt_att_desc customize_att_db[CUS_VOICE_IDX_NB] = {
    // Service Declaration
    [CUS_VOICE_IDX_SVC]             = {ATT_DECL_PRIMARY_SERVICE_UUID, PROP(RD), 0},

    // Command TX Characteristic Declaration
    [CUS_VOICE_IDX_CMD_TX_CHAR]     = {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Command TX Characteristic Value
    [CUS_VOICE_IDX_CMD_TX_VAL]      = {cus_cmd_tx_char_val_uuid_128_content, PROP(N) | PROP(I)
                                       | PROP(RD) | SEC_LVL(RP, AUTH) | ATT_UUID(128), AI_MAX_LEN},
    // Command TX Characteristic - Client Characteristic Configuration Descriptor
    [CUS_VOICE_IDX_CMD_TX_NTF_CFG]  = {ATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WC)
                                       | SEC_LVL(NIP, AUTH), 0},

    // Command RX Characteristic Declaration
    [CUS_VOICE_IDX_CMD_RX_CHAR]     = {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Command RX Characteristic Value
    [CUS_VOICE_IDX_CMD_RX_VAL]      = {cus_cmd_rx_char_val_uuid_128_content, PROP(WR) | PROP(WC)
                                       | SEC_LVL(WP, AUTH) | PROP(I) | ATT_UUID(128), AI_MAX_LEN},

    // Data TX Characteristic Declaration
    [CUS_VOICE_IDX_DATA_TX_CHAR]    = {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Data TX Characteristic Value
    [CUS_VOICE_IDX_DATA_TX_VAL]     = {cus_data_tx_char_val_uuid_128_content, PROP(N) | PROP(I)
                                       | PROP(RD) | SEC_LVL(RP, AUTH) | ATT_UUID(128), AI_MAX_LEN},
    // Data TX Characteristic - Client Characteristic Configuration Descriptor
    [CUS_VOICE_IDX_DATA_TX_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR)
                                       | SEC_LVL(NIP, AUTH), 0},

    // Data RX Characteristic Declaration
    [CUS_VOICE_IDX_DATA_RX_CHAR]    = {ATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // Data RX Characteristic Value
    [CUS_VOICE_IDX_DATA_RX_VAL]     = {cus_data_rx_char_val_uuid_128_content, PROP(WR) | PROP(WC)
                                       | SEC_LVL(WP, AUTH) | PROP(I) | ATT_UUID(128), AI_MAX_LEN},
};

/**
 ****************************************************************************************
 * @brief Handles reception of the read request from peer device
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static void customize_read_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                      uint16_t offset, uint16_t max_length)
{	
    // Get the address of the environment
    co_buf_t* p_data = NULL;
    uint16_t dataLen = 0;
    PRF_ENV_T(ai) *voice_env = PRF_ENV_GET(AI, ai);
    uint8_t status = GAP_ERR_NO_ERROR;

    BLE_GATT_DBG("gattc_read_req_ind_handler read handle %d shdl %d", param->handle, sv_env->shdl);

    if (hdl == (voice_env->shdl + CUS_VOICE_IDX_CMD_TX_NTF_CFG)) {
        uint16_t notify_ccc;
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, sizeof(notify_ccc));

        if (voice_env->ntfIndEnableFlag[conidx] & 1) {
        	notify_ccc = 1;
        } else {
        	notify_ccc = 0;
        }
        memcpy(p_data, (uint8_t *)&notify_ccc, dataLen);
    }
    if (hdl == (voice_env->shdl + CUS_VOICE_IDX_DATA_TX_NTF_CFG)) {
        uint16_t notify_ccc;
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, sizeof(notify_ccc));

        if (voice_env->ntfIndEnableFlag[conidx] & (1<<2)) {
        	notify_ccc = 1;
        } else {
        	notify_ccc = 0;
        }
        memcpy(p_data, (uint8_t *)&notify_ccc, dataLen);
    }
    else {
        dataLen = 0;
        p_data  = NULL;
        status = ATT_ERR_REQUEST_NOT_SUPPORTED;
    }
    gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, dataLen, p_data);
}

static void customize_event_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t dummy, uint16_t hdl,
                            uint16_t max_length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static void customize_info_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl)
{
    uint16_t status=0;
    uint16_t length = 0;

    //Send write response

    PRF_ENV_T(ai) *voice_env = PRF_ENV_GET(AI, ai);

    if ((hdl == (voice_env->shdl + CUS_VOICE_IDX_CMD_TX_NTF_CFG)) ||
        (hdl == (voice_env->shdl + CUS_VOICE_IDX_DATA_TX_NTF_CFG))) {
        length = 2;
        status = GAP_ERR_NO_ERROR;
	} else if ((hdl == (voice_env->shdl + CUS_VOICE_IDX_CMD_RX_VAL)) ||
		(hdl == (voice_env->shdl + CUS_VOICE_IDX_DATA_RX_VAL))) {
        // force length to zero to reject any write starting from something != 0
        length = 0;
        status = GAP_ERR_NO_ERROR;
    } else {
        length = 0;
        status = ATT_ERR_WRITE_NOT_PERMITTED;
    }

    gatt_srv_att_info_get_cfm(conidx, user_lid, token, status, length);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GL2C_CODE_ATT_WR_CMD_IND message.
 * The handler compares the new values with current ones and notifies them if they changed.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static void  customize_event_sent_cb(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    // notification or indication has been sent out
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    ai_event_ind_t *ind = KE_MSG_ALLOC(PRF_AI_TX_DONE_IND,
                                 TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
    ind->conidx = conidx;
    ind->ai_type = SVC_AI_CUSTOMIZE;
    ind->data_len = 0;
    ke_msg_send(ind);
}

static void customize_set_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                          uint16_t offset, co_buf_t* p_buf)
{
    // Get the address of the environment
    PRF_ENV_T(ai) *voice_env = PRF_ENV_GET(AI, ai);
    uint8_t* pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;
    uint8_t status = GAP_ERR_NO_ERROR;

    BLE_GATT_DBG("gattc_write_req_ind_handler sv_env 0x%x write handle %d shdl %d", 
        voice_env, hdl, voice_env->shdl);

    if (voice_env != NULL) {
        if (hdl == (voice_env->shdl + CUS_VOICE_IDX_CMD_TX_NTF_CFG)) {
            uint16_t value = 0x0000;

            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));

            if (value == PRF_CLI_STOP_NTFIND) {
                voice_env->ntfIndEnableFlag[conidx] &= (~1);
            } else if (value == PRF_CLI_START_NTF) {
                voice_env->ntfIndEnableFlag[conidx] |= 1;
            } else {
                status = PRF_APP_ERROR;
            }

            if (status == GAP_ERR_NO_ERROR) {
                //Inform APP of TX ccc change
                ai_change_ccc_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_CMD_CHANGGE_CCC_IND,
                                             TASK_APP, PRF_SRC_TASK(AI), ai_change_ccc_ind, 1);
                ind->conidx = conidx;
                ind->ai_type = SVC_AI_CUSTOMIZE;
                ind->ntf_ind_flag  = voice_env->ntfIndEnableFlag[conidx] & 1;
                ke_msg_send(ind);
            }
        }
        else if (hdl == (voice_env->shdl + CUS_VOICE_IDX_DATA_TX_NTF_CFG)) {
            uint16_t value = 0x0000;

            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));

            if (value == PRF_CLI_STOP_NTFIND) {
                voice_env->ntfIndEnableFlag[conidx] &= (~(1<<2));
            }
            else if (value == PRF_CLI_START_NTF) {
                voice_env->ntfIndEnableFlag[conidx] |= (1<<2);
            }
            else {
                status = PRF_APP_ERROR;
            }

            if (status == GAP_ERR_NO_ERROR) {
                //Inform APP of TX ccc change
                ai_change_ccc_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_DATA_CHANGGE_CCC_IND,
                                             TASK_APP, PRF_SRC_TASK(AI), ai_change_ccc_ind, 1);
                ind->conidx = conidx;
                ind->ai_type = SVC_AI_CUSTOMIZE;
                ind->ntf_ind_flag  = (voice_env->ntfIndEnableFlag[conidx] & (1<<2)) >> 2;
                ke_msg_send(ind);

            }
        }
        else if (hdl == (voice_env->shdl + CUS_VOICE_IDX_CMD_RX_VAL)) {
            ai_event_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_CMD_RECEIVED_IND,
                                            TASK_APP, PRF_SRC_TASK(AI), ai_event_ind, dataLen);
            ind->conidx = conidx;
            ind->ai_type = SVC_AI_CUSTOMIZE;
            ind->data_len = dataLen;
            memcpy(ind->data, pData, dataLen);
            ke_msg_send(ind);
        }
        else if (hdl == (voice_env->shdl + CUS_VOICE_IDX_DATA_RX_VAL)) {
            ai_event_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_DATA_RECEIVED_IND,
                                            TASK_APP, PRF_SRC_TASK(AI), ai_event_ind, dataLen);
            ind->conidx = conidx;
            ind->ai_type = SVC_AI_CUSTOMIZE;
            ind->data_len = dataLen;
            memcpy(ind->data, pData, dataLen);
            ke_msg_send(ind);
        }
        else {
            status = PRF_APP_ERROR;
        }
    }

    //Send write response
    gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
}


/// Set of callbacks functions for communication with GATT as a GATT User Server
static const gatt_srv_cb_t customize_gatt_srv_cb = {
    .cb_event_sent      = customize_event_sent_cb,
    .cb_att_read_get    = customize_read_cb,
    .cb_att_event_get   = customize_event_get_cb,
    .cb_att_info_get    = customize_info_get_cb,
    .cb_att_val_set     = customize_set_cb,
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static int customize_data_send_handler(ke_msg_id_t const msgid,
                                                  struct ai_data_send_cfm *param,
                                                  ke_task_id_t const dest_id,
                                                  ke_task_id_t const src_id)
{
    uint16_t ret = 0;
    uint16_t handle_offset = 0xFFFF;
    PRF_ENV_T(ai) *customize_env = PRF_ENV_GET(AI, ai);
    enum gatt_evt_type evtType = param->gatt_event_type;

    if ((AI_CMD == param->data_type) && (customize_env->ntfIndEnableFlag[param->conidx] & 1))
    {
        handle_offset = CUS_VOICE_IDX_CMD_TX_VAL;
    }
    if ((AI_DATA == param->data_type) && (customize_env->ntfIndEnableFlag[param->conidx] & (1<<2)))
    {
        handle_offset = CUS_VOICE_IDX_DATA_TX_VAL;
    }

    if (0xFFFF != handle_offset)
    {
        co_buf_t* p_buf = NULL;
        prf_buf_alloc(&p_buf, param->data_len);

        uint8_t* p_data = co_buf_data(p_buf);
        memcpy(p_data, param->data, param->data_len);

        // Dummy parameter provided to GATT
        uint16_t dummy = 0;

        // Inform the GATT that notification must be sent
        ret = gatt_srv_event_send(param->conidx, customize_env->srv_user_lid, dummy, evtType,
                            customize_env->shdl + handle_offset, p_buf);

        // Release the buffer
        co_buf_release(p_buf);

        if(ret){
            TRACE(1, "%s[ERROR]customize send fail. err=%x!", __func__, ret);
        }
    }
    else
    {
        TRACE(1, "%s[ERROR]customize send fail. %d!", __func__, handle_offset);
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(customize) { /// AKA ancc_msg_handler_tab
    /// handlers for command from upper layer
    {AMA_DATA_DEND_CFM,       (ke_msg_func_t)customize_data_send_handler},
};

static void customize_task_init(struct ke_task_desc *task_desc, PRF_ENV_T(ai)*p_env)
{
    TRACE(1, "%s Entry.", __func__);

    task_desc->msg_handler_tab = customize_msg_handler_tab;
    task_desc->msg_cnt = ARRAY_LEN(customize_msg_handler_tab);
    task_desc->state   = &p_env->state;
    task_desc->idx_max = 1;
}

/**
 ****************************************************************************************
 * @brief Initialization of the VOICE module.
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
static uint16_t customize_voice_init(prf_data_t* p_env, uint16_t* p_start_hdl, uint8_t sec_lvl,
                                             uint8_t user_prio, const void* p_params, const void* p_cb)
{
    TRACE(0, "%s start_hdl:%d", __func__, *p_start_hdl);
    uint16_t status = GAP_ERR_NO_ERROR;

    /// allocate&init profile environment data
    PRF_ENV_T(ai) *sv_env = (PRF_ENV_T(ai) *)ke_malloc(sizeof(PRF_ENV_T(ai)), KE_MEM_PROFILE);
    memset((uint8_t *)sv_env, 0, sizeof(PRF_ENV_T(ai)));

    do
    {
        /// registor GATT server user
        status = gatt_user_srv_register(PREFERRED_BLE_MTU, user_prio,
                                        &customize_gatt_srv_cb, &(sv_env->srv_user_lid));

        /// check the GATT server user registation excution result
        if (GAP_ERR_NO_ERROR != status)
        {
            break;
        }

        status = gatt_db_svc_add(sv_env->srv_user_lid, sec_lvl,
                         CUS_SERVICE_UUID_128,
                         CUS_VOICE_IDX_NB, NULL, customize_att_db, CUS_VOICE_IDX_NB,
                         &sv_env->shdl);

        /// check the GATT database add execution result
        if (GAP_ERR_NO_ERROR == status)
        {
            *p_start_hdl = sv_env->shdl;
            /// initialize environment variable
            p_env->p_env = (prf_hdr_t *)sv_env;
            /// init descriptor
            customize_task_init(&(p_env->desc), sv_env);

            ai_add_svc_ind_t *ind = KE_MSG_ALLOC(PRF_AI_SVC_ADD_DONE_IND,TASK_APP,
                                                    PRF_SRC_TASK(AI), ai_add_svc_ind);

            ind->ai_type = SVC_AI_SMART;
            ind->start_hdl = sv_env->shdl;
            ind->att_num   = CUS_VOICE_IDX_NB;
            ke_msg_send(ind);
        }

        /* Put HRS in Idle state */
        ke_state_set(sv_env->state, AI_IDLE);
    }while(0);

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the VOICE module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static uint16_t customize_voice_destroy(prf_data_t* p_env, uint8_t reason)
{
    PRF_ENV_T(ai)* voice_env = (PRF_ENV_T(ai)*) p_env->p_env;

    TRACE(2,"%s env %p", __func__, p_env);
    // free profile environment variables
    p_env->p_env = NULL;
    ke_free(voice_env);

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
static void customize_voice_create(prf_data_t* p_env, uint8_t conidx, bool is_le_con)
{
    TRACE(3,"%s env %p conidx %d", __func__, p_env, conidx);
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
static void customize_voice_cleanup(prf_data_t* p_env, uint8_t conidx, uint16_t reason)
{
    TRACE(4,"%s env %p, conidx %d reason %d", __func__, p_env, conidx, reason);
    /* Nothing to do */
}

static void customize_voice_con_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param)
{
    TRACE(0, "%s", __func__);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// VOICE Task interface required by profile manager
const prf_task_cbs_t customize_itf =
{
    .cb_init        = customize_voice_init,
    .cb_destroy     = customize_voice_destroy,
    .cb_con_create  = customize_voice_create,
    .cb_con_cleanup = customize_voice_cleanup,
    .cb_con_upd     = customize_voice_con_upd,
};

const struct prf_task_cbs* customize_prf_itf_get(void)
{
    return &customize_itf;
}

#endif



