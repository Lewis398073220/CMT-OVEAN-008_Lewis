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
#include <string.h>
#include "apps.h"
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "app_ibrt_if.h"
#include "app_ibrt_customif_ui.h"
#include "me_api.h"
#include "app_vendor_cmd_evt.h"
#include "besaud_api.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "app_hfp.h"
#include "app_a2dp.h"
#include "btm_i.h"
#include "app_bt_func.h"
#include "app_bt.h"
#include "app_custom_adapter.h"
#include "app_custom_thread.h"
#include "app_custom_api.h"
#include "app_ibrt_customif_cmd.h"

#if BLE_AUDIO_ENABLED
#include "app_ble_include.h"
#include "ble_audio_core.h"
#include "aob_media_api.h"
#include "aob_bis_api.h"
#endif

#if !defined(IBRT_UI_V2) && defined(__REPORT_EVENT_TO_CUSTOMIZED_UX__)

void app_custom_ui_on_tws_role_changed(const bt_bdaddr_t *addr,ibrt_conn_role_change_state state,ibrt_role_e role);
extern int *a2dp_decoder_bth_get_cc_sbm_param(void);
/****************************function defination****************************/
static int _system_tx_pwr_get(SYSTEM_TX_RSSI_TYPE_T type, int* val)
{
    int ret = 0;
    uint16_t conHdl = INVALID_HANDLE;

    if (type == SYSTEM_TX_RSSI_TYPE_BUD)
    {
        ibrt_ctrl_t* p_ctx = app_tws_ibrt_get_bt_ctrl_ctx();
        conHdl = p_ctx->tws_conhandle;
    } else {
        conHdl = app_ibrt_conn_get_mobile_handle(NULL);
    }

    if (conHdl != INVALID_HANDLE)
    {
        int8_t tws_txpwr = bt_drv_reg_op_get_tx_pwr_dbm(conHdl);
        if (tws_txpwr == -127)
        {
            ret = -1;
        } else {
            *val = (int)tws_txpwr;
        }
    } else {
        ret = -2;
    }
    return ret;
}

static int _system_rssi_get(SYSTEM_TX_RSSI_TYPE_T type, int* val)
{
    int ret = 0;
    rx_agc_t agc_t = {0,0,};
    bool ret_rssi = false;
    uint16_t conHdl = INVALID_HANDLE;

    if (type == SYSTEM_TX_RSSI_TYPE_BUD)
    {
        ibrt_ctrl_t* p_ctx = app_tws_ibrt_get_bt_ctrl_ctx();
        conHdl = p_ctx->tws_conhandle;
    } else {
        conHdl = app_ibrt_conn_get_mobile_handle(NULL);
    }

    if (conHdl != INVALID_HANDLE)
    {
        ret_rssi =  bt_drv_reg_op_read_rssi_in_dbm(conHdl, &agc_t);
        if (ret_rssi == false)
        {
            ret = -1;
        } else {
            *val = (int)agc_t.rssi;
        }
     } else {
        ret = -2;
     }

    return ret;
}

bool app_ctm_adpt_tx_pwr_rssi_read(SYSTEM_TX_RSSI_TYPE_T type, int *t_val, int *r_val)
{
    int retRssi = 0;
    int retPower = 0;

    ASSERT((t_val != NULL) && (r_val != NULL), "%s:parameter error t_val: %p r_val: %p",
        __func__, t_val, r_val);

    retPower = _system_tx_pwr_get(type, t_val);
    retRssi = _system_rssi_get(type, r_val);

    TRACE(1,"%s get(%d) result: %d/%d t_val: %d r_val: %d", __func__, type,
        retPower, retRssi, *t_val, *r_val);

    return ((!retPower) && (!retRssi));
}

static void app_ctm_adpt_try_to_start_ibrt(const bt_bdaddr_t *addr)
{
    TRACE(0, "%s", __func__);

    if (btif_besaud_is_connected())
    {
        if (addr != NULL)
        {
            if (app_ibrt_conn_connect_ibrt(addr) != IBRT_STATUS_SUCCESS) {
                DUMP8("%02x ", addr, BT_ADDR_OUTPUT_PRINT_NUM);
                TRACE(0,"CTM_ADPT: start ibrt fail_1");
            }
        } else {
            bt_bdaddr_t mobile_addr_list[BT_DEVICE_NUM];
            uint8_t connected_mobile_num = app_ibrt_conn_get_connected_mobile_list(mobile_addr_list);
            for (uint8_t i = 0; i < connected_mobile_num; i++)
            {
                if (app_ibrt_conn_connect_ibrt(&mobile_addr_list[i]) != IBRT_STATUS_SUCCESS)
                {
                    DUMP8("%02x ", &mobile_addr_list[i], BT_ADDR_OUTPUT_PRINT_NUM);
                    TRACE(0,"CTM_ADPT: start ibrt fail_2");
                }
            }
        }
    }
}

uint32_t app_ctm_adpt_get_profiles_sync_time(const uint8_t *uuid_data_ptr, uint8_t uuid_len)
{
    // While spp connect and no basic profile established, this func will be called.
    // Add custom implementation here, if you need to make peer buds connects to the spp as soon as possible,
    // just return a none-zero value. For example, "return 1" will be the fastest.
    // return t : wait t ms to start profile exchange
    // return 0 : will not change the process of "profile exchange"
    TRACE(1, "spp_uuid len=%d bytes, uuid:", uuid_len);
    DUMP8("0x%x ", uuid_data_ptr, uuid_len);
    return 80;
}

static bool app_ctm_adpt_mobile_connection_handler(void)
{
    // cancel tws page, accept mobile incoming connection request
    ibrt_ctrl_t *p_ibrt_ctx = app_tws_ibrt_get_bt_ctrl_ctx();
    btif_hci_cancel_create_connection((bt_bdaddr_t *)&p_ibrt_ctx->peer_addr);

    return true; // dont reject mobile connection req
}

static void app_ctm_adpt_tws_share_info(void)
{
    TRACE(0, "%s", __func__);

    if (app_ibrt_conn_is_nv_master()) {
        TRACE(0, "%s +++", __func__);
        ADPT_TWS_SHARE_INFO_T info;
        info.initiator_cnt = app_ibrt_conn_get_local_connected_mobile_count();
        info.responsor_cnt = 0;
        int *sbm_param = a2dp_decoder_bth_get_cc_sbm_param();
        if (bt_a2dp_is_run())
        {
            memcpy(info.custom_data, sbm_param, sizeof(int)*3);
            TRACE(0, "MASTER SEND PARAM1 %d PARAM2 %d PARAM3 %d", info.custom_data[0], info.custom_data[1], info.custom_data[2]);
        }
        else
        {
            memset(info.custom_data, 0, sizeof(int)*3);
            TRACE(0, "MASTER NOT PLAY");
        }
        tws_ctrl_send_cmd(APP_TWS_CMD_SHARE_LINK_INFO, (uint8_t *)&info, sizeof(ADPT_TWS_SHARE_INFO_T));
    }
}

void app_ctm_adpt_tws_share_info_cmd_cb(uint16_t rsp_seq, uint8_t *buf, uint16_t len)
{
    ADPT_TWS_SHARE_INFO_T *info = (ADPT_TWS_SHARE_INFO_T *)buf;
    info->responsor_cnt = app_ibrt_conn_get_local_connected_mobile_count();

    if (info->responsor_cnt && info->initiator_cnt) {
        // TODO: Disconnect all mobile links
        // app_ibrt_conn_disconnect_all_mobile_links();
        info->responsor_cnt = 0;
    }

    if (info->responsor_cnt) {
        // set master
        app_custom_ui_on_tws_role_changed(NULL, IBRT_CONN_ROLE_UPDATE, IBRT_MASTER);
        app_ctm_adpt_try_to_start_ibrt(NULL);
    } else {
        // set slave
        app_custom_ui_on_tws_role_changed(NULL, IBRT_CONN_ROLE_UPDATE, IBRT_SLAVE);
    }

    if (bt_a2dp_is_run())
    {
        int *sbm_param = a2dp_decoder_bth_get_cc_sbm_param();
        memcpy(info->custom_data, sbm_param, sizeof(int)*3);
        TRACE(0, "SLAVE SEND PARAM1 %d PARAM2 %d PARAM3 %d", info->custom_data[0], info->custom_data[1], info->custom_data[2]);
    }
    else
    {
        int *sbm_param = a2dp_decoder_bth_get_cc_sbm_param();
        memcpy(sbm_param, info->custom_data, sizeof(int)*3);
        TRACE(0, "SLAVE RECEIVE PARAM1 %d PARAM2 %d PARAM3 %d", sbm_param[0], sbm_param[1], sbm_param[2]);
    }

    tws_ctrl_send_rsp(APP_TWS_CMD_SHARE_LINK_INFO, rsp_seq, (uint8_t *)info, len);
}

void app_ctm_adpt_tws_share_info_rsp_cb(uint16_t rsp_seq, uint8_t *buf, uint16_t len)
{
    ADPT_TWS_SHARE_INFO_T *info = (ADPT_TWS_SHARE_INFO_T *)buf;
    if (info->responsor_cnt) {
        // set slave
        app_custom_ui_on_tws_role_changed(NULL, IBRT_CONN_ROLE_UPDATE, IBRT_SLAVE);
    }
    else {
        // set master
        app_custom_ui_on_tws_role_changed(NULL, IBRT_CONN_ROLE_UPDATE, IBRT_MASTER);
        app_ctm_adpt_try_to_start_ibrt(NULL);
    }


    if (bt_a2dp_is_run())
    {
        TRACE(0, "MASTER ALREADY SEND PARAM1 %d PARAM2 %d PARAM3 %d", info->custom_data[0], info->custom_data[1], info->custom_data[2]);
    }
    else
    {
        int *sbm_param = a2dp_decoder_bth_get_cc_sbm_param();
        memcpy(sbm_param, info->custom_data, sizeof(int)*3);
        TRACE(0, "MASTER RECEIVE PARAM1 %d PARAM2 %d PARAM3 %d", info->custom_data[0], info->custom_data[1], info->custom_data[2]);
    }
}

static void _ctm_adpt_convert_ibrtevent_to_aclevent(const bt_bdaddr_t *addr,
    ibrt_conn_ibrt_state state,ibrt_role_e role,uint8_t reason_code)
{
    ibrt_mobile_conn_state_event evt;
    evt.header.type     = IBRT_CONN_EVENT_MOBILE_CONNECTION_STATE;
    evt.state.bluetooth_reason_code = 0;
    memcpy((void*)&evt.addr, addr, sizeof(bt_bdaddr_t));
    evt.device_id = app_bt_get_device_id_byaddr(&evt.addr);
    evt.current_role = app_tws_get_ibrt_role(&evt.addr);
    switch (state)
    {
    case IBRT_CONN_IBRT_DISCONNECTED:
        if (!app_ibrt_conn_mobile_link_connected(addr) && reason_code == 0)
        {
            TRACE(0, "CTM_ADPT: convert to ibrt evt->acl event");
            evt.state.acl_state = IBRT_CONN_ACL_DISCONNECTED;
            evt.state.bluetooth_reason_code = 0x16;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
        }
        break;
    case IBRT_CONN_IBRT_ACL_CONNECTED:
        if (!app_ibrt_conn_mobile_link_connected(addr))
        {
            TRACE(0, "CTM_ADPT: convert to ibrt evt->acl event");
            evt.state.acl_state = IBRT_CONN_ACL_CONNECTED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
        }
        break;
    case IBRT_CONN_IBRT_CONNECTED:
        {
            btif_remote_device_t *rdev = btif_me_get_remote_device_by_bdaddr((bt_bdaddr_t*)addr);
            if (rdev)
            {
                bool isSimplePairingCompleted = btif_me_is_connection_simple_pairing_completed(rdev);
                if (isSimplePairingCompleted)
                {
                    TRACE(0, "CTM_ADPT: distribute simple pairing completed to slave.");
                    evt.state.acl_state = IBRT_CONN_ACL_SIMPLE_PIARING_COMPLETE;
                    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
                }
            }
        }
        break;
    default:
        break;
    }
}

void app_ctm_adpt_init(void)
{
    ibrt_core_param_t core_config;

    // freeman mode config, default should be false
    core_config.freeman_enable = false;

    // set ibrt retry times
    core_config.reconnect_ibrt_max_times = 10;

    // support max remote link count
    core_config.support_max_remote_link = 2;

    // do not stop ibrt even if no profile
    core_config.no_profile_stop_ibrt = false;

    // controller basband monitor
    core_config.lowlayer_monitor_enable = true;

    // controller basband monitor report format
    core_config.llmonitor_report_format = REP_FORMAT_PACKET;

    // controller basband monitor report count
    core_config.llmonitor_report_count = 1000;

    core_config.allow_sniff_in_sco = false;

#ifdef IBRT_UI_MASTER_ON_TWS_DISCONNECTED
    core_config.is_changed_to_ui_master_on_tws_disconnected = true;
#else
    core_config.is_changed_to_ui_master_on_tws_disconnected = false;
#endif

    core_config.profile_concurrency_supported = true;

#ifdef BT_ALWAYS_IN_DISCOVERABLE_MODE
    btif_me_configure_keeping_both_scan(true);
#endif

    app_tws_ibrt_core_reconfig(&core_config, NULL);

    app_custom_ux_thread_init();

    app_tws_ibrt_register_mobile_connection_callback(app_ctm_adpt_mobile_connection_handler);
}

/*****************************************************************************
 Description: Strong function used to override the original function
 which implemented by iBRT UI
 It will be executed when BES UI is not applicable
*****************************************************************************/
void app_custom_ui_on_mobile_acl_state_changed(const bt_bdaddr_t *addr, ibrt_conn_acl_state state, uint8_t reason_code)
{
    ibrt_mobile_conn_state_event evt;
    evt.header.type     = IBRT_CONN_EVENT_MOBILE_CONNECTION_STATE;
    evt.state.bluetooth_reason_code = 0;
    memcpy((void*)&evt.addr, addr, sizeof(bt_bdaddr_t));
    evt.state.acl_state = state;
    evt.device_id = app_bt_get_device_id_byaddr(&evt.addr);
    evt.current_role = app_tws_get_ibrt_role(&evt.addr);

    if ((state == IBRT_CONN_ACL_DISCONNECTED) ||
        (state == IBRT_CONN_ACL_CONNECTING_FAILURE) ||
        (state == IBRT_CONN_ACL_AUTH_COMPLETE))
    {
        evt.state.bluetooth_reason_code = reason_code;
    }
    if (state == IBRT_CONN_ACL_CONNECTED) {
        app_ctm_adpt_try_to_start_ibrt(addr);
    }

    TRACE(3, "CTM_ADPT(d%x): mobile ACL state changed %d reason_code %x", evt.device_id, state, reason_code);

#ifdef GFPS_ENABLED
    if ((state == IBRT_CONN_ACL_DISCONNECTED) && gfps_is_last_response_pending())
    {
        gfps_enter_connectable_mode_req_handler(gfps_get_last_response());
    }
#endif

    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
}

void app_custom_ui_tws_on_acl_state_changed(ibrt_conn_acl_state state, uint8_t reason_code)
{
    ibrt_conn_tws_conn_state_event  POSSIBLY_UNUSED evt;

    evt.header.type = IBRT_CONN_EVENT_TW_CONNECTION_STATE;
    evt.state.acl_state  = state;

    if ((state == IBRT_CONN_ACL_DISCONNECTED) ||
        (state == IBRT_CONN_ACL_CONNECTING_CANCELED) ||
        (state == IBRT_CONN_ACL_CONNECTING_FAILURE))
    {
        evt.state.bluetooth_reason_code = reason_code;
        app_custom_ui_on_tws_role_changed(NULL, IBRT_CONN_ROLE_UPDATE, IBRT_MASTER);
    }
    else if (state == IBRT_CONN_ACL_AUTH_COMPLETE)
    {
        app_ctm_adpt_tws_share_info();
    }

#if BLE_AUDIO_ENABLED
    if (state == IBRT_CONN_ACL_PROFILES_CONNECTED)
    {
        bis_tws_trans_handler_t* aob_bis_tws_evt_handler = bis_get_tws_evt_handler();
        if (aob_bis_tws_evt_handler->bis_tws_rejoin_cb)
        {
            aob_bis_tws_evt_handler->bis_tws_rejoin_cb();
        }
    }
#endif

    evt.current_role = app_ibrt_conn_get_ui_role();

    TRACE(3,"CTM_ADPT: TWS ACL state changed =0x%x, reason code:%d role %d", state, reason_code, evt.current_role);
    app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
}

void app_custom_ui_on_tws_role_changed(const bt_bdaddr_t *addr,ibrt_conn_role_change_state state,ibrt_role_e role)
{
    ibrt_conn_tw_role_change_state_event twsRoleChangeEvt;
    twsRoleChangeEvt.header.type = IBRT_CONN_EVENT_TW_ROLE_CHANGE_STATE;
    twsRoleChangeEvt.role_state = state;

    TRACE(2,"CTM_ADPT: role switch changed = %d,role = %d", state, role);
    //Address will be set to NULL if ibrt core disallow the role swap
    //memset the address to zero for the function app_ui_handle_role_change_evt can handle this case
    if (NULL != addr)
    {
        memcpy((uint8_t*)&twsRoleChangeEvt.addr, addr, sizeof(bt_bdaddr_t));
        DUMP8("%02x ",addr->address, BT_ADDR_OUTPUT_PRINT_NUM);
    } else {
        memset((uint8_t*)&twsRoleChangeEvt.addr, 0x00,sizeof(bt_bdaddr_t));
    }

    switch(state)
    {
        case IBRT_CONN_ROLE_SWAP_INITIATED:
            twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_SUCCESS;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
            break;
        case IBRT_CONN_ROLE_SWAP_PASSIVE:
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
            app_ibrt_conn_set_ui_role((TWS_UI_ROLE_E)role);
            }
            break;
        case IBRT_CONN_ROLE_CHANGE_COMPLETE: //update role
            twsRoleChangeEvt.role = role;
            twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_SUCCESS;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
            app_ibrt_conn_set_ui_role((TWS_UI_ROLE_E)role);
            break;
        case IBRT_CONN_ROLE_UPDATE:
            twsRoleChangeEvt.role = role;
            twsRoleChangeEvt.role_change_status = IBRT_CONN_STATUS_SUCCESS;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&twsRoleChangeEvt);
            app_ibrt_conn_set_ui_role((TWS_UI_ROLE_E)role);
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

void app_custom_ui_hfp_callback(uint8_t device_id, void *param1,void *param2,void* param3)
{
    static bool already_slc_open = false;
    static bool need_report_sco_open = false;

    struct hfp_context *hfp_ctx = (hfp_context *)param2;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    TRACE(1, "CTM_ADPT: HFP callback event=%d", hfp_ctx->event);

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
            already_slc_open = false;
            need_report_sco_open = false;
            evt.hfp_state = IBRT_CONN_HFP_SLC_DISCONNECTED;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_SERVICE_CONNECTED:
        case BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED:
            if (curr_device->mock_hfp_after_force_disc)
            {
                break;
            }
            already_slc_open = true;
            evt.hfp_state = IBRT_CONN_HFP_SLC_OPEN;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            if (need_report_sco_open) {
                TRACE(0, "[CTM_ADPT]slave: HFP report sco open after slc open");
                need_report_sco_open = false;
                evt.hfp_state = IBRT_CONN_HFP_SCO_OPEN;
                app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            }
            break;

        case BTIF_HF_EVENT_AUDIO_CONNECTED:
            if (!already_slc_open && !app_ibrt_conn_mobile_link_connected(&evt.addr)) {
                TRACE(0,"[CTM_ADPT]slave: HFP report sco open later");
                need_report_sco_open = true;
                break;
            }
            evt.hfp_state = IBRT_CONN_HFP_SCO_OPEN;
            app_custom_ui_notify_ibrt_core_event((ibrt_conn_evt_header *)&evt);
            break;

        case BTIF_HF_EVENT_AUDIO_DISCONNECTED:
            need_report_sco_open = false;
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

    return;
}

void app_custom_ui_on_ibrt_state_changed(const bt_bdaddr_t *addr,
    ibrt_conn_ibrt_state state,ibrt_role_e role,uint8_t reason_code)
{
    ibrt_connection_state_event ibrt_state_event;

    ibrt_state_event.header.type = IBRT_CONN_EVENT_IBRT_CONNECTION_STATE;
    memcpy((uint8_t*)&ibrt_state_event.addr, addr, sizeof(bt_bdaddr_t));
    ibrt_state_event.state.ibrt_state = state;
    ibrt_state_event.device_id = app_bt_get_device_id_byaddr(&ibrt_state_event.addr);
    ibrt_state_event.current_role = app_tws_get_ibrt_role(&ibrt_state_event.addr);

    if(state == IBRT_CONN_IBRT_DISCONNECTED)
    {
        ibrt_state_event.state.ibrt_reason_code = reason_code;
    }
#ifdef TWS_RS_BY_BTC
    else if (state == IBRT_CONN_IBRT_CONNECTED)
    {

        app_ibrt_conn_btc_role_switch(addr);
    }
#endif

    TRACE(4,"(d%x) notify ibrt state changed %d role %x reason %x", ibrt_state_event.device_id, state, role, reason_code);
    _ctm_adpt_convert_ibrtevent_to_aclevent(addr, state, role, reason_code);
}

void app_custom_ui_on_snoop_state_changed(const bt_bdaddr_t *addr,uint8_t snoop_connected)
{
    //snoop connected,set initial UI role for the first snoop link connected
    //More than one snoop connected cases,role will update by tws role switch event
    if ((snoop_connected) && (app_ibrt_conn_get_snoop_connected_link_num() == 1))
    {
        if (app_ibrt_conn_mobile_link_connected(addr))
        {
            app_custom_ui_on_tws_role_changed(addr, IBRT_CONN_ROLE_UPDATE, IBRT_MASTER);
        }
        else
        {
            app_custom_ui_on_tws_role_changed(addr, IBRT_CONN_ROLE_UPDATE, IBRT_SLAVE);
        }
    }
}

#endif

