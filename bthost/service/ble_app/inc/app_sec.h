/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
 * @addtogroup APP_SEC
 * @{
 ****************************************************************************************
 */

#ifndef APP_SEC_H_
#define APP_SEC_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "ble_core_common.h"

#if (BLE_APP_SEC)

#include <stdint.h>          // Standard Integer Definition
#include "nvrecord_extension.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_SEC_AUTH_CT2_BIT            (1 << 5)
#define APP_SEC_AUTH_KEYPRESS_BIT       (1 << 4)
#define APP_SEC_AUTH_SC_BIT             (1 << 3)
#define APP_SEC_AUTH_MITM_BIT           (1 << 2)
#ifdef CTKD_ENABLE
#define TMP2_LEN                        (4)
#define BRLE_LEN                        (4)
#define SALT_LEN                        (16)
#define LTK_LEN                         (16)
#define BT_LINKKEY_SIZE                 (16)
#endif

/*
 * STRUCTURES DEFINITIONS
 ****************************************************************************************
 */

struct app_sec_env_tag
{
    // Bond status
    bool bonded;
};

struct smp_identity {
    /* The order to get 'Identity Information(0x08)' and 'Identity Address Information(0x09)' is not always same */
    int recv;
    /* Identity Resolving Key */
    uint8_t irk[BLE_IRK_SIZE];
    /* Identity Address */
    BLE_ADDR_INFO_T pBdAddr;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// Table of message handlers
extern const struct app_subtask_handlers app_sec_handlers;

/*
 * GLOBAL FUNCTIONS DECLARATIONS
 ****************************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief Initialize the Application Security Module
 ****************************************************************************************
 */
void app_sec_init(void);


#if (NVDS_SUPPORT)
/**
 ****************************************************************************************
 * @brief Remove all bond data stored in NVDS
 ****************************************************************************************
 */
void app_sec_remove_bond(void);
#endif //(NVDS_SUPPORT)

/**
 ****************************************************************************************
 * @brief Send a security request to the peer device. This function is used to require the
 * central to start the encryption with a LTK that would have shared during a previous
 * bond procedure.
 *
 * @param[in]   - conidx: Connection Index
 ****************************************************************************************
 */
void app_sec_send_security_req(uint8_t conidx, uint8_t authority);
bool app_sec_send_encrypt_req(uint8_t conidx, uint8_t * deviceSecurityInfo);
void app_sec_send_pair_req(uint8_t conidx, struct gapc_pairing_feat * pair_feat);
void app_sec_nc_exch_accept(uint8_t conidx, bool accept);
void app_sec_reset_env_on_connection(void);
void app_sec_reg_dist_lk_bit_set_callback(set_rsp_dist_lk_bit_field_func callback);
void *app_sec_reg_dist_lk_bit_get_callback(void);
void app_sec_reg_smp_identify_info_cmp_callback(smp_identify_addr_exch_complete callback);
bool app_sec_store_and_sync_ble_info(uint8_t *ble_save_info);
void app_sec_start_pairing(uint8_t conidx);

#ifdef __cplusplus
}
#endif

#endif //(BLE_APP_SEC)

#endif // APP_SEC_H_

/// @} APP_SEC
