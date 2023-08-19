 /**
 * @file aob_mgr_gaf_evt.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2021-07-08
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
 */

/*****************************header include********************************/
#if BLE_AUDIO_ENABLED
#include "ble_app_dbg.h"
#include "app_gaf_custom_api.h"
#include "app_bap_bc_assist_msg.h"
#include "bap_bc_sink_msg.h"
#include "ble_audio_earphone_info.h"

#include "acc_mc.h"
#include "acc_tb.h"
#include "app_acc.h"
#include "app_ble_include.h"
#include "app.h"
#include "aob_mgr_gaf_evt.h"
#include "app_tws_ctrl_thread.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "app_bt_func.h"
#include "aob_media_api.h"
#include "aob_call_api.h"
#include "aob_bis_api.h"
#include "aob_volume_api.h"
#include "aob_conn_api.h"
#include "ble_audio_core.h"
#include "ble_audio_earphone_info.h"
#include "gaf_media_pid.h"
#include "gaf_media_stream.h"
#include "ble_audio_tws_cmd_handler.h"
#include "acc_tbc_msg.h"
#include "csis.h"
#include "aob_csip_api.h"
#include "bap_uc_srv_msg.h"
#include "aob_dts_api.h"
#include "gatt.h"
#include "aob_gatt_cache.h"
#include "aob_service_sync.h"
#include "besaud_api.h"
#include "aob_cis_api.h"
#include "aob_pacs_api.h"

#ifdef AOB_UC_TEST
#include "app_ble_cmd_handler.h"
#include "cmsis_os.h"

#define TEST_READ_ISO_QUALITY_INTERVAL 6*1000*10

void aob_uc_test_get_iso_quality_report(void const *param)
{
    uint8_t ase_lid = aob_media_get_cur_streaming_ase_lid(0, AOP_MGR_DIRECTION_SINK);
    TRACE(2, "%s %04X", __func__, GET_CURRENT_MS());
    aob_media_read_iso_link_quality(ase_lid);
}

osTimerDef(AOB_UC_TEST_BUDS_ISO_QUALITY_REPORT, aob_uc_test_get_iso_quality_report);
static osTimerId aob_uc_test_buds_iso_quality_report_timer_id = NULL;
#endif

/************************private macro defination***************************/

/************************private type defination****************************/

/************************extern function declaration************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
static call_event_handler_t aob_mgr_call_evt_handler = {NULL,};
static media_event_handler_t aob_mgr_media_evt_handler = {NULL,};
static vol_event_handler_t aob_mgr_vol_evt_handler = {NULL,};
static sink_event_handler_t aob_mgr_sink_evt_handler = {NULL,};
static scan_event_handler_t aob_mgr_scan_evt_handler = {NULL,};
static deleg_event_handler_t aob_mgr_deleg_evt_handler ={NULL,};
static csip_event_handler_t aob_mgr_csip_evt_handler ={NULL,};
static dts_coc_event_handler_t aob_mgr_dts_coc_evt_handler ={NULL,};
static cis_conn_evt_handler_t aob_mgr_cis_conn_evt_handler ={NULL,};
static pacs_event_handler_t aob_mgr_pacs_event_handler = {NULL,};

/****************************function defination****************************/
static void aob_mgr_ble_audio_connected_report(uint8_t con_lid)
{
    LOG_I("%s with con_lid: %x",__func__,con_lid);

    uint8_t *remote_addr = NULL;
    BLE_AUD_CORE_EVT_CB_T* p_cbs = ble_audio_get_evt_cb();
    app_ble_get_peer_solved_addr(con_lid, &remote_addr);

        if (NULL != p_cbs->ble_audio_connected_cb)
    {
        p_cbs->ble_audio_connected_cb(con_lid, remote_addr);
    }
}

static void aob_mgr_gaf_ascs_cis_established_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_uc_srv_cis_state_ind_t *ascs_cis_established = (app_gaf_uc_srv_cis_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CIS_ESTABLISHED_IND, __func__);

    /// Update iso handle for gaf media
    if (GAF_INVALID_LID != ascs_cis_established->ase_lid_sink)
    {
        gaf_audio_update_stream_iso_hdl(ascs_cis_established->ase_lid_sink);
    }

    if (GAF_INVALID_LID != ascs_cis_established->ase_lid_src)
    {
       gaf_audio_update_stream_iso_hdl(ascs_cis_established->ase_lid_src);
    }

    if (aob_mgr_cis_conn_evt_handler.cis_established_cb)
    {
        aob_mgr_cis_conn_evt_handler.cis_established_cb(ascs_cis_established);
    }

#ifdef AOB_UC_TEST
    if (NULL == aob_uc_test_buds_iso_quality_report_timer_id)
    {
       aob_uc_test_buds_iso_quality_report_timer_id = osTimerCreate(osTimer(AOB_UC_TEST_BUDS_ISO_QUALITY_REPORT), osTimerPeriodic, NULL);
    }
    osTimerStart(aob_uc_test_buds_iso_quality_report_timer_id, TEST_READ_ISO_QUALITY_INTERVAL);
#endif
}

static void aob_mgr_gaf_ascs_cis_disconnected_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_uc_srv_cis_state_ind_t *ascs_cis_disconnected = (app_gaf_uc_srv_cis_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CIS_DISCONNETED_IND, __func__);

    if (aob_mgr_cis_conn_evt_handler.cis_disconnected_cb)
    {
        aob_mgr_cis_conn_evt_handler.cis_disconnected_cb(ascs_cis_disconnected);
    }
#ifdef AOB_UC_TEST
    if (NULL != aob_uc_test_buds_iso_quality_report_timer_id)
    {
        osTimerStop(aob_uc_test_buds_iso_quality_report_timer_id);
    }
#endif
}

static void aob_mgr_gaf_ascs_cis_stream_started_ind(void *event)
{
    app_gaf_ascs_cis_stream_started_t *ascs_cis_stream_started = (app_gaf_ascs_cis_stream_started_t *)event;

    LOG_I("[%d]%s ase_lid:%d direction:%d", ascs_cis_stream_started->con_lid, __func__,
        ascs_cis_stream_started->ase_lid, ascs_cis_stream_started->direction);

    #ifndef AOB_MOBILE_ENABLED
    ble_audio_connection_interval_mgr(ascs_cis_stream_started->con_lid, LEA_CI_MODE_ISO_DATA_ACT);
    #endif

    gaf_audio_stream_update_and_start_handler(ascs_cis_stream_started->ase_lid);
}

static void aob_mgr_gaf_ascs_cis_stream_stopped_ind(void *event)
{
    app_gaf_ascs_cis_stream_stopped_t *ascs_cis_stream_stopped = (app_gaf_ascs_cis_stream_stopped_t *)event;
    LOG_I("%s ase lid %d direction %d", __func__,
        ascs_cis_stream_stopped->ase_lid, ascs_cis_stream_stopped->direction);

#ifndef AOB_MOBILE_ENABLED
    ble_audio_connection_interval_mgr(ascs_cis_stream_stopped->con_lid, LEA_CI_MODE_ISO_DATA_STOP);
#endif

    gaf_audio_stream_update_and_stop_handler(ascs_cis_stream_stopped->ase_lid);
}

static void aob_mgr_gaf_ascs_configure_codec_ri(void *event)
{
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CONFIGURE_CODEC_RI, __func__);
    app_gaf_uc_srv_configure_codec_req_ind_t *p_cfg_codec_ri = (app_gaf_uc_srv_configure_codec_req_ind_t *)event;
    if (aob_mgr_media_evt_handler.ase_codec_cfg_req_handler_cb)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_cfg_codec_ri->ase_lid);
        aob_mgr_media_evt_handler.ase_codec_cfg_req_handler_cb(p_cfg_codec_ri->con_lid, p_ase_info,
                                                                p_cfg_codec_ri->tgt_latency, &p_cfg_codec_ri->codec_id,
                                                                &p_cfg_codec_ri->cfg);
    }
}

static void aob_mgr_gaf_ascs_iso_link_quality_ind(void *event)
{
    if (NULL != aob_mgr_media_evt_handler.media_iso_link_quality_cb)
    {
        aob_mgr_media_evt_handler.media_iso_link_quality_cb(event);
    }
#ifdef AOB_UC_TEST
    app_bap_uc_srv_quality_rpt_evt_t *event_p = (app_bap_uc_srv_quality_rpt_evt_t *)event;
    LOG_I("tx_unack = %04x", event_p->tx_unacked_packets);
    LOG_I("tx_flush = %04x", event_p->tx_flushed_packets);
    LOG_I("tx_last_sub = %04x", event_p->tx_last_subevent_packets);
    LOG_I("tx_rtn = %04x", event_p->retx_packets);
    LOG_I("crc_err = %04x", event_p->crc_error_packets);
    LOG_I("rx_unrx = %04x", event_p->rx_unrx_packets);
    LOG_I("rx_dulp = %04x", event_p->duplicate_packets);
#endif
}

static void aob_mgr_gaf_ascs_enable_ri(void *event)
{
    app_gaf_uc_srv_enable_req_ind_t *p_enable_ri = (app_gaf_uc_srv_enable_req_ind_t *)event;
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_enable_ri->ase_lid);
    app_ble_audio_event_t evt = APP_BLE_AUDIO_MAX_IND;

    if (NULL == p_ase_info) {
        LOG_W("WARNING: %s ase lid %d context %d", __func__,
            p_enable_ri->ase_lid, p_enable_ri->metadata.param.context_bf);
        goto exit;
    }

    LOG_I("[%d]%s ase_lid: %d context 0x%x", p_ase_info->con_lid, __func__, p_enable_ri->ase_lid, p_enable_ri->metadata.param.context_bf);

    /// Record enable req bring what kind of context during streaming
    p_ase_info->init_context_bf = p_enable_ri->metadata.param.context_bf;

    if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)
    {
        evt = APP_BLE_AUDIO_CALL_ENABLE_REQ;
    }
    else if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
    {
        evt = APP_BLE_AUDIO_MUSIC_ENABLE_REQ;
    }
    else
    {
        evt = APP_BLE_AUDIO_FLEXIBLE_ENABLE_REQ;
    }

    app_ble_audio_sink_streaming_handle_event(p_ase_info->con_lid, p_enable_ri->ase_lid, APP_GAF_DIRECTION_MAX, evt);

    if (aob_mgr_media_evt_handler.ase_enable_req_handler_cb)
    {
        aob_mgr_media_evt_handler.ase_enable_req_handler_cb(p_ase_info->con_lid, p_enable_ri->ase_lid,
                                                            &p_enable_ri->metadata);
    }

exit:
    return;
}

static void aob_mgr_gaf_ascs_update_metadata_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_uc_srv_update_metadata_req_ind_t *p_metadata_ri = (app_gaf_uc_srv_update_metadata_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_UPDATE_METADATA_RI, __func__);

    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_metadata_ri->ase_lid);
    if (aob_mgr_media_evt_handler.ase_update_metadata_req_handler_cb)
    {
        aob_mgr_media_evt_handler.ase_update_metadata_req_handler_cb(p_ase_info->con_lid, p_metadata_ri->ase_lid,
                                                              &p_metadata_ri->metadata);
    }
}

static void aob_mgr_gaf_ascs_release_ri(void *event)
{
    app_gaf_uc_srv_release_req_ind_t *p_release_ri = (app_gaf_uc_srv_release_req_ind_t *)event;
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_release_ri->ase_lid);
    app_ble_audio_event_t evt = APP_BLE_AUDIO_MAX_IND;

    if (NULL == p_ase_info) {
        LOG_W("WARNING: %s ase lid %d", __func__, p_release_ri->ase_lid);
        goto exit;
    }

    LOG_I("[%d]%s ase_lid: %d", p_ase_info->con_lid, __func__, p_ase_info->ase_lid);

    if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL)
    {
        evt = APP_BLE_AUDIO_CALL_RELEASE_REQ;
    }
    else if (p_ase_info->init_context_bf & AOB_AUDIO_CONTEXT_TYPE_MEDIA)
    {
        evt = APP_BLE_AUDIO_MUSIC_RELEASE_REQ;
    }
    else
    {
        evt = APP_BLE_AUDIO_FLEXIBLE_RELEASE_REQ;
    }

    app_ble_audio_sink_streaming_handle_event(p_ase_info->con_lid, p_ase_info->ase_lid,APP_GAF_DIRECTION_MAX, evt);

    if (aob_mgr_media_evt_handler.ase_release_req_handler_cb)
    {
        aob_mgr_media_evt_handler.ase_release_req_handler_cb(p_ase_info->con_lid, p_release_ri->ase_lid,
                                                              p_ase_info->p_metadata);
    }

exit:
    return;
}

static void aob_mgr_gaf_ascs_stream_updated_ind(void *event)
{
    app_gaf_cis_stream_state_updated_ind_t *p_stream_updated = (app_gaf_cis_stream_state_updated_ind_t *)event;
    if (aob_mgr_media_evt_handler.media_stream_status_change_cb)
    {
        aob_mgr_media_evt_handler.media_stream_status_change_cb(\
            p_stream_updated->con_lid, p_stream_updated->ase_instance_idx , (AOB_MGR_STREAM_STATE_E)p_stream_updated->currentState);
    }

    if(APP_GAF_CIS_STREAM_QOS_CONFIGURED == p_stream_updated->currentState)
    {
        app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(p_stream_updated->ase_instance_idx);
        if (NULL == p_ase_info)
        {
            LOG_W("WARNING: %s ase lid %d", __func__,p_stream_updated->ase_instance_idx);
            return;
        }

        LOG_I("[%d]%s ase_lid: %d", p_ase_info->con_lid, __func__, p_ase_info->ase_lid);
        app_ble_audio_sink_streaming_handle_event(\
            p_ase_info->con_lid, p_ase_info->ase_lid, APP_GAF_DIRECTION_MAX,APP_BLE_AUDIO_CALL_QOS_CONFIG_IND);
        app_ble_audio_sink_streaming_handle_event(\
            p_ase_info->con_lid, p_ase_info->ase_lid, APP_GAF_DIRECTION_MAX,APP_BLE_AUDIO_MUSIC_QOS_CONFIG_IND);

#if defined (BLE_AUDIO_DOLPHIN_COMPATIBLE_SUPPORT)
#if APP_GAF_ACC_ENABLE
        if (!app_bap_uc_srv_acc_already_discovered(p_ase_info->con_lid))
        {
            uint8_t *remote_addr = NULL;
            app_ble_get_peer_solved_addr(p_ase_info->con_lid, &remote_addr);
            if (aob_gattc_cache_load(p_ase_info->con_lid, remote_addr, GATT_SVC_GENERIC_MEDIA_CONTROL)
                && aob_gattc_cache_load(p_ase_info->con_lid, remote_addr, GATT_SVC_GENERIC_TELEPHONE_BEARER))
            {
                LOG_I("[%d]%s earbuds load cache success", p_ase_info->con_lid, __func__);
                app_acc_mcc_set_cfg(p_ase_info->con_lid, 0, ACC_MC_CHAR_TYPE_PLAYER_NAME, 1);
                app_acc_tbc_set_cfg(p_ase_info->con_lid, 0, ACC_TB_CHAR_TYPE_PROV_NAME, 1);
            }
            else
            {
                if (app_ble_audio_support_sync_service()
                    && ble_audio_is_ux_slave()
                    && btif_besaud_is_connected())
                {
                    LOG_I("[%d]%s earbuds slave wait sync srv info", p_ase_info->con_lid,__func__);
                }
                else
                {
                    LOG_I("[%d]%s earbuds start acc upon first qos configuered", p_ase_info->con_lid,__func__);
                    #ifdef AOB_MOBILE_ENABLED
                    ble_audio_discovery_modify_interval(p_ase_info->con_lid, DISCOVER_START, SERVICE_ACC_MCC);
                    ble_audio_discovery_modify_interval(p_ase_info->con_lid, DISCOVER_START, SERVICE_ACC_TBC);
                    #else
                    ble_audio_connection_interval_mgr(p_ase_info->con_lid, LEA_CI_MODE_SVC_DISC_START);
                    #endif
                    app_acc_start(p_ase_info->con_lid, false);
                }
            }
            app_bap_uc_srv_set_acc_discovery_status(p_ase_info->con_lid,true);
        }
#endif
#endif
    }
}

static void aob_mgr_gaf_ascs_bond_data_ind(void *event)
{
    app_gaf_bap_uc_srv_bond_data_ind_t *ascs_bond_data = (app_gaf_bap_uc_srv_bond_data_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_BOND_DATA_IND, __func__);
    uint8_t *remote_addr = NULL;
    app_ble_get_peer_solved_addr(ascs_bond_data->con_lid, &remote_addr);
    // app_ble_audio_send_service_data((bt_bdaddr_t*)remote_addr,GATT_SVC_AUDIO_STREAM_CTRL,(void *)ascs_bond_data);

    BLE_AUD_CORE_EVT_CB_T* p_cbs = ble_audio_get_evt_cb();
    /// Just compare to corresponding bit
    if ((NULL != p_cbs->ble_ase_cp_cccd_written_cb) &&
        (ascs_bond_data->cli_cfg_bf == 0x01) &&
        /// No ase cccd written but ase cp
        (ascs_bond_data->ase_cli_cfg_bf == 0x00))
    {
        p_cbs->ble_ase_cp_cccd_written_cb(ascs_bond_data->con_lid);
    }
    else
    {
        aob_gattc_cache_save(remote_addr, GATT_SVC_AUDIO_STREAM_CTRL, (void *)ascs_bond_data);
    }
    aob_mgr_ble_audio_connected_report(ascs_bond_data->con_lid);
}

static void aob_mgr_gaf_ascs_cis_rejected_ind(void *event)
{
    app_gaf_bap_uc_srv_cis_rejected_ind_t *ascs_cis_rejected = (app_gaf_bap_uc_srv_cis_rejected_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CIS_REJECTED_IND, __func__);

    if (aob_mgr_cis_conn_evt_handler.cis_rejected_cb)
    {
        aob_mgr_cis_conn_evt_handler.cis_rejected_cb(ascs_cis_rejected->con_hdl, ascs_cis_rejected->error);
    }
}

static void aob_mgr_gaf_ascs_cig_terminated_ind(void *event)
{
    app_gaf_bap_uc_srv_cig_terminated_ind_t *ascs_cig_terminated = (app_gaf_bap_uc_srv_cig_terminated_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_CIG_TERMINATED_IND, __func__);

    if (aob_mgr_cis_conn_evt_handler.cis_terminated_cb)
    {
        aob_mgr_cis_conn_evt_handler.cis_terminated_cb(ascs_cig_terminated->cig_id, ascs_cig_terminated->group_lid,
                                                       ascs_cig_terminated->stream_lid, ascs_cig_terminated->reason);
    }
}

static void aob_mgr_gaf_ascs_ase_ntf_value_ind(void *event)
{
    app_gaf_bap_uc_srv_ase_ntf_value_ind_t *ascs_ase_ntf_value = (app_gaf_bap_uc_srv_ase_ntf_value_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_ASCS_ASE_NTF_VALUE_IND, __func__);

    if (aob_mgr_cis_conn_evt_handler.ase_ntf_value_cb)
    {
        aob_mgr_cis_conn_evt_handler.ase_ntf_value_cb(ascs_ase_ntf_value->opcode, ascs_ase_ntf_value->nb_ases,
                ascs_ase_ntf_value->ase_lid, ascs_ase_ntf_value->rsp_code, ascs_ase_ntf_value->reason);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_ascs_evt_cb_list[] = {
    //BAP ASCS Callback Functions
    {APP_GAF_ASCS_CIS_ESTABLISHED_IND,      aob_mgr_gaf_ascs_cis_established_ind},
    {APP_GAF_ASCS_CIS_DISCONNETED_IND,      aob_mgr_gaf_ascs_cis_disconnected_ind},
    {APP_GAF_ASCS_CIS_STREAM_STARTED_IND,   aob_mgr_gaf_ascs_cis_stream_started_ind},
    {APP_GAF_ASCS_CIS_STREAM_STOPPED_IND,   aob_mgr_gaf_ascs_cis_stream_stopped_ind},
    {APP_GAF_ASCS_CONFIGURE_CODEC_RI,       aob_mgr_gaf_ascs_configure_codec_ri},
    {APP_GAF_ASCS_ENABLE_RI,                aob_mgr_gaf_ascs_enable_ri},
    {APP_GAF_ASCS_UPDATE_METADATA_RI,       aob_mgr_gaf_ascs_update_metadata_ri},
    {APP_GAF_ASCS_RELEASE_RI,               aob_mgr_gaf_ascs_release_ri},
    {APP_GAF_ASCS_CLI_STREAM_STATE_UPDATED, aob_mgr_gaf_ascs_stream_updated_ind},
    {APP_GAF_ASCS_ISO_LINK_QUALITY_EVT,     aob_mgr_gaf_ascs_iso_link_quality_ind},
    {APP_GAF_ASCS_BOND_DATA_IND,            aob_mgr_gaf_ascs_bond_data_ind},
    {APP_GAF_ASCS_CIS_REJECTED_IND,         aob_mgr_gaf_ascs_cis_rejected_ind},
    {APP_GAF_ASCS_CIG_TERMINATED_IND,       aob_mgr_gaf_ascs_cig_terminated_ind},
    {APP_GAF_ASCS_ASE_NTF_VALUE_IND,        aob_mgr_gaf_ascs_ase_ntf_value_ind},
};

static void aob_mgr_gaf_pacs_location_set_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_capa_srv_location_ind_t *location_set = (app_gaf_capa_srv_location_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_PACS_LOCATION_SET_IND, __func__);
}

static void aob_mgr_gaf_pacs_bond_data_ind(void *event)
{
    app_gaf_capa_srv_bond_data_ind_t *pacs_bond_data = (app_gaf_capa_srv_bond_data_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_PACS_BOND_DATA_IND, __func__);
    uint8_t *remote_addr = NULL;
    app_ble_get_peer_solved_addr(pacs_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr, GATT_SVC_PUBLISHED_AUDIO_CAPA, (void *)pacs_bond_data);

}

static void aob_mgr_gaf_pacs_cccd_written_ind(void *event)
{
    app_gaf_capa_srv_cccd_written_ind_t *p_cccd_written_ind = (app_gaf_capa_srv_cccd_written_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_PACS_CCCD_WRITTEN_IND, __func__);

    if (NULL != aob_mgr_pacs_event_handler.pacs_cccd_written_cb)
    {
        aob_mgr_pacs_event_handler.pacs_cccd_written_cb(p_cccd_written_ind->con_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_pacs_evt_cb_list[] = {
    //BAP PACS Callback Function
    {APP_GAF_PACS_LOCATION_SET_IND,         aob_mgr_gaf_pacs_location_set_ind},
    {APP_GAF_PACS_BOND_DATA_IND,            aob_mgr_gaf_pacs_bond_data_ind},
    {APP_GAF_PACS_CCCD_WRITTEN_IND,         aob_mgr_gaf_pacs_cccd_written_ind},
};

static void aob_mgr_gaf_scan_timeout_ind(void *event)
{
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_TIMEOUT_IND, __func__);
}

static void aob_mgr_gaf_scan_report_ind(void *event)
{
#if (CFG_BAP_BC)
    POSSIBLY_UNUSED app_gaf_bc_scan_adv_report_t *scan_adv_report = (app_gaf_bc_scan_adv_report_t *)event;

    LOG_D("app_gaf event handle: %04x, %s method %d", APP_GAF_SCAN_REPORT_IND, __func__, scan_adv_report->scan_trigger_method);

    /// Get bcast_id that earphone want to recv and sync with assoiated pa
    uint8_t *bcast_id_p = ble_audio_earphone_info_get_bis_bcast_id();

    DUMP8("%02x ", bcast_id_p, BAP_BC_BROADCAST_ID_LEN);
    DUMP8("%02x ", &scan_adv_report->bcast_id.id[0], BAP_BC_BROADCAST_ID_LEN);
    uint8_t *src_addr = ble_audio_get_bis_src_ble_addr();

    if ((memcmp(bcast_id_p, &scan_adv_report->bcast_id.id[0], BAP_BC_BROADCAST_ID_LEN) == 0)||
        (memcmp(src_addr, scan_adv_report->adv_report.adv_id.addr, GAP_BD_ADDR_LEN) == 0))
    {
#ifdef AOB_MOBILE_ENABLED
        if (APP_BAP_BC_ASSIST_TRIGGER == scan_adv_report->scan_trigger_method)
        {
            app_bap_bc_assist_add_src_t p_src_add;
            memcpy(&p_src_add.adv_report, &scan_adv_report->adv_report, sizeof(app_gaf_extend_adv_report_t));
            p_src_add.pa_sync = APP_BAP_BC_PA_SYNC_SYNC_PAST;
            memcpy(&p_src_add.bcast_id.id,&scan_adv_report->bcast_id.id, BAP_BC_BROADCAST_ID_LEN);
            p_src_add.pa_intv_frames = APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL;
            p_src_add.nb_subgroups = APP_BAP_DFT_BC_SRC_NB_SUBGRPS;
            p_src_add.bis_sync_bf = 0xFFFFFFFF; //0xFFFFFFFF No preference
            app_bap_bc_assist_source_add(&p_src_add);

        }
        else if (APP_BAP_BC_SINK_TRIGGER == scan_adv_report->scan_trigger_method)
#else
        if (APP_BAP_BC_SINK_TRIGGER == scan_adv_report->scan_trigger_method)
#endif
        {
            app_bap_bc_scan_pa_sync(&scan_adv_report->adv_report.adv_id);
        }
    }
#endif
}

static void aob_mgr_gaf_scan_pa_report_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_pa_report_ind_t *p_pa_report = (app_gaf_bc_scan_pa_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_REPORT_IND, __func__);
}

static void aob_mgr_gaf_scan_pa_established_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_pa_established_ind_t *p_pa_established = (app_gaf_bc_scan_pa_established_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_ESTABLISHED_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_pa_established_cb)
    {
        aob_mgr_scan_evt_handler.scan_pa_established_cb(p_pa_established->pa_lid, p_pa_established->adv_addr.addr,
                        p_pa_established->adv_addr.addr_type, p_pa_established->adv_addr.adv_sid, p_pa_established->serv_data);
    }
}

static void aob_mgr_gaf_scan_pa_terminated_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_pa_terminated_ind_t *p_pa_terminated = (app_gaf_bc_scan_pa_terminated_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_TERMINATED_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_pa_terminated_cb)
    {
        aob_mgr_scan_evt_handler.scan_pa_terminated_cb(p_pa_terminated->pa_lid, p_pa_terminated->reason);
    }
}

static void aob_mgr_gaf_scan_group_report_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_group_report_ind_t *p_group_report = (app_gaf_bc_scan_group_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_GROUP_REPORT_IND, __func__);
}

static void aob_mgr_gaf_scan_subgroup_report_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_subgroup_report_ind_t *p_subgroup_report = (app_gaf_bc_scan_subgroup_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_SUBGROUP_REPORT_IND, __func__);
}

static void aob_mgr_gaf_scan_stream_report_ind(void *event)
{
    app_gaf_bc_scan_stream_report_ind_t *p_stream_report = (app_gaf_bc_scan_stream_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_STREAM_REPORT_IND, __func__);

    AOB_BIS_STREAM_INFO_T *p_stream_info = ble_audio_earphone_info_get_bis_stream_info();
    p_stream_info->frame_octet = p_stream_report->cfg.param.frame_octet;
    p_stream_info->sampling_freq = p_stream_report->cfg.param.sampling_freq;
    if (aob_mgr_scan_evt_handler.scan_stream_report_cb)
    {
        aob_mgr_scan_evt_handler.scan_stream_report_cb(p_stream_report);
    }
}

static void aob_mgr_gaf_scan_biginfo_report_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_big_info_report_ind_t *p_big_info_report = (app_gaf_bc_scan_big_info_report_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_BIGINFO_REPORT_IND, __func__);
    LOG_D("Big id %d info:", p_big_info_report->pa_lid);
    LOG_D("SDU interval %d us ISO interval %d us", p_big_info_report->report.sdu_interval,
        (uint32_t)(p_big_info_report->report.iso_interval*1.25*1000));
    LOG_D("Includes %d bis, NSE %d BN %d", p_big_info_report->report.num_bis,
        p_big_info_report->report.nse, p_big_info_report->report.bn);
    LOG_D("PTO %d, IRC %d PHY %d", p_big_info_report->report.pto,
        p_big_info_report->report.irc, p_big_info_report->report.phy);
    LOG_D("framing %d, encrypted %d", p_big_info_report->report.framing,
        p_big_info_report->report.encrypted);

    if (aob_mgr_scan_evt_handler.scan_big_info_report_cb)
    {
        aob_mgr_scan_evt_handler.scan_big_info_report_cb(p_big_info_report);
    }
}

static void aob_mgr_gaf_scan_pa_sync_req_ind(void *event)
{
    app_gaf_bc_scan_pa_synchronize_req_ind_t *p_scan_pa_sync_req = (app_gaf_bc_scan_pa_synchronize_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_SYNC_REQ_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_pa_sync_req_cb)
    {
        aob_mgr_scan_evt_handler.scan_pa_sync_req_cb(p_scan_pa_sync_req->pa_lid);
    }
}

static void aob_mgr_gaf_scan_pa_terminated_req_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_scan_pa_terminate_req_ind_t *p_scan_pa_terminated_req = (app_gaf_bc_scan_pa_terminate_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SCAN_PA_TERMINATED_REQ_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_pa_terminate_req_cb)
    {
        aob_mgr_scan_evt_handler.scan_pa_terminate_req_cb(p_scan_pa_terminated_req->pa_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_bis_scan_evt_cb_list[] = {
    //BAP BIS Scan Callback Functions
    {APP_GAF_SCAN_TIMEOUT_IND,              aob_mgr_gaf_scan_timeout_ind},
    {APP_GAF_SCAN_REPORT_IND,               aob_mgr_gaf_scan_report_ind},
    {APP_GAF_SCAN_PA_REPORT_IND,            aob_mgr_gaf_scan_pa_report_ind},
    {APP_GAF_SCAN_PA_ESTABLISHED_IND,       aob_mgr_gaf_scan_pa_established_ind},
    {APP_GAF_SCAN_PA_TERMINATED_IND,        aob_mgr_gaf_scan_pa_terminated_ind},
    {APP_GAF_SCAN_GROUP_REPORT_IND,         aob_mgr_gaf_scan_group_report_ind},
    {APP_GAF_SCAN_SUBGROUP_REPORT_IND,      aob_mgr_gaf_scan_subgroup_report_ind},
    {APP_GAF_SCAN_STREAM_REPORT_IND,        aob_mgr_gaf_scan_stream_report_ind},
    {APP_GAF_SCAN_BIGINFO_REPORT_IND,       aob_mgr_gaf_scan_biginfo_report_ind},
    {APP_GAF_SCAN_PA_SYNC_REQ_IND,          aob_mgr_gaf_scan_pa_sync_req_ind},
    {APP_GAF_SCAN_PA_TERMINATED_REQ_IND,    aob_mgr_gaf_scan_pa_terminated_req_ind},
    {APP_GAF_SCAN_STOPPED_IND,              NULL},
};

static void aob_mgr_gaf_sink_bis_status_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_sink_status_ind_t *p_sink_ind = (app_gaf_bc_sink_status_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s, nb of bis %d", APP_GAF_SINK_BIS_STATUS_IND, __func__, p_sink_ind->nb_bis);

    AOB_BIS_GROUP_INFO_T *bis_gropu_info_p = ble_audio_earphone_info_get_bis_group_info();

    if (BAP_BC_SINK_ESTABLISHED == p_sink_ind->state)
    {
        uint8_t stream_lid = 0;
        uint32_t stream_pos_bf = p_sink_ind->stream_pos_bf;
        /// Set stream pos bf that local synced
        ble_audio_earphone_info_set_bis_stream_sink_pos_bf(stream_pos_bf);
        uint8_t stream_pos = co_ctz(stream_pos_bf) + 1;
        uint16_t *bis_hdl = &p_sink_ind->conhdl[0];

        memset(&bis_gropu_info_p->bis_hdl[0], 0xFF, IAP_NB_STREAMS*sizeof(uint16_t));

        while (stream_pos_bf != 0)
        {
            stream_lid = ble_audio_earphone_info_bis_stream_pos_2_stream_lid(stream_pos);
            if (stream_lid >= IAP_NB_STREAMS)
            {
                break;
            }

            LOG_I("app_gaf bc stream_lid = %d, conhdl = 0x%04x", stream_lid, *bis_hdl);
            bis_gropu_info_p->bis_hdl[stream_lid] = *bis_hdl;

            stream_pos_bf &= ~CO_BIT(stream_pos - 1);
            stream_pos = co_ctz(stream_pos_bf) + 1;
            // Move to next bis handle
            bis_hdl++;
        }
    }

    if (aob_mgr_sink_evt_handler.bis_sink_state_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_state_cb(p_sink_ind->grp_lid, p_sink_ind->state, p_sink_ind->stream_pos_bf);
    }
}

static void aob_mgr_gaf_sink_bis_stream_started_ind(void *event)
{
    uint8_t grp_lid = *(uint8_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SINK_BIS_STREAM_STARTED_IND, __func__);

    /// BES BIS sink stream start callback
    if (aob_mgr_sink_evt_handler.bis_sink_stream_started_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_stream_started_cb(grp_lid);
    }
}

static void aob_mgr_gaf_sink_bis_stream_stopped_ind(void *event)
{
    uint8_t grp_lid = *(uint8_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SINK_BIS_STREAM_STOPPED_IND, __func__);

    /// BES BIS sink stream stop callback
    if (aob_mgr_sink_evt_handler.bis_sink_stream_stoped_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_stream_stoped_cb(grp_lid);
    }
}

static void aob_mgr_gaf_sink_bis_sink_enabled_ind(void *event)
{
    uint8_t grp_lid = *(uint8_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SINK_BIS_SINK_ENABLED_IND, __func__);

    /// BES BIS sink enabled callback
    if (aob_mgr_sink_evt_handler.bis_sink_enabled_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_enabled_cb(grp_lid);
    }
}

static void aob_mgr_gaf_sink_bis_sink_disabled_ind(void *event)
{
    uint8_t grp_lid = *(uint8_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_SINK_BIS_SINK_DISABLED_IND, __func__);

    /// BES BIS sink disabled callback
    if (aob_mgr_sink_evt_handler.bis_sink_disabled_cb)
    {
        aob_mgr_sink_evt_handler.bis_sink_disabled_cb(grp_lid);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_bis_sink_evt_cb_list[] = {
    //BAP BIS Sink Callback Functions
    {APP_GAF_SINK_BIS_STATUS_IND,           aob_mgr_gaf_sink_bis_status_ind},
    {APP_GAF_SINK_BIS_SINK_ENABLED_IND,     aob_mgr_gaf_sink_bis_sink_enabled_ind},
    {APP_GAF_SINK_BIS_SINK_DISABLED_IND,    aob_mgr_gaf_sink_bis_sink_disabled_ind},
    {APP_GAF_SINK_BIS_STREAM_STARTED_IND,   aob_mgr_gaf_sink_bis_stream_started_ind},
    {APP_GAF_SINK_BIS_STREAM_STOPPED_IND,   aob_mgr_gaf_sink_bis_stream_stopped_ind},
};

static void aob_mgr_gaf_deleg_solicite_started_ind(void *event)
{
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOLICITE_STARTED_IND, __func__);

    /// BES BIS deleg solicite started callback
    if (aob_mgr_deleg_evt_handler.deleg_solicite_started_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_solicite_started_cb();
    }
}

static void aob_mgr_gaf_deleg_solicite_stopped_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_solicite_stopped_ind_t *reason = (app_gaf_bc_deleg_solicite_stopped_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOLICITE_STOPPED_IND, __func__);

    /// BES BIS deleg solicite stop callback
    if (aob_mgr_deleg_evt_handler.deleg_solicite_stoped_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_solicite_stoped_cb();
    }
}

static void aob_mgr_gaf_deleg_remote_scan_started_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_bond_remote_scan_ind_t *deleg_remote_scan_started = (app_gaf_bc_deleg_bond_remote_scan_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_REMOTE_SCAN_STARTED_IND, __func__);
}

static void aob_mgr_gaf_deleg_remote_scan_stopped_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_bond_remote_scan_ind_t *deleg_remote_scan_stopped = (app_gaf_bc_deleg_bond_remote_scan_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_REMOTE_SCAN_STOPPED_IND, __func__);
}

static void aob_mgr_gaf_deleg_source_add_ri(void *event)
{
    app_gaf_bc_deleg_source_add_req_ind_t *p_source_add_ri = (app_gaf_bc_deleg_source_add_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOURCE_ADD_RI, __func__);

    /// BES BIS sink deleg add source
    if (aob_mgr_deleg_evt_handler.deleg_source_add_ri_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_source_add_ri_cb(p_source_add_ri->src_lid,\
                    &p_source_add_ri->bcast_id.id[0], p_source_add_ri->con_lid);
    }
}

static void aob_mgr_gaf_deleg_source_remove_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_source_remove_req_ind_t *p_source_remove_ri = (app_gaf_bc_deleg_source_remove_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOURCE_REMOVE_RI, __func__);

    /// BES BIS sink deleg remove source
    if (aob_mgr_deleg_evt_handler.deleg_source_remove_ri_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_source_remove_ri_cb(p_source_remove_ri->src_lid, p_source_remove_ri->con_lid);
    }
}

static void aob_mgr_gaf_deleg_source_update_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_bc_deleg_source_update_req_ind_t *p_source_update_ri = (app_gaf_bc_deleg_source_update_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_DELEG_SOURCE_UPDATE_RI, __func__);

    /// BES BIS deleg update source
    if (aob_mgr_deleg_evt_handler.deleg_source_update_ri_cb)
    {
        aob_mgr_deleg_evt_handler.deleg_source_update_ri_cb(p_source_update_ri->src_lid, p_source_update_ri->con_lid, &p_source_update_ri->metadata);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_deleg_evt_cb_list[] = {
    //BAP BIS Delegator Callback Functions
    {APP_GAF_DELEG_SOLICITE_STARTED_IND,    aob_mgr_gaf_deleg_solicite_started_ind},
    {APP_GAF_DELEG_SOLICITE_STOPPED_IND,    aob_mgr_gaf_deleg_solicite_stopped_ind},
    {APP_GAF_DELEG_REMOTE_SCAN_STARTED_IND, aob_mgr_gaf_deleg_remote_scan_started_ind},
    {APP_GAF_DELEG_REMOTE_SCAN_STOPPED_IND, aob_mgr_gaf_deleg_remote_scan_stopped_ind},
    {APP_GAF_DELEG_SOURCE_ADD_RI,           aob_mgr_gaf_deleg_source_add_ri},
    {APP_GAF_DELEG_SOURCE_REMOVE_RI,        aob_mgr_gaf_deleg_source_remove_ri},
    {APP_GAF_DELEG_SOURCE_UPDATE_RI,        aob_mgr_gaf_deleg_source_update_ri},
};

static void aob_mgr_gaf_mcc_svc_discoveryed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_mcc_cmp_evt_t *mcc_svc_discoveryed = (app_gaf_acc_mcc_cmp_evt_t *)event;
    LOG_I("app_gaf event handle: %04x, %s", APP_GAF_MCC_SVC_DISCOVERYED_IND, __func__);
    #ifdef AOB_MOBILE_ENABLED
    ble_audio_discovery_modify_interval(mcc_svc_discoveryed->con_lid, DISCOVER_COMPLEPE, SERVICE_ACC_MCC);
    #endif
}

static void aob_mgr_gaf_mcc_track_changed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_mcc_track_changed_ind_t *p_track_changed_ind = (app_gaf_acc_mcc_track_changed_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_MCC_TRACK_CHANGED_IND, __func__);

    if (aob_mgr_media_evt_handler.media_track_change_cb)
    {
        aob_mgr_media_evt_handler.media_track_change_cb(p_track_changed_ind->con_lid);
    }
}

static void aob_mgr_gaf_mcc_media_value_ind(void *event)
{
    app_gaf_acc_mcc_value_ind_t *p_val_ind = (app_gaf_acc_mcc_value_ind_t *)event;

    aob_mgr_ble_audio_connected_report(p_val_ind->con_lid);

    switch (p_val_ind->char_type)
    {
        case AOB_MGR_MC_CHAR_TYPE_MEDIA_STATE:
            if (aob_mgr_media_evt_handler.media_playback_status_change_cb)
            {
                aob_mgr_media_evt_handler.media_playback_status_change_cb(p_val_ind->con_lid, (AOB_MGR_PLAYBACK_STATE_E)p_val_ind->val.state);
            }
        break;
        default:
            LOG_D("unknown char type %d", p_val_ind->char_type);
        break;
    }

}

static void aob_mgr_gaf_mcc_media_value_long_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_mcc_value_long_ind_t *p_val_long_ind = (app_gaf_acc_mcc_value_long_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_MCC_MEDIA_VALUE_LONG_IND, __func__);
}

static void aob_mgr_gaf_mcc_svc_changed_handler(uint8_t con_lid)
{
    uint8_t *remote_addr = NULL;
    app_ble_get_peer_solved_addr(con_lid, &remote_addr);
    aob_gattc_delete_nv_cache(remote_addr, GATT_SVC_GENERIC_MEDIA_CONTROL);
    /// clr svc restore success state
    aob_conn_clr_gatt_service_restore_state(con_lid, GATT_C_MCS_POS);
    aob_media_mcs_discovery(con_lid);
}

static void aob_mgr_gaf_mcc_svc_changed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_mcc_svc_changed_ind_t *mcc_svc_changed = (app_gaf_acc_mcc_svc_changed_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_MCC_SVC_CHANGED_IND, __func__);

    app_bt_start_custom_function_in_bt_thread(mcc_svc_changed->con_lid, 0,
        (uint32_t)aob_mgr_gaf_mcc_svc_changed_handler);
}

static void aob_mgr_gaf_mcc_bond_data_ind(void *event)
{
    uint8_t *remote_addr = NULL;
    app_gaf_acc_mcc_bond_data_ind_t *mcc_bond_data = (app_gaf_acc_mcc_bond_data_ind_t *)event;
    LOG_I("app_gaf event handle: %04x, %s", APP_GAF_MCC_BOND_DATA_IND, __func__);
    app_ble_get_peer_solved_addr(mcc_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr, mcc_bond_data->mcs_info.uuid, (void *)mcc_bond_data);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mcc_evt_cb_list[] = {
    //ACC Media Control Callback Functions
    {APP_GAF_MCC_SVC_DISCOVERYED_IND,       aob_mgr_gaf_mcc_svc_discoveryed_ind},
    {APP_GAF_MCC_TRACK_CHANGED_IND,         aob_mgr_gaf_mcc_track_changed_ind},
    {APP_GAF_MCC_MEDIA_VALUE_IND,           aob_mgr_gaf_mcc_media_value_ind},
    {APP_GAF_MCC_MEDIA_VALUE_LONG_IND,      aob_mgr_gaf_mcc_media_value_long_ind},
    {APP_GAF_MCC_SVC_CHANGED_IND,           aob_mgr_gaf_mcc_svc_changed_ind},
    {APP_GAF_MCC_BOND_DATA_IND,             aob_mgr_gaf_mcc_bond_data_ind},
};

static void aob_mgr_gaf_tbc_svc_discoveryed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_cmp_evt_t *tbc_svc_discoveryed = (app_gaf_acc_tbc_cmp_evt_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_SVC_DISCOVERYED_IND, __func__);
    LOG_D("cmd_code:%x con_lid:%x bearer_lid:%x call_id:%x", tbc_svc_discoveryed->cmd_code, tbc_svc_discoveryed->con_lid,
                                                tbc_svc_discoveryed->bearer_lid, tbc_svc_discoveryed->call_id);
    if (ble_audio_earphone_info_set_bearer_lid(tbc_svc_discoveryed->con_lid, tbc_svc_discoveryed->call_id, tbc_svc_discoveryed->bearer_lid))
    {
        LOG_I("Discovery tbs bearer_id complete and map device con_lid with bearer_id");
    }

    #ifdef AOB_MOBILE_ENABLED
    ble_audio_discovery_modify_interval(tbc_svc_discoveryed->con_lid, DISCOVER_COMPLEPE, SERVICE_ACC_TBC);
    #else
    ble_audio_connection_interval_mgr(tbc_svc_discoveryed->con_lid, LEA_CI_MODE_SVC_DISC_CMP);
    #endif
}

static void aob_mgr_gaf_tbc_call_state_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_call_state_ind_t *p_call_state_ind = (app_gaf_acc_tbc_call_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_CALL_STATE_IND, __func__);
    LOG_D("ind_code:%x bearer_lid:%x call_id:%x state:%x", p_call_state_ind->ind_code, p_call_state_ind->bearer_lid,
                                                p_call_state_ind->con_lid, p_call_state_ind->state);
}

static void aob_mgr_gaf_tbc_call_state_long_ind(void *event)
{
    app_gaf_acc_tbc_call_state_long_ind_t *pCallStateInd = (app_gaf_acc_tbc_call_state_long_ind_t *)event;
    uint8_t newState = pCallStateInd->state;
    uint8_t callId = pCallStateInd->id;
    AOB_SINGLE_CALL_INFO_T *pInfo = NULL;
    aob_mgr_ble_audio_connected_report(pCallStateInd->con_lid);
    if ((ACC_TB_CALL_STATE_INCOMING == pCallStateInd->state)
        || (ACC_TB_CALL_STATE_DIALING == pCallStateInd->state)
        || (ACC_TB_CALL_STATE_ALERTING == pCallStateInd->state)
        || (ACC_TB_CALL_STATE_ACTIVE == pCallStateInd->state)
        )
    {
        // 1. Report duplicated ringtone. 2 wechat don't exist dialing.
        if ((ACC_TB_CALL_STATE_INCOMING == pCallStateInd->state)
            || (ACC_TB_CALL_STATE_ALERTING == pCallStateInd->state)
            || (ACC_TB_CALL_STATE_ACTIVE == pCallStateInd->state))
        {
            pInfo = ble_audio_earphone_info_find_call_info(pCallStateInd->con_lid, pCallStateInd->id);
            if (pInfo == NULL)
            {
                pInfo = ble_audio_earphone_info_make_call_info(pCallStateInd->con_lid, pCallStateInd->id, pCallStateInd->uri_len);
            }
            else
            {
                if(ACC_TB_CALL_STATE_INCOMING == pCallStateInd->state || ACC_TB_CALL_STATE_ALERTING == pCallStateInd->state)
                {
                    LOG_I("%s, call_id:%d,dumplicate call.new_state:%d", __func__, callId, newState);
                    return;
                }
            }
        }
        else
        {
            pInfo = ble_audio_earphone_info_make_call_info(pCallStateInd->con_lid, pCallStateInd->id, pCallStateInd->uri_len);
        }
    }
    else
    {
        pInfo = ble_audio_earphone_info_find_call_info(pCallStateInd->con_lid, callId);
    }

    ASSERT(pInfo, "%s, newState:%d, callId:%d", __func__, newState, callId);

    uint8_t call_state_pre = pInfo->state;
    LOG_I("%s, call_id:%d, new_state:%d,pre_state:%d", __func__, callId, newState, call_state_pre);
    if (newState < ACC_TB_CALL_STATE_MAX)
    {
        pInfo->bearer_lid = pCallStateInd->bearer_lid;
        pInfo->call_id = callId;
        pInfo->call_flags.outgoing_call_flag = pCallStateInd->flags&0x01;
        pInfo->call_flags.withheld_server_flag = pCallStateInd->flags&0x02;
        pInfo->call_flags.withheld_network_flag = pCallStateInd->flags&0x04;
        pInfo->uri_len = pCallStateInd->uri_len;
        memcpy(pInfo->uri, pCallStateInd->uri, pCallStateInd->uri_len);
        if (newState < ACC_TB_CALL_STATE_MAX)
        {
            pInfo->state = (AOB_CALL_STATE_E)(newState);
            pInfo->stm_state = (AOB_CALL_STATE_E)(newState);
        }
        else
        {
            pInfo->state = AOB_CALL_STATE_IDLE;
            pInfo->stm_state = AOB_CALL_STATE_IDLE;
        }
    }

    if (pInfo->state != call_state_pre || (pInfo->state == AOB_CALL_STATE_ACTIVE && call_state_pre == AOB_CALL_STATE_ACTIVE))
    {
        if (aob_mgr_call_evt_handler.call_state_change_cb)
        {
            aob_mgr_call_evt_handler.call_state_change_cb(pCallStateInd->con_lid, callId, pInfo);
        }
    }
}

/// Callback function called when value of one of the following characteristic is
/// received:
///     - Bearer Technology characteristic
///     - Bearer Signal Strength characteristic
///     - Bearer Signal Strength Reporting Interval characteristic
///     - Content Control ID characteristic
///     - Status Flags characteristic
///     - Call Control Point Optional Opcodes characteristic
///     - Termination Reason characteristic
static void aob_mgr_gaf_tbc_call_value_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_value_ind_t *p_val_ind = (app_gaf_acc_tbc_value_ind_t *)event;
    LOG_I("%s, con_lid:%d, call_id:%d, char_type:%d", __func__, p_val_ind->con_lid, p_val_ind->call_id, p_val_ind->char_type);

    uint16_t char_value = 0;
    AOB_SINGLE_CALL_INFO_T *pInfo = ble_audio_earphone_info_find_call_info(p_val_ind->con_lid, p_val_ind->call_id);
    AOB_CALL_ENV_INFO_T *pCallEnvInfo = ble_audio_earphone_info_get_call_env_info(p_val_ind->con_lid);

    switch (p_val_ind->char_type)
    {
        case ACC_TB_CHAR_TYPE_SIGN_STRENGTH:
            if (pInfo)
            {
                pInfo->signal_strength = p_val_ind->val.sign_strength;
            }
            if (aob_mgr_call_evt_handler.call_srv_signal_strength_value_ind_cb)
            {
                aob_mgr_call_evt_handler.call_srv_signal_strength_value_ind_cb(p_val_ind->con_lid,
                    p_val_ind->call_id, p_val_ind->val.sign_strength);
            }
            break;
        case ACC_TB_CHAR_TYPE_STATUS_FLAGS:
            char_value = p_val_ind->val.status_flags_bf;
            if (pCallEnvInfo)
            {
                pCallEnvInfo->status_flags.inband_ring_enable = char_value & 0x01;
                pCallEnvInfo->status_flags.silent_mode_enable = char_value & 0x02;
            }
            if (aob_mgr_call_evt_handler.call_status_flags_ind_cb)
            {
                aob_mgr_call_evt_handler.call_status_flags_ind_cb(p_val_ind->con_lid,
                    p_val_ind->call_id, (char_value & 0x01), (char_value & 0x02));
            }
            break;
        case ACC_TB_CHAR_TYPE_CALL_CTL_PT_OPT_OPCODES:
            char_value = p_val_ind->val.opt_opcodes_bf;
            if (pCallEnvInfo)
            {
                pCallEnvInfo->opt_opcode_flags.local_hold_op_supported = char_value & 0x01;
                pCallEnvInfo->opt_opcode_flags.join_op_supported = char_value & 0x02;
            }
            if (aob_mgr_call_evt_handler.call_ccp_opt_supported_opcode_ind_cb)
            {
                aob_mgr_call_evt_handler.call_ccp_opt_supported_opcode_ind_cb(p_val_ind->con_lid, (char_value & 0x01), (char_value & 0x02));
            }
            break;
        case ACC_TB_CHAR_TYPE_TERM_REASON:
            AOB_CALL_SRV_TERMINATE_IND_T call_terminate_ind;
            call_terminate_ind.con_lid = p_val_ind->con_lid;
            call_terminate_ind.bearer_id = p_val_ind->bearer_lid;
            call_terminate_ind.call_id = p_val_ind->call_id;
            call_terminate_ind.terminate_reason = p_val_ind->val.term_reason;
            aob_call_stm_execute(call_terminate_ind.call_id, AOB_CALL_SERVER_TERMINATE_IND_EVT, (AOB_CALL_CLI_DATA_T*)&call_terminate_ind);
            if (pInfo)
            {
                pInfo->state = AOB_CALL_STATE_IDLE;
                if (aob_mgr_call_evt_handler.call_state_change_cb)
                {
                    aob_mgr_call_evt_handler.call_state_change_cb(call_terminate_ind.con_lid, call_terminate_ind.call_id, pInfo);
                }
            }
            if (aob_mgr_call_evt_handler.call_terminate_reason_ind_cb)
            {
                char_value = p_val_ind->val.term_reason;
                aob_mgr_call_evt_handler.call_terminate_reason_ind_cb(p_val_ind->con_lid, p_val_ind->call_id, char_value);
            }
            break;
        case ACC_TB_CHAR_TYPE_CALL_CTL_PT:
            break;
        default:
            break;
    }
}

/// Callback function called when value of one of the following characteristic is
/// received:
///     - Bearer Provider Name characteristic
///     - Bearer UCI characteristic
///     - Bearer URI Schemes Supported List characteristic
///     - Incoming Call Target Bearer URI characteristic
///     - Incoming Call characteristic
///     - Call Friendly Name characteristic
static void aob_mgr_gaf_tbc_call_value_long_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_value_long_ind_t *p_val_long_ind = (app_gaf_acc_tbc_value_long_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_CALL_VALUE_LONG_IND, __func__);
    switch (p_val_long_ind->char_type)
    {
        case ACC_TB_CHAR_TYPE_IN_TGT_CALLER_ID:
            break;
        case ACC_TB_CHAR_TYPE_INCOMING_CALL:
            if (aob_mgr_call_evt_handler.call_incoming_number_inf_ind_cb)
            {
                aob_mgr_call_evt_handler.call_incoming_number_inf_ind_cb(p_val_long_ind->con_lid, p_val_long_ind->call_id,
                                                p_val_long_ind->val_len, p_val_long_ind->val);
            }
            break;
        case ACC_TB_CHAR_TYPE_CALL_FRIENDLY_NAME:
            break;
        default:
            break;
    }
}

static void aob_mgr_gaf_tbc_svc_changed_handler(uint8_t con_lid)
{
    uint8_t *remote_addr = NULL;
    app_ble_get_peer_solved_addr(con_lid, &remote_addr);
    aob_gattc_delete_nv_cache(remote_addr, GATT_SVC_GENERIC_TELEPHONE_BEARER);
    /// clr svc restore success state
    aob_conn_clr_gatt_service_restore_state(con_lid, GATT_C_TBS_POS);
    aob_call_tbs_discovery(con_lid);
}

static void aob_mgr_gaf_tbc_svc_changed_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_acc_tbc_svc_changed_ind_t *tbc_svc_changed = (app_gaf_acc_tbc_svc_changed_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_SVC_CHANGED_IND, __func__);
    LOG_D("ind_code:%x", tbc_svc_changed->con_lid);
    if (aob_mgr_call_evt_handler.call_svc_changed_ind_cb)
    {
        aob_mgr_call_evt_handler.call_svc_changed_ind_cb(tbc_svc_changed->con_lid);
    }
    app_bt_start_custom_function_in_bt_thread(tbc_svc_changed->con_lid, 0,
        (uint32_t)aob_mgr_gaf_tbc_svc_changed_handler);
}

static void aob_mgr_gaf_tbc_call_action_result_ind(void *event)
{
    POSSIBLY_UNUSED acc_tbc_cmp_evt_t *tbc_action_result = (acc_tbc_cmp_evt_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_TBC_CALL_ACTION_RESULT_IND, __func__);

    if (aob_mgr_call_evt_handler.call_action_result_ind_cb)
    {
        AOB_CALL_CLI_ACTION_RESULT_IND_T action_result;
        action_result.con_lid = tbc_action_result->con_lid;
        action_result.bearer_id = tbc_action_result->bearer_lid;
        action_result.action_opcode = tbc_action_result->u.opcode;
        action_result.result = tbc_action_result->result;
        aob_mgr_call_evt_handler.call_action_result_ind_cb(tbc_action_result->con_lid, &action_result);
    }
}

static void aob_mgr_gaf_tbc_svc_bond_data_ind(void *event)
{
    uint8_t *remote_addr = NULL;
    app_gaf_acc_tbc_bond_data_ind_t *tbc_bond_data = (app_gaf_acc_tbc_bond_data_ind_t *)event;
    LOG_I("app_gaf event handle: %04x, %s", APP_GAF_TBC_BOND_DATA_IND, __func__);

    app_ble_get_peer_solved_addr(tbc_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr, tbc_bond_data->tbs_info.uuid, (void *)tbc_bond_data);
    aob_mgr_ble_audio_connected_report(tbc_bond_data->con_lid);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_tbc_evt_cb_list[] = {
    //ACC Call Control Callback Functions
    {APP_GAF_TBC_SVC_DISCOVERYED_IND,       aob_mgr_gaf_tbc_svc_discoveryed_ind},
    {APP_GAF_TBC_CALL_STATE_IND,            aob_mgr_gaf_tbc_call_state_ind},
    {APP_GAF_TBC_CALL_STATE_LONG_IND,       aob_mgr_gaf_tbc_call_state_long_ind},
    {APP_GAF_TBC_CALL_VALUE_IND,            aob_mgr_gaf_tbc_call_value_ind},
    {APP_GAF_TBC_CALL_VALUE_LONG_IND,       aob_mgr_gaf_tbc_call_value_long_ind},
    {APP_GAF_TBC_SVC_CHANGED_IND,           aob_mgr_gaf_tbc_svc_changed_ind},
    {APP_GAF_TBC_CALL_ACTION_RESULT_IND,    aob_mgr_gaf_tbc_call_action_result_ind},
    {APP_GAF_TBC_BOND_DATA_IND,             aob_mgr_gaf_tbc_svc_bond_data_ind}
};

static void aob_mgr_gaf_aics_state_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_aics_state_ind_t *p_state_ind = (app_gaf_arc_aics_state_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_AICS_STATE_IND, __func__);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_aics_evt_cb_list[] = {
    //ARC Audio Input Control Callback Function
    {APP_GAF_AICS_STATE_IND,                aob_mgr_gaf_aics_state_ind},
};

static void aob_mgr_gaf_mics_mute_ind(void *event)
{
    app_gaf_arc_mics_mute_ind_t *mute = (app_gaf_arc_mics_mute_ind_t *)event;

    if (aob_mgr_media_evt_handler.media_mic_state_cb)
    {
        aob_mgr_media_evt_handler.media_mic_state_cb(mute->mute);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_mics_evt_cb_list[] = {
    //ARC Microphone Control Callback Function
    {APP_GAF_MICS_MUTE_IND,                 aob_mgr_gaf_mics_mute_ind},
};

static void aob_mgr_gaf_vcs_volume_ind(void *event)
{
    app_gaf_arc_vcs_volume_ind_t *p_volume_ind = (app_gaf_arc_vcs_volume_ind_t *)event;
    uint8_t reason = p_volume_ind->reason;
    LOG_D("%s volume %d mute %d reason %d", __func__, p_volume_ind->volume,
                    p_volume_ind->mute, reason);
    if (aob_mgr_vol_evt_handler.vol_changed_cb)
    {
        aob_mgr_vol_evt_handler.vol_changed_cb(0, p_volume_ind->volume,
                    p_volume_ind->mute, reason);
    }
}

static void aob_mgr_gaf_vcs_flags_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vcs_flags_ind_t *p_flags_ind = (app_gaf_arc_vcs_flags_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VCS_FLAGS_IND, __func__);
}

static void aob_mgr_gaf_vcs_bond_data_ind(void* event)
{
    app_gaf_arc_vcs_bond_data_ind_t *vcs_bond_data = (app_gaf_arc_vcs_bond_data_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VCS_BOND_DATA_IND, __func__);
    uint8_t *remote_addr = NULL;
    app_ble_get_peer_solved_addr(vcs_bond_data->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr, GATT_SVC_VOLUME_CONTROL, (void *)vcs_bond_data);
    if (aob_mgr_vol_evt_handler.vcs_bond_data_changed_cb) {
        aob_mgr_vol_evt_handler.vcs_bond_data_changed_cb(vcs_bond_data->con_lid,
            vcs_bond_data->char_type, vcs_bond_data->cli_cfg_bf);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_vcs_evt_cb_list[] = {
    // VCS Events
    {APP_GAF_VCS_VOLUME_IND,                aob_mgr_gaf_vcs_volume_ind},
    {APP_GAF_VCS_FLAGS_IND,                 aob_mgr_gaf_vcs_flags_ind},
    {APP_GAF_VCS_BOND_DATA_IND,             aob_mgr_gaf_vcs_bond_data_ind},
};

static void aob_mgr_gaf_vocs_location_set_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vocs_set_location_req_ind_t *p_vocs_loc_req_ind = (app_gaf_arc_vocs_set_location_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VOCS_LOCATION_SET_RI, __func__);
}

static void aob_mgr_gaf_vocs_offset_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vocs_offset_ind_t *p_offset_ind = (app_gaf_arc_vocs_offset_ind_t *)event;
    if (aob_mgr_vol_evt_handler.vocs_offset_changed_cb) {
        aob_mgr_vol_evt_handler.vocs_offset_changed_cb(p_offset_ind->offset, p_offset_ind->output_lid);
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VOCS_OFFSET_IND, __func__);
}

static void aob_mgr_gaf_vocs_bond_data_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_arc_vocs_cfg_ind_t *p_cfg_ind = (app_gaf_arc_vocs_cfg_ind_t *)event;
    if (aob_mgr_vol_evt_handler.vocs_bond_data_changed_cb) {
        aob_mgr_vol_evt_handler.vocs_bond_data_changed_cb(p_cfg_ind->output_lid, p_cfg_ind->cli_cfg_bf);
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_VOCS_BOND_DATA_IND, __func__);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_vocs_evt_cb_list[] = {
    //ARC Volume Offset Control Callback Functions
    {APP_GAF_VOCS_LOCATION_SET_RI,          aob_mgr_gaf_vocs_location_set_ri},
    {APP_GAF_VOCS_OFFSET_IND,               aob_mgr_gaf_vocs_offset_ind},
    {APP_GAF_VOCS_BOND_DATA_IND,            aob_mgr_gaf_vocs_bond_data_ind},
};

static void aob_mgr_gaf_csism_lock_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_atc_csism_lock_ind_t *p_lock_ind = (app_gaf_atc_csism_lock_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISM_LOCK_IND, __func__);
}

static void aob_mgr_gaf_csism_ltk_ri(void *event)
{
    POSSIBLY_UNUSED app_gaf_atc_csism_ltk_req_ind_t *p_authorization_ri = (app_gaf_atc_csism_ltk_req_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISM_LTK_RI, __func__);
}

static void aob_mgr_gaf_csism_rsi_generated_ind(void *event)
{
    POSSIBLY_UNUSED app_gaf_atc_sism_rsi_ind_t *p_rsi_ind = (app_gaf_atc_sism_rsi_ind_t *)event;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISM_NEW_RSI_GENERATED_IND, __func__);
    DUMP8("%02x ", p_rsi_ind->rsi.rsi, CSIS_RSI_LEN);

     if (aob_mgr_csip_evt_handler.csip_rsi_value_updated_cb)
     {
         aob_mgr_csip_evt_handler.csip_rsi_value_updated_cb(p_rsi_ind->rsi.rsi, CSIS_RSI_LEN);
     }
}

static void aob_mgr_gaf_csism_ntf_sent_ind(void *event)
{
    app_gaf_atc_csism_ntf_sent_t *ntf_sent_ind = (app_gaf_atc_csism_ntf_sent_t*)event;
    if (aob_mgr_csip_evt_handler.csip_ntf_sent_cb)
    {
        aob_mgr_csip_evt_handler.csip_ntf_sent_cb(ntf_sent_ind->con_lid, ntf_sent_ind->char_type);
    }
}

static void aob_mgr_gaf_csism_read_rsp_sent_ind(void *event)
{
    app_gaf_atc_csism_read_rsp_sent_t *read_rsp_sent_ind = (app_gaf_atc_csism_read_rsp_sent_t*)event;
    LOG_I("app_gaf event handle: %s", __func__);
    if (aob_mgr_csip_evt_handler.csip_read_rsp_sent_cb)
    {
        aob_mgr_csip_evt_handler.csip_read_rsp_sent_cb(read_rsp_sent_ind->con_lid,
            read_rsp_sent_ind->char_type, read_rsp_sent_ind->data, read_rsp_sent_ind->data_len);
    }
}

static void aob_mgr_gaf_csism_bond_data_ind(void *param)
{
    app_gaf_atc_csism_bond_data_ind_t *bond_data_ind = (app_gaf_atc_csism_bond_data_ind_t *)param;
    LOG_D("app_gaf event handle: %04x, %s", APP_GAF_CSISM_BOND_DATA_IND, __func__);
    uint8_t *remote_addr = NULL;
    app_ble_get_peer_solved_addr(bond_data_ind->con_lid, &remote_addr);
    aob_gattc_cache_save(remote_addr, GATT_SVC_COORD_SET_IDENTIFICATION, (void *)bond_data_ind);
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_csim_evt_cb_list[] = {
    //ATC CSISM Callback Functions
    {APP_GAF_CSISM_LOCK_IND,                  aob_mgr_gaf_csism_lock_ind},
    {APP_GAF_CSISM_LTK_RI,                    aob_mgr_gaf_csism_ltk_ri},
    {APP_GAF_CSISM_NEW_RSI_GENERATED_IND,     aob_mgr_gaf_csism_rsi_generated_ind},
    {APP_GAF_CSISM_BOND_DATA_IND,             aob_mgr_gaf_csism_bond_data_ind},
    {APP_GAF_CSISM_NTF_SENT_IND,              aob_mgr_gaf_csism_ntf_sent_ind},
    {APP_GAF_CSISM_READ_RSP_SENT_IND,         aob_mgr_gaf_csism_read_rsp_sent_ind},
};

static void aob_mgr_gaf_bc_scan_state_idle_ind(void *event)
{
    if (aob_mgr_scan_evt_handler.scan_state_idle_cb) {
        aob_mgr_scan_evt_handler.scan_state_idle_cb();
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_IDLE_IND, __func__);
}

static void aob_mgr_gaf_bc_scan_state_scanning_ind(void *event)
{
    if (aob_mgr_scan_evt_handler.scan_state_scanning_cb) {
        aob_mgr_scan_evt_handler.scan_state_scanning_cb();
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_SCANNING_IND, __func__);
}

static void aob_mgr_gaf_bc_scan_state_synchronizing_ind(void *event)
{
    if (aob_mgr_scan_evt_handler.scan_state_synchronizing_cb) {
        aob_mgr_scan_evt_handler.scan_state_synchronizing_cb();
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_SYNCHRONIZING_IND, __func__);
}

static void aob_mgr_gaf_bc_scan_state_synchronized_ind(void *event)
{
    if (aob_mgr_scan_evt_handler.scan_state_synchronized_cb) {
        aob_mgr_scan_evt_handler.scan_state_synchronized_cb();
    }
    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_SYNCHRONIZED_IND, __func__);
}

static void aob_mgr_gaf_bc_scan_state_streaming_ind(void *event)
{
    app_gaf_bc_scan_state_stream_t *p_scan_state_ind = (app_gaf_bc_scan_state_stream_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_BAP_BC_SCAN_STATE_STREAMING_IND, __func__);

    if (aob_mgr_scan_evt_handler.scan_state_streaming_cb) {
        aob_mgr_scan_evt_handler.scan_state_streaming_cb(p_scan_state_ind);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_bc_scan_state_evt_cb_list[] = {
    //BIS scan state callback function
    {APP_BAP_BC_SCAN_STATE_IDLE_IND,          aob_mgr_gaf_bc_scan_state_idle_ind},
    {APP_BAP_BC_SCAN_STATE_SCANNING_IND,      aob_mgr_gaf_bc_scan_state_scanning_ind},
    {APP_BAP_BC_SCAN_STATE_SYNCHRONIZING_IND, aob_mgr_gaf_bc_scan_state_synchronizing_ind},
    {APP_BAP_BC_SCAN_STATE_SYNCHRONIZED_IND,  aob_mgr_gaf_bc_scan_state_synchronized_ind},
    {APP_BAP_BC_SCAN_STATE_STREAMING_IND,     aob_mgr_gaf_bc_scan_state_streaming_ind},
};

static void aob_mgr_gaf_dts_coc_registered_ind(void *event)
{
    app_gaf_dts_cmp_evt_t *p_cmp_evt_t = (app_gaf_dts_cmp_evt_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_REGISTERED_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_registered_cb) {
        aob_mgr_dts_coc_evt_handler.dts_coc_registered_cb(p_cmp_evt_t->status,
            p_cmp_evt_t->spsm);
    }
}

static void aob_mgr_gaf_dts_coc_connected_ind(void *event)
{
    app_gaf_dts_coc_connected_ind_t *p_coc_connected_ind = (app_gaf_dts_coc_connected_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_CONNECTED_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_connected_cb) {
        aob_mgr_dts_coc_evt_handler.dts_coc_connected_cb(p_coc_connected_ind->con_lid,
            p_coc_connected_ind->tx_mtu, p_coc_connected_ind->tx_mps,
            p_coc_connected_ind->spsm, p_coc_connected_ind->initial_credits);
    }
}

static void aob_mgr_gaf_dts_coc_disconnected_ind(void *event)
{
    app_gaf_dts_coc_disconnected_ind_t* p_coc_disconnected_ind = (app_gaf_dts_coc_disconnected_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_DISCONNECTED_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_disconnected_cb) {
        aob_mgr_dts_coc_evt_handler.dts_coc_disconnected_cb(p_coc_disconnected_ind->con_lid,
            p_coc_disconnected_ind->reason, p_coc_disconnected_ind->spsm);
    }
}

static void aob_mgr_gaf_dts_coc_data_ind(void *event)
{
    app_gaf_dts_coc_data_ind_t *p_data_coc_ind = (app_gaf_dts_coc_data_ind_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_DATA_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_data_cb) {
        aob_mgr_dts_coc_evt_handler.dts_coc_data_cb(p_data_coc_ind->con_lid, p_data_coc_ind->spsm,
        p_data_coc_ind->length, p_data_coc_ind->sdu);
    }
}

static void aob_mgr_gaf_dts_coc_send_ind(void *event)
{
    app_gaf_dts_cmp_evt_t *p_cmp_evt_t = (app_gaf_dts_cmp_evt_t *)event;

    LOG_D("app_gaf event handle: %04x, %s", APP_DTS_COC_SEND_IND, __func__);

    if (aob_mgr_dts_coc_evt_handler.dts_coc_send_cb) {
        aob_mgr_dts_coc_evt_handler.dts_coc_send_cb(p_cmp_evt_t->con_lid, p_cmp_evt_t->spsm);
    }
}

const aob_app_gaf_evt_cb_t aob_mgr_gaf_dts_evt_cb_list[] = {
    //DTS event callback function
    {APP_DTS_COC_REGISTERED_IND,         aob_mgr_gaf_dts_coc_registered_ind},
    {APP_DTS_COC_CONNECTED_IND,          aob_mgr_gaf_dts_coc_connected_ind},
    {APP_DTS_COC_DISCONNECTED_IND,       aob_mgr_gaf_dts_coc_disconnected_ind},
    {APP_DTS_COC_DATA_IND,               aob_mgr_gaf_dts_coc_data_ind},
    {APP_DTS_COC_SEND_IND,               aob_mgr_gaf_dts_coc_send_ind},
};

void aob_mgr_gaf_evt_handle(uint16_t id, void *event_id)
{
    uint8_t module_id = (uint8_t)GAF_ID_GET(id);
    uint16_t event = (uint16_t)GAF_EVENT_GET(id);
    aob_app_gaf_evt_cb_t *evt_cb = NULL;

    switch (module_id) {
        case APP_GAF_ASCS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_ascs_evt_cb_list[0];
        }
            break;
        case APP_GAF_PACS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_pacs_evt_cb_list[0];
        }
            break;
        case APP_GAF_BIS_SCAN_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_bis_scan_evt_cb_list[0];
        }
            break;

        case APP_GAF_BIS_SINK_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_bis_sink_evt_cb_list[0];
        }
            break;
        case APP_GAF_DELEG_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_deleg_evt_cb_list[0];
        }
            break;
        case APP_GAF_MCC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mcc_evt_cb_list[0];
        }
            break;

        case APP_GAF_TBC_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_tbc_evt_cb_list[0];
        }
            break;
        case APP_GAF_AICS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_aics_evt_cb_list[0];
        }
            break;
        case APP_GAF_MICS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_mics_evt_cb_list[0];
        }
            break;
        case APP_GAF_VCS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_vcs_evt_cb_list[0];
        }
            break;
        case APP_GAF_VOCS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_vocs_evt_cb_list[0];
        }
            break;
        case APP_GAF_CSISM_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_csim_evt_cb_list[0];
        }
            break;
        case APP_GAF_BC_SCAN_STATE_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_bc_scan_state_evt_cb_list[0];
        }
            break;
        case APP_GAF_DTS_MODULE:
        {
            evt_cb = (aob_app_gaf_evt_cb_t *)&aob_mgr_gaf_dts_evt_cb_list[0];
        }
            break;

        default:
            break;
    }

    if ((evt_cb) && evt_cb[event].cb) {
        evt_cb[event].cb(event_id);
    }
}

void aob_mgr_call_evt_handler_register(call_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_call_evt_handler, handlerBundle, sizeof(call_event_handler_t));
}

void aob_mgr_media_evt_handler_register(media_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_media_evt_handler, handlerBundle, sizeof(media_event_handler_t));
}

void aob_mgr_gaf_vol_evt_handler_register(vol_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_vol_evt_handler, handlerBundle, sizeof(vol_event_handler_t));
}

void aob_mgr_gaf_sink_evt_handler_register(sink_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_sink_evt_handler, handlerBundle, sizeof(sink_event_handler_t));
}

void aob_mgr_gaf_scan_evt_handler_register(scan_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_scan_evt_handler, handlerBundle, sizeof(scan_event_handler_t));
}

void aob_mgr_gaf_deleg_evt_handler_register(deleg_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_deleg_evt_handler, handlerBundle, sizeof(deleg_event_handler_t));
}

void aob_mgr_csip_evt_handler_register(csip_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_csip_evt_handler, handlerBundle, sizeof(csip_event_handler_t));
}

void aob_mgr_dts_coc_evt_handler_register(dts_coc_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_dts_coc_evt_handler, handlerBundle, sizeof(dts_coc_event_handler_t));
}

void aob_mgr_cis_conn_evt_handler_t_register(cis_conn_evt_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_cis_conn_evt_handler, handlerBundle, sizeof(cis_conn_evt_handler_t));
}

void aob_mgr_pacs_evt_handler_t_register(pacs_event_handler_t *handlerBundle)
{
    memcpy(&aob_mgr_pacs_event_handler, handlerBundle, sizeof(pacs_event_handler_t));
}

void aob_mgr_gaf_evt_init(void)
{
    aob_csip_if_init();
    aob_conn_api_init();
    aob_media_api_init();
    aob_call_if_init();
    aob_vol_api_init();
    aob_pacs_api_init();

#if CFG_BAP_BC
    aob_bis_scan_api_init();
    aob_bis_sink_api_init();
    aob_bis_deleg_api_init();
#endif

    aob_cis_api_init();
}
#endif

/// @} APP
