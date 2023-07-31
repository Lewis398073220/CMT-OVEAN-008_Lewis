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
#ifndef __APP_HFP_H__
#define __APP_HFP_H__
#include "bluetooth.h"
#include "btapp.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef BT_HFP_SUPPORT

#define HF_VOICE_DISABLE  0
#define HF_VOICE_ENABLE   1

#define HF_SENDBUFF_SIZE (320)
#define HF_SENDBUFF_MEMPOOL_NUM (2)

#define MAX_HFP_VOL    (15)
#define APP_BT_HFP_GET_CLCC_TIMEOUT 1500
struct hf_sendbuff_control {
    struct {
        btif_bt_packet_t packet;
        uint8_t buffer[HF_SENDBUFF_SIZE];
    } mempool[HF_SENDBUFF_MEMPOOL_NUM];
    uint8_t index;
};

typedef struct{
    uint8_t device_id;
    btif_hf_hold_call_t action;
} app_hfp_hold_call_t;

typedef enum app_hfp_hf_event {
    APP_HFP_HF_EVENT_SERVICE_CONNECTED = 0,
    APP_HFP_HF_EVENT_SERVICE_DISCONNECTED,
    APP_HFP_HF_EVENT_RING_IND,
    APP_HFP_HF_EVENT_CALLER_ID_IND,
    APP_HFP_HF_EVENT_AUDIO_CONNECTED,
    APP_HFP_HF_EVENT_AUDIO_DISCONNECTED,
} app_hfp_hf_event_t;

typedef struct app_hfp_hf_event_param_t {
    union {
        struct {
            int device_id;
            bt_bdaddr_t *addr;
            btif_hf_channel_t *channel;
        } service_connected;
        struct {
            int device_id;
            btif_hf_channel_t *channel;
        } service_disconnected;
        struct {
            int device_id;
            const char *number;
            btif_hf_channel_t *channel;
        } ring_ind;
        struct {
            int device_id;
            int number_len;
            const char *number;
            btif_hf_channel_t *channel;
        } caller_id_ind;
        struct {
            int device_id;
            btif_hf_channel_t *channel;
        } audio_connected;
        struct {
            int device_id;
            btif_hf_channel_t *channel;
        } audio_disconnected;
    } p;
} app_hfp_hf_event_param_t;

typedef int (*app_hfp_hf_callback_t)(app_hfp_hf_event_t event, app_hfp_hf_event_param_t *param);

void app_hfp_hf_register_callback(app_hfp_hf_callback_t cb);
void btapp_hfp_report_speak_gain(void);
bool btapp_hfp_mic_need_skip_frame(void);
uint8_t btapp_hfp_need_mute(void);
void hfp_volume_local_set(int id, uint8_t vol);
uint8_t app_bt_hfp_adjust_volume(uint8_t device_id, bool up, bool adjust_local_vol_level);

#ifndef _SCO_BTPCM_CHANNEL_
extern struct hf_sendbuff_control  hf_sendbuff_ctrl;
#endif

int app_hfp_battery_report(uint8_t level);
bt_status_t app_hfp_send_at_command(const char *cmd);

void app_hfp_send_call_hold_request(uint8_t device_id, btif_hf_hold_call_t action);

void app_hfp_report_battery_hf_indicator(uint8_t device_id, uint8_t level);

void app_hfp_report_enhanced_safety(uint8_t device_id, uint8_t value);

void app_bt_hf_send_at_command(uint8_t device_id, const char* at_str);

void app_bt_hf_create_sco_directly(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_service_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_call_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_callsetup_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_callheld_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_signal_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_roam_value(uint8_t device_id);

struct btif_hf_cind_value app_bt_hf_get_cind_battchg_value(uint8_t device_id);

uint32_t app_bt_hf_get_ag_features(uint8_t device_id);

void app_bt_hf_report_batt_level(btif_hf_channel_t* chan_h, uint8_t level);

void app_bt_hfp_enahnced_battery_report(uint8_t level);

bool app_bt_is_hfp_disconnected(uint8_t device_id);

void app_hfp_init(void);

void app_hfp_set_battery_level(uint8_t level);

int app_hfp_battery_report_reset(uint8_t bt_device_id);

void hfp_speak_volume_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx);

void btapp_hfp_mic_need_skip_frame_set(int32_t skip_frame);

uint8_t btapp_hfp_incoming_calls(void);

uint8_t btapp_hfp_get_call_setup(void);

bool btapp_hfp_is_pc_call_active(void);

bool btapp_hfp_is_call_active(void);

bool btapp_hfp_is_sco_active(void);

#ifdef __INTERCONNECTION__
uint8_t ask_is_selfdefined_battery_report_AT_command_support(void);

uint8_t send_selfdefined_battery_report_AT_command(uint8_t device_id);
#endif

bool app_hfp_curr_audio_up(btif_hf_channel_t* hfp_chnl);

void app_hfp_bt_driver_callback(uint8_t device_id, hf_event_t event);

int app_hfp_siri_voice(bool en);

uint8_t app_bt_hf_get_reject_dev(void);

void app_bt_hf_set_reject_dev(uint8_t device_id);

bt_status_t app_hf_send_current_call_at_commond(btif_hf_channel_t* chan_h);

#endif /* BT_HFP_SUPPORT */
#ifdef __cplusplus
}
#endif
#endif /*__APP_HFP_H__*/

