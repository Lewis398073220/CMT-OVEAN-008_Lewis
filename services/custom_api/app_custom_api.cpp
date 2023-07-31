
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
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "hci_api.h"
#include "me_api.h"
#include "besaud_api.h"
#include "app_utils.h"
#include "bt_if.h"
#include "hfp_api.h"
#include "avrcp_api.h"
#include "a2dp_api.h"
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_conn_api.h"
#include "app_custom_api.h"
#include "bt_drv_reg_op.h"
#include "hal_timer.h"
#include "app_thread.h"
#include "app_tws_ctrl_thread.h"
#include "app_bt.h"
#include "btapp.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "factory_section.h"
#include "btm_i.h"
#include "app_bt_func.h"
#include "app_ui_api.h"
#include "apps.h"
#include "app_custom_api.h"
#include "app_ui_evt_thread.h"
#include "app_custom_thread.h"
#include "app_custom_adapter.h"

#ifdef GFPS_ENABLED
#include "gfps.h"
#ifdef SASS_ENABLED
#include "gfps_sass.h"
#endif
#endif

#if defined(__IAG_BLE_INCLUDE__)
#include "bluetooth_ble_api.h"
#if BLE_AUDIO_ENABLED
#include "ble_audio_core.h"
#include "aob_media_api.h"
#include "aob_bis_api.h"
#include "aob_csip_api.h"
#include "app_sec.h"
#include "ble_audio_core_api.h"
#endif
#include "app_ble_include.h"
#endif

#include "beslib_info.h"

void app_custom_ui_notify_ibrt_core_event(ibrt_conn_evt_header* evt)
{
#if defined(IBRT_UI_V2)
    app_ui_notify_bt_core_event((ibrt_conn_event_packet*)evt);
#endif
#ifdef __REPORT_EVENT_TO_CUSTOMIZED_UX__
    app_custom_ux_notify_bt_event(evt);
#endif
}

void app_custom_ui_notify_bluetooth_enabled(void)
{
    ibrt_global_state_change_event evt;
    evt.header.type = IBRT_CONN_EVENT_GLOBAL_STATE;
    evt.state = IBRT_BLUETOOTH_ENABLED;
    app_custom_ui_notify_ibrt_core_event(&evt.header);
}

void app_custom_ui_notify_bluetooth_disabled(void)
{
    ibrt_global_state_change_event evt;
    evt.header.type = IBRT_CONN_EVENT_GLOBAL_STATE;
    evt.state = IBRT_BLUETOOTH_DISABLED;
    app_custom_ui_notify_ibrt_core_event(&evt.header);
}



/*****************************************************************************
 Prototype    : app_custom_ui_a2dp_callback
 Description  : ibrt a2dp callback function
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/12/16
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_custom_ui_a2dp_callback(uint8_t device_id, void *param1,void *param2,void* param3)
{
    a2dp_stream_t *a2dp_stream = (a2dp_stream_t *)param1;
    btif_a2dp_callback_parms_t *info = (btif_a2dp_callback_parms_t *)param2;
    ibrt_conn_a2dp_state_change evt = {{(ibrt_conn_event_type)0}};
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    evt.header.type = IBRT_CONN_EVENT_A2DP_STATE;
    memcpy((uint8_t*)&evt.addr,(uint8_t*)param3,sizeof(bt_bdaddr_t));
    evt.device_id = device_id;

    if (info->event != BTIF_A2DP_EVENT_STREAM_DATA_IND)
    {
        TRACE(1,"[Notify]A2dp callback event=%d", info->event);
    }

    switch(info->event)
    {
        case BTIF_A2DP_EVENT_STREAM_OPEN:
        case BTIF_A2DP_EVENT_STREAM_OPEN_MOCK:
            if (curr_device->mock_a2dp_after_force_disc)
            {
                break;
            }
            evt.a2dp_state = IBRT_CONN_A2DP_OPEN;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_A2DP_EVENT_STREAM_CLOSED:
            if (curr_device->ibrt_slave_force_disc_a2dp)
            {
                break;
            }
            if (btif_a2dp_is_disconnected(a2dp_stream))
            {
                evt.a2dp_state = IBRT_CONN_A2DP_IDLE;
            }
            else
            {
                evt.a2dp_state = IBRT_CONN_A2DP_CLOSED;
            }
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_A2DP_EVENT_STREAM_SUSPENDED:
            evt.a2dp_state = IBRT_CONN_A2DP_SUSPENED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_A2DP_EVENT_CODEC_INFO:
        case BTIF_A2DP_EVENT_CODEC_INFO_MOCK:
        case BTIF_A2DP_EVENT_STREAM_RECONFIG_IND:
            evt.a2dp_state = IBRT_CONN_A2DP_CODEC_CONFIGURED;
            evt.audio_settings.codec = (ibrt_conn_codec_type)info->p.codec->codecType;
            evt.delay_report_support = btif_a2dp_is_stream_device_has_delay_reporting(a2dp_stream);
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_A2DP_EVENT_STREAM_STARTED:
        case BTIF_A2DP_EVENT_STREAM_STARTED_MOCK:
            evt.a2dp_state = IBRT_CONN_A2DP_STREAMING;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        default:
            evt.a2dp_state = IBRT_CONN_A2DP_STREAMING;
            break;
    }
#ifdef SASS_ENABLED
    if(info->event == BTIF_A2DP_EVENT_STREAM_OPEN || info->event == BTIF_A2DP_EVENT_STREAM_OPEN_MOCK \
        || info->event == BTIF_A2DP_EVENT_STREAM_CLOSED || info->event == BTIF_A2DP_EVENT_STREAM_SUSPENDED \
        || info->event == BTIF_A2DP_EVENT_STREAM_STARTED || info->event == BTIF_A2DP_EVENT_STREAM_STARTED_MOCK)
    {
        gfps_sass_profile_event_handler(SASS_PROFILE_A2DP, SET_BT_ID(device_id), info->event, NULL);
    }
#endif

}

/*****************************************************************************
 Prototype    : app_custom_ui_hfp_callback
 Description  : app tws ibrt hfp callback
 Input        : void *param1
                void *param2
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/12/16
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void WEAK app_custom_ui_hfp_callback(uint8_t device_id, void *param1,void *param2,void* param3)
{
    struct hfp_context *hfp_ctx = (hfp_context *)param2;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    ibrt_conn_hfp_state_change evt = {{(ibrt_conn_event_type)0}};
    evt.header.type = IBRT_CONN_EVENT_HFP_STATE;
    memcpy((uint8_t*)&evt.addr,(uint8_t*)param3,sizeof(bt_bdaddr_t));
    evt.device_id = device_id;

    switch(hfp_ctx->event)
    {
        case BTIF_HF_EVENT_SERVICE_DISCONNECTED:
            if (curr_device->ibrt_slave_force_disc_hfp)
            {
                break;
            }
            evt.hfp_state = IBRT_CONN_HFP_SLC_DISCONNECTED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_SERVICE_CONNECTED:
        case BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED:
            if (curr_device->mock_hfp_after_force_disc)
            {
                break;
            }
            evt.hfp_state = IBRT_CONN_HFP_SLC_OPEN;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_AUDIO_CONNECTED:
            evt.hfp_state = IBRT_CONN_HFP_SCO_OPEN;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_AUDIO_DISCONNECTED:
            evt.hfp_state = IBRT_CONN_HFP_SCO_CLOSED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_RING_IND:
            evt.hfp_state = IBRT_CONN_HFP_RING_IND;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_CALL_IND:
            if (curr_device->hf_conn_flag || hfp_ctx->call)
            {
                evt.hfp_state = IBRT_CONN_HFP_CALL_IND;
                evt.ciev_status = hfp_ctx->call;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            break;

        case BTIF_HF_EVENT_CALLSETUP_IND:
            if (curr_device->hf_conn_flag || hfp_ctx->call_setup)
            {
                evt.hfp_state = IBRT_CONN_HFP_CALLSETUP_IND;
                evt.ciev_status = hfp_ctx->call_setup;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            break;

        case BTIF_HF_EVENT_CALLHELD_IND:
            if (curr_device->hf_conn_flag || hfp_ctx->call_held)
            {
                evt.hfp_state = IBRT_CONN_HFP_CALLHELD_IND;
                evt.ciev_status = hfp_ctx->call_held;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            break;

        case BTIF_HF_EVENT_SERVICE_IND:
            if (curr_device->hf_conn_flag)
            {
                evt.hfp_state = IBRT_CONN_HFP_CIEV_SERVICE_IND;
                evt.ciev_status = hfp_ctx->ciev_status;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            break;

        case BTIF_HF_EVENT_SIGNAL_IND:
            if (curr_device->hf_conn_flag)
            {
                evt.hfp_state = IBRT_CONN_HFP_CIEV_SIGNAL_IND;
                evt.ciev_status = hfp_ctx->ciev_status;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            break;

        case BTIF_HF_EVENT_ROAM_IND:
            if (curr_device->hf_conn_flag)
            {
                evt.hfp_state = IBRT_CONN_HFP_CIEV_ROAM_IND;
                evt.ciev_status = hfp_ctx->ciev_status;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            break;

        case BTIF_HF_EVENT_BATTERY_IND:
            if (curr_device->hf_conn_flag)
            {
                evt.hfp_state = IBRT_CONN_HFP_CIEV_BATTCHG_IND;
                evt.ciev_status = hfp_ctx->ciev_status;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            break;

        case BTIF_HF_EVENT_SPEAKER_VOLUME:
            evt.hfp_state = IBRT_CONN_HFP_SPK_VOLUME_IND;
            evt.volume_ind = hfp_ctx->speaker_volume;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_MIC_VOLUME:
            evt.hfp_state = IBRT_CONN_HFP_MIC_VOLUME_IND;
            evt.volume_ind = hfp_ctx->mic_volume;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_IN_BAND_RING:
            evt.hfp_state = IBRT_CONN_HFP_IN_BAND_RING_IND;
            evt.in_band_ring_enable = hfp_ctx->bsir_enable;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;
        case BTIF_HF_EVENT_VOICE_REC_STATE:
            evt.hfp_state = IBRT_CONN_HFP_VR_STATE_IND;
            evt.voice_rec_state = hfp_ctx->voice_rec_state;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_COMMAND_COMPLETE:
            evt.hfp_state = IBRT_CONN_HFP_AT_CMD_COMPLETE;
            evt.error_code = hfp_ctx->error_code;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        default:
            evt.hfp_state = IBRT_CONN_HFP_SLC_DISCONNECTED;
            break;
    }

#ifdef SASS_ENABLED
    if (hfp_ctx->event == BTIF_HF_EVENT_SERVICE_CONNECTED || hfp_ctx->event == BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED \
        || hfp_ctx->event == BTIF_HF_EVENT_SERVICE_DISCONNECTED || hfp_ctx->event == BTIF_HF_EVENT_CALL_IND \
        || hfp_ctx->event == BTIF_HF_EVENT_RING_IND || hfp_ctx->event == BTIF_HF_EVENT_AUDIO_CONNECTED \
        || hfp_ctx->event == BTIF_HF_EVENT_AUDIO_DISCONNECTED)
    {
        uint8_t *p = NULL;
        if (hfp_ctx->event == BTIF_HF_EVENT_CALL_IND)
        {
            p = (uint8_t *)&(hfp_ctx->call);
        }
        gfps_sass_profile_event_handler(SASS_PROFILE_HFP, SET_BT_ID(device_id), hfp_ctx->event, p);
    }
#endif


    return;
}
/*****************************************************************************
 Prototype    : app_custom_ui_avrcp_callback
 Description  : app ibrt avrcp callback
 Input        : void *param1
                void *param2
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/12/16
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_custom_ui_avrcp_callback(uint8_t device_id, void *param1,void *param2,void* param3)
{
    btif_avrcp_channel_t *btif_avrcp = (btif_avrcp_channel_t *)param1;
    btif_avctp_event_t avrcp_event = btif_avrcp_get_callback_event((avrcp_callback_parms_t *)param2);
    avrcp_callback_parms_t *avrcp_param_ptr = (avrcp_callback_parms_t *)param2;
    ibrt_conn_avrcp_state_change evt = {{(ibrt_conn_event_type)0}};
    struct avrcp_remote_sdp_info remote_sdp_info = {0};
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    btif_avrcp_operation_t avrcp_op;
    avrcp_adv_notify_parms_t *avrcp_notify_params = NULL;
#ifdef SASS_ENABLED
    bool ntfSass = false;
    uint8_t sassParam;
#endif

    evt.header.type = IBRT_CONN_EVENT_AVRCP_STATE;
    memcpy((uint8_t*)&evt.addr,(uint8_t*)param3,sizeof(bt_bdaddr_t));
    evt.device_id = device_id;

    TRACE(1,"[Notify]Avrcp callback event=%d", avrcp_event);
    switch (avrcp_event)
    {
        case BTIF_AVCTP_CONNECT_EVENT:
        case BTIF_AVCTP_CONNECT_EVENT_MOCK:
            if (curr_device->mock_avrcp_after_force_disc)
            {
                break;
            }
            evt.avrcp_state = IBRT_CONN_AVRCP_CONNECTED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_AVCTP_DISCONNECT_EVENT:
            if (curr_device->ibrt_slave_force_disc_avrcp)
            {
                break;
            }
            evt.avrcp_state = IBRT_CONN_AVRCP_DISCONNECTED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_AVRCP_EVENT_CT_SDP_INFO:
            remote_sdp_info = btif_avrcp_get_remote_sdp_info(btif_avrcp, false);
            evt.avrcp_state = IBRT_CONN_AVRCP_REMOTE_CT_0104;
            evt.volume = app_bt_get_a2dp_current_abs_volume(evt.device_id);

            if ((remote_sdp_info.remote_avrcp_version >= 0x0103) && (remote_sdp_info.remote_support_features & 0x2))
            {
                evt.support = true;
            }
            else
            {
                evt.support = false;
            }
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_AVRCP_EVENT_PLAYBACK_STATUS_CHANGE_EVENT_SUPPORT:
            evt.avrcp_state = IBRT_CONN_AVRCP_REMOTE_SUPPORT_PLAYBACK_STATUS_CHANGED_EVENT;
            evt.support = app_bt_get_device(evt.device_id)->avrcp_remote_support_playback_status_change_event;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
            evt.avrcp_state = IBRT_CONN_AVRCP_PLAYBACK_STATUS_CHANGED;
            evt.playback_status = app_bt_get_device(evt.device_id)->avrcp_palyback_status;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
#ifdef SASS_ENABLED
            ntfSass = true;
            sassParam = evt.playback_status;
#endif
            break;

        case BTIF_AVRCP_EVENT_ADV_RESPONSE://18
            if (btif_get_avrcp_cb_channel_state(avrcp_param_ptr) != BT_STS_SUCCESS)
            {
                break;
            }
            avrcp_op = btif_get_avrcp_cb_channel_advOp(avrcp_param_ptr);
            if (avrcp_op == BTIF_AVRCP_OP_GET_CAPABILITIES)
            {
                evt.avrcp_state = IBRT_CONN_AVRCP_REMOTE_SUPPORT_PLAYBACK_STATUS_CHANGED_EVENT;
                evt.support = app_bt_get_device(evt.device_id)->avrcp_remote_support_playback_status_change_event;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            else if(avrcp_op == BTIF_AVRCP_OP_REGISTER_NOTIFY)
            {
                avrcp_notify_params = btif_get_avrcp_adv_notify(avrcp_param_ptr);
                if (avrcp_notify_params->event == BTIF_AVRCP_EID_MEDIA_STATUS_CHANGED)
                {
                    evt.avrcp_state = IBRT_CONN_AVRCP_PLAYBACK_STATUS_CHANGED;
                    evt.playback_status = avrcp_notify_params->p.mediaStatus;
                    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
#ifdef SASS_ENABLED
                    ntfSass = true;
                    sassParam = evt.playback_status;
#endif
                }
            }
            break;
        case BTIF_AVRCP_EVENT_ADV_NOTIFY://17
            avrcp_notify_params = btif_get_avrcp_adv_notify(avrcp_param_ptr);
            if(avrcp_notify_params->event == BTIF_AVRCP_EID_MEDIA_STATUS_CHANGED)
            {
                evt.avrcp_state = IBRT_CONN_AVRCP_PLAYBACK_STATUS_CHANGED;
                evt.playback_status = avrcp_notify_params->p.mediaStatus;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
#ifdef SASS_ENABLED
                ntfSass = true;
                sassParam = evt.playback_status;
#endif
            }
            break;
        case BTIF_AVRCP_EVENT_COMMAND:
            {
                struct BT_DEVICE_T *bt_dev;
                avctp_cmd_frame_t * cmd_frame = btif_get_avrcp_cmd_frame(avrcp_param_ptr);

                ASSERT(cmd_frame, "get frame wrong");
                if (cmd_frame->ctype == BTIF_AVCTP_CTYPE_CONTROL)
                {
                    if (cmd_frame->operands[3] == BTIF_AVRCP_OP_SET_ABSOLUTE_VOLUME)
                    {
                        evt.avrcp_state = IBRT_CONN_AVRCP_VOLUME_UPDATED;
                        bt_dev = app_bt_get_device(evt.device_id);
                        evt.volume = bt_dev->a2dp_current_abs_volume;
                        app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
                    }
                }
            }
            break;
        default:
            TRACE(1,"tws_ibrt_log:AVRCP callback event=0x%x", avrcp_event);
            evt.avrcp_state = IBRT_CONN_AVRCP_DISCONNECTED;
            break;
    }

#ifdef SASS_ENABLED
    if(avrcp_event == BTIF_AVCTP_CONNECT_EVENT || avrcp_event == BTIF_AVCTP_CONNECT_EVENT_MOCK || \
        avrcp_event == BTIF_AVCTP_DISCONNECT_EVENT || ((avrcp_event == BTIF_AVRCP_EVENT_ADV_NOTIFY || \
        avrcp_event == BTIF_AVRCP_EVENT_ADV_RESPONSE || avrcp_event == BTIF_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED) && \
        ntfSass))
    {
        gfps_sass_profile_event_handler(SASS_PROFILE_AVRCP, SET_BT_ID(device_id), avrcp_event, &sassParam);
    }
#endif
}

void app_custom_ui_gsound_state_change(bool state)
{

}

void app_custom_ui_tws_on_paring_state_changed(ibrt_conn_pairing_state state,uint8_t reason_code)
{
    ibrt_conn_tws_pairing_state_evt  POSSIBLY_UNUSED evt;

    evt.header.type   = IBRT_CONN_EVENT_TW_PAIRING_STATE;
    TRACE(1,"[Notify]TWS pairing state changed: %d",state);
    evt.pairing_state = state;
    evt.connection_state.bluetooth_reason_code = reason_code;

    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
}

void WEAK app_custom_ui_tws_on_acl_state_changed(ibrt_conn_acl_state state,uint8_t reason_code)
{
    ibrt_conn_tws_conn_state_event  POSSIBLY_UNUSED evt;

    evt.header.type     = IBRT_CONN_EVENT_TW_CONNECTION_STATE;
    evt.state.acl_state  = state;

    if((state == IBRT_CONN_ACL_DISCONNECTED)
        || (state == IBRT_CONN_ACL_CONNECTING_CANCELED)
        || (state == IBRT_CONN_ACL_CONNECTING_FAILURE))
    {
        evt.state.bluetooth_reason_code = reason_code;
        #ifdef IBRT_UI_V2
        if (app_ui_get_config()->is_changed_to_ui_master_on_tws_disconnected)
        {
            app_ibrt_conn_set_ui_role(TWS_UI_MASTER);
        }
        else
        #endif
        {
            app_ibrt_conn_set_ui_role(TWS_UI_UNKNOWN);
        }
    }

#if BLE_AUDIO_ENABLED
    if (state == IBRT_CONN_ACL_PROFILES_CONNECTED)
    {
        if (GETB(app_bap_get_role_bit_filed(), BAP_ROLE_SUPP_BC_SINK))
        {
#if defined(CFG_BAP_BC)
            bis_tws_trans_handler_t* aob_bis_tws_evt_handler = bis_get_tws_evt_handler();
            if (aob_bis_tws_evt_handler->bis_tws_rejoin_cb)
            {
                aob_bis_tws_evt_handler->bis_tws_rejoin_cb();
            }
#endif
        }
    }
#endif

    evt.current_role = app_ibrt_conn_get_ui_role();

    TRACE(3,"[Notify]TWS ACL state changed =0x%x, reason code:%d role %d", state, reason_code, evt.current_role);

    if ((state == IBRT_CONN_ACL_SIMPLE_PIARING_COMPLETE) ||
        (state == IBRT_CONN_ACL_AUTH_COMPLETE)) {
        return ;
    }

    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
}

void app_custom_ui_on_mobile_paring_state_changed(const bt_bdaddr_t *addr,ibrt_conn_pairing_state state,uint8_t reason_code)
{

}

void WEAK app_custom_ui_on_mobile_acl_state_changed(const bt_bdaddr_t *addr,ibrt_conn_acl_state state,uint8_t reason_code)
{
    ibrt_mobile_conn_state_event evt;
    evt.header.type     = IBRT_CONN_EVENT_MOBILE_CONNECTION_STATE;
    evt.state.bluetooth_reason_code = 0;
    memcpy((void*)&evt.addr,addr,sizeof(bt_bdaddr_t));
    evt.state.acl_state = state;
    evt.device_id = app_bt_get_device_id_byaddr(&evt.addr);
    evt.current_role = app_tws_get_ibrt_role(&evt.addr);

    if((state == IBRT_CONN_ACL_DISCONNECTED)
        || (state == IBRT_CONN_ACL_CONNECTING_FAILURE)
        || (state == IBRT_CONN_ACL_AUTH_COMPLETE))
    {
        evt.state.bluetooth_reason_code = reason_code;
    }
    TRACE(3,"(d%x) [Notify]mobile ACL state changed %d reason_code %x", evt.device_id, state, reason_code);
#if defined(IBRT_UI_V2)
    if ((state == IBRT_CONN_ACL_RAW_CONNECTED) || (state == IBRT_CONN_ACL_SIMPLE_PIARING_COMPLETE))
    {
        return ;
    }
#endif

#ifdef GFPS_ENABLED
    if ((state == IBRT_CONN_ACL_DISCONNECTED) && gfps_is_last_response_pending())
    {
        gfps_enter_connectable_mode_req_handler(gfps_get_last_response());
    }
#endif

    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
}

void app_custom_ui_on_mobile_sco_state_changed(const bt_bdaddr_t *addr,ibrt_conn_sco_state state,uint8_t reason_code)
{
    ibrt_sco_conn_state_event evt;
    evt.header.type     = IBRT_CONN_EVENT_SCO_CONNECTION_STATE;
    evt.state.sco_reason_code = 0;
    memcpy((void*)&evt.addr,addr,sizeof(bt_bdaddr_t));
    evt.state.sco_state = state;
    evt.device_id = app_bt_get_device_id_byaddr(&evt.addr);
    evt.current_role = app_tws_get_ibrt_role(&evt.addr);

    if((state == IBRT_CONN_SCO_DISCONNECTED)
        || (state == IBRT_CONN_SCO_CONNECTED))
    {
        evt.state.sco_reason_code = reason_code;
    }
    TRACE(3,"(d%x) [Notify]mobile sco state changed %d reason_code %x", evt.device_id, state, reason_code);
    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
}
void WEAK app_custom_ui_on_ibrt_state_changed(const bt_bdaddr_t *addr,
    ibrt_conn_ibrt_state state,ibrt_role_e role,uint8_t reason_code)
{
    ibrt_connection_state_event ibrt_state_event;

    ibrt_state_event.header.type = IBRT_CONN_EVENT_IBRT_CONNECTION_STATE;
    memcpy((uint8_t*)&ibrt_state_event.addr,addr,sizeof(bt_bdaddr_t));
    ibrt_state_event.state.ibrt_state = state;
    ibrt_state_event.device_id = app_bt_get_device_id_byaddr(&ibrt_state_event.addr);
    ibrt_state_event.current_role = app_tws_get_ibrt_role(&ibrt_state_event.addr);

    if(state == IBRT_CONN_IBRT_DISCONNECTED)
    {
        ibrt_state_event.state.ibrt_reason_code = reason_code;
#if defined(USE_SAFE_DISCONNECT)
        app_custom_ui_safe_disconnect_process(ibrt_state_event.device_id);
#endif
    }
#ifdef TWS_RS_BY_BTC
    else if (state == IBRT_CONN_IBRT_CONNECTED)
    {

        app_ibrt_conn_btc_role_switch(addr);
    }
#endif

    TRACE(4,"(d%x) [Notify]IBRT state changed %d role %x reason %x", ibrt_state_event.device_id, state, role, reason_code);
    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&ibrt_state_event);
}

void app_custom_ui_on_phone_connect_ind(const bt_bdaddr_t *addr)
{
#if defined(IBRT_UI_V2)
    app_ui_ibrt_pkt phone_connect_evt;

    phone_connect_evt.header.type = APP_UI_EVENT_OPERATION_STATE;
    phone_connect_evt.ibrt_evt = APP_UI_EV_PHONE_CONNECT;
    memcpy((uint8_t*)&phone_connect_evt.addr,addr,sizeof(bt_bdaddr_t));

    TRACE(0,"[Notify] phone connect indication");
    app_ui_notify_ui_event((app_ui_evt_pkt*)&phone_connect_evt);
#endif
}

void WEAK app_custom_ui_on_snoop_state_changed(const bt_bdaddr_t *addr,uint8_t snoop_connected)
{
}

void WEAK app_custom_ui_on_tws_role_changed(const bt_bdaddr_t *addr,ibrt_conn_role_change_state state,ibrt_role_e role)
{
    ibrt_conn_tw_role_change_state_event  POSSIBLY_UNUSED twsRoleChangeEvt;
    twsRoleChangeEvt.header.type     = IBRT_CONN_EVENT_TW_ROLE_CHANGE_STATE;
    twsRoleChangeEvt.role_state = state;

    TRACE(2,"[Notify] role switch changed = %d,role = %d",state,role);
    //Address will be set to NULL if ibrt core disallow the role swap
    //memset the address to zero for the function app_ui_handle_role_change_evt can handle this case
    if (NULL != addr)
    {
        memcpy((uint8_t*)&twsRoleChangeEvt.addr,addr,sizeof(bt_bdaddr_t));
        DUMP8("%02x ",addr->address, BT_ADDR_OUTPUT_PRINT_NUM);
    }
    else
    {
        memset((uint8_t*)&twsRoleChangeEvt.addr, 0x00,sizeof(bt_bdaddr_t));
    }

    switch(state)
    {
        case IBRT_CONN_ROLE_SWAP_INITIATED:
            twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_SUCCESS;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
            break;
        case IBRT_CONN_ROLE_SWAP_COMPLETE:
            {
            twsRoleChangeEvt.role = role;
            twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_SUCCESS;
#ifdef CODEC_ERROR_HANDLING
            uint8_t device_id = app_bt_get_device_id_byaddr(&twsRoleChangeEvt.addr);
            app_ibrt_conn_switch_to_ibrt_slave_process(device_id);
#endif
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
            }
            break;
        case IBRT_CONN_ROLE_CHANGE_COMPLETE: //update role
            twsRoleChangeEvt.role = role;
            twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_SUCCESS;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
            break;
        case IBRT_CONN_ROLE_SWAP_DISALLOW:
            twsRoleChangeEvt.role = role;
            twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_ERROR_OP_NOT_ALLOWED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
        break;
        case IBRT_CONN_ROLE_SWAP_FAILED:
            twsRoleChangeEvt.role = role;
            twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_ERROR_ROLE_SWITCH_FAILED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
        break;
        default:
            break;
    }
}

#if BLE_AUDIO_ENABLED
/*
 * CTKD OVER EDR
*/
static void app_custom_ui_ble_smp_ia_exch_cmp(uint8_t *ia_addr)
{
    ibrt_conn_smp_ia_exch_cmp_t evt;

    evt.header.type = IBRT_CONN_EVENT_SMP_IA_EXCH_CMP;
    memcpy(evt.addr,ia_addr,BLE_ADDR_SIZE);

    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
}
#endif

#if defined(USE_SAFE_DISCONNECT)
/* tws safe disconenct implement */
#define IBRT_DISCONNECT_MAX_TRY_TIME (3)
static uint8_t stop_ibrt_try_count = 0;
static uint8_t wait_ibrt_disconnect_mask = 0x00;
typedef ibrt_status_t (*ibrt_disconn_post_func)(void);
static ibrt_disconn_post_func ibrt_post = NULL;
void wait_for_ibrt_disconnect_timer_cb(void const *arg);
osTimerDef(wait_tws_diconnect, wait_for_ibrt_disconnect_timer_cb);  // when the timer expires, the function start_machine is called
osTimerId wait_ibrt_diconnect_id = NULL;

bool stop_ibrt_ongoing(void)
{
    return (wait_ibrt_disconnect_mask!=0);
}

void wait_ibrt_disconnect_timer_start()
{
    if (wait_ibrt_diconnect_id == NULL) {
        wait_ibrt_diconnect_id = osTimerCreate(osTimer(wait_tws_diconnect), osTimerOnce, (void *)0);
        ASSERT(wait_ibrt_diconnect_id, "wait_tws_disconnect_timer create failed");
    }

    osTimerStart(wait_ibrt_diconnect_id, WAIT_FOR_IBRT_DISCONNECT_TIMEOUT);
}

void wait_for_ibrt_disconnect_timer_cb(void const *arg)
{
    TRACE(0, "custom_api:: wait for ibrt disconnect timer timeout!!!");
    bt_bdaddr_t mobile_addr_list[BT_DEVICE_NUM];
    uint8_t ibrt_count = app_ibrt_conn_get_ibrt_connected_list(mobile_addr_list);
    wait_ibrt_disconnect_mask = 0;
    app_ibrt_conn_set_stop_ibrt_ongoing(false);
    if (ibrt_count == 0 || (stop_ibrt_try_count++ == IBRT_DISCONNECT_MAX_TRY_TIME)) {
        TRACE(0, "custom_api:: no ibrt links, trigger post func");
        if (ibrt_post) {
            ibrt_post();
        }
        return ;
    }
    TRACE(0, "custom_api:: start ibrt disconnect timer again");
    wait_ibrt_disconnect_timer_start();
    for (uint8_t i = 0; i < ibrt_count; i ++) {
        uint8_t device_id = app_bt_get_device_id_byaddr(&mobile_addr_list[i]);
        wait_ibrt_disconnect_mask |= (1 << device_id);
        app_ibrt_conn_set_stop_ibrt_ongoing(true);
        TRACE(3, "custom_api:: %s device_id=%d, mask=0x%x", __func__, device_id, wait_ibrt_disconnect_mask);
        app_ibrt_conn_disconnect_ibrt(&mobile_addr_list[i]);
    }
}

void app_custom_ui_safe_disconnect_process(uint8_t device_id)
{
    if (wait_ibrt_disconnect_mask) {
        wait_ibrt_disconnect_mask &= ~(1 << device_id);
        if (wait_ibrt_disconnect_mask == 0) {
            app_ibrt_conn_set_stop_ibrt_ongoing(false);
            TRACE(0, "custom_api:: all ibrt disconnected, trigger post func!!!");
            osTimerStop(wait_ibrt_diconnect_id);
            if (ibrt_post) {
                ibrt_post();
            }
        }
    }
}

void app_custom_ui_safe_disconnect(ibrt_disconn_post_func post_func)
{
    bt_bdaddr_t mobile_addr_list[BT_DEVICE_NUM];
    uint8_t ibrt_count = app_ibrt_conn_get_ibrt_connected_list(mobile_addr_list);
    ibrt_post = post_func;
    if (ibrt_count == 0) {
        TRACE(0, "custom_api:: no ibrt links, trigger post func");
        if (ibrt_post) {
            ibrt_post();
        }
        return ;
    }
    TRACE(0, "custom_api:: start ibrt disconnect timer");
    stop_ibrt_try_count = 0;
    wait_ibrt_disconnect_timer_start();
    for (uint8_t i = 0; i < ibrt_count; i ++) {
        uint8_t device_id = app_bt_get_device_id_byaddr(&mobile_addr_list[i]);
        wait_ibrt_disconnect_mask |= (1 << device_id);
        TRACE(3, "custom_api:: %s device_id=%d, mask=0x%x", __func__, device_id, wait_ibrt_disconnect_mask);
        app_ibrt_conn_set_stop_ibrt_ongoing(true);
        app_ibrt_conn_disconnect_ibrt(&mobile_addr_list[i]);
    }
}

/* safe disconnect tws */
void app_custom_ui_tws_safe_disconnect(void)
{
    app_custom_ui_safe_disconnect(app_ibrt_conn_tws_disconnect);
}

/* safe disconnect all links implement */
ibrt_status_t app_custom_ui_disconnect_all_mobile_links(void)
{
    bt_bdaddr_t mobile_addr_list[BT_DEVICE_NUM];
    uint8_t mobile_count = app_ibrt_conn_get_connected_mobile_list(mobile_addr_list);
    TRACE(0, "custom_api:: disconnect all mobile!!!");
    for (uint8_t i = 0; i < mobile_count; i ++) {
        uint8_t device_id = app_bt_get_device_id_byaddr(&mobile_addr_list[i]);
        TRACE(3, "custom_api:: %s connected device_id=%d", __func__, device_id);
        app_ibrt_conn_remote_dev_disconnect_request(&mobile_addr_list[i], NULL);
    }

    return IBRT_STATUS_SUCCESS;
}

void app_custom_ui_mobiles_safe_disconnect(void)
{
    app_custom_ui_safe_disconnect(app_custom_ui_disconnect_all_mobile_links);
}

/* safe disconnect all links */
ibrt_status_t app_custom_ui_disconnect_all_links(void)
{
    TRACE(0, "custom_api:: disconnect all mobiles and tws!!!");
    app_custom_ui_disconnect_all_mobile_links();
    app_ibrt_conn_tws_disconnect();
    return IBRT_STATUS_SUCCESS;
}

void app_custom_ui_all_safe_disconnect(void)
{
    app_custom_ui_safe_disconnect(app_custom_ui_disconnect_all_links);
}

/* cancel all links implementation*/
void app_custom_ui_cancel_all_connection(void)
{
    TRACE(0, "custom_api:: cancel paging device");
    bt_bdaddr_t mobile_addr_list[BT_DEVICE_NUM];
    ibrt_ctrl_t *p_ibrt_ctx = app_tws_ibrt_get_bt_ctrl_ctx();
    if (!app_ibrt_conn_is_tws_connected()) {
        btif_hci_cancel_create_connection((bt_bdaddr_t *)&p_ibrt_ctx->peer_addr);
    }
    uint8_t mobile_count = app_ibrt_conn_get_valid_device_list(mobile_addr_list);
    for (uint8_t i = 0; i < mobile_count; i ++) {
        if (app_ibrt_conn_is_mobile_connecting(&mobile_addr_list[i])) {
            btif_hci_cancel_create_connection((bt_bdaddr_t *)&mobile_addr_list[i]);
        }
    }
}
#endif

void app_custom_ui_access_mode_changed(btif_accessible_mode_t access_mode)
{
    ibrt_access_mode_change_event evt;
    evt.header.type = IBRT_CONN_EVENT_ACCESS_MODE;
    evt.access_mode = access_mode;
    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
}

void app_custom_ui_cancel_connect_state_changed(int8_t state)
{
    ibrt_conn_api_status_event POSSIBLY_UNUSED status;
    status.header.type   = IBRT_CONN_EVENT_API_STATUS;
    status.header.length = sizeof(ibrt_conn_api_status_event);
    status.command = IBRT_CORE_V2_HOST_CONNECT_CANCEL;

    TRACE(1,"[Notify] cancel connect state = %d", state);
    switch (state) {
      case BT_STS_SUCCESS:
        status.status = IBRT_CONN_STATUS_SUCCESS;
        break;
      case BT_STS_FAILED:
        status.status = IBRT_CONN_STATUS_ERROR_INVALID_STATE;
        break;
      case -1:
        status.status = IBRT_CONN_STATUS_ERROR_INVALID_PARAMETERS;
        break;
      default:
        status.status = IBRT_CONN_STATUS_ERROR_UNEXPECTED_VALUE;
        break;
    }
#if defined(IBRT_UI_V2)
    //app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&ibrt_conn_api_status_event);
#endif
}

void app_custom_ui_reset_tws_acl_data_packet_check(void)
{
    app_bt_reset_tws_acl_data_packet_check();
}

void app_custom_ui_handler_vender_evevnt(uint8_t evt_type, uint8_t * buffer, uint32_t length)
{
    #ifdef IBRT_UI_V2
    app_ui_handle_vender_event(evt_type, buffer, length);
    #endif
}


#if BLE_AUDIO_ENABLED

static void app_custom_ui_notify_le_audio_core_event(AOB_EVENT_HEADER_T *ev)
{
 #if defined(__REPORT_EVENT_TO_CUSTOMIZED_UX__)
    app_custom_ux_aob_event_notify(ev);
#endif

#if defined(IBRT_UI_V2)
    app_ui_notify_le_core_event((void*)ev);
#endif
}

static void app_custom_ui_sirk_refreshed_callback()
{
    TRACE(0,"sirk_refreshed_callback");
    AOB_EVENT_HEADER_T evt;

    evt.type = AOB_EVENT_SIRK_REFRESHED;
    app_custom_ui_notify_le_audio_core_event(&evt);
}

static void app_custom_ui_ble_audio_adv_state_changed_callback(AOB_ADV_STATE_T state, uint8_t err_code)
{
    AOB_EVENT_ADV_STATE_T evt;

    evt.header.type = AOB_EVENT_ADV_STATE;
    evt.adv_state = state;
    evt.err_code = err_code;
    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_tws_acl_state_changed_callback(uint32_t evt_type, ble_event_handled_t *p)
{
    ASSERT(p != NULL, "%s null pointer %p", __func__, p);

    AOB_EVENT_TWS_STATE_T evt;
    evt.header.type = AOB_EVENT_TW_CONNECTION_STATE;
    evt.evt_type = evt_type;
    evt.conidx = p->connect_handled.conidx;
    evt.peer_bdaddr.addr_type = p->connect_handled.peer_bdaddr.addr_type;
    memcpy(evt.peer_bdaddr.addr, p->connect_handled.peer_bdaddr.addr, BTIF_BD_ADDR_SIZE);

    switch (evt_type)
    {
        case BLE_TWS_CONNECTING:
        case BLE_TWS_CANCEL_CONNECTION:
        case BLE_TWS_CONNECTION_TIMEOUT:
            break;
        case BLE_TWS_CONNECTION_CANCELED:
            break;
        case BLE_TWS_CONNECTION_FAILED:
            evt.state.acl_state = AOB_ACL_FAILED;
            evt.state.err_code = p->connecting_failed_handled.err_code;
            break;
        case BLE_TWS_CONNECTED:
            evt.state.acl_state = AOB_ACL_CONNECTED;
            break;
        case BLE_TWS_DISCONNECTING:
            evt.state.acl_state = AOB_ACL_DISCONNECTING;
            break;
        case BLE_TWS_DISCONNECTED:
            evt.state.acl_state = AOB_ACL_DISCONNECTED;
            evt.state.err_code = p->connecting_failed_handled.err_code;
            break;
        default:
            TRACE(0, "ERROR, UNDEFINE TWS EVENT, PLEASE CHECK!!!");
            goto exit;
            break;
    }

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
exit:
        return;
}

static void app_custom_ui_ble_mobile_acl_state_changed_callback(uint32_t evt_type, ble_bdaddr_t *peer_addr, uint8_t con_idx,uint8_t err_code)
{
    AOB_EVENT_MOB_STATE_T evt;
    evt.header.type = AOB_EVENT_MOB_CONNECTION_STATE;
    evt.evt_type = evt_type;
    evt.conidx = con_idx;
    if ((evt_type != BLE_MOB_BOND_SUCCESS)  && (evt_type != BLE_MOB_BOND_FAIL))
    {
        memcpy((uint8_t*)&evt.peer_bdaddr,(uint8_t*)peer_addr,sizeof(ble_bdaddr_t));
    }

    switch (evt_type)
    {
        case BLE_MOB_CONNECTING:
        case BLE_MOB_CANCEL_CONNECTION:
            break;
        case BLE_MOB_CONNECTION_CANCELED:
            break;
        case BLE_MOB_CONNECTION_FAILED:
            break;
        case BLE_MOB_CONNECTION_TIMEOUT:
            break;
        case BLE_MOB_BOND_SUCCESS:
            evt.state.acl_state = AOB_ACL_BOND_SUCCESS;
            break;
        case BLE_MOB_BOND_FAIL:
            evt.state.acl_state = AOB_ACL_BOND_FAILURE;
            evt.state.err_code = err_code;
            break;
        case BLE_MOB_ENCRYPT:
            evt.state.acl_state = AOB_ACL_ENCRYPT;
            evt.state.err_code = err_code;
            break;
        case BLE_MOB_CONNECTED:
            evt.state.acl_state = AOB_ACL_CONNECTED;
            break;
        case BLE_MOB_DISCONNECTING:
            break;
        case BLE_MOB_DISCONNECTED:
            evt.state.acl_state = AOB_ACL_DISCONNECTED;
            evt.state.err_code = err_code;
            break;
        default:
            TRACE(0, "ERROR, UNDEFINE MOB EVENT, PLEASE CHECK!!!");
            goto exit;
            break;
    }

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
exit:
    return;
}

static void app_custom_ui_ble_vol_changed_callback(uint8_t con_lid, uint8_t volume,
                                                                        uint8_t mute, uint8_t reason)
{
    AOB_EVENT_VOL_CHANGED_T evt;
    evt.header.type = AOB_EVENT_VOL_CHANGED;

    evt.con_lid         = con_lid;
    evt.volume          = volume;
    evt.mute            = mute;
    evt.reason          = reason;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_vocs_offset_changed_callback(int16_t offset, uint8_t output_lid)
{
    AOB_EVENT_VOCS_OFFSET_CHANGED_T evt;
    evt.header.type = AOB_EVENT_VOCS_OFFSET_CHANGED;

    evt.offset = offset;
    evt.output_lid = output_lid;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_vocs_bond_data_changed_callback(uint8_t output_lid, uint8_t cli_cfg_bf)
{
    AOB_EVENT_VOCS_BOND_DATA_CHANGED_T evt;
    evt.header.type = AOB_EVENT_VOCS_BOND_DATA_CHANGED;

    evt.output_lid = output_lid;
    evt.cli_cfg_bf = cli_cfg_bf;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_media_track_change_cb(uint8_t con_lid)
{
    AOB_EVENT_MEDIA_TRACK_CHANGED_T evt;
    evt.header.type = AOB_EVENT_MEDIA_TRACK_CHANGED;

    evt.con_lid = con_lid;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_media_stream_status_change_cb(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state)
{
    AOB_EVENT_STREAM_STATUS_CHANGED_T evt;
    evt.header.type = AOB_EVENT_STREAM_STATUS_CHANGED;

    evt.con_lid = con_lid;
    evt.ase_lid = ase_lid;
    evt.state = state;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_media_playback_status_change_cb(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state)
{
    AOB_EVENT_PLAYBACK_STATUS_CHANGED_T evt;
    evt.header.type = AOB_EVENT_PLAYBACK_STATUS_CHANGED;

    evt.con_lid = con_lid;
    evt.state = state;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_media_mic_state_cb(uint8_t mute)
{
    AOB_EVENT_MIC_STATE_T evt;
    evt.header.type = AOB_EVENT_MIC_STATE;
    evt.mute = mute;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_media_iso_link_quality_cb(void *event)
{
    AOB_EVENT_ISO_LINK_QUALITY_IND_T evt;
    evt.header.type = AOB_EVENT_ISO_LINK_QUALITY_IND;
    memcpy(&evt.param, event, sizeof(evt.param));

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_media_pacs_cccd_written_cb(uint8_t con_lid)
{
    AOB_EVENT_PACS_CCCD_WRITTEN_IND_T evt;
    evt.header.type = AOB_EVENT_PACS_CCCD_WRITTEN_IND;

    evt.con_lid = con_lid;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_call_state_change_cb(uint8_t con_lid, void *param)
{
    AOB_EVENT_CALL_STATE_CHANGE_T evt;
    evt.header.type = AOB_EVENT_CALL_STATE_CHANGE;

    evt.con_lid = con_lid;
    evt.param = param;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_bble_call_srv_signal_strength_value_ind_cb(uint8_t con_lid, uint8_t call_id, uint8_t value)
{
    AOB_EVENT_CALL_SRV_SIG_STRENGTH_VALUE_IND_T evt;
    evt.header.type = AOB_EVENT_CALL_SRV_SIG_STRENGTH_VALUE_IND;

    evt.con_lid = con_lid;
    evt.value = value;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_call_status_flags_ind_cb(uint8_t con_lid, uint8_t call_id, bool inband_ring, bool silent_mode)
{
    AOB_EVENT_CALL_STATUS_FLAGS_IND_T evt;
    evt.header.type = AOB_EVENT_CALL_STATUS_FLAGS_IND;

    evt.con_lid = con_lid;
    evt.inband_ring = inband_ring;
    evt.silent_mode = silent_mode;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_call_ccp_opt_supported_opcode_ind_cb(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported)
{
    AOB_EVENT_CALL_CCP_OPT_SUPPORTED_OPCODE_IND_T evt;
    evt.header.type = AOB_EVENT_CALL_CCP_OPT_SUPPORTED_OPCODE_IND;

    evt.con_lid = con_lid;
    evt.local_hold_op_supported = local_hold_op_supported;
    evt.join_op_supported = join_op_supported;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_call_terminate_reason_ind_cb(uint8_t con_lid, uint8_t call_id, uint8_t reason)
{
    AOB_EVENT_CALL_TERMINATE_REASON_IND_T evt;
    evt.header.type = AOB_EVENT_CALL_TERMINATE_REASON_IND;

    evt.con_lid = con_lid;
    evt.call_id = call_id;
    evt.reason = reason;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_call_incoming_number_inf_ind_cb(uint8_t con_lid, uint8_t call_id, uint8_t url_len, uint8_t *url)
{
    AOB_EVENT_CALL_INCOMING_NUM_INF_IND_T evt;
    evt.header.type = AOB_EVENT_CALL_INCOMING_NUM_INF_IND;

    evt.con_lid = con_lid;
    evt.url_len = url_len;
    evt.url = url;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_call_svc_changed_ind_cb(uint8_t con_lid)
{
    AOB_EVENT_CALL_SVC_CHANGED_IND_T evt;
    evt.header.type = AOB_EVENT_CALL_SVC_CHANGED_IND;

    evt.con_lid = con_lid;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_call_action_result_ind_cb(uint8_t con_lid, void *param)
{
    AOB_EVENT_CALL_ACTION_RESULT_IND_T evt;
    evt.header.type = AOB_EVENT_CALL_ACTION_RESULT_IND;

    evt.con_lid = con_lid;
    evt.param = param;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_cis_established(AOB_UC_SRV_CIS_INFO_T *ascs_cis_established)
{
    AOB_EVENT_CIS_ESTABLISHED_IND_T evt;
    evt.header.type = AOB_EVENT_CIS_ESTABLISHED_IND;

    evt.ascs_cis_established = ascs_cis_established;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_cis_rejected(uint16_t con_hdl, uint8_t error)
{
    AOB_EVENT_CIS_REJECTED_IND_T evt;
    evt.header.type = AOB_EVENT_CIS_REJECTED_IND;

    evt.con_hdl = con_hdl;
    evt.error = error;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_cig_terminated(uint8_t cig_id, uint8_t group_lid, uint8_t stream_lid, uint8_t reason)
{
    AOB_EVENT_CIG_TERMINATED_IND_T evt;
    evt.header.type = AOB_EVENT_CIG_TERMINATED_IND;

    evt.cig_id = cig_id;
    evt.group_lid = group_lid;
    evt.stream_lid = stream_lid;
    evt.stream_lid = reason;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_ase_ntf_value_cb(uint8_t opcode, uint8_t nb_ases, uint8_t ase_lid, uint8_t rsp_code, uint8_t reason)
{
    AOB_EVENT_ASE_NTF_VALUE_IND_T evt;
    evt.header.type = AOB_EVENT_ASE_NTF_VALUE_IND;

    evt.opcode = opcode;
    evt.nb_ases = nb_ases;
    evt.ase_lid = ase_lid;
    evt.rsp_code = rsp_code;
    evt.reason = reason;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_ase_codec_cfg_cb(uint8_t ase_lid, const AOB_CODEC_ID_T *codec_id, uint8_t tgt_latency,
                                                            AOB_BAP_CFG_T *codec_cfg_req, AOB_BAP_QOS_REQ_T *ntf_qos_req)
{
    AOB_EVENT_ASE_CODEC_CFG_T evt;
    evt.header.type = AOB_EVENT_ASE_CODEC_CFG_VALUE_IND;

    evt.ase_lid = ase_lid;
    evt.codec_id = *codec_id;
    evt.tgt_latency = tgt_latency;
    evt.codec_cfg_req = *codec_cfg_req;
    // No need additional value
    evt.codec_cfg_req.add_cfg.len = 0;
    evt.ntf_qos_req = *ntf_qos_req;

    app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
}

static void app_custom_ui_ble_audio_connected_cb(uint8_t con_lid, uint8_t *remote_addr)
{
    if (bes_ble_audio_make_new_le_core_sm(con_lid, remote_addr))
       {
            TRACE(1,"%s report ble_audio_connected to ui",__func__);
            AOB_EVENT_MOB_STATE_T evt;
            evt.header.type = AOB_EVENT_MOB_CONNECTION_STATE;
            evt.evt_type = BLE_MOB_AUD_ATTR_BOND;
            evt.conidx = con_lid;
            evt.state.acl_state = AOB_ACL_ATTR_BOND;
            app_custom_ui_notify_le_audio_core_event((AOB_EVENT_HEADER_T *)&evt);
       }
}

BLE_AUD_CORE_EVT_CB_T ble_audio_core_event_callback =
{
    .ble_tws_sirk_refreshed                     = app_custom_ui_sirk_refreshed_callback,
    .ble_audio_adv_state_changed                = app_custom_ui_ble_audio_adv_state_changed_callback,
    .ble_tws_acl_state_changed                  = app_custom_ui_ble_tws_acl_state_changed_callback,
    .ble_mob_acl_state_changed                  = app_custom_ui_ble_mobile_acl_state_changed_callback,
    .ble_vol_changed                            = app_custom_ui_ble_vol_changed_callback,
    .ble_vocs_offset_changed_cb                 = app_custom_ui_ble_vocs_offset_changed_callback,
    .ble_vocs_bond_data_changed_cb              = app_custom_ui_ble_vocs_bond_data_changed_callback,
    .ble_media_track_change_cb                  = app_custom_ui_ble_media_track_change_cb,
    .ble_media_stream_status_change_cb          = app_custom_ui_ble_media_stream_status_change_cb,
    .ble_media_playback_status_change_cb        = app_custom_ui_ble_media_playback_status_change_cb,
    .ble_media_mic_state_cb                     = app_custom_ui_ble_media_mic_state_cb,
    .ble_media_iso_link_quality_cb              = app_custom_ui_ble_media_iso_link_quality_cb,
    .ble_media_pacs_cccd_written_cb             = app_custom_ui_ble_media_pacs_cccd_written_cb,
    .ble_call_state_change_cb                   = app_custom_ui_ble_call_state_change_cb,
    .ble_call_srv_signal_strength_value_ind_cb  = app_custom_ui_bble_call_srv_signal_strength_value_ind_cb,
    .ble_call_status_flags_ind_cb               = app_custom_ui_ble_call_status_flags_ind_cb,
    .ble_call_ccp_opt_supported_opcode_ind_cb   = app_custom_ui_ble_call_ccp_opt_supported_opcode_ind_cb,
    .ble_call_terminate_reason_ind_cb           = app_custom_ui_ble_call_terminate_reason_ind_cb,
    .ble_call_incoming_number_inf_ind_cb        = app_custom_ui_ble_call_incoming_number_inf_ind_cb,
    .ble_call_svc_changed_ind_cb                = app_custom_ui_ble_call_svc_changed_ind_cb,
    .ble_call_action_result_ind_cb              = app_custom_ui_ble_call_action_result_ind_cb,
    .ble_cis_established                        = app_custom_ui_ble_cis_established,
    .ble_cis_rejected                           = app_custom_ui_ble_cis_rejected,
    .ble_cig_terminated                         = app_custom_ui_ble_cig_terminated,
    .ble_ase_ntf_value_cb                       = app_custom_ui_ble_ase_ntf_value_cb,
    .ble_ase_codec_cfg_req_cb                   = app_custom_ui_ble_ase_codec_cfg_cb,
    .ble_audio_connected_cb                     = app_custom_ui_ble_audio_connected_cb,
};

#ifdef AOB_MOBILE_ENABLED
static void app_custom_ui_mobile_ble_acl_state_changed_callback(uint32_t evt_type, ble_event_handled_t *p)
{
    ASSERT(p != NULL, "%s null pointer %p", __func__, p);

    switch (evt_type)
    {
        case MOB_BLE_AUD_CONNECTING:
        case MOB_BLE_AUD_CANCEL_CONNECTION:
            break;
        case MOB_BLE_AUD_CONNECTION_CANCELED:
            break;
        case MOB_BLE_AUD_CONNECTION_FAILED:
            TRACE(1, "%s, reconnnect failed %d", __func__, p->connecting_failed_handled.err_code);
            DUMP8("%02x ", p->connecting_failed_handled.peer_bdaddr.addr, BT_ADDR_OUTPUT_PRINT_NUM);
            ble_mobile_connect_failed(true);
            break;
        case MOB_BLE_AUD_CONNECTION_TIMEOUT:
            TRACE(1, "%s, timeout", __func__);
            break;
        case MOB_BLE_AUD_BOND:
            break;
        case MOB_BLE_AUD_BOND_FAIL:
                app_bap_set_activity_type(GAF_BAP_ACT_TYPE_CIS_AUDIO);
                app_ble_start_scan(BLE_DEFAULT_SCAN_POLICY, 20, 50);
            break;
        case MOB_BLE_AUD_ENCRYPT:
            break;
        case MOB_BLE_AUD_CONNECTED:
            if (!ble_audio_mobile_conn_get_connecting_dev()) {
                ble_audio_mobile_conn_next_paired_dev(&p->connect_handled.peer_bdaddr);
            }
            break;
        case MOB_BLE_AUD_DISCONNECTING:
            break;
        case MOB_BLE_AUD_DISCONNECTED:
            {
                TRACE(1, "%s err code %d", __func__, p->disconnect_handled.errCode);
                if (ble_mobile_is_connect_failed() || p->disconnect_handled.errCode == CO_ERROR_TERMINATED_MIC_FAILURE) {
                    app_bap_set_activity_type(GAF_BAP_ACT_TYPE_CIS_AUDIO);
                    app_ble_start_scan(BLE_DEFAULT_SCAN_POLICY, 20, 50);
                    ble_mobile_connect_failed(false);
                } else if (p->disconnect_handled.errCode == CO_ERROR_CON_TERM_BY_LOCAL_HOST) {
                    TRACE(1, "Ble disconnected by local host");
                } else {
                    if (!ble_audio_mobile_conn_get_connecting_dev()) {
                        ble_audio_mobile_conn_next_paired_dev(&p->disconnect_handled.peer_bdaddr);
                    }
                }
            }
            break;
        default:
            TRACE(0, "ERROR, UNDEFINE MOBILE EVENT, PLEASE CHECK!!!");
            goto exit;
            break;
    }

exit:
    return;
}


BLE_AUD_MOB_CORE_EVT_CB_T ble_audio_mobile_core_event_callback =
{
    .mob_ble_acl_state_changed                  = app_custom_ui_mobile_ble_acl_state_changed_callback,
};
#endif
#endif

static const app_ibrt_conn_event_cb custom_ui_event_cb = {
        .ibrt_conn_a2dp_callback                   = app_custom_ui_a2dp_callback,
        .ibrt_conn_hfp_callback                    = app_custom_ui_hfp_callback,
        .ibrt_conn_avrcp_callback                  = app_custom_ui_avrcp_callback,
        .ibrt_conn_gsound_state_change             = app_custom_ui_gsound_state_change,
        .ibrt_conn_tws_on_pairing_changed          = app_custom_ui_tws_on_paring_state_changed,
        .ibrt_conn_tws_on_acl_state_changed        = app_custom_ui_tws_on_acl_state_changed,
        .ibrt_conn_on_host_paring_changed          = app_custom_ui_on_mobile_paring_state_changed,
        .ibrt_conn_on_mobile_acl_state_changed     = app_custom_ui_on_mobile_acl_state_changed,
        .ibrt_conn_on_tws_role_changed             = app_custom_ui_on_tws_role_changed,
        .ibrt_conn_access_mode_change              = app_custom_ui_access_mode_changed,
        .ibrt_conn_cancel_connect_state_changed    = app_custom_ui_cancel_connect_state_changed,
        .ibrt_conn_sco_state_changed               = app_custom_ui_on_mobile_sco_state_changed,
        .ibrt_conn_on_ibrt_state_changed           = app_custom_ui_on_ibrt_state_changed,
        .ibrt_conn_on_phone_connect_indication     = app_custom_ui_on_phone_connect_ind,
        .ibrt_conn_on_snoop_state_changed          = app_custom_ui_on_snoop_state_changed,
        .ibrt_conn_reset_tws_acl_data_packet_check = app_custom_ui_reset_tws_acl_data_packet_check,
    };

void app_custom_ui_ctx_init()
{
    TRACE(0, "custom api %s", BESLIB_INFO_STR);
    app_ibrt_conn_reg_evt_cb(&custom_ui_event_cb);
#if BLE_AUDIO_ENABLED
    ble_audio_core_register_event_cb(ble_audio_core_event_callback);
    app_sec_reg_smp_identify_info_cmp_callback(app_custom_ui_ble_smp_ia_exch_cmp);
#endif

#ifdef AOB_MOBILE_ENABLED
    ble_audio_mobile_core_register_event_cb(ble_audio_mobile_core_event_callback);
#endif

#if !defined(IBRT_UI_V2) && defined(__REPORT_EVENT_TO_CUSTOMIZED_UX__)
    app_ctm_adpt_init();
#endif
}

void app_custom_ui_report_enhanced_rs_evt(bool allowed, ibrt_role_e role)
{
#ifdef TWS_RS_WITHOUT_MOBILE
    ibrt_conn_tw_role_change_state_event twsRoleChangeEvt;
    twsRoleChangeEvt.header.type = IBRT_CONN_EVENT_TW_ROLE_CHANGE_STATE;
    memset((uint8_t*)&twsRoleChangeEvt.addr, 0x00, sizeof(bt_bdaddr_t));

    if (allowed) {
        twsRoleChangeEvt.role_state = IBRT_CONN_ROLE_SWAP_COMPLETE;
        twsRoleChangeEvt.role = role;
        twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_SUCCESS;
    }
    else {
        twsRoleChangeEvt.role_state = IBRT_CONN_ROLE_SWAP_DISALLOW;
        twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_ERROR_OP_NOT_ALLOWED;
    }
    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
#endif
}
