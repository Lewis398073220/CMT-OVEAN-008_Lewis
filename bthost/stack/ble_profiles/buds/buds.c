/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
 * @addtogroup Buds
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "gapm_msg.h"

#if (BLE_BUDS)
#include <string.h>
#include "gap.h"
#include "buds.h"
#include "buds_task.h"
#include "prf_utils.h"
#include "gatt_msg.h"
#include "ke_mem.h"
#include "co_utils.h"
#include "gatt.h"
#include "prf.h"

POSSIBLY_UNUSED static buds_att_evt_callback_t buds_att_evt_cb = NULL;

/*
 * BUDS PROFILE ATTRIBUTES
 ****************************************************************************************
 */
#define buds_service_uuid_128_content           {0x22, 0x34, 0x56, 0x78, 0x90, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01 }
#define buds_tx_char_val_uuid_128_content       {0x22, 0x34, 0x56, 0x78, 0x91, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02 }
#define buds_rx_char_val_uuid_128_content       {0x22, 0x34, 0x56, 0x78, 0x92, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03 }

#define GATT_DECL_PRIMARY_SERVICE               { 0x00, 0x28 }
#define GATT_DECL_CHARACTERISTIC_UUID           { 0x03, 0x28 }
#define GATT_DESC_CLIENT_CHAR_CFG_UUID          { 0x02, 0x29 }

static const uint8_t BUDS_SERVICE_UUID_128[GATT_UUID_128_LEN] = buds_service_uuid_128_content;

/// Full BUDS Database Description - Used to add attributes into the database
const struct gatt_att_desc buds_att_db[BUDS_IDX_NB] =
{
    // Service Declaration
    [BUDS_IDX_SVC]        =   {GATT_DECL_PRIMARY_SERVICE, PROP(RD), 0},

    // TX Characteristic Declaration
    [BUDS_IDX_TX_CHAR]    =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // TX Characteristic Value
    [BUDS_IDX_TX_VAL]     =   {buds_tx_char_val_uuid_128_content, PROP(N) | PROP(RD) | ATT_UUID(128), BUDS_MAX_LEN},
    // TX Characteristic - Client Characteristic Configuration Descriptor
    [BUDS_IDX_TX_NTF_CFG] =   {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), PRF_SVC_DESC_CLI_CFG_LEN},

    // RX Characteristic Declaration
    [BUDS_IDX_RX_CHAR]    =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    // RX Characteristic Value
    [BUDS_IDX_RX_VAL]     =   {buds_rx_char_val_uuid_128_content, PROP(WR) | PROP(WC) | ATT_UUID(128), BUDS_MAX_LEN},
};

__STATIC void buds_gatt_cb_event_sent(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    // notification or indication has been sent out
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

__STATIC void buds_gatt_cb_att_read_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl, uint16_t offset,
                             uint16_t max_length)
{
    co_buf_t* p_data = NULL;
    uint16_t dataLen = 0;
    uint16_t status = GAP_ERR_NO_ERROR;

    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    TRACE(1, "read hdl %d", hdl);

    // Get the address of the environment
    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);

    if (hdl == (buds_env->shdl + BUDS_IDX_TX_NTF_CFG)) {
        uint16_t notify_ccc = buds_env->ntfIndEnableFlag[conidx];        
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

__STATIC void buds_gatt_cb_att_event_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t dummy, uint16_t hdl,
                              uint16_t max_length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

__STATIC void buds_gatt_cb_att_info_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl)
{
    uint16_t length = 0;
    uint16_t status = GAP_ERR_NO_ERROR;
    
    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);

    // check if it's a client configuration char
    if (hdl == buds_env->shdl + BUDS_IDX_TX_NTF_CFG)
    {
        // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    }
    else if (hdl == buds_env->shdl + BUDS_IDX_RX_VAL)
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

    TRACE(1, "func %s status %d hdl %d", __FUNCTION__, status, hdl);

    // Send the confirmation
    gatt_srv_att_info_get_cfm(conidx, user_lid, token, status, length);
}

__STATIC void buds_gatt_cb_att_set(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                          uint16_t offset, co_buf_t* p_buf)
{
    // Get the address of the environment
    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);

    uint8_t status = GAP_ERR_NO_ERROR;

    TRACE(4, "%s buds_env 0x%x write handle %d shdl %d", 
        __FUNCTION__, (uint32_t)buds_env, hdl, buds_env->shdl);

    uint8_t* pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;

    DUMP8("%02x ", pData, dataLen);

    if (buds_env != NULL)
    {
        // TX ccc
        if (hdl == (buds_env->shdl + BUDS_IDX_TX_NTF_CFG))
        {
            TRACE(0, "BUDS_IDX_TX_NTF_CFG 0x%x", BUDS_IDX_TX_NTF_CFG);
            uint16_t value = 0x0000;

            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));

            if (value <= (PRF_CLI_START_IND|PRF_CLI_START_NTF))
            {
                buds_env->ntfIndEnableFlag[conidx] = value;
            }
            else 
            {
                status = PRF_APP_ERROR;
            }
        }
        // RX data
        else if (hdl == (buds_env->shdl + BUDS_IDX_RX_VAL))
        {
            TRACE(0, "BUDS_IDX_RX_VAL 0x%x dataLen %d", BUDS_IDX_RX_VAL, dataLen);
            //DUMP8("0x%x ", pData, dataLen);
            // handle received data
            if (buds_att_evt_cb)
            {
                buds_att_evt_cb(BUDS_IDX_RX_VAL, pData, dataLen);
            }
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }

    // Inform GATT about handling
    gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
}

__STATIC void buds_gatt_cb_write_cmp(uint8_t con_lid, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    TRACE(1, "client write done: con_lid %d user_lid %d dummy 0x%02x status %d",
        con_lid, user_lid, dummy, status);

    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);
    if (dummy == buds_env->shdl + BUDS_IDX_TX_NTF_CFG)
    {
        //notify finished
        TRACE(1, "notify finished");
        if (buds_att_evt_cb)
        {
            buds_att_evt_cb(BUDS_IDX_TX_NTF_CFG, NULL, 0);
        }
    }
}

__STATIC void buds_gatt_cb_att_val_evt(uint8_t con_lid, uint8_t user_lid, uint16_t token,
                                          uint8_t event_type, bool complete, uint16_t hdl, co_buf_t* p_buf)
{
    // Get the address of the environment
    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);

    TRACE(4, "%s get ntf/ind handle %d shdl %d", 
        __FUNCTION__, hdl, buds_env->shdl);

    uint8_t* pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;

    DUMP8("%02x ", pData, dataLen);
    if (hdl == (buds_env->shdl + BUDS_IDX_TX_VAL))
    {
        if (buds_att_evt_cb)
        {
            buds_att_evt_cb(BUDS_IDX_TX_VAL, pData, dataLen);
        }
    }

    gatt_cli_att_event_cfm(con_lid, user_lid, token);
}

/// Set of callbacks functions for communication with GATT as a GATT User Server
__STATIC const gatt_srv_cb_t buds_gatt_srv_cb = {
    .cb_event_sent      = buds_gatt_cb_event_sent,
    .cb_att_read_get    = buds_gatt_cb_att_read_get,
    .cb_att_event_get   = buds_gatt_cb_att_event_get,
    .cb_att_info_get    = buds_gatt_cb_att_info_get,
    .cb_att_val_set     = buds_gatt_cb_att_set,
};

/// Set of callbacks functions for communication with GATT as a GATT User Client
__STATIC const gatt_cli_cb_t buds_gatt_cli_cb = {
    .cb_discover_cmp = NULL,
    .cb_read_cmp = NULL,
    .cb_write_cmp = buds_gatt_cb_write_cmp,
    .cb_att_val_get = NULL,
    .cb_svc = NULL,
    .cb_svc_info = NULL,
    .cb_inc_svc = NULL,
    .cb_char = NULL,
    .cb_desc = NULL,
    .cb_att_val = NULL,
    .cb_att_val_evt = buds_gatt_cb_att_val_evt,
    .cb_svc_changed = NULL,
};


/**
 ****************************************************************************************
 * @brief Initialization of the BUDS module.
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
static uint16_t _init_cb(prf_data_t* env, uint16_t* start_hdl, 
    uint8_t sec_lvl, uint8_t user_prio, const void* params, const void* p_cb)
{
    uint16_t status = GAP_ERR_NO_ERROR;

    TRACE(0, "attm_svc_create_db_128 returns %d start handle is %d", status, *start_hdl);

    // Allocate BUDS required environment variable
    buds_env_t* buds_env =
            (buds_env_t* ) ke_malloc(sizeof(buds_env_t), KE_MEM_PROFILE);

    memset((uint8_t *)buds_env, 0, sizeof(buds_env_t));
    // Initialize BUDS environment
    env->p_env           = (prf_hdr_t*) buds_env;

    // Register as GATT User Client
    status = gatt_user_srv_register(PREFERRED_BLE_MTU, 0, &buds_gatt_srv_cb,
                                    &(buds_env->srv_user_lid));
    
    if (GAP_ERR_NO_ERROR == status)
    {
        buds_env->shdl = *start_hdl;
        status = gatt_db_svc_add(buds_env->srv_user_lid, SVC_SEC_LVL(NOT_ENC), BUDS_SERVICE_UUID_128,
                                   BUDS_IDX_NB, NULL, buds_att_db, BUDS_IDX_NB, &buds_env->shdl);
        TRACE(1, "func %s line %d status %d", __FUNCTION__, __LINE__, status);
        if (GAP_ERR_NO_ERROR == status)
        {       
            // initialize environment variable
            env->api_id = TASK_ID_BUDS;
            buds_task_init(&(env->desc), (void *)buds_env);

            /* Put HRS in Idle state */
            // ke_state_set(env->prf_task, BUDS_IDLE);
        }
    }

    uint16_t ret = gatt_user_cli_register(PREFERRED_BLE_MTU, 0, &buds_gatt_cli_cb,
                                        &(buds_env->cli_user_lid));

    TRACE(1, "Buds cli user register returns %d", ret);
    
    TRACE(1, "BUDS srv uid %d cli uid %d shdl %d", 
        buds_env->srv_user_lid, buds_env->cli_user_lid, buds_env->shdl);
    
    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the BUDS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static uint16_t _destroy_cb(prf_data_t* p_env, uint8_t reason)
{
    buds_env_t* buds_env = (buds_env_t*) p_env->p_env;

    TRACE(0, "%s reason 0x%x", __func__, reason);
    // free profile environment variables
    p_env->p_env = NULL;
    ke_free(buds_env);

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
static void _con_create_cb(prf_data_t* env, uint8_t conidx, bool is_le_con)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
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
static void _con_cleanup_cb(prf_data_t* env, uint8_t conidx, uint16_t reason)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    /* Nothing to do */
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// BUDS Task interface required by profile manager
const struct prf_task_cbs buds_itf =
{
    _init_cb,
    _destroy_cb,
    _con_create_cb,
    _con_cleanup_cb,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* buds_prf_itf_get(void)
{
   return &buds_itf;
}

void app_add_buds(void)
{
    TRACE(1, "app_add_buds %d", TASK_ID_BUDS);
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, 0);
    
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = SVC_SEC_LVL(NOT_ENC);
    req->user_prio = 0;
    req->app_task = TASK_APP;
    req->start_hdl = 0;

    req->prf_api_id = TASK_ID_BUDS;

    // Send the message
    ke_msg_send(req);
}

void buds_register_cli_event(uint8_t conidx)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);
    gatt_cli_event_register(conidx, buds_env->cli_user_lid, 
        buds_env->shdl, buds_env->shdl+BUDS_IDX_NB - 1);
}

static bool buds_send_ind_ntf_generic(bool isNotification, uint8_t conidx, const uint8_t* ptrData, uint32_t length)
{
    enum gatt_evt_type evtType = isNotification?GATT_NOTIFY:GATT_INDICATE;

    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);

    TRACE(1, "ntfIndEnableFlag %d, %d", buds_env->ntfIndEnableFlag[conidx],
        1 << (uint8_t)evtType);

    if ((buds_env->ntfIndEnableFlag[conidx])&(1 << (uint8_t)evtType)) {

        co_buf_t* p_buf = NULL;
        prf_buf_alloc(&p_buf, length);

        uint8_t* p_data = co_buf_data(p_buf);
        memcpy(p_data, ptrData, length);

        // Dummy parameter provided to GATT
        uint16_t dummy = 0;

        // Inform the GATT that notification must be sent
        uint16_t ret = gatt_srv_event_send(conidx, buds_env->srv_user_lid, dummy, evtType,
                            buds_env->shdl + BUDS_IDX_TX_VAL, p_buf);

        // Release the buffer
        co_buf_release(p_buf);

        return (GAP_ERR_NO_ERROR == ret);
    }
    else
    {
        return false;
    }
}

bool buds_send_indication(uint8_t conidx, const uint8_t* ptrData, uint32_t length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    return buds_send_ind_ntf_generic(false, conidx, ptrData, length);
}

bool buds_send_notification(uint8_t conidx, const uint8_t* ptrData, uint32_t length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    DUMP8("%02x ", ptrData, length);
    return buds_send_ind_ntf_generic(true, conidx, ptrData, length);
}

static bool buds_send_write_req_cmd_generic(bool isNoRsp, uint8_t conidx, const uint8_t* ptrData, uint32_t length)
{
    enum gatt_write_type writeType = isNoRsp?GATT_WRITE_NO_RESP:GATT_WRITE;
    
    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);

    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    co_buf_t* p_buf = NULL;
    prf_buf_alloc(&p_buf, length);

    uint8_t* p_data = co_buf_data(p_buf);
    memcpy(p_data, ptrData, length);

    // Dummy parameter provided to GATT
    uint16_t dummy = 0;
        
    uint16_t ret = gatt_cli_write(conidx, buds_env->cli_user_lid, dummy, writeType,
                                    buds_env->shdl + BUDS_IDX_RX_VAL, 0, p_buf);

    // Release the buffer
    co_buf_release(p_buf);

    return (GAP_ERR_NO_ERROR == ret);   
}

bool buds_send_data_via_write_command(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    DUMP8("%02x ", ptrData, length);
    return buds_send_write_req_cmd_generic(true, conidx, ptrData, length);                            
}

bool buds_send_data_via_write_request(uint8_t conidx, uint8_t* ptrData, uint32_t length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    return buds_send_write_req_cmd_generic(false, conidx, ptrData, length); 
}

void buds_control_ind_ntf(uint8_t conidx, bool isEnable)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    uint16_t cccVal = 0;
    
    if (isEnable)
    {
        cccVal = PRF_CLI_START_IND|PRF_CLI_START_NTF;
    }
    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);

    co_buf_t* p_buf = NULL;
    prf_buf_alloc(&p_buf, 2);

    uint8_t* p_data = co_buf_data(p_buf);
    memcpy(p_data, (uint8_t *)&cccVal, 2);

    // Dummy parameter provided to GATT
    uint16_t dummy = buds_env->shdl + BUDS_IDX_TX_NTF_CFG;

    TRACE(1, "control ntf %d", __LINE__);
    gatt_cli_write(conidx, buds_env->cli_user_lid, dummy, GATT_WRITE,
                                    buds_env->shdl + BUDS_IDX_TX_NTF_CFG, 0, p_buf);

    // Release the buffer
    co_buf_release(p_buf);
}

void buds_data_received_register(buds_att_evt_callback_t callback)
{
    buds_att_evt_cb = callback;
}

#endif /* BLE_BUDS */

/// @} BUDS

