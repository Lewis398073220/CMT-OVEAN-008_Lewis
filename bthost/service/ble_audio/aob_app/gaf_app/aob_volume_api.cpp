/**
 * @file aob_volume_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2021-06-30
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
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

/*****************************header include********************************/
#include "app.h"
#include "gapm_le.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "app_gaf_custom_api.h"
#include "app_gaf_define.h"
#include "app_gaf_dbg.h"
#include "aob_volume_api.h"
#include "audioflinger.h"
#include "ble_audio_core.h"
#include "app_tws_ctrl_thread.h"
#include "ble_audio_tws_cmd_handler.h"
#include "app_bt_sync.h"
#include "nvrecord_ble.h"
#include "aob_conn_api.h"
#include "app_audio_active_device_manager.h"
#include "hal_codec.h"
#include "aob_volume_api.h"
#include "app_tws_ibrt_conn_api.h"

#include "app_bt_func.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "arc_vcs.h"

#ifdef __cplusplus
}
#endif

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/
static void aob_tws_sync_volume_offset_status_handler();

/************************private variable defination************************/
static BLE_AUD_CORE_EVT_CB_T *p_volume_cb = NULL;
static app_gaf_arc_vcs_volume_ind_t volume_ind;
static app_gaf_arc_vcs_volume_ind_t volume_sync_info;
static app_gaf_arc_vocs_offset_ind_t p_volume_offset_ind;
static bool vol_synced = false;

/****************************function defination****************************/
app_gaf_arc_vcs_volume_ind_t* aob_volume_info_get(void)
{
    return &volume_ind;
}

void aob_vol_mute(void)
{
    app_arc_vcs_control(GAF_ARC_VC_OPCODE_VOL_MUTE, 0, 0);
}

void aob_vol_unmute(void)
{
    app_arc_vcs_control(GAF_ARC_VC_OPCODE_VOL_UNMUTE, 0, 0);
}

void aob_vol_up(void)
{
    app_arc_vcs_control(GAF_ARC_VC_OPCODE_VOL_UP_UNMUTE, 0, 0);
}

void aob_vol_down(void)
{
    app_arc_vcs_control(GAF_ARC_VC_OPCODE_VOL_DOWN_UNMUTE, 0, 0);
}

void aob_vol_set_abs(uint8_t vol)
{
    int8_t leVol = aob_convert_local_vol_to_le_vol(vol);
    app_arc_vcs_control(GAF_ARC_VC_OPCODE_VOL_SET_ABS, leVol, 0);
}

void aob_vol_set_local_volume(uint8_t local_vol)
{
    app_bt_stream_volumeset(local_vol);
}

void aob_vol_set_volume_offset(uint8_t output_lid, uint32_t value)
{
    app_arc_vocs_set(output_lid, GAF_ARC_VOC_SET_TYPE_OFFSET, value);
}

void aob_vol_set_audio_location(uint8_t output_lid, uint32_t value)
{
    app_arc_vocs_set(output_lid, GAF_ARC_VOC_SET_TYPE_LOCATION, value);
}

uint8_t aob_convert_local_vol_to_le_vol(uint8_t bt_vol)
{
    return unsigned_range_value_map(bt_vol, TGT_VOLUME_LEVEL_MUTE, TGT_VOLUME_LEVEL_MAX, 0, MAX_AOB_VOL);
}

uint8_t aob_convert_le_vol_to_local_vol(uint8_t le_vol)
{
    return unsigned_range_value_map(le_vol, 0, MAX_AOB_VOL, TGT_VOLUME_LEVEL_MUTE, TGT_VOLUME_LEVEL_MAX);
}

void aob_vol_store_volume_into_nv(uint8_t con_lid, uint8_t leVol)
{
    // update the volume info to BLE stack
    uint8_t *remote_addr = NULL;
    bool ret = app_ble_get_peer_solved_addr(con_lid, &remote_addr);
    LOG_I("(d%d)%s, vol:%d, ret:%d", con_lid, __func__, leVol, ret);
    nv_record_ble_write_volume_via_bdaddr(remote_addr, leVol);

    app_arc_vcs_update_info_req(ARC_VC_UPDATE_VOL_MUTE, leVol, false);
}

void aob_vol_send_notification(uint8_t con_lid, enum app_vc_char_type char_type)
{
    app_arc_vcs_send_ntf(con_lid, (uint8_t)char_type);
}

void aob_vol_update_volume_info(uint8_t vol, bool muted)
{
    uint8_t conidx = 0;
    uint8_t* remote_addr = NULL;

    conidx = app_audio_adm_get_le_audio_active_device();
    /// TODO:no active device
    if (BT_DEVICE_INVALID_ID == conidx) {
        LOG_E("%s no active device to set vol, use default!", __func__);
        conidx = 0;
    }

    /// Check if vol set into global value success
    bool status = ble_audio_earphone_info_set_vol_info(conidx, vol, muted);

    if (!status)
    {
        LOG_I("%s connection is not configued before vol is changed", __func__);
        return;
    }

    app_ble_get_peer_solved_addr(conidx, &remote_addr);
    if (!nv_record_ble_write_volume_via_bdaddr(remote_addr, vol))
    {
        LOG_E("%s peer solved addr null, nv vol invalid!", __func__);
        AOB_MOBILE_INFO_T *p_info = ble_audio_earphone_info_get_mobile_info(conidx);
        if (p_info)
        {
            /// Before this turn to false, get vol from @see ble_audio_earphone_info_get_vol_info
            p_info->nv_vol_invalid = true;
        }
        else
        {
            LOG_E("%s cann not get mobile %d info!", __func__, conidx);
        }
    }
}

static void aob_vocs_offset_changed_cb(int16_t offset, uint8_t output_lid)
{
    p_volume_offset_ind.offset = offset;
    aob_tws_sync_volume_offset_status_handler();

    if (NULL != p_volume_cb) {
        p_volume_cb->ble_vocs_offset_changed_cb(offset, output_lid);
    }
}

static void aob_vocs_bond_data_changed_cb(uint8_t output_lid, uint8_t cli_cfg_bf)
{
    TRACE(2, " %s app_arc vocs bond_data_ind output_lid= %d, cli_cfg_bf = %02x",
         __func__, output_lid, cli_cfg_bf);

    if (NULL != p_volume_cb) {
        p_volume_cb->ble_vocs_bond_data_changed_cb(output_lid, cli_cfg_bf);
    }
}

static void aob_vol_update_vol_info(app_gaf_arc_vcs_volume_ind_t *vol_info)
{
    uint8_t localVol = aob_convert_le_vol_to_local_vol(vol_info->volume);

    aob_vol_update_volume_info(vol_info->volume, vol_info->mute);
    localVol = vol_info->mute ? TGT_VOLUME_LEVEL_MUTE : localVol;
    LOG_I("%s vol:%d->%d, mute:%d", __func__, vol_info->volume, localVol, vol_info->mute);
    app_bt_stream_volumeset(localVol);

    app_arc_vcs_update_info_req(ARC_VC_UPDATE_VOL_MUTE, vol_info->volume, (bool)vol_info->mute);
}

static void aob_vol_start_sync_vol()
{
    uint8_t bt_role = app_tws_ibrt_get_local_tws_role();
    if (bt_role == BTIF_BCR_UNKNOWN)
    {
        aob_vol_update_vol_info(&volume_ind);
    }
    else if (bt_role == BTIF_BCR_MASTER)
    {
        vol_synced = true;
        if (!app_bt_sync_enable(
            APP_BT_SYNC_OP_VOLUME, sizeof(app_gaf_arc_vcs_volume_ind_t),(uint8_t*)&volume_ind,true))
        {
            vol_synced = false;
            //aob_vol_update_vol_info(&volume_ind);
        }
        else
        {
            volume_sync_info = volume_ind;
        }
    }
}

static void aob_tws_sync_volume_status_handler(void)
{
    aob_vol_update_vol_info(&volume_sync_info);
    if (!vol_synced)
    {
        vol_synced = false;
    }

    if ((volume_ind.volume != volume_sync_info.volume) || (volume_ind.mute != volume_sync_info.mute))
    {
        app_bt_call_func_in_bt_thread(0, 0, 0, 0, (uint32_t)aob_vol_start_sync_vol);
    }
}

APP_BT_SYNC_COMMAND_TO_ADD(APP_BT_SYNC_OP_VOLUME, aob_tws_sync_volume_status_handler, NULL);

void aob_vol_sync_volume_info_cb(uint8_t *buf, uint8_t len)
{
    volume_sync_info = *((app_gaf_arc_vcs_volume_ind_t *)buf);
    LOG_I("%s volume %d mute %d", __func__, volume_sync_info.volume, volume_sync_info.mute);
}

void aob_tws_sync_volume_offset_status_handler(void)
{
    int16_t leVol = 0;
    uint8_t localVol = 0;
    leVol = volume_ind.mute ? TGT_VOLUME_LEVEL_MUTE : (volume_ind.volume + p_volume_offset_ind.offset);
    if(leVol > MAX_AOB_VOL)
    {
        leVol = MAX_AOB_VOL;
    }
    if(leVol < MIN_AOB_VOL)
    {
        leVol = MIN_AOB_VOL;
    }
    localVol = aob_convert_le_vol_to_local_vol(leVol);

    aob_vol_update_volume_info(leVol, volume_ind.mute);
    /// configure codec
    app_bt_stream_volumeset(localVol);
}

APP_BT_SYNC_COMMAND_TO_ADD(APP_BT_SYNC_OP_VOLUME_OFFSET, aob_tws_sync_volume_offset_status_handler, NULL);

uint8_t aob_vol_get_real_time_volume(uint8_t con_lid)
{
    uint8_t *remote_addr = NULL;
    bool muted = false;

#ifdef CODEC_DAC_A2DP_VOLUME_128_LEVEL
    uint8_t default_vol = aob_convert_local_vol_to_le_vol(AUDIO_A2DP_OUTPUT_VOLUME_DEFAULT);
#else
    uint8_t default_vol = aob_convert_local_vol_to_le_vol(hal_codec_get_default_dac_volume_index());
#endif

    uint8_t leVol = default_vol;

    ble_audio_earphone_info_get_vol_info(con_lid, &leVol, &muted);

    if (muted) {
        LOG_W("%s,muted:%d", __func__, muted);
        return TGT_VOLUME_LEVEL_MUTE;
    }

    bool nv_invalid = false;
    AOB_MOBILE_INFO_T *mobile_info = ble_audio_earphone_info_get_mobile_info(con_lid);
    if (mobile_info)
    {
        nv_invalid = mobile_info->nv_vol_invalid;
    }
    if (!nv_invalid)
    {
        if (true == app_ble_get_peer_solved_addr(con_lid, &remote_addr))
        {
            nv_record_ble_read_volume_via_bdaddr((uint8_t *)remote_addr, &leVol);
        }
        else
        {
            LOG_W("%s: null address", __func__);
            return default_vol;
        }
    }

    app_arc_vcs_update_info_req(ARC_VC_UPDATE_VOL, leVol, false);

    LOG_I("%s vol:%d, dft vol:%d", __func__, leVol, default_vol);

    return leVol;
}

static void aob_vol_changed_cb(uint8_t con_lid, uint8_t volume, uint8_t mute, uint8_t reason)
{
    LOG_I("%s con_lid %d, volume %d, mute %d, reason %d, pre_sync_vol = %d",
                __func__, con_lid, volume, mute, reason, volume_sync_info.volume);
#ifndef __CUSTOMER_DEFINE_VCS_CONTROL__
    if ((volume_ind.volume != volume) || (volume_ind.mute != mute))
    {
        volume_ind.volume = volume;
        volume_ind.mute = mute;
#ifdef BLE_AOB_VOLUME_SYNC_ENABLED
        aob_vol_start_sync_vol();
#else
        aob_vol_update_vol_info(&volume_ind);
#endif
    }
#endif ///__CUSTOMER_DEFINE_VCS_CONTROL__

    if ((NULL != p_volume_cb) && (NULL != p_volume_cb->ble_vol_changed)) {
        p_volume_cb->ble_vol_changed(con_lid, volume, mute, reason);
    }
}

void aob_vol_vcs_bond_data_changed_cb(uint8_t con_lid, uint8_t char_type, uint8_t cfg_bf)
{
    if ((NULL != p_volume_cb) &&
        (NULL != p_volume_cb->ble_vcs_cccd_changed_cb)) {
        if (APP_VC_CHAR_TYPE_STATE == char_type) {
            uint8_t leVol = aob_vol_get_real_time_volume(con_lid);
            p_volume_cb->ble_vcs_cccd_changed_cb(con_lid, leVol, (bool)cfg_bf);
        }
    }
}

static vol_event_handler_t vol_event_cb = {
    .vol_changed_cb = aob_vol_changed_cb,
    .vcs_bond_data_changed_cb = aob_vol_vcs_bond_data_changed_cb,
    .vocs_offset_changed_cb = aob_vocs_offset_changed_cb,
    .vocs_bond_data_changed_cb = aob_vocs_bond_data_changed_cb,
};

void aob_vol_api_init(void)
{
    p_volume_cb = ble_audio_get_evt_cb();
    aob_mgr_gaf_vol_evt_handler_register(&vol_event_cb);
}

#ifdef AOB_MOBILE_ENABLED

/****************************for client(mobile)*****************************/
/*********************external function declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/

/****************************function defination****************************/
static void aob_mobile_vol_stream_volumeset_handler(int8_t vol)
{
    uint32_t ret;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    ret = af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg, false);
    if (ret == 0 && stream_cfg) {
        stream_cfg->vol = vol;
        af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, stream_cfg);
    }
}

void aob_mobile_vol_mute(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_MUTE, 0);
}

void aob_mobile_vol_unmute(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_UNMUTE, 0);
}

void aob_mobile_vol_up(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_UP, 0);
}

void aob_mobile_vol_down(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_DOWN, 0);
}

void aob_mobile_vol_set_abs(uint8_t con_lid, uint8_t vol)
{
    int8_t local_vol = aob_convert_local_vol_to_le_vol(vol);
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_SET_ABS, local_vol);
}

// Volume offset control api
void aob_mobile_vol_set_volume_offset(uint8_t con_lid, uint8_t output_lid, int16_t value)
{
    app_arc_vocc_control(con_lid, output_lid, GAF_ARC_VOC_SET_TYPE_OFFSET, value);
}

void aob_mobile_vol_set_audio_location(uint8_t con_lid, uint8_t output_lid, int16_t value)
{
    app_arc_vocc_control(con_lid, output_lid, GAF_ARC_VOC_SET_TYPE_LOCATION, value);
}

static void aob_mobile_vol_changed_cb(uint8_t con_lid, uint8_t volume, uint8_t mute, uint8_t reason)
{
    int8_t vol = 0;

    if (mute)
    {
        vol = TGT_VOLUME_LEVEL_MUTE;
    }
    else
    {
        vol = aob_convert_le_vol_to_local_vol(volume);
    }
    aob_mobile_vol_stream_volumeset_handler(vol);
}

static void aob_mobile_vocc_offset_changed_cb(uint8_t con_lid, int16_t value, uint8_t output_lid)
{
    // TODO:handle it.
    LOG_I("%s con_lid %d, value %d", __func__, con_lid, value);
}

static void aob_mobile_vocc_bond_data_changed_cb(uint8_t con_lid, uint8_t output_lid)
{
    TRACE(2, " %s app_arc vocc bond_data_ind con_lid = %d, output_id = %04x",
         __func__, con_lid, output_lid);
}

static vol_event_handler_t vol_mobile_event_cb = {
    .vol_changed_cb = aob_mobile_vol_changed_cb,
    .vocc_offset_changed_cb = aob_mobile_vocc_offset_changed_cb,
    .vocc_bond_data_changed_cb = aob_mobile_vocc_bond_data_changed_cb,
};

void aob_vol_mobile_api_init(void)
{
    aob_mgr_gaf_mobile_vol_evt_handler_register(&vol_mobile_event_cb);
}
#endif

/// @} AOB_APP
