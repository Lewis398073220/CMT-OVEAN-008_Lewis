/**
 ****************************************************************************************
 *
 * @file app_bap_bc_sink_msg.h
 *
 * @brief BLE Audio Broadcast Sink
 *
 * Copyright 2015-2019 BES.
 *
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
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_BAP_BC_SINK_MSG_H_
#define APP_BAP_BC_SINK_MSG_H_
#if BLE_AUDIO_ENABLED
#include "app_gaf.h"
#include "bap_bc.h"

#define APP_BAP_DFT_BC_SINK_DP_ID                  0
#define APP_BAP_DFT_BC_SINK_CTL_DELAY_US           0x0102
#define APP_BAP_DFT_BC_SINK_TIMEOUT_10MS           100
#define APP_BAP_DFT_BC_SINK_MSE                    5

/// Structure for BAP_BC_SINK_ENABLE
typedef struct app_bap_bc_sink_enable
{
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Broadcast ID
    bap_bcast_id_t       bcast_id;
    /// Maximum number of subevents the controller should use to receive data payloads in each interval
    uint8_t              mse;
    /// Stream position bit field indicating streams to synchronize with.
    uint32_t             stream_pos_bf;
    /// Timeout duration (10ms unit) before considering synchronization lost (Range 100 ms to 163.84 s).
    uint16_t             timeout_10ms;
    /// Indicate if streams are encrypted (!= 0) or not
    uint8_t              encrypted;
    /// Broadcast code. Meaningful only if encrypted parameter indicates that streams are encrypted
    app_gaf_bc_code_t bcast_code;
} app_bap_bc_sink_enable_t;

/// Start Sink Streaming
typedef struct app_bap_bc_sink_audio_streaming
{
    /// Group local index
    uint8_t             grp_lid;
    /// Position of the stream in the group (range 1 to 32)
    uint8_t             stream_pos;
    /// Codec ID value
    app_gaf_codec_id_t  codec_id;
    /// Length of Codec Configuration value - in bytes
    uint8_t             cfg_len;
    /// Codec Configuration - to be casted as bap_lc3_cfg_t for LC3 codec.
    /// Array of bytes - 32-bit aligned to be casted as a SW structure
    uint32_t            cfg[0];
}app_bap_bc_sink_audio_streaming_t;

#ifdef __cplusplus
extern "C" {
#endif

uint32_t app_bap_bc_sink_cmp_evt_handler(void const *param);
uint32_t app_bap_bc_sink_ind_handler(void const *param);
uint32_t app_bap_bc_sink_req_ind_handler(void const *param);
uint8_t app_bap_bc_sink_get_stream_pos(uint32_t stream_pos_bf);
uint8_t app_bap_bc_sink_get_stream_lid(uint32_t stream_pos_bf);

void app_bap_bc_sink_init(void);

#ifdef __cplusplus
}
#endif


#endif
#endif // APP_BAP_BC_SINK_MSG_H_

/// @} APP_BAP
