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

/*****************************header include********************************/
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "me_api.h"
#include "dip_api.h"
#include "hal_timer.h"
#include "app_ibrt_if.h"
#include "app_tws_ibrt_conn_api.h"
#include "app_ibrt_conn_evt.h"
#include "app_custom_thread.h"

#if !defined(IBRT_UI_V2) && defined(__REPORT_EVENT_TO_CUSTOMIZED_UX__)
/**********************private function declaration*************************/
static void app_custom_ux_thread(const void* arg);

/************************private variable defination************************/
static ibrt_core_status_changed_cb_t *usrhandler = NULL;
static osMailQId custom_ux_mbox = NULL;
static uint8_t custom_ux_mbox_cnt = 0;
static osThreadId custom_ux_thread_id;

osThreadDef(app_custom_ux_thread, osPriorityAboveNormal, 1, APP_CUSTOM_THREAD_SIZE, "custom ux thread");
osMailQDef(custom_ux_mbox, APP_CUSTOM_EVENT_MAX, CUSTOM_UX_EVENT_PACKET);

#if BLE_AUDIO_ENABLED
static CUSTOM_UX_AOB_EVENT_CB_HANDLER_T *aobUsrHandler = NULL;
#endif

/****************************function defination****************************/
void app_custom_ux_register_callback(ibrt_core_status_changed_cb_t *cb)
{
    usrhandler = cb;
    app_ibrt_conn_set_ui_reject_connect_req_cb(usrhandler->incoming_connect_request_response_cb);
#ifdef BT_DIP_SUPPORT
    app_dip_register_dip_info_queried_callback(usrhandler->dip_info_queried_cb);
#endif
    app_ibrt_if_register_BSIR_command_event_callback(usrhandler->bsir_event_cb);
}

#if BLE_AUDIO_ENABLED
void app_custom_ux_register_aob_callback(CUSTOM_UX_AOB_EVENT_CB_HANDLER_T *cb)
{
    aobUsrHandler = cb;
}
#endif

static int app_custom_ux_mailbox_get(CUSTOM_UX_EVENT_PACKET** msg_p)
{
    osEvent evt;
    evt = osMailGet(custom_ux_mbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (CUSTOM_UX_EVENT_PACKET *)evt.value.p;
        return 0;
    }
    return -1;
}

static int app_custom_ux_mailbox_free(CUSTOM_UX_EVENT_PACKET* msg_p)
{
    osStatus status = osOK;

    status = osMailFree(custom_ux_mbox, msg_p);

    if (osOK == status)
    {
        custom_ux_mbox_cnt--;
    }

    return (int)status;
}

static uint8_t app_custom_ux_bt_event_packet_parser(ibrt_conn_event_type type)
{
  switch (type)
  {
    case IBRT_CONN_EVENT_API_STATUS:
        return sizeof(ibrt_conn_api_status_event);
    case IBRT_CONN_EVENT_TW_PAIRING_STATE:
        return sizeof(ibrt_conn_tws_pairing_state_evt);
    case IBRT_CONN_EVENT_TW_CONNECTION_STATE:
      return sizeof(ibrt_conn_tws_conn_state_event);
    case IBRT_CONN_EVENT_TW_ROLE_CHANGE_STATE:
      return sizeof(ibrt_conn_tw_role_change_state_event);
    case IBRT_CONN_EVENT_HOST_PAIRING_STATE:
        return sizeof(ibrt_mobile_pairing_state_event);
    case IBRT_CONN_EVENT_MOBILE_CONNECTION_STATE:
      return sizeof(ibrt_mobile_conn_state_event);
    case IBRT_CONN_EVENT_IBRT_CONNECTION_STATE:
        return sizeof(ibrt_connection_state_event);
    case IBRT_CONN_EVENT_AVRCP_STATE:
        return sizeof(ibrt_conn_avrcp_state_change);
    case IBRT_CONN_EVENT_A2DP_STATE:
        return sizeof(ibrt_conn_a2dp_state_change);
    case IBRT_CONN_EVENT_HFP_STATE:
        return sizeof(ibrt_conn_hfp_state_change);
    case IBRT_CONN_EVENT_GLOBAL_STATE:
        return sizeof(ibrt_global_state_change_event);
    case IBRT_CONN_EVENT_SCO_CONNECTION_STATE:
        return sizeof(ibrt_sco_conn_state_event);
    case IBRT_CONN_EVENT_ACCESS_MODE:
        return sizeof(ibrt_access_mode_change_event);
    default:
         TRACE(1,"custom_ux: error ibrt event pkt size %d!!!!",type);
        break;
  }

  return 0;
}

static int app_custom_ux_evt_put_mailbox(void *evt, uint8_t length, bool is_aob)
{
    CUSTOM_UX_EVENT_PACKET *pkt = NULL;

    osStatus status = (osStatus)osErrorValue;

    if (length)
    {
        pkt = (CUSTOM_UX_EVENT_PACKET *)osMailCAlloc(custom_ux_mbox, 0);

        if (pkt == NULL)
        {
            TRACE(0, "custom_ux: osMailCAlloc fail");
            return (int)status;
        }

#if BLE_AUDIO_ENABLED
        pkt->aob_event = is_aob;
        if (!is_aob) {
            memcpy((uint8_t *)&pkt->u.bt_con_event_packet, (uint8_t *)evt, length);
        } else {
            memcpy((uint8_t *)&pkt->u.aob_event_packet, (uint8_t *)evt, length);
        }
#else
        pkt->aob_event = false;
        memcpy((uint8_t *)&pkt->u.bt_con_event_packet, (uint8_t *)evt, length);
#endif
        status = osMailPut(custom_ux_mbox, pkt);
        if (osOK == status)
        {
            custom_ux_mbox_cnt++;
        }
        else
        {
           TRACE(1, "custom_ux: osMailPut fail,status= 0x%x",status);
        }
    }

    return (int)status;
}

static void app_custom_ux_bt_event_handle(ibrt_conn_evt_header *event)
{
    if (NULL == usrhandler)
    {
        TRACE(1, "usrhandler is NULL!!");
        return;
    }

    ibrt_conn_event_packet *packet = (ibrt_conn_event_packet *)event;

    switch (event->type)
    {
        case IBRT_CONN_EVENT_GLOBAL_STATE:
            if (usrhandler->global_state_changed != NULL)
            {
                usrhandler->global_state_changed(&packet->global_state);
            }
            break;
        case IBRT_CONN_EVENT_ACCESS_MODE:
            if (usrhandler->access_mode_changed != NULL)
            {
                usrhandler->access_mode_changed(packet->access_mode_state.access_mode);
            }
            break;
        case IBRT_CONN_EVENT_MOBILE_CONNECTION_STATE:
        {
            ibrt_mobile_conn_state_event *mobile_acl_state = &packet->mobile_conn_state;
            uint8_t reason_code = mobile_acl_state->state.bluetooth_reason_code;
            if (usrhandler->mobile_acl_state_changed != NULL)
            {
                usrhandler->mobile_acl_state_changed(&mobile_acl_state->addr, mobile_acl_state, reason_code);
            }
            break;
        }
        case IBRT_CONN_EVENT_SCO_CONNECTION_STATE:
        {
            ibrt_sco_conn_state_event *mobile_sco_state = &packet->sco_conn_state;
            if (mobile_sco_state->state.sco_state == IBRT_CONN_SCO_DISCONNECTED ) {
                if (usrhandler->sco_disconnect_cb != NULL)
                {
                    usrhandler->sco_disconnect_cb(&mobile_sco_state->addr, mobile_sco_state->state.sco_reason_code);
                }
            }
            break;
        }
        case IBRT_CONN_EVENT_TW_CONNECTION_STATE:
        {
            ibrt_conn_tws_conn_state_event *tws_acl_state = &packet->tws_conn_state;
            if (usrhandler->tws_acl_state_changed != NULL)
            {
                usrhandler->tws_acl_state_changed(tws_acl_state, tws_acl_state->state.bluetooth_reason_code);
            }
            break;
        }
        case IBRT_CONN_EVENT_TW_ROLE_CHANGE_STATE:
        {
            ibrt_conn_tw_role_change_state_event *role_changed_state = &packet->role_change_state;
            if (usrhandler->tws_role_switch_status_ind != NULL)
            {
                usrhandler->tws_role_switch_status_ind(&role_changed_state->addr, role_changed_state->role_state, role_changed_state->role);
            }
            break;
        }
        case IBRT_CONN_EVENT_AVRCP_STATE:
        {
            ibrt_conn_avrcp_state_change *avrcp_state = &packet->avrcp_state;
            if (usrhandler->avrcp_state_changed != NULL)
            {
                usrhandler->avrcp_state_changed(&avrcp_state->addr, avrcp_state);
            }
            break;
        }
        case IBRT_CONN_EVENT_HFP_STATE:
        {
            ibrt_conn_hfp_state_change *hfp_state = &packet->hfp_state;
            if (usrhandler->hfp_state_changed != NULL)
            {
                usrhandler->hfp_state_changed(&hfp_state->addr, hfp_state);
            }
            break;
        }
        case IBRT_CONN_EVENT_A2DP_STATE:
        {
            ibrt_conn_a2dp_state_change *a2dp_state = &packet->a2dp_state;
            if (usrhandler->a2dp_state_changed != NULL)
            {
                usrhandler->a2dp_state_changed(&a2dp_state->addr, a2dp_state);
            }
            break;
        }
        default:
            break;
    }
}

#if BLE_AUDIO_ENABLED
static void app_custom_ux_aob_event_handle(AOB_EVENT_HEADER_T *event)
{
    ASSERT(event != NULL, "%s null pointer error %p", __func__, event);

    if (NULL == aobUsrHandler)
    {
        TRACE(2, "aobUsrHandler is NULL!!");
        return;
    }

    AOB_EVENT_PACKET *packet = (AOB_EVENT_PACKET *)event;

    TRACE(2, "%s type %d", __func__, event->type);

    switch (event->type)
    {
        case AOB_EVENT_TW_CONNECTION_STATE:
        {
            if (aobUsrHandler->aob_tws_acl_state_changed != NULL)
            {
                aobUsrHandler->aob_tws_acl_state_changed(packet->aob_mob_connection_state.evt_type, \
                                        packet->aob_tws_connection_state.conidx, \
                                        packet->aob_tws_connection_state.state, \
                                        packet->aob_tws_connection_state.peer_bdaddr);

            }

        }
            break;
        case AOB_EVENT_MOB_CONNECTION_STATE:
        {
            if (aobUsrHandler->aob_mob_acl_state_changed != NULL)
            {
                TRACE(2, "%s state %d addr type %d", __func__, packet->aob_mob_connection_state.state.acl_state, packet->aob_mob_connection_state.peer_bdaddr.addr_type);
                DUMP8("%02x ", &packet->aob_mob_connection_state.peer_bdaddr.addr, BTIF_BD_ADDR_SIZE);
                aobUsrHandler->aob_mob_acl_state_changed(packet->aob_mob_connection_state.evt_type, \
                                                        packet->aob_mob_connection_state.conidx, \
                                                        packet->aob_mob_connection_state.state, \
                                                        packet->aob_mob_connection_state.peer_bdaddr);
            }
        }
            break;
        case AOB_EVENT_VOL_CHANGED:
        {
            if (aobUsrHandler->aob_vol_changed_cb != NULL)
            {
                aobUsrHandler->aob_vol_changed_cb(packet->aob_vol_changed.con_lid, \
                                                packet->aob_vol_changed.volume, \
                                                packet->aob_vol_changed.mute);
            }
        }
            break;
        case AOB_EVENT_VOCS_OFFSET_CHANGED:
        {
            if (aobUsrHandler->aob_vocs_offset_changed_cb != NULL)
            {
                aobUsrHandler->aob_vocs_offset_changed_cb(packet->aob_vocs_offset_changed.offset, \
                                                        packet->aob_vocs_offset_changed.output_lid);
            }
        }
            break;
        case AOB_EVENT_VOCS_BOND_DATA_CHANGED:
        {
            if (aobUsrHandler->aob_vocs_bond_data_changed_cb != NULL)
            {
                aobUsrHandler->aob_vocs_bond_data_changed_cb(packet->aob_vocs_bond_data_changed.output_lid, \
                                                            packet->aob_vocs_bond_data_changed.cli_cfg_bf);
            }
        }
            break;
        case AOB_EVENT_MEDIA_TRACK_CHANGED:
        {
            if (aobUsrHandler->aob_media_track_change_cb != NULL)
            {
                aobUsrHandler->aob_media_track_change_cb(packet->aob_media_track_changed.con_lid);
            }
        }
            break;
        case AOB_EVENT_STREAM_STATUS_CHANGED:
        {
            if (aobUsrHandler->aob_media_stream_status_change_cb != NULL)
            {
                aobUsrHandler->aob_media_stream_status_change_cb(packet->aob_stream_state.con_lid, \
                                                                packet->aob_stream_state.ase_lid, \
                                                                packet->aob_stream_state.state);
            }
        }
            break;
        case AOB_EVENT_PLAYBACK_STATUS_CHANGED:
        {
            if (aobUsrHandler->aob_media_playback_status_change_cb != NULL)
            {
                aobUsrHandler->aob_media_playback_status_change_cb(packet->aob_playback_status_changed.con_lid, \
                                                                packet->aob_playback_status_changed.state);
            }
        }
            break;
        case AOB_EVENT_MIC_STATE:
        {
            if (aobUsrHandler->aob_media_mic_state_cb != NULL)
            {
                aobUsrHandler->aob_media_mic_state_cb(packet->aob_mic_state.mute);
            }
        }
            break;
        case AOB_EVENT_ISO_LINK_QUALITY_IND:
        {
            if (aobUsrHandler->aob_media_iso_link_quality_cb != NULL)
            {
                aobUsrHandler->aob_media_iso_link_quality_cb(packet->aob_iso_link_quality_ind.param);
            }
        }
            break;
        case AOB_EVENT_PACS_CCCD_WRITTEN_IND:
        {
            if (aobUsrHandler->aob_media_pacs_cccd_written_cb != NULL)
            {
                aobUsrHandler->aob_media_pacs_cccd_written_cb(packet->aob_pacs_cccd_written_ind.con_lid);
            }
        }
            break;
        case AOB_EVENT_CALL_STATE_CHANGE:
        {
            if (aobUsrHandler->aob_call_state_change_cb != NULL)
            {
                aobUsrHandler->aob_call_state_change_cb(packet->aob_call_state_change.con_lid, \
                                                        packet->aob_call_state_change.param);
            }
        }
            break;
        case AOB_EVENT_CALL_SRV_SIG_STRENGTH_VALUE_IND:
        {
            if (aobUsrHandler->aob_call_srv_signal_strength_value_ind_cb != NULL)
            {
                aobUsrHandler->aob_call_srv_signal_strength_value_ind_cb(packet->aob_call_srv_sig_strength_value_ind.con_lid, \
                                                                        packet->aob_call_srv_sig_strength_value_ind.value);
            }
        }
            break;
        case AOB_EVENT_CALL_STATUS_FLAGS_IND:
        {
            if (aobUsrHandler->aob_call_status_flags_ind_cb != NULL)
            {
                aobUsrHandler->aob_call_status_flags_ind_cb(packet->aob_call_status_flags_ind.con_lid, \
                                                            packet->aob_call_status_flags_ind.inband_ring, \
                                                            packet->aob_call_status_flags_ind.silent_mode);
            }
        }
            break;
        case AOB_EVENT_CALL_CCP_OPT_SUPPORTED_OPCODE_IND:
        {
            if (aobUsrHandler->aob_call_ccp_opt_supported_opcode_ind_cb != NULL)
            {
                aobUsrHandler->aob_call_ccp_opt_supported_opcode_ind_cb(packet->aob_call_ccp_opt_support_opcode_ind.con_lid, \
                                                                        packet->aob_call_ccp_opt_support_opcode_ind.local_hold_op_supported, \
                                                                        packet->aob_call_ccp_opt_support_opcode_ind.join_op_supported);
            }
        }
            break;
        case AOB_EVENT_CALL_TERMINATE_REASON_IND:
        {
            if (aobUsrHandler->aob_call_terminate_reason_ind_cb != NULL)
            {
                aobUsrHandler->aob_call_terminate_reason_ind_cb(packet->aob_call_terminate_reason_ind.con_lid, \
                                                                packet->aob_call_terminate_reason_ind.call_id, \
                                                                packet->aob_call_terminate_reason_ind.reason);
            }
        }
            break;
        case AOB_EVENT_CALL_INCOMING_NUM_INF_IND:
        {
            if (aobUsrHandler->aob_call_incoming_number_inf_ind_cb != NULL)
            {
                aobUsrHandler->aob_call_incoming_number_inf_ind_cb(packet->aob_call_incoming_num_inf_ind.con_lid, \
                                                                packet->aob_call_incoming_num_inf_ind.url_len, \
                                                                packet->aob_call_incoming_num_inf_ind.url);
            }
        }
            break;
        case AOB_EVENT_CALL_SVC_CHANGED_IND:
        {
            if (aobUsrHandler->aob_aob_call_svc_changed_ind_cb != NULL)
            {
                aobUsrHandler->aob_aob_call_svc_changed_ind_cb(packet->aob_call_svc_changed_ind.con_lid);
            }
        }
            break;
        case AOB_EVENT_CALL_ACTION_RESULT_IND:
        {
            if (aobUsrHandler->aob_call_action_result_ind_cb != NULL)
            {
                aobUsrHandler->aob_call_action_result_ind_cb(packet->aob_call_action_result_ind.con_lid, \
                                                            packet->aob_call_action_result_ind.param);
            }
        }
            break;
        case AOB_EVENT_CIS_ESTABLISHED_IND:
        {
            if (aobUsrHandler->aob_cis_established_ind_cb != NULL)
            {
                aobUsrHandler->aob_cis_established_ind_cb(packet->aob_cis_established_ind.ascs_cis_established);
            }
        }
            break;
        case AOB_EVENT_CIS_REJECTED_IND:
        {
            if (aobUsrHandler->aob_cis_rejected_ind_cb != NULL)
            {
                aobUsrHandler->aob_cis_rejected_ind_cb(packet->aob_cis_rejected_ind.con_hdl, packet->aob_cis_rejected_ind.error);
            }
        }
            break;
        case AOB_EVENT_CIG_TERMINATED_IND:
        {
            if (aobUsrHandler->aob_cig_terminated_ind_cb != NULL)
            {
                aobUsrHandler->aob_cig_terminated_ind_cb(packet->aob_cig_terminated_ind.cig_id,
                                                        packet->aob_cig_terminated_ind.group_lid,
                                                        packet->aob_cig_terminated_ind.stream_lid);
            }
        }
            break;
        case AOB_EVENT_ASE_NTF_VALUE_IND:
        {
            if (aobUsrHandler->aob_ase_ntf_value_ind_cb != NULL)
            {
                aobUsrHandler->aob_ase_ntf_value_ind_cb(packet->aob_ase_ntf_value_ind.opcode,
                                                        packet->aob_ase_ntf_value_ind.nb_ases,
                                                        packet->aob_ase_ntf_value_ind.ase_lid,
                                                        packet->aob_ase_ntf_value_ind.rsp_code,
                                                        packet->aob_ase_ntf_value_ind.reason);
            }
        }
            break;
        default:
            break;
    }
}

static uint8_t app_custom_ux_aob_event_parser(AOB_EVENT_E type)
{
    uint8_t length = 0;

    switch (type)
    {
        case AOB_EVENT_TW_CONNECTION_STATE:
            length = sizeof(AOB_EVENT_TWS_STATE_T);
            break;
        case AOB_EVENT_MOB_CONNECTION_STATE:
            length = sizeof(AOB_EVENT_MOB_STATE_T);
            break;
        case AOB_EVENT_VOL_CHANGED:
            length = sizeof(AOB_EVENT_VOL_CHANGED_T);
            break;
        case AOB_EVENT_VOCS_OFFSET_CHANGED:
            length = sizeof(AOB_EVENT_VOCS_OFFSET_CHANGED_T);
            break;
        case AOB_EVENT_VOCS_BOND_DATA_CHANGED:
            length = sizeof(AOB_EVENT_VOCS_BOND_DATA_CHANGED_T);
            break;
        case AOB_EVENT_MEDIA_TRACK_CHANGED:
            length = sizeof(AOB_EVENT_MEDIA_TRACK_CHANGED_T);
            break;
        case AOB_EVENT_STREAM_STATUS_CHANGED:
            length = sizeof(AOB_EVENT_STREAM_STATUS_CHANGED_T);
            break;
        case AOB_EVENT_PLAYBACK_STATUS_CHANGED:
            length = sizeof(AOB_EVENT_PLAYBACK_STATUS_CHANGED_T);
            break;
        case AOB_EVENT_MIC_STATE:
            length = sizeof(AOB_EVENT_MIC_STATE_T);
            break;
        case AOB_EVENT_CALL_STATE_CHANGE:
            length = sizeof(AOB_EVENT_CALL_STATE_CHANGE_T);
            break;
        case AOB_EVENT_CALL_SRV_SIG_STRENGTH_VALUE_IND:
            length = sizeof(AOB_EVENT_CALL_SRV_SIG_STRENGTH_VALUE_IND_T);
            break;
        case AOB_EVENT_CALL_STATUS_FLAGS_IND:
            length = sizeof(AOB_EVENT_CALL_STATUS_FLAGS_IND_T);
            break;
        case AOB_EVENT_CALL_CCP_OPT_SUPPORTED_OPCODE_IND:
            length = sizeof(AOB_EVENT_CALL_CCP_OPT_SUPPORTED_OPCODE_IND_T);
            break;
        case AOB_EVENT_CALL_TERMINATE_REASON_IND:
            length = sizeof(AOB_EVENT_CALL_TERMINATE_REASON_IND_T);
            break;
        case AOB_EVENT_CALL_INCOMING_NUM_INF_IND:
            length = sizeof(AOB_EVENT_CALL_INCOMING_NUM_INF_IND_T);
            break;
        case AOB_EVENT_CALL_SVC_CHANGED_IND:
            length = sizeof(AOB_EVENT_CALL_SVC_CHANGED_IND_T);
            break;
        case AOB_EVENT_CALL_ACTION_RESULT_IND:
            length = sizeof(AOB_EVENT_CALL_ACTION_RESULT_IND_T);
            break;
        case AOB_EVENT_ISO_LINK_QUALITY_IND:
            length = sizeof(AOB_EVENT_ISO_LINK_QUALITY_IND_T);
            break;
        case AOB_EVENT_PACS_CCCD_WRITTEN_IND:
            length = sizeof(AOB_EVENT_PACS_CCCD_WRITTEN_IND_T);
            break;
        case AOB_EVENT_CIS_ESTABLISHED_IND:
            length = sizeof(AOB_EVENT_CIS_ESTABLISHED_IND_T);
            break;
        case AOB_EVENT_CIS_REJECTED_IND:
            length = sizeof(AOB_EVENT_CIS_REJECTED_IND_T);
            break;
        case AOB_EVENT_CIG_TERMINATED_IND:
            length = sizeof(AOB_EVENT_CIG_TERMINATED_IND_T);
            break;
        case AOB_EVENT_ASE_NTF_VALUE_IND:
            length = sizeof(AOB_EVENT_ASE_NTF_VALUE_IND_T);
            break;
        default:
            TRACE(1, "custom_ux: undefined aob event %d!!!!",type);
            break;
    }

    return length;

}

#endif

static void app_custom_ux_thread(const void* arg)
{
    while (true)
    {
        CUSTOM_UX_EVENT_PACKET *pkt = NULL;
        if (app_custom_ux_mailbox_get(&pkt) == -1)
        {
            continue;
        }
#if BLE_AUDIO_ENABLED
        if (!pkt->aob_event) {
            app_custom_ux_bt_event_handle((ibrt_conn_evt_header *)&pkt->u.bt_con_event_packet);
        } else {
            app_custom_ux_aob_event_handle((AOB_EVENT_HEADER_T *)&pkt->u.aob_event_packet);
        }
#else
        app_custom_ux_bt_event_handle((ibrt_conn_evt_header *)&pkt->u.bt_con_event_packet);
#endif
        app_custom_ux_mailbox_free(pkt);
    }
}

void app_custom_ux_thread_init(void)
{
    custom_ux_mbox = osMailCreate(osMailQ(custom_ux_mbox), NULL);
    ASSERT(custom_ux_mbox, "custom_ux mailbox create failed");
    custom_ux_mbox_cnt = 0;

    custom_ux_thread_id = osThreadCreate(osThread(app_custom_ux_thread), NULL);
    ASSERT(custom_ux_thread_id,"custom_ux thread create failed");
}

#endif //__REPORT_EVENT_TO_CUSTOMIZED_UX__

#if BLE_AUDIO_ENABLED
void app_custom_ux_aob_event_notify(AOB_EVENT_HEADER_T *ev)
{
#if !defined(IBRT_UI_V2) && defined(__REPORT_EVENT_TO_CUSTOMIZED_UX__)
    uint8_t length = app_custom_ux_aob_event_parser(ev->type);
    app_custom_ux_evt_put_mailbox(ev, length, true);
#endif
}
#endif

void app_custom_ux_notify_bt_event(ibrt_conn_evt_header* ev)
{
#if !defined(IBRT_UI_V2) && defined(__REPORT_EVENT_TO_CUSTOMIZED_UX__)
    uint8_t length = app_custom_ux_bt_event_packet_parser(ev->type);
    app_custom_ux_evt_put_mailbox(ev, length, false);
#endif
}

