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

#include "hal_aud.h"
#include "hal_chipid.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "apps.h"
#include "app_utils.h"
#include "app_thread.h"
#include "app_status_ind.h"
#include "bluetooth.h"
#include "nvrecord_bt.h"
#include "besbt.h"
#include "besbt_cfg.h"
#include "me_api.h"
#include "a2dp_api.h"
#include "hci_api.h"
#include "l2cap_api.h"
#include "hfp_api.h"
#include "dip_api.h"
#include "btapp.h"
#include "app_bt.h"
#include "app_hfp.h"
#include "app_bt_func.h"
#include "bt_drv_interface.h"
#include "bt_if.h"
#include "bt_drv_reg_op.h"
#include "app_a2dp.h"
#include "app_dip.h"
#if defined(BT_SOURCE)
#include "bt_source.h"
#endif
#include "app_bt_media_manager.h"
#include "app_bt_stream.h"
#include "app_audio.h"
#include "audio_policy.h"
#include "a2dp_decoder.h"
#if defined(IBRT)
#include "app_tws_ibrt.h"
#include "app_ibrt_if.h"
#include "app_tws_ibrt_cmd_sync_a2dp_status.h"
#include "app_ibrt_a2dp.h"
#include "app_tws_besaud.h"
#include "app_tws_ibrt_cmd_handler.h"
#ifdef IBRT_UI_V1
#include "app_ibrt_ui.h"
#endif
#ifdef IBRT_CORE_V2_ENABLE
#include "app_tws_ibrt_conn_api.h"
#include "app_ibrt_tws_ext_cmd.h"
#endif
#endif

#ifdef __AI_VOICE__
#include "app_ai_voice.h"
#endif

#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#endif

#include "app_audio_bt_device.h"
#include "app_audio_active_device_manager.h"
#include "app_audio_control.h"
#include "app_media_player.h"

/*
 * SDK support different audio policy configuration.Each configration should folow different audio focus request rule.
*/
extern uint8_t bt_media_current_music_get(void);
extern uint8_t bt_media_current_sco_get(void);
extern void bt_media_clear_media_type(uint16_t media_type, int device_id);
extern void bt_media_clear_current_media(uint16_t media_type);
static uint8_t app_bt_audio_create_sco_for_another_call_active_device(uint8_t device_id);
static app_bt_audio_ui_allow_resume_request bt_request_ui_resume_a2dp_callback = NULL;

void app_bt_init_config_postphase(struct app_bt_config *config)
{
    if (config->a2dp_prompt_play_mode)
    {
        TRACE(8, "%s ori a2dp prompt %d%d%d,call_pause_a2dp:%d sco %d%d a2dp:%d%d", __func__,
                config->mute_a2dp_stream,
                config->pause_a2dp_stream,
                config->close_a2dp_stream,
                config->pause_a2dp_when_call_exist,
                config->sco_prompt_play_mode,
                config->second_sco_handle_mode,
                config->a2dp_delay_prompt_play,
                config->a2dp_prompt_delay_ms);
    }
    else
    {
        TRACE(8, "%s ori a2dp non-prompt %d%d%d call_pause_a2dp:%d sco %d%d%d", __func__,
                config->mute_a2dp_stream,
                config->pause_a2dp_stream,
                config->close_a2dp_stream,
                config->pause_a2dp_when_call_exist,
                config->sco_prompt_play_mode,
                config->keep_only_one_stream_close_connected_a2dp,
                config->second_sco_handle_mode);
    }
}

void app_bt_switch_to_non_prompt_disc_a2dp_play_mode(void)
{
    TRACE(1, "%s", __func__);

    /* BT_KEEP_ONE_STREAM_CLOSE_CONNECTED_A2DP */
    app_bt_manager.config.a2dp_prompt_play_mode = false;
    app_bt_manager.config.keep_only_one_stream_close_connected_a2dp = true;
    app_bt_manager.config.close_a2dp_stream = true;
    app_bt_manager.config.pause_a2dp_when_call_exist = true;

    // disable other modes
    app_bt_manager.config.mute_a2dp_stream = false;
    app_bt_manager.config.pause_a2dp_stream = false;
}

void app_bt_switch_to_multi_a2dp_quick_switch_play_mode(void)
{
    TRACE(1, "%s", __func__);

    /* BT_MUTE_NEW_A2DP */
    app_bt_manager.config.a2dp_prompt_play_mode = false;
    app_bt_manager.config.mute_a2dp_stream = true;

    // disable other modes
    app_bt_manager.config.keep_only_one_stream_close_connected_a2dp = false;
    app_bt_manager.config.pause_a2dp_when_call_exist = true;
    app_bt_manager.config.close_a2dp_stream = false;
    app_bt_manager.config.pause_a2dp_stream = false;
}

void app_bt_switch_to_prompt_a2dp_mode(void)
{
    app_bt_manager.config.a2dp_prompt_play_mode = true;
    app_bt_manager.config.mute_a2dp_stream = false;
}

static inline void app_bt_audio_set_audio_focus(struct BT_DEVICE_T *device,AUDIO_USAGE_TYPE_E stream_type,int focus_state)
{
    if(stream_type == USAGE_MEDIA)
    {
        device->a2dp_audio_focus = focus_state;
    }
    else if(stream_type == USAGE_CALL)
    {
        device->call_audio_focus = focus_state;
    }
    else if((stream_type == USAGE_RINGTONE))
    {
        device->ring_audio_focus = focus_state;
    }
}

uint8_t app_bt_audio_select_another_streaming_a2dp(uint8_t curr_device_id)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        if (i == curr_device_id)
        {
            continue;
        }
        curr_device = app_bt_get_device(i);
        if (curr_device->a2dp_streamming && !curr_device->a2dp_disc_on_process)
        {
            device_id = i;
        }
    }

    return device_id;
}

uint8_t app_bt_audio_select_streaming_a2dp(void)
{
    return app_bt_audio_select_another_streaming_a2dp(BT_DEVICE_INVALID_ID);
}

uint8_t app_bt_audio_count_streaming_a2dp(void)
{
    uint8_t count = 0;
    struct BT_DEVICE_T* curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->a2dp_streamming)
        {
            count += 1;
        }
    }

    return count;
}

uint8_t app_bt_audio_count_straming_mobile_links(void)
{
    uint8_t count = 0;
    struct BT_DEVICE_T* curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device)
        {
            if ((curr_device->a2dp_streamming) || app_bt_is_sco_connected(i))
            {
                count += 1;
            }
        }
    }

    return count;
}

uint8_t app_bt_audio_count_connected_a2dp(void)
{
    uint8_t count = 0;
    struct BT_DEVICE_T* curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->a2dp_conn_flag)
        {
            count += 1;
        }
    }

    return count;
}

uint8_t app_bt_audio_count_connected_hfp(void)
{
    uint8_t count = 0;
    struct BT_DEVICE_T* curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->hf_conn_flag)
        {
            count += 1;
        }
    }
    return count;
}

uint8_t app_bt_audio_get_curr_playing_a2dp(void)
{
    return app_bt_manager.curr_playing_a2dp_id;
}

uint8_t app_bt_audio_select_another_streaming_sco(uint8_t curr_device_id)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        if (i == curr_device_id)
        {
            continue;
        }
        curr_device = app_bt_get_device(i);
        if (curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
        {
            device_id = i;
        }
    }

    return device_id;
}

uint8_t app_bt_audio_select_streaming_sco(void)
{
    return app_bt_audio_select_another_streaming_sco(BT_DEVICE_INVALID_ID);
}

uint8_t app_bt_audio_count_connected_sco(void)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t count = 0;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
        {
            count += 1;
        }
    }

    return count;
}

uint8_t app_bt_audio_count_request_waiting_sco(void)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t count = 0;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->is_accepting_sco_request)
        {
            count += 1;
        }
    }

    return count;
}

uint8_t app_bt_audio_get_another_request_waiting_sco(uint8_t skip_device_id)
{
    struct BT_DEVICE_T* curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        if (i == skip_device_id)
        {
            continue;
        }
        curr_device = app_bt_get_device(i);
        if (curr_device->is_accepting_sco_request)
        {
            return i;
        }
    }

    return BT_DEVICE_INVALID_ID;
}

bool app_bt_get_if_sco_audio_connected(uint8_t curr_device_id)
{
    struct BT_DEVICE_T* curr_device = NULL;
    bool status = false;

    if (curr_device_id < BT_DEVICE_NUM)
    {
        curr_device = app_bt_get_device(curr_device_id);
        if (curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
        {
            status = true;
        }
    }

    return status;
}

uint8_t app_bt_audio_get_curr_playing_sco(void)
{
    return app_bt_manager.curr_playing_sco_id;
}

uint8_t app_bt_audio_select_connected_device(void)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected)
        {
            device_id = i;
        }
    }

    return device_id;
}

uint8_t app_bt_audio_select_another_connected_hfp(uint8_t curr_device_id)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        if (i == curr_device_id)
        {
            continue;
        }
        curr_device = app_bt_get_device(i);
        if (curr_device->hf_conn_flag)
        {
            device_id = i;
        }
    }

    return device_id;
}

uint8_t app_bt_audio_select_connected_hfp(void)
{
    return app_bt_audio_select_another_connected_hfp(BT_DEVICE_INVALID_ID);
}

uint8_t app_bt_audio_select_another_call_setup_hfp(uint8_t curr_device_id)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        if (i == curr_device_id)
        {
            continue;
        }
        curr_device = app_bt_get_device(i);
        if (curr_device->hfchan_callSetup)
        {
            device_id = i;
        }
    }

    return device_id;
}

uint8_t app_bt_audio_select_call_setup_hfp(void)
{
    return app_bt_audio_select_another_call_setup_hfp(BT_DEVICE_INVALID_ID);
}

static const char* app_bt_audio_get_active_calls_state(void)
{
    struct BT_DEVICE_T* curr_device = NULL;
    static char buff[64] = {0};
    int len = 0;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        len += sprintf(buff+len, "%d", curr_device->hfchan_call);
    }
    return buff;
}

static uint8_t app_bt_audio_choice_other_call_active_device(uint8_t sco_disconnected_device_id)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint8_t found_id = BT_DEVICE_INVALID_ID;
    uint8_t first_call_active_device = BT_DEVICE_INVALID_ID;

    TRACE(2, "(d%x) active calls state: %s", sco_disconnected_device_id, app_bt_audio_get_active_calls_state());

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        if (i == sco_disconnected_device_id)
        {
            continue;
        }
        curr_device = app_bt_get_device(i);
        if (curr_device->hfchan_call)
        {
            if (first_call_active_device == BT_DEVICE_INVALID_ID)
            {
                first_call_active_device = i;
            }
            found_id = i;
        }
    }

    if (found_id == BT_DEVICE_INVALID_ID)
    {
        found_id = first_call_active_device;
    }

    return found_id;
}

uint8_t app_bt_audio_select_another_device_to_create_sco(uint8_t curr_device_id)
{
    uint8_t found_id = BT_DEVICE_INVALID_ID;
    struct BT_DEVICE_T* curr_device = NULL;

    found_id = app_bt_audio_choice_other_call_active_device(curr_device_id);

    if (found_id == BT_DEVICE_INVALID_ID) // continue check call callsetup status
    {
        found_id = app_bt_audio_select_another_call_setup_hfp(curr_device_id);
        if (found_id != BT_DEVICE_INVALID_ID)
        {
            curr_device = app_bt_get_device(found_id);
            if (curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN && !btif_hf_is_inbandring_enabled(curr_device->hf_channel))
            {
                found_id = BT_DEVICE_INVALID_ID; // only create sco for in-band ring incoming call
            }
        }
    }

    return found_id;
}

uint8_t app_bt_audio_select_another_call_active_hfp(uint8_t curr_device_id)
{
    return app_bt_audio_choice_other_call_active_device(curr_device_id);
}

uint8_t app_bt_audio_select_call_active_hfp(void)
{
    return app_bt_audio_select_another_call_active_hfp(BT_DEVICE_INVALID_ID);
}

void app_bt_audio_stop_media_playing(uint8_t device_id)
{
    bt_media_clear_media_type(BT_STREAM_MEDIA, device_id);
    if (bt_media_cur_is_bt_stream_media())
    {
        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE, 0);
        bt_media_clear_current_media(BT_STREAM_MEDIA);
    }
}

static void app_bt_a2dp_set_curr_stream(uint8_t device_id)
{
    //only start/stop a2dp play can set curr stream
    app_bt_manager.curr_a2dp_stream_id = device_id;
}

void app_bt_audio_enable_active_link(bool enable, uint8_t active_id)
{
    btif_remote_device_t* active_rem = NULL;
    uint8_t active_role = 0;
    uint16_t active_handle = 0;
    btif_remote_device_t* prev_active_rem = NULL;
    uint8_t prev_active_role = 0;
    uint16_t prev_active_handle = 0;
    uint8_t link_id[BT_DEVICE_NUM];
    uint8_t active[BT_DEVICE_NUM];
    uint8_t num = 0;
    if (BT_DEVICE_NUM <= 1)
    {
        return;
    }

    if (enable && active_id == BT_DEVICE_INVALID_ID)
    {
        return;
    }

    if (enable && active_id == BT_DEVICE_AUTO_CHOICE_ID)
    {
        if (app_bt_audio_count_connected_a2dp() >= 2 && app_bt_audio_get_curr_playing_a2dp() != BT_DEVICE_INVALID_ID)
        {
            active_id = app_bt_audio_get_curr_playing_a2dp();
        }
        else
        {
            enable = false;
            active_id = BT_DEVICE_INVALID_ID;
        }
    }

    TRACE(4, "%s %d active_id %x prev_id %x", __func__, enable, active_id, app_bt_manager.prev_active_audio_link);

    if (app_bt_manager.prev_active_audio_link != BT_DEVICE_INVALID_ID)
    {
        prev_active_rem = btif_a2dp_get_remote_device(app_bt_get_device(app_bt_manager.prev_active_audio_link)->a2dp_connected_stream);
        if (prev_active_rem)
        {
            prev_active_role = btif_me_get_remote_device_role(prev_active_rem);
            prev_active_handle = btif_me_get_remote_device_hci_handle(prev_active_rem);
            if (prev_active_role == 0) // MASTER
            {
                bt_drv_reg_op_set_tpoll(prev_active_handle-0x80, 0x40); // restore default tpoll
            }
            if(prev_active_handle-0x80 >= bt_drv_reg_op_get_max_acl_nb())
            {
                TRACE(4, "%s restore ERROR handle %x, rem %x!!!", __func__, prev_active_handle,(uint32_t)prev_active_rem);
            }
            link_id[num] = prev_active_handle-0x80;
            active[num] = 0;
            num++;
        }
        app_bt_manager.prev_active_audio_link = BT_DEVICE_INVALID_ID;
    }

    if (enable)
    {
        active_rem = btif_a2dp_get_remote_device(app_bt_get_device(active_id)->a2dp_connected_stream);
        if (active_rem)
        {
            active_role = btif_me_get_remote_device_role(active_rem);
            active_handle = btif_me_get_remote_device_hci_handle(active_rem);

            bt_drv_reg_op_set_music_link(active_handle-0x80);
            bt_drv_reg_op_set_music_link_duration_extra(11);

            if (active_role == 0) // MASTER
            {
                bt_drv_reg_op_set_tpoll(active_handle-0x80, 0x10); // use smaller tpoll
            }
            if(prev_active_handle-0x80 >= bt_drv_reg_op_get_max_acl_nb())
            {
                TRACE(4, "%s use ERROR handle %x, rem %x!!!", __func__, prev_active_handle,(uint32_t)prev_active_rem);
            }
            link_id[num] = active_handle-0x80;
            active[num] = 1;
            num++;
            bt_drv_reg_op_multi_ibrt_music_config(link_id,active,num);
            app_bt_manager.prev_active_audio_link = active_id;
            return;
        }
    }
    if(num > 0)
    {
        bt_drv_reg_op_multi_ibrt_music_config(link_id,active,num);
    }
    bt_drv_reg_op_set_music_link(BT_DEVICE_INVALID_ID);
    app_bt_manager.prev_active_audio_link = BT_DEVICE_INVALID_ID;
}

static void app_bt_audio_start_a2dp_playing(uint8_t device_id)
{
#if defined(BT_WATCH_APP)
    return;
#endif
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if(!curr_device->acl_is_connected || !curr_device->a2dp_conn_flag)
    {
        return;
    }

    app_bt_a2dp_set_curr_stream(device_id);
    app_bt_audio_enable_active_link(true, device_id); // let current playing a2dp get more timing

    if (besbt_cfg.a2dp_play_stop_media_first)
    {
#ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
        for (int i = 0; i < BT_DEVICE_NUM; i += 1) // stop media if any
        {
            if (bt_media_is_media_active_by_device(BT_STREAM_MEDIA, i)
#ifdef __AI_VOICE__
                && (!app_ai_voice_is_need2block_a2dp())
#endif
                )
            {
                app_bt_audio_stop_media_playing(i);
            }
        }
#endif
    }
    if(curr_device->a2dp_streamming)
    {
        if (app_bt_manager.curr_playing_a2dp_id != device_id || app_bt_manager.sco_trigger_a2dp_replay)
        {
            app_bt_manager.sco_trigger_a2dp_replay = false;
            app_bt_manager.curr_playing_a2dp_id = device_id;
            a2dp_audio_sysfreq_update_normalfreq();
            app_audio_adm_ibrt_handle_action(device_id,A2DP_STREAMING_CHANGED,STRAEMING_START);
            app_audio_ctrl_update_bt_music_state(device_id, STRAEMING_START);
            app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START, BT_STREAM_MUSIC, device_id, MAX_RECORD_NUM);
        }
    }
}

void app_bt_audio_stop_a2dp_playing(uint8_t device_id)
{
#if defined(BT_WATCH_APP)
    return;
#endif

    if (app_bt_manager.curr_playing_a2dp_id == device_id)
    {
        app_bt_manager.curr_playing_a2dp_id = BT_DEVICE_INVALID_ID;
        app_audio_adm_ibrt_handle_action(device_id,A2DP_STREAMING_CHANGED,STRAEMING_STOP);
        app_audio_ctrl_update_bt_music_state(device_id, STRAEMING_STOP);
        app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP, BT_STREAM_MUSIC, device_id, MAX_RECORD_NUM);
        app_bt_audio_enable_active_link(false,device_id);
    }
    else
    {
        TRACE(2, "stop a2dp (d%x) fail,curr playing id: %d", device_id,app_bt_manager.curr_playing_a2dp_id);
    }
}

uint32_t app_bt_audio_create_new_prio(void)
{
    return ++app_bt_manager.audio_prio_seed;
}

#define APP_BT_AUDIO_WAIT_BLOCKED_SCO_BECOME_CONNECTED_TIME_MS (1000)
static void app_bt_audio_wait_blocked_sco_handler(void const *n);
osTimerDef (APP_BT_AUDIO_WAIT_BLOCKED_SCO_BECOME_CONNECTED_TIMER, app_bt_audio_wait_blocked_sco_handler);
#define APP_BT_AUDIO_WAIT_BLOCKED_SCO_MAX_TIMES (2)
static uint8_t g_app_bt_audio_sco_connect_times_count = 0;

static void app_bt_audio_wait_this_sco_become_connected(uint8_t device_id, bool first_time)
{
    if (!app_bt_manager.wait_sco_connected_timer)
    {
        app_bt_manager.wait_sco_connected_timer = osTimerCreate(osTimer(APP_BT_AUDIO_WAIT_BLOCKED_SCO_BECOME_CONNECTED_TIMER), osTimerOnce, NULL);
    }

    if (app_bt_manager.wait_sco_connected_device_id != BT_DEVICE_INVALID_ID &&
        app_bt_manager.wait_sco_connected_device_id != device_id)
    {
        TRACE(1, "(d%x) already has a sco waiting device", device_id);
        g_app_bt_audio_sco_connect_times_count = 0;
        return;
    }

    if (first_time)
    {
        g_app_bt_audio_sco_connect_times_count = 1;
    }
    else
    {
        g_app_bt_audio_sco_connect_times_count += 1;
    }

    app_bt_manager.wait_sco_connected_device_id = device_id;

    osTimerStop(app_bt_manager.wait_sco_connected_timer);

    osTimerStart(app_bt_manager.wait_sco_connected_timer, APP_BT_AUDIO_WAIT_BLOCKED_SCO_BECOME_CONNECTED_TIME_MS);
}

static void app_bt_audio_route_sco_path_to_phone(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        app_bt_manager.device_routed_sco_to_phone = device_id;
        TRACE(2,"%s d%x", __func__, device_id);
        app_bt_HF_DisconnectAudioLink(curr_device->hf_channel);
        app_bt_audio_set_audio_focus(app_bt_get_device(device_id), USAGE_CALL, AUDIOFOCUS_LOSS_TRANSIENT);
    }
}

static bool app_bt_audio_route_sco_path_back_to_earbud(uint8_t device_id, bool create_sco_in_normal_way)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device->hf_conn_flag)
    {
        return false;
    }

    if (curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
    {
        return false;
    }

    if (curr_device->hfchan_call == BTIF_HF_CALL_ACTIVE  || curr_device->hfchan_callSetup != BTIF_HF_CALL_SETUP_NONE)
    {
        if (create_sco_in_normal_way)
        {
            app_bt_HF_CreateAudioLink(curr_device->hf_channel);
        }
        else
        {
            app_bt_hf_create_sco_directly(device_id);
        }

        if (device_id == app_bt_manager.device_routed_sco_to_phone)
        {
            app_bt_manager.device_routed_sco_to_phone = BT_DEVICE_INVALID_ID;
        }

        app_bt_manager.device_routing_sco_back = device_id;

        return true;
    }
    else
    {
        return false;
    }
}

#define APP_BT_AUDIO_DELAY_SEND_HOLD_TIME (1000)
#if defined(IBRT_V2_MULTIPOINT)
static bool app_bt_send_hold_command_to_phone(uint8_t device_id, bool hold_to_active)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(hold_to_active)
    {
        if(curr_device->hf_callheld == BTIF_HF_CALL_HELD_NO_ACTIVE)
        {
            app_ibrt_if_hf_call_hold(device_id);
            TRACE(1,"d%x switch hold to call active",device_id);
            return true;
        }
    }
    else
    {
        if(curr_device->hfchan_call == BTIF_HF_CALL_ACTIVE)
        {
            osTimerStart(curr_device->hfp_delay_send_hold_timer, APP_BT_AUDIO_DELAY_SEND_HOLD_TIME);
            TRACE(1,"d%x switch call active to hold",device_id);
            return true;
        }
    }
    return false;
}
#endif

#if defined(IBRT_V2_MULTIPOINT)
static bool app_bt_audio_resume_sco_to_earbud(uint8_t device_id)
{
    bool resume_result = false;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(app_tws_ibrt_mobile_link_connected(&curr_device->remote)){
        if(app_bt_manager.config.second_sco_bg_action == IBRT_ACTION_ROUTE_SCO_TO_PHONE)
        {
            resume_result = app_bt_audio_route_sco_path_back_to_earbud(device_id, true);
        }
        else if(app_bt_manager.config.second_sco_bg_action == IBRT_ACTION_HOLD_ACTIVE_SCO)
        {
            resume_result = app_bt_send_hold_command_to_phone(device_id, true);
            TRACE(1,"d%x resume send hold cmd", device_id);
        }
    }
    return resume_result;
}
#endif


static void app_bt_audio_wait_blocked_sco_handler(void const *n)
{
    uint8_t device_id = app_bt_manager.wait_sco_connected_device_id;
    uint8_t another_connected_sco = app_bt_audio_select_another_streaming_sco(device_id);
    struct BT_DEVICE_T *curr_device = NULL;
    bool wait_sco_conn_failed = false;

    app_bt_manager.wait_sco_connected_device_id = BT_DEVICE_INVALID_ID;

    curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return ;
    }

    TRACE(7, "(d%x) %s %d conn %d call %d callsetup %d sco %d", device_id, __func__, g_app_bt_audio_sco_connect_times_count,
        curr_device->hf_conn_flag, curr_device->hfchan_call, curr_device->hfchan_callSetup, curr_device->hf_audio_state);

    if (!curr_device->hf_conn_flag)
    {
        wait_sco_conn_failed = true;
        goto wait_finished;
    }

    if (curr_device->hfchan_call != BTIF_HF_CALL_ACTIVE  && curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_NONE)
    {
        wait_sco_conn_failed = true;
        goto wait_finished;
    }

    if (curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
    {
        goto wait_finished;
    }

    if (another_connected_sco != BT_DEVICE_INVALID_ID)
    {
        // route exist connected sco path to phone
        app_bt_audio_route_sco_path_to_phone(another_connected_sco); // after this sco disc, it will auto create sco for call active device
        goto wait_finished;
    }

    if (app_bt_audio_route_sco_path_back_to_earbud(device_id, (g_app_bt_audio_sco_connect_times_count == 1) ? true : false))
    {
        if (g_app_bt_audio_sco_connect_times_count < APP_BT_AUDIO_WAIT_BLOCKED_SCO_MAX_TIMES)
        {
            goto continue_wait;
        }
    }

wait_finished:
    g_app_bt_audio_sco_connect_times_count = 0;
    if (wait_sco_conn_failed)
    {
        /* wait blocked sco connected failed, may be due to call already hangup normal cases.
            we need check other call active device to replay because we just disc its sco link */
        app_bt_audio_create_sco_for_another_call_active_device(device_id);
    }
    return;

continue_wait:
    app_bt_audio_wait_this_sco_become_connected(device_id, false);
    return;
}

typedef enum {
    APP_BT_AUDIO_A2DP_RECHECK_CONTEXT_NULL,
    APP_BT_AUDIO_A2DP_WAIT_PHONE_AUTO_START_STREAM = 1,
    APP_BT_AUDIO_A2DP_WAIT_PAUSED_STREAM_SUSPEND,
} app_bt_audio_a2dp_recheck_enum_t;

#define APP_BT_AUDIO_A2DP_WAIT_PHONE_AUTO_START_STREAM_MS (3000)
#define APP_BT_AUDIO_A2DP_WAIT_AVRCP_PAUSED_STREAM_SUSPEND_MS (10000)

static void app_bt_delay_send_hfp_hold_handler(void const *param)
{
#ifdef IBRT
   struct BT_DEVICE_T *curr_device =(struct BT_DEVICE_T*)param;
   TRACE(1,"%s",__func__);
   if((curr_device->call_audio_focus == AUDIOFOCUS_LOSS || curr_device->call_audio_focus == AUDIOFOCUS_LOSS_TRANSIENT) &&
       curr_device->hfchan_call == BTIF_HF_CALL_ACTIVE && app_ibrt_conn_mobile_link_connected(&curr_device->remote))
   {
        app_ibrt_if_hf_call_hold(curr_device->device_id);
   }
#endif
}

/******************************************************************************
1、if prompt return false,and top focus maybe loss.
2、if non-prompt return true,and do handler to handle current request.
********************************************************************************/
static int app_bt_audio_music_ext_policy(focus_device_info_t* cdevice_info, focus_device_info_t *rdevice_info)
{
    TRACE(1,"%s start",__func__);
    struct BT_DEVICE_T *curr_device = app_bt_get_device(cdevice_info->device_idx); 
    if(app_bt_audio_count_streaming_a2dp() > 1 && app_bt_manager.a2dp_switch_trigger_device == cdevice_info->device_idx
        && app_bt_manager.trigger_a2dp_switch && cdevice_info->stream_type == USAGE_MEDIA && rdevice_info->stream_type == USAGE_MEDIA){
        TRACE(1,"a2dp switch to d%x", rdevice_info->device_idx);
        app_bt_manager.trigger_a2dp_switch = false;
        return AUDIOFOCUS_REQUEST_GRANTED;
    }

    if(cdevice_info->stream_type == USAGE_MEDIA &&
       (rdevice_info->stream_type == USAGE_CALL || rdevice_info->stream_type == USAGE_RINGTONE))
    {
        TRACE(1,"call always prompt media");
        return AUDIOFOCUS_REQUEST_GRANTED;
    }

    if(app_bt_manager.config.a2dp_prompt_play_mode)
    {
        if(cdevice_info->stream_type == USAGE_MEDIA && (rdevice_info->stream_type == USAGE_MEDIA))
        {
            TRACE(1,"(d%x)a2dp is streaming",cdevice_info->device_idx);
            return AUDIOFOCUS_REQUEST_GRANTED;
        }

        if(curr_device->a2dp_audio_focus != AUDIOFOCUS_GAIN)
        {
            return AUDIOFOCUS_REQUEST_GRANTED;
        }
    }
    else
    {
        if(cdevice_info->stream_type == USAGE_MEDIA && (rdevice_info->stream_type == USAGE_MEDIA))
        {
            TRACE(1,"(d%x)don't allow play", rdevice_info->device_idx);
            return AUDIOFOCUS_REQUEST_FAILED;
        }
        else
        {
            TRACE(2,"a2dp is not playing (%d)/streming (%d)",curr_device->a2dp_streamming,curr_device->avrcp_palyback_status);
            return AUDIOFOCUS_REQUEST_GRANTED;
        }
    }
    return AUDIOFOCUS_REQUEST_FAILED;
}

HF_CALL_FOCUS_MACHINE_T app_audio_get_current_hfp_machine()
{
    audio_focus_req_info_t *audio_focus = app_audio_get_curr_audio_focus();
    HF_CALL_FOCUS_MACHINE_T hf_focus_machine = HF_FOCUS_MACHINE_CURRENT_IDLE_ANOTHER_IDLE;
    if(audio_focus && (audio_focus->device_info.stream_type == USAGE_RINGTONE || audio_focus->device_info.stream_type == USAGE_CALL))
    {
        hf_focus_machine = (HF_CALL_FOCUS_MACHINE_T)app_audio_hfp_machine_convert(audio_focus->device_info.device_idx);
    }
    else
    {
        hf_focus_machine = HF_FOCUS_MACHINE_CURRENT_IDLE_ANOTHER_IDLE;
        TRACE(1,"%s error!!!", __func__);
    }
    TRACE(2,"%s:%d",__func__,hf_focus_machine);
    return hf_focus_machine;
}

uint8_t app_audio_hfp_machine_convert(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    uint8_t status = HF_FOCUS_MACHINE_CURRENT_IDLE_ANOTHER_IDLE;
    uint8_t another_device_id =(device_id == BT_DEVICE_ID_2) ? BT_DEVICE_ID_1:BT_DEVICE_ID_2;
    struct BT_DEVICE_T *another_device = app_bt_get_device(another_device_id);
    app_bt_hfp_state_checker();
    btif_hf_call_setup_t    current_callSetup  = curr_device->hfchan_callSetup;
    btif_hf_call_active_t   current_call       = curr_device->hfchan_call;
    btif_hf_call_held_state current_callheld   = curr_device->hf_callheld;
    btif_hf_call_setup_t    another_callSetup  = another_device->hfchan_callSetup;
    btif_hf_call_active_t   another_call       = another_device->hfchan_call;
    btif_hf_call_held_state another_callheld   = another_device->hf_callheld;

    if(!another_device->hf_conn_flag || app_bt_audio_current_device_is_hfp_idle_state(another_device_id))
    {
        if( current_callSetup ==BTIF_HF_CALL_SETUP_NONE &&
            current_call == BTIF_HF_CALL_NONE &&
            curr_device->hf_audio_state == BTIF_HF_AUDIO_DISCON)
        {
            TRACE(0,"[FM]:IDLE");
            status = HF_FOCUS_MACHINE_CURRENT_IDLE;
        }
        // current AG is incomming.
        else if( current_callSetup == BTIF_HF_CALL_SETUP_IN &&
                 current_call == BTIF_HF_CALL_NONE )
        {
            TRACE(0,"[FM]:incoming");
            status = HF_FOCUS_MACHINE_CURRENT_INCOMING;
        }
        // current AG is outgoing.
        else if( (current_callSetup >= BTIF_HF_CALL_SETUP_OUT) &&
            current_call == BTIF_HF_CALL_NONE)
        {
            TRACE(0,"[FM]:outgoing");
            status = HF_FOCUS_MACHINE_CURRENT_OUTGOING;
        }
        // current AG is calling.
        else if( (current_callSetup ==BTIF_HF_CALL_SETUP_NONE) &&
                current_call == BTIF_HF_CALL_ACTIVE &&
                current_callheld != BTIF_HF_CALL_HELD_ACTIVE)
        {
            TRACE(0,"[FM]:calling");
            status = HF_FOCUS_MACHINE_CURRENT_CALLING;
        }
        // current AG is 3way incomming.
        else if( current_callSetup ==BTIF_HF_CALL_SETUP_IN &&
                current_call == BTIF_HF_CALL_ACTIVE &&
                current_callheld == BTIF_HF_CALL_HELD_NONE)
        {
            TRACE(0,"[FM]:3way incoming");
            status = HF_FOCUS_MACHINE_CURRENT_3WAY_INCOMING;
        }
        // current AG is 3way hold calling.
        else if( current_callSetup ==BTIF_HF_CALL_SETUP_NONE &&
                current_call == BTIF_HF_CALL_ACTIVE &&
                current_callheld == BTIF_HF_CALL_HELD_ACTIVE)
        {
            TRACE(0,"[FM]:3way hold calling");
            status = HF_FOCUS_MACHINE_CURRENT_3WAY_HOLD_CALING;
        }
    }
    else
    {
        if( current_callSetup == BTIF_HF_CALL_SETUP_IN &&
            current_call == BTIF_HF_CALL_NONE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE&&
            another_callSetup == BTIF_HF_CALL_SETUP_IN &&
            another_call == BTIF_HF_CALL_NONE &&
            another_callheld == BTIF_HF_CALL_HELD_NONE)

        {
            TRACE(0,"[FM]:incoming_incoming");
            status = HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING;
        }
        else if( current_callSetup == BTIF_HF_CALL_SETUP_IN &&
                current_call == BTIF_HF_CALL_NONE &&
                current_callheld == BTIF_HF_CALL_HELD_NONE&&
                another_callSetup >= BTIF_HF_CALL_SETUP_OUT &&
                another_call == BTIF_HF_CALL_NONE &&
                another_callheld == BTIF_HF_CALL_HELD_NONE)
        {
            TRACE(0,"[FM]:incoming_outgoing");
            status = HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING;
        }
        else if(current_callSetup == BTIF_HF_CALL_SETUP_IN &&
                current_call == BTIF_HF_CALL_NONE &&
                current_callheld == BTIF_HF_CALL_HELD_NONE &&
                another_call == BTIF_HF_CALL_ACTIVE)
        {
            TRACE(0,"[FM]:incoming_calling");
            status = HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING;
        }
        else if( current_callSetup >= BTIF_HF_CALL_SETUP_OUT &&
                current_call == BTIF_HF_CALL_NONE &&
                another_callSetup == BTIF_HF_CALL_SETUP_IN &&
                another_call == BTIF_HF_CALL_NONE)
        {
            TRACE(0,"[FM]:outgoing_incoming");
            status = HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_INCOMMING;
        }
        else if( current_callSetup >= BTIF_HF_CALL_SETUP_OUT &&
                 current_call == BTIF_HF_CALL_NONE &&
                 another_call == BTIF_HF_CALL_ACTIVE)

        {
            TRACE(0,"[FM]:outgoing_calling");
            status = HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_CALLING;
        }
        else if(current_call == BTIF_HF_CALL_NONE &&
                another_call == BTIF_HF_CALL_NONE &&
                current_callSetup >= BTIF_HF_CALL_SETUP_OUT &&
                another_callSetup >= BTIF_HF_CALL_SETUP_OUT)
        {
            TRACE(0,"[FM]:outgoing_outgoing");
            status = HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_OUTGOING;
        }
        else if(current_call == BTIF_HF_CALL_ACTIVE &&
                another_callSetup == BTIF_HF_CALL_SETUP_IN &&
                another_call != BTIF_HF_CALL_ACTIVE)
        {
            TRACE(0,"[FM]:calling_incoming");
            status = HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_INCOMING;
        }
        else if(current_call == BTIF_HF_CALL_ACTIVE &&
                another_callSetup >= BTIF_HF_CALL_SETUP_OUT &&
                another_call != BTIF_HF_CALL_ACTIVE)
        {
            TRACE(0,"[FM]:calling_outgoing");
            status = HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_OUTGOING;
        }
        else if(current_call == BTIF_HF_CALL_ACTIVE &&
                current_callheld == BTIF_HF_CALL_HELD_NONE &&
                another_call == BTIF_HF_CALL_ACTIVE &&
                another_callheld == BTIF_HF_CALL_HELD_NONE)
        {
            TRACE(0,"[FM]:calling_calling");
            status = HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_CALLING;
        }
        else if(current_call == BTIF_HF_CALL_ACTIVE &&
                current_callSetup == BTIF_HF_CALL_SETUP_NONE &&
                current_callheld == BTIF_HF_CALL_HELD_NONE &&
                another_callheld == BTIF_HF_CALL_HELD_NO_ACTIVE)
        {
            status = HF_FOCUS_MACHINE_CURRENT_ACTIVE_ANOTHER_HOLD;
            TRACE(0,"[FM]:calling_hold");
        }
        else if(current_callheld != BTIF_HF_CALL_HELD_NONE &&
                another_call == BTIF_HF_CALL_ACTIVE)
        {
            TRACE(0,"[FM]:hold_calling");
            status = HF_FOCUS_MACHINE_CURRENT_HOLD_ANOTHER_ACTIVE;
        }
    }
    return status;
}

POSSIBLY_UNUSED static int app_bt_audio_hfp_stream_ext_policy(focus_device_info_t* cdevice_info, focus_device_info_t *rdevice_info)
{
    bool disallow_prompt = false;
    if(rdevice_info->stream_type == USAGE_MEDIA)
    {
        return AUDIOFOCUS_REQUEST_DELAYED;
    }
#if defined(AUTO_ACCEPT_SECOND_SCO)
    if(app_bt_manager.config.sco_prompt_play_mode){
        disallow_prompt = false;
    }
    else
    {
        uint8_t focus_machine = app_audio_hfp_machine_convert(cdevice_info->device_idx);
        TRACE(2,"%s focus_machine:%d", __func__, focus_machine);
        switch(focus_machine)
        {
            case HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING:
                    disallow_prompt = true;
            break;
            case HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING:
            case HF_FOCUS_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING:
            case HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_INCOMMING:
            case HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_OUTGOING:
            case HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_OUTGOING:
            case HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_CALLING:
                disallow_prompt = true;
            break;
            case HF_FOCUS_MACHINE_CURRENT_OUTGOING_ANOTHER_CALLING:
                if(app_bt_get_device(rdevice_info->device_idx)->hfchan_callSetup >= 2)
                {
                    disallow_prompt = true;
                }
                else{
                    btif_hf_hang_up_call(app_bt_get_device(cdevice_info->device_idx)->hf_channel);
                    disallow_prompt = false;
                }
            break;
            case HF_FOCUS_MACHINE_CURRENT_CALLING_ANOTHER_INCOMING:
            case HF_FOCUS_MACHINE_CURRENT_HOLD_ANOTHER_ACTIVE:
                disallow_prompt = false;
            break;
            default:
            break;
        }
    }
#else
    if(app_bt_manager.config.sco_prompt_play_mode == false &&
       app_bt_manager.config.second_sco_handle_mode == IBRT_HOST_DECIDE_SECONED_SCO){
        return AUDIOFOCUS_REQUEST_DELAYED;
    }else{
        disallow_prompt = (app_bt_manager.config.sco_prompt_play_mode == true) ? false : true;
    }
#endif
    if(disallow_prompt){
        return AUDIOFOCUS_REQUEST_DELAYED;
    }else{
        return AUDIOFOCUS_REQUEST_GRANTED;
    }
}

static int app_bt_audio_virtual_call_ext_policy(focus_device_info_t* cdevice_info, focus_device_info_t *rdevice_info)
{
    TRACE(1,"%s start", __func__);
    bool virtual_call_r = false;
    bool virtual_call_c = btapp_hfp_current_is_virtual_call(cdevice_info->device_idx);

    if(rdevice_info->stream_type == USAGE_MEDIA)
    {
        return AUDIOFOCUS_REQUEST_FAILED;
    }
    if(app_bt_manager.config.virtual_call_handle == VIRTUAL_HANDLE_NON_PROMPTED)
    {
        return virtual_call_c ? AUDIOFOCUS_REQUEST_FAILED : AUDIOFOCUS_REQUEST_GRANTED;
    }
    else
    {
        if(rdevice_info->audio_type == AUDIO_TYPE_BT)
        {
            virtual_call_r = btapp_hfp_current_is_virtual_call(rdevice_info->device_idx);
        }
        if(virtual_call_c)
        {
            return AUDIOFOCUS_REQUEST_GRANTED;
        }
        if(virtual_call_r)
        {
            return AUDIOFOCUS_REQUEST_FAILED;
        }
    }
    return AUDIOFOCUS_REQUEST_GRANTED;
}

static int app_bt_audio_ringtone_stream_ext_policy(focus_device_info_t* cdevice_info, focus_device_info_t *rdevice_info)
{
    return AUDIOFOCUS_REQUEST_GRANTED;
}

#if defined(IBRT_V2_MULTIPOINT)
static int app_bt_audio_switch_call_focus(struct BT_DEVICE_T *device, int request_type,AUDIO_USAGE_TYPE_E stream_type,bool allow_delay)
{
    audio_focus_req_info_t request_info;
    int ret;

    memcpy((uint8_t*)&request_info.device_info.device_addr,(void*)&device->remote,sizeof(bt_bdaddr_t));
    request_info.device_info.device_idx = device->device_id;
    request_info.device_info.audio_type = AUDIO_TYPE_BT;
    request_info.device_info.focus_request_type = request_type;
    request_info.device_info.stream_type = stream_type;
    request_info.device_info.delayed_focus_allow = false;
    if((stream_type == USAGE_CALL))
    {
        request_info.focus_changed_listener = device->sco_focus_changed_listener;
        request_info.ext_policy = app_bt_audio_hfp_stream_ext_policy;
    }
    ret = app_audio_switch_sco_audio_focus_get(&request_info);
    return ret;
}
#endif

static int app_bt_audio_request_audio_focus(struct BT_DEVICE_T *device,int request_type,AUDIO_USAGE_TYPE_E stream_type,bool allow_delay)
{
    audio_focus_req_info_t request_info;
    int ret;

    memcpy((uint8_t*)&request_info.device_info.device_addr.address[0],(void*)&device->remote,sizeof(bt_bdaddr_t));
    request_info.device_info.device_idx = device->device_id;
    request_info.device_info.audio_type = AUDIO_TYPE_BT;
    request_info.device_info.focus_request_type = request_type;
    request_info.device_info.stream_type = stream_type;
    request_info.device_info.delayed_focus_allow = allow_delay;

    if(stream_type == USAGE_MEDIA)
    {
        request_info.focus_changed_listener = device->a2dp_focus_changed_listener;
        request_info.ext_policy = app_bt_audio_music_ext_policy;
    }
    else if(stream_type == USAGE_CALL)
    {
        request_info.focus_changed_listener = device->sco_focus_changed_listener;
        if(app_bt_manager.config.virtual_call_handle == VIRTUAL_HANDLE_NON_PROMPT ||
           app_bt_manager.config.virtual_call_handle == VIRTUAL_HANDLE_NON_PROMPTED)
        {
            request_info.ext_policy = app_bt_audio_virtual_call_ext_policy;
        }else{
            request_info.ext_policy = app_bt_audio_hfp_stream_ext_policy;
        }
    }
    else if(stream_type == USAGE_RINGTONE)
    {
        request_info.focus_changed_listener = device->ring_focus_changed_listener;
        request_info.ext_policy = app_bt_audio_ringtone_stream_ext_policy;
    }
    ret = app_audio_request_focus(&request_info);
    if(ret == AUDIOFOCUS_REQUEST_GRANTED)
    {
        TRACE(0,"audio_policy:request audio focus sucess!");
        app_bt_audio_set_audio_focus(device,stream_type,AUDIOFOCUS_GAIN);
    }
    else if(ret == AUDIOFOCUS_REQUEST_DELAYED)
    {
        app_bt_audio_set_audio_focus(device,stream_type,AUDIOFOCUS_GAIN_TRANSIENT);
        TRACE(0,"audio_policy:request audio focus delay!");
    }
    else /* AUDIOFOCUS_REQUEST_FAILED */
    {
        //app_bt_audio_set_audio_focus(device,stream_type,AUDIOFOCUS_NONE);
        TRACE(0,"audio_policy:request audio focus fail!");
    }

    return ret;
}

static void app_bt_audio_abandon_audio_focus(struct BT_DEVICE_T *device,AUDIO_USAGE_TYPE_E stream_type)
{
    audio_focus_req_info_t removed_fouces;

    memcpy((uint8_t*)&removed_fouces.device_info.device_addr,(void*)&device->remote,sizeof(bt_bdaddr_t));
    removed_fouces.device_info.device_idx = device->device_id;
    removed_fouces.device_info.audio_type = AUDIO_TYPE_BT;
    removed_fouces.device_info.stream_type = stream_type;

    if(stream_type == USAGE_MEDIA)
    {
        removed_fouces.focus_changed_listener = device->a2dp_focus_changed_listener;
        removed_fouces.ext_policy = app_bt_audio_music_ext_policy;
    }
    else if(stream_type == USAGE_CALL)
    {
        removed_fouces.focus_changed_listener = device->sco_focus_changed_listener;
        removed_fouces.ext_policy = app_bt_audio_hfp_stream_ext_policy;
    }
    else if(stream_type == USAGE_RINGTONE)
    {
        removed_fouces.focus_changed_listener = device->ring_focus_changed_listener;
        removed_fouces.ext_policy = app_bt_audio_ringtone_stream_ext_policy;
    }
    app_audio_abandon_focus(&removed_fouces);
    app_bt_audio_set_audio_focus(device,stream_type,AUDIOFOCUS_NONE);
}

static void app_bt_audio_call_force_route_to_phone(struct BT_DEVICE_T *device)
{
    audio_focus_req_info_t focus_ower;
    AUDIO_USAGE_TYPE_E curr_media_usage_type = app_audio_get_curr_audio_stream_type(AUDIO_TYPE_BT,device->device_id);

    if (!app_audio_is_this_curr_play_src(AUDIO_TYPE_BT,device->device_id,curr_media_usage_type))
    {
        TRACE(1,"audio_policy:d(%d) already is not play source",device->device_id);
        return;
    }

    TRACE(1,"audio_policy:d(%d) route call to phone",device->device_id);
    memcpy((uint8_t*)&focus_ower.device_info.device_addr,(void*)&device->remote,sizeof(bt_bdaddr_t));
    focus_ower.device_info.device_idx = device->device_id;
    focus_ower.device_info.audio_type = AUDIO_TYPE_BT;
    if(app_audio_get_curr_audio_stream_type(AUDIO_TYPE_BT,device->device_id) == USAGE_RINGTONE)
    {
         focus_ower.device_info.stream_type = USAGE_RINGTONE;
    }
    else
    {
        focus_ower.device_info.stream_type = USAGE_CALL;
    }

    if(app_audio_hands_over_audio_focus(&focus_ower))
    {
        device->call_audio_focus = AUDIOFOCUS_NONE;
    }
}

static void app_bt_audio_abandon_call_focus(struct BT_DEVICE_T *device)
{
    if(app_audio_get_curr_audio_stream_type(AUDIO_TYPE_BT,device->device_id) == USAGE_RINGTONE)
    {
         app_bt_audio_abandon_audio_focus(device,USAGE_RINGTONE);
    }
    else
    {
        app_bt_audio_abandon_audio_focus(device,USAGE_CALL);
    }
}

static void app_bt_audio_abanon_focus(struct BT_DEVICE_T *device)
{
    AUDIO_USAGE_TYPE_E focus_type = app_audio_get_curr_audio_stream_type(AUDIO_TYPE_BT,device->device_id);
    if(focus_type != USAGE_UNKNOWN)
    {
        app_bt_audio_abandon_audio_focus(device,focus_type);
    }
}

POSSIBLY_UNUSED static void app_bt_audio_ringtone_request_focus(struct BT_DEVICE_T *device)
{
    uint8_t focus_result = 0;     /* A failed focus change request.*/
    POSSIBLY_UNUSED audio_focus_req_info_t* audio_focus = app_audio_get_curr_audio_focus();
    if(device->ring_audio_focus == AUDIOFOCUS_GAIN)
    {
        media_PlayAudio(AUD_ID_BT_CALL_INCOMING_CALL, device->device_id);
        return;
    }
    if(app_bt_manager.curr_playing_a2dp_id != BT_DEVICE_INVALID_ID)
    {
        focus_result = app_bt_audio_request_audio_focus(device,AUDIOFOCUS_GAIN_TRANSIENT,USAGE_RINGTONE,false);
        TRACE(2,"audio_policy:d%x ringtone interupt a2dp result:%d",device->device_id, focus_result);
    }
    else if(app_bt_manager.curr_playing_sco_id != BT_DEVICE_INVALID_ID)
    {
        focus_result = app_bt_audio_request_audio_focus(device,AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK,USAGE_RINGTONE,false);
        TRACE(2,"audio_policy:d%x ringtone mix with sco result:%d",device->device_id, focus_result);
    }
    else
    {
        focus_result = app_bt_audio_request_audio_focus(device,AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK,USAGE_RINGTONE,false);
    }

    if(focus_result == AUDIOFOCUS_REQUEST_GRANTED)
    {
        TRACE(0,"[audio_policy] request ring focus granted!!!");
#if defined(BT_UPDATE_ACTIVE_DEVICE_WHEN_INCOMING_CALL)
        app_audio_adm_ibrt_handle_action(device->device_id, RINGTONE_CHANGED, RINGTONE_START);
        app_audio_bt_update_call_state(device->device_id, false);
#endif
        device->ring_audio_focus = AUDIOFOCUS_GAIN;
        media_PlayAudio(AUD_ID_BT_CALL_INCOMING_CALL, device->device_id);
    }
    else
    {
        TRACE(0,"audio_policy:ringtone request focus fail/delay");
    }
}

static void app_bt_audio_a2dp_streaming_recheck_timer_handler(void const *param)
{
    struct BT_DEVICE_T *recheck_device = (struct BT_DEVICE_T *)param;
    uint8_t curr_playing_a2dp = app_bt_audio_get_curr_playing_a2dp();
    struct BT_DEVICE_T *curr_playing_device = app_bt_get_device(curr_playing_a2dp);
    uint8_t reselected_a2dp = BT_DEVICE_INVALID_ID;

    if (app_bt_manager.a2dp_stream_play_recheck)
    {
        app_bt_manager.a2dp_stream_play_recheck = false;

        reselected_a2dp = app_bt_audio_select_streaming_a2dp();
    }
    else if (recheck_device->a2dp_stream_recheck_context == APP_BT_AUDIO_A2DP_WAIT_PAUSED_STREAM_SUSPEND)
    {
        recheck_device->a2dp_stream_recheck_context = APP_BT_AUDIO_A2DP_RECHECK_CONTEXT_NULL;

        if (curr_playing_a2dp != BT_DEVICE_INVALID_ID)
        {
            return;
        }

        if (recheck_device->a2dp_streamming)
        {
            reselected_a2dp = recheck_device->device_id;
        }
    }
    else if (recheck_device->a2dp_stream_recheck_context == APP_BT_AUDIO_A2DP_WAIT_PHONE_AUTO_START_STREAM)
    {
        recheck_device->a2dp_stream_recheck_context = APP_BT_AUDIO_A2DP_RECHECK_CONTEXT_NULL;

        if (curr_playing_a2dp != BT_DEVICE_INVALID_ID)
        {
            if (curr_playing_device->a2dp_streamming)
            {
                return;
            }

            app_bt_audio_stop_a2dp_playing(curr_playing_a2dp);
            curr_playing_a2dp = BT_DEVICE_INVALID_ID;
        }

        if (curr_playing_a2dp == BT_DEVICE_INVALID_ID)
        {
            reselected_a2dp = app_bt_audio_select_streaming_a2dp();
        }
    }

    TRACE(4, "(d%x) a2dp streaming recheck: curr_playing_a2dp %x strming %d reselect %x", recheck_device->device_id,
        curr_playing_a2dp, curr_playing_device ? curr_playing_device->a2dp_streamming : 0, reselected_a2dp);

    if (reselected_a2dp != BT_DEVICE_INVALID_ID)
    {
        struct BT_DEVICE_T *curr_device = app_bt_get_device(reselected_a2dp);
        if(app_bt_audio_request_audio_focus(curr_device,AUDIOFOCUS_GAIN,USAGE_MEDIA,true) != AUDIOFOCUS_REQUEST_GRANTED)
        {
            app_bt_audio_select_a2dp_behavior_handler(reselected_a2dp);
        }
    }
}

void app_bt_audio_a2dp_stream_recheck_timer_callback(void const *param)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)param, 0, (uint32_t)(uintptr_t)app_bt_audio_a2dp_streaming_recheck_timer_handler);
}

void app_bt_delay_send_hfp_hold_callback(void const *param)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)param, 0, (uint32_t)(uintptr_t)app_bt_delay_send_hfp_hold_handler);
}

void app_bt_audio_recheck_a2dp_streaming(void)
{
    app_bt_manager.a2dp_stream_play_recheck = true;
    app_bt_audio_a2dp_stream_recheck_timer_callback((void *)app_bt_get_device(BT_DEVICE_ID_1));
}

static void app_bt_audio_start_a2dp_recheck(uint8_t device_id, uint32_t delay_ms)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device && curr_device->a2dp_stream_recheck_timer)
    {
        osTimerStop(curr_device->a2dp_stream_recheck_timer);

        osTimerStart(curr_device->a2dp_stream_recheck_timer, delay_ms);
    }
}

static void app_bt_audio_a2dp_streaming_recheck_timer_start(uint8_t device_id, app_bt_audio_a2dp_recheck_enum_t ctx)
{
    uint32_t wait_ms = 0;

    if (app_bt_manager.config.dont_auto_play_bg_a2dp && ctx == APP_BT_AUDIO_A2DP_WAIT_PAUSED_STREAM_SUSPEND)
    {
        return;
    }

    if (app_bt_manager.config.keep_only_one_stream_close_connected_a2dp && app_bt_audio_count_connected_sco())
    {
        // dont try to recheck a2dp when sco exist in case a2dp auto disc under mode
        // of keep_only_one_stream_close_connected_a2dp
        return;
    }

    if (ctx == APP_BT_AUDIO_A2DP_WAIT_PAUSED_STREAM_SUSPEND)
    {
        wait_ms = APP_BT_AUDIO_A2DP_WAIT_AVRCP_PAUSED_STREAM_SUSPEND_MS;
    }
    else
    {
        wait_ms = APP_BT_AUDIO_A2DP_WAIT_PHONE_AUTO_START_STREAM_MS;
    }

    app_bt_get_device(device_id)->a2dp_stream_recheck_context = ctx;
    app_bt_audio_start_a2dp_recheck(device_id, wait_ms);
}

#if defined(IBRT_V2_MULTIPOINT)
void app_bt_ibrt_audio_play_a2dp_stream(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (tws_besaud_is_connected())
    {
        if (app_tws_ibrt_mobile_link_connected(&curr_device->remote) &&
            curr_device->avrcp_conn_flag)
        {
            // only ibrt master need to trigger action
            app_bt_a2dp_send_key_request(device_id, AVRCP_KEY_PLAY);
        }
    }
    else
    {
        app_bt_a2dp_send_key_request(device_id, AVRCP_KEY_PLAY);
    }
}

void app_bt_ibrt_audio_pause_a2dp_stream(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (tws_besaud_is_connected())
    {
        bool mobile_connected = app_ibrt_conn_mobile_link_connected(&curr_device->remote);
        TRACE(3, "(d%x) %s mobile_connected:%d", device_id, __func__, mobile_connected);
        if (mobile_connected)
        {
            // only ibrt master need to trigger action
            app_bt_a2dp_send_key_request(device_id, AVRCP_KEY_PAUSE);
        }
    }
    else
    {
        app_bt_a2dp_send_key_request(device_id, AVRCP_KEY_PAUSE);
    }
}
#endif

void app_bt_audio_set_bg_a2dp_device(bool is_paused_bg_device, bt_bdaddr_t *bdaddr)
{

}

uint8_t app_bt_audio_select_bg_a2dp_to_resume()
{
    struct BT_DEVICE_T* curr_device = NULL;
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if(curr_device->this_is_bg_a2dp)
        {
            return i;
        }
    }
    return BT_DEVICE_INVALID_ID;
}

typedef enum {
    APP_KILL_OLD_A2DP = 0x01,
    APP_OLD_SCO_A2DP_PLAYING_KILL_NEW_A2DP = 0x10,
    APP_OLD_SCO_PLAYING_KILL_NEW_A2DP,
    APP_OLD_A2DP_PLAYING_KILL_NEW_A2DP,
    APP_NO_A2DP_PLAYING_KILL_CONNECTED_A2DP,
    APP_OLD_A2DP_PLAYING_NEW_SCO_COME_KILL_OLD_A2DP,
    APP_NO_A2DP_PLAYING_NEW_SCO_COME_KILL_CONNECTED_A2DP,
} app_bt_audio_a2dp_behavior_ctx_enum;

void app_bt_audio_a2dp_close_this_profile(uint8_t device_id)
{
#if BT_DEVICE_NUM > 1
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    curr_device->this_is_closed_bg_a2dp = true;
    curr_device->auto_make_remote_play = false;
#if defined(IBRT_V2_MULTIPOINT)
    if (tws_besaud_is_connected())
    {
        if (!app_tws_ibrt_mobile_link_connected(&curr_device->remote))
        {
            TRACE(2, "(d%x) %s not master", device_id, __func__);
            curr_device->this_is_closed_bg_a2dp = false; // ibrt slave only set to true when master disc profile request received
            return;
        }
        if (!app_ibrt_conn_is_profile_exchanged(&curr_device->remote))
        {
            TRACE(2, "(d%x) %s profile not exchanged", device_id, __func__);
            curr_device->this_is_closed_bg_a2dp = false;
            return;
        }
    }
    app_ibrt_if_master_disconnect_a2dp_profile(device_id);
#else
    app_bt_disconnect_a2dp_profile(curr_device->a2dp_connected_stream);
#endif
#endif
}

bool app_bt_audio_a2dp_disconnect_self_check(uint8_t device_id)
{
#if BT_DEVICE_NUM > 1
    if (app_bt_manager.config.keep_only_one_stream_close_connected_a2dp && app_bt_manager.config.close_a2dp_stream)
    {
        struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
        uint8_t another_playing_a2dp = app_bt_audio_get_curr_playing_a2dp();
        uint8_t another_playing_sco = app_bt_audio_get_curr_playing_sco();

        if (another_playing_sco != BT_DEVICE_INVALID_ID && device_id != another_playing_sco
#if defined(IBRT_V2_MULTIPOINT)
            && app_ibrt_conn_is_profile_exchanged(&app_bt_get_device(another_playing_sco)->remote)
#endif
            )
        {
            // another profile exchanged sco device is playing, dont allow current a2dp connect req
            curr_device->this_is_closed_bg_a2dp = true;
            TRACE(1, "(d%x) another sco device playing, not allow a2dp connect req", device_id);
            return true;
        }

        if (another_playing_a2dp != BT_DEVICE_INVALID_ID && device_id != another_playing_a2dp
#if defined(IBRT_V2_MULTIPOINT)
            && app_ibrt_conn_is_profile_exchanged(&app_bt_get_device(another_playing_a2dp)->remote)
#endif
            )
        {
            // another profile exchanged a2dp device is playing, dont allow current a2dp connect req
            curr_device->this_is_closed_bg_a2dp = true;
            TRACE(1, "(d%x) another a2dp device playing, not allow a2dp connect req", device_id);
            return true;
        }
    }
#endif

    return false;
}

void app_bt_audio_a2dp_resume_this_profile(uint8_t device_id)
{
#if BT_DEVICE_NUM > 1
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (app_bt_manager.config.pause_a2dp_stream || app_bt_manager.config.pause_a2dp_when_call_exist)
    {
        if (curr_device->a2dp_conn_flag && (!curr_device->a2dp_streamming || curr_device->waiting_pause_suspend
            || curr_device->avrcp_palyback_status != BTIF_AVRCP_MEDIA_PLAYING))
        {
#if defined(IBRT_V2_MULTIPOINT)
            curr_device->waiting_pause_suspend = false;
            app_bt_ibrt_audio_play_a2dp_stream(device_id);
#else
            app_bt_a2dp_send_key_request(device_id, AVRCP_KEY_PLAY);
#endif
        }
        else
        {
            TRACE(5, "%s d%x a2dp %d strm %d playstat %d", __func__,
                device_id, curr_device->a2dp_conn_flag, curr_device->a2dp_streamming, curr_device->avrcp_palyback_status);
        }
    }
    else if(app_bt_manager.config.close_a2dp_stream)
    {
#if defined(IBRT_V2_MULTIPOINT)
        app_ibrt_if_master_connect_a2dp_profile(device_id);
#else
        app_bt_reconnect_a2dp_profile(&curr_device->remote);
#endif
    }
#endif
}

#define APP_BT_A2DP_DELAY_RECONNECT_MS (500)

void app_bt_audio_update_local_a2dp_playing_device(uint8_t device_id)
{
#if defined(IBRT_V2_MULTIPOINT)
    if (app_bt_audio_count_connected_a2dp() > 1)
    {
        app_ibrt_send_ext_cmd_a2dp_playing_device(device_id, false);
    }
#endif
}

void app_bt_audio_receive_peer_a2dp_playing_device(bool is_response, uint8_t device_id)
{
#if defined(IBRT_V2_MULTIPOINT)
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    uint8_t curr_playing_a2dp = app_bt_audio_get_curr_playing_a2dp();
    if (!is_response)
    {
        if (app_tws_ibrt_is_profile_exchanged(&curr_device->remote) && curr_playing_a2dp != BT_DEVICE_INVALID_ID && curr_device->a2dp_streamming)
        {
            if (curr_playing_a2dp != device_id)
            {
                app_bt_audio_stop_a2dp_playing(curr_playing_a2dp);
                app_bt_audio_start_a2dp_playing(device_id);
                app_ibrt_send_ext_cmd_a2dp_playing_device(device_id, true);
            }
        }
    }
    else
    {
        if (app_bt_audio_count_connected_a2dp() > 1 && curr_playing_a2dp != BT_DEVICE_INVALID_ID && curr_playing_a2dp != device_id)
        {
            app_ibrt_send_ext_cmd_a2dp_playing_device(curr_playing_a2dp, false);
        }
    }
#endif
}

void app_bt_audio_switch_streaming_sco(void)
{
    uint8_t curr_playing_sco = app_bt_audio_get_curr_playing_sco();
    uint8_t sco_count = app_bt_audio_count_connected_sco();
    uint8_t active_call_device = BT_DEVICE_INVALID_ID;

    if (curr_playing_sco != BT_DEVICE_INVALID_ID)
    {
        if (sco_count > 1)
        {
            app_bt_audio_route_sco_path_to_phone(curr_playing_sco); // after this sco disc, it will auto play other straming sco
            return;
        }

        active_call_device = app_bt_audio_select_another_device_to_create_sco(curr_playing_sco);

        if (active_call_device == BT_DEVICE_INVALID_ID)
        {
            TRACE(1,"%s no another active call", __func__);
            return;
        }
        if(app_bt_manager.config.sco_prompt_play_mode)
        {
            app_bt_audio_route_sco_path_back_to_earbud(active_call_device, true);
        }
        else
        {
            app_bt_audio_route_sco_path_to_phone(curr_playing_sco); // after this sco disc, it will auto create sco for call active device
        }
    }
    else
    {
        uint8_t curr_active_call = app_bt_audio_select_call_active_hfp();

        if (curr_active_call == BT_DEVICE_INVALID_ID)
        {
            return;
        }

        // consider curr_active_call is current sco, route other call active sco path back
        active_call_device = app_bt_audio_select_another_device_to_create_sco(curr_active_call);
        if (active_call_device != BT_DEVICE_INVALID_ID)
        {
            app_bt_audio_route_sco_path_back_to_earbud(active_call_device, true);
        }
    }
}

bool app_bt_audio_switch_streaming_a2dp(void)
{
    uint8_t curr_playing_a2dp = app_bt_audio_get_curr_playing_a2dp();
    uint8_t selected_streaming_a2dp = BT_DEVICE_INVALID_ID;

    if (curr_playing_a2dp == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no playing a2dp", __func__);
        app_bt_manager.trigger_a2dp_switch = false;
        return false;
    }

    selected_streaming_a2dp = app_bt_audio_select_another_streaming_a2dp(curr_playing_a2dp);
    if (selected_streaming_a2dp == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no other streaming a2dp", __func__);
        app_bt_manager.trigger_a2dp_switch = false;
        return false;
    }
#if defined(IBRT)
    if(!app_tws_ibrt_tws_link_connected())
    {
        app_bt_manager.trigger_a2dp_switch = true;
        app_bt_manager.a2dp_switch_trigger_device = curr_playing_a2dp;
    }
#endif
    app_bt_audio_event_handler(selected_streaming_a2dp, APP_BT_AUDIO_EVENT_A2DP_STREAM_SWITCH, 0);     //switch to device_id
    return true;
}

#define A2DP_SWITCH_DELAY_TICKS (20*32) // 200ms - 32 * 312.5 is 10ms

void app_bt_audio_a2dp_switch_trigger(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    uint8_t curr_playing_a2dp = app_bt_audio_get_curr_playing_a2dp();
    uint8_t selected_streaming_a2dp = BT_DEVICE_INVALID_ID;
    uint32_t curr_btclk = 0;
    bool time_reached = false;

    if (!app_bt_manager.trigger_a2dp_switch)
    {
        return;
    }

    if (curr_playing_a2dp == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no playing a2dp", __func__);
        app_bt_manager.trigger_a2dp_switch = false;
        return;
    }

    selected_streaming_a2dp = app_bt_audio_select_another_streaming_a2dp(curr_playing_a2dp);
    if (selected_streaming_a2dp == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no other streaming a2dp", __func__);
        app_bt_manager.trigger_a2dp_switch = false;
        return ;
    }

    if (curr_playing_a2dp != app_bt_manager.a2dp_switch_trigger_device)
    {
        TRACE(3,"%s trigger device not match %x %x", __func__, curr_playing_a2dp, app_bt_manager.a2dp_switch_trigger_device);
        app_bt_manager.trigger_a2dp_switch = false;
        return;
    }

    curr_device = app_bt_get_device(curr_playing_a2dp);
    curr_btclk = bt_syn_get_curr_ticks(curr_device->acl_conn_hdl);

    TRACE(3,"%s playing_a2dp %x selected_a2dp %x", __func__, curr_playing_a2dp, selected_streaming_a2dp);
    TRACE(2,"tgtclk %x curclk %x", app_bt_manager.a2dp_switch_trigger_btclk, curr_btclk);

    if (curr_btclk < app_bt_manager.a2dp_switch_trigger_btclk)
    {
        if (app_bt_manager.a2dp_switch_trigger_btclk - curr_btclk > A2DP_SWITCH_DELAY_TICKS * 2)
        {
            TRACE(3,"%s btclk may wrapped %d %d", __func__, app_bt_manager.a2dp_switch_trigger_btclk, curr_btclk);
            time_reached = true; // bt clock may wrapped
        }
        else
        {
            time_reached = false;
        }
    }
    else
    {
        time_reached = true;
    }

    if (time_reached)
    {
        app_bt_audio_switch_streaming_a2dp();
    }
}

// TODO:
uint32_t app_bt_audio_trigger_switch_streaming_a2dp(uint32_t btclk)
{
    uint8_t curr_playing_a2dp = app_bt_audio_get_curr_playing_a2dp();
    uint8_t selected_streaming_a2dp = BT_DEVICE_INVALID_ID;
    struct BT_DEVICE_T *curr_device = NULL;
    uint32_t curr_btclk = 0;
    uint32_t trigger_btclk = 0;

    if (app_bt_manager.trigger_a2dp_switch)
    {
        TRACE(2,"%s already has a a2dp switch trigger %x", __func__, app_bt_manager.a2dp_switch_trigger_device);
        return 0;
    }

    if (curr_playing_a2dp == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no playing a2dp", __func__);
        return 0;
    }

    selected_streaming_a2dp = app_bt_audio_select_another_streaming_a2dp(curr_playing_a2dp);
    if (selected_streaming_a2dp == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no other streaming a2dp", __func__);
        return 0;
    }

    curr_device = app_bt_get_device(curr_playing_a2dp);
    if (btclk == 0)
    {
        curr_btclk = bt_syn_get_curr_ticks(curr_device->acl_conn_hdl);
        trigger_btclk = curr_btclk + A2DP_SWITCH_DELAY_TICKS;
    }
    else
    {
        trigger_btclk = btclk;
    }

    app_bt_manager.trigger_a2dp_switch = true;
    app_bt_manager.a2dp_switch_trigger_btclk = trigger_btclk;
    app_bt_manager.a2dp_switch_trigger_device = curr_playing_a2dp;

    app_bt_audio_a2dp_switch_trigger();

    return trigger_btclk;
}

static void app_bt_hf_set_curr_stream(uint8_t device_id)
{
    app_bt_manager.curr_hf_channel_id = device_id;
}

extern uint8_t bt_media_current_music_get(void);
extern uint8_t bt_media_current_sco_get(void);

uint8_t app_bt_audio_get_curr_a2dp_device(void)
{
    uint8_t device_id = bt_media_current_music_get();
    return (device_id != BT_DEVICE_INVALID_ID ? device_id : app_bt_manager.curr_a2dp_stream_id);
}

uint8_t app_bt_audio_get_curr_hfp_device(void)
{
    uint8_t device_id = app_bt_audio_get_curr_playing_sco();
    return (device_id != BT_DEVICE_INVALID_ID ? device_id : app_bt_manager.curr_hf_channel_id);
}

uint8_t app_bt_audio_get_curr_sco_device(void)
{
    uint8_t device_id = bt_media_current_sco_get();
    return (device_id != BT_DEVICE_INVALID_ID ? device_id : app_bt_manager.curr_hf_channel_id);
}

static uint8_t app_bt_audio_select_hfp_call_activity_device(void)
{
    /** device priority for user action
     *      1. device has call in setup flow    ---> answer, reject, hold
     *      2. current sco playing device       ---> hungup, hold, sco path switch
     *      3. other sco connected device       ---> hungup, hold, sco path switch
     *      4. device has active/held call      ---> hungup, hold/active switch
     */
    uint8_t device_id = app_bt_audio_select_call_setup_hfp();
    if (device_id == BT_DEVICE_INVALID_ID)
    {
        device_id = app_bt_audio_get_curr_playing_sco();
        if (device_id == BT_DEVICE_INVALID_ID)
        {
            device_id = app_bt_audio_select_streaming_sco();
            if (device_id == BT_DEVICE_INVALID_ID)
            {
                device_id = app_bt_audio_select_call_active_hfp();
            }
        }
    }
    return device_id;
}

// TODO:  use common interface
uint8_t app_bt_audio_get_hfp_device_for_user_action(void)
{
    uint8_t device_id = app_bt_audio_select_hfp_call_activity_device();
    if (device_id == BT_DEVICE_INVALID_ID)
    {
        device_id = app_bt_audio_select_connected_hfp();
        if (device_id == BT_DEVICE_INVALID_ID)
        {
            device_id = BT_DEVICE_ID_1;
        }
    }
    return device_id;
}

// TODO:  use common interface
uint8_t app_bt_audio_get_device_for_user_action(void)
{
    uint8_t device_id = app_bt_audio_select_hfp_call_activity_device();
    if (device_id == BT_DEVICE_INVALID_ID)
    {
        device_id = app_bt_audio_get_curr_a2dp_device();
    }
    return device_id;
}

uint8_t app_bt_audio_get_another_hfp_device_for_user_action(uint8_t curr_device_id)
{
//need considration
    return (curr_device_id == BT_DEVICE_ID_1) ? BT_DEVICE_ID_2 : BT_DEVICE_ID_1;
}

uint8_t app_bt_audio_get_low_prio_sco_device(uint8_t device_id)
{
    struct BT_DEVICE_T* curr_device = NULL;
    uint32_t stream_prio = app_bt_get_device(device_id)->sco_audio_prio;
    uint8_t low_device_id = BT_DEVICE_INVALID_ID;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        if(i== device_id)
        {
            continue;
        }
        curr_device = app_bt_get_device(i);
        if (curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
        {
            if(curr_device->sco_audio_prio < stream_prio)
            {
                stream_prio = curr_device->sco_audio_prio;
                low_device_id = i;
            }
        }
    }
    TRACE(3,"(d%x)%s,sco_prio:%d", low_device_id, __func__, stream_prio);
    return low_device_id;
}

static void app_bt_audio_start_sco_playing(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (besbt_cfg.vendor_codec_en && btif_hf_is_sco_wait_codec_sync(curr_device->hf_channel))
    {
        curr_device->this_sco_wait_to_play = true;
        return;
    }
    TRACE(3,"(d%x)%s,hf_audio_state:%d", device_id, __func__, curr_device->hf_audio_state);
    if (curr_device->hf_audio_state == BTIF_HF_AUDIO_DISCON)
    {
        return;
    }

    curr_device->this_sco_wait_to_play = false;

    app_bt_hf_set_curr_stream(device_id);

#ifndef AUDIO_PROMPT_USE_DAC2_ENABLED
    for (int i = 0; i < BT_DEVICE_NUM; i += 1) // stop media if any
    {
        if (bt_media_is_media_active_by_device(BT_STREAM_MEDIA, i))
        {
            app_bt_audio_stop_media_playing(i);
        }
    }
#endif
    if (app_bt_manager.curr_playing_sco_id != device_id)
    {
#if defined(AUTO_ACCEPT_SECOND_SCO)
        if(app_bt_audio_count_connected_sco() > 1)
        {
            uint8_t low_prio_device = app_bt_audio_get_low_prio_sco_device(device_id);
            if(low_prio_device != BT_DEVICE_INVALID_ID && low_prio_device != device_id)
            {
                struct BT_DEVICE_T *Fg_device = app_bt_get_device(low_prio_device);
                uint16_t scohandle = btif_hf_get_sco_hcihandle(Fg_device->hf_channel);
                app_bt_Me_switch_sco(scohandle);
                TRACE(0,"###low prio device switch sco,wait switch sucess to play");
                return;
            }
        }
#endif
        app_bt_manager.curr_playing_sco_id = device_id;
        //reject sniff req from remote dev
        bt_drv_reg_op_set_ibrt_reject_sniff_req(true);
        app_audio_adm_ibrt_handle_action(device_id,HFP_STREAMING_CHANGED,STRAEMING_START);
        app_audio_bt_update_call_state(device_id,false);
        app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START, BT_STREAM_VOICE, device_id, MAX_RECORD_NUM);
    }
}

void app_bt_audio_peer_sco_codec_received(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (besbt_cfg.vendor_codec_en && curr_device->this_sco_wait_to_play && curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
    {
        app_bt_audio_start_sco_playing(device_id);
    }

    curr_device->this_sco_wait_to_play = false;
}

static void app_bt_audio_stop_sco_playing(uint8_t device_id)
{
    if (app_bt_manager.curr_playing_sco_id == device_id)
    {
        app_bt_manager.curr_playing_sco_id = BT_DEVICE_INVALID_ID;
        bt_drv_reg_op_set_ibrt_reject_sniff_req(false);
        app_audio_adm_ibrt_handle_action(device_id,HFP_STREAMING_CHANGED,STRAEMING_STOP);
        app_audio_bt_update_call_state(device_id,false);
        app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP, BT_STREAM_VOICE, device_id, MAX_RECORD_NUM);
    }
}

static uint8_t app_bt_audio_create_sco_for_another_call_active_device(uint8_t device_id)
{
    uint8_t device_with_active_call = BT_DEVICE_INVALID_ID;

    device_with_active_call = app_bt_audio_select_another_device_to_create_sco(device_id);

    if (device_with_active_call != BT_DEVICE_INVALID_ID)
    {
        if (app_bt_audio_route_sco_path_back_to_earbud(device_with_active_call, true))
        {
            TRACE(2, "(d%x) %s create sco for waiting active call", device_with_active_call, __func__);
            return device_with_active_call;
        }
    }

    return BT_DEVICE_INVALID_ID;
}

#define CHECK_HF_PAUSE_A2DP_TIME    (1000)
void app_bt_audio_check_hf_pause_a2dp_timer_handler(void const *param)
{
    TRACE(0, "check_hf_pause_a2dp_timer timeout handle");
    struct BT_DEVICE_T* curr_device = (struct BT_DEVICE_T *)param;
    audio_focus_req_info_t* top_focus_info = NULL;
    top_focus_info = app_audio_focus_ctrl_stack_top();
    if (curr_device == NULL || curr_device->acl_is_connected == false || top_focus_info == NULL)
    {
        TRACE(0, "acl_dis curr %p top %p", curr_device, top_focus_info);
        return;
    }

    bool is_usage_call_or_ring_on_top = false;
    if ( (top_focus_info->device_info.audio_type  == AUDIO_TYPE_BT)
        && (top_focus_info->device_info.stream_type == USAGE_CALL || top_focus_info->device_info.stream_type == USAGE_RINGTONE))
    {
        is_usage_call_or_ring_on_top = true;
    }

    if (is_usage_call_or_ring_on_top == false)
    {
        TRACE(0, "User pause A2DP, aband a2dp focus");
        app_bt_audio_abandon_audio_focus(curr_device, USAGE_MEDIA);
        return;
    }

    if (top_focus_info->device_info.device_idx == curr_device->device_id)
    {
        TRACE(0, "d(%d) HF pause its A2DP, dont aband a2dp focus", curr_device->device_id);
        curr_device->is_hf_pause_its_a2dp = true;
    }
    else
    {
        TRACE(0, "d(%d) HF pause d(%d) A2DP, dont aband a2dp focus", top_focus_info->device_info.device_idx, curr_device->device_id);
    }
    return;
}

void app_bt_audio_check_a2dp_restreaming_timer_handler(void const *param)
{
    TRACE(0, "check_a2dp_restreaming_timer_handler");
    struct BT_DEVICE_T* curr_device = (struct BT_DEVICE_T *)param;
    audio_focus_req_info_t* top_focus_info = NULL;
    top_focus_info = app_audio_focus_ctrl_stack_top();

    if (curr_device == NULL || curr_device->acl_is_connected == false || top_focus_info == NULL)
    {
        TRACE(0, "acl_dis curr %p top %p", curr_device, top_focus_info);
        return;
    }

    if ( (top_focus_info->device_info.audio_type  == AUDIO_TYPE_BT)
         && (top_focus_info->device_info.stream_type == USAGE_MEDIA)
         && (top_focus_info->device_info.device_idx == curr_device->device_id)
         && (curr_device->a2dp_streamming == false))
    {
        TRACE(0, "d(%d) not streaming after granted, aband focus", curr_device->device_id);
        app_bt_audio_abandon_audio_focus(curr_device, USAGE_MEDIA);
        curr_device->is_hf_pause_its_a2dp = false;
    }
}

#define APP_BT_AUDIO_AVRCP_PLAY_STATUS_WAIT_TIMER_MS 500

static bool app_bt_audio_assume_play_status_notify_rsp_event_received(
    struct BT_DEVICE_T *curr_device, uint32_t curr_play_status, bool is_real_event, uint32_t *result_play_status)
{
    if (!is_real_event) // the timer is timeout before NOTIFY RSP event come
    {
        curr_device->avrcp_play_status_wait_to_handle = false;
        *result_play_status = curr_device->avrcp_palyback_status;
        return true;
    }
    else
    {
        // the NOTIFY RSP event come before timer timeout

        if (curr_device->avrcp_play_status_wait_to_handle)
        {
            curr_device->avrcp_play_status_wait_to_handle = false;

            // curr_device->avrcp_palyback_status is the previous play status

            if (curr_device->avrcp_palyback_status == curr_play_status)
            {
                *result_play_status = curr_play_status;
                return true;
            }

            *result_play_status = curr_play_status;
            return true;
        }
        else
        {
            return false;
        }
    }
}

static void app_bt_audio_event_avrcp_play_status_changed(struct BT_DEVICE_T *curr_device, uint32_t play_status)
{
    TRACE(2, "(d%x) avrcp play status timer generated event %d", curr_device->device_id, play_status);
    app_bt_audio_event_handler(curr_device->device_id, APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS, play_status);
}

void app_bt_audio_avrcp_play_status_wait_timer_callback(void const *param)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T *)param;
    uint32_t play_status = 0;
    if (app_bt_audio_assume_play_status_notify_rsp_event_received(curr_device, curr_device->avrcp_palyback_status, false, &play_status))
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)curr_device, play_status, (uint32_t)app_bt_audio_event_avrcp_play_status_changed);
    }
}

static bool app_bt_audio_event_judge_avrcp_play_status(
    uint8_t device_id, enum app_bt_audio_event_t event, uint32_t curr_play_status, uint32_t *result_play_status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    bool handle_play_status_event = false;

    if (event == APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED)
    {
        curr_device->avrcp_play_status_wait_to_handle = false;

        if (curr_play_status == BTIF_AVRCP_MEDIA_PAUSED)
        {
            // this pause play status may be incorrectly sent by phone, wait and recheck it
            curr_device->avrcp_play_status_wait_to_handle = true;
            osTimerStart(curr_device->avrcp_play_status_wait_timer, APP_BT_AUDIO_AVRCP_PLAY_STATUS_WAIT_TIMER_MS);
            handle_play_status_event = false;
        }
        else
        {
            *result_play_status = curr_play_status;
            handle_play_status_event = true;
        }
    }
    else
    {
        // the play status register NOTIFY RSP event come, stop the timer
        osTimerStop(curr_device->avrcp_play_status_wait_timer);
        handle_play_status_event = app_bt_audio_assume_play_status_notify_rsp_event_received(curr_device, curr_play_status, true, result_play_status);
    }

    TRACE(4, "(d%x) %s %d result %d", device_id, __func__, handle_play_status_event, *result_play_status);

    return handle_play_status_event;
}

static bool app_bt_audio_accept_new_call_active_sco(uint8_t device_id)
{
    // another device is sco connected, and current device is call active, it can accept current sco and disc other sco now
    uint8_t another_streaming_sco = app_bt_audio_select_another_streaming_sco(device_id);
    if (another_streaming_sco != BT_DEVICE_INVALID_ID)
    {
        app_bt_audio_route_sco_path_to_phone(another_streaming_sco); // after this sco disc, it will auto create sco for call active device
        app_bt_audio_wait_this_sco_become_connected(device_id, true);
        return true;
    }
    else
    {
        return false;
    }
}

void app_bt_audio_new_sco_is_rejected_by_controller(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device || curr_device->hfchan_call != BTIF_HF_CALL_ACTIVE)
    {
        return;
    }

    TRACE(4, "(d%x) sco rejected by controller: config %d%d sco_connect_times %d",
        device_id,
        app_bt_manager.config.sco_prompt_play_mode,
        app_bt_manager.config.second_sco_handle_mode,
        g_app_bt_audio_sco_connect_times_count);

    if (app_bt_manager.config.sco_prompt_play_mode && app_bt_manager.config.second_sco_handle_mode)
    {
        if (app_bt_audio_select_another_streaming_sco(device_id) == BT_DEVICE_INVALID_ID)
        {
            return;
        }

#if defined(IBRT_V2_MULTIPOINT)
        if (!app_tws_ibrt_is_profile_exchanged(&curr_device->remote))
        {
            return; // dont accept new sco before profile exchanged
        }
#endif

        if (g_app_bt_audio_sco_connect_times_count != 0)
        {
            return;
        }

        app_bt_audio_accept_new_call_active_sco(device_id);
    }
}

#define APP_BT_AUDIO_AVRCP_STATUS_QUICK_SWITCH_FILTER_TIMER_MS 2000

void app_bt_audio_avrcp_status_quick_switch_filter_timer_callback(void const *param)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T *)param;

    curr_device->filter_avrcp_pause_play_quick_switching = false;

    TRACE(3, "(d%x) avrcp pause/play status quick switch filtered %d %d",
            curr_device->device_id, curr_device->a2dp_streamming, curr_device->avrcp_palyback_status);

    if (curr_device->a2dp_streamming && curr_device->avrcp_palyback_status != BTIF_AVRCP_MEDIA_PLAYING)
    {
        if (tws_besaud_is_connected())
        {
            if (app_tws_ibrt_mobile_link_connected(&curr_device->remote))
            {
                app_ibrt_if_switch_streaming_a2dp();
            }
        }
        else
        {
            app_ibrt_if_switch_streaming_a2dp();
        }
    }
}

static bool app_bt_aduio_a2dp_start_filter_avrcp_quick_switch_timer(struct BT_DEVICE_T *curr_device)
{
    uint8_t curr_playing_a2dp = app_bt_audio_get_curr_playing_a2dp();
    bool timer_started = false;

    if (curr_playing_a2dp == curr_device->device_id)
    {
        osTimerStop(curr_device->avrcp_pause_play_quick_switch_filter_timer);

        osTimerStart(curr_device->avrcp_pause_play_quick_switch_filter_timer, APP_BT_AUDIO_AVRCP_STATUS_QUICK_SWITCH_FILTER_TIMER_MS);

        timer_started = true;
    }

    return timer_started;
}

static void app_bt_aduio_a2dp_stop_avrcp_pause_status_wait_timer(struct BT_DEVICE_T *curr_device)
{
    if (curr_device->filter_avrcp_pause_play_quick_switching)
    {
        curr_device->filter_avrcp_pause_play_quick_switching = false;
        osTimerStop(curr_device->avrcp_pause_play_quick_switch_filter_timer);
    }
}

#if defined(IBRT)
bool app_bt_audio_allow_send_profile_immediate(void *remote)
{
    if(remote == NULL)
    {
        return false;
    }
    if(app_bt_count_connected_device() < 2 || app_bt_manager.curr_playing_a2dp_id == BT_DEVICE_INVALID_ID)
    {
        return true;
    }
    struct BT_DEVICE_T *curr_play_device = app_bt_get_device(app_bt_manager.curr_playing_a2dp_id);
    uint8_t device_id = app_bt_get_device_id_byaddr((bt_bdaddr_t *)remote);
    if(device_id == app_bt_manager.curr_playing_a2dp_id)
    {
        return true;
    }
    else
    {
        if(app_tws_ibrt_is_profile_exchanged(&curr_play_device->remote))
        {
            return true;
        }
        else
        {
            TRACE(1,"(d%x)waiting another device send profile fristlly",device_id);
            return false;
        }
    }
}
#endif

#define APP_BT_SCO_REJECT_REQUEST 0
#define APP_BT_SCO_ACCEPT_REQUEST 1
#define APP_BT_SCO_LET_UPPER_LAYER_HANDLE 2
#define APP_BT_SCO_PACKET_HV3 0x02
#define APP_BT_ESCO_PACKET_EV3 0x07

#define PHONE_AUTO_PAUSE_A2DP_AND_CREATE_SCO_LINK_TIME_DIFF_MS 1000
#define PHONE_AUTO_START_A2DP_AFTER_SCO_LINK_DISC_TIME_DIFF_MS 2000

static void app_bt_audio_request_call_focus(struct BT_DEVICE_T *device)
{
    int req_focus_result = AUDIOFOCUS_REQUEST_FAILED;
    req_focus_result = app_bt_audio_request_audio_focus(device, AUDIOFOCUS_GAIN_TRANSIENT,USAGE_CALL,true);
    if(req_focus_result == AUDIOFOCUS_REQUEST_GRANTED)
    {
        if(device->hf_audio_state == BTIF_HF_AUDIO_CON)
        {
            app_bt_audio_start_sco_playing(device->device_id);
        }
        else
        {
            app_bt_audio_abandon_audio_focus(device, USAGE_CALL);
        }
    }
    else
    {
        app_bt_select_hfp_behavior_handler(device->device_id);
        TRACE(1,"%s failed", __func__);
    }
}

static void app_bt_audio_sco_request_to_play(struct BT_DEVICE_T *device)
{
    int req_focus_result = AUDIOFOCUS_REQUEST_FAILED;
    if(app_audio_get_curr_audio_stream_type(AUDIO_TYPE_BT,device->device_id) == USAGE_RINGTONE)
    {
         app_bt_audio_stop_self_ring_tone_play(device, false);
    }
    req_focus_result = app_bt_audio_request_audio_focus(device, AUDIOFOCUS_GAIN_TRANSIENT,USAGE_CALL,true);
    if(req_focus_result == AUDIOFOCUS_REQUEST_GRANTED)
    {
        app_bt_audio_start_sco_playing(device->device_id);
        device->call_audio_focus = AUDIOFOCUS_GAIN;
    }
    else
    {
        app_bt_select_hfp_behavior_handler(device->device_id);
        device->call_audio_focus = AUDIOFOCUS_LOSS_TRANSIENT;
    }
}

POSSIBLY_UNUSED static void app_bt_audio_push_request_blow_top_focus(struct BT_DEVICE_T *device)
{
    audio_focus_req_info_t request_info;
    memcpy((uint8_t*)&request_info.device_info.device_addr,(void*)&device->remote,sizeof(bt_bdaddr_t));
    request_info.device_info.device_idx = device->device_id;
    request_info.device_info.audio_type = AUDIO_TYPE_BT;
    request_info.device_info.focus_request_type = AUDIOFOCUS_GAIN;
    request_info.device_info.stream_type = USAGE_MEDIA;
    request_info.device_info.delayed_focus_allow = false;

    request_info.focus_changed_listener = device->a2dp_focus_changed_listener;
    request_info.ext_policy = app_bt_audio_music_ext_policy;
    app_audio_focus_insert_blow_top_focus(&request_info);
}

static void app_bt_audio_a2dp_request_to_play(struct BT_DEVICE_T *device)
{
    int req_focus_ret = AUDIOFOCUS_REQUEST_FAILED;

    if (device->a2dp_audio_focus == AUDIOFOCUS_GAIN)
    {
        app_bt_audio_start_a2dp_playing(device->device_id);
        return;
    }

    req_focus_ret = app_bt_audio_request_audio_focus(device,AUDIOFOCUS_GAIN,USAGE_MEDIA,
        app_bt_manager.config.mute_a2dp_stream || app_bt_manager.config.close_a2dp_stream || app_bt_manager.config.pause_a2dp_stream);

    if(req_focus_ret == AUDIOFOCUS_REQUEST_GRANTED)
    {
        app_bt_audio_start_a2dp_playing(device->device_id);
        device->this_is_bg_a2dp = false;
    }
    else
    {
        if(req_focus_ret == AUDIOFOCUS_REQUEST_DELAYED)
        {
            device->this_is_bg_a2dp = true;
            device->a2dp_audio_focus = AUDIOFOCUS_LOSS_TRANSIENT;
            TRACE(1,"audio_policy:delay play,set device %d as the bg a2dp",device->device_id);
        }
        else if(req_focus_ret == AUDIOFOCUS_REQUEST_FAILED)
        {
            device->a2dp_audio_focus = AUDIOFOCUS_LOSS;
        }
        TRACE(1,"d(%d) not allow play this time,disconnect/pause/mute on going",device->device_id);
        app_bt_audio_select_a2dp_behavior_handler(device->device_id);
    }
}

static void app_bt_audio_focus_empty_check_valid_stream(uint8_t device_id)
{
    struct BT_DEVICE_T* device = NULL;
    uint8_t another_device_id = BT_DEVICE_INVALID_ID;
    another_device_id = app_bt_audio_select_another_call_active_hfp(device_id);
    if(another_device_id != BT_DEVICE_INVALID_ID)
    {
        app_bt_audio_route_sco_path_back_to_earbud(another_device_id, true);
        TRACE(2,"d%x %s", another_device_id, __func__);
        return;
    }
    another_device_id = app_bt_audio_select_another_call_setup_hfp(device_id);
    if(another_device_id != BT_DEVICE_INVALID_ID)
    {
        device = app_bt_get_device(another_device_id);
        if(device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN &&
           !btif_hf_is_inbandring_enabled(device->hf_channel))
        {
            app_bt_audio_ringtone_request_focus(device);
            TRACE(1,"d%x focus_empty_request_ringtone", another_device_id);
        }
        else
        {
            app_bt_audio_route_sco_path_back_to_earbud(another_device_id, true);
        }
        return;
    }
    another_device_id = app_bt_audio_select_another_streaming_a2dp(device_id);
    if(another_device_id != BT_DEVICE_INVALID_ID)
    {
        device = app_bt_get_device(another_device_id);
        TRACE(1,"d%x Tiktok or tencent request focus again!",another_device_id);
        app_bt_audio_a2dp_request_to_play(device);
    }
}

#define APP_BT_AUDIO_SELF_PLAY_RING_TONE_INTERVAL 5000
void app_bt_audio_self_ring_tone_play_timer_handler(void const *param)
{
    struct BT_DEVICE_T* curr_device = (struct BT_DEVICE_T *)param;
    if(curr_device == NULL)
    {
        return;
    }
    if(curr_device->ignore_ring_and_play_tone_self)
    {
        TRACE(0,"app_bt_audio_self_ring_tone_play");
        app_bt_audio_ringtone_request_focus(curr_device);
        osTimerStop(curr_device->self_ring_tone_play_timer);
        osTimerStart(curr_device->self_ring_tone_play_timer, APP_BT_AUDIO_SELF_PLAY_RING_TONE_INTERVAL);
    }
}

void app_bt_audio_stop_self_ring_tone_play(void* device, bool stop_all)
{
    struct BT_DEVICE_T *other_device = NULL;
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T*)device;
    TRACE(1,"[audio_policy] d%x tigger stop ring", curr_device->device_id);
    if(curr_device->ignore_ring_and_play_tone_self)
    {
        curr_device->ignore_ring_and_play_tone_self =false;
        osTimerStop(curr_device->self_ring_tone_play_timer);
        app_audio_adm_ibrt_handle_action(curr_device->device_id, RINGTONE_CHANGED, RINGTONE_STOP);
#if !defined(FPGA) && defined(MEDIA_PLAYER_SUPPORT)
#if defined (PROMPT_SELF_MANAGEMENT)
        app_stop_both_prompt_playing();
#else
        app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP_MEDIA, BT_STREAM_MEDIA, curr_device->device_id, 0);
#endif
#endif
    }
    if(stop_all)
    {
        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            if (i == curr_device->device_id)
            {
                continue;
            }
            other_device = app_bt_get_device(i);
            if (other_device->ignore_ring_and_play_tone_self)
            {
                other_device->ignore_ring_and_play_tone_self =false;
                osTimerStop(other_device->self_ring_tone_play_timer);
                app_audio_adm_ibrt_handle_action(curr_device->device_id, RINGTONE_CHANGED, RINGTONE_STOP);
#if !defined(FPGA) && defined(MEDIA_PLAYER_SUPPORT)
#if defined (PROMPT_SELF_MANAGEMENT)
                app_stop_both_prompt_playing();
#else
                app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP_MEDIA, BT_STREAM_MEDIA, curr_device->device_id, 0);
#endif
#endif
            }
        }
    }
}

void app_bt_audio_self_ring_tone_play_check(struct BT_DEVICE_T *curr_device)
{
    if(curr_device && curr_device->hf_audio_state == BTIF_HF_AUDIO_DISCON && curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN)
    {
        curr_device->ignore_ring_and_play_tone_self = true;
        app_bt_audio_self_ring_tone_play_timer_handler(curr_device);
    }
}

bool app_bt_audio_current_device_is_hfp_idle_state(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(curr_device->hf_conn_flag && curr_device->hf_audio_state == BTIF_HF_AUDIO_DISCON && curr_device->hfchan_call == BTIF_HF_CALL_NONE
        && curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_NONE && curr_device->hf_callheld == BTIF_HF_CALL_HELD_NONE){
        return true;
    }
    return false;
}

void app_bt_audio_hfp_get_clcc_timeout_handler(void const *param)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T*)param;
    if(curr_device && curr_device->is_accepting_sco_request)
    {
        if(!app_bt_audio_count_connected_sco())
        {
            TRACE(1,"d%x get clcc timeout accept sco", curr_device->device_id);
            btif_me_accept_pending_sco_request(curr_device->device_id);
        }
        else
        {
            app_bt_audio_sco_request_to_play(curr_device);
            TRACE(2,"d%x clcc_timeout_focus_gain:%d", curr_device->device_id, curr_device->call_audio_focus);
            if (curr_device->call_audio_focus == AUDIOFOCUS_GAIN)
            {
                app_bt_audio_wait_this_sco_become_connected(curr_device->device_id, true);
            }
            else
            {
                btif_me_finish_pending_sco_request((void*)&curr_device->remote);
            }
        }
    }
}

static int app_bt_handle_second_sco_request(uint8_t device_id)
{
    BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (app_bt_manager.config.sco_prompt_play_mode)
    {
       if (curr_device->hfchan_call != BTIF_HF_CALL_ACTIVE &&
           curr_device->hfchan_callSetup != BTIF_HF_CALL_SETUP_OUT &&
           curr_device->hfchan_callSetup != BTIF_HF_CALL_SETUP_ALERT)
       {
           return APP_BT_SCO_REJECT_REQUEST; // reject new sco before call active, let user to choice answer the new call or not
       }
       else
       {
           osTimerStop(curr_device->clcc_timer);
           app_hf_send_current_call_at_commond(curr_device->hf_channel);
           osTimerStart(curr_device->clcc_timer, APP_BT_HFP_GET_CLCC_TIMEOUT);
           curr_device->is_accepting_sco_request = true;
           return APP_BT_SCO_LET_UPPER_LAYER_HANDLE;
       }
    }
    else
    {
        return APP_BT_SCO_REJECT_REQUEST; // older sco is not interrupted, reject new sco
    }
}

int app_bt_audio_handle_sco_req(uint8_t device_id)
{
    BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

#if defined(IBRT)
    uint8_t sco_request_id = 0xff;
    sco_request_id = app_tws_ibrt_get_remote_sync_id_by_bdaddr(&curr_device->remote);
    if(app_bt_manager.config.host_reject_unexcept_sco_packet)
    {
        uint8_t sco_type = bt_drv_reg_op_get_sco_type(sco_request_id);
        if(sco_type == APP_BT_SCO_PACKET_HV3 || sco_type == APP_BT_ESCO_PACKET_EV3)
        {
            TRACE(0,"###host reject hv3 or ev3=%d packet sco", sco_type);
            return APP_BT_SCO_REJECT_REQUEST;
        }
    }
#endif
#ifdef SASS_ENABLED
    if(app_bt_hf_get_reject_dev() == device_id)
    {
        return APP_BT_SCO_REJECT_REQUEST;
    }
#endif
    if (!app_bt_audio_count_connected_sco())
    {
        if (app_bt_audio_count_request_waiting_sco())
        {
             goto handle_2nd_sco_request;
        }
        curr_device->is_accepting_sco_request = true;
        return APP_BT_SCO_ACCEPT_REQUEST;
    }
handle_2nd_sco_request:
    return app_bt_handle_second_sco_request(device_id);
}

void app_bt_audio_handle_sco_connected_event(uint8_t device_id)
{
    BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    TRACE(1,"d%x audio_handle_evt: sco connect", device_id);
    if(app_bt_audio_count_connected_sco() > 1)
    {
        app_bt_sco_switch_trigger_init(app_bt_audio_sco_switch_trigger);
    }

    if (app_bt_manager.device_routed_sco_to_phone == device_id)
    {
        app_bt_manager.device_routed_sco_to_phone = BT_DEVICE_INVALID_ID;
    }
    if (app_bt_manager.device_routing_sco_back == device_id)
    {
        app_bt_manager.device_routing_sco_back = BT_DEVICE_INVALID_ID;
    }
    if(app_bt_manager.wait_sco_connected_device_id == device_id)
    {
        app_bt_manager.wait_sco_connected_device_id = BT_DEVICE_INVALID_ID;
    }
    if (curr_device->disc_sco_when_it_connected)
    {
        TRACE(0, "(d%x) sco connected and disc_sco_when_it_connected", device_id);
        app_bt_audio_route_sco_path_to_phone(device_id);
    }

    curr_device->is_accepting_sco_request = false;
    curr_device->disc_sco_when_it_connected = false;
    if(app_bt_manager.curr_playing_sco_id != BT_DEVICE_INVALID_ID &&
       curr_device->ignore_ring_and_play_tone_self &&
       curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN){
        TRACE(0,"dont request sco focus when incoming_incoming");
        return;
    }
    if(app_bt_audio_count_connected_hfp() < 1){
        app_bt_audio_sco_request_to_play(curr_device);
    }else{
        if(app_bt_audio_get_curr_playing_sco() != BT_DEVICE_INVALID_ID){
            if(curr_device->hfchan_call == BTIF_HF_CALL_NONE &&
               curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_NONE){
                TRACE(0,"dont allow request focus when call not invalid");
                return;
            }
            app_bt_audio_sco_request_to_play(curr_device);
        }else{
            app_bt_audio_sco_request_to_play(curr_device);
        }
    }
}

void app_bt_audio_handle_hfp_callsetup_event(uint8_t device_id)
{
    BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    TRACE(2,"d%x audio_handle_evt:hfp_callsetup: %d", device_id, curr_device->hfchan_callSetup);
    if (!app_bt_is_hfp_connected(device_id))
    {
        return;
    }
    if(curr_device->hfchan_callSetup
       && btif_hf_is_inbandring_enabled(curr_device->hf_channel) == true
       && app_bt_manager.config.sco_prompt_play_mode == false
       && app_bt_manager.config.second_sco_handle_mode == IBRT_HOST_DECIDE_SECONED_SCO)
    {
        app_bt_audio_request_call_focus(curr_device);
    }
    if (curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN)
    {

#ifdef BT_USB_AUDIO_DUAL_MODE
        if(!btusb_is_bt_mode())
        {
            TRACE(0,"audio_policy:btusb_usbaudio_close doing.");
            btusb_usbaudio_close();
        }
#endif
#if !defined(FPGA) && defined(MEDIA_PLAYER_SUPPORT)
        audio_focus_req_info_t *curr_focus = app_audio_get_curr_audio_focus();
        if(curr_focus)
        {
            if(app_bt_manager.config.allow_duck_ringtone)
            {
                if(curr_focus->device_info.stream_type == USAGE_CALL && curr_focus->device_info.device_idx != device_id)
                {
                    app_bt_audio_self_ring_tone_play_check(curr_device);
                }
                else if(btif_hf_is_inbandring_enabled(curr_device->hf_channel) == false)
                {
                    /*A play music,A incoming call but A2DP suspend slowly*/
                    app_bt_audio_self_ring_tone_play_check(curr_device);
                }
            }
        }
        else
        {
            if(!btif_hf_is_inbandring_enabled(curr_device->hf_channel))
            {
                app_bt_audio_self_ring_tone_play_check(curr_device);
                TRACE(0,"audio_policy:start outbandring normally");
            }
            else
            {
                TRACE(0,"audio_policy:waiting sco connected");
            }
        }
#endif
    }
    else if (curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_NONE)
    {
        curr_device->ring_audio_focus = AUDIOFOCUS_NONE;
        if (curr_device->hfchan_call == BTIF_HF_CALL_ACTIVE)
        {
            // call is answered,frist call active,then callsetup none
            app_bt_audio_stop_self_ring_tone_play(curr_device, false);
            TRACE(1,"audio_policy:(d%x) !!!HF_EVENT_CALLSETUP_IND ANSWERCALL\n", device_id);
        }
        else
        {
            // call is refused,call is none,now callsetup none
            btif_hf_set_virtual_call_disable(curr_device->hf_channel);
            app_bt_audio_stop_self_ring_tone_play(curr_device,false);
            TRACE(1,"audio_policy:(d%x) !!!HF_EVENT_CALLSETUP_IND REFUSECALL\n", device_id);
            if(curr_device->hf_audio_state == BTIF_HF_AUDIO_DISCON)
            {
                app_bt_audio_abandon_call_focus(curr_device);
            }
        }
    }
}

void app_bt_audio_handle_hfp_call_event(uint8_t device_id)
{
    BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    TRACE(2,"d%x audio_handle_evt:hfp_call:%d", device_id, curr_device->hfchan_call);
    if (!app_bt_is_hfp_connected(device_id))
    {
        return;
    }

    if(curr_device->hfchan_call == BTIF_HF_CALL_ACTIVE)
    {
        if(app_audio_get_curr_audio_stream_type(AUDIO_TYPE_BT,curr_device->device_id) == USAGE_RINGTONE){
             app_bt_audio_abandon_call_focus(curr_device);
            app_bt_audio_stop_self_ring_tone_play(curr_device, true);
            TRACE(1, "audio_policy:(d%x) ANSWERCALL, stop no in-band ring\n", device_id);
        }
        if (curr_device->hf_audio_state) {
            curr_device->hfp_call_active_time = Plt_GetTicks();
            app_bt_audio_sco_request_to_play(curr_device);
        }
    }
    else if(app_bt_audio_current_device_is_hfp_idle_state(device_id))
    {
        TRACE(1,"audio_policy:(d%x) !!!HF_EVENT_CALL_IND HANGUPCALL\n", device_id);
        /* sco disconnected doesn't means call is not end */
        app_bt_audio_abandon_call_focus(curr_device);
    }
}

#define PHONE_AUTO_DISC_SCO_AFTER_CALL_ACTIVE_TIME_DIFF 1000
void app_bt_audio_handle_sco_disconnect_event(uint8_t device_id)
{
    BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    TRACE(2, "(d%x) sco disconnected wait connect sco d%x", device_id, app_bt_manager.wait_sco_connected_device_id);
    curr_device->is_accepting_sco_request = false;
    curr_device->disc_sco_when_it_connected = false;
    if(app_bt_audio_count_connected_sco() < 2)
    {
        app_bt_sco_switch_trigger_deinit();
    }
    if (app_bt_manager.wait_sco_connected_device_id != BT_DEVICE_INVALID_ID && app_bt_manager.wait_sco_connected_device_id != device_id)
    {
        btif_me_accept_pending_sco_request(app_bt_manager.wait_sco_connected_device_id);
    }
    app_bt_audio_stop_sco_playing(device_id);
    if(curr_device->call_audio_focus == AUDIOFOCUS_GAIN)
    {
        app_bt_audio_abandon_call_focus(curr_device);
        TRACE(0,"route sco by phone abandon it!");
    }
    else if(!app_bt_audio_current_device_is_hfp_idle_state(device_id) &&
             app_audio_focus_ctrl_stack_length() >= 2)
    {
        app_bt_audio_call_force_route_to_phone(curr_device);
    }
    else
    {
        // such as wechat ...
        if(app_bt_audio_current_device_is_hfp_idle_state(device_id) &&
           (app_audio_get_curr_audio_stream_type(AUDIO_TYPE_BT,curr_device->device_id) == USAGE_CALL ||
            app_audio_get_curr_audio_stream_type(AUDIO_TYPE_BT,curr_device->device_id) == USAGE_RINGTONE))
        {
            TRACE(3,"audio_policy:d(%d) call status = %d,setup = %d,stack len = %d",
            curr_device->device_id,curr_device->hfchan_call,curr_device->hfchan_callSetup,app_audio_focus_ctrl_stack_length());
            app_bt_audio_abandon_call_focus(curr_device);
        }
    }

    if (app_bt_manager.config.reconn_sco_if_fast_disc_after_call_active_for_iphone_auto_mode &&
        curr_device->hfchan_call && !app_bt_audio_count_connected_sco() && !app_bt_audio_count_request_waiting_sco()) {
        uint32_t diff_time = TICKS_TO_MS(Plt_GetTicks() - curr_device->hfp_call_active_time);
        TRACE(0, "(d%x) sco disconnected with time diff %dms", device_id, diff_time);
        if (diff_time < PHONE_AUTO_DISC_SCO_AFTER_CALL_ACTIVE_TIME_DIFF) {
            app_bt_audio_route_sco_path_back_to_earbud(device_id, true);
        }
    }

#if defined(BT_WATCH_APP)
    app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_HF_SCO_DISCONNECTED, NULL);
#endif
}

#if defined(IBRT_V2_MULTIPOINT)
static void app_bt_request_switch_sco_background(struct BT_DEVICE_T *device)
{
    int req_focus_ret = AUDIOFOCUS_REQUEST_FAILED;
    if(device->a2dp_audio_focus != AUDIOFOCUS_GAIN && app_audio_get_curr_audio_focus_type() == USAGE_CALL)
    {
        req_focus_ret = app_bt_audio_switch_call_focus(device,AUDIOFOCUS_GAIN_TRANSIENT,USAGE_CALL,true);
    }
    if(req_focus_ret == AUDIOFOCUS_REQUEST_GRANTED)
    {
        //app_bt_audio_start_sco_playing(device->device_id); //wait switch complete event to start play
        app_bt_send_hold_command_to_phone(device->device_id, true);
    }
}
#endif

int app_bt_audio_event_handler(uint8_t device_id, enum app_bt_audio_event_t event, uint32_t data)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    uint32_t avrcp_play_status = data;
    int sco_handle_result;
    uint8_t another_call_device = BT_DEVICE_INVALID_ID;

    if (event == APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED || event == APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_NOTIFY_RSP)
    {
        uint32_t play_status = 0;
        if (!app_bt_audio_event_judge_avrcp_play_status(device_id, event, data, &play_status))
        {
            return 0; // skip this avrcp status event
        }
        else
        {
            event = APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS;
            avrcp_play_status = play_status; // continue handle the avrcp status event
        }
    }
    TRACE(1, "avrcp status:%d", avrcp_play_status);
    TRACE(2, "app_bt_audio_event_handler event :d(%d),event:0x%x",device_id,event);
    switch (event)
    {
    case APP_BT_AUDIO_EVENT_A2DP_STREAM_OPEN:
        {
            BT_AUDIO_DEVICE_T *active_device = app_audio_adm_get_active_device();
            if (active_device->device_id == device_id)
            {
                app_audio_ctrl_update_bt_music_state(device_id,event);
                //bt_media_current_music_set(active_device->device_id);
            }

            if(app_bt_manager.config.close_a2dp_stream &&
            !curr_device->a2dp_streamming &&
            curr_device->auto_make_remote_play &&
            curr_device->a2dp_audio_focus == AUDIOFOCUS_GAIN)
            {
                curr_device->auto_make_remote_play = false;
#if defined(IBRT_V2_MULTIPOINT)
            app_bt_ibrt_audio_play_a2dp_stream(device_id);
#else
            app_bt_a2dp_send_key_request(device_id, AVRCP_KEY_PLAY);
#endif
            }
        }
        break;
    case APP_BT_AUDIO_EVENT_PROFILE_EXCHANGED:
#ifdef IBRT
        if((app_bt_audio_both_profile_exchanged() ||
            app_audio_focus_ctrl_stack_length() >= 2) &&
            app_tws_get_ibrt_role(&curr_device->remote) == IBRT_MASTER)
        {
            app_bt_handle_profile_exchanged_done();
        }
#endif
        break;
    case APP_BT_AUDIO_EVENT_IBRT_STARTED:
        app_bt_audio_enable_active_link(true, app_bt_audio_get_curr_playing_a2dp());
        app_audio_adm_ibrt_handle_action(device_id,DEVICE_CONN_STATUS_CHANGED,IBRT_CONNECTED);
        break;
    case APP_BT_AUDIO_EVENT_A2DP_STREAM_MOCK_START:
        /* fall through */
    case APP_BT_AUDIO_EVENT_A2DP_STREAM_START:
        curr_device->is_hf_pause_its_a2dp = false;
        osTimerStop(curr_device->check_a2dp_restreaming_timer);
        osTimerStop(curr_device->check_hf_pause_a2dp_timer);
music_player_status_is_playing:
        if(curr_device->this_is_bg_a2dp &&
           app_audio_focus_ctrl_stack_length() > 0 &&
           event == APP_BT_AUDIO_EVENT_A2DP_STREAM_MOCK_START)
        {
            TRACE(1,"audio_policy:dont allow prmopt bg a2dp");
            app_bt_audio_push_request_blow_top_focus(curr_device);
            break;
        }
        curr_device->this_is_bg_a2dp = false;
        curr_device->a2dp_streaming_available = true;
        app_bt_stop_audio_focus_timer(device_id);
        app_bt_audio_a2dp_request_to_play(curr_device);
        if(app_bt_audio_count_streaming_a2dp() > 1 && app_bt_manager.curr_playing_a2dp_id != BT_DEVICE_INVALID_ID)
        {
            a2dp_audio_sysfreq_update_normalfreq();
            TRACE(0,"audio_policy:more than one a2dp streaming increase freq");
        }
        break;
    case APP_BT_AUDIO_EVENT_A2DP_STREAM_CLOSE:
        curr_device->is_hf_pause_its_a2dp = false;
        curr_device->a2dp_streaming_available = false;
        app_bt_stop_audio_focus_timer(device_id);
        a2dp_audio_sysfreq_update_normalfreq();
        // if select close new a2dp,don't abandon this audio focus
        if(app_bt_manager.config.close_a2dp_stream && curr_device->this_is_bg_a2dp)
        {
            TRACE(1, "audio_policy:(d%x) don't abandon this focus from stack",device_id);
        }
        else
        {
            curr_device->this_is_bg_a2dp = false;
            app_bt_audio_stop_a2dp_playing(device_id);
            app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
        }
        break;
    case APP_BT_AUDIO_EVENT_A2DP_STREAM_SUSPEND:
        curr_device->is_hf_pause_its_a2dp = false;
        curr_device->a2dp_streaming_available = false;
        a2dp_audio_sysfreq_update_normalfreq();
        app_bt_aduio_a2dp_stop_avrcp_pause_status_wait_timer(curr_device);
music_player_status_is_paused:
        if(event == APP_BT_AUDIO_EVENT_A2DP_STREAM_SUSPEND)
        {
            app_bt_audio_stop_a2dp_playing(device_id);
            if(app_bt_manager.config.mute_a2dp_stream)
            {
                TRACE (0, "start check_hf_pause_a2dp_timer");
                //Delay determining whether to discard focus
                osTimerStart(curr_device->check_hf_pause_a2dp_timer, CHECK_HF_PAUSE_A2DP_TIME);
            }
            else if((app_bt_manager.config.pause_a2dp_stream
                     || app_bt_manager.config.pause_a2dp_when_call_exist)
                     && (!curr_device->this_is_paused_bg_a2dp))
            {
                TRACE(0,"Phone auto suspend abandon focus!");
                app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
            }
        }
        else
        {
            if(curr_device->a2dp_audio_focus == AUDIOFOCUS_GAIN &&
               app_bt_audio_count_connected_a2dp() > 1 &&
               app_audio_focus_ctrl_stack_length() > 1)
            {
                app_bt_audio_stop_a2dp_playing(device_id);
                app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
            }
        }
        break;
    case APP_BT_AUDIO_EVENT_A2DP_STREAM_SWITCH:
        app_bt_audio_a2dp_request_to_play(curr_device);
        break;
    case APP_BT_AUDIO_EVENT_AVRCP_CONNECTED:
    case APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED_MOCK:
#if BLE_AUDIO_ENABLED
         app_audio_ctrl_update_bt_music_state(device_id, STATE_CONNECTED);
#endif
         break;
    case APP_BT_AUDIO_EVENT_AVRCP_IS_REALLY_PAUSED:
        if(app_bt_manager.config.mute_a2dp_stream || app_bt_manager.config.close_a2dp_stream)
        {
            app_bt_audio_stop_a2dp_playing(device_id);
            app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
        }
        break;
    case APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS:
        /* fall through */
    case APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_MOCK:
        if (avrcp_play_status == BTIF_AVRCP_MEDIA_PAUSED)
        {
            /**
             * after pause iphone music player, it takes very long to suspend a2dp stream,
             * but the playback pause status is updated very soon via avrcp, use this info
             * can speed up the switch to other playing a2dp stream.
             *
             * music_player_status_is_paused is used to switch between multiple a2dp quickly,
             * if there is only one a2dp stream, no need to use it due to the phone may send
             * wrong media paused status.
             *
             * If there is a2dp need to resume, we may trigger this event in order to
             * resume quickly: `app_bt_audio_select_bg_a2dp_to_resume() != BT_DEVICE_INVALID_ID`.
             * But there is a compatibility issue case:
             * 1. d0 play music, and d1 play music
             * 2. stop d0 playing and pause d0
             * 3. but d1 send incorrect avrcp PAUSE status
             * 4. this will resume to play d0 and d1 is still playing
             */
            app_audio_ctrl_update_bt_music_state(device_id, AVRCP_PAUSED);
            if (app_bt_count_connected_device() < 2)
            {
                break;
            }
			curr_device->waiting_pause_suspend = false;

            if (app_bt_audio_count_streaming_a2dp() > 1)
            {
                TRACE(1, "(d%x) music_player_status_is_paused", device_id);

                // filter avrcp puase/play quick status switching, such as select to play next song on the phone
                if (app_bt_aduio_a2dp_start_filter_avrcp_quick_switch_timer(curr_device))
                {
                    curr_device->filter_avrcp_pause_play_quick_switching = true;
                }
                else
                {
                    curr_device->filter_avrcp_pause_play_quick_switching = false;
                }

                if (app_bt_manager.config.keep_only_one_stream_close_connected_a2dp)
                {
                    // If phone send wrong pause status, and the a2dp is not suspended in N seconds, this a2dp will replay
                    // The waiting time shall be long enough, because if it is too short, it will replay quickly before it suspended,
                    // d1 is try reconnecting a2dp profile, this will lead disconnect d1's connecting profile, and d1 music played on the phone
                    app_bt_audio_a2dp_streaming_recheck_timer_start(device_id, APP_BT_AUDIO_A2DP_WAIT_PAUSED_STREAM_SUSPEND);
                }

                if (false == curr_device->filter_avrcp_pause_play_quick_switching)
                {
                    goto music_player_status_is_paused;
                }
            }
        }
        else if (avrcp_play_status == BTIF_AVRCP_MEDIA_PLAYING)
        {
            app_audio_ctrl_update_bt_music_state(device_id, AVRCP_PLAYING);
            app_bt_aduio_a2dp_stop_avrcp_pause_status_wait_timer(curr_device);
            if(curr_device->a2dp_streamming)
            {
                goto music_player_status_is_playing;
            }
        }
        break;
    case APP_BT_AUDIO_EVENT_HFP_SERVICE_CONNECTED:
        curr_device->ignore_ring_and_play_tone_self =false;
        curr_device->sco_audio_prio = 0;
        if(!btif_hf_is_inbandring_enabled(curr_device->hf_channel))
        {
            app_bt_audio_self_ring_tone_play_check(curr_device);
        }
        break;
    case APP_BT_AUDIO_EVENT_HFP_SCO_CONNECT_REQ:
        sco_handle_result = app_bt_audio_handle_sco_req(device_id);
        TRACE(2,"(d%x)sco_req_result: %d",device_id, sco_handle_result);
        return sco_handle_result;

    case APP_BT_AUDIO_EVENT_HFP_SCO_CONNECTED:
        curr_device->sco_audio_prio = app_bt_audio_create_new_prio();
        TRACE(2,"audio_policy:d(%d)call setup status = %d",curr_device->device_id,curr_device->hfchan_callSetup);
#if defined(BT_WATCH_APP)
        app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_HF_SCO_CONNECTED, NULL);
#endif
        app_bt_audio_handle_sco_connected_event(device_id);
        break;
    case APP_BT_AUDIO_EVENT_HFP_SERVICE_DISCONNECTED:
        if (app_bt_manager.device_routed_sco_to_phone == device_id)
        {
            app_bt_manager.device_routed_sco_to_phone = BT_DEVICE_INVALID_ID;
        }
        if (app_bt_manager.device_routing_sco_back == device_id)
        {
            app_bt_manager.device_routing_sco_back = BT_DEVICE_INVALID_ID;
        }
        app_bt_audio_stop_self_ring_tone_play(curr_device, false);
        /* fall through */
    case APP_BT_AUDIO_EVENT_HFP_SCO_DISCONNECTED:
        curr_device->sco_audio_prio = 0;
        app_bt_audio_handle_sco_disconnect_event(device_id);
        break;
    case APP_BT_AUDIO_EVENT_HFP_CCWA_IND:
        break;
    case APP_BT_AUDIO_EVENT_HFP_RING_IND:
        break;
    case APP_BT_AUDIO_EVENT_HFP_CLCC_IND:
#if !defined(FPGA) && defined(MEDIA_PLAYER_SUPPORT) && defined(__BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__)
        if (curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN)
        {
            media_PlayAudio(AUD_ID_RING_WARNING, device_id);
        }
#endif
        if(curr_device->is_accepting_sco_request)
        {
            app_bt_audio_sco_request_to_play(curr_device);
            if (curr_device->call_audio_focus == AUDIOFOCUS_GAIN)
            {
                if(!app_bt_audio_count_connected_sco())
                {
                    btif_me_accept_pending_sco_request(device_id);
                }
                else
                {
                    app_bt_audio_wait_this_sco_become_connected(device_id, true);
                }
            }
            else
            {
                btif_me_finish_pending_sco_request((void*)&curr_device->remote);
            }
        }
        break;
    case APP_BT_AUDIO_EVENT_HFP_CALLSETUP_IND:
    case APP_BT_AUDIO_EVENT_HFP_CALLSETUP_MOCK:
        app_bt_audio_handle_hfp_callsetup_event(device_id);
        app_audio_bt_update_call_state(device_id,true);
        break;
    case APP_BT_AUDIO_EVENT_HFP_CALL_IND:
    case APP_BT_AUDIO_EVENT_HFP_CALL_MOCK:
        app_bt_audio_handle_hfp_call_event(device_id);
        app_audio_bt_update_call_state(device_id,false);
        break;
    case APP_BT_AUDIO_EVENT_HFP_CALLHELD_IND:
    case APP_BT_AUDIO_EVENT_HFP_CALLHOLD_MOCK:
        another_call_device = app_bt_audio_select_another_call_setup_hfp(device_id);
        if(curr_device->call_audio_focus == AUDIOFOCUS_GAIN)
        {
            if(curr_device->hf_callheld == BTIF_HF_CALL_HELD_NO_ACTIVE && another_call_device != BT_DEVICE_INVALID_ID)
            {
                app_bt_audio_stop_sco_playing(device_id);
                if(app_bt_get_device(another_call_device)->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN)
                {
                    btif_hf_answer_call(app_bt_get_device(another_call_device)->hf_channel);
                }
            }
        }
        app_audio_bt_update_call_state(device_id,false);
        break;
    case APP_BT_AUDIO_EVENT_ROUTE_CALL_TO_LE:
#if BLE_AUDIO_ENABLED
        app_bt_audio_abandon_call_focus(curr_device);
#endif
        break;
    case APP_BT_AUDIO_EVENT_ACL_CONNECTED:
        app_audio_adm_ibrt_handle_action(device_id,DEVICE_CONN_STATUS_CHANGED,STATE_CONNECTED);
        break;
    case APP_BT_AUDIO_EVENT_ACL_DISC:
        curr_device->is_hf_pause_its_a2dp = false;
        osTimerStop(curr_device->check_hf_pause_a2dp_timer);
        app_audio_adm_ibrt_handle_action(device_id,DEVICE_CONN_STATUS_CHANGED,STATE_DISCONNECTED);
        app_bt_audio_stop_sco_playing(device_id);
        app_bt_audio_stop_a2dp_playing(device_id);
        app_bt_audio_abanon_focus(curr_device);
        break;
    case APP_BT_AUDIO_EVENT_AUTHENTICATED:
        app_audio_adm_ibrt_handle_action(device_id,DEVICE_CONN_STATUS_CHANGED,STATE_CONNECTED);
        break;
    case APP_BT_AUDIO_EVENT_HUNGUP_ACTIVE_CALL:
    case APP_BT_AUDIO_EVENT_HUNGUP_INCOMING_CALL:
        app_audio_bt_update_call_state(device_id,true);
        break;
    case APP_BT_AUDIO_HOLD_SWICTH_BACKGROUND:
#if defined(IBRT_V2_MULTIPOINT)
        app_bt_request_switch_sco_background(curr_device);
#endif
        break;
    case APP_BT_AUDIO_BG_TO_FG_COMPLETE:
        if(app_bt_manager.curr_playing_sco_id != BT_DEVICE_INVALID_ID)
        {
            app_bt_audio_stop_sco_playing(app_bt_manager.curr_playing_sco_id);
        }
        if(curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
        {
            curr_device->sco_audio_prio = 0;
            app_bt_audio_start_sco_playing(device_id);
        }
        break;
    default:
        break;
    }

    return 0;
}

void app_bt_audio_select_a2dp_behavior_handler(uint8_t device_id)
{
#if BT_DEVICE_NUM > 1
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(app_bt_manager.config.pause_a2dp_when_call_exist &&
       (app_audio_get_curr_audio_focus_type() == USAGE_CALL || app_audio_get_curr_audio_focus_type() == USAGE_RINGTONE))
    {
        curr_device->this_is_paused_bg_a2dp = true;
#if defined(IBRT_V2_MULTIPOINT)
        app_bt_ibrt_audio_pause_a2dp_stream(device_id);
#else
        app_bt_a2dp_send_key_request(device_id, AVRCP_KEY_PAUSE);
#endif
        TRACE(0,"[audio_policy]:pause a2dp when call exist");
    }

    if (app_bt_manager.config.pause_a2dp_stream)
    {
        curr_device->this_is_paused_bg_a2dp = true;
#if defined(IBRT_V2_MULTIPOINT)
        app_bt_ibrt_audio_pause_a2dp_stream(device_id);
        curr_device->waiting_pause_suspend = true;
#else
        app_bt_a2dp_send_key_request(device_id, AVRCP_KEY_PAUSE);
#endif
    }
    else if (app_bt_manager.config.close_a2dp_stream)
    {
        app_bt_audio_a2dp_close_this_profile(device_id);
    }
#endif
}

#if defined(IBRT_V2_MULTIPOINT)
void app_bt_select_hfp_behavior_handler(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(app_tws_ibrt_mobile_link_connected(&curr_device->remote))
    {
        if(app_bt_manager.config.second_sco_bg_action == IBRT_ACTION_ROUTE_SCO_TO_PHONE)
        {
            if(curr_device->hf_audio_state == BTIF_HF_AUDIO_CON)
            {
                app_bt_audio_route_sco_path_to_phone(device_id);
            }
        }
        else if(app_bt_manager.config.second_sco_bg_action == IBRT_ACTION_HOLD_ACTIVE_SCO)
        {
            if(curr_device->hfchan_call && curr_device->hf_callheld == BTIF_HF_CALL_HELD_NONE){
                app_bt_send_hold_command_to_phone(device_id, false);
                TRACE(1,"d%x hold active call",device_id);
            }else{
                TRACE(0,"d%x wait call active or hungup", device_id);
            }
        }
    }
}
#endif

void app_bt_on_a2dp_audio_focus_change(uint8_t device_id,AUDIO_USAGE_TYPE_E media_type,int audio_focus_change)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    TRACE(3,"audio_policy:(d%x) a2dp_audio_focus_change:focus changed %s,streaming = %d",
         device_id,app_audio_focus_change_to_string(audio_focus_change),curr_device->a2dp_streaming_available);
    DUMP8("%02x ",curr_device->remote.address, BT_ADDR_OUTPUT_PRINT_NUM);
    curr_device->a2dp_audio_focus = audio_focus_change;
    ASSERT(media_type == USAGE_MEDIA,"a2dp_audio_focus change type error %d",media_type);
    switch (audio_focus_change)
    {
        case AUDIOFOCUS_GAIN:
            curr_device->a2dp_audio_focus = AUDIOFOCUS_GAIN;
            TRACE(4,"audio_policy:a2dp_focus_granted:acl state = %d,stream = %d,this_is_bg_a2dp = %d,bg_flag:%d",
+                        curr_device->acl_is_connected,curr_device->a2dp_streaming_available, curr_device->this_is_bg_a2dp,
+                        curr_device->this_is_paused_bg_a2dp || curr_device->this_is_closed_bg_a2dp);
            /*
             * ACL link disconnected,don't maintain this focus in stack.
            */
            if(curr_device->acl_is_connected)
            {
                if(curr_device->a2dp_streaming_available || (curr_device->this_is_bg_a2dp == true))
                {
                    if(bt_request_ui_resume_a2dp_callback)
                    {
                        bool request_result = bt_request_ui_resume_a2dp_callback(&curr_device->remote);
                        curr_device->this_is_bg_a2dp = false;
                        TRACE(2,"audio_policy:(d%x)a2dp_focus_granted_result:%d",device_id, request_result);
                        if(request_result)
                        {
                            if(curr_device->a2dp_streamming)
                            {
                                app_bt_audio_start_a2dp_playing(device_id);
                            }
                            else
                            {
                                app_bt_audio_a2dp_resume_this_profile(device_id);
                            }
                        }
                        else
                        {
                            app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
                            TRACE(0,"audio_policy:disallow resume a2dp ui reject");
                        }
                    }
                    else
                    {
                        if(curr_device->a2dp_streamming)
                        {
                            app_bt_audio_start_a2dp_playing(device_id);
                        }
                        else
                        {
                            if(curr_device->this_is_closed_bg_a2dp &&
                               app_bt_manager.config.close_a2dp_stream &&
                               !curr_device->a2dp_conn_flag)
                            {
                                if(!curr_device->auto_make_remote_play)
                                {
                                    app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
                                }
                            }
                            app_bt_audio_a2dp_resume_this_profile(device_id);
                        }
                    }
                }
            }
            else
            {
                app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
            }
            break;
        case AUDIOFOCUS_LOSS:
            if(curr_device->a2dp_streamming)
            {
                TRACE(1,"[d%x][audio_policy]start waiting abandon focus",device_id);
                curr_device->a2dp_audio_focus = AUDIOFOCUS_LOSS;
                curr_device->this_is_bg_a2dp = true;
                app_bt_audio_stop_a2dp_playing(device_id);
                app_bt_start_audio_focus_timer(device_id);
                app_bt_audio_select_a2dp_behavior_handler(device_id);
                //app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
            }
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT:
             curr_device->a2dp_audio_focus = AUDIOFOCUS_LOSS_TRANSIENT;
            if(curr_device->a2dp_streaming_available)
            {
                curr_device->this_is_bg_a2dp = true;
                app_bt_audio_stop_a2dp_playing(device_id);
                app_bt_audio_select_a2dp_behavior_handler(device_id);
            }
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            break;
        default:
            break;
    }
}

void app_bt_on_call_audio_focus_change(uint8_t device_id,AUDIO_USAGE_TYPE_E media_type,int audio_focus_change)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    TRACE(3,"audio_policy:(d%x) sco_audio_focus_change:stream_type:%d,focus changed %s",
                    device_id,media_type,app_audio_focus_change_to_string(audio_focus_change));
    DUMP8("%02x ",curr_device->remote.address, BT_ADDR_OUTPUT_PRINT_NUM);

    switch (audio_focus_change)
    {
        case AUDIOFOCUS_GAIN:
            curr_device->call_audio_focus = AUDIOFOCUS_GAIN;
            if(media_type == USAGE_RINGTONE)
            {
                if(app_bt_audio_current_device_is_hfp_idle_state(device_id))
                {
                    app_bt_audio_abandon_call_focus(curr_device);
                    TRACE(1,"(d%x)maybe hungup gain should abanon",device_id);
                }
                break;
            }
#if defined(IBRT_V2_MULTIPOINT)
            if (media_type == USAGE_CALL && app_bt_audio_resume_sco_to_earbud(device_id))
            {
                TRACE(2, "audio_policy:(d%x) %s resume active call", device_id, __func__);
            }
            else
#endif
            {
                if (curr_device->hf_audio_state != BTIF_HF_AUDIO_CON)
                {
                    TRACE(4, "audio_policy:(d%x) route sco backto earbuds fail,%d,%d,%d",
                        device_id,curr_device->hf_conn_flag,curr_device->hf_audio_state,curr_device->hfchan_call);
                    app_bt_audio_abandon_call_focus(curr_device);
                }
            }
            if(curr_device->hf_audio_state == BTIF_HF_AUDIO_CON && app_bt_manager.config.second_sco_bg_action == IBRT_ACTION_HOLD_ACTIVE_SCO)
            {
                if(app_bt_manager.curr_playing_sco_id != BT_DEVICE_INVALID_ID && device_id != app_bt_manager.curr_playing_sco_id)
                {
                    app_bt_audio_stop_sco_playing(app_bt_manager.curr_playing_sco_id);
                }
                app_bt_audio_start_sco_playing(device_id);
            }
            break;
        case AUDIOFOCUS_LOSS:
            curr_device->call_audio_focus = AUDIOFOCUS_LOSS;
            if(media_type == USAGE_CALL)
            {
                app_bt_audio_stop_sco_playing(device_id);
                app_bt_select_hfp_behavior_handler(device_id);
            }
            else if(media_type == USAGE_RINGTONE)
            {
                app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP_MEDIA, BT_STREAM_MEDIA, device_id, 0);
                app_bt_audio_abandon_audio_focus(curr_device, USAGE_RINGTONE);
                break;
            }
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT:  // call + call
            curr_device->call_audio_focus = AUDIOFOCUS_LOSS_TRANSIENT;
            if(media_type == USAGE_CALL)
            {
                app_bt_audio_stop_sco_playing(device_id);
                app_bt_select_hfp_behavior_handler(device_id);
            }
            else if(media_type == USAGE_RINGTONE)
            {
                app_bt_audio_abandon_audio_focus(curr_device, USAGE_RINGTONE);
                app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP_MEDIA, BT_STREAM_MEDIA, device_id, 0);
                break;
            }
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            break;
        default:
            break;
    }
}

void app_bt_on_ring_audio_focus_change(uint8_t device_id,AUDIO_USAGE_TYPE_E media_type,int audio_focus_change)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    TRACE(3,"audio_policy:(d%x) ring_audio_focus_change:stream_type:%d,focus changed %s",
                    device_id,media_type,app_audio_focus_change_to_string(audio_focus_change));
    DUMP8("%02x ",curr_device->remote.address, BT_ADDR_OUTPUT_PRINT_NUM);

    switch (audio_focus_change)
    {
        case AUDIOFOCUS_GAIN:
            if(curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN &&
               curr_device->hf_audio_state == BTIF_HF_AUDIO_DISCON &&
               btif_hf_is_inbandring_enabled(curr_device->hf_channel) == false)
            {
                curr_device->ring_audio_focus = AUDIOFOCUS_GAIN;
                media_PlayAudio(AUD_ID_BT_CALL_INCOMING_CALL, device_id);
            }
            else
            {
                app_bt_audio_abandon_audio_focus(curr_device, USAGE_RINGTONE);
            }
            break;
        case AUDIOFOCUS_LOSS:
        case AUDIOFOCUS_LOSS_TRANSIENT:
            app_bt_audio_abandon_audio_focus(curr_device, USAGE_RINGTONE);
            app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP_MEDIA, BT_STREAM_MEDIA, device_id, 0);
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            break;
        default:
            break;
    }
}

void app_bt_audio_init_focus_listener(void* device)
{
    struct BT_DEVICE_T* bt_device = (struct BT_DEVICE_T*)device;

    bt_device->a2dp_streaming_available = false;
    bt_device->sco_streaming_available = false;
    bt_device->a2dp_audio_focus  = AUDIOFOCUS_NONE;
    bt_device->call_audio_focus  = AUDIOFOCUS_NONE;
    bt_device->a2dp_focus_changed_listener = app_bt_on_a2dp_audio_focus_change;
    bt_device->sco_focus_changed_listener = app_bt_on_call_audio_focus_change;
    bt_device->ring_focus_changed_listener = app_bt_on_ring_audio_focus_change;
}

#define ABANDON_AUDIO_FOCUS_TIME (8000)
void app_bt_start_audio_focus_timer(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(curr_device->abandon_focus_timer && curr_device->a2dp_audio_focus == AUDIOFOCUS_LOSS){
        osTimerStop(curr_device->abandon_focus_timer);
        osTimerStart(curr_device->abandon_focus_timer, ABANDON_AUDIO_FOCUS_TIME);
    }
}

void app_bt_stop_audio_focus_timer(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(curr_device && curr_device->abandon_focus_timer && curr_device->a2dp_audio_focus == AUDIOFOCUS_LOSS){
        TRACE(1,"d%x,stop audio focus timer", device_id);
        osTimerStop(curr_device->abandon_focus_timer);
        curr_device->a2dp_audio_focus = AUDIOFOCUS_NONE;
    }
}

void app_bt_audio_abandon_focus_timer_handler(void const *param)
{
    struct BT_DEVICE_T* curr_device = (struct BT_DEVICE_T *)param;
    if(curr_device && curr_device->abandon_focus_timer && curr_device->a2dp_audio_focus == AUDIOFOCUS_LOSS)
    {
        if(app_bt_manager.config.mute_a2dp_stream && curr_device->a2dp_streamming)
        {
            TRACE(0,"[audio_policy]dont abandon focus in there");
            return;
        }
        curr_device->this_is_bg_a2dp = false;
        curr_device->a2dp_audio_focus = AUDIOFOCUS_NONE;
        app_bt_audio_abandon_audio_focus(curr_device,USAGE_MEDIA);
        TRACE(1,"[d%x][audio_policy]abandon focus",curr_device->device_id);
    }
}

#define SCO_SWITCH_DELAY_TICKS  (3200)    //how to caculate
uint32_t app_bt_audio_trigger_switch_mute_streaming_sco(uint32_t btclk)
{
    uint8_t curr_playing_sco = app_bt_audio_get_curr_playing_sco();
    uint8_t selected_streaming_sco = BT_DEVICE_INVALID_ID;
    struct BT_DEVICE_T *curr_device = NULL;
    uint32_t curr_btclk = 0;
    uint32_t trigger_btclk = 0;

    if (app_bt_manager.trigger_sco_device_id != BT_DEVICE_INVALID_ID)
    {
        TRACE(2,"%s already has a sco switch trigger %x", __func__, app_bt_manager.a2dp_switch_trigger_device);
        return 0;
    }

    if (curr_playing_sco == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no playing sco", __func__);
        return 0;
    }

    selected_streaming_sco = app_bt_audio_select_another_streaming_sco(curr_playing_sco);
    if (selected_streaming_sco == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no other streaming sco", __func__);
        return 0;
    }

    curr_device = app_bt_get_device(curr_playing_sco);
    if (btclk == 0)
    {
        curr_btclk = bt_syn_get_curr_ticks(curr_device->acl_conn_hdl);
        trigger_btclk = curr_btclk + SCO_SWITCH_DELAY_TICKS;
    }
    else
    {
        trigger_btclk = btclk;
    }

    app_bt_manager.sco_switch_trigger_btclk = trigger_btclk;
    app_bt_manager.trigger_sco_device_id = curr_playing_sco;

    app_bt_audio_sco_switch_trigger();

    return trigger_btclk;
}

bool app_bt_audio_sco_switch_trigger(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    uint8_t curr_playing_sco = app_bt_audio_get_curr_playing_sco();
    uint8_t selected_streaming_sco = BT_DEVICE_INVALID_ID;
    uint32_t curr_btclk = 0;
    bool time_reached = false;

    if (app_bt_manager.trigger_sco_device_id == BT_DEVICE_INVALID_ID || app_bt_manager.sco_switch_trigger_btclk == 0)
    {
        return false;
    }

    if (curr_playing_sco == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no playing sco", __func__);
        app_bt_manager.trigger_a2dp_switch = false;
        return false;
    }

    selected_streaming_sco = app_bt_audio_select_another_streaming_sco(curr_playing_sco);
    if (selected_streaming_sco == BT_DEVICE_INVALID_ID)
    {
        TRACE(1,"%s no other streaming sco", __func__);
        app_bt_manager.trigger_a2dp_switch = false;
        return false;
    }

    if (curr_playing_sco != app_bt_manager.trigger_sco_device_id)
    {
        TRACE(3,"%s trigger device not match %x %x", __func__, curr_playing_sco, app_bt_manager.trigger_sco_device_id);
        app_bt_manager.trigger_sco_device_id = BT_DEVICE_INVALID_ID;
        return false;
    }

    curr_device = app_bt_get_device(curr_playing_sco);
    curr_btclk = bt_syn_get_curr_ticks(curr_device->acl_conn_hdl);

    TRACE(5,"%s playing_sco %x selected_sco %x tgtclk %x curclk %x", __func__,
            curr_playing_sco, selected_streaming_sco, app_bt_manager.sco_switch_trigger_btclk, curr_btclk);

    if (curr_btclk < app_bt_manager.sco_switch_trigger_btclk)
    {
        time_reached = false;
    }
    else
    {
        time_reached = true;
    }

    if (time_reached)
    {
       struct BT_DEVICE_T *trigger_device = app_bt_get_device(app_bt_manager.curr_playing_sco_id);
       uint16_t scohandle = btif_hf_get_sco_hcihandle(trigger_device->hf_channel);
       app_bt_Me_switch_sco(scohandle);
       app_bt_audio_event_handler(selected_streaming_sco, APP_BT_AUDIO_HOLD_SWICTH_BACKGROUND, 0);
       app_bt_manager.trigger_sco_device_id = BT_DEVICE_INVALID_ID;
       return true;
    }
    return false;
}

void app_bt_audio_ui_allow_resume_a2dp_register(app_bt_audio_ui_allow_resume_request allow_resume_request_cb)
{
    bt_request_ui_resume_a2dp_callback = allow_resume_request_cb;
}

void app_bt_audio_strategy_init(void)
{
#if defined(IBRT)
    app_tws_ibrt_allow_send_profile_init(app_bt_audio_allow_send_profile_immediate);
#endif
    app_af_empty_search_init(app_bt_audio_focus_empty_check_valid_stream);
}

void app_bt_handle_profile_exchanged_done(void)
{
    uint8_t focus_buff[APP_FOCOUS_DATA_SIZE];
    focus_buff[0] = (uint8_t)app_audio_focus_ctrl_stack_length();
    if(focus_buff[0] == 0)
    {
        TRACE(1,"audio_policy:no any device in streaming!!!");
        return;
    }
    uint16_t length = app_audio_focus_get_device_info(&focus_buff[1]);
    app_ibrt_send_ext_cmd_sync_focus_info(focus_buff, length+1);
}

bool app_bt_audio_both_profile_exchanged(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    uint8_t num = 0;
    for(uint8_t i=0; i<BT_DEVICE_NUM; i++)
    {
        curr_device = app_bt_get_device(i);
        if(app_tws_ibrt_is_profile_exchanged(&curr_device->remote))
        {
            num++;
        }
    }
    TRACE(1,"audio_policy profile exchanged num:%d", num);
    return num == BT_DEVICE_NUM;
}

static int app_bt_audio_slave_request_focus(focus_device_info_t *device_info, bool top)
{
    audio_focus_req_info_t request_info;

    memcpy((uint8_t*)&request_info.device_info.device_addr.address[0],(void*)&device_info->device_addr, sizeof(bt_bdaddr_t));
    request_info.device_info.device_idx = device_info->device_idx;
    request_info.device_info.audio_type = device_info->audio_type;
    request_info.device_info.focus_request_type = device_info->focus_request_type;
    request_info.device_info.stream_type = device_info->stream_type;
    request_info.device_info.delayed_focus_allow = false;

    if(device_info->audio_type == AUDIO_TYPE_BT)
    {
        struct BT_DEVICE_T* device = app_bt_get_device(device_info->device_idx);
        if(device_info->stream_type == USAGE_MEDIA)
        {
            request_info.focus_changed_listener = device->a2dp_focus_changed_listener;
            request_info.ext_policy = app_bt_audio_music_ext_policy;
        }
        else if(device_info->stream_type == USAGE_CALL)
        {
            request_info.focus_changed_listener = device->sco_focus_changed_listener;
            if(app_bt_manager.config.virtual_call_handle == VIRTUAL_HANDLE_NON_PROMPT ||
               app_bt_manager.config.virtual_call_handle == VIRTUAL_HANDLE_NON_PROMPTED)
            {
                request_info.ext_policy = app_bt_audio_virtual_call_ext_policy;
            }else{
                request_info.ext_policy = app_bt_audio_hfp_stream_ext_policy;
            }
        }
        else if(device_info->stream_type == USAGE_RINGTONE)
        {
            request_info.focus_changed_listener = device->ring_focus_changed_listener;
            request_info.ext_policy = app_bt_audio_ringtone_stream_ext_policy;
        }
    }

    return app_audio_handle_peer_focus(&request_info, top);
}

bool app_bt_audio_handle_peer_focus_info(uint8_t *buff, uint16_t length)
{
    uint8_t offset = 0;
    uint8_t local_device_id = BT_DEVICE_INVALID_ID;
    uint8_t peer_stack_len = buff[0];
    focus_device_info_t device_info;
    offset = offset + 1;
    audio_focus_req_info_t* top_focus = app_audio_get_curr_audio_focus();
    if(top_focus == NULL)
    {
        return false;
    }
    memcpy(&device_info, &buff[offset], sizeof(focus_device_info_t));
    offset = offset + sizeof(focus_device_info_t);
    TRACE(1,"focus stack audio type:%s", app_audio_audio_type_to_string(device_info.audio_type));
    TRACE(1,"focus stack request type:%d", device_info.focus_request_type);
    TRACE(1,"focus stack media type:%s", app_audio_media_type_to_string(device_info.stream_type));
    DUMP8("%02x ",&device_info.device_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    if(device_info.audio_type == top_focus->device_info.audio_type &&
       device_info.stream_type == top_focus->device_info.stream_type &&
       memcmp(&top_focus->device_info.device_addr.address[0], &device_info.device_addr.address[0], 6) == 0)
    {
        TRACE(0,"audio_policy:curr_focus == peer focus!!!");
        return true;
    }
    else
    {
        local_device_id = app_bt_get_device_id_byaddr(&device_info.device_addr);
        if(local_device_id != device_info.device_idx){
            device_info.device_idx = local_device_id;
        }
        if(app_bt_audio_slave_request_focus(&device_info, true) == AUDIOFOCUS_REQUEST_GRANTED)
        {
            if(device_info.stream_type == USAGE_MEDIA)
            {
                app_bt_audio_start_a2dp_playing(device_info.device_idx);
            }
            else if(device_info.stream_type == USAGE_CALL)
            {
                app_bt_audio_start_sco_playing(device_info.device_idx);
            }
            TRACE(1,"focus mismatch!!!");
        }
    }
    peer_stack_len = peer_stack_len - 1;
    while(peer_stack_len)
    {
        memcpy(&device_info, &buff[offset], sizeof(focus_device_info_t));
        local_device_id = app_bt_get_device_id_byaddr(&device_info.device_addr);
        if(local_device_id != device_info.device_idx){
            device_info.device_idx = local_device_id;
        }
        app_bt_audio_slave_request_focus(&device_info, false);
        offset = offset + sizeof(focus_device_info_t);
        peer_stack_len = peer_stack_len - 1;
    }
    return false;
}

uint8 app_bt_audio_a2dp_close_req_allow(uint8_t device_id)
{
#if defined(IBRT_V2_MULTIPOINT)
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(!app_ibrt_a2dp_profile_is_exchanged(&curr_device->remote) &&
        app_tws_ibrt_slave_ibrt_link_connected(&curr_device->remote)){
        return FAILURE;
    }else{
        return SUCCESS;
    }
#else
    return SUCCESS;
#endif
}
