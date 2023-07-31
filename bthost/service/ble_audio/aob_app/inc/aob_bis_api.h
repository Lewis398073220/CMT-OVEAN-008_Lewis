/**
 * @file aob_bis_api.h
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

#ifndef __AOB_BIS_API_H__
#define __AOB_BIS_API_H__

#ifdef __cplusplus
extern "C" {
#endif
/*****************************header include********************************/
#include "ble_audio_define.h"
#include "aob_mgr_gaf_evt.h"

#define AOB_BIS_TWS_SYNC_ENABLED 1
/******************************macro defination*****************************/

/******************************type defination******************************/
typedef struct
{
    uint16_t sampling_freq;
    uint16_t frame_octet;
} AOB_BIS_MEDIA_INFO_T;

/****************************function declaration***************************/

/*BAP Broadcast Source APIs*/
/**
 ****************************************************************************************
 * @brief Set BIG encrypt configuration.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] is_encrypted     0:Not encrypted, !=0:encrypted
 * @param[in] bcast_code       Broadcast Code, @see app_gaf_bc_code_t, only meaningful when is_encrypted != 0
 *
 ****************************************************************************************
 */
void aob_bis_src_set_encrypt(uint8_t big_idx, uint8_t *bcast_code);

/**
 ****************************************************************************************
 * @brief Set Codec configuration of specific BIS stream.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] stream_lid       Stream local index
 * @param[in] frame_octet      Length of a codec frame in octets
 * @param[in] sampling_freq    Sampling Frequency (see #bap_sampling_freq enumeration)
 *
 ****************************************************************************************
 */
void aob_bis_src_set_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, uint16_t frame_octet, uint8_t sampling_freq);

/**
 ****************************************************************************************
 * @brief Enable Periodic Advertising for a Broadcast Group.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 *
 ****************************************************************************************
 */
void aob_bis_src_enable_pa(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief Disable Periodic Advertising for a Broadcast Group.
 *
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
void aob_bis_src_disable_pa(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Enable a Broadcast Group.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 *
 ****************************************************************************************
 */
void aob_bis_src_enable(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief Disable a Broadcast Group.
 *
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
void aob_bis_src_disable(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Add a BIG to BIS source.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 *
 ****************************************************************************************
 */
void aob_bis_src_add_group_req(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief Start src steaming
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] stream_lid_bf    Stream local index bit field indicating for which stream streaming must be started
 *                             0xFFFFFFFF means that streaming must be started for all BISes
 *
 ****************************************************************************************
 */
void aob_bis_src_start_streaming(uint8_t big_idx, uint32_t stream_lid_bf);

/**
 ****************************************************************************************
 * @brief Update metadeta request
 *
 * @param[in] grp_lid          BIS Group(BIG) local index
 * @param[in] sgrp_lid         BIS SubGroup local index
 * @param[in] metadata         Metadata for Codec Configuration see @app_bap_cfg_metadata_t
 *
 ****************************************************************************************
 */
void aob_bis_src_update_metadata(uint8_t grp_lid, uint8_t sgrp_lid, app_bap_cfg_metadata_t* metadata);

/**
 ****************************************************************************************
 * @brief Stop src steaming
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] stream_lid_bf    Stream local index bit field indicating for which stream streaming must be stopped
 *                             0xFFFFFFFF means that streaming must be stopped for all BISes
 *
 ****************************************************************************************
 */
void aob_bis_src_stop_streaming(uint8_t big_idx, uint32_t stream_lid_bf);

void aob_bis_src_mobile_api_init(void);

/*BAP Broadcast Sink APIs*/
/**
 ****************************************************************************************
 * @brief Enable a group of sink streams.
 *
 * @param[in]  pa_lid          Periodic Advertising local index
 * @param[in]  mse             Maximum number of subevents the controller should use to receive data payloads in each interval
 * @param[in]  stream_pos_bf   Stream position bit field indicating streams to synchronize with.
 * @param[in]  timeout_10ms    Timeout duration (10ms unit) before considering synchronization lost (Range 100 ms to 163.84 s).
 * @param[in]  encrypted       Indicate if streams are encrypted (!= 0) or not
 * @param[in]  bcast_code      Broadcast Code value
 * @param[in]  bcast_id        Broadcast ID
 *
 ****************************************************************************************
 */
void aob_bis_sink_enable(uint8_t pa_lid, uint8_t mse, uint8_t stream_pos_bf,
                              uint16_t timeout_10ms, uint8_t encrypted, uint8_t *bcast_code,
                              uint8_t *bcast_id);

/**
 ****************************************************************************************
 * @brief Disable a group of sink streams.
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
void aob_bis_sink_disable(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Media start sink scan.
 *
 ****************************************************************************************
 */
void aob_bis_sink_start_scan(void);

/**
 ****************************************************************************************
 * @brief Start periodic advertising synchronize.
 *          Expected callback event: APP_GAF_SCAN_PA_ESTABLISHED_IND.
 * @param[in] addr         BD Address of device
 * @param[in] addr_type    Address type of the device 0=public/1=private random
 * @param[in] adv_sid      Advertising SID
 ****************************************************************************************
 */
void aob_bis_sink_scan_pa_sync(uint8_t *addr, uint8_t addr_type, uint8_t adv_sid);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize.
 *          Expected callback event: APP_GAF_SCAN_PA_TERMINATED_IND.
 * @param[in] pa_lid         Periodic advertising local index
 *
 ****************************************************************************************
 */
void aob_bis_sink_scan_pa_terminate(uint8_t pa_lid);

/**
 ****************************************************************************************
 * @brief Start one of a group of sink streams.
 *          Expected callback event: APP_GAF_SINK_BIS_STREAM_STARTED_IND.
 * @param[in] grp_lid         Group local index
 * @param[in] stream_pos_bf   Stream position bit field indicating streams to synchronize with
 * @param[in] codec_type      Codec ID value
 * @param[in] media_info      @see AOB_BIS_MEDIA_INFO_T
 ****************************************************************************************
 */
void aob_bis_sink_start_streaming(uint8_t grp_lid, uint32_t stream_pos_bf,
                                                        uint8_t codec_type, AOB_BIS_MEDIA_INFO_T *media_info);

/**
 ****************************************************************************************
 * @brief Stop one of a group of sink streams.
 *          Expected callback event: APP_GAF_SINK_BIS_STREAM_STOPPED_IND.
 * @param[in] grp_lid         Sink streams group local index
 * @param[in] stream_pos      Stream position in group
 *
 ****************************************************************************************
 */
void aob_bis_sink_stop_streaming(uint8_t grp_lid, uint8_t stream_pos);

/**
 ****************************************************************************************
 * @brief Init sink status cb
 *
 ****************************************************************************************
 */
void aob_bis_sink_api_init(void);

/*BAP Broadcast Scan APIs*/
/**
 ****************************************************************************************
 * @brief Media set scan param.
 *
 * @param[in] intv_1m_slot      Scan interval for LE 1M PHY in multiple of 0.625ms - Must be higher than 2.5ms
 * @param[in] intv_coded_slot   Scan interval for LE Codec PHY in multiple of 0.625ms - Must be higher than 2.5ms
 * @param[in] wd_1m_slot        Scan window for LE 1M PHY in multiple of 0.625ms - Must be higher than 2.5ms
 * @param[in] wd_coded_slot     Scan window for LE Codec PHY in multiple of 0.625ms - Must be higher than 2.5ms
 ****************************************************************************************
 */
void aob_bis_scan_set_scan_param(uint16_t scan_timeout_s, uint16_t intv_1m_slot,
        uint16_t intv_coded_slot, uint16_t wd_1m_slot, uint16_t wd_coded_slot);

/**
 ****************************************************************************************
 * @brief Media stop scan.
 *
 ****************************************************************************************
 */
void aob_bis_sink_stop_scan(void);

/**
 ****************************************************************************************
 * @brief Send TWS sync bis state request
 *
 ****************************************************************************************
 */
void aob_bis_tws_sync_state_req(void);

/**
 ****************************************************************************************
 * @brief Process bis sync requests
 *
 ****************************************************************************************
 */
void aob_bis_tws_sync_state_req_handler(uint8_t *buf);

/**
 ****************************************************************************************
 * @brief Init scan status cb
 *
 ****************************************************************************************
 */
void aob_bis_scan_api_init(void);

/*BAP Broadcast Deleg APIs*/
/**
 ****************************************************************************************
 * @brief Start solicite advertising.
 *          Expected callback event: APP_GAF_DELEG_SOLICITE_STARTED_IND.
 *
 * @param[in] timeout_s         Timeout duration of adv, Unit:s
 * @param[in] context_bf        Available audio contexts bit field in adv data, , @see enum gaf_bap_context_type_bf
 *
 ****************************************************************************************
 */
void aob_bis_deleg_start_solicite(uint16_t timeout_s, uint32_t context_bf);

/**
 ****************************************************************************************
 * @brief Stop solicite advertising.
 *          Expected callback event: APP_GAF_DELEG_SOLICITE_STOPPED_IND.
 *
 ****************************************************************************************
 */
void aob_bis_deleg_stop_solicite(void);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize req.
 *          Expected callback event: BAP_BC_SCAN_PA_SYNCHRONIZE_RI.
 * @param[in] pa_lid         Periodic advertising local index
 * @param[in] accept         false: not accept, true: accept
 *
 ****************************************************************************************
 */
void aob_bis_deleg_pa_sync_ri(uint8_t pa_lid, bool accept);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize req.
 *          Expected callback event: BAP_BC_SCAN_PA_TERMINATE_RI.
 * @param[in] pa_lid         Periodic advertising local index
 * @param[in] accept         false: not accept, true: accept
 *
 ****************************************************************************************
 */
void aob_bis_deleg_pa_terminate_ri(uint8_t pa_lid, bool accept);

void aob_bis_deleg_api_init(void);

/*BAP Broadcast Assist APIs*/
/**
 ****************************************************************************************
 * @brief Media start assitant scan.
 *
 * @param[in] con_lid         Connection local index
 ****************************************************************************************
 */
void aob_bis_assist_scan_bc_src(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Start scanning for solicitation.
 *          Expected callback event: APP_GAF_SCAN_REPORT_IND and APP_GAF_SCAN_PA_ESTABLISHED_IND.
 * @param[in] timeout_s         Scanning time duration, Unit:s
 *
 ****************************************************************************************
 */
void aob_bis_assist_start_scan(uint16_t timeout_s);

/**
 ****************************************************************************************
 * @brief Stop scanning for solicitation.
 *          Expected callback event: APP_GAF_SCAN_STOPPED_IND.
 *
 ****************************************************************************************
 */
void aob_bis_assist_stop_scan(void);

/**
 ****************************************************************************************
 * @brief Assist finds BASS service of the peer device
 *
 * @param[in] con_lid         Connection local index
 ****************************************************************************************
 */
void aob_bis_assist_discovery(uint8_t con_lid);

void aob_bis_assist_mobile_api_init(void);

#ifdef __cplusplus
}
#endif

#endif
