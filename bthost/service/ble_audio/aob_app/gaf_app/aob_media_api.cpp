/**
 * @file aob_media_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-31
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */

/*****************************header include********************************/
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "app_trace_rx.h"
#include "plat_types.h"
#include "app_gaf_dbg.h"
#include "heap_api.h"
#include "app_bap.h"
#include "app_bap_bc_src_msg.h"
#include "app_bap_uc_srv_msg.h"
#include "aob_media_api.h"
#include "app_gaf_define.h"
#include "ble_audio_ase_stm.h"
#include "ble_audio_earphone_info.h"
#include "ble_audio_core.h"
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "me_api.h"
#include "app_acc_mcc_msg.h"
#include "app_ble_audio_stream_handler.h"
#include "app_audio_active_device_manager.h"

/****************************for server(earbud)*****************************/

/*********************external function declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
BLE_AUD_CORE_EVT_CB_T *p_media_cb = NULL;
/**
 * @brief use for fill  qos req in codec req_ind - cfm cmd
 *
 */
static get_qos_req_cfg_info_cb aob_ascs_srv_codec_req_handler_cb = NULL;
/****************************function defination****************************/
void aob_media_mcs_discovery(uint8_t con_lid)
{
    app_acc_mcc_start(con_lid);
}

void aob_mc_char_type_get(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type)
{
    uint8_t con_lid = 1;
    app_acc_mcc_get(con_lid, 0, (uint8_t)char_type);
}

void aob_mc_char_type_set(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, int32_t val)
{
    uint8_t con_lid = 1;
    app_acc_mcc_set(con_lid, 0, (uint8_t)char_type, val);
}

void aob_mc_get_cfg(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type)
{
    uint8_t con_lid = 1;
    app_acc_mcc_get_cfg(con_lid, 0, (uint8_t)char_type);
}

void aob_mc_set_cfg(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, uint8_t enable)
{
    uint8_t con_lid = 1;
    app_acc_mcc_set_cfg(con_lid, 0, (uint8_t)char_type, enable);
}

void aob_mc_set_obj_id(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, uint8_t *obj_id)
{
    uint8_t con_lid = 1;
    app_acc_mcc_set_obj_id(con_lid, 0, (uint8_t)char_type, obj_id);
}

void aob_media_control(uint8_t con_lid, AOB_MGR_MC_OPCODE_E opcode, uint32_t val)
{
    app_acc_mcc_control(con_lid, 0 , (uint8_t)opcode, val);
}

void aob_media_play(uint8_t con_lid)
{
    aob_media_control(con_lid, AOB_MGR_MC_OP_PLAY, 0);
}

void aob_media_pause(uint8_t con_lid)
{
    aob_media_control(con_lid, AOB_MGR_MC_OP_PAUSE, 0);
}

void aob_media_stop(uint8_t con_lid)
{
    aob_media_control(con_lid, AOB_MGR_MC_OP_STOP, 0);
}

void aob_media_next(uint8_t con_lid)
{
    aob_media_control(con_lid, AOB_MGR_MC_OP_NEXT_TRACK, 0);
}

void aob_media_prev(uint8_t con_lid)
{
    aob_media_control(con_lid, AOB_MGR_MC_OP_PREV_TRACK, 0);
}

void aob_media_search(uint8_t device_id, uint8_t param_len, uint8_t *param)
{
    uint8_t con_lid = 1;
    app_acc_mcc_search(con_lid, 0, param_len, param);
}

AOB_MGR_STREAM_STATE_E aob_media_get_cur_ase_state(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ascs_ase = app_bap_uc_srv_get_ase_info(ase_lid);

    return (p_ascs_ase != NULL) ? (AOB_MGR_STREAM_STATE_E)p_ascs_ase->ase_state : AOB_MGR_STREAM_STATE_MAX;
}

void aob_media_set_location(AOB_MGR_LOCATION_BF_E location)
{
    app_bap_capa_srv_set_supp_location_bf(APP_GAF_DIRECTION_SINK, (uint32_t)location);
    app_bap_capa_srv_set_supp_location_bf(APP_GAF_DIRECTION_SRC, (uint32_t)location);
}

uint32_t aob_media_get_cur_location(uint8_t direction)
{
    return app_bap_capa_srv_get_location_bf((enum app_gaf_direction)direction);
}

void aob_media_set_ava_audio_context(uint8_t con_lid, uint16_t context_bf_ava_sink, uint16_t context_bf_ava_src)
{
    app_bap_capa_srv_set_ava_context_bf(con_lid, context_bf_ava_sink, context_bf_ava_src);
}

void aob_media_set_supp_audio_context(uint8_t con_lid, AOB_MGR_CONTEXT_TYPE_BF_E context_bf_supp)
{
    app_bap_capa_srv_set_supp_context_bf(con_lid, APP_GAF_DIRECTION_SINK, (uint16_t)context_bf_supp);
    app_bap_capa_srv_set_supp_context_bf(con_lid, APP_GAF_DIRECTION_SRC, (uint16_t)context_bf_supp);
}

void aob_media_send_enable_rsp(uint8_t ase_id, bool accept)
{
    app_bap_uc_srv_send_enable_rsp(ase_id, accept);
}

void aob_media_release_stream(uint8_t ase_id, uint8_t switchToIdle)
{
    app_bap_uc_srv_stream_release(ase_id, switchToIdle);
}

void aob_media_read_iso_link_quality(uint8_t ase_id)
{
    if (AOB_MGR_STREAM_STATE_STREAMING == aob_media_get_cur_ase_state(ase_id)) {
        app_bap_uc_srv_read_iso_link_quality(ase_id);
    }
}

void aob_media_set_iso_quality_rep_thr(uint8_t ase_lid, uint16_t qlty_rep_evt_cnt_thr,
    uint16_t tx_unack_pkts_thr, uint16_t tx_flush_pkts_thr, uint16_t tx_last_subevent_pkts_thr, uint16_t retrans_pkts_thr,
    uint16_t crc_err_pkts_thr, uint16_t rx_unreceived_pkts_thr, uint16_t duplicate_pkts_thr)
{
    app_bap_ascs_ase_t *p_ase = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase->cis_hdl != GAP_INVALID_CONHDL) {
        btif_me_bt_dbg_set_iso_quality_rep_thr(p_ase->cis_hdl, qlty_rep_evt_cnt_thr,
            tx_unack_pkts_thr, tx_flush_pkts_thr, tx_last_subevent_pkts_thr, retrans_pkts_thr,
            crc_err_pkts_thr, rx_unreceived_pkts_thr, duplicate_pkts_thr);
    }
}

void aob_media_disable_stream(uint8_t ase_id)
{
    app_bap_uc_srv_stream_disable(ase_id);
}

void aob_media_update_metadata(uint8_t ase_lid, app_bap_cfg_metadata_t *meta_data)
{
    app_bap_uc_srv_update_metadata_req(ase_lid, meta_data);
}

uint8_t aob_media_get_cur_streaming_ase_lid(uint8_t con_lid, AOB_MGR_DIRECTION_E direction)
{
    return app_bap_uc_srv_get_streaming_ase_lid(con_lid, (enum app_gaf_direction)direction);
}

uint8_t aob_media_get_curr_streaming_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list)
{
    return app_bap_uc_srv_get_streaming_ase_lid_list(con_lid, ase_lid_list);
}

AOB_MGR_CONTEXT_TYPE_BF_E aob_media_get_cur_context_type_by_ase_lid(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);
    if ((NULL != p_ase_info) && (NULL != p_ase_info->p_metadata)) {
        return (AOB_MGR_CONTEXT_TYPE_BF_E)p_ase_info->p_metadata->param.context_bf;
    }

    return AOB_AUDIO_CONTEXT_TYPE_UNSPECIFIED;
}

AOB_MGR_CONTEXT_TYPE_BF_E aob_media_get_cur_context_type(uint8_t ase_lid)
{
    AOB_MGR_CONTEXT_TYPE_BF_E ctxType = AOB_AUDIO_CONTEXT_TYPE_UNSPECIFIED;

    uint8_t con_lid = app_audio_adm_get_le_audio_active_device();

    if (INVALID_CONNECTION_INDEX != con_lid) {
        for(uint32_t i = 0; i < AOP_MGR_DIRECTION_MAX; i++) {
            uint8_t ase_lid = app_bap_uc_srv_get_streaming_ase_lid(con_lid, (enum app_gaf_direction)i);
            if (GAF_INVALID_LID != ase_lid) {
                ctxType = aob_media_get_cur_context_type_by_ase_lid(ase_lid);
                break;
            }
        }
    }
    return ctxType;
}

void aob_media_set_qos_info(uint8_t ase_lid, app_bap_qos_req_t *qos_info)
{
    app_bap_uc_srv_set_ase_qos_req(ase_lid, qos_info);
}

void aob_media_mics_set_mute(uint8_t mute)
{
    AOB_MOBILE_INFO_T *p_info = NULL;
    uint8_t conidx = app_audio_adm_get_le_audio_active_device();
    /// TODO:no active device
    if (BT_DEVICE_INVALID_ID == conidx) {
        LOG_E("%s no active device to set vol", __func__);
        conidx = 0;
    }
    p_info = ble_audio_earphone_info_get_mobile_info(conidx);
    if (NULL != p_info) {
        p_info->media_info.mic_mute = mute;
    }

    app_arc_mics_set_mute(mute);
}

APP_GAF_CODEC_TYPE_T aob_media_get_codec_type(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ascs_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (!p_ascs_info) {
        TRACE(2, "%s ASCS null, ase_lid %d", __func__, ase_lid);
        return APP_GAF_CODEC_TYPE_SIG_MAX;
    }

    return (APP_GAF_CODEC_TYPE_T)p_ascs_info->codec_id.codec_id[0];
}

GAF_BAP_SAMLLING_REQ_T aob_media_get_sample_rate(uint8_t ase_lid)
{
    app_bap_ascs_ase_t *p_ascs_info = app_bap_uc_srv_get_ase_info(ase_lid);
    uint8_t ret = GAF_BAP_SAMPLE_FREQ_MAX;
    APP_GAF_CODEC_TYPE_T code_type = aob_media_get_codec_type(ase_lid);

    if (!p_ascs_info) {
        TRACE(2, "%s ASCS null, ase_lid %d", __func__, ase_lid);
        goto exit;
    }

    if (APP_GAF_CODEC_TYPE_LC3 == code_type) {
        ret = p_ascs_info->p_cfg->param.sampling_freq;

    } else {
#ifdef LC3PLUS_SUPPORT
        ret = p_ascs_info->p_cfg->param.sampling_freq;
#endif
        //TODO: Add vendor codec
    }

exit:
    return (GAF_BAP_SAMLLING_REQ_T)ret;
}

AOB_MEDIA_METADATA_CFG_T *aob_media_get_metadata(uint8_t ase_lid)
{
    AOB_MEDIA_METADATA_CFG_T *p_metadata_cfg = NULL;

    app_bap_ascs_ase_t *p_ascs_info = app_bap_uc_srv_get_ase_info(ase_lid);
    if (!p_ascs_info) {
        LOG_E("%s ASCS null, ase_lid %d", __func__, ase_lid);
        goto exit;
    }

    p_metadata_cfg = (AOB_MEDIA_METADATA_CFG_T *)&p_ascs_info->p_metadata->add_metadata.len;

exit:
    return p_metadata_cfg;
}

bool aob_media_is_exist_qos_configured_ase(void)
{
    uint8_t aseNum = app_bap_uc_srv_get_nb_ases_cfg();

    for (uint8_t i = 0; i < aseNum; i++)
    {
        if (aob_media_get_cur_ase_state(i) == AOB_MGR_STREAM_STATE_QOS_CONFIGURED) {
            return true;
        }
    }

    return false;
}

AOB_MGR_PLAYBACK_STATE_E aob_media_get_state(uint8_t con_lid)
{
    AOB_MEDIA_INFO_T *p_media_info= ble_audio_earphone_info_get_media_info(con_lid);

    if (!p_media_info)
    {
        return AOB_MGR_PLAYBACK_STATE_MAX;
    }

    return (AOB_MGR_PLAYBACK_STATE_E)p_media_info->media_state;
}

void aob_media_ascs_srv_set_codec(uint8_t ase_lid, const app_gaf_codec_id_t *codec_id,
                                app_bap_qos_req_t *ntf_qos_req, app_bap_cfg_t *ntf_codec_cfg)
{
    if (!ntf_qos_req || !ntf_codec_cfg)
    {
        LOG_E("%s Err Params", __func__);
        return;
    }

    app_bap_ascs_ase_t *p_ase_info = app_bap_uc_srv_get_ase_info(ase_lid);

    if (p_ase_info->ase_state != APP_BAP_UC_ASE_STATE_QOS_CONFIGURED &&
        p_ase_info->ase_state != APP_BAP_UC_ASE_STATE_CODEC_CONFIGURED &&
        p_ase_info->ase_state != APP_BAP_UC_ASE_STATE_IDLE)
    {
        LOG_W("%s ase state: %d", __func__, p_ase_info->ase_state);
        return;
    }

    // Record Qos req for codec cfg ntf use
    p_ase_info->qos_req = *ntf_qos_req;
    // Record Codec id
    memcpy(&p_ase_info->codec_id, codec_id, sizeof(app_gaf_codec_id_t));
    //Call gaf api
    app_bap_uc_srv_configure_codec_req(p_ase_info, ntf_codec_cfg);
}

static void aob_media_stream_status_cb(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state)
{
    AOB_MEDIA_INFO_T *p_media_info = ble_audio_earphone_info_get_media_info(con_lid);

    ASSERT( ((p_media_info != NULL) || (ase_lid < APP_BAP_DFT_ASCS_NB_ASE_CHAR)),
        "%s con_lid %d ase)lid %d", __FUNCTION__, con_lid, ase_lid);


    if (NULL != p_media_cb) {
        p_media_cb->ble_media_stream_status_change_cb(con_lid, ase_lid, state);
    }
}

static void aob_media_playback_status_cb(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state)
{
    LOG_I("%s con_lid %d state %d", __func__, con_lid, state);

    AOB_MEDIA_INFO_T *p_media_info= ble_audio_earphone_info_get_media_info(con_lid);

    ASSERT( p_media_info != NULL,
        "%s con_lid %d media_state %d", __FUNCTION__, con_lid, p_media_info->media_state);

    p_media_info->media_state = state;
    if (NULL != p_media_cb) {
        p_media_cb->ble_media_playback_status_change_cb(con_lid, state);
    }
}

static void aob_media_track_changed_cb(uint8_t con_lid)
{
    LOG_I("(d%d)%s", con_lid, __func__);

    if (NULL != p_media_cb) {
        p_media_cb->ble_media_track_change_cb(con_lid);
    }
}

uint8_t aob_media_get_mic_state(uint8_t con_lid)
{
    AOB_MOBILE_INFO_T *p_info = NULL;

    p_info = ble_audio_earphone_info_get_mobile_info(con_lid);
    if (NULL != p_info) {
        return p_info->media_info.mic_mute;
    }
    return 0;
}

static void aob_media_mics_state_cb(uint8_t mute)
{
    uint8_t conidx = 0;
    AOB_MOBILE_INFO_T *p_info = NULL;

    conidx = app_audio_adm_get_le_audio_active_device();
    /// TODO:no active device
    if (BT_DEVICE_INVALID_ID == conidx) {
        LOG_E("%s no active device to set vol", __func__);
        conidx = 0;
    }


    LOG_I("(d%d)%s mute:%d", conidx, __func__, mute);

    p_info = ble_audio_earphone_info_get_mobile_info(conidx);
    if (NULL != p_info) {
        p_info->media_info.mic_mute = mute;
    }
    if (NULL != p_media_cb) {
        p_media_cb->ble_media_mic_state_cb(mute);
    }
}

static void aob_media_iso_link_quality_cb(void *event)
{
    if (NULL != p_media_cb->ble_media_iso_link_quality_cb)
    {
        p_media_cb->ble_media_iso_link_quality_cb(event);
    }
}

static void aob_media_pacs_cccd_written_cb(uint8_t con_lid)
{
    if (NULL != p_media_cb->ble_media_pacs_cccd_written_cb)
    {
        p_media_cb->ble_media_pacs_cccd_written_cb(con_lid);
    }
}

static void aob_media_ase_codec_cfg_req_handler_cb(uint8_t con_lid, app_bap_ascs_ase_t *p_ase_info, uint8_t tgt_latency,
                                                    const app_gaf_codec_id_t *codec_id, app_bap_cfg_t *codec_cfg)
{
    //Check is there an valid ase for codec cfg
    if (NULL == p_ase_info)
    {
        LOG_W("%s ava_ase_info error!", __func__);
        return ;
    }

    app_bap_qos_req_t ntf_qos_req;
    bool accept = false;
    // check qos req fill callback function is registed
    if (aob_ascs_srv_codec_req_handler_cb)
    {
        // Fill Qos req using callback function, can modify codec_cfg in callback
        accept = aob_ascs_srv_codec_req_handler_cb(p_ase_info->direction, codec_id, tgt_latency, codec_cfg, &ntf_qos_req);
        // If accept, fill local ase info
        if (accept)
        {
            // Customer may fill params in this callback
            p_media_cb->ble_ase_codec_cfg_req_cb(p_ase_info->ase_lid, (const AOB_CODEC_ID_T*)codec_id,
                                                 tgt_latency, (AOB_BAP_CFG_T *)codec_cfg, (AOB_BAP_QOS_REQ_T *)&ntf_qos_req);
            // Record con_lid and codec id
            p_ase_info->con_lid = con_lid;
            memcpy(&p_ase_info->codec_id, codec_id, sizeof(app_gaf_codec_id_t));
            // Record codec cfg and qos req
            app_bap_uc_srv_store_codec_cfg_and_qos_req_for_ase(p_ase_info->ase_lid, codec_cfg, &ntf_qos_req);
        }
        // Send codec cfg cfm rsp
        app_bap_uc_srv_send_codec_cfg_rsp(accept, p_ase_info->ase_lid);
    }
    else
    {
        //Fill any param that not NULL
        app_bap_uc_srv_send_codec_cfg_rsp(false, p_ase_info->ase_lid);
    }
}


static void aob_ascs_ase_enable_req_handler_cb(uint8_t con_lid, uint8_t ase_lid, app_bap_cfg_metadata_t *context)
{

}

static void aob_ascs_ase_release_req_handler_cb(uint8_t con_lid, uint8_t ase_lid, app_bap_cfg_metadata_t *context)
{

}

static void aob_ascs_ase_update_metadata_req_handler_cb(uint8_t con_lid, uint8_t ase_lid, app_bap_cfg_metadata_t *context)
{

}

static media_event_handler_t media_event_cb = {
    .media_track_change_cb                  = aob_media_track_changed_cb,
    .media_stream_status_change_cb          = aob_media_stream_status_cb,
    .media_playback_status_change_cb        = aob_media_playback_status_cb,
    .media_mic_state_cb                     = aob_media_mics_state_cb,
    .media_iso_link_quality_cb              = aob_media_iso_link_quality_cb,
    .media_pacs_cccd_written_cb             = aob_media_pacs_cccd_written_cb,
    .ase_codec_cfg_req_handler_cb           = aob_media_ase_codec_cfg_req_handler_cb,
    .ase_enable_req_handler_cb              = aob_ascs_ase_enable_req_handler_cb,
    .ase_release_req_handler_cb             = aob_ascs_ase_release_req_handler_cb,
    .ase_update_metadata_req_handler_cb     = aob_ascs_ase_update_metadata_req_handler_cb,
};

void aob_media_ascs_register_codec_req_handler_cb(get_qos_req_cfg_info_cb cb_func)
{
    LOG_I("%s [old cb] = %p, [new cb] = %p", __func__, aob_ascs_srv_codec_req_handler_cb, cb_func);
    aob_ascs_srv_codec_req_handler_cb = cb_func;
}

void aob_media_api_init(void)
{
    AOB_MOBILE_INFO_T *p_mob_info = NULL;

    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++) {
        p_mob_info = ble_audio_earphone_info_get_mobile_info(i);
        if (p_mob_info) {
            uint8_t base_ase_lid = i * APP_BAP_DFT_ASCS_NB_ASE_CHAR;
            for (uint8_t instance = 0; instance < APP_BAP_DFT_ASCS_NB_ASE_CHAR; instance++) {
                p_mob_info->media_info.aob_media_ase_info[instance] = app_bap_uc_srv_get_ase_info(base_ase_lid + instance);
            }
        }
    }

    p_media_cb = ble_audio_get_evt_cb();
    aob_mgr_media_evt_handler_register(&media_event_cb);
}

#ifdef AOB_MOBILE_ENABLED

/****************************for client(mobile)*****************************/
/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/
static void aob_mobile_media_djob_timer_timeout(void const *para);

/************************private variable defination************************/

/****************************function defination****************************/
osTimerDef(aob_mobile_media_djob_timer, aob_mobile_media_djob_timer_timeout);
osTimerId   aob_mobile_media_djob_timer_id = NULL;
static AOB_MOBILE_MEDIA_ENV_T aob_mobile_media_env;

static void aob_mobile_media_djob_proc(uint8_t con_lid)
{
    uint8_t statusBf = 0;

    if (!aob_mobile_media_djob_timer_id) {
        aob_mobile_media_djob_timer_id =
            osTimerCreate(osTimer(aob_mobile_media_djob_timer), osTimerOnce, NULL);
    }

    LOG_I("%s statusBf:0x%x, %d", __func__, aob_mobile_media_env.djobStatusBf,
        aob_mobile_media_env.info[con_lid].delayCnt);

   statusBf = aob_mobile_media_env.djobStatusBf & (0x1 << con_lid);
    if ((statusBf) && (!(--aob_mobile_media_env.info[con_lid].delayCnt))) {
        aob_mobile_media_env.djobStatusBf &= ~(0x1 << con_lid);
        app_bt_call_func_in_bt_thread((uint32_t)&aob_mobile_media_env.info[con_lid].pCfgInfo, (uint32_t)con_lid,
            (uint32_t)aob_mobile_media_env.info[con_lid].biDirection, 0,
            (uint32_t)aob_media_mobile_start_stream);
    }

    if (!aob_mobile_media_env.djobStatusBf) {
        aob_mobile_media_env.djobTimerStarted = false;
        osTimerStop(aob_mobile_media_djob_timer_id);
        goto exit;
    }

    if (!aob_mobile_media_env.djobTimerStarted) {
        aob_mobile_media_env.djobTimerStarted = true;
        osTimerStart(aob_mobile_media_djob_timer_id, 500);
    }

exit:
    return;
}

static void aob_mobile_media_djob_timer_timeout(void const *para)
{
    aob_mobile_media_env.djobTimerStarted = false;

    for (uint8_t i= 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
        aob_mobile_media_djob_proc(i);
    }
}

void aob_media_mobile_update_metadata(uint8_t ase_lid, app_bap_cfg_metadata_t *meta_data)
{
    app_bap_uc_cli_update_metadata(ase_lid, meta_data);
}

void aob_media_mobile_start_ase_disvovery(uint8_t con_lid)
{
    app_bap_uc_cli_discovery_start(con_lid);
}

void aob_media_mobile_start_stream(AOB_MEDIA_ASE_CFG_INFO_T *pInfo, uint8_t con_lid, bool biDirection)
{
    if ((!pInfo) || (con_lid == INVALID_CONNECTION_INDEX)) {
        LOG_W("%s error detected,con_lid:%d", __func__, con_lid);
        return;
    }

    if (!app_bap_uc_cli_is_already_bonded(con_lid)) {
        LOG_W("%s ASCS not bond,con_lid:%d", __func__, con_lid);
        return;
    }

    for (uint32_t i = 0; i < APP_GAF_DIRECTION_MAX; i++) {
        uint8_t ase_instance_idx = 0;
        if (pInfo->direction == APP_GAF_DIRECTION_SRC) {
            ase_instance_idx += 2;
        } else if(pInfo->direction == APP_GAF_DIRECTION_SINK) {
           if ((pInfo->context_type == APP_BAP_CONTEXT_TYPE_CONVERSATIONAL) ||
            (pInfo->context_type == APP_BAP_CONTEXT_TYPE_GAME) ||
              (pInfo->context_type == APP_BAP_CONTEXT_TYPE_LIVE)) {
                ase_instance_idx++;
            }
        }
        uint8_t ase_lid = app_bap_uc_cli_get_ava_ase_lid_by_ase_instance_idx(con_lid, ase_instance_idx, true);
        LOG_I("%s, ase lid %d, direction %d", __func__, ase_lid, pInfo->direction);

        ble_ase_stm_t *p_ase_sm = ble_audio_find_ase_sm_by_ase_lid(ase_lid);
        if (!p_ase_sm) {
            p_ase_sm = ble_audio_ase_stm_alloc(con_lid, ase_lid);
            if (NULL != p_ase_sm) {
                ble_audio_ase_stm_startup(p_ase_sm);
                ble_audio_ase_stm_send_msg(p_ase_sm, BLE_REQ_ASE_CODEC_CONFIGURE, (uint32_t)pInfo, (uint32_t)biDirection);
            }

            if (!biDirection) {
                return;
            }
            pInfo++;
        }

        BLE_ASE_STATE_E state = ble_audio_ase_stm_get_cur_state(con_lid, ase_lid);
        switch (state) {
            case BLE_ASE_STATE_QOS_CONFIGUED:
                ble_audio_ase_stm_send_msg(p_ase_sm, BLE_REQ_ASE_ENABLE, (uint32_t)con_lid, 0);
                break;
            default:
                break;
        }
    }
    return;
}

void aob_media_mobile_release_stream(uint8_t ase_lid)
{
    ble_ase_stm_t *p_ase_sm = ble_audio_find_ase_sm_by_ase_lid(ase_lid);

    ble_audio_ase_stm_send_msg(p_ase_sm, BLE_REQ_ASE_RELEASE, (uint32_t)ase_lid, 0);
}

void aob_media_mobile_disable_stream(uint8_t ase_lid)
{
    ble_ase_stm_t *p_ase_sm = ble_audio_find_ase_sm_by_ase_lid(ase_lid);

    ble_audio_ase_stm_send_msg(p_ase_sm, BLE_REQ_ASE_DISABLE, (uint32_t)ase_lid, 0);
}

void aob_media_mobile_enable_stream(uint8_t ase_lid)
{
    ble_ase_stm_t *p_ase_sm = ble_audio_find_ase_sm_by_ase_lid(ase_lid);

    ble_audio_ase_stm_send_msg(p_ase_sm, BLE_REQ_ASE_ENABLE, (uint32_t)ase_lid, 0);
}

void aob_media_mobile_set_player_name(uint8_t media_lid, uint8_t *name, uint8_t name_len)
{
    app_acc_mcs_set_player_name_req(media_lid, name, name_len);
}

void aob_media_mobile_change_track(uint8_t media_lid, uint32_t duration, uint8_t *title, uint8_t title_len)
{
    app_acc_mcs_track_change_req(media_lid, duration, title, title_len);
}

uint8_t aob_media_mobile_get_cur_streaming_ase_lid(uint8_t con_lid, AOB_MGR_DIRECTION_E direction)
{
    return app_bap_uc_cli_get_streaming_ase_lid(con_lid, (enum app_gaf_direction)direction);
}

void aob_media_mobile_micc_set_mute(uint8_t con_lid, uint8_t mute)
{
    app_arc_micc_set_mute(con_lid, mute);
}

AOB_MGR_STREAM_STATE_E aob_media_mobile_get_cur_ase_state(uint8_t ase_lid)
{
    app_bap_ascc_ase_t *p_ascc_ase = app_bap_uc_cli_get_ase_info(ase_lid);

    return (p_ascc_ase != NULL) ? (AOB_MGR_STREAM_STATE_E)p_ascc_ase->ase_state : AOB_MGR_STREAM_STATE_MAX;
}

void aob_media_mobile_action_control(uint8_t media_lid, AOB_MGR_MC_OPCODE_E action)
{
    app_acc_mcs_action_req(media_lid, (uint8_t)action, 5, 0);
}

static void aob_media_mobile_capa_changed_cb(uint8_t con_lid, uint8_t type)
{
    TRACE(3, "%s con_lid %d added codec %d", __func__, con_lid, type);
}

static void aob_media_mobile_stream_status_cb(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state)
{
    LOG_D("%s con_lid %d state %d ase lid %d", __func__, con_lid, state, ase_lid);
}

static void aob_media_mobile_control_cb(uint8_t con_lid, AOB_MGR_MC_OPCODE_E opCode)
{
    TRACE(3, "%s con_lid %d opcode %d", __func__, con_lid, opCode);

    switch (opCode) {
        case AOB_MGR_MC_OP_PLAY:
        {
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                uint8_t ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_BAP_UC_ASE_STATE_QOS_CONFIGURED, APP_GAF_DIRECTION_SINK);
                ble_ase_stm_t *p_ase_sm = ble_audio_find_ase_sm_by_ase_lid(ase_lid);
                if (!p_ase_sm) {
                    AOB_MEDIA_ASE_CFG_INFO_T ase_to_start =
                    {
                        GAF_BAP_SAMPLE_FREQ_48000, 120, APP_GAF_DIRECTION_SINK, AOB_CODEC_ID_LC3, APP_BAP_CONTEXT_TYPE_MEDIA
                    };

                    aob_media_mobile_start_stream(&ase_to_start, i, false);
                } else {
                    aob_media_mobile_enable_stream(ase_lid);
                }
            }
        }
        break;
        case AOB_MGR_MC_OP_PAUSE:
        {
            uint8_t ase_lid = GAF_INVALID_LID;
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_BAP_UC_ASE_STATE_STREAMING, APP_GAF_DIRECTION_SINK);
                aob_media_mobile_disable_stream(ase_lid);
            }
        }
            break;
        case AOB_MGR_MC_OP_STOP:
        {
            uint8_t ase_lid = GAF_INVALID_LID;
            for (uint8_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_BAP_UC_ASE_STATE_STREAMING, APP_GAF_DIRECTION_SINK);
                aob_media_mobile_release_stream(ase_lid);
            }
        }
            break;
        default:
            break;
    }
}

static void aob_media_mobile_pac_found_cb(uint8_t con_lid)
{
    gaf_bap_activity_type_e type = app_bap_get_activity_type();

    if (type == GAF_BAP_ACT_TYPE_CIS_AUDIO)
    {
        aob_media_mobile_start_ase_disvovery(con_lid);
    }
    else if (type == GAF_BAP_ACT_TYPE_BIS_SHARE)
    {
#if (CFG_BAP_BC)
        app_bap_bc_assist_start(con_lid);
#endif
    }
}

static void aob_media_mobile_ase_found_cb(uint8_t con_lid, uint8_t nb_ases)
{
    TRACE(3, "%s con_lid %d %d ASEs found", __func__, con_lid, nb_ases);
}

static void aob_media_mobile_grp_state_change_cb(bool isCreate, uint8_t ase_lid, uint16_t status, uint8_t grp_lid)
{
    ble_ase_stm_t *p_ase_sm = ble_audio_find_ase_sm_by_grp_lid(grp_lid);

    if (isCreate) {
        ble_audio_ase_stm_send_msg(p_ase_sm, BLE_EVT_ASE_CIG_CREATED, grp_lid, 0);
    } else {
        ble_audio_ase_stm_send_msg(p_ase_sm, BLE_EVT_ASE_CIG_REMOVED, grp_lid, 0);
    }
}

static void aob_media_mobile_micc_state_cb(uint8_t mute)
{
    TRACE(3, "%s micc %d", __func__, mute);
}

static media_mobile_event_handler_t media_mobile_event_cb = {
    .media_stream_status_change_cb = aob_media_mobile_stream_status_cb,
    .media_codec_capa_change_cb = aob_media_mobile_capa_changed_cb,
    .media_control_cb = aob_media_mobile_control_cb,
    .media_pac_found_cb = aob_media_mobile_pac_found_cb,
    .media_ase_found_cb = aob_media_mobile_ase_found_cb,
    .media_cis_group_state_change_cb = aob_media_mobile_grp_state_change_cb,
    .media_mic_state_cb = aob_media_mobile_micc_state_cb,
};

void aob_media_mobile_api_init(void)
{
    aob_mgr_gaf_mobile_media_evt_handler_register(&media_mobile_event_cb);
}
#endif
