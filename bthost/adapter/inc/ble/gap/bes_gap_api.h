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
#ifndef __BES_GAP_API_H__
#define __BES_GAP_API_H__
#include "ble_core_common.h"
#include "stdbool.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define BES_BLE_INVALID_CONNECTION_INDEX    0xFF

/// error code, sync see@co_error
enum bes_error
{
/*****************************************************
 ***              ERROR CODES                      ***
 *****************************************************/

    BES_ERROR_NO_ERROR                        = 0x00,
    BES_ERROR_UNKNOWN_HCI_COMMAND             = 0x01,
    BES_ERROR_UNKNOWN_CONNECTION_ID           = 0x02,
    BES_ERROR_HARDWARE_FAILURE                = 0x03,
    BES_ERROR_PAGE_TIMEOUT                    = 0x04,
    BES_ERROR_AUTH_FAILURE                    = 0x05,
    BES_ERROR_PIN_MISSING                     = 0x06,
    BES_ERROR_MEMORY_CAPA_EXCEED              = 0x07,
    BES_ERROR_CON_TIMEOUT                     = 0x08,
    BES_ERROR_CON_LIMIT_EXCEED                = 0x09,
    BES_ERROR_SYNC_CON_LIMIT_DEV_EXCEED       = 0x0A,
    BES_ERROR_CON_ALREADY_EXISTS              = 0x0B,
    BES_ERROR_COMMAND_DISALLOWED              = 0x0C,
    BES_ERROR_CONN_REJ_LIMITED_RESOURCES      = 0x0D,
    BES_ERROR_CONN_REJ_SECURITY_REASONS       = 0x0E,
    BES_ERROR_CONN_REJ_UNACCEPTABLE_BDADDR    = 0x0F,
    BES_ERROR_CONN_ACCEPT_TIMEOUT_EXCEED      = 0x10,
    BES_ERROR_UNSUPPORTED                     = 0x11,
    BES_ERROR_INVALID_HCI_PARAM               = 0x12,
    BES_ERROR_REMOTE_USER_TERM_CON            = 0x13,
    BES_ERROR_REMOTE_DEV_TERM_LOW_RESOURCES   = 0x14,
    BES_ERROR_REMOTE_DEV_POWER_OFF            = 0x15,
    BES_ERROR_CON_TERM_BY_LOCAL_HOST          = 0x16,
    BES_ERROR_REPEATED_ATTEMPTS               = 0x17,
    BES_ERROR_PAIRING_NOT_ALLOWED             = 0x18,
    BES_ERROR_UNKNOWN_LMP_PDU                 = 0x19,
    BES_ERROR_UNSUPPORTED_REMOTE_FEATURE      = 0x1A,
    BES_ERROR_SCO_OFFSET_REJECTED             = 0x1B,
    BES_ERROR_SCO_INTERVAL_REJECTED           = 0x1C,
    BES_ERROR_SCO_AIR_MODE_REJECTED           = 0x1D,
    BES_ERROR_INVALID_LMP_PARAM               = 0x1E,
    BES_ERROR_UNSPECIFIED_ERROR               = 0x1F,
    BES_ERROR_UNSUPPORTED_LMP_PARAM_VALUE     = 0x20,
    BES_ERROR_ROLE_CHANGE_NOT_ALLOWED         = 0x21,
    BES_ERROR_LMP_RSP_TIMEOUT                 = 0x22,
    BES_ERROR_LMP_COLLISION                   = 0x23,
    BES_ERROR_LMP_PDU_NOT_ALLOWED             = 0x24,
    BES_ERROR_ENC_MODE_NOT_ACCEPT             = 0x25,
    BES_ERROR_LINK_KEY_CANT_CHANGE            = 0x26,
    BES_ERROR_QOS_NOT_SUPPORTED               = 0x27,
    BES_ERROR_INSTANT_PASSED                  = 0x28,
    BES_ERROR_PAIRING_WITH_UNIT_KEY_NOT_SUP   = 0x29,
    BES_ERROR_DIFF_TRANSACTION_COLLISION      = 0x2A,
    BES_ERROR_QOS_UNACCEPTABLE_PARAM          = 0x2C,
    BES_ERROR_QOS_REJECTED                    = 0x2D,
    BES_ERROR_CHANNEL_CLASS_NOT_SUP           = 0x2E,
    BES_ERROR_INSUFFICIENT_SECURITY           = 0x2F,
    BES_ERROR_PARAM_OUT_OF_MAND_RANGE         = 0x30,
    BES_ERROR_ROLE_SWITCH_PEND                = 0x32, /* LM_ROLE_SWITCH_PENDING               */
    BES_ERROR_RESERVED_SLOT_VIOLATION         = 0x34, /* LM_RESERVED_SLOT_VIOLATION           */
    BES_ERROR_ROLE_SWITCH_FAIL                = 0x35, /* LM_ROLE_SWITCH_FAILED                */
    BES_ERROR_EIR_TOO_LARGE                   = 0x36, /* LM_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE */
    BES_ERROR_SP_NOT_SUPPORTED_HOST           = 0x37,
    BES_ERROR_HOST_BUSY_PAIRING               = 0x38,
    BES_ERROR_CONTROLLER_BUSY                 = 0x3A,
    BES_ERROR_UNACCEPTABLE_CONN_PARAM         = 0x3B,
    BES_ERROR_ADV_TO                          = 0x3C,
    BES_ERROR_TERMINATED_MIC_FAILURE          = 0x3D,
    BES_ERROR_CONN_FAILED_TO_BE_EST           = 0x3E,
    BES_ERROR_CCA_REJ_USE_CLOCK_DRAG          = 0x40,
    BES_ERROR_TYPE0_SUBMAP_NOT_DEFINED        = 0x41,
    BES_ERROR_UNKNOWN_ADVERTISING_ID          = 0x42,
    BES_ERROR_LIMIT_REACHED                   = 0x43,
    BES_ERROR_OPERATION_CANCELED_BY_HOST      = 0x44,
    BES_ERROR_PKT_TOO_LONG                    = 0x45,

    BES_ERROR_UNDEFINED                       = 0xFF,
};

///BD address type
/// sync see@addr_type
enum bes_addr_type
{
    ///Public BD address
    BES_ADDR_PUBLIC                   = 0x00,
    ///Random BD Address
    BES_ADDR_RAND,
    /// Controller generates Resolvable Private Address based on the
    /// local IRK from resolving list. If resolving list contains no matching
    /// entry, use public address.
    BES_ADDR_RPA_OR_PUBLIC,
    /// Controller generates Resolvable Private Address based on the
    /// local IRK from resolving list. If resolving list contains no matching
    /// entry, use random address.
    BES_ADDR_RPA_OR_RAND,
    /// mask used to determine Address type in the air
    BES_ADDR_MASK                     = 0x01,
    /// mask used to determine if an address is an RPA
    BES_ADDR_RPA_MASK                 = 0x02,
    /// Random device address (controller unable to resolve)
    BES_ADDR_RAND_UNRESOLVED          = 0xFE,
    /// No address provided (anonymous advertisement)
    BES_ADDR_NONE                     = 0xFF,
};

///ADV channel map
/// sync see@adv_channel_map
enum bes_adv_channel_map
{
    ///Byte value for advertising channel map for channel 37 enable
    BES_ADV_CHNL_37_EN                = 0x01,
    ///Byte value for advertising channel map for channel 38 enable
    BES_ADV_CHNL_38_EN                = 0x02,
    ///Byte value for advertising channel map for channel 39 enable
    BES_ADV_CHNL_39_EN                = 0x04,
    ///Byte value for advertising channel map for channel 37, 38 and 39 enable
    BES_ADV_ALL_CHNLS_EN              = 0x07,
};

///Advertising filter policy
/// sync see@adv_filter_policy
enum bes_adv_filter_policy
{
    ///Allow both scan and connection requests from anyone
    BES_ADV_ALLOW_SCAN_ANY_CON_ANY    = 0x00,
    ///Allow both scan req from White List devices only and connection req from anyone
    BES_ADV_ALLOW_SCAN_WLST_CON_ANY,
    ///Allow both scan req from anyone and connection req from White List devices only
    BES_ADV_ALLOW_SCAN_ANY_CON_WLST,
    ///Allow scan and connection requests from White List devices only
    BES_ADV_ALLOW_SCAN_WLST_CON_WLST,
};

///Advertising HCI Type
enum
{
    ///Connectable Undirected advertising
    BES_ADV_CONN_UNDIR                = 0x00,
    ///Connectable high duty cycle directed advertising
    BES_ADV_CONN_DIR,
    ///Discoverable undirected advertising
    BES_ADV_DISC_UNDIR,
    ///Non-connectable undirected advertising
    BES_ADV_NONCONN_UNDIR,
    ///Connectable low duty cycle directed advertising
    BES_ADV_CONN_DIR_LDC,
};

/// Own BD address source of the device
/// sync see@APP_GAPM_OWN_ADDR_E
typedef enum
{
   /// Public or Private Static Address according to device address configuration
   BES_GAP_STATIC_ADDR,
   /// Generated resolvable private random address
   BES_GAP_GEN_RSLV_ADDR,
   /// Generated non-resolvable private random address
   BES_GAP_GEN_NON_RSLV_ADDR,
} BES_GAP_OWN_ADDR_E;

typedef struct bes_ble_bdaddr
{
    /// BD Address of device
    uint8_t addr[6];
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
} bes_ble_bdaddr_t;

typedef struct
{
    BLE_ADV_ACTIVITY_USER_E actv_user;
    bool is_custom_adv_flags;
    BLE_ADV_ADDR_TYPE_E type;
    uint8_t *local_addr;
    ble_bdaddr_t *peer_addr;
    uint32_t adv_interval;
    BLE_ADV_TYPE_E adv_type;
    ADV_MODE_E adv_mode;
    int8_t tx_power_dbm;
    uint8_t *adv_data;
    uint8_t adv_data_size;
    uint8_t *scan_rsp_data;
    uint8_t scan_rsp_data_size;
} bes_ble_gap_cus_adv_param_t;

void bes_ble_gap_core_register_global_handler(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler);

void bes_ble_gap_stub_user_init(void);

void bes_ble_gap_force_switch_adv(enum BLE_ADV_SWITCH_USER_E user, bool isToEnableAdv);

void bes_ble_gap_start_connectable_adv(uint16_t advInterval);

void bes_ble_gap_start_scan(enum BLE_SCAN_FILTER_POLICY scanFilterPolicy, uint16_t scanWindow, uint16_t scanInterval);

void bes_ble_gap_stop_scan(void);

void bes_ble_gap_set_white_list(BLE_WHITE_LIST_USER_E user, ble_bdaddr_t *bdaddr, uint8_t size);

void bes_ble_gap_set_rpa_list(uint8_t *peer_addr,const uint8_t *irk);

void bes_ble_gap_set_rpa_timeout(uint16_t rpa_timeout);

void bes_ble_gap_start_three_adv(uint32_t BufPtr, uint32_t BufLen);

void bes_ble_gap_custom_adv_start(BLE_ADV_ACTIVITY_USER_E actv_user);

void bes_ble_gap_custom_adv_write_data(bes_ble_gap_cus_adv_param_t *param);

void bes_ble_gap_custom_adv_stop(BLE_ADV_ACTIVITY_USER_E actv_user);

void bes_ble_gap_stop_all_adv(uint32_t BufPtr, uint32_t BufLen);

void bes_ble_gap_set_rpa_list(uint8_t *peer_addr,const uint8_t *irk);

void bes_ble_gap_refresh_irk(void);

bool bes_ble_gap_get_peer_solved_addr(uint8_t conidx, uint8_t** p_addr);

void bes_ble_gap_start_connect(bes_ble_bdaddr_t *addr, BES_GAP_OWN_ADDR_E own_type);

void bes_ble_gap_cancel_connecting(void);

void bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_E adv_intv_user, BLE_ADV_USER_E adv_user, uint32_t interval);

bool bes_ble_gap_is_in_advertising_state(void);

void bes_ble_gap_refresh_adv_state(uint16_t advInterval);

void bes_ble_gap_start_disconnect(uint8_t conIdx);

bool bes_ble_gap_is_connection_on(uint8_t index);

void bes_ble_gap_register_data_fill_handle(BLE_ADV_USER_E user, BLE_DATA_FILL_FUNC_T func, bool enable);

void bes_ble_gap_data_fill_enable(BLE_ADV_USER_E user, bool enable);

void bes_ble_gap_force_switch_adv(enum BLE_ADV_SWITCH_USER_E user, bool isToEnableAdv);

void bes_ble_gap_disconnect_all(void);

int8_t bes_ble_gap_get_rssi(uint8_t conidx);

void bes_ble_gap_conn_update_param(uint8_t conidx, uint32_t min_interval_in_ms, uint32_t max_interval_in_ms,
        uint32_t supervision_timeout_in_ms, uint8_t  slaveLatency);

void bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_E mode, bool isEnable);

void bes_ble_gap_sec_reg_dist_lk_bit_set_callback(set_rsp_dist_lk_bit_field_func callback);

void bes_ble_gap_sec_reg_smp_identify_info_cmp_callback(smp_identify_addr_exch_complete callback);

/// IBRT CALL FUNC
void bes_ble_roleswitch_start(void);

void bes_ble_roleswitch_complete(uint8_t newRole);

void bes_ble_role_update(uint8_t newRole);

void bes_ble_ibrt_event_entry(uint8_t ibrt_evt_type);

#ifdef TWS_SYSTEM_ENABLED
void bes_ble_sync_ble_info(void);

void bes_ble_gap_mode_tws_sync_init(void);

#endif

bool bes_ble_gap_is_any_connection_exist(void);

uint8_t bes_ble_gap_connection_count(void);

uint16_t bes_ble_gap_get_conhdl_from_conidx(uint8_t conidx);

uint16_t bes_ble_gap_btgatt_con_create(uint8_t* btadress,uint16_t conn_handle);

/// END IBRT CALL FUNC

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BES_GAP_API_H__ */
