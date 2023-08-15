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

#ifndef __BLE_AUDIO_EARPHONE_INFO_H__
#define __BLE_AUDIO_EARPHONE_INFO_H__

/*****************************header include********************************/
#include "nvrecord_extension.h"
#include "ble_audio_define.h"
#include "app_ble_include.h"
#include "aob_call_info_define.h"
#include "aob_media_api.h"

/******************************macro defination*****************************/

#define BLE_AUDIO_CAPA_SINK_CONTEXT_BF        (APP_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_BAP_CONTEXT_TYPE_MEDIA |\
                                               APP_BAP_CONTEXT_TYPE_UNSPECIFIED | APP_BAP_CONTEXT_TYPE_LIVE | APP_BAP_CONTEXT_TYPE_GAME)
#define BLE_AUDIO_CAPA_SRC_CONTEXT_BF         (APP_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_BAP_CONTEXT_TYPE_MEDIA |\
                                               APP_BAP_CONTEXT_TYPE_MAN_MACHINE | APP_BAP_CONTEXT_TYPE_LIVE | APP_BAP_CONTEXT_TYPE_GAME)

/******************************type defination******************************/
typedef enum
{
    BLE_AUDIO_TWS_MASTER,
    BLE_AUDIO_TWS_SLAVE,
    BLE_AUDIO_MOBILE,
    BLE_AUDIO_BIS_SRC,
    BLE_AUDIO_ROLE_UNKNOW,
} BLE_AUDIO_TWS_ROLE_E;

typedef struct
{
    bool                        connected;
    uint8_t                     conidx;
    uint8_t                     volume;
    bool                        muted;
    bool                        nv_vol_invalid;
    BLE_ADDR_INFO_T             peer_ble_addr;

    // TODO: use AOB_CALL_INFO_T* to map the g_aob_call_scb_info.call_info
    AOB_CALL_ENV_INFO_T         call_env_info;
    AOB_MEDIA_INFO_T            media_info;
} AOB_MOBILE_INFO_T;

typedef struct
{
    /// Group local index
    uint8_t grp_lid;
    /// Broadcast ID earphone wants to recv
    uint8_t bcast_id[BAP_BC_BROADCAST_ID_LEN];
    /// Broadcast Code earphone use to decrypt
    uint8_t bcast_code[APP_GAP_KEY_LEN];
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Delegator source local index
    uint8_t src_lid;
    /// Stream position in group
    uint32_t stream_pos_bf;
    /// BIG info recv
    app_gaf_big_info_t group_info;
    /// Synced Stream(BIS) position bf
    uint32_t stream_sink_pos_bf;
    /// BIS hdl (synced bis)
    uint16_t bis_hdl[IAP_NB_STREAMS];

    uint32_t sink_sync_ref_offset;

    uint32_t sink_play_delay;
    /// BIS recving
    app_bap_bc_sink_audio_streaming_t *sink_audio_straming_info;
} AOB_BIS_GROUP_INFO_T;

typedef struct
{
    uint16_t frame_octet;
    uint8_t sampling_freq;
} AOB_BIS_STREAM_INFO_T;

typedef struct
{
    bool init_done;

    bool aob_enable_adv;

    BLE_AUDIO_TWS_ROLE_E nv_role;
    BLE_AUDIO_TWS_ROLE_E current_role;

    bool tws_connected;
    uint8_t tws_conidx;

    uint8_t local_ble_addr[BLE_ADDR_SIZE];
    uint8_t peer_ble_addr[BLE_ADDR_SIZE];
    uint8_t bis_src_ble_addr[BLE_ADDR_SIZE];

    uint8_t rsi[6];
    bool is_adv_rsi_refresh_need;
    AOB_MOBILE_INFO_T mobile_info[MOBILE_CONNECTION_MAX];

    AOB_BIS_GROUP_INFO_T bis_group_info;
    AOB_BIS_STREAM_INFO_T bis_stream_info;
} AOB_EARPHONE_INFO_T;

/****************************function declaration***************************/
#ifdef __cplusplus
extern "C" {
#endif

void ble_audio_earphone_info_init(void);

void ble_audio_tws_init(void);

bool ble_audio_is_ux_mobile(void);

bool ble_audio_is_ux_master(void);

bool ble_audio_is_ux_slave(void);

bool ble_audio_is_ux_bis_src(void);

void ble_audio_set_tws_nv_role(uint8_t role);

void ble_audio_update_tws_nv_role(uint8_t role);

void ble_audio_set_tws_nv_role_via_nv_addr(void);

uint8_t ble_audio_get_tws_nv_role(void);

void ble_audio_update_tws_current_role(uint8_t role);

uint8_t ble_audio_request_tws_current_role(void);

void ble_audio_set_tws_local_ble_addr(uint8_t *addr);

uint8_t *ble_audio_get_tws_local_ble_addr(void);

void ble_audio_set_tws_peer_ble_addr(const uint8_t *addr);

uint8_t *ble_audio_get_tws_peer_ble_addr(void);

void ble_audio_set_bis_src_ble_addr(const uint8_t *addr);

uint8_t *ble_audio_get_bis_src_ble_addr(void);

bool app_ble_audio_adv_enable(void);

bool app_ble_audio_adv_disable(void);

uint8_t ble_audio_get_tws_conidx(void);

void app_ble_audio_set_rsi_2_aob_earphone_info(uint8_t* rsi);

bool ble_audio_earphone_info_connected_set(uint8_t conidx, ble_bdaddr_t *mobile_addr);

bool ble_audio_earphone_info_disconnected_clear(uint8_t conidx);

void ble_audio_advData_prepare(BLE_ADV_PARAM_T *adv_param, uint8_t adv_flag);

void ble_audio_clear_adv_rsi_refresh_need_flag(void);

void ble_audio_get_rsi(uint8_t* rsi);

/**
 * @brief Set nv vol info flag valid for next read vol procedure
 *
 * @param[in] con_lid
 */
void ble_audio_earphone_info_cache_vol_to_nv(uint8_t con_lid);

/**
 * @brief Get available monbile info that has not been connected
 *
 * @return AOB_MOBILE_INFO_T*
 */
AOB_MOBILE_INFO_T *ble_audio_earphone_info_get_ava_mobile_info(void);

/**
 * @brief Get local mobile info according to the given connection index
 *
 * @param conidx        BLE connection index
 * @return AOB_MOBILE_INFO_T* Pointer of the mobile info
 */
AOB_MOBILE_INFO_T *ble_audio_earphone_info_get_mobile_info(uint8_t conidx);

/**
 * @brief Get a new call info according to the given connection index when call incoming or outgoing
 *
 * @param conidx        Connection index
 * @param call_id       Call index
 * @return AOB_SINGLE_CALL_INFO_T* Pointer of the call info
 */
AOB_SINGLE_CALL_INFO_T *ble_audio_earphone_info_make_call_info(uint8_t conidx, uint8_t call_id, uint8_t uriLen);

/**
* @brief Get the call index according to the given connection index and bearer index
*
* @param conidx        Connection index
* @param bearer_lid    Bearer local index
* @return uint8_t      Call index
*/
uint8_t ble_audio_bearer_id_get_call_id(uint8_t conidx, uint8_t bearer_lid);


/**
 * @brief Update the call info of given mobile connection
 *
 * @param conidx        BLE connection index
 * @param callInfo      Pointer of the call environment info
 */
void ble_audio_earphone_info_update_call_info(uint8_t conidx, AOB_CALL_ENV_INFO_T  *callInfo);

/**
 * @brief Clear the call info when call terminate
 *
 * @param conidx        BLE connection index
 * @param call_id       Call index
 */
void ble_audio_earphone_info_clear_call_info(uint8_t conidx, uint8_t call_id);

/**
 * @brief Set bear_id
 *
 * @param conidx        BLE connection index
 * @param call_id       Call index
 * @param bearer_lid     Bearer local index
 */
bool ble_audio_earphone_info_set_bearer_lid(uint8_t conidx, uint8_t call_id, uint8_t bearer_lid);

/**
 * @brief Get the media info according to the given connection index
 *
 * @param[in] con_lid        Connection local index
 * @return AOB_MEDIA_INFO_T* Pointer of the media info
 */
AOB_MEDIA_INFO_T *ble_audio_earphone_info_get_media_info(uint8_t con_lid);

/**
 * @brief Get the bis group info
 *
 * @return AOB_BIS_GROUP_INFO_T* Pointer of the group info
 */
AOB_BIS_GROUP_INFO_T *ble_audio_earphone_info_get_bis_group_info(void);

/**
 * @brief Get the bis stream info
 *
 * @return AOB_BIS_STREAM_INFO_T* Pointer of the stream info
 */
AOB_BIS_STREAM_INFO_T *ble_audio_earphone_info_get_bis_stream_info(void);

/**
 * @brief Set bis grp_lid
 *
 * @param[in] grp_lid        Group local index
 */
void ble_audio_earphone_info_set_bis_grp_lid(uint8_t grp_lid);

/**
 * @brief Get bis grp_lid
 *
 * @return uint8_t
 */
uint8_t ble_audio_earphone_info_get_bis_grp_lid(void);

/**
 * @brief Set bis broadcast ID that earphone wants to recv
 *
 * @param[in] bcast_id  Broadcast ID
 */
void ble_audio_earphone_info_set_bis_bcast_id(uint8_t *bcast_id_p);
/**
 * @brief Get bis bcast ID that earphone wants to recv
 *
 * @return uint8_t*
 */
uint8_t *ble_audio_earphone_info_get_bis_bcast_id(void);

/**
 * @brief Set bis bcast_code for decrypt
 *
 * @param bcast_code_p
 */
void ble_audio_earphone_info_set_bis_bcast_code(uint8_t *bcast_code_p);

/**
 * @brief Get bis bcast_code for decrypt
 *
 */
uint8_t *ble_audio_earphone_info_get_bis_bcast_code(void);

/**
 * @brief Set bis pa_lid
 *
 * @param[in] pa_lid        Periodic Advertising local index
 */
void ble_audio_earphone_info_set_bis_pa_lid(uint8_t pa_lid);

/**
 * @brief Get bis pa_lid
 *
 * @return uint8_t
 */
uint8_t ble_audio_earphone_info_get_bis_pa_lid(void);

/**
 * @brief Get bis src_lid
 *
 * @return src_lid        Source local index
 */
uint8_t ble_audio_earphone_info_get_bis_src_lid(void);

/**
 * @brief Set bis src_lid
 *
 * @param[in] src_lid        Source local index
 */
void ble_audio_earphone_info_set_bis_src_lid(uint8_t src_lid);

/**
 * @brief Record bis stream position in BIG Group Air tansimitted
 *
 * @param[in] stream_pos_bf        Stream position in group
 */
void ble_audio_earphone_info_set_bis_stream_pos_bf(uint32_t stream_pos_bf);

/**
 * @brief Record bis stream sink position in Local Synced BIG
 *
 * @param stream_pos_bf     Stream position in group
 */
void ble_audio_earphone_info_set_bis_stream_sink_pos_bf(uint32_t stream_pos_bf);
/**
 * @brief Get bis sink ref offset
 *
 * @return uint32_t sink ref offset
 */
uint32_t ble_audio_earphone_info_get_sink_sync_ref_offset(void);

/**
 * @brief Set bis sink ref offset
 *
 * @param[in] sinktransdelay        sink ref offset, calculate by big info
 */
void ble_audio_earphone_info_update_sink_sync_ref_offset(uint32_t sink_audio_paly_delay);

/**
 * @brief Get bis sink paly delay
 *
 * @return uint32_t sink paly delay
 */
uint32_t ble_audio_earphone_info_get_sink_audio_paly_delay(void);

/**
 * @brief Set bis sink play delay
 *
 * @param[in] sinktransdelay        sink paly delay, seted by user
 */
void ble_audio_earphone_info_update_sink_audio_paly_delay(uint32_t sink_audio_paly_delay);

/**
 * @brief Update real time volume info
 *
 */
bool ble_audio_earphone_info_set_vol_info(uint8_t con_lid, uint8_t vol, bool muted);

/**
 * @brief Get the real time volume info
 *
 */
bool ble_audio_earphone_info_get_vol_info(uint8_t con_lid, uint8_t *vol, bool *muted);

void ble_audio_register_fill_eir_func(void);

uint8_t ble_audio_earphone_info_bis_stream_pos_2_stream_lid(uint8_t stream_pos);

AOB_SINGLE_CALL_INFO_T *ble_audio_earphone_info_find_call_info(uint8_t conidx, uint8_t call_id);

/**
 * @brief Get call environment
 *
 */
AOB_CALL_ENV_INFO_T * ble_audio_earphone_info_get_call_env_info(uint8_t con_lid);

uint8_t ble_audio_earphone_info_get_call_id_by_conidx(uint8_t conidx);

uint8_t ble_audio_earphone_info_get_calling_call_id_by_conidx(uint8_t conidx);

uint8_t ble_audio_earphone_info_get_incoming_call_id_by_conidx(uint8_t conidx);

uint8_t ble_audio_earphoe_info_get_call_id_by_conidx_and_type(uint8_t conidx, AOB_CALL_STATE_E call_state);

uint8_t ble_audio_earphone_info_get_another_valid_call_id(uint8_t call_id, AOB_CALL_ENV_INFO_T *call_info);
#ifdef __cplusplus
}
#endif

#endif


