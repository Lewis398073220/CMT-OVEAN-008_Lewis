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
#ifndef APP_H_
#define APP_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration
#include "app_ble_mode_switch.h"
#include "app_ble_param_config.h"
#include "ble_core_common.h"

#ifdef  BLE_APP_PRESENT

#include <stdint.h>          // Standard Integer Definition
//#include <co_bt.h>           // Common BT Definitions
#include "arch.h"            // Platform Definitions
#include "gapc_le_msg.h"       // GAPC Definitions
#include "gapm_msg.h"
#include "gap.h"

#if (NVDS_SUPPORT)
#include "nvds.h"
#endif // (NVDS_SUPPORT)

#if defined (IO_CAPABILITY_NO_INPUT_NO_OUTPUT_MITM_FALSE)
/// Disable MITM bit
#define AUTH_LEVLE_MITM_CTRL_BIT        (GAP_AUTH_REQ_MASK - GAP_AUTH_MITM)
#else
#define AUTH_LEVLE_MITM_CTRL_BIT        (GAP_AUTH_REQ_MASK)
#endif

#if defined (CFG_APP_SEC)
#if defined (CFG_SEC_CON)
#define BLE_AUTHENTICATION_LEVEL        (GAP_AUTH_REQ_SEC_CON_BOND & AUTH_LEVLE_MITM_CTRL_BIT)
#else // !defined (CFG_SEC_CON)
#define BLE_AUTHENTICATION_LEVEL        (GAP_AUTH_REQ_MITM_BOND & AUTH_LEVLE_MITM_CTRL_BIT)
#endif
#else // !defined (CFG_APP_SEC)
#define BLE_AUTHENTICATION_LEVEL        (GAP_AUTH_REQ_NO_MITM_NO_BOND & AUTH_LEVLE_MITM_CTRL_BIT)
#endif
/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximal length of the Device Name value
#define APP_DEVICE_NAME_MAX_LEN      (32)

// Advertising mode
#define APP_FAST_ADV_MODE   (1)
#define APP_SLOW_ADV_MODE   (2)
#define APP_STOP_ADV_MODE   (3)
#define APP_MAX_TX_OCTETS   251
#define APP_MAX_TX_TIME     2120

#define INVALID_BLE_ACTIVITY_INDEX  0xFF
#define INVALID_BLE_CONIDX          0xFF

#define BLE_CONN_PARAM_SLAVE_LATENCY_CNT        0
#define BLE_CONN_PARAM_SUPERVISE_TIMEOUT_MS     6000

/*
 * MACROS
 ****************************************************************************************
 */

#define APP_HANDLERS(subtask)    {&subtask##_msg_handler_list[0], ARRAY_LEN(subtask##_msg_handler_list)}

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Activity state machine
enum app_actv_state
{
    /// Activity does not exists
    APP_ACTV_STATE_IDLE = 0,

    /// Creating advertising activity
    APP_ADV_STATE_CREATING,
    /// Setting advertising data
    APP_ADV_STATE_SETTING_ADV_DATA,
    /// Setting scan response data
    APP_ADV_STATE_SETTING_SCAN_RSP_DATA,

    /// Starting advertising activity
    APP_ADV_STATE_STARTING,
    /// Advertising activity started
    APP_ADV_STATE_STARTED,
    /// Stopping advertising activity
    APP_ADV_STATE_STOPPING,
    /// Deleting advertising activity
    APP_ADV_STATE_DELETING,

    /// Creating scanning activity
    APP_SCAN_STATE_CREATING,        //8
    /// Starting scanning activity
    APP_SCAN_STATE_STARTING,
    /// Scanning activity started
    APP_SCAN_STATE_STARTED,
    /// Stopping scanning activity
    APP_SCAN_STATE_STOPPING,
    /// Deleting scanning activity
    APP_SCAN_STATE_DELETING,

    /// Creating connecting activity
    APP_CONNECT_STATE_CREATING,     //13
    /// Starting connecting activity
    APP_CONNECT_STATE_STARTING,
    /// Connecting activity started
    APP_CONNECT_STATE_STARTED,
    /// Stopping connecting activity
    APP_CONNECT_STATE_STOPPING,
    /// Deleting connecting activity
    APP_CONNECT_STATE_DELETING,

};


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Structure containing information about the handlers for an application subtask
struct app_subtask_handlers
{
    /// Pointer to the message handler table
    const struct ke_msg_handler *p_msg_handler_tab;
    /// Number of messages handled
    uint16_t msg_cnt;
};

typedef enum
{
    BLE_CONN_PARAM_PRIORITY_NORMAL = 0,
    BLE_CONN_PARAM_PRIORITY_ABOVE_NORMAL0,
    BLE_CONN_PARAM_PRIORITY_ABOVE_NORMAL1,
    BLE_CONN_PARAM_PRIORITY_ABOVE_NORMAL2,
    BLE_CONN_PARAM_PRIORITY_HIGH,
} BLE_CONN_PARAM_PRIORITY_E;

typedef enum{
    BLE_TX_PWR_LEVEL_1M          = 0,
    BLE_TX_PWR_LEVEL_2M          = 1,
    BLE_TX_PWR_LEVEL_LE_CODED_S8 = 2,
    BLE_TX_PWR_LEVEL_LE_CODED_S2 = 3,
}ble_phy_pwr_value_e;

typedef enum{
    BLE_TX_LOCAL     = 0,
    BLE_TX_REMOTE    = 1,
}ble_tx_object_e;

typedef struct
{
    uint8_t     ble_conn_param_mode;
    uint8_t     priority;
    uint16_t    conn_interval_min;    // in the unit of 1.25ms
    uint16_t    conn_interval_max;    // in the unit of 1.25ms
    uint16_t    conn_slave_latency_cnt;
} BLE_CONN_PARAM_CONFIG_T;

/// Application environment structure
typedef struct {
    /// Connection handle
    uint16_t conhdl;
    BLE_CONNECT_STATE_E connectStatus;
    uint8_t isFeatureExchanged;
    /// Bonding status
    uint8_t bonded;
    uint8_t peerAddrType;
    uint8_t isBdAddrResolvingInProgress;
    uint8_t isGotSolvedBdAddr;
    uint8_t bdAddr[BD_ADDR_LEN];
    uint8_t solvedBdAddr[BD_ADDR_LEN];
    /// LE connection param in use
    APP_BLE_CONN_PARAM_T connParam;
    /// LE connection interval nego ongoing
    APP_BLE_UPDATE_CI_T ongoingCI;
    /// LE connection interval pending
    APP_BLE_UPDATE_CI_T pendingCI;
    osTimerId           updateCITimerId;
    uint8_t addr_resolv_supp;
    /// ble connection param update times
    uint8_t conn_param_update_times;
    uint8_t supportRpaOnly;
} APP_BLE_CONN_CONTEXT_T;

/// Application environment structure
struct app_env_tag
{
    uint8_t conn_cnt;

    enum app_actv_state state[BLE_ACTIVITY_MAX];

    /// Advertising activity index
    uint8_t adv_actv_idx[BLE_ADV_ACTIVITY_USER_NUM];
    /// Scanning activity index
    uint8_t scan_actv_idx;
    /// Connecting activity index
    uint8_t connect_actv_idx;

    bool need_restart_adv;
    bool need_update_adv;
    bool need_set_rsp_data;
    BLE_ADV_PARAM_T advParam;

    bool need_restart_scan;
    uint16_t scanIntervalInMs;
    uint16_t scanWindowInMs;
    uint32_t scanFiltPolicy;

    bool need_restart_connect;
    BLE_INIT_PARAM_T ble_init_param;

    bool need_set_white_list;
    bool setting_white_list;
    uint8_t white_list_size;
    gap_bdaddr_t white_list_addr[8];

    bool is_resolving_not_empty;

    bool isNeedToAddDevicesToReslovingList;
    bool addingDevicesToReslovingList;
    uint8_t numberOfDevicesAddedToResolvingList;
    gap_ral_dev_info_t devicesInfoAddedToResolvingList[RESOLVING_LIST_MAX_NUM];

    /// Last initialized profile
    uint8_t next_svc;

    /// Device Name length
    uint8_t dev_name_len;
    /// Device Name
    uint8_t dev_name[APP_DEVICE_NAME_MAX_LEN];

    /// Local device IRK
    uint8_t loc_irk[KEY_LEN];

    APP_BLE_CONN_CONTEXT_T context[BLE_CONNECTION_MAX];

};

// max adv data length is 31, and 3 byte is used for adv type flag(0x01)
#define ADV_DATA_MAX_LEN                            (28)

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */

/// Application environment
extern struct app_env_tag app_env;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize the BLE demo application.
 ****************************************************************************************
 */
void appm_init(void);

/**
 ****************************************************************************************
 * @brief Add a required service in the database
 ****************************************************************************************
 */
bool appm_add_svc(void);

/**
 ****************************************************************************************
 * @brief Add multiple devices to the resolving list in Controller in order that the device address which Host gets from Controller is Identity Address.
 ****************************************************************************************
 */
void appm_add_multiple_devices_to_resolving_list_in_controller(gap_ral_dev_info_t *devicesResolvingLists, uint8_t size);

/**
 ****************************************************************************************
 * @brief prepare the devices info that will be added to resolving list in Controller.
 ****************************************************************************************
 */
uint8_t appm_prepare_devices_info_added_to_resolving_list(gap_ral_dev_info_t *devicesInfo);

/**
 * @brief Add the ral to controller, no thread call
 *
 * @param ral
 * @param size
 */
void appm_set_rpa_list(gap_ral_dev_info_t *ral, uint8_t size);
/**
 ****************************************************************************************
 * @brief Add the bdaddr to white list, no thread call
 ****************************************************************************************
 */
void appm_set_white_list(gap_bdaddr_t *bdaddr, uint8_t size);

/**
 * @brief Get all paired addr list
 *
 * @param paired_addr_list
 * @return uint8_t
 */
uint8_t appm_get_all_paired_dev_addr_from_nv(ble_bdaddr_t *paired_addr_list);

/**
 ****************************************************************************************
 * @brief Put the device in general discoverable and connectable mode
 ****************************************************************************************
 */
void appm_start_advertising(void *param);

/**
 ****************************************************************************************
 * @brief
 ****************************************************************************************
 */
void appm_actv_fsm_next(uint8_t actv_idx, uint8_t status);

/**
 ****************************************************************************************
 * @brief Put the device in non discoverable and non connectable mode
 ****************************************************************************************
 */
void appm_stop_advertising(uint8_t actv_idx);

/**
 ****************************************************************************************
 * @brief Send to request to update the connection parameters
 ****************************************************************************************
 */
void appm_update_param(uint8_t conidx, uint32_t min_interval,
    uint32_t max_interval, uint32_t time_out, uint8_t  slaveLatency);

/**
 ****************************************************************************************
 * @brief Send a disconnection request
 ****************************************************************************************
 */
void appm_disconnect(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Retrieve device name
 *
 * @param[out] device name
 * @param[in] offset
 *
 * @return name length
 ****************************************************************************************
 */
int8_t appm_get_dev_name(uint8_t* name, uint16_t offset);

uint8_t appm_is_connected();

/**
 ****************************************************************************************
 * @brief Return if the device is currently bonded
 ****************************************************************************************
 */
bool app_sec_get_bond_status(void);

/*---------------------------------------------------------------------------
 *            app_ble_connected_evt_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    callback function of BLE connected event
 *
 * Parameters:
 *    conidx - connection index
 *    pPeerBdAddress - connected BLE device address
 *
 * Return:
 *    void
 */
void app_ble_connected_evt_handler(uint8_t conidx, gap_bdaddr_t *pPeerBdAddress);


/**
 ****************************************************************************************
 * @brief delete advertising
 *
 * @param[in] none
 ****************************************************************************************
 */

void appm_delete_activity(uint8_t actv_idx);

void appm_update_actv_state(uint8_t actv_idx, enum app_actv_state newState);

/*---------------------------------------------------------------------------
 *            app_ble_disconnected_evt_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    ble disconnect event received callback
 *
 * Parameters:
 *    conidx - connection index
 *
 * Return:
 *    void
 */
void app_ble_disconnected_evt_handler(uint8_t conidx, uint8_t errCode);

void app_connecting_stopped(gap_bdaddr_t *peer_addr);

bool appm_check_adv_activity_index(uint8_t actv_idx);

void l2cap_update_param_in_standard_unit(uint8_t conidx,
    uint32_t min_interval, uint32_t max_interval,
    uint32_t supTimeout_in_ms, uint8_t  slaveLatency);

void l2cap_update_param(uint8_t  conidx,
    uint32_t min_interval_in_ms, uint32_t max_interval_in_ms,
    uint32_t supTimeout_in_ms, uint8_t  slaveLatency);

#ifdef CFG_LE_PWR_CTRL
void appm_set_path_loss_rep_param_cmd(uint8_t conidx, uint8_t enable, uint8_t high_threshold,
                                        uint8_t high_hysteresis, uint8_t low_threshold,
                                        uint8_t low_hysteresis, uint8_t min_time);
#endif

void appm_start_connecting(BLE_INIT_PARAM_T *init_param);

void appm_stop_connecting(void);

void appm_start_scanning(uint16_t intervalInMs, uint16_t windowInMs, uint32_t filtPolicy);

void appm_stop_scanning(void);

void appm_create_advertising(void);

void appm_create_connecting(void);

void app_advertising_stopped(uint8_t actv_idx);

void app_advertising_starting_failed(uint8_t actv_idx, uint8_t err_code);

void app_scanning_stopped(void);

void app_scanning_starting_failed(uint8_t actv_idx, uint8_t err_code);

void app_connecting_failed(ble_bdaddr_t *peer_addr, uint8_t err_code);

void app_ble_update_param_failed(uint8_t conidx, uint8_t errCode);

void app_ble_update_param_successful(uint8_t conidx, APP_BLE_CONN_PARAM_T* pConnParam);

void appm_exchange_mtu(uint8_t conidx);

void app_ble_system_ready(void);

void app_ble_add_devices_info_to_resolving(void);

void app_ble_add_dev_to_rpa_list_in_controller(uint8_t peer_addr[],const uint8_t irk[]);

void appm_set_private_bd_addr(uint8_t* bdAddr);

void appm_add_dev_into_whitelist(struct gap_bdaddr* ptBdAddr);

void app_scanning_started(void);

void app_advertising_started(uint8_t actv_idx);

void app_connecting_started(void);

bool appm_resolve_random_ble_addr_from_nv(uint8_t conidx, uint8_t* randomAddr);

void appm_resolve_random_ble_addr_with_sepcific_irk(uint8_t conidx, uint8_t* randomAddr, uint8_t* pIrk);

void appm_random_ble_addr_solved(bool isSolvedSuccessfully, uint8_t* irkUsedForSolving);

void appm_le_set_rpa_timeout(uint16_t rpa_timeout);

uint8_t app_ble_get_actv_state(uint8_t actv_idx);

uint8_t app_ble_get_activity_user_adv_state(BLE_ADV_ACTIVITY_USER_E actv_user);

uint8_t app_ble_connection_count(void);

bool app_is_arrive_at_max_ble_connections(void);

bool app_is_resolving_ble_bd_addr(void);

void app_enter_fastpairing_mode(void);

bool app_is_in_fastpairing_mode(void);

void app_set_in_fastpairing_mode_flag(bool isEnabled);

uint16_t appm_get_conhdl_from_conidx(uint8_t conidx);

uint8_t appm_get_conidx_from_conhdl(uint16_t conhdl);;

void appm_check_and_resolve_ble_address(uint8_t conidx);

uint8_t* appm_get_current_ble_addr(void);

void app_trigger_ble_service_discovery(uint8_t conidx, uint16_t shl, uint16_t ehl);

uint8_t* appm_get_local_identity_ble_addr(void);

void app_exchange_remote_feature(uint8_t conidx);

void app_ble_update_conn_param_mode_of_specific_connection(uint8_t conidx, BLE_CONN_PARAM_MODE_E mode, bool isEnable);

void app_ble_reset_conn_param_mode_of_specifc_connection(uint8_t conidx);

void app_ble_update_conn_param_mode(BLE_CONN_PARAM_MODE_E mode, bool isEnable);

void app_ble_reset_conn_param_mode(uint8_t conidx);

void appm_refresh_ble_irk(void);

bool app_ble_get_conn_param(uint8_t conidx,  APP_BLE_CONN_PARAM_T* pConnParam);

void appm_update_adv_data(void *param);

bool gattc_check_if_notification_processing_is_busy(uint8_t conidx);

#ifdef GFPS_ENABLED
void fp_update_ble_connect_param_start(uint8_t ble_conidx);
void fp_update_ble_connect_param_stop(uint8_t ble_conidx);
#endif

bool app_ble_is_connection_on_by_addr(uint8_t *addr);

void app_ble_set_white_list_complete(void);

void app_ble_set_resolving_list_complete(uint16_t status);

uint8_t* app_ble_get_peer_addr(uint8_t conidx);

bool app_ble_get_peer_solved_addr(uint8_t conidx, uint8_t** p_addr);

uint8_t app_ble_get_peer_solved_addr_type(uint8_t conidx);

void app_ble_rpa_addr_parsed_success(uint8_t conidx);

bool app_ble_is_remote_mobile_connected(uint8_t *p_addr);

bool app_ble_is_connection_on_by_index(uint8_t conidx);

void app_init_ble_name(const char *name);

void app_ble_on_bond_status_changed(uint8_t conidx,bool success,uint16_t reason);

void app_ble_on_bond_failed(uint8_t conidx);

void app_ble_on_encrypt_success(uint8_t conidx, uint8_t pairing_lvl);

uint8_t *appm_get_current_ble_irk(void);

bool app_ble_is_resolving_list_not_empty(void);

void appm_get_tx_power(uint8_t conidx, ble_tx_object_e object, ble_phy_pwr_value_e phy);

void appm_tx_power_report_enable(uint8_t conidx, bool local_enable, bool remote_enable);

void app_ble_set_rpa_only(uint8_t conidx, uint8_t rpaOnly);

void app_ble_set_resolv_support(uint8_t conidx, uint8_t support);

void appm_subrate_interface(uint8_t conidx, uint16_t subrate_min, uint16_t subrate_max,
                                      uint16_t latency_max, uint16_t cont_num, uint16_t timeout);

int8_t appm_ble_get_rssi(uint8_t conidx);

int hci_no_operation_cmd_cmp_evt_handler(uint16_t opcode, void const *param);

void appm_read_rpa_addr(void);

uint8_t* appm_get_local_rpa_addr(void);

uint8_t* appm_get_local_identity_ble_addr(void);


#ifdef __cplusplus
}
#endif
/// @} APP

#endif //(BLE_APP_PRESENT)

#endif // APP_H_
