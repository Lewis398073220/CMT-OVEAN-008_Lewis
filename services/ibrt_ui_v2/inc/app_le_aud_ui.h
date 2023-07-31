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
#ifndef __APP_LE_AUD_UI_H__
#define __APP_LE_AUD_UI_H__

#if BLE_AUDIO_ENABLED

#include "ble_audio_core_evt_pkt.h"

#define BLE_RANDOM_BRADDR       (1)

typedef struct
{
    void (*ble_audio_adv_state_changed)(AOB_ADV_STATE_T state, uint8_t err_code);
    void (*mob_acl_state_changed)(uint8_t conidx, const ble_bdaddr_t* addr, AOB_ACL_STATE_T state, uint8_t errCode);
    void (*vol_changed_cb)(uint8_t con_lid, uint8_t volume, uint8_t mute);
    void (*vocs_offset_changed_cb)(int16_t offset, uint8_t output_lid);
    void (*vocs_bond_data_changed_cb)(uint8_t output_lid, uint8_t cli_cfg_bf);
    void (*media_track_change_cb)(uint8_t con_lid);
    void (*media_stream_status_change_cb)(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state);
    void (*media_playback_status_change_cb)(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state);
    void (*media_mic_state_cb)(uint8_t mute);
    void (*media_iso_link_quality_cb)(AOB_ISO_LINK_QUALITY_INFO_T param);
    void (*media_pacs_cccd_written_cb)(uint8_t con_lid);
    void (*call_state_change_cb)(uint8_t con_lid, void *param);
    void (*call_srv_signal_strength_value_ind_cb)(uint8_t con_lid, uint8_t value);
    void (*call_status_flags_ind_cb)(uint8_t con_lid, bool inband_ring, bool silent_mode);
    void (*call_ccp_opt_supported_opcode_ind_cb)(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported);
    void (*call_terminate_reason_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t reason);
    void (*call_incoming_number_inf_ind_cb)(uint8_t con_lid, uint8_t url_len, uint8_t *url);
    void (*call_svc_changed_ind_cb)(uint8_t con_lid);
    void (*call_action_result_ind_cb)(uint8_t con_lid, void *param);
}app_ble_audio_event_cb_t;

void app_ui_register_custom_ui_le_aud_callback(app_ble_audio_event_cb_t* cb);

void app_ui_notify_bt_nv_recored_changed(bt_bdaddr_t* mobile_addr);

bool app_ui_start_ble_connecteable_adv(uint32_t adv_duration, ble_bdaddr_t *bt_address);

bool app_ui_bt_trigger_direct_connectable_adv(const bt_bdaddr_t *p_addr);

#endif

#endif /* APP_LE_AUD_UI_H */