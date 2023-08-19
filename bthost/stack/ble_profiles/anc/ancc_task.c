/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
 * @addtogroup ANCCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_ANC_CLIENT)
#include "gatt.h"
#include "anc_common.h"
#include "prf_utils.h"
#include "prf_types.h"
#include "ke_mem.h"
#include "prf_dbg.h"
#include "ancc.h"
#include "ancc_task.h"

#ifdef ANCS_ENABLED
#include "ancs.h"
#include "ancs_task.h"
#endif

#define PRF_TAG         "[ANCC]"
#define HAVE_DEFERED_OP 1
#define NO_DEFERED_OP   0

#define SUBSCRIBE_ANC_SVC_CCCD_VAL      PRF_CLI_START_NTF
#define SUBSCRIBE_ANC_SVC_CCCD_LEN      2

static bool is_need_write_continue[BLE_CONNECTION_MAX];

/*
 * STRUCTURES
 ****************************************************************************************
 */
/// ANCS UUID
const uint8_t anc_svc_uuid[GATT_UUID_128_LEN] = {
    0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4,
    0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79,
};

/// State machine used to retrieve ANCS characteristics information
const prf_char128_def_t ancc_anc_char[ANCC_CHAR_MAX] = {
    /// Notification Source
    [ANCC_CHAR_NTF_SRC] = {
        ANC_CHAR_NS,
        PRF_ATT_REQ_PRES_MAND,
        PROP(N),
    },

    /// Control Point
    [ANCC_CHAR_CTRL_PT] = {
        ANC_CHAR_CP,
        PRF_ATT_REQ_PRES_OPT,
        PROP(WR),
    },

    /// Data Source
    [ANCC_CHAR_DATA_SRC] = {
        ANC_CHAR_DS,
        PRF_ATT_REQ_PRES_OPT,
        PROP(N),
    },
};

/// State machine used to retrieve ANCS characteristic descriptor information
const prf_desc_def_t ancc_anc_desc[ANCC_DESC_MAX] = {
    /// Notification Source Char. - Client Characteristic Configuration
    [ANCC_DESC_NTF_SRC_CL_CFG] = {
        GATT_DESC_CLIENT_CHAR_CFG,
        PRF_ATT_REQ_PRES_OPT,
        ANCC_CHAR_NTF_SRC,
    },
    /// Data Source Char. - Client Characteristic Configuration
    [ANCC_DESC_DATA_SRC_CL_CFG] = {
        GATT_DESC_CLIENT_CHAR_CFG,
        PRF_ATT_REQ_PRES_OPT,
        ANCC_CHAR_DATA_SRC,
    },
};

/*
 * LOCAL FUNCTIONS DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Handles reception of the @ref ANCC_ENABLE_REQ message.
 * @param[in] msgid Id of the message received.
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int _enable_req_handler(ke_msg_id_t const msgid,
                               struct ancc_enable_req *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref ANCC_WRITE_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int _write_cmd_handler(ke_msg_id_t const msgid,
                              gatt_cli_write_cmd_t *param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref ANCC_READ_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int _read_cmd_handler(ke_msg_id_t const msgid,
                             anc_cli_rd_cmd_t *param,
                             ke_task_id_t const dest_id,
                             ke_task_id_t const src_id);

/**
 * @brief Handles reception of the @ref GATT_CMP_EVT message.
 * 
 * @param msgid         msgid Id of the message received.
 * @param param         param Pointer to the parameters of the message.
 * @param dest_id       dest_id ID of the receiving task instance.
 * @param src_id        src_id ID of the sending task instance.
 * @return int          If the message was consumed or not.
 */
static int _gatt_cmp_evt_handler(ke_msg_id_t const msgid,
                                 gatt_proc_cmp_evt_t *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);

/**
 * @brief This function is called when GATT client user discovery procedure is over.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param status        Status of the procedure (@see enum hl_err)
 */
static void _cb_discover_cmp(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status);

/**
 * @brief This function is called when GATT client user read procedure is over.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param status        Status of the procedure (@see enum hl_err)
 */
static void _cb_read_cmp(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status);

/**
 * @brief This function is called when GATT client user write procedure is over.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param status        Status of the procedure (@see enum hl_err)
 */
static void _cb_write_cmp(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status);

/**
 * @brief This function is called when GATT client user has initiated a write procedure.
 *
 *        @see gatt_cli_att_val_get_cfm shall be called to provide attribute value.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param token         Procedure token that must be returned in confirmation function
 * @param dummy         Dummy parameter provided by upper layer for command execution - 0x0000 else.
 * @param hdl           Attribute handle
 * @param offset        Value offset
 * @param max_length    Maximum value length to return
 */
static void _cb_att_val_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t dummy,
                            uint16_t hdl, uint16_t offset, uint16_t max_length);

/**
 * @brief This function is called when a full service has been found during a discovery procedure.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param hdl           First handle value of following list
 * @param disc_info     Discovery information (@see enum gatt_svc_disc_info)
 * @param nb_att        Number of attributes
 * @param p_atts        Pointer to attribute information present in a service
 */
static void _cb_svc(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t hdl, uint8_t disc_info,
                    uint8_t nb_att, const gatt_svc_att_t* p_atts);

/**
 * @brief This function is called when a service has been found during a discovery procedure.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param start_hdl     Service start handle
 * @param end_hdl       Service end handle
 * @param uuid_type     UUID Type (@see enum gatt_uuid_type)
 * @param p_uuid        Pointer to service UUID (LSB first)
 */
static void _cb_svc_info(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t start_hdl, uint16_t end_hdl,
                         uint8_t uuid_type, const uint8_t* p_uuid);

/**
 * @brief This function is called when an include service has been found during a discovery procedure.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param inc_svc_hdl   Include service attribute handle
 * @param start_hdl     Service start handle
 * @param end_hdl       Service end handle
 * @param uuid_type     UUID Type (@see enum gatt_uuid_type)
 * @param p_uuid        Pointer to service UUID (LSB first)
 */
static void _cb_inc_svc(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t inc_svc_hdl,
                        uint16_t start_hdl, uint16_t end_hdl, uint8_t uuid_type, const uint8_t* p_uuid);

/**
 * @brief This function is called when a characteristic has been found during a discovery procedure.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param char_hdl      Characteristic attribute handle
 * @param val_hdl       Value handle
 * @param prop          Characteristic properties (@see enum gatt_att_info_bf - bits [0-7])
 * @param uuid_type     UUID Type (@see enum gatt_uuid_type)
 * @param p_uuid        Pointer to characteristic value UUID (LSB first)
 */
static void _cb_char(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t char_hdl, uint16_t val_hdl,
                     uint8_t prop, uint8_t uuid_type, const uint8_t* p_uuid);

/**
 * @brief This function is called when a descriptor has been found during a discovery procedure.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param desc_hdl      Characteristic descriptor attribute handle
 * @param uuid_type     UUID Type (@see enum gatt_uuid_type)
 * @param p_uuid        Pointer to attribute UUID (LSB first)
 */
static void _cb_desc(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t desc_hdl,
                     uint8_t uuid_type, const uint8_t* p_uuid);

/**
 * @brief This function is called during a read procedure when attribute value is retrieved
 *        form peer device.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param dummy         Dummy parameter provided by upper layer for command execution
 * @param hdl           Attribute handle
 * @param offset        Value offset
 * @param p_data        Pointer to buffer that contains attribute value starting from offset
 */
static void _cb_att_val(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t hdl, uint16_t offset,
                        co_buf_t* p_data);

/**
 * @brief This function is called when a notification or an indication is received onto
 *        register handle range (@see gatt_cli_event_register).
 * 
 *        @see gatt_cli_val_event_cfm must be called to confirm event reception.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param token         Procedure token that must be returned in confirmation function
 * @param evt_type      Event type triggered (@see enum gatt_evt_type)
 * @param complete      True if event value if complete value has been received
 *                      False if data received is equals to maximum attribute protocol value.
 *                      In such case GATT Client User should perform a read procedure.
 * @param hdl           Attribute handle
 * @param p_data        Pointer to buffer that contains attribute value
 */
static void _cb_att_val_evt(uint8_t conidx, uint8_t user_lid, uint16_t token, uint8_t evt_type, bool complete, 
                            uint16_t hdl, co_buf_t* p_data);

/**
 * @brief Event triggered when a service change has been received or if an attribute
 *        transaction triggers an out of sync error.
 * 
 * @param conidx        Connection index
 * @param user_lid      GATT user local identifier
 * @param out_of_sync   True if an out of sync error has been received
 * @param start_hdl     Service start handle
 * @param end_hdl       Service end handle
 */
static void _cb_svc_changed(uint8_t conidx, uint8_t user_lid, bool out_of_sync, uint16_t start_hdl, uint16_t end_hdl);

/*
 * PRIVATE VARIABLE DECLARATIONS
 ****************************************************************************************
 */
static const gatt_cli_cb_t ancc_gatt_cli_cbs = {
    .cb_discover_cmp    = _cb_discover_cmp,
    .cb_read_cmp        = _cb_read_cmp,
    .cb_write_cmp       = _cb_write_cmp,
    .cb_att_val_get     = _cb_att_val_get,
    .cb_svc             = _cb_svc,
    .cb_svc_info        = _cb_svc_info,
    .cb_inc_svc         = _cb_inc_svc,
    .cb_char            = _cb_char,
    .cb_desc            = _cb_desc,
    .cb_att_val         = _cb_att_val,
    .cb_att_val_evt     = _cb_att_val_evt,
    .cb_svc_changed     = _cb_svc_changed,
};

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(ancc) { /// AKA ancc_msg_handler_tab
    /// handlers for command from upper layer
    {ANCC_ENABLE_REQ,       (ke_msg_func_t)_enable_req_handler},
    {ANCC_WRITE_CMD,        (ke_msg_func_t)_write_cmd_handler},
    {ANCC_READ_CMD,         (ke_msg_func_t)_read_cmd_handler},

    /// handlers for command from GATT layer
    {GATT_CMP_EVT,          (ke_msg_func_t)_gatt_cmp_evt_handler},
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
bool ancc_is_need_wirte_continue(uint8_t conidx)
{
    return is_need_write_continue[conidx];
}

void ancc_set_wirte_state(uint8_t conidx, bool state)
{
    is_need_write_continue[conidx] = state;
}

static int _enable_req_handler(ke_msg_id_t const msgid,
                               struct ancc_enable_req *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    PRF_ENV_T(ancc) *ancc_env = PRF_ENV_GET(ANCC, ancc);
    // Get connection index
    uint8_t conidx = param->conidx;

    ancc_set_wirte_state(conidx, false);
    LOG_I("%s Entry. conidx=%d", __func__, conidx);

    if (ancc_env->env[conidx] == NULL)
    {
        LOG_I("%s passed state check", __func__);
        // allocate environment variable for task instance
        ancc_env->env[conidx] = (struct ancc_cnx_env *)ke_malloc(sizeof(struct ancc_cnx_env), KE_MEM_PROFILE);
        ASSERT(ancc_env->env[conidx], "%s alloc error", __func__);
        memset(ancc_env->env[conidx], 0, sizeof(struct ancc_cnx_env));

        ancc_env->env[conidx]->last_char_code = ANCC_ENABLE_OP_CODE;

        /// Start discovering
        gatt_cli_discover_svc_cmd_t *cmd = KE_MSG_ALLOC(GATT_CMD,
                                                        TASK_GATT,
                                                        dest_id,
                                                        gatt_cli_discover_svc_cmd);

        cmd->cmd_code = GATT_CLI_DISCOVER_SVC;
        cmd->dummy = 0;
        cmd->user_lid = ancc_env->user_lid;
        cmd->conidx = conidx;
        cmd->disc_type = GATT_DISCOVER_SVC_PRIMARY_BY_UUID;
        cmd->full = true;
        cmd->start_hdl = GATT_MIN_HDL;
        cmd->end_hdl = GATT_MAX_HDL;
        cmd->uuid_type = GATT_UUID_128;
        memcpy(cmd->uuid, anc_svc_uuid, GATT_UUID_128_LEN);

        /// send msg to GATT layer
        ke_msg_send(cmd);

        // Configure the environment for a discovery procedure
        ancc_env->env[conidx]->last_req = GATT_DECL_PRIMARY_SERVICE;
    }
    else
    {
        LOG_W("ancc_env->env[conidx] != NULL, pls check");
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        LOG_W("Enable request status error:%d", status);
    }

    return (KE_MSG_CONSUMED);
}

static int _write_cmd_handler(ke_msg_id_t const msgid,
                              gatt_cli_write_cmd_t *param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id)
{
    LOG_I("%s hdl=0x%4.4x, conidx=%d, len=%d",
          __func__, param->hdl, param->conidx, param->value_length);

    uint8_t conidx = KE_IDX_GET(dest_id);

    // Get the address of the environment
    PRF_ENV_T(ancc) *ancc_env = PRF_ENV_GET(ANCC, ancc);

    if (ancc_env != NULL)
    { /// ask GATT layer to read server ATT value
        gatt_cli_write_cmd_t *cmd = KE_MSG_ALLOC_DYN(GATT_CMD,
                                                     TASK_GATT,
                                                     dest_id,
                                                     gatt_cli_write_cmd,
                                                     param->value_length);

        cmd->cmd_code = GATT_CLI_WRITE;
        cmd->dummy = HAVE_DEFERED_OP;
        cmd->user_lid = ancc_env->user_lid;
        cmd->conidx = param->conidx;
        cmd->write_type = param->write_type;
        cmd->hdl = param->hdl;
        cmd->offset = param->offset;
        cmd->value_length = param->value_length;
        memcpy((uint8_t*)cmd->value, (uint8_t*)param->value, param->value_length);

        ancc_env->wrInfo[conidx].conidx = param->conidx;
        ancc_env->wrInfo[conidx].user_lid = param->user_lid;
        ancc_env->wrInfo[conidx].token = param->dummy;
        ancc_env->wrInfo[conidx].status = GAP_ERR_NO_ERROR;

        // Send the message
        ke_msg_send(cmd);
    }
    else
    {
        // ancc_send_no_conn_cmp_evt(dest_id, src_id, param->handle,
        // ANCC_WRITE_CL_CFG_OP_CODE);
        ASSERT(0, "%s implement me", __func__);
    }

    return KE_MSG_CONSUMED;
}


static int _read_cmd_handler(ke_msg_id_t const msgid,
                             anc_cli_rd_cmd_t *param,
                             ke_task_id_t const dest_id,
                             ke_task_id_t const src_id)
{
    LOG_I("%s Entry. hdl=0x%4.4x", __func__, param->hdl);

    // Get the address of the environment
    PRF_ENV_T(ancc) *ancc_env = PRF_ENV_GET(ANCC, ancc);

    if (ancc_env != NULL)
    {
        // Send the read request
        gatt_cli_read_cmd_t *cmd = KE_MSG_ALLOC(GATT_CMD,
                                                TASK_GATT,
                                                dest_id,
                                                gatt_cli_read_cmd);

        cmd->cmd_code = GATT_CLI_READ;
        cmd->dummy = 0;
        cmd->user_lid = ancc_env->user_lid;
        cmd->conidx = param->conidx;
        cmd->hdl = param->hdl;
        cmd->offset = 0;
        cmd->length = 0;

        ke_msg_send(cmd);
    }
    else
    {
        ASSERT(0, "%s implement me", __func__);
    }

    return KE_MSG_CONSUMED;
}

static int _gatt_cmp_evt_handler(ke_msg_id_t const msgid,
                                 gatt_proc_cmp_evt_t *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    LOG_I("%s", __func__);

    return KE_MSG_CONSUMED;
}

static void _cb_discover_cmp(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    LOG_I("%s status=%d, conidx=%d", __func__, status, conidx);;

    // Get the address of the environment
    PRF_ENV_T(ancc) *ancc_env = PRF_ENV_GET(ANCC, ancc);
    ASSERT(ancc_env->env[conidx], "%s %d", __func__, __LINE__);

    uint8_t ret = status;

    if ((ATT_ERR_ATTRIBUTE_NOT_FOUND == status) ||
        (GAP_ERR_NO_ERROR == status))
    {
        // Discovery
        // check characteristic validity
        if (ancc_env->env[conidx]->nb_svc == 1)
        {
            ret = prf_check_svc128_validity(ANCC_CHAR_MAX, ancc_env->env[conidx]->anc.chars, ancc_anc_char,
                                            ANCC_DESC_MAX, ancc_env->env[conidx]->anc.descs, ancc_anc_desc);
        }
        // too much services
        else if (ancc_env->env[conidx]->nb_svc > 1)
        {
            ret = PRF_ERR_MULTIPLE_SVC;
            LOG_W("Multiple service error");
        }
        // no services found
        else
        {
            ret = PRF_ERR_STOP_DISC_CHAR_MISSING;
            LOG_W("Character missing");
        }
    }

    if (GAP_ERR_NO_ERROR == ret)
    {
        /// registor ANCC event handle for GATT layer
        gatt_cli_event_register(conidx,
                                ancc_env->user_lid,
                                ancc_env->env[conidx]->anc.svc.shdl,
                                ancc_env->env[conidx]->anc.svc.ehdl);

        /// Subscribe Data Source to NP
        co_buf_t* pBuf = NULL;
        prf_buf_alloc(&pBuf, SUBSCRIBE_ANC_SVC_CCCD_LEN);

        /// Dummy parameter provided to GATT
        uint16_t dummy = ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl;
        /// CCCD value for subscribe notification source
        uint16_t cccdVal = SUBSCRIBE_ANC_SVC_CCCD_VAL;

        uint8_t* pData = co_buf_data(pBuf);
        memcpy(pData, (uint8_t *)&cccdVal, SUBSCRIBE_ANC_SVC_CCCD_LEN);

        LOG_I("Subscribe Data Source, hdl:%d, value: 0x%02x 0x%02x",
              ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl,
              pData[0], pData[1]);
        gatt_cli_write(conidx,
                       ancc_env->user_lid,
                       dummy,
                       GATT_WRITE,
                       ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl,
                       0,
                       pBuf);

        /// Release the buffer
        co_buf_release(pBuf);
    }

    LOG_I("NSVal=0x%4.4x, NSCfg=0x%4.4x",
          ancc_env->env[conidx]->anc.chars[ANCC_CHAR_NTF_SRC].val_hdl,
          ancc_env->env[conidx]->anc.descs[ANCC_DESC_NTF_SRC_CL_CFG].desc_hdl);
    LOG_I("DSVal=0x%4.4x, DSCfg=0x%4.4x",
          ancc_env->env[conidx]->anc.chars[ANCC_CHAR_DATA_SRC].val_hdl,
          ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl);
    LOG_I("CPVal=0x%4.4x",
          ancc_env->env[conidx]->anc.chars[ANCC_CHAR_CTRL_PT].val_hdl);

#ifdef ANCS_ENABLED
    ancs_proxy_set_ready_flag(conidx,
                              0,
                              ancc_env->env[conidx]->anc.chars[ANCC_CHAR_NTF_SRC].val_hdl,
                              ancc_env->env[conidx]->anc.descs[ANCC_DESC_NTF_SRC_CL_CFG].desc_hdl,
                              0,
                              ancc_env->env[conidx]->anc.chars[ANCC_CHAR_DATA_SRC].val_hdl,
                              ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl,
                              0,
                              ancc_env->env[conidx]->anc.chars[ANCC_CHAR_CTRL_PT].val_hdl);
#endif
}

static void _cb_read_cmp(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    LOG_I("%s", __func__);
}

static void _cb_write_cmp(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    LOG_I("%s", __func__);

    /// Get the address of the environment
    PRF_ENV_T(ancc) *ancc_env = PRF_ENV_GET(ANCC, ancc);
    ASSERT(ancc_env->env[conidx], "%s %d", __func__, __LINE__);

    if (HAVE_DEFERED_OP == dummy || status == ATT_ERR_INSUFF_AUTHEN)
    {
        /// inform APP layer that client write process is complished
        anc_cli_wr_cmp_t *cmp = KE_MSG_ALLOC(ANCC_WRITE_CMP,
                                             KE_BUILD_ID(TASK_APP, conidx),
                                             KE_BUILD_ID(prf_get_task_from_id(TASK_ID_ANCC), conidx),
                                             anc_cli_wr_cmp);

        cmp->conidx = conidx;
        cmp->user_lid = user_lid;
        cmp->status = status;

        ke_msg_send(cmp);

        if(status == ATT_ERR_INSUFF_AUTHEN)
        {
            ancc_set_wirte_state(conidx, true);
            return;
        }
    }
    else if (dummy == ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl)
    {
        /// Subscribe Notification Source to NP
        co_buf_t* pBuf = NULL;
        prf_buf_alloc(&pBuf, SUBSCRIBE_ANC_SVC_CCCD_LEN);

        /// Dummy parameter provided to GATT
        uint16_t dummy = ancc_env->env[conidx]->anc.descs[ANCC_DESC_NTF_SRC_CL_CFG].desc_hdl;
        /// CCCD value for subscribe notification source
        uint16_t cccdVal = SUBSCRIBE_ANC_SVC_CCCD_VAL;

        uint8_t* pData = co_buf_data(pBuf);
        memcpy(pData, (uint8_t *)&cccdVal, SUBSCRIBE_ANC_SVC_CCCD_LEN);

        LOG_I("Subscribe Notification Source, hdl:%d, value: 0x%02x 0x%02x",
              ancc_env->env[conidx]->anc.descs[ANCC_DESC_NTF_SRC_CL_CFG].desc_hdl,
              pData[0], pData[1]);
        gatt_cli_write(conidx,
                       ancc_env->user_lid,
                       dummy,
                       GATT_WRITE,
                       ancc_env->env[conidx]->anc.descs[ANCC_DESC_NTF_SRC_CL_CFG].desc_hdl,
                       0,
                       pBuf);
        co_buf_release(pBuf);
    }
}

void ancc_write_continue(uint8_t conidx)
{
    /// Get the address of the environment
    PRF_ENV_T(ancc) *ancc_env = PRF_ENV_GET(ANCC, ancc);

    /// Subscribe Data Source to NP
    co_buf_t* pBuf = NULL;
    prf_buf_alloc(&pBuf, SUBSCRIBE_ANC_SVC_CCCD_LEN);

    /// Dummy parameter provided to GATT
    uint16_t dummy = ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl;
    /// CCCD value for subscribe notification source
    uint16_t cccdVal = SUBSCRIBE_ANC_SVC_CCCD_VAL;

    uint8_t* pData = co_buf_data(pBuf);
    memcpy(pData, (uint8_t *)&cccdVal, SUBSCRIBE_ANC_SVC_CCCD_LEN);

    LOG_I("Subscribe Data Source, hdl:%d, value: 0x%02x 0x%02x",
          ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl,
          pData[0], pData[1]);
    gatt_cli_write(conidx,
                   ancc_env->user_lid,
                   dummy,
                   GATT_WRITE,
                   ancc_env->env[conidx]->anc.descs[ANCC_DESC_DATA_SRC_CL_CFG].desc_hdl,
                   0,
                   pBuf);

    /// Release the buffer
    co_buf_release(pBuf);
    ancc_set_wirte_state(conidx, false);
}

static void _cb_att_val_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t dummy,
                            uint16_t hdl, uint16_t offset, uint16_t max_length)
{
    LOG_I("%s", __func__);
}

static void _cb_svc(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t hdl, uint8_t disc_info,
                    uint8_t nb_att, const gatt_svc_att_t* p_atts)
{
    LOG_I("%s incoming hdl=0x%4.4x, nb_att=%d",
          __func__, hdl, nb_att);

    LOG_I("charac[0] info: prop=%d, handle=0x%4.4x, att_type=%d",
          p_atts[0].info.charac.prop, p_atts[0].info.charac.val_hdl, p_atts[0].att_type);
    LOG_I("svc[0] info: att_type=%d, svc.start_hdl=0x%4.4x, svc.end_hdl=0x%4.4x",
          p_atts[0].att_type, p_atts[0].info.svc.start_hdl, p_atts[0].info.svc.end_hdl);

    PRF_ENV_T(ancc) *ancc_env = PRF_ENV_GET(ANCC, ancc);
    ASSERT_INFO(ancc_env, 0, 0);
    ASSERT_INFO(ancc_env->env[conidx], 0, 0);
    ancc_env->shdl = hdl;

    if (ancc_env->env[conidx]->nb_svc == 0)
    {
        LOG_I("retrieving characteristics and descriptors.");
        for (uint8_t i = 0; i < nb_att; i++)
        {
            LOG_I("att_type:%d, uuid_type:%d, uuid:", p_atts[i].att_type, p_atts[i].uuid_type);
            DUMP8("0x%02x ", p_atts[i].uuid, 16);
            if ((GATT_ATT_PRIMARY_SVC == p_atts[i].att_type) || //!< 1
                (GATT_ATT_SECONDARY_SVC == p_atts[i].att_type) || //!< 2
                (GATT_ATT_INCL_SVC == p_atts[i].att_type)) //!< 3
            {
                LOG_I("start_hdl:%d, end_hdl:%d", p_atts[i].info.svc.start_hdl, p_atts[i].info.svc.end_hdl);
            }
            else
            {
                LOG_I("val_hdl:%d, prop:%d", p_atts[i].info.charac.val_hdl, p_atts[i].info.charac.prop);
            }
        }

        /// Retrieve ANC characteristics and descriptors
        prf_extract_svc128_info(ancc_env->shdl, nb_att, p_atts,
                                ANCC_CHAR_MAX, ancc_anc_char, ancc_env->env[conidx]->anc.chars,
                                ANCC_DESC_MAX, ancc_anc_desc, ancc_env->env[conidx]->anc.descs);

        for (uint8_t j = 0; j < ANCC_CHAR_MAX; j++)
        {
            LOG_I("[characteristic] val_hdl:%d, prop:%d", ancc_env->env[conidx]->anc.chars[j].val_hdl, ancc_env->env[conidx]->anc.chars[j].prop);
        }

        for (uint8_t k = 0; k < ANCC_DESC_MAX; k++)
        {
            LOG_I("[descriptor] val_hdl:%d", ancc_env->env[conidx]->anc.descs[k].desc_hdl);
        }

        // Even if we get multiple responses we only store 1 range
        ancc_env->env[conidx]->anc.svc.shdl = p_atts[0].info.svc.start_hdl;
        ancc_env->env[conidx]->anc.svc.ehdl = p_atts[0].info.svc.end_hdl;
    }

    ancc_env->env[conidx]->nb_svc++;
}

static void _cb_svc_info(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t start_hdl, uint16_t end_hdl,
                        uint8_t uuid_type, const uint8_t* p_uuid)
{
    LOG_I("%s", __func__);
}

static void _cb_inc_svc(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t inc_svc_hdl,
                        uint16_t start_hdl, uint16_t end_hdl, uint8_t uuid_type, const uint8_t* p_uuid)
{
    LOG_I("%s", __func__);
}

static void _cb_char(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t char_hdl, uint16_t val_hdl,
                    uint8_t prop, uint8_t uuid_type, const uint8_t* p_uuid)
{
    LOG_I("%s", __func__);
}

static void _cb_desc(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t desc_hdl,
                    uint8_t uuid_type, const uint8_t* p_uuid)
{
    LOG_I("%s", __func__);
}

static void _cb_att_val(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t hdl, uint16_t offset,
                        co_buf_t* p_data)
{
    uint8_t *p_value = co_buf_data(p_data);
    LOG_I("%s hdl:%d, received dataLen:%d, data:", __func__, hdl, p_data->data_len);
    DUMP8("0x%02x ", p_value, p_data->data_len);
    /// report received data to APP layer
    anc_cli_att_val_t *val = KE_MSG_ALLOC_DYN(ANCC_ATT_VAL,
                                              KE_BUILD_ID(TASK_APP, conidx),
                                              KE_BUILD_ID(prf_get_task_from_id(TASK_ID_ANCC), conidx),
                                              anc_cli_att_val,
                                              p_data->data_len);
    val->conidx = conidx;
    val->user_lid = user_lid;
    val->dummy = dummy;
    val->hdl = hdl;
    val->offset = offset;
    val->value_length = p_data->data_len;
    memcpy(val->value, p_value, p_data->data_len);

    ke_msg_send(val);
}

static void _cb_att_val_evt(uint8_t conidx, uint8_t user_lid, uint16_t token, uint8_t evt_type, bool complete, 
                            uint16_t hdl, co_buf_t* p_data)
{
    LOG_I("%s evt_type:%d", __func__, evt_type);
    ASSERT(complete, "%s %d, event not complete", __func__, __LINE__);

    /// inform APP layer that client received an indication
    anc_cli_ind_evt_t *cmd =
        KE_MSG_ALLOC_DYN(ANCC_IND_EVT,
                         KE_BUILD_ID(TASK_APP, conidx),
                         KE_BUILD_ID(prf_get_task_from_id(TASK_ID_ANCC), conidx),
                         anc_cli_ind_evt,
                         p_data->data_len);

    cmd->conidx = conidx;
    cmd->hdl = hdl;
    cmd->value_length = p_data->data_len;
    memcpy(cmd->value, co_buf_data(p_data), p_data->data_len);

    ke_msg_send(cmd);

    // confirm event reception
    gatt_cli_att_event_cfm(conidx, user_lid, token);
}

static void _cb_svc_changed(uint8_t conidx, uint8_t user_lid, bool out_of_sync, uint16_t start_hdl, uint16_t end_hdl)
{
    LOG_I("%s", __func__);
}

void ancc_task_init(struct ke_task_desc *task_desc, void *p_env)
{
    LOG_I("%s Entry.", __func__);

    /// Get the address of the environment
    PRF_ENV_T(ancc) *ancc_env = (PRF_ENV_T(ancc) *)p_env;

    task_desc->msg_handler_tab = ancc_msg_handler_tab;
    task_desc->msg_cnt = ARRAY_LEN(ancc_msg_handler_tab);
    task_desc->state = ancc_env->state;
    task_desc->idx_max = ANCC_IDX_MAX;
}

const gatt_cli_cb_t* ancc_task_get_cli_cbs(void)
{
    return (&ancc_gatt_cli_cbs);
}

#endif //(BLE_ANC_CLIENT)

/// @} ANCCTASK
