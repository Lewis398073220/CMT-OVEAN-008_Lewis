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
#ifndef __APP_UI_API_H__
#define __APP_UI_API_H__

#include "app_tws_ibrt.h"
#include "app_ui_evt.h"
#include "app_tws_ibrt_core_type.h"

/* BES UI Version */
#define BES_UI_MAJOR (2)
#define BES_UI_MINOR (9)

#define BLE_AUDIO_ADV_DURATION (60000)
#define ADV_FOREVER (0)

typedef struct {
    void (*ibrt_global_state_changed)(ibrt_global_state_change_event *state);
    void (*ibrt_a2dp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_a2dp_state_change *state);
    void (*ibrt_hfp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_hfp_state_change *state);
    void (*ibrt_avrcp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_avrcp_state_change *state);
    void (*ibrt_tws_pairing_changed)(ibrt_conn_pairing_state state, uint8_t reason_code);
    void (*ibrt_tws_acl_state_changed)(ibrt_conn_tws_conn_state_event *state, uint8_t reason_code);
    void (*ibrt_mobile_acl_state_changed)(const bt_bdaddr_t *addr, ibrt_mobile_conn_state_event *state, uint8_t reason_code);
    void (*ibrt_sco_state_changed)(const bt_bdaddr_t *addr, ibrt_sco_conn_state_event *state, uint8_t reason_code);
    void (*ibrt_tws_role_switch_status_ind)(const bt_bdaddr_t *addr, ibrt_conn_role_change_state state, ibrt_role_e role);
    void (*ibrt_ibrt_state_changed)(const bt_bdaddr_t *addr, ibrt_connection_state_event *state, ibrt_role_e role, uint8_t reason_code);
    void (*ibrt_access_mode_changed)(btif_accessible_mode_t newAccessMode);
    /* connection request, return false to reject */
    bool (*incoming_conn_req_callback)(const bt_bdaddr_t *addr);
    /* connection request, return false to reject */
    bool (*extra_incoming_conn_req_callback)(const bt_bdaddr_t *addr);
    void (*ibrt_case_event_complete_callback)(app_ui_evt_t box_evt);
    bool (*ibrt_custom_disallow_start_reconnect)(const bt_bdaddr_t *addr, const uint16_t active_event);
    void (*ibrt_notify_custom_start_reconnect_mobile)();
    bool (*ibrt_custom_need_tws_connection)(void);//true:yes, false:no
    void (*ibrt_notify_tws_role_switch_done)(TWS_UI_ROLE_E role);
} ibrt_link_status_changed_cb_t;

typedef struct {
    void (*ibrt_pairing_mode_entry_func)();
    void (*ibrt_pairing_mode_exit_func)();
} ibrt_pairing_mode_changed_cb_t;

typedef struct {
    void (*ibrt_switch_ui_role_complete_callback)(TWS_UI_ROLE_E current_role, uint8_t errCode);
} ibrt_sw_ui_role_complete_cb_t;

typedef void (*ibrt_vender_event_handler_ind)(uint8_t, uint8_t *, uint8_t);

typedef enum {
    // dont disc any mob
    IBRT_PAIRING_DISC_NONE = 0,
    // disc all already connected mob when enter pairing
    IBRT_PAIRING_DISC_ALL_MOB,
    // support multipoint & current already exist two link, but only disc one when enter pairing
    IBRT_PAIRING_DISC_ONE_MOB,
} ibrt_pairing_with_disc_num;


typedef struct
{
    uint32_t rx_seq_error_timeout;
    uint32_t rx_seq_error_threshold;
    uint32_t rx_seq_recover_wait_timeout;
    uint32_t rssi_monitor_timeout;
    uint32_t tws_conn_failed_wait_time;
    uint32_t connect_no_03_timeout;
    uint32_t disconnect_no_05_timeout;
    uint32_t tws_cmd_send_timeout;
    uint32_t tws_cmd_send_counter_threshold;

    /// freeman mode config, default should be false
    bool freeman_enable;
    /// enable sdk lea adv strategy, default should be true
    bool sdk_lea_adv_enable;
    /// tws earphone set the same addr, UI will be flexible, default should be true
    bool tws_use_same_addr;
    /// pairing mode timeout value config
    uint32_t pairing_timeout_value;
    /// SDK pairing enable, the default should be true
    bool sdk_pairing_enable;
    /// passive enter pairing when no mobile record, the default should be false
    bool enter_pairing_on_empty_record;
    /// passive enter pairing when reconnect failed, the default should be false
    bool enter_pairing_on_reconnect_mobile_failed;
    /// passive enter pairing when mobile disconnect, the default should be false
    bool enter_pairing_on_mobile_disconnect;
    /// passive enter pairing when ssp fail
    bool enter_pairing_on_ssp_fail;
    /// exit pairing when peer close, the default should be true
    bool exit_pairing_on_peer_close;
    /// exit pairing when a2dp or hfp streaming, the default should be true
    bool exit_pairing_on_streaming;
    /// disconnect remote devices when entering pairing mode actively
    uint8_t paring_with_disc_mob_num;
    /// disconnect SCO or not when accepting phone connection in pairing mode
    bool pairing_with_disc_sco;
    /// not start ibrt in pairing mode, the default should be false
    bool pairing_without_start_ibrt;
    ///no need to reconnect mobile when easbuds in busy mode
    bool disallow_reconnect_in_streaming_state;
    /// do tws switch when RSII value change, default should be true
    bool tws_switch_according_to_rssi_value;
    /// controller basband monitor
    bool lowlayer_monitor_enable;
    bool support_steal_connection;
    bool check_plugin_excute_closedbox_event;
    //if add ai feature
    bool ibrt_with_ai;
    bool giveup_reconn_when_peer_unpaired;
    bool no_profile_stop_ibrt;

    bool tws_switch_tx_data_protect;

    bool without_reconnect_when_fetch_out_wear_up;
    /// do tws switch when rssi value change over threshold
    uint8_t rssi_threshold;
    /// do tws switch when RSII value change, timer threshold
    uint8_t role_switch_timer_threshold;
    uint8_t audio_sync_mismatch_resume_version;

    uint8_t  profile_concurrency_supported;
    uint8_t  connect_new_mobile_enable;

    /// close box debounce time config
    uint16_t close_box_event_wait_response_timeout;
    /// reconnect event internal config wait timer when tws disconnect
    uint16_t reconnect_wait_ready_timeout;
    uint16_t reconnect_mobile_wait_ready_timeout;
    uint16_t reconnect_tws_wait_ready_timeout;

    /// wait time before launch reconnect event
    uint16_t reconnect_mobile_wait_response_timeout;
    uint16_t reconnect_ibrt_wait_response_timeout;
    uint16_t nv_master_reconnect_tws_wait_response_timeout;
    uint16_t nv_slave_reconnect_tws_wait_response_timeout;

    /// open box reconnect mobile times config
    uint16_t open_reconnect_mobile_max_times;
    /// open box reconnect tws times config
    uint16_t open_reconnect_tws_max_times;
    /// connection timeout reconnect mobile times config
    uint16_t reconnect_mobile_max_times;
    /// connection timeout reconnect tws times config
    uint16_t reconnect_tws_max_times;

    /// connection timeout reconnect ibrt times config
    uint16_t reconnect_ibrt_max_times;
    uint16_t mobile_page_timeout;

    /// tws connection supervision timeout
    uint16_t tws_connection_timeout;
    uint16_t wait_time_before_disc_tws;

    uint16_t radical_scan_interval_nv_slave;
    uint16_t radical_scan_interval_nv_master;

    uint16_t scan_interval_in_sco_tws_disconnected;
    uint16_t scan_window_in_sco_tws_disconnected;

    uint16_t scan_interval_in_sco_tws_connected;
    uint16_t scan_window_in_sco_tws_connected;

    uint16_t scan_interval_in_a2dp_tws_disconnected;
    uint16_t scan_window_in_a2dp_tws_disconnected;

    uint16_t scan_interval_in_a2dp_tws_connected;
    uint16_t scan_window_in_a2dp_tws_connected;

    bool support_steal_connection_in_sco;
    bool support_steal_connection_in_a2dp_steaming;
    bool allow_sniff_in_sco;
    bool always_interlaced_scan;
    uint8_t llmonitor_report_format;
    uint32_t llmonitor_report_count;

    bool is_changed_to_ui_master_on_tws_disconnected;
    /// if tws&mobile disc, reconnect tws first until tws connected then reconn mobile
    bool delay_reconn_mob_until_tws_connected;
    uint8_t delay_reconn_mob_max_times;

    //change ui role follow nv role if all mob disc
    bool change_to_nv_role_when_all_mob_disconnected;

    //To save energy, disable pscan when all mobile in nv connected
    bool disable_pscan_when_all_nvmob_connected;
} app_ui_config_t;

void app_ui_init();

app_ui_config_t* app_ui_get_config();

ibrt_pairing_mode_changed_cb_t *app_ui_get_pairing_changed_cb();

void app_ui_custom_role_switch_cb_ind(const bt_bdaddr_t *addr, ibrt_conn_role_change_state state, ibrt_role_e role);

int app_ui_reconfig_env(app_ui_config_t *config);

void app_ui_event_entry(app_ui_evt_t evt);

void app_ui_dump_status();

void app_ui_monitor_dump(void);

void app_ui_sdp_init(void);

void app_ui_handle_vender_event(uint8_t evt_type, uint8_t * buffer, uint32_t length);

bud_box_state app_ui_get_local_box_state(void);

bud_box_state app_ui_get_peer_box_state(void);

void app_ui_update_scan_type_policy(ibrt_update_scan_evt_type trigger_evt_type);

void app_ui_send_tws_reconnect_event(app_ui_evt_t reconect_event);

void app_ui_send_mobile_reconnect_event(uint8_t link_id);

void app_ui_send_mobile_reconnect_event_by_addr(const bt_bdaddr_t* mobile_addr);

void app_ui_choice_mobile_connect(const bt_bdaddr_t* mobile_addr);

uint8_t app_ui_get_connected_remote_dev_count();

bool app_ui_any_mobile_device_connected(void);

bool app_ui_max_mobile_device_connected(void);

bool app_ui_mobile_device_connected_with_valid_device(void);

bool app_ui_is_accepting_device(const bt_bdaddr_t *addr);

void app_ui_clr_pending_incoming_conn_req(void);

bool app_ui_accept_pending_incoming_conn_req(bt_bdaddr_t *addr);

void app_ui_register_custom_ui_callback(ibrt_link_status_changed_cb_t* custom_ui_cb);

void app_ui_register_pairing_mode_changed_callback(ibrt_pairing_mode_changed_cb_t *cbs);

void app_ui_register_sw_ui_role_complete_callback(ibrt_sw_ui_role_complete_cb_t *cbs);

void app_ui_register_vender_event_update_ind(ibrt_vender_event_handler_ind handler);

bool app_ui_is_addr_null(bt_bdaddr_t *addr);

bool app_ui_pop_from_pending_list(bt_bdaddr_t *addr);

bool app_ui_get_from_pending_list(bt_bdaddr_t *addr);

void app_ui_push_device_to_pending_list(bt_bdaddr_t *addr);

uint16_t app_ui_pending_conn_req_list_size(void);

bool app_ui_event_has_been_queued(const bt_bdaddr_t* remote_addr,app_ui_evt_t event);

app_ui_evt_t app_ui_get_active_event(const bt_bdaddr_t* remote_addr);

bool app_ui_high_priority_event_interrupt_reconnec();

bool app_ui_disallow_reconnect_mobile_by_peer_status(void);

void app_ui_send_mobile_disconnect_event(const bt_bdaddr_t* mobile_addr);

bool app_ui_notify_peer_to_destroy_device(const bt_bdaddr_t *addr, bool delete_record);

bool app_ui_destroy_device_ongoing(void);

void app_ui_destroy_device(const bt_bdaddr_t *del_nv_addr, bool delete_record);

void app_ui_destroy_the_other_device(const bt_bdaddr_t *active_addr, bool need_delete_nv);

/**
 ****************************************************************************************
 * @brief To disconnect all connections and then shutdown the system
 *
 ****************************************************************************************
 */
void app_ui_shutdown(void);

void app_ui_set_enter_pairing_flag(void);

void app_ui_clr_enter_pairing_flag(void);

bool app_ui_is_enter_pairing_mode(void);

void app_ui_pairing_ctl_init(void);

/**
 ****************************************************************************************
 * @brief To get current pairing mode state
 *
 * @return true                     In pairing mode
 * @return false                    Not in pairing mode
 ****************************************************************************************
 */
bool app_ui_in_pairing_mode(void);

/**
 ****************************************************************************************
 * @brief Enter pairing mode
 *
 * @param timeout                   Pairing timeout value
 * @param notify_peer               If true, notify peer enter pairing
 ****************************************************************************************
 */
void app_ui_enter_pairing_mode(uint32_t timeout, bool notify_peer);

/**
 ****************************************************************************************
 * @brief Exit pairing mode
 *
 * @param notify_peer               If true, notify peer exit pairing
 ****************************************************************************************
 */
void app_ui_exit_pairing_mode(bool notify_peer);

void app_ui_pairing_evt_handler(pairing_evt_e evt, uint32_t para);

bool app_ui_need_disconnect_mob_on_pairing(const bt_bdaddr_t *addr);

bool app_ui_allow_start_ibrt_in_pairing(void);

bool app_ui_in_tws_mode(void);

bool app_ui_support_bleaudio(void);

bool app_ui_support_multipoint(void);

/**
 ****************************************************************************************
 * @brief role switch > ibrt role-switch
 *
 * @param[in] switch2master         if true will switch to master else to slave
 ****************************************************************************************
 */
bool app_ui_user_role_switch(bool switch2master);

/**
****************************************************************************************
* @brief user get current ui role
*
****************************************************************************************
*/
TWS_UI_ROLE_E app_ui_get_current_role(void);

/**
****************************************************************************************
* @brief user get is any connected device doing role switch
*
****************************************************************************************
*/
bool app_ui_role_switch_ongoing(void);

/**
 ****************************************************************************************
 * @brief trigger role switch by local and peer box state
 *
 ****************************************************************************************
 */
void app_ui_trigger_role_switch_by_box_state(void);

/**
 ****************************************************************************************
 * @brief change ui mode
 *
 * @param[in] enable_bleaudio       enable le audio feature
 * @param[in] enable_multipoint     enable multipoint feature
 * @param[in] addr                  active device which will be remained
 ****************************************************************************************
 */
bool app_ui_change_mode(bool enable_bleaudio, bool enable_multipoint, const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief change ui mode extend, only ui master can change ui mode and send to peer side
 *
 * @param[in] enable_bleaudio       enable le audio feature
 * @param[in] enable_multipoint     enable multipoint feature
 * @param[in] addr                  active device which will be remained
 ****************************************************************************************
 */
bool app_ui_change_mode_ext(bool enable_leaudio, bool enable_multipoint, const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief enable/disable page function
 *
 * @param enable                    If true, enable page; else, disable page
 ****************************************************************************************
 */
void app_ui_set_page_enabled(bool enable);

/**
 ****************************************************************************************
 * @brief Get current enable or disable page
 *
 * @return true                     Enable page
 * @return false                    Disable Page
 ****************************************************************************************
 */
bool app_ui_enabled_page(void);

void app_ui_send_box_msg(app_ui_evt_t box_evt);

bool app_ui_need_delay_mob_reconn();

bool app_ui_custom_disallow_reconn_dev(const bt_bdaddr_t *addr, uint16_t active_event);
void app_ui_notify_custom_start_reconnect_mobile();

/**
 ****************************************************************************************
 * @brief exit eabud mode for enter other mode
 *
 * @return None
 ****************************************************************************************
 */
void app_ui_exit_earbud_mode(void);

#if BLE_AUDIO_ENABLED
/**
 ****************************************************************************************
 * @brief User start lea adv, if sdk_lea_adv_enable config set false
 *
 * @param[in] duration              Lea adv duration
 * @param[in] remote                Null for general adv, otherwise, directed adv
 * @param[in] notify_peer           Notify peer to start lea adv
 * @return true                     Start adv success
 * @return false                    Start adv failed
 ****************************************************************************************
 */
bool app_ui_user_start_lea_adv(uint32_t duration, BLE_BDADDR_T *remote, bool notify_peer);

/**
 ****************************************************************************************
 * @brief User stop lea adv, if sdk_lea_adv_enable config set false
 *
 * @param[in] notify_peer           Notify peer to stop lea adv
 ****************************************************************************************
 */
void app_ui_user_stop_lea_adv(bool notify_peer);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 ****************************************************************************************
 */
bool app_ui_any_ble_audio_links(void);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 * @param[in] conidx                The LE connection index
 ****************************************************************************************
 */
void app_ui_keep_only_the_leaudio_device(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Return true if any ble audio device connected
 *
 * @param[in] conidx                The LE connection index
 ****************************************************************************************
 */
void app_ui_stop_ble_connecteable_adv(void);
#endif

#endif /* __APP_UI_API_H__ */
