/**
 * @file app_gaf_common.h
 * @author BES AI team
 * @version 0.1
 * @date 2021-06-22
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
 * @addtogroup APP_GAF
 * @{
 ****************************************************************************************
 */
/**
 * NOTE: This header file defines the common used module within GAF_core layer
 */

#ifndef __APP_GAF_COMMON_H__
#define __APP_GAF_COMMON_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include <stdint.h>
#include <stdbool.h>       // standard boolean definitions

#include "compiler.h"
/******************************macro defination*****************************/

#define APP_CO_BIT(pos) (1UL<<(pos))
/// Length of GAP Key
#define APP_GAP_KEY_LEN         (16)
/// Length of Device Address
#define APP_GAP_BD_ADDR_LEN     (6)
/// Default Preffered MTU
#define APP_GAF_DFT_PREF_MTU    (128)
/// Length of Codec ID value
#define APP_GAF_CODEC_ID_LEN    (5)

/******************************type defination******************************/

/// ASE Direction
typedef enum app_gaf_direction
{
    /// Sink direction
    APP_GAF_DIRECTION_SINK = 0,
    /// Source direction
    APP_GAF_DIRECTION_SRC,

    APP_GAF_DIRECTION_MAX,
} app_gaf_direction_t;

/// Direction requirements bit field
enum app_gaf_direction_bf
{
    /// Required for sink direction
    APP_GAF_DIRECTION_BF_SINK_POS = 0,
    APP_GAF_DIRECTION_BF_SINK_BIT = 0x01,
    /// Required for source direction
    APP_GAF_DIRECTION_BF_SRC_POS = 1,
    APP_GAF_DIRECTION_BF_SRC_BIT = 0x02,
    /// Required for both directions
    APP_GAF_DIRECTION_BF_BOTH = APP_GAF_DIRECTION_BF_SRC_BIT + APP_GAF_DIRECTION_BF_SINK_BIT,
};

typedef enum app_gaf_stream_context_state
{
    APP_GAF_CONTEXT_STREAM_STARTED = 0,

    APP_GAF_CONTEXT_SINGLE_STREAM_STOPPED = 1,
    APP_GAF_CONTEXT_ALL_STREAMS_STOPPED = 2,

} app_gaf_stream_context_state_t;

/// Codec Identifier
typedef struct app_gaf_codec_id
{
    /// Codec ID value
    uint8_t codec_id[APP_GAF_CODEC_ID_LEN];
} app_gaf_codec_id_t;

/// Periodic advertising address information
typedef struct app_gaf_per_adv_bdaddr
{
    /// BD Address of device
    uint8_t addr[APP_GAP_BD_ADDR_LEN];
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
    /// Advertising SID
    uint8_t adv_sid;
} app_gaf_per_adv_bdaddr_t;


/// Advertising identification structure
typedef app_gaf_per_adv_bdaddr_t app_bap_adv_id_t;

/// Extended advertising information
typedef struct
{
    // Device Address
    app_bap_adv_id_t        adv_id;
    // Length of complete extend advertising data
    uint8_t                     length;
    // Complete extend advertising data containing the complete Broadcast Audio Announcement
    uint8_t                     data[251];
} app_gaf_extend_adv_report_t;

/// Data value in LTV format
typedef struct app_gaf_ltv
{
    /// Length of data value
    uint8_t len;
    /// Data value
    uint8_t data[__ARRAY_EMPTY];
} app_gaf_ltv_t;

/// Broadcast code used for stream encryption
typedef struct app_gaf_bc_code_t
{
    /// Broadcast Code value
    uint8_t bcast_code[APP_GAP_KEY_LEN];
} app_gaf_bc_code_t;


/******************************macro defination*****************************/

/****************************function declaration***************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __APP_GAF_COMMON_H__ */

/// @} APP_GAF
