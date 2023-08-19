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

#if BLE_DUAL_MIC_REC_VOICE
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
#define rec_service_uuid_128_content                                                                       \
    {                                                                                                  \
        0x00, 0x00, 0x82, 0x6f, 0x63, 0x2e, 0x74, 0x6e, 0x69, 0x6f, 0x70, 0x6c, 0x65, 0x63, 0x78, 0x65 \
    }
#define rec_cmd_tx_char_val_uuid_128_content                                                               \
    {                                                                                                  \
        0x01, 0x00, 0x82, 0x6f, 0x63, 0x2e, 0x74, 0x6e, 0x69, 0x6f, 0x70, 0x6c, 0x65, 0x63, 0x78, 0x65 \
    }
#define rec_cmd_rx_char_val_uuid_128_content                                                               \
    {                                                                                                  \
        0x02, 0x00, 0x82, 0x6f, 0x63, 0x2e, 0x74, 0x6e, 0x69, 0x6f, 0x70, 0x6c, 0x65, 0x63, 0x78, 0x65 \
    }
#define rec_data_tx_char_val_uuid_128_content                                                              \
    {                                                                                                  \
        0x03, 0x00, 0x82, 0x6f, 0x63, 0x2e, 0x74, 0x6e, 0x69, 0x6f, 0x70, 0x6c, 0x65, 0x63, 0x78, 0x65 \
    }
#define rec_data_rx_char_val_uuid_128_content                                                              \
    {                                                                                                  \
        0x04, 0x00, 0x82, 0x6f, 0x63, 0x2e, 0x74, 0x6e, 0x69, 0x6f, 0x70, 0x6c, 0x65, 0x63, 0x78, 0x65 \
    }

///Attributes State Machine
enum rec_voice_att{
    REC_VOICE_IDX_SVC,

    REC_VOICE_IDX_CMD_TX_CHAR,
    REC_VOICE_IDX_CMD_TX_VAL,
    REC_VOICE_IDX_CMD_TX_NTF_CFG,

    REC_VOICE_IDX_CMD_RX_CHAR,
    REC_VOICE_IDX_CMD_RX_VAL,

    REC_VOICE_IDX_DATA_TX_CHAR,
    REC_VOICE_IDX_DATA_TX_VAL,
    REC_VOICE_IDX_DATA_TX_NTF_CFG,

    REC_VOICE_IDX_DATA_RX_CHAR,
    REC_VOICE_IDX_DATA_RX_VAL,

    REC_VOICE_IDX_NB,
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static const uint8_t REC_VOICE_SERVICE_UUID_128[GATT_UUID_128_LEN] = rec_service_uuid_128_content;

/// Full SMARTVOICE SERVER Database Description - Used to add attributes into the database
static const struct gatt_att_desc record_att_db[REC_VOICE_IDX_NB] = {
    // Service Declaration
    [REC_VOICE_IDX_SVC]             = {UUID_16_TO_ARRAY(GATT_DECL_PRIMARY_SERVICE), PROP(RD), 0},

    // Command TX Characteristic Declaration
    [REC_VOICE_IDX_CMD_TX_CHAR]     = {UUID_16_TO_ARRAY(GATT_DECL_CHARACTERISTIC), PROP(RD), 0},
    // Command TX Characteristic Value
    [REC_VOICE_IDX_CMD_TX_VAL]      = {rec_cmd_tx_char_val_uuid_128_content,PROP(N) | PROP(I) | PROP(RD)
                                       | SEC_LVL(RP, AUTH) | ATT_UUID(128), AI_MAX_LEN,},
    // Command TX Characteristic - Client Characteristic Configuration Descriptor
    [REC_VOICE_IDX_CMD_TX_NTF_CFG]  = {UUID_16_TO_ARRAY(GATT_DESC_CLIENT_CHAR_CFG), PROP(RD)
                                       | PROP(WR) | SEC_LVL(NIP, AUTH), 0},

    // Command RX Characteristic Declaration
    [REC_VOICE_IDX_CMD_RX_CHAR]     = {UUID_16_TO_ARRAY(GATT_DECL_CHARACTERISTIC), PROP(RD), 0},
    // Command RX Characteristic Value
    [REC_VOICE_IDX_CMD_RX_VAL]      = {rec_cmd_rx_char_val_uuid_128_content,PROP(WR) | PROP(WC)
                                       | SEC_LVL(WP, AUTH) | ATT_UUID(128), AI_MAX_LEN},

    // Data TX Characteristic Declaration
    [REC_VOICE_IDX_DATA_TX_CHAR]    = {UUID_16_TO_ARRAY(GATT_DECL_CHARACTERISTIC), PROP(RD), 0},
    // Data TX Characteristic Value
    [REC_VOICE_IDX_DATA_TX_VAL]     = {rec_data_tx_char_val_uuid_128_content,PROP(N) | PROP(I) | PROP(RD)
                                       | SEC_LVL(RP, AUTH) | ATT_UUID(128), AI_MAX_LEN},
    // Data TX Characteristic - Client Characteristic Configuration Descriptor
    [REC_VOICE_IDX_DATA_TX_NTF_CFG] = {UUID_16_TO_ARRAY(GATT_DESC_CLIENT_CHAR_CFG),PROP(RD) | PROP(WR)
                                       | SEC_LVL(NIP, AUTH),0},

    // Data RX Characteristic Declaration
    [REC_VOICE_IDX_DATA_RX_CHAR]    = {UUID_16_TO_ARRAY(GATT_DECL_CHARACTERISTIC), PROP(RD), 0},
    // Data RX Characteristic Value
    [REC_VOICE_IDX_DATA_RX_VAL]     = {rec_data_rx_char_val_uuid_128_content,PROP(WR) | PROP(WC)
                                       | SEC_LVL(WP, AUTH) | ATT_UUID(128), AI_MAX_LEN,},
};

/**
 * @brief This function is called when GATT server user has initiated event send to peer
 *        device or if an error occurs.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param status        Status of the procedure (@see enum hl_err)
 */
static void recording_event_sent_cb(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    TRACE(0, "%s", __func__);
    ai_event_ind_t *ind = KE_MSG_ALLOC(PRF_AI_TX_DONE_IND,
                                 TASK_APP, PRF_SRC_TASK(AI), ai_event_ind);
    ind->conidx = conidx;
    ind->ai_type = SVC_AI_CUSTOMIZE;
    ind->data_len = 0;
    ke_msg_send(ind);
}

/**
 * @brief This function is called when peer want to read local attribute database value.
 *
 *        @see gatt_srv_att_read_get_cfm shall be called to provide attribute value
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param token         Procedure token that must be returned in confirmation function
 * @param hdl           Attribute handle
 * @param offset        Value offset
 * @param max_length    Maximum value length to return
 */
static void recording_att_read_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token,
                                                uint16_t hdl, uint16_t offset, uint16_t max_length)
{
    TRACE(0, "%s hdl:%d", __func__, hdl);
    PRF_ENV_T(ai) *voice_env = PRF_ENV_GET(AI, ai);
    uint16_t status = GAP_ERR_NO_ERROR;
    uint16_t notify_ccc = 0;
    uint16_t dataLen = sizeof(notify_ccc);
    co_buf_t *p_data = NULL;

    if (REC_VOICE_IDX_CMD_TX_NTF_CFG == hdl - voice_env->shdl)
    {
        notify_ccc = voice_env->ntfIndEnableFlag[conidx]
                         ? 1
                         : 0;

        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
    }
    else if (REC_VOICE_IDX_DATA_TX_NTF_CFG == hdl - voice_env->shdl)
    {
        notify_ccc = voice_env->ntfIndEnableFlag[conidx]
                         ? 1
                         : 0;

        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
    }
    else
    {
        dataLen = 0;
        status = ATT_ERR_REQUEST_NOT_SUPPORTED;
    }

    gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, dataLen, p_data);

    // Release the buffer
    co_buf_release(p_data);
}


/**
 * @brief This function is called when GATT server user has initiated event send procedure,
 *
 *        @see gatt_srv_att_event_get_cfm shall be called to provide attribute value
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param token         Procedure token that must be returned in confirmation function
 * @param dummy         Dummy parameter provided by upper layer for command execution.
 * @param hdl           Attribute handle
 * @param max_length    Maximum value length to return
 */
static void recording_att_event_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token,
                                                  uint16_t dummy, uint16_t hdl, uint16_t max_length)
{
    PRF_ENV_T(ai) *voice_env = PRF_ENV_GET(AI, ai);
    TRACE(0, "%s hdl:%d", __func__, (hdl - voice_env->shdl));
}

/**
 * @brief This function is called during a write procedure to get information about a
 *        specific attribute handle.
 *
 *        @see gatt_srv_att_info_get_cfm shall be called to provide attribute information
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param token         Procedure token that must be returned in confirmation function
 * @param hdl           Attribute handle
 */
static void recording_att_info_get_cb(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl)
{
    TRACE(0, "%s hdl:%d", __func__, hdl);
    uint16_t status = GAP_ERR_NO_ERROR;
    uint16_t length = 0;
    PRF_ENV_T(ai) *voice_env = PRF_ENV_GET(AI, ai);

    if ((REC_VOICE_IDX_CMD_TX_NTF_CFG == (hdl - voice_env->shdl)) ||
        (REC_VOICE_IDX_DATA_TX_NTF_CFG == (hdl - voice_env->shdl)))
    {
       length = 2;
    }
    else if ((REC_VOICE_IDX_CMD_RX_VAL == (hdl - voice_env->shdl)) ||
            (REC_VOICE_IDX_DATA_RX_VAL == (hdl - voice_env->shdl)))
    {
        length = 0;
    }
    else
    {
        length = 0;
        status = ATT_ERR_WRITE_NOT_PERMITTED;
    }

    /// Send the confirmation
    gatt_srv_att_info_get_cfm(conidx, user_lid, token, status, length);
}

/**
 * @brief This function is called during a write procedure to modify attribute handle.
 *
 *        @see gatt_srv_att_val_set_cfm shall be called to accept or reject attribute
 *        update.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param token         Procedure token that must be returned in confirmation function
 * @param hdl           Attribute handle
 * @param offset        Value
 * @param p_data        Pointer to buffer that contains data to write starting from offset
 */
static void recording_att_val_set_cb(uint8_t conidx, uint8_t user_lid, uint16_t token,
                            uint16_t hdl, uint16_t offset, co_buf_t *p_buf)
{
    uint16_t status = GAP_ERR_NO_ERROR;
    uint8_t *pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;
    TRACE(0, "%s Write req, hdl:%d, conidx:%d", __func__, hdl, conidx);

    /// Get the address of the environment
    PRF_ENV_T(ai) *voice_env = PRF_ENV_GET(AI, ai);
    uint16_t handle = hdl - voice_env->shdl;

    if (voice_env != NULL)
    {
        if (REC_VOICE_IDX_CMD_TX_NTF_CFG == handle)
        {
            uint16_t value = 0x0000;

            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));

            if (value == PRF_CLI_STOP_NTFIND)
            {
                voice_env->ntfIndEnableFlag[conidx] = false;
            }
            else if (value == PRF_CLI_START_NTF)
            {
                voice_env->ntfIndEnableFlag[conidx] = true;
            }
            else
            {
                status = PRF_APP_ERROR;
            }

            if (status == GAP_ERR_NO_ERROR)
            {
                //Inform APP of TX ccc change
                ai_change_ccc_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_CMD_CHANGGE_CCC_IND,
                                             TASK_APP, PRF_SRC_TASK(AI), ai_change_ccc_ind, 1);
                ind->conidx = conidx;
                ind->ai_type = SVC_AI_RECORDING; 
                ind->ntf_ind_flag  = voice_env->ntfIndEnableFlag[conidx];
                ke_msg_send(ind);
            }
        }
        else if (REC_VOICE_IDX_DATA_TX_NTF_CFG == handle)
        {
            uint16_t value = 0x0000;

            /// Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));

            if (value == PRF_CLI_STOP_NTFIND)
            {
                voice_env->ntfIndEnableFlag[conidx] = false;
            }
            else if (value == PRF_CLI_START_NTF)
            {
                voice_env->ntfIndEnableFlag[conidx] = true;
            }
            else
            {
                status = PRF_APP_ERROR;
            }

            if (status == GAP_ERR_NO_ERROR)
            {
                //Inform APP of TX ccc change
                ai_change_ccc_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_DATA_CHANGGE_CCC_IND,
                                             TASK_APP, PRF_SRC_TASK(AI), ai_change_ccc_ind, 1);
                ind->conidx = conidx;
                ind->ai_type = SVC_AI_RECORDING;
                ind->ntf_ind_flag  = voice_env->ntfIndEnableFlag[conidx];
                ke_msg_send(ind);
            }
        }
        else if (REC_VOICE_IDX_CMD_RX_VAL == handle)
        {
            ai_event_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_CMD_RECEIVED_IND,
                                            TASK_APP, PRF_SRC_TASK(AI), ai_event_ind, dataLen);
            ind->conidx = conidx;
            ind->ai_type = SVC_AI_RECORDING;
            ind->data_len = dataLen;
            memcpy(ind->data, pData, dataLen);
            ke_msg_send(ind);
        }
        else if (REC_VOICE_IDX_DATA_RX_VAL == handle)
        {
            ai_event_ind_t *ind = KE_MSG_ALLOC_DYN(PRF_AI_DATA_RECEIVED_IND,
                                            TASK_APP, PRF_SRC_TASK(AI), ai_event_ind, dataLen);
            ind->conidx = conidx;
            ind->ai_type = SVC_AI_RECORDING;
            ind->data_len = dataLen;
            memcpy(ind->data, pData, dataLen);
            ke_msg_send(ind);
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }
    /// Inform GATT about handling
    gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
}

/// Set of callbacks functions for communication with GATT as a GATT User Server
__STATIC const gatt_srv_cb_t _gatt_srv_cb = {
    .cb_event_sent      = recording_event_sent_cb,
    .cb_att_read_get    = recording_att_read_get_cb,
    .cb_att_event_get   = recording_att_event_get_cb,
    .cb_att_info_get    = recording_att_info_get_cb,
    .cb_att_val_set     = recording_att_val_set_cb,
};

static int recording_data_send_handler(ke_msg_id_t const msgid,
                                                  struct ai_data_send_cfm *param,
                                                  ke_task_id_t const dest_id,
                                                  ke_task_id_t const src_id)
{
    uint16_t ret = 0;
    uint16_t handle_offset = 0xFFFF;
    PRF_ENV_T(ai) *recording_env = PRF_ENV_GET(AI, ai);
    enum gatt_evt_type evtType = param->gatt_event_type;

    if (recording_env->ntfIndEnableFlag[param->conidx])
    {
        if(AI_CMD == param->data_type)
        {
            handle_offset = REC_VOICE_IDX_CMD_TX_VAL;
        }
        else if(AI_DATA == param->data_type)
        {
            handle_offset = REC_VOICE_IDX_DATA_TX_VAL;
        }
    }

    if (0xFFFF != handle_offset)
    {
        co_buf_t *p_buf = NULL;
        prf_buf_alloc(&p_buf, param->data_len);

        uint8_t *p_data = co_buf_data(p_buf);
        memcpy(p_data, param->data, param->data_len);

        ret =gatt_srv_event_send(param->conidx, recording_env->srv_user_lid, 0, evtType,
                            recording_env->shdl + handle_offset, p_buf);

        // Release the buffer
        co_buf_release(p_buf);
        
        if(ret){
            TRACE(1, "%s[ERROR]ama send fail. err=%x!", __func__, ret);
        }
    }
    else
    {
        TRACE(1, "%s[ERROR]ama send fail. 0x%x", __func__, handle_offset);
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/* Default State handlers definition. */
KE_MSG_HANDLER_TAB(recording)
{
    {CUSTOMIZE_DATA_DEND_CFM,      (ke_msg_func_t) recording_data_send_handler},
};

static void recording_task_init(struct ke_task_desc *task_desc, PRF_ENV_T(ai) *recording_env)
{
    task_desc->msg_handler_tab = recording_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(recording_msg_handler_tab);
    task_desc->state           = &(recording_env->state);
    task_desc->idx_max         = 1;
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
static uint16_t recording_init(prf_data_t *env, uint16_t *p_start_hdl, uint8_t sec_lvl,
                                      uint8_t user_prio, const void *params, const void *p_cb)
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
                                        &_gatt_srv_cb, &(sv_env->srv_user_lid));

        /// check the GATT server user registation excution result
        if (GAP_ERR_NO_ERROR != status)
        {
            break;
        }

        status = gatt_db_svc_add(sv_env->srv_user_lid, sec_lvl,
                                 REC_VOICE_SERVICE_UUID_128,
                                 REC_VOICE_IDX_NB, NULL, record_att_db, REC_VOICE_IDX_NB,
                                 &sv_env->shdl);

        /// check the GATT database add execution result
        if (GAP_ERR_NO_ERROR == status)
        {
            *p_start_hdl = sv_env->shdl;
            /// initialize environment variable
            env->p_env = (prf_hdr_t *)sv_env;
            /// init descriptor
            recording_task_init(&(env->desc), sv_env);

            ai_add_svc_ind_t *ind = KE_MSG_ALLOC(PRF_AI_SVC_ADD_DONE_IND,TASK_APP,
                                         PRF_SRC_TASK(AI), ai_add_svc_ind);

            ind->ai_type = SVC_AI_SMART;
            ind->start_hdl = sv_env->shdl;
            ind->att_num   = REC_VOICE_IDX_NB;
            ke_msg_send(ind);
        }

        // bes_ble_gap_ke_state_set(KE_BUILD_ID(p_env->prf_task, 0), 0);
    } while (0);

    return status;
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
static uint16_t recording_destroy(prf_data_t *p_env, uint8_t reason)
{
    PRF_ENV_T(ai)* voice_env = (PRF_ENV_T(ai)*) p_env->p_env;

    TRACE(0, "%s env %p", __func__, p_env);
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
static void recording_create(prf_data_t* p_env, uint8_t conidx, bool is_le_con)
{
    TRACE(0, "%s env %p conidx %d", __func__, p_env, conidx);
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
static void recording_cleanup(prf_data_t *p_env, uint8_t conidx, uint16_t reason)
{
    TRACE(0, "%s env %p, conidx %d reason %d", __func__, p_env, conidx, reason);
    /* Nothing to do */
}

static void recording_con_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param)
{
    TRACE(0, "%s", __func__);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// VOICE Task interface required by profile manager
const struct prf_task_cbs recording_itf =
{
    .cb_init        = recording_init,
    .cb_destroy     = recording_destroy,
    .cb_con_create  = recording_create,
    .cb_con_cleanup = recording_cleanup,
    .cb_con_upd     = recording_con_upd,
};

const struct prf_task_cbs* recording_prf_itf_get(void)
{
    return &recording_itf;
}

#endif
