/**
 * @file ancs.c
 * @author BES AI team
 * @version 0.1
 * @date 2020-10-26
 * 
 * @copyright Copyright (c) 2015-2020 BES Technic.
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
 */

/*****************************header include********************************/
#include "rwapp_config.h"

#ifdef ANCS_ENABLED
#include "anc_common.h"
#include "ancs.h"
#include "ancs_task.h"
#include "ancc.h"
#include "ke_mem.h"
#include "ancc_task.h"
#include "prf_dbg.h"

/*********************external function declearation************************/

/************************private macro defination***************************/
#define PRF_TAG             "[ANCS]"
#define ANC_MAX_LEN         244

/***********************
 * BES Characteristic Property Flags
 ************************
 *   PERM_MASK_BROADCAST       /// Broadcast descriptor present
 *   PERM_MASK_RD              /// Read Access Mask
 *   PERM_MASK_WRITE_COMMAND   /// Write Command Enabled attribute Mask
 *   PERM_MASK_WRITE_REQ       /// Write Request Enabled attribute Mask
 *   PERM_MASK_NTF             /// Notification Access Mask
 *   PERM_MASK_IND             /// Indication Access Mask
 *   PERM_MASK_WRITE_SIGNED    /// Write Signed Enabled attribute Mask
 *   PERM_MASK_EXT             /// Extended properties descriptor present
 */
#define RD_PERM         PROP(RD) | SEC_LVL(RP, AUTH)
#define WR_REQ_PERM     PROP(WR) | SEC_LVL(WP, AUTH)
#define NTF_PERM        PROP(N)  | SEC_LVL(NIP, AUTH)

/************************private type defination****************************/

/************************extern function declearation***********************/

/**********************private function declearation************************/
/**
 * @brief Initialization of the Profile module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 * 
 * @param p_env         Collector or Service allocated environment data.
 * @param p_start_hdl   Service start handle (0 - dynamically allocated), only applies for services.
 * @param sec_lvl       Security level (@see enum enum gatt_svc_info_bf)
 * @param user_prio     GATT User priority
 * @param p_params      Configuration parameters of profile collector or service (32 bits aligned)
 * @param p_cb          Callback structure that handles event from profile
 * @return uint16_t     status code to know if profile initialization succeed or not.
 */
static uint16_t _init(prf_data_t *p_env, uint16_t *p_start_hdl, uint8_t sec_lvl, uint8_t user_prio,
                      const void *p_params, const void *p_cb);

/**
 * @brief Destruction of the profile module - due to a reset or profile remove.
 * 
 * @param p_env         Collector or Service allocated environment data.
 * @param reason        Destroy reason (@see enum prf_destroy_reason)
 * @return uint16_t     status of the destruction, if fails, profile considered not removed.
 */
static uint16_t _destroy(prf_data_t* p_env, uint8_t reason);

/**
 * @brief Handles Connection creation for profile
 * 
 * @param p_env         Collector or Service allocated environment data.
 * @param conidx        Connection index
 * @param p_con_param   Pointer to connection parameters information
 */
static void _con_create(prf_data_t* p_env, uint8_t conidx, bool is_le_con);

/**
 * @brief Handles Disconnection for profile
 * 
 * @param p_env         Collector or Service allocated environment data.
 * @param conidx        Connection index
 * @param reason        Detach reason
 */
static void _con_cleanup(prf_data_t *p_env, uint8_t conidx, uint16_t reason);

/**
 * @brief Indicates update of connection parameters
 * 
 * @param p_env         Collector or Service allocated environment data.
 * @param conidx        Connection index
 * @param p_con_param   Pointer to new connection parameters information
 */
static void _con_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param);

/************************private variable defination************************/
static const uint8_t ancs_uuid_128[GATT_UUID_128_LEN] = ANC_UUID_128;

/*******************************************************************************
 *******************************************************************************
 * ANCS Service - 67A846AD-DE3E-451B-A6D8-7B2899CA2370
 *******************************************************************************
 *******************************************************************************/
static const struct gatt_att_desc ancs_att_db[] = {

    /**************************************************
     *  ANCS Service Declaration
     **************************************************/

    [ANCS_PROXY_SVC] = {
        UUID_16_TO_ARRAY(GATT_DECL_PRIMARY_SERVICE),
        PROP(RD),
        0,
    },

    /**************************************************
     * ANCS Notification Source
     *    - UUID: 9FBF120D-6301-42D9-8C58-25E699A21DBD
     *    - Notifiable (Direct proxy of Apple's ANCS characteristic and matches
     *its UUID and properties)
     **************************************************/
    // Characteristic Declaration
    [ANCS_PROXY_NS_CHAR] = {
        UUID_16_TO_ARRAY(GATT_DECL_CHARACTERISTIC),
        PROP(RD),
        0,
    },
    // Characteristic Value
    [ANCS_PROXY_NS_VAL] = {
        ANC_CHAR_NS,
        NTF_PERM | ATT_UUID(128),
        ANC_MAX_LEN,
    },
    // Client Characteristic Configuration Descriptor
    [ANCS_PROXY_NS_CFG] = {
        UUID_16_TO_ARRAY(GATT_DESC_CLIENT_CHAR_CFG),
        RD_PERM | WR_REQ_PERM,
        0,
    },

    /**************************************************
     * ANCS Ready (Google specific)
     *    - UUID: FBE87F6C-3F1A-44B6-B577-0BAC731F6E85
     *    - provides the current state of ANCS (0 - not ready, 1 - ready)
     *    - read/notify characteristic
     **************************************************/
    // Characteristic Declaration
    [ANCS_PROXY_READY_CHAR] = {
        UUID_16_TO_ARRAY(GATT_DECL_CHARACTERISTIC),
        PROP(RD),
        0,
    },
    // Characteristic Value
    [ANCS_PROXY_READY_VAL] = {
        ANC_CHAR_READY,
        NTF_PERM | RD_PERM | ATT_UUID(128),
        ANC_MAX_LEN,
    },
    // Client Characteristic Configuration Descriptor
    [ANCS_PROXY_READY_CFG] = {
        UUID_16_TO_ARRAY(GATT_DESC_CLIENT_CHAR_CFG),
        RD_PERM | WR_REQ_PERM,
        0,
    },

    /**************************************************
     * ANCS Data Source
     *    - UUID: 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB
     *    - Notifiable (Direct proxy of Apple's ANCS characteristic and matches
     *its UUID and properties)
     **************************************************/
    // Characteristic Declaration
    [ANCS_PROXY_DS_CHAR] = {
        UUID_16_TO_ARRAY(GATT_DECL_CHARACTERISTIC),
        PROP(RD),
        0,
    },
    // Characteristic Value
    [ANCS_PROXY_DS_VAL] = {
        ANC_CHAR_DS,
        NTF_PERM | ATT_UUID(128),
        ANC_MAX_LEN,
    },
    // Client Characteristic Configuration Descriptor
    [ANCS_PROXY_DS_CFG] = {
        UUID_16_TO_ARRAY(GATT_DESC_CLIENT_CHAR_CFG),
        RD_PERM | WR_REQ_PERM,
        0,
    },

    /**************************************************
     * ANCS Control Point
     *    - UUID: 69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9
     *    - Writeable with response (Direct proxy of Apple's ANCS characteristic
     *and matches its UUID and properties)
     **************************************************/
    // Characteristic Declaration
    [ANCS_PROXY_CP_CHAR] = {
        UUID_16_TO_ARRAY(GATT_DECL_CHARACTERISTIC),
        PROP(RD),
        0,
    },
    // Characteristic Value
    [ANCS_PROXY_CP_VAL] = {
        ANC_CHAR_CP,
        WR_REQ_PERM | ATT_UUID(128),
        ANC_MAX_LEN,
    },
};

static const struct prf_task_cbs ancs_itf = {
    .cb_init = _init,
    .cb_destroy = _destroy,
    .cb_con_create = _con_create,
    .cb_con_cleanup = _con_cleanup,
    .cb_con_upd = _con_upd,
};

/****************************function defination****************************/
static uint16_t _init(prf_data_t *p_env, uint16_t *p_start_hdl, uint8_t sec_lvl, uint8_t user_prio,
                      const void *p_params, const void *p_cb)
{
    LOG_I("%s ", __func__);
    uint8_t status = GAP_ERR_NO_ERROR;

    /// allocate&init profile environment data
    PRF_ENV_T(ancs) *ancs_env = ke_malloc(sizeof(PRF_ENV_T(ancs)), KE_MEM_PROFILE);
    memset((uint8_t *)ancs_env, 0, sizeof(PRF_ENV_T(ancs)));

    do
    {
        /// registor GATT server user
        status = gatt_user_srv_register(PREFERRED_BLE_MTU, 
                                        user_prio,
                                        (gatt_srv_cb_t*)ancs_task_get_srv_cbs(),
                                        &(ancs_env->user_lid));

        /// check the GATT server user registation excution result
        if (GAP_ERR_NO_ERROR != status)
        {
            break;
        }

        status = gatt_db_svc_add(ancs_env->user_lid, sec_lvl,
                                 ancs_uuid_128,
                                 ANCS_PROXY_NUM, NULL, ancs_att_db, ANCS_PROXY_NUM,
                                 &ancs_env->shdl);

        /// check the GATT database add execution result
        if (GAP_ERR_NO_ERROR != status)
        {
            break;
        }

        /// initialize environment variable
        p_env->p_env = (prf_hdr_t *)ancs_env;
        ancs_task_init(&(p_env->desc), (void *)ancs_env);
        // ke_state_set(p_env->prf_task, 0);
    } while (0);

    return status;
}

static uint16_t _destroy(prf_data_t* p_env, uint8_t reason)
{
    PRF_ENV_T(ancs) *ancs_env = (PRF_ENV_T(ancs) *)p_env->p_env;
    p_env->p_env = NULL;
    ke_free(ancs_env);

    return GAP_ERR_NO_ERROR;
}

static void _con_create(prf_data_t* p_env, uint8_t conidx, bool is_le_con)
{
    PRF_ENV_T(ancs) *ancs_env = (PRF_ENV_T(ancs) *)p_env->p_env;
    ancs_env->subEnv[conidx].conidx = conidx;

    LOG_I("%s create: conidx=%d start=0x%x end=0x%x",
          __func__,
          conidx,
          ancs_env->shdl,
          ancs_env->shdl + ARRAY_SIZE(ancs_att_db));
}

static void _con_cleanup(prf_data_t *p_env, uint8_t conidx, uint16_t reason)
{
    PRF_ENV_T(ancs) *ancs_env = (PRF_ENV_T(ancs) *)p_env->p_env;
    ProxySubEnv_t *pSubEnv = &(ancs_env->subEnv[conidx]);

    pSubEnv->ready = false;
    pSubEnv->ready_ntf_enable = false;
    memset(&pSubEnv->client_handles[0], 0, sizeof(pSubEnv->client_handles));
}

static void _con_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param)
{
    LOG_I("%s", __func__);
}

const struct prf_task_cbs* ancs_proxy_prf_itf_get(void)
{
    LOG_I("%s entry.", __func__);
    return &ancs_itf;
}

#endif
