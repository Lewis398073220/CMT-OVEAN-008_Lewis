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
//#include "mbed.h"
#include <stdio.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "analog.h"


#include "hfp_api.h"
#include "me_api.h"
#include "a2dp_api.h"
#include "avrcp_api.h"

#include "besbt.h"

#include "cqueue.h"
#include "btapp.h"
#include "app_key.h"
#include "app_audio.h"
#include "app_audio_control.h"
#include "apps.h"
#include "app_bt_stream.h"
#include "app_bt_media_manager.h"
#include "app_bt.h"
#include "app_bt_func.h"
#include "app_hfp.h"
#if defined(IBRT)
#include "app_ibrt_if.h"
#endif
#include "bt_if.h"
#if defined(BT_SOURCE)
#include "bt_source.h"
#endif
#include "nvrecord_env.h"

#ifdef BT_HID_DEVICE
#include "app_bt_hid.h"
#endif

#ifdef BT_PBAP_SUPPORT
#include "app_pbap.h"
#endif

#ifdef IBRT_CORE_V2_ENABLE
#include "app_tws_ibrt_conn.h"
#endif

#include "app_media_player.h"
#if BLE_AUDIO_ENABLED
#include "app_audio_control.h"
#endif


#ifdef BT_HFP_SUPPORT

#ifdef SUPPORT_SIRI
int open_siri_flag = 0;
void bt_key_handle_siri_key(enum APP_KEY_EVENT_T event)
{
     switch(event)
     {
        case  APP_KEY_EVENT_NONE:
            if(open_siri_flag == 1){
                TRACE(0,"open siri");
                app_ibrt_if_enable_hfp_voice_assistant(true);
                open_siri_flag = 0;
            } /*else {
                TRACE(0,"evnet none close siri");
                app_ibrt_if_enable_hfp_voice_assistant(false);
            }*/
            break;
        case  APP_KEY_EVENT_LONGLONGPRESS:
        case  APP_KEY_EVENT_UP:
            //TRACE(0,"long long/up/click event close siri");
            //app_ibrt_if_enable_hfp_voice_assistant(false);
            break;
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
        }
}
#endif

void hfcall_next_sta_handler(uint8_t device_id, hf_event_t event)
{
#if BT_DEVICE_NUM > 1
    btif_hf_channel_t* hf_channel_another = app_bt_get_device(app_bt_manager.hfp_key_handle_another_id)->hf_channel;
    btif_hf_channel_t* hf_channel_curr = app_bt_get_device(app_bt_manager.hfp_key_handle_curr_id)->hf_channel;
    TRACE(3, "(d%x) !!!hfcall_next_sta_handler curr/another_hfp_device %x %x hf_call_next_state %d\n",
        device_id, app_bt_manager.hfp_key_handle_curr_id, app_bt_manager.hfp_key_handle_another_id, app_bt_manager.hf_call_next_state);
    switch(app_bt_manager.hf_call_next_state)
    {
        case HFCALL_NEXT_STA_NULL:
            break;
        case HFCALL_NEXT_STA_ANOTHER_ANSWER:
            if(event == BTIF_HF_EVENT_AUDIO_DISCONNECTED)
            {
                TRACE(0,"NEXT_ACTION = HFP_ANSWER_ANOTHER_CALL\n");
                btif_hf_answer_call(hf_channel_another);
                app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_NULL;
            }
            break;
        case HFCALL_NEXT_STA_ANOTHER_ADDTOEARPHONE:
            if(event == BTIF_HF_EVENT_AUDIO_DISCONNECTED)
            {
                TRACE(0,"NEXT_ACTION = HFP_ANOTHER_ADDTOEARPHONE\n");
                btif_hf_create_audio_link(hf_channel_another);
                app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_NULL;
            }
            break;
        case HFCALL_NEXT_STA_CURR_ANSWER:
             TRACE(0,"NEXT_ACTION = HF_CURRENT_ANSWER\n");
             btif_hf_answer_call(hf_channel_curr);
             app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_NULL;
        break;
    }
#endif
}

#endif /* BT_HFP_SUPPORT */

void hfp_handle_key(uint8_t hfp_key)
{
#ifdef BT_HFP_SUPPORT
    app_bt_manager.hfp_key_handle_curr_id = app_bt_audio_get_hfp_device_for_user_action();
    btif_hf_channel_t* hf_channel_curr = app_bt_get_device(app_bt_manager.hfp_key_handle_curr_id)->hf_channel;
#if BT_DEVICE_NUM > 1
    app_bt_manager.hfp_key_handle_another_id = app_bt_audio_get_another_hfp_device_for_user_action(app_bt_manager.hfp_key_handle_curr_id);
    btif_hf_channel_t* hf_channel_another = app_bt_get_device(app_bt_manager.hfp_key_handle_another_id)->hf_channel;
#endif

    switch(hfp_key)
    {
        case HFP_KEY_ANSWER_CALL:
            ///answer a incomming call
            TRACE(0,"avrcp_key = HFP_KEY_ANSWER_CALL\n");
            btif_hf_answer_call(hf_channel_curr);
            break;
        case HFP_KEY_HANGUP_CALL:
            TRACE(0,"avrcp_key = HFP_KEY_HANGUP_CALL\n");
            btif_hf_hang_up_call(hf_channel_curr);
            break;
        case HFP_KEY_REDIAL_LAST_CALL:
            ///redail the last call
            TRACE(0,"avrcp_key = HFP_KEY_REDIAL_LAST_CALL\n");
            btif_hf_redial_call(hf_channel_curr);
            break;
        case HFP_KEY_CHANGE_TO_PHONE:
            ///remove sco and voice change to phone
            if(app_bt_is_hfp_audio_on())
            {
                TRACE(0,"avrcp_key = HFP_KEY_CHANGE_TO_PHONE\n");
                btif_hf_disc_audio_link(hf_channel_curr);
            }
            break;
        case HFP_KEY_ADD_TO_EARPHONE:
            ///add a sco and voice change to earphone
            if(!app_bt_is_hfp_audio_on())
            {
                TRACE(0,"avrcp_key = HFP_KEY_ADD_TO_EARPHONE ver:%x\n",  btif_hf_get_version(hf_channel_curr));
                btif_hf_create_audio_link(hf_channel_curr);
            }
            break;
        case HFP_KEY_MUTE:
            TRACE(0,"avrcp_key = HFP_KEY_MUTE\n");
            app_bt_manager.hf_tx_mute_flag = 1;
            break;
        case HFP_KEY_CLEAR_MUTE:
            TRACE(0,"avrcp_key = HFP_KEY_CLEAR_MUTE\n");
            app_bt_manager.hf_tx_mute_flag = 0;
            break;
        case HFP_KEY_THREEWAY_HOLD_AND_ANSWER:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_AND_ANSWER\n");
            btif_hf_call_hold(hf_channel_curr, BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
            break;
        case HFP_KEY_THREEWAY_HANGUP_AND_ANSWER:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_SWAP_ANSWER\n");
            btif_hf_call_hold(hf_channel_curr, BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS, 0);
            break;
        case HFP_KEY_THREEWAY_HOLD_REL_INCOMING:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_REL_INCOMING\n");
            btif_hf_call_hold(hf_channel_curr, BTIF_HF_HOLD_RELEASE_HELD_CALLS, 0);
            break;
#if BT_DEVICE_NUM > 1
        case HFP_KEY_DUAL_HF_HANGUP_ANOTHER:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HANGUP_ANOTHER\n");
            btif_hf_hang_up_call(hf_channel_another);
            break;
        case HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER\n");
            btif_hf_hang_up_call(hf_channel_curr);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_ANOTHER_ANSWER;
            break;
        case HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER\n");
            break;
        case HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_ANOTHER:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_ANOTHER\n");
            btif_hf_disc_audio_link(hf_channel_curr);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_ANOTHER_ANSWER;
            break;
        case HFP_KEY_DUAL_HF_CHANGETOPHONE_ANOTHER_ADDTOEARPHONE:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_CHANGETOPHONE_ANOTHER_ADDTOEARPHONE\n");
            btif_hf_disc_audio_link(hf_channel_curr);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_ANOTHER_ADDTOEARPHONE;
            break;
        case HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE\n");
            btif_hf_hang_up_call(hf_channel_curr);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_ANOTHER_ADDTOEARPHONE;
            break;
        case HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR:
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR\n");
            btif_hf_disc_audio_link(hf_channel_another);
            app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_CURR_ANSWER;
            break;
#endif
        default :
            break;
    }
#endif /* BT_HFP_SUPPORT */
}

#ifdef BT_AVRCP_SUPPORT

void app_bt_a2dp_send_set_abs_volume(uint8_t device_id, uint8_t volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device)
    {
        return;
    }

    btif_avrcp_ct_set_absolute_volume(curr_device->avrcp_channel, volume);
}

void app_bt_a2dp_send_key_request(uint8_t device_id, uint8_t a2dp_key)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device)
    {
        return;
    }

    if(!curr_device->a2dp_conn_flag)
    {
        TRACE(1, "%s a2dp is not connected", __func__);
        return;
    }

    if (!btif_avrcp_is_control_channel_connected(curr_device->avrcp_channel))
    {
        TRACE(2, "%s avrcp_key %d avrcp is not connected", __func__, a2dp_key);
        return;
    }

#ifdef IBRT_CORE_V2_ENABLE
    app_ibrt_conn_rs_task_set(RS_TASK_AVRCP_KEY, 2000);
#endif

    switch(a2dp_key)
    {
        case AVRCP_KEY_STOP:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_STOP %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_STOP,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_STOP,FALSE);
            curr_device->a2dp_play_pause_flag = 0;
            break;
        case AVRCP_KEY_PLAY:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_PLAY press %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PLAY,TRUE);
            curr_device->a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_PAUSE:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_PAUSE press %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PAUSE,TRUE);
            curr_device->a2dp_play_pause_flag = 0;
            break;
        case AVRCP_KEY_FORWARD:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_FORWARD %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_FORWARD,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_FORWARD,FALSE);
            curr_device->a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_BACKWARD:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_BACKWARD %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_BACKWARD,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_BACKWARD,FALSE);
            curr_device->a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_VOLUME_UP:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_VOLUME_UP %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_VOLUME_UP,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_VOLUME_UP,FALSE);
            break;
        case AVRCP_KEY_VOLUME_DOWN:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_VOLUME_DOWN %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_VOLUME_DOWN,TRUE);
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_VOLUME_DOWN,FALSE);
            break;
        case AVRCP_KEY_FAST_FORWARD_START:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_FAST_FORWARD %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_FAST_FORWARD,TRUE);
            break;
        case AVRCP_KEY_FAST_FORWARD_STOP:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_FAST_FORWARD_STOP %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_FAST_FORWARD,FALSE);
            break;
        case AVRCP_KEY_REWIND_START:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_REWIND_START %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_REWIND,TRUE);
            break;
        case AVRCP_KEY_REWIND_STOP:
            TRACE(2, "(d%x) avrcp_key = AVRCP_KEY_REWIND_STOP %p", device_id, __builtin_return_address(0));
            btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_REWIND,FALSE);
            break;
        default :
            break;
    }
}

extern void a2dp_handleKey(uint8_t a2dp_key)
{
    uint8_t a2dp_id = app_bt_audio_get_curr_a2dp_device();
    struct BT_DEVICE_T* curr_device = NULL;

    TRACE(2,"!!!a2dp_handleKey curr a2dp device %x last_paused_device %x", a2dp_id, app_bt_manager.a2dp_last_paused_device);

    if (a2dp_key == AVRCP_KEY_PLAY && app_bt_manager.a2dp_last_paused_device != BT_DEVICE_INVALID_ID)
    {
        if (app_bt_get_device(app_bt_manager.a2dp_last_paused_device)->a2dp_conn_flag)
        {
            a2dp_id = app_bt_manager.a2dp_last_paused_device; // only select last paused device when it still connected
        }
    }

    curr_device = app_bt_get_device(a2dp_id);

    if(!curr_device->a2dp_conn_flag)
    {
        TRACE(0,"a2dp is not connected:a2dp_conn_flag=0");
        return;
    }

    if (!btif_avrcp_is_control_channel_connected(curr_device->avrcp_channel))
    {
        TRACE(1, "avrcp_key %d the channel is not connected", a2dp_key);
        return;
    }

    if (a2dp_key == AVRCP_KEY_PAUSE)
    {
        app_bt_manager.a2dp_last_paused_device = a2dp_id;
    }
    else
    {
        app_bt_manager.a2dp_last_paused_device = BT_DEVICE_INVALID_ID;
    }

    app_bt_a2dp_send_key_request(a2dp_id, a2dp_key);
}

#endif /* BT_AVRCP_SUPPORT */


HFCALL_MACHINE_ENUM app_get_hfcall_machine(void)
{
#ifdef BT_HFP_SUPPORT
    HFCALL_MACHINE_ENUM status = HFCALL_MACHINE_NUM;
    int current_device_id = app_bt_audio_get_hfp_device_for_user_action();
    struct BT_DEVICE_T* curr_device = app_bt_get_device(current_device_id);
    btif_hf_call_setup_t    current_callSetup  = curr_device->hfchan_callSetup;
    btif_hf_call_active_t   current_call       = curr_device->hfchan_call;
    btif_hf_call_held_state current_callheld   = curr_device->hf_callheld;
    btif_audio_state_t      current_audioState = curr_device->hf_audio_state;

#if BT_DEVICE_NUM > 1
    int another_device_id = app_bt_audio_get_another_hfp_device_for_user_action(current_device_id);
    curr_device = app_bt_get_device(another_device_id);
    btif_hf_call_setup_t    another_callSetup  = curr_device->hfchan_callSetup;
    btif_hf_call_active_t   another_call       = curr_device->hfchan_call;
    btif_hf_call_held_state another_callheld   = curr_device->hf_callheld;
    btif_audio_state_t      another_audioState = curr_device->hf_audio_state;
#endif

    app_bt_hfp_state_checker();

#if BT_DEVICE_NUM < 2
    // current AG is idle.
    if( current_callSetup ==BTIF_HF_CALL_SETUP_NONE &&
        current_call == BTIF_HF_CALL_NONE &&
        current_audioState == BTIF_HF_AUDIO_DISCON )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_IDLE;
    }
    // current AG is incomming.
    else if( current_callSetup == BTIF_HF_CALL_SETUP_IN &&
             current_call == BTIF_HF_CALL_NONE )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMMING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING;
    }
    // current AG is outgoing.
    else if( (current_callSetup >= BTIF_HF_CALL_SETUP_OUT) &&
        current_call == BTIF_HF_CALL_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_OUTGOING!!!!");
        status = HFCALL_MACHINE_CURRENT_OUTGOING;
    }
    // current AG is calling.
    else if( (current_callSetup ==BTIF_HF_CALL_SETUP_NONE) &&
            current_call == BTIF_HF_CALL_ACTIVE &&
            current_callheld != BTIF_HF_CALL_HELD_ACTIVE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING!!!!");
        status = HFCALL_MACHINE_CURRENT_CALLING;
    }
    // current AG is 3way incomming.
    else if( current_callSetup ==BTIF_HF_CALL_SETUP_IN &&
            current_call == BTIF_HF_CALL_ACTIVE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_3WAY_INCOMMING!!!!");
        status = HFCALL_MACHINE_CURRENT_3WAY_INCOMMING;
    }
    // current AG is 3way hold calling.
    else if( current_callSetup ==BTIF_HF_CALL_SETUP_NONE &&
            current_call == BTIF_HF_CALL_ACTIVE &&
            current_callheld == BTIF_HF_CALL_HELD_ACTIVE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING!!!!");
        status = HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING;
    }
    else
    {
        TRACE(0,"current hfcall machine status is not found!!!!!!");
    }
#else
    // current AG is idle , another AG is idle.
    if( current_callSetup ==BTIF_HF_CALL_SETUP_NONE &&
        current_call == BTIF_HF_CALL_NONE &&
        current_audioState == BTIF_HF_AUDIO_DISCON &&
        another_callSetup==BTIF_HF_CALL_SETUP_NONE &&
        another_call == BTIF_HF_CALL_NONE &&
        another_audioState == BTIF_HF_AUDIO_DISCON )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE;
    }
    // current AG is on incomming , another AG is idle.
    else if( current_callSetup == BTIF_HF_CALL_SETUP_IN &&
             current_call == BTIF_HF_CALL_NONE &&
             another_callSetup==BTIF_HF_CALL_SETUP_NONE &&
             another_call == BTIF_HF_CALL_NONE &&
             another_audioState == BTIF_HF_AUDIO_DISCON  )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE;
    }
    // current AG is on outgoing , another AG is idle.
    else if( current_callSetup >= BTIF_HF_CALL_SETUP_OUT &&
            current_call == BTIF_HF_CALL_NONE &&
            another_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            another_call == BTIF_HF_CALL_NONE &&
            another_audioState == BTIF_HF_AUDIO_DISCON  )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE;
    }
    // current AG is on calling , another AG is idle.
    else if( current_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            current_call == BTIF_HF_CALL_ACTIVE &&
            current_callheld != BTIF_HF_CALL_HELD_ACTIVE&&
            another_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            another_call == BTIF_HF_CALL_NONE &&
            another_audioState == BTIF_HF_AUDIO_DISCON  )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE;
    }
    // current AG is 3way incomming , another AG is idle.
    else if( current_callSetup ==BTIF_HF_CALL_SETUP_IN &&
            current_call == BTIF_HF_CALL_ACTIVE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE&&
            another_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            another_call == BTIF_HF_CALL_NONE &&
            another_audioState == BTIF_HF_AUDIO_DISCON   )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE;
    }
    // current AG is 3way hold calling , another AG is without connecting.
    else if( current_callSetup ==BTIF_HF_CALL_SETUP_NONE &&
            current_call == BTIF_HF_CALL_ACTIVE &&
            current_callheld == BTIF_HF_CALL_HELD_ACTIVE&&
            another_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            another_call == BTIF_HF_CALL_NONE &&
            another_audioState == BTIF_HF_AUDIO_DISCON   )
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE!!!!");
        status = HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE;
    }
    // current AG is incomming , another AG is incomming too.
    else if( current_callSetup == BTIF_HF_CALL_SETUP_IN &&
            current_call == BTIF_HF_CALL_NONE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE&&
            another_callSetup == BTIF_HF_CALL_SETUP_IN &&
            another_call == BTIF_HF_CALL_NONE &&
            another_callheld == BTIF_HF_CALL_HELD_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING;
    }
    // current AG is ingcomming , another AG is outgoing too.
    else if( current_callSetup == BTIF_HF_CALL_SETUP_IN &&
            current_call == BTIF_HF_CALL_NONE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE&&
            another_callSetup >= BTIF_HF_CALL_SETUP_OUT &&
            another_call == BTIF_HF_CALL_NONE &&
            another_callheld == BTIF_HF_CALL_HELD_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING;
    }
	// current AG is calling , another AG is incomming.(just for PC case)
	else if(current_callSetup == BTIF_HF_CALL_SETUP_IN &&
            current_call == BTIF_HF_CALL_NONE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE &&
            another_call == BTIF_HF_CALL_NONE &&
            another_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            another_audioState == BTIF_HF_AUDIO_CON)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_INCOMMING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING;
    }
    // current AG is on calling , another AG calling changed to phone.
    else if( current_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            current_call == BTIF_HF_CALL_ACTIVE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE&&
            another_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            another_call == BTIF_HF_CALL_ACTIVE &&
            another_callheld == BTIF_HF_CALL_HELD_NONE&&
            another_audioState == BTIF_HF_AUDIO_DISCON)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE!!!!");
        status = HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE;
    }
    // current AG is on calling , another AG calling is hold.
    else if( current_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            current_call == BTIF_HF_CALL_ACTIVE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE&&
            another_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            another_call == BTIF_HF_CALL_ACTIVE &&
            another_callheld == BTIF_HF_CALL_HELD_ACTIVE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD!!!!");
        status = HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD;
    }
    //current AG is on incoming,another AG is calling.
    else if(current_callSetup == BTIF_HF_CALL_SETUP_IN &&
            current_call == BTIF_HF_CALL_NONE &&
            current_callheld == BTIF_HF_CALL_HELD_NONE &&
            another_callSetup == BTIF_HF_CALL_SETUP_NONE &&
            another_call == BTIF_HF_CALL_ACTIVE &&
            another_callheld == BTIF_HF_CALL_HELD_NONE)
    {
        TRACE(0,"current hfcall machine status is HFCALL_MACHINE_CURRENT_INCOMING_ANOTHER_CALLING!!!!");
        status = HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING;
    }
    else
    {
        TRACE(0,"current hfcall machine status is not found!!!!!!");
    }
#endif
//    TRACE(0,"%s status is %d",__func__,status);
    return status;
#else
    return HFCALL_MACHINE_CURRENT_IDLE;
#endif /* BT_HFP_SUPPORT */
}

#ifdef BT_TEST_CURRENT_KEY
extern "C" void hal_intersys_wakeup_btcore(void);
void bt_drv_i2v_enable_sleep_for_bt_access(void);
void bt_drv_accessmode_switch_test(void)
{
    static uint8_t access_mode=0;

    app_bt_start_custom_function_in_bt_thread((uint32_t)access_mode,
                0, (uint32_t)btif_me_set_accessible_mode);

    if(access_mode ==0)
    {
        access_mode = 2;
    }
    else if(access_mode == 2)
    {
        access_mode= 3;
    }
    else if(access_mode == 3)
    {
        access_mode= 0;
    }

}
#endif

POSSIBLY_UNUSED static void bt_key_handle_music_playback()
{
    PLAYBACK_STATE_E music_state = app_bt_get_music_playback_status();
    if(music_state == PLAYING)
    {
        app_audio_control_media_pause();
    }
    else
    {
        app_audio_control_media_play();
    }
}

POSSIBLY_UNUSED static void bt_key_handle_call(CALL_STATE_E call_state)
{
    switch (call_state)
    {
        case CALL_STATE_INCOMING:
           app_audio_control_call_answer();
        break;
        case CALL_STATE_OUTGOING:
            app_audio_control_call_terminate();
        break;
        case CALL_STATE_ACTIVE:
            app_audio_control_call_terminate();
        break;
        case CALL_STATE_THREE_WAY_INCOMING:
            app_audio_control_handle_three_way_incoming_call();
            break;
        case CALL_STATE_TRREE_WAY_HOLD_CALLING:
            app_audio_control_release_active_call();
            break;
        //case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
        //    hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        //break;
        //case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
        //   hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        //break;
        default:
            
        break;
    }
}

POSSIBLY_UNUSED static void bt_key_handle_bt_func_click()
{
    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();
    POSSIBLY_UNUSED struct BT_DEVICE_T* a2dp_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
    switch(hfcall_machine)
    {
#ifdef BT_AVRCP_SUPPORT
        case HFCALL_MACHINE_CURRENT_IDLE:
        {
            if(a2dp_device && (a2dp_device->a2dp_play_pause_flag == 0)){
                a2dp_handleKey(AVRCP_KEY_PLAY);
            }else{
                a2dp_handleKey(AVRCP_KEY_PAUSE);
            }
        }
        break;
#if BT_DEVICE_NUM > 1
        case HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE:
        {
            if(a2dp_device && (a2dp_device->a2dp_play_pause_flag == 0)){
                a2dp_handleKey(AVRCP_KEY_PLAY);
            }else{
                a2dp_handleKey(AVRCP_KEY_PAUSE);
            }
        }
        break;
#endif
#endif /* BT_AVRCP_SUPPORT */
#ifdef BT_HFP_SUPPORT
        case HFCALL_MACHINE_CURRENT_INCOMMING:
           hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        break;
#if BT_DEVICE_NUM > 1
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING:
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE:
            hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD:
            hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING:
            hfp_handle_key(HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR);
        break;
#endif
#endif /* BT_HFP_SUPPORT */
        default:
        break;
    }
}

void bt_key_handle_func_click(void)
{
    TRACE(0,"%s enter",__func__);

#ifdef BT_TEST_CURRENT_KEY
    bt_drv_accessmode_switch_test();
    return;
#endif

#if BLE_AUDIO_ENABLED
    CALL_STATE_E call_state = app_bt_get_call_state();
    if (call_state == CALL_STATE_IDLE)
    {
       bt_key_handle_music_playback();
    }
    else
    {
        bt_key_handle_call(call_state);
    }
#else
    bt_key_handle_bt_func_click();
#endif
#if HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_SIRI_REPORT
    open_siri_flag = 0;
#endif
    return;
}

void bt_key_handle_customer_doubleclick(void)
{
    bt_key_handle_music_playback();
}

void bt_key_handle_func_doubleclick(void)
{
    TRACE(0,"%s enter",__func__);

    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();

#ifdef SUPPORT_SIRI
    open_siri_flag=0;
#endif

    switch(hfcall_machine)
    {
        case HFCALL_MACHINE_CURRENT_IDLE:
#ifdef BT_HID_DEVICE
            app_bt_hid_send_capture();
#else
            bt_key_handle_customer_doubleclick();
#endif
        break;
#ifdef BT_HFP_SUPPORT
        case HFCALL_MACHINE_CURRENT_INCOMMING:
            bt_key_handle_call(CALL_STATE_INCOMING);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING:
            bt_key_handle_call(CALL_STATE_OUTGOING);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING:
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
            bt_key_handle_call(CALL_STATE_ACTIVE);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
        break;
#if BT_DEVICE_NUM > 1
        case HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE:
#ifdef BT_HID_DEVICE
            app_bt_hid_send_capture();
#else
            bt_key_handle_customer_doubleclick();
#endif
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE:
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE:
            if(app_bt_manager.hf_tx_mute_flag == 0){
                hfp_handle_key(HFP_KEY_MUTE);
            }else{
                hfp_handle_key(HFP_KEY_CLEAR_MUTE);
            }
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_REL_INCOMING);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE:
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING:
            hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD:
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING:
            hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
#endif
#endif /* BT_HFP_SUPPORT */
        default:
        break;
    }
}

void bt_key_handle_func_tripleclick(void)
{
    TRACE(0,"%s enter",__func__);
    CALL_STATE_E call_state = app_bt_get_call_state();
    PLAYBACK_INFO_T* playback_info = app_bt_get_music_playback_info();

    if(call_state == CALL_STATE_IDLE)
    {
        switch(playback_info->playback_status)
        {
            case IDLE:
            case PAUSED:
                app_ibrt_if_enable_hfp_voice_assistant(true);
                break;
            case PLAYING:
                {
                    app_audio_control_media_pause();
                    app_ibrt_if_enable_hfp_voice_assistant(true);
                }
                break;
            default:
                break;
        }
    }
}

void bt_key_handle_customer_volume(void)
{
#if defined(IBRT)
    if(app_ibrt_if_is_nv_master())
        {
            app_audio_control_streaming_volume_up();
            }
        else
        {
            app_audio_control_streaming_volume_down();
            }
#endif
}

void bt_key_handle_func_longpress(void)
{
    TRACE(0,"%s enter",__func__);
    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();
#ifdef SUPPORT_SIRI
    open_siri_flag=0;
#endif
#ifndef FPGA
    media_PlayAudio(AUD_ID_BT_WARNING, 0);
#endif

    switch(hfcall_machine)
    {
        case HFCALL_MACHINE_CURRENT_IDLE:
        {
            bt_key_handle_customer_volume();
#ifdef BT_PBAP_SUPPORT
            app_bt_pbap_client_test();
#endif
#ifdef BT_MAP_SUPPORT
            bt_map_client_test(&app_bt_get_device(BT_DEVICE_ID_1)->remote);
#endif
#if HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_SIRI_REPORT
            if(open_siri_flag == 0 )
            {
#ifndef FPGA
                media_PlayAudio(AUD_ID_BT_WARNING, 0);
#endif
                open_siri_flag = 1;
            }
#endif
        }
        break;
#ifdef BT_HFP_SUPPORT
        case HFCALL_MACHINE_CURRENT_INCOMMING:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING:
        {
            uint8_t device_id = app_bt_audio_get_curr_hfp_device();
            struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
            if(curr_device->switch_sco_to_earbud) {
                //call is active, switch from phone to earphone
                hfp_handle_key(HFP_KEY_ADD_TO_EARPHONE);
            } else {
                //call is active, switch from earphone to phone
                hfp_handle_key(HFP_KEY_CHANGE_TO_PHONE);
            }
        }
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
        {
#ifndef FPGA
            media_PlayAudio(AUD_ID_BT_WARNING, 0);
#endif
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        }
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
#if BT_DEVICE_NUM > 1
        case HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE:
        {
                bt_key_handle_customer_volume();
        }
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE:
        {
            uint8_t device_id = app_bt_audio_get_curr_hfp_device();
            struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
            if(curr_device->switch_sco_to_earbud) {
                //call is active, switch from phone to earphone
                hfp_handle_key(HFP_KEY_ADD_TO_EARPHONE);
            } else {
                //call is active, switch from earphone to phone
                hfp_handle_key(HFP_KEY_CHANGE_TO_PHONE);
            }
        }
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING:
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING:
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE:
            hfp_handle_key(HFP_KEY_DUAL_HF_CHANGETOPHONE_ANOTHER_ADDTOEARPHONE);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD:
            hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
#endif
#endif /* BT_HFP_SUPPORT */
        default:
        break;
    }
}

void bt_key_handle_func_key(enum APP_KEY_EVENT_T event)
{
    switch (event) {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            bt_key_handle_func_click();
            break;
        case  APP_KEY_EVENT_DOUBLECLICK:
            bt_key_handle_func_doubleclick();
            break;
        case APP_KEY_EVENT_TRIPLECLICK:
            bt_key_handle_func_tripleclick(); 
            break;
        case  APP_KEY_EVENT_LONGPRESS:
            bt_key_handle_func_longpress();
            break;
        default:
            TRACE(0,"unregister func key event=%x", event);
            break;
    }
}

void app_bt_volumeup(void)
{
    app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_UP, 0);
}

void app_bt_volumedown(void)
{
    app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN, 0);
}

#if defined(__APP_KEY_FN_STYLE_A__)
void bt_key_handle_up_key(enum APP_KEY_EVENT_T event)
{
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            app_bt_volumeup();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_LONGPRESS:
            a2dp_handleKey(AVRCP_KEY_FORWARD);
            break;
#endif
#if defined(BT_SOURCE)
        case  APP_KEY_EVENT_DOUBLECLICK:
            if (app_bt_source_nv_snk_or_src_enabled())  {
                //debug switch src mode
                struct nvrecord_env_t *nvrecord_env=NULL;
                nv_record_env_get(&nvrecord_env);
                if(app_bt_source_is_enabled())
                {
                    nvrecord_env->src_snk_flag.src_snk_mode = 1;
                }
                else
                {
                    nvrecord_env->src_snk_flag.src_snk_mode = 0;
                }
                nv_record_env_set(nvrecord_env);
                app_reset();
            }
            break;
#endif
        default:
            TRACE(1,"unregister up key event=%x",event);
            break;
    }
}


void bt_key_handle_down_key(enum APP_KEY_EVENT_T event)
{
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            app_bt_volumedown();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_LONGPRESS:
            a2dp_handleKey(AVRCP_KEY_BACKWARD);
            break;
#endif
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
    }
}
#else //#elif defined(__APP_KEY_FN_STYLE_B__)
void bt_key_handle_up_key(enum APP_KEY_EVENT_T event)
{
    TRACE(1,"%s",__func__);
    switch(event)
    {
        case  APP_KEY_EVENT_REPEAT:
            app_bt_volumeup();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            a2dp_handleKey(AVRCP_KEY_FORWARD);
            break;
#endif
        default:
            TRACE(1,"unregister up key event=%x",event);
            break;
    }
}


void bt_key_handle_down_key(enum APP_KEY_EVENT_T event)
{
    switch(event)
    {
        case  APP_KEY_EVENT_REPEAT:
            app_bt_volumedown();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            a2dp_handleKey(AVRCP_KEY_BACKWARD);
            break;
#endif
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
    }
}
#endif

#if defined(BT_SOURCE)
void bt_key_handle_source_func_key(enum APP_KEY_EVENT_T event)
{
    uint8_t connected_source_count = 0;
    TRACE(2,"%s,%d",__FUNCTION__,event);
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            connected_source_count = app_bt_source_count_connected_device();
            if (connected_source_count < BT_SOURCE_DEVICE_NUM)
            {
                app_bt_source_search_device();
            }
            break;
        case APP_KEY_EVENT_LONGPRESS:
#if defined(BT_HFP_AG_ROLE)
            app_hfp_ag_toggle_audio_link();
#endif
            break;
        case APP_KEY_EVENT_DOUBLECLICK:
            app_a2dp_source_toggle_stream(app_bt_source_get_current_a2dp());
            break;
        case APP_KEY_EVENT_TRIPLECLICK:
            app_a2dp_source_start_stream(app_bt_source_get_current_a2dp());
            break;
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
    }
}
#endif

APP_KEY_STATUS bt_key;
static void bt_update_key_event(uint32_t code, uint8_t event)
{
    TRACE(3,"%s code:%d evt:%d",__func__, code, event);

    bt_key.code = code;
    bt_key.event = event;
    osapi_notify_evm();
}

void bt_key_send(APP_KEY_STATUS *status)
{
    uint32_t lock = int_lock();
    bool isKeyBusy = false;
    if (0xff != bt_key.code)
    {
        isKeyBusy = true;
    }
    int_unlock(lock);

    if (!isKeyBusy)
    {
        app_bt_start_custom_function_in_bt_thread(
            (uint32_t)status->code,
            (uint32_t)status->event,
            (uint32_t)bt_update_key_event);
    }
}

void bt_key_handle(void)
{
    if(bt_key.code != 0xff)
    {
        TRACE(3,"%s code:%d evt:%d",__func__, bt_key.code, bt_key.event);
        switch(bt_key.code)
        {
            case BTAPP_FUNC_KEY:
#if defined(BT_SOURCE)
                if(app_bt_source_is_enabled())
                {
                    bt_key_handle_source_func_key((enum APP_KEY_EVENT_T)bt_key.event);
                }
                else
#endif
                {
                    bt_key_handle_func_key((enum APP_KEY_EVENT_T)bt_key.event);
                }
                break;
            case BTAPP_VOLUME_UP_KEY:
                bt_key_handle_up_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
            case BTAPP_VOLUME_DOWN_KEY:
                bt_key_handle_down_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
#ifdef SUPPORT_SIRI
            case BTAPP_RELEASE_KEY:
                bt_key_handle_siri_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
#endif
            default:
                TRACE(0,"bt_key_handle  undefined key");
                break;
        }
        bt_key.code = 0xff;
    }
}

void bt_key_init(void)
{
#ifdef APP_KEY_ENABLE
    Besbt_hook_handler_set(BESBT_HOOK_USER_2, bt_key_handle);
    bt_key.code = 0xff;
    bt_key.event = 0xff;
#endif
}

