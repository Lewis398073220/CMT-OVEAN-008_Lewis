/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
/**
 ****************************************************************************************
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_BAP_STREAM_H_
#define APP_BAP_STREAM_H_

/*****************************header include********************************/
#include "gaf_media_stream.h"
#include "audioflinger.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
typedef uint8_t (*iso_send_data_func)(uint8_t **payload, uint16_t payload_len, uint32_t ref_time);
typedef bool (*iso_rev_data_func)(uint8_t channel, uint8_t *buf, uint32_t *len, void *iso_buffer);

/// HCI ISO_Data_Load - Packet Status Flag
typedef enum
{
    /// Valid data. The complete ISO_SDU was received correctly
    BAP_ISO_PKT_STATUS_VALID   = (0),
    /// Possibly invalid data. The contents of the ISO_SDU may contain errors or part of the ISO_SDU may
    /// be missing. This is reported as "data with possible errors".
    BAP_ISO_PKT_STATUS_INVALID = (1),
    /// Part(s) of the ISO_SDU were not received correctly. This is reported as "lost data".
    BAP_ISO_PKT_STATUS_LOST    = (2),
} BAP_ISO_PKT_STATUS_E;

/// BAP STREAM TYPE
typedef enum
{
    BAP_STREAM_CIS = (0),
    BAP_STREAM_BIS = (1),
} BAP_STREAM_TYPE_E;

typedef struct
{
    uint8_t stream_type_flag;
    uint8_t stream_type_flag_has_set;
    /// audio playback data buff
    uint8_t *playback_data_buf;
    /// audio capture data buff
    uint8_t *capture_data_buf;
} gaf_bis_src_stream_buf_t;

typedef struct
{
    bool enable;
    uint16_t conhdl;
    uint32_t location;
    uint8_t direction;
} gaf_bis_src_stream_info_t;

#ifdef __cplusplus
extern "C" {
#endif

void gaf_bis_src_audio_stream_stop_handler(uint8_t grp_lid);
void gaf_bis_src_audio_stream_start_handler(uint8_t grp_lid);
void gaf_bis_src_audio_stream_init(void);

void gaf_bis_audio_stream_stop_handler(uint8_t grp_lid);
void gaf_bis_audio_stream_start_handler(uint8_t grp_lid);
void gaf_bis_audio_stream_init(void);

#ifdef __cplusplus
}
#endif

#endif // APP_BAP_STREAM_H_

/// @} APP_BAP
