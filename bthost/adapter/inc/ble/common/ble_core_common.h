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
#ifndef __BLE_CORE_COMMON_H__
#define __BLE_CORE_COMMON_H__
#include "stdbool.h"
#include "ble_common_define.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#ifndef INVALID_CONNECTION_INDEX
#define INVALID_CONNECTION_INDEX 0xFF
#endif

// the default interval is 160ms, note that for Bisto user case, to
// let GVA iOS version pop-out notification smoothly, the maximum interval should be this value
#define BLE_ADV_INVALID_INTERVAL (0)
#define BLE_FAST_ADVERTISING_INTERVAL (48)
#define BLE_ADVERTISING_INTERVAL (160)

#define EXT_ADV_DATA_LEN       251
#define SCAN_RSP_EX_DATA_LEN   251

typedef enum
{
    BLE_ADV_INTERVALREQ_USER_A2DP,
    BLE_ADV_INTERVALREQ_USER_SCO,
    BLE_ADV_INTERVALREQ_USER_TWS_STM,
    BLE_ADV_INTERVALREQ_USER_NUM,
} BLE_ADV_INTERVALREQ_USER_E;

typedef enum
{
    BLE_CONN_PARAM_MODE_DEFAULT        = 0,
    BLE_CONN_PARAM_MODE_AI_STREAM_ON,
    BLE_CONN_PARAM_MODE_A2DP_ON,
    BLE_CONN_PARAM_MODE_HFP_ON,
    BLE_CONN_PARAM_MODE_OTA,
    BLE_CONN_PARAM_MODE_OTA_SLOWER,
    BLE_CONN_PARAM_MODE_SNOOP_EXCHANGE,
    BLE_CONN_PARAM_MODE_SVC_DISC,
    BLE_CONN_PARAM_MODE_ISO_DATA,
    BLE_CONN_PARAM_MODE_NUM,
} BLE_CONN_PARAM_MODE_E;

//Scan filter policy
enum BLE_SCAN_FILTER_POLICY {
    ///Allow advertising packets from anyone
    BLE_SCAN_ALLOW_ADV_ALL            = 0x00,
    ///Allow advertising packets from White List devices only
    BLE_SCAN_ALLOW_ADV_WLST,
    ///Allow advertising packets from anyone and Direct adv using RPA in InitA
    BLE_SCAN_ALLOW_ADV_ALL_AND_INIT_RPA,
    ///Allow advertising packets from White List devices only and Direct adv using RPA in InitA
    BLE_SCAN_ALLOW_ADV_WLST_AND_INIT_RPA,
};

#ifdef BLE_ADV_RPA_ENABLED
#define BLE_DEFAULT_SCAN_POLICY BLE_SCAN_ALLOW_ADV_ALL_AND_INIT_RPA
#else
#define BLE_DEFAULT_SCAN_POLICY BLE_SCAN_ALLOW_ADV_ALL
#endif

enum BLE_ADV_SWITCH_USER_E {
    BLE_SWITCH_USER_RS          = 0, // used for role switch
    BLE_SWITCH_USER_BOX         = 1, // used for box open/close
    BLE_SWITCH_USER_AI          = 2, // used for ai
    BLE_SWITCH_USER_BT_CONNECT  = 3, // used for bt connect
    BLE_SWITCH_USER_SCO         = 4, // used for sco
    BLE_SWITCH_USER_IBRT        = 5, // used for ibrt
    BLE_SWITCH_USER_FPGA        = 6, // used for fpga
    BLE_SWITCH_USER_BLE_AUDIO   = 7, // used for ble audio

    BLE_SWITCH_USER_NUM,
};

/**
 * @brief The user of the ble adv
 *
 */
typedef enum
{
    USER_INUSE = 0,
    USER_ALL = USER_INUSE,
    USER_STUB,
    USER_GFPS,
    USER_SWIFT,
    USER_GSOUND,
    USER_AI,
    USER_INTERCONNECTION,
    USER_TILE,
    USER_OTA,
    USER_BLE_AUDIO,
    USER_SPOT,
    USER_BLE_CUSTOMER_0,
    USER_BLE_CUSTOMER_1,
    USER_BLE_CUSTOMER_2,
    USER_BLE_CUSTOMER_3,
    USER_BLE_DEMO0,
    USER_BLE_DEMO1,
    BLE_ADV_USER_NUM,
} BLE_ADV_USER_E;

typedef void (*BLE_DATA_FILL_FUNC_T)(void *advParam);

/**
 * @brief The user of the adv activity
 *
 */
typedef enum
{
    BLE_ADV_ACTIVITY_USER_0 = 0,
    BLE_ADV_ACTIVITY_USER_1,
    BLE_ADV_ACTIVITY_USER_2,
    BLE_ADV_ACTIVITY_USER_3,

    BLE_ADV_ACTIVITY_USER_NUM,
} BLE_ADV_ACTIVITY_USER_E;

/* Advertise Types                                  */
/* ************************************************ */
typedef enum app_ble_adv_type
{
    /// Connectable and scannable undirected advertising
    ADV_TYPE_UNDIRECT = 0,
    /// Directed connectable with Low Duty Cycle
    ADV_TYPE_DIRECT_LDC,
    /// Directed connectable with High Duty Cycle
    ADV_TYPE_DIRECT_HDC,
    /// Non-connectable but Scannable undirected advertising
    ADV_TYPE_NON_CONN_SCAN,
    /// Non-connectable and nonscannable undirected
    ADV_TYPE_NON_CONN_NON_SCAN,
    /// Undirected connectable but scannable with extended ADV
    ADV_TYPE_CONN_EXT_ADV,
    /// Directed connectable no scannable with extended ADV
    ADV_TYPE_EXT_CON_DIRECT,
    /// Type MAx Number
    ADV_TYPE_MAX,
} BLE_ADV_TYPE_E;

/* Advertising mode types                           */
/* ************************************************ */
typedef enum adv_mode
{
    /// Legacy advertising
    ADV_MODE_LEGACY = 0,
    /// Extended advertising
    ADV_MODE_EXTENDED,
    /// Periodic advertising
    ADV_MODE_PERIODIC,
}ADV_MODE_E;

/* Advertising discoverable mode types                           */
/* ************************************************ */
typedef enum adv_disc_mode
{
    /// Mode in non-discoverable
    ADV_DISC_MODE_NON_DISC = 0,
    /// Mode in general discoverable
    ADV_DISC_MODE_GEN_DISC,
    /// Mode in limited discoverable
    ADV_DISC_MODE_LIM_DISC,
    /// Broadcast mode without presence of AD_TYPE_FLAG in advertising data
    ADV_DISC_MODE_BEACON,
    ADV_DISC_MODE_MAX,
}ADV_DISC_MODE_E;

typedef enum BLE_CONNECT_STATE {
    BLE_DISCONNECTED  = 0,
    BLE_DISCONNECTING = 1,
    BLE_CONNECTED     = 2,
}BLE_CONNECT_STATE_E;

typedef enum
{
    BLE_ADV_PUBLIC_STATIC = 0,
    BLE_ADV_PRIVATE_STATIC = 1,
    BLE_ADV_RPA = 2
} BLE_ADV_ADDR_TYPE_E;

/**
 * @brief Advertise TX power level
 *
 */
typedef enum
{
    BLE_ADV_TX_POWER_LEVEL_0,
    BLE_ADV_TX_POWER_LEVEL_1,
    BLE_ADV_TX_POWER_LEVEL_2,
} BLE_ADV_TX_POWER_LEVEL_E;

typedef enum _BLE_WHITE_LIST_USER_E {
    BLE_WHITE_LIST_USER_TWS     = 0,    // used for aob_tws
    BLE_WHITE_LIST_USER_MOBILE,         // used for mobile manager

    BLE_WHITE_LIST_USER_NUM,
} BLE_WHITE_LIST_USER_E;

/// Key Distribution Flags
/// keep up with @gap_kdist
enum bes_gap_kdist
{
    /// No Keys to distribute
    BES_GAP_KDIST_NONE   = 0x00,
    /// Encryption key in distribution
    BES_GAP_KDIST_ENCKEY = (1 << 0),
    /// IRK (ID key)in distribution
    BES_GAP_KDIST_IDKEY  = (1 << 1),
    /// CSRK(Signature key) in distribution
    BES_GAP_KDIST_SIGNKEY= (1 << 2),
    /// LTK in distribution
    BES_GAP_KDIST_LINKKEY= (1 << 3),

    BES_GAP_KDIST_LAST =   (1 << 4)
};

typedef struct ble_bdaddr
{
    /// BD Address of device
    uint8_t addr[BTIF_BD_ADDR_SIZE];
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
} ble_bdaddr_t;

typedef struct {
    BLE_ADV_ACTIVITY_USER_E adv_actv_user;
    bool isBleFlagsAdvDataConfiguredByAppLayer;
    /// Advertising filtering policy (@see enum adv_filter_policy)
    uint32_t PeriodicIntervalMin;
    uint32_t PeriodicIntervalMax;
    uint8_t filter_pol;
    BLE_ADV_TYPE_E advType;
    ADV_MODE_E advMode;
    ADV_DISC_MODE_E discMode;
    uint32_t advInterval;
    uint32_t advUserInterval[BLE_ADV_USER_NUM];
    // Maximum power level
    BLE_ADV_TX_POWER_LEVEL_E advTxPwr;
    uint8_t advDataLen;
    uint8_t advData[EXT_ADV_DATA_LEN];
    uint8_t scanRspDataLen;
    uint8_t scanRspData[EXT_ADV_DATA_LEN];
    uint8_t localAddrType;
    uint8_t localAddr[BTIF_BD_ADDR_SIZE];
    ble_bdaddr_t peerAddr;
}  __attribute__((__packed__)) BLE_ADV_PARAM_T;

/******************************type defination******************************/
/**
 * @brief The event type of the ble
 *
 */
typedef enum{
    BLE_LINK_CONNECTED_EVENT               = 0,
    BLE_CONNECT_BOND_EVENT                 = 1,    //pairing success
    BLE_CONNECT_BOND_FAIL_EVENT            = 2,    //pairing failed
    BLE_CONNECT_NC_EXCH_EVENT              = 3,    //Numeric Comparison - Exchange of Numeric Value
    BLE_CONNECT_ENCRYPT_EVENT              = 4,    //encrypt complete
    BLE_CONNECTING_STOPPED_EVENT           = 5,
    BLE_CONNECTING_FAILED_EVENT            = 6,
    BLE_DISCONNECT_EVENT                   = 7,
    BLE_CONN_PARAM_UPDATE_REQ_EVENT        = 8,
    BLE_CONN_PARAM_UPDATE_FAILED_EVENT     = 9,
    BLE_CONN_PARAM_UPDATE_SUCCESSFUL_EVENT = 10,
    BLE_SET_RANDOM_BD_ADDR_EVENT           = 11,
    BLE_ADV_STARTED_EVENT                  = 12,
    BLE_ADV_STARTING_FAILED_EVENT          = 13,
    BLE_ADV_STOPPED_EVENT                  = 14,
    BLE_SCAN_STARTED_EVENT                 = 15,
    BLE_SCAN_DATA_REPORT_EVENT             = 16,
    BLE_SCAN_STARTING_FAILED_EVENT         = 17,
    BLE_SCAN_STOPPED_EVENT                 = 18,
    BLE_CREDIT_BASED_CONN_REQ_EVENT        = 19,
    BLE_RPA_ADDR_PARSED_EVENT              = 20,
    BLE_GET_TX_PWR_LEVEL                   = 21,
    BLE_TX_PWR_REPORT_EVENT                = 22,
    BLE_PATH_LOSS_REPORT_EVENT             = 23,
    BLE_SET_RAL_CMP_EVENT                  = 24,
    BLE_SUBRATE_CHANGE_EVENT               = 25,
    BLE_ENCRYPT_LTK_REPORT_EVENT           = 26,

    BLE_EVENT_NUM_MAX,
} ble_evnet_type_e;

/**
 * @brief The event type of other module
 *
 */
typedef enum{
    BLE_CALLBACK_RS_START = 0,
    BLE_CALLBACK_RS_COMPLETE,
    BLE_CALLBACK_ROLE_UPDATE,
    BLE_CALLBACK_IBRT_EVENT_ENTRY,

    BLE_CALLBACK_EVENT_NUM_MAX,
} ble_callback_evnet_type_e;

typedef struct {
    uint8_t conidx;
    ble_bdaddr_t peer_bdaddr;
} connect_handled_t;

typedef struct {
    uint8_t conidx;
    bool success;
    uint16_t reason;
} connect_bond_handled_t;

typedef struct {
    uint8_t conidx;
} connect_nc_exch_handled_t;

typedef struct {
    uint8_t conidx;
    uint8_t pairing_lvl;
    uint8_t addr_type;
    uint8_t addr[BTIF_BD_ADDR_SIZE];
} connect_encrypt_handled_t;

typedef struct {
    uint8_t peer_bdaddr[BTIF_BD_ADDR_SIZE];
} stopped_connecting_handled_t;

typedef struct {
    uint8_t actv_idx;
    uint8_t err_code;
    ble_bdaddr_t peer_bdaddr;
} connecting_failed_handled_t;

typedef struct {
    uint8_t conidx;
    uint8_t errCode;
    ble_bdaddr_t peer_bdaddr;
} disconnect_handled_t;

typedef struct {
    uint8_t conidx;
    /// Connection interval minimum
    uint16_t intv_min;
    /// Connection interval maximum
    uint16_t intv_max;
    /// Latency
    uint16_t latency;
    /// Supervision timeout
    uint16_t time_out;
} conn_param_update_req_handled_t;

typedef struct {
    uint8_t conidx;
    uint8_t err_code;
} conn_param_update_failed_handled_t;

typedef struct {
    uint8_t conidx;
    ///Connection interval value
    uint16_t con_interval;
    ///Connection latency value
    uint16_t con_latency;
    ///Supervision timeout
    uint16_t sup_to;
} conn_param_update_successful_handled_t;

typedef struct {
    uint8_t *new_bdaddr;
} set_random_bd_addr_handled_t;

typedef struct {
    uint8_t actv_user;
} adv_started_handled_t;

typedef struct {
    uint8_t actv_user;
    uint8_t err_code;
} adv_starting_failed_handled_t;

typedef struct {
    uint8_t actv_user;
} adv_stopped_handled_t;

typedef struct {
    /// Transmitter device address
    ble_bdaddr_t trans_addr;
    /// RSSI (between -127 and +20 dBm)
    int8_t rssi;
    /// Report data length
    uint16_t length;
    /// Report data
    uint8_t data[EXT_ADV_DATA_LEN];
} scan_data_report_handled_t;

typedef struct {
    uint8_t actv_idx;
    uint8_t err_code;
} scan_starting_failed_handled_t;

typedef struct {
    bool     isEnhanced;    // true: eatt 0x17; false: le 0x14
    uint16_t spsm;
    uint16_t mtu;
    uint16_t mps;
    uint16_t initial_credits;
} credit_based_conn_req_handled_t;

typedef struct {
    /// Connection index
    uint8_t conidx;
    //object (see enum #ble_tx_object_e)
    uint8_t object;
    /// PHY (see enum #ble_phy_pwr_value_e)
    uint8_t phy;
    /// Current transmit power level (dBm)
    int8_t tx_pwr;
    /// object is BLE_TX_LOCAL, data is max transmit power level (dBm)
    /// object is BLE_TX_REMOTE, data is flags,
    ///                          bit[0] minimum power level
    ///                          bit[1] maximum power level
    int8_t data;
} read_tx_pwr_handled_t;

typedef struct {
    /// Connection index
    uint8_t conidx;
    //object (see enum #ble_tx_target_e)
    uint8_t object;
    /// PHY (see enum #ble_phy_pwr_value_e)
    uint8_t phy;
    /// bit[0] - Sender is at the minimum supported power level
    /// bit[1] - Sender is at the maximum supported power level
    uint8_t flags;
    /// Transmit Power level (dBm)
    int8_t tx_pwr;
    /// Delta (dB)
    int8_t delta;
} tx_pwr_change_reporting_handled_t;

typedef struct {
    /// Connection index
    uint8_t conidx;
    //object (see enum #ble_tx_target_e)
    uint8_t curr_path_loss;
    /// curr_path_loss Units:dB
    uint8_t zone_entered;
    //0x00 low zone,0x01 middle zone,0x02 high zone
} path_loss_report_handled_t;

typedef struct {
    uint8_t status;
} path_set_ral_cmp_handled_t;

typedef struct {
    /// Connection index
    uint8_t conidx;
    /// status
    uint8_t status;
    /// Subrate Factor
    uint16_t sub_factor;
    /// Peripheral_Latency
    uint16_t per_latency;
    /// Continuation Number
    uint16_t cont_num;
    /// Supervision timeout (timeout * 10ms)
    uint16_t timeout;
} subrate_change_handled_t;

typedef struct {
    /// Connection index
    uint8_t conidx;
    /// LTK found flag
    uint8_t ltk_existed;
} le_conn_encrypt_ltk_handled_t;

typedef union {
    connect_handled_t connect_handled;
    connect_bond_handled_t connect_bond_handled;
    connect_nc_exch_handled_t connect_nc_exch_handled;
    connect_encrypt_handled_t connect_encrypt_handled;
    stopped_connecting_handled_t stopped_connecting_handled;
    connecting_failed_handled_t connecting_failed_handled;
    disconnect_handled_t disconnect_handled;
    conn_param_update_req_handled_t conn_param_update_req_handled;
    conn_param_update_failed_handled_t conn_param_update_failed_handled;
    conn_param_update_successful_handled_t conn_param_update_successful_handled;
    set_random_bd_addr_handled_t set_random_bd_addr_handled;
    adv_started_handled_t adv_started_handled;
    adv_starting_failed_handled_t adv_starting_failed_handled;
    adv_stopped_handled_t adv_stopped_handled;
    scan_data_report_handled_t scan_data_report_handled;
    scan_starting_failed_handled_t scan_starting_failed_handled;
    credit_based_conn_req_handled_t credit_based_conn_req_handled;
    read_tx_pwr_handled_t read_tx_power_handled;
    tx_pwr_change_reporting_handled_t tx_power_change_reporting_handled;
    path_loss_report_handled_t path_loss_report_handled;
    path_set_ral_cmp_handled_t set_ral_cmp_handled;
    subrate_change_handled_t subrate_change_handled;
    le_conn_encrypt_ltk_handled_t le_conn_encrypt_ltk_handled;
} ble_event_handled_t;

typedef struct {
    ble_evnet_type_e evt_type;
    ble_event_handled_t p;
} ble_event_t;

///HCI ISO data packets header structure
/// sync see@bes_hci_iso_hdr
struct bes_hci_iso_hdr
{
    /// Connection handle & Data Flags
    uint16_t conhdl_flags;
    /// ISO Data load length in number of bytes
    uint16_t iso_data_load_len;
};

typedef void (*APP_BLE_CORE_GLOBAL_HANDLER_FUNC)(ble_event_t *, void *);

typedef uint8_t (*set_rsp_dist_lk_bit_field_func)(void);

typedef void (*smp_identify_addr_exch_complete)(uint8_t *ia_addr);

void bes_ble_stack_init(void);

void bes_ble_ke_event_schedule(void);

void bes_ble_init_ble_name(const char *name);

int bes_ble_gap_no_operation_cmd_cmp_evt_handler(uint16_t opcode, void const *param);

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_CORE_COMMON_H__ */
