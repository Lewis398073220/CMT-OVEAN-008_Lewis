/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __BES_AOB_API_H__
#define __BES_AOB_API_H__
#include "ble_aob_common.h"
#include "nvrecord_extension.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#if BLE_AUDIO_ENABLED

/// ASE Direction, sync @see app_gaf_direction
typedef enum bes_gaf_direction
{
    /// Sink direction
    BES_GAF_DIRECTION_SINK = 0,
    /// Source direction
    BES_GAF_DIRECTION_SRC,

    BES_GAF_DIRECTION_MAX,
} bes_gaf_direction_t;


#ifdef AOB_MOBILE_ENABLED
void bes_ble_mobile_connect_failed(bool is_failed);

void bes_ble_audio_mobile_core_register_event_cb(BLE_AUD_MOB_CORE_EVT_CB_T cb);

void bes_ble_audio_mobile_conn_next_paired_dev(ble_bdaddr_t* bdaddr);

bool bes_ble_mobile_is_connect_failed(void);

uint8_t* bes_ble_audio_mobile_conn_get_connecting_dev(void);

#endif

bool bes_ble_audio_make_new_le_core_sm(uint8_t conidx, uint8_t *peer_bdaddr);

void bes_ble_audio_register_fill_eir_func(void);

void bes_ble_update_tws_nv_role(uint8_t role);

void bes_ble_bap_set_activity_type(gaf_bap_activity_type_e type);

bool bes_ble_gap_is_remote_mobile_connected(uint8_t *p_addr);

bool bes_ble_aob_conn_start_adv(bool br_edr_support, ble_bdaddr_t *peer_addr);

bool bes_ble_aob_conn_stop_adv(void);

void bes_ble_audio_mobile_req_disconnect(ble_bdaddr_t *addr);

bool bes_ble_audio_is_mobile_link_connected(ble_bdaddr_t *addr);

void bes_ble_audio_disconnect_all_connection(void);

bool bes_ble_audio_is_any_mobile_connnected(void);

bool bes_ble_audio_is_ux_mobile(void);

void bes_ble_mobile_start_connect(void);

uint8_t bes_ble_audio_get_mobile_lid_by_pub_address(uint8_t *pub_addr);

void bes_ble_audio_mobile_disconnect_device(uint8_t conidx);

bool bes_ble_aob_csip_is_use_custom_sirk(void);

void bes_ble_aob_gattc_rebuild_cache(GATTC_NV_SRV_ATTR_t *record);

void bes_ble_aob_csip_if_use_temporary_sirk();

void bes_ble_aob_csip_if_refresh_sirk(uint8_t *sirk);

bool bes_ble_aob_csip_sirk_already_refreshed();

bool bes_ble_aob_csip_if_get_sirk(uint8_t *sirk);

void bes_ble_aob_conn_dump_state_info(void);

void bes_ble_aob_bis_tws_sync_state_req(void);

void bes_ble_audio_core_register_event_cb(BLE_AUD_CORE_EVT_CB_T cb);

void bes_ble_aob_call_if_outgoing_dial(uint8_t conidx, uint8_t *uri, uint8_t uriLen);

ble_bdaddr_t *bes_ble_aob_conn_get_remote_address(uint8_t con_lid);

void bes_ble_aob_media_prev(uint8_t con_lid);

void bes_ble_aob_media_next(uint8_t con_lid);

void bes_ble_aob_call_if_terminate_call(uint8_t conidx, uint8_t call_id);

void bes_ble_aob_call_if_accept_call(uint8_t conidx, uint8_t call_id);

void bes_ble_aob_media_play(uint8_t con_lid);

void bes_ble_aob_media_pause(uint8_t con_lid);

void bes_ble_aob_vol_down(void);

void bes_ble_aob_vol_up(void);

uint8_t bes_ble_aob_convert_local_vol_to_le_vol(uint8_t bt_vol);

BLE_AUDIO_POLICY_CONFIG_T* bes_ble_audio_get_policy_config();

uint8_t bes_ble_audio_get_mobile_sm_index_by_addr(ble_bdaddr_t *addr);

void bes_ble_audio_sink_streaming_handle_event(uint8_t con_lid, uint8_t data,
                                                               bes_gaf_direction_t direction, app_ble_audio_event_t event);


void bes_ble_audio_dump_conn_state(void);

uint8_t bes_ble_audio_get_mobile_addr(uint8_t deviceId, uint8_t *addr);

int bes_ble_aob_ibrt_cmd_table_get(void **cmd_tbl, uint16_t *cmd_size);

uint8_t bes_ble_aob_get_call_id_by_conidx_and_type(uint8_t device_id, uint8_t call_state);

uint8_t bes_ble_aob_get_call_id_by_conidx(uint8_t device_id);

#endif /* BLE_AUDIO_ENABLED */

#ifdef __cplusplus
}
#endif
#endif /* BLE_HOST_SUPPORT */
#endif /* __BES_AOB_API_H__ */
