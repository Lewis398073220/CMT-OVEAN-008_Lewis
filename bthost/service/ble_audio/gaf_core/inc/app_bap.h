/**
 ****************************************************************************************
 *
 * @file app_bap.h
 *
 * @brief BLE Audio Basic Audio Profile
 *
 * Copyright 2015-2021 BES.
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

#ifndef APP_BAP_H_
#define APP_BAP_H_

#if BLE_AUDIO_ENABLED
#include "app_gaf_common.h"
#include "ble_aob_common.h"


#define APP_EXT_ADV_DATA_MAX_LEN                  229 // HCI:7.7.65.13
#define APP_PER_ADV_DATA_MAX_LEN                  247 // 248 // HCI:7.7.65.16

/// LTV structure format
enum app_bap_ltv_fmt
{
    /// Length
    APP_BAP_LTV_LENGTH_POS = 0,
    /// Type
    APP_BAP_LTV_TYPE_POS,
    /// Value
    APP_BAP_LTV_VALUE_POS,

    /// Minimal length of LTV structure
    APP_BAP_LTV_LENGTH_MIN = 1,
};

/// Codec Specific Capabilities Types values
enum app_bap_capa_type
{
    /// Supported Sampling Frequencies
    APP_BAP_CAPA_TYPE_SAMP_FREQ = 1,
    /// Supported Frame Durations
    APP_BAP_CAPA_TYPE_FRAME_DUR,
    /// Audio Channel Counts
    APP_BAP_CAPA_TYPE_CHNL_CNT,
    /// Supported Octets per Codec Frame
    APP_BAP_CAPA_TYPE_OCTETS_FRAME,
    /// Maximum Supported Codec Frames per SDU
    APP_BAP_CAPA_TYPE_FRAMES_SDU,
};

/// Metadata Types values
enum app_bap_metadata_type
{
    /// Preferred Audio Contexts
    APP_BAP_METADATA_TYPE_PREF_CONTEXTS = 1,
    /// Streaming Audio Contexts
    APP_BAP_METADATA_TYPE_STREAM_CONTEXTS,

    /// Vendor Specific
    APP_BAP_METADATA_TYPE_VENDOR = 0xFF,
};

/// Minimal value of length field for Codec Specific Capabilities LTV structure
enum app_bap_capa_length
{
    /// Supported Sampling Frequencies
    APP_BAP_CAPA_LENGTH_SAMP_FREQ = 3,
    /// Supported Frame Durations
    APP_BAP_CAPA_LENGTH_FRAME_DUR = 2,
    /// Audio Channel Counts
    APP_BAP_CAPA_LENGTH_CHNL_CNT = 2,
    /// Supported Octets per Codec Frame
    APP_BAP_CAPA_LENGTH_OCTETS_FRAME = 5,
    /// Maximum Supported Codec Frames per SDU
    APP_BAP_CAPA_LENGTH_FRAMES_SDU = 2,
};

/// Minimal value of length field for Metadata LTV structure
enum app_bap_metadata_length
{
    /// Preferred Audio Contexts
    APP_BAP_METADATA_LENGTH_PREF_CONTEXTS = 3,
    /// Streaming Audio Contexts
    APP_BAP_METADATA_LENGTH_STREAM_CONTEXTS = 3,
};

/// Codec Specific Configuration Types values
enum app_bap_cfg_type
{
    /// Sampling Frequencies
    APP_BAP_CFG_TYPE_SAMP_FREQ = 1,
    /// Frame Duration
    APP_BAP_CFG_TYPE_FRAME_DUR,
    /// Audio Channel Allocation
    APP_BAP_CFG_TYPE_CHNL_LOCATION,
    /// Octets per Codec Frame
    APP_BAP_CFG_TYPE_OCTETS_FRAME,
    /// Codec Frame Blocks Per SDU
    APP_BAP_CFG_TYPE_FRAMES_SDU,
};

/// Sampling_Frequency
typedef enum gaf_bap_sampling_freq
{
    GAF_BAP_SAMPLE_FREQ_8000    = 0x01,
    GAF_BAP_SAMPLE_FREQ_11025,
    GAF_BAP_SAMPLE_FREQ_16000,
    GAF_BAP_SAMPLE_FREQ_22050,
    GAF_BAP_SAMPLE_FREQ_24000,
    GAF_BAP_SAMPLE_FREQ_32000,
    GAF_BAP_SAMPLE_FREQ_44100,
    GAF_BAP_SAMPLE_FREQ_48000,
    GAF_BAP_SAMPLE_FREQ_88200,
    GAF_BAP_SAMPLE_FREQ_96000,
    GAF_BAP_SAMPLE_FREQ_176400,
    GAF_BAP_SAMPLE_FREQ_192000,
    GAF_BAP_SAMPLE_FREQ_384000,

    GAF_BAP_SAMPLE_FREQ_MAX,
} GAF_BAP_SAMLLING_REQ_T;

// Frame_Duration   #app_bap_frame_dur
enum gaf_bap_frame_duration
{
    GAF_BAP_FRAME_DURATION_7_5MS    = 0x00,
    GAF_BAP_FRAME_DURATION_10MS     = 0x01,
    GAF_BAP_FRAME_DURATION_5MS      = 0x02,
    GAF_BAP_FRAME_DURATION_2_5MS    = 0x03,
    GAF_BAP_FRAME_DURATION_MAX,
};

/// Minimal value of length field for Codec Specific Configuration LTV structure
enum app_bap_cfg_length
{
    /// Sampling Frequencies
    APP_BAP_CFG_LENGTH_SAMP_FREQ = 2,
    /// Frame Duration
    APP_BAP_CFG_LENGTH_FRAME_DUR = 2,
    /// Audio Channel Allocation
    APP_BAP_CFG_LENGTH_CHNL_LOCATION = 5,
    /// Octets per Codec Frame
    APP_BAP_CFG_LENGTH_OCTETS_FRAME = 3,
    /// Codec Frame Blocks Per SDU
    APP_BAP_CFG_LENGTH_FRAMES_SDU = 2,
};

/// Sampling Frequency values
enum app_bap_sampling_freq
{
    APP_BAP_SAMPLING_FREQ_MIN = 1,
    /// 8000 Hz
    APP_BAP_SAMPLING_FREQ_8000HZ = APP_BAP_SAMPLING_FREQ_MIN,
    /// 11025 Hz
    APP_BAP_SAMPLING_FREQ_11025HZ,
    /// 16000 Hz
    APP_BAP_SAMPLING_FREQ_16000HZ,
    /// 22050 Hz
    APP_BAP_SAMPLING_FREQ_22050HZ,
    /// 24000 Hz
    APP_BAP_SAMPLING_FREQ_24000HZ,
    /// 32000 Hz
    APP_BAP_SAMPLING_FREQ_32000HZ,
    /// 44100 Hz
    APP_BAP_SAMPLING_FREQ_44100HZ,
    /// 48000 Hz
    APP_BAP_SAMPLING_FREQ_48000HZ,
    /// 88200 Hz
    APP_BAP_SAMPLING_FREQ_88200HZ,
    /// 96000 Hz
    APP_BAP_SAMPLING_FREQ_96000HZ,
    /// 176400 Hz
    APP_BAP_SAMPLING_FREQ_176400HZ,
    /// 192000 Hz
    APP_BAP_SAMPLING_FREQ_192000HZ,
    /// 384000 Hz
    APP_BAP_SAMPLING_FREQ_384000HZ,

    /// Maximum value
    APP_BAP_SAMPLING_FREQ_MAX
};

/// Supported Sampling Frequencies bit field meaning
enum app_bap_sampling_freq_bf
{
    /// 8000 Hz - Position
    APP_BAP_SAMPLING_FREQ_8000HZ_POS = 0,
    /// 8000 Hz - Bit
    APP_BAP_SAMPLING_FREQ_8000HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_8000HZ_POS),

    /// 11025 Hz - Position
    APP_BAP_SAMPLING_FREQ_11025HZ_POS = 1,
    /// 11025 Hz - Bit
    APP_BAP_SAMPLING_FREQ_11025HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_11025HZ_POS),

    /// 16000 Hz - Position
    APP_BAP_SAMPLING_FREQ_16000HZ_POS = 2,
    /// 16000 Hz - Bit
    APP_BAP_SAMPLING_FREQ_16000HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_16000HZ_POS),

    /// 22050 Hz - Position
    APP_BAP_SAMPLING_FREQ_22050HZ_POS = 3,
    /// 22050 Hz - Bit
    APP_BAP_SAMPLING_FREQ_22050HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_22050HZ_POS),

    /// 24000 Hz - Position
    APP_BAP_SAMPLING_FREQ_24000HZ_POS = 4,
    /// 24000 Hz - Bit
    APP_BAP_SAMPLING_FREQ_24000HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_24000HZ_POS),

    /// 32000 Hz - Position
    APP_BAP_SAMPLING_FREQ_32000HZ_POS = 5,
    /// 32000 Hz - Bit
    APP_BAP_SAMPLING_FREQ_32000HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_32000HZ_POS),

    /// 44100 Hz - Position
    APP_BAP_SAMPLING_FREQ_44100HZ_POS = 6,
    /// 44100 Hz - Bit
    APP_BAP_SAMPLING_FREQ_44100HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_44100HZ_POS),

    /// 48000 Hz - Position
    APP_BAP_SAMPLING_FREQ_48000HZ_POS = 7,
    /// 48000 Hz - Bit
    APP_BAP_SAMPLING_FREQ_48000HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_48000HZ_POS),

    /// 88200 Hz - Position
    APP_BAP_SAMPLING_FREQ_88200HZ_POS = 8,
    /// 88200 Hz - Bit
    APP_BAP_SAMPLING_FREQ_88200HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_88200HZ_POS),

    /// 96000 Hz - Position
    APP_BAP_SAMPLING_FREQ_96000HZ_POS = 9,
    /// 96000 Hz - Bit
    APP_BAP_SAMPLING_FREQ_96000HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_96000HZ_POS),

    /// 176400 Hz - Position
    APP_BAP_SAMPLING_FREQ_176400HZ_POS = 10,
    /// 176400 Hz - Bit
    APP_BAP_SAMPLING_FREQ_176400HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_176400HZ_POS),

    /// 192000 Hz - Position
    APP_BAP_SAMPLING_FREQ_192000HZ_POS = 11,
    /// 192000 Hz - Bit
    APP_BAP_SAMPLING_FREQ_192000HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_192000HZ_POS),

    /// 384000 Hz - Position
    APP_BAP_SAMPLING_FREQ_384000HZ_POS = 12,
    /// 384000 Hz - Bit
    APP_BAP_SAMPLING_FREQ_384000HZ_BIT = APP_CO_BIT(APP_BAP_SAMPLING_FREQ_384000HZ_POS),
};

/// Frame Duration values
enum app_bap_frame_dur
{
    APP_BAP_FRAME_DUR_MIN    = 0,
    /// Use 7.5ms Codec frames
    APP_BAP_FRAME_DUR_7_5MS  = 0,
    /// Use 10ms Codec frames
    APP_BAP_FRAME_DUR_10MS   = 1,
#ifdef LC3PLUS_SUPPORT
    /// Use 5ms Codec frames
    APP_BAP_FRAME_DUR_5MS    = 2,
    /// Use 2.5ms Codec frames
    APP_BAP_FRAME_DUR_2_5MS  = 3,
#endif
    /// Maximum value
    APP_BAP_FRAME_DUR_MAX
};


/// Supported Frame Duration Bitfield
/*
Examples:
0x01 = 0b00000001: The codec supports the 7.5 ms frame duration.
0x02 = 0b00000010: The codec supports the 10 ms frame duration.
0x03 = 0b00000011: The codec supports the 7.5 ms frame duration and the 10 ms frame duration.
0x13 = 0b00010011: The codec supports the 7.5 ms frame duration and the 10 ms frame duration and prefers the 7.5 ms frame duration.
*/
/// Supported Frame Durations bit field meaning
enum app_bap_frame_dur_bf
{
    /// 7.5ms frame duration is supported - Position
    APP_BAP_FRAME_DUR_7_5MS_POS = 0,
    /// 7.5ms frame duration is supported - Bit
    APP_BAP_FRAME_DUR_7_5MS_BIT = APP_CO_BIT(APP_BAP_FRAME_DUR_7_5MS_POS),

    /// 10ms frame duration is supported - Position
    APP_BAP_FRAME_DUR_10MS_POS = 1,
    /// 10ms frame duration is supported - Bit
    APP_BAP_FRAME_DUR_10MS_BIT = APP_CO_BIT(APP_BAP_FRAME_DUR_10MS_POS),

#ifdef LC3PLUS_SUPPORT
    /// 5ms frame duration is supported - Position
    APP_BAP_FRAME_DUR_5MS_POS  = 2,
    /// 5ms frame duration is supported - Bit
    APP_BAP_FRAME_DUR_5MS_BIT  = APP_CO_BIT(APP_BAP_FRAME_DUR_5MS_POS),

    /// 10ms frame duration is supported - Position
    APP_BAP_FRAME_DUR_2_5MS_POS = 3,
    /// 10ms frame duration is supported - Bit
    APP_BAP_FRAME_DUR_2_5MS_BIT = APP_CO_BIT(APP_BAP_FRAME_DUR_2_5MS_POS),
#endif

    /// 7.5ms frame duration is preferred - Position
    APP_BAP_FRAME_DUR_7_5MS_PREF_POS = 4,
    /// 7.5ms frame duration is preferred - Bit
    APP_BAP_FRAME_DUR_7_5MS_PREF_BIT = APP_CO_BIT(APP_BAP_FRAME_DUR_7_5MS_PREF_POS),

    /// 10ms frame duration is preferred - Position
    APP_BAP_FRAME_DUR_10MS_PREF_POS = 5,
    /// 10ms frame duration is preferred - Bit
    APP_BAP_FRAME_DUR_10MS_PREF_BIT = APP_CO_BIT(APP_BAP_FRAME_DUR_10MS_PREF_POS),
#ifdef LC3PLUS_SUPPORT
    /// 10ms frame duration is preferred - Position
    APP_BAP_FRAME_DUR_5MS_PREF_POS = 6,
    /// 10ms frame duration is preferred - Bit
    APP_BAP_FRAME_DUR_5MS_PREF_BIT = APP_CO_BIT(APP_BAP_FRAME_DUR_5MS_PREF_POS),

    /// 10ms frame duration is preferred - Position
    APP_BAP_FRAME_DUR_2_5MS_PREF_POS = 7,
    /// 10ms frame duration is preferred - Bit
    APP_BAP_FRAME_DUR_2_5MS_PREF_BIT = APP_CO_BIT(APP_BAP_FRAME_DUR_2_5MS_PREF_POS),
#endif
};

/// Context type bit field meaning
enum app_bap_context_type_bf
{
    /// Unspecified - Position
    APP_BAP_CONTEXT_TYPE_UNSPECIFIED_POS = 0,
    /// Unspecified - Bit
    APP_BAP_CONTEXT_TYPE_UNSPECIFIED_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_UNSPECIFIED_POS),

    /// Conversational - Position
    APP_BAP_CONTEXT_TYPE_CONVERSATIONAL_POS = 1,
    /// Conversational - Bit\n
    /// Conversation between humans as, for example, in telephony or video calls
    APP_BAP_CONTEXT_TYPE_CONVERSATIONAL_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_CONVERSATIONAL_POS),

    /// Media - Position
    APP_BAP_CONTEXT_TYPE_MEDIA_POS = 2,
    /// Media - Bit\n
    /// Media as, for example, in music, public radio, podcast or video soundtrack.
    APP_BAP_CONTEXT_TYPE_MEDIA_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_MEDIA_POS),

    /// Game - Position
    APP_BAP_CONTEXT_TYPE_GAME_POS = 3,
    /// Game - Bit\n
    /// Audio associated with video gaming, for example gaming media, gaming effects, music and in-game voice chat
    /// between participants; or a mix of all the above
    APP_BAP_CONTEXT_TYPE_GAME_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_GAME_POS),

    /// Instructional - Position
    APP_BAP_CONTEXT_TYPE_INSTRUCTIONAL_POS = 4,
    /// Instructional - Bit\n
    /// Instructional audio as, for example, in navigation, traffic announcements or user guidance
    APP_BAP_CONTEXT_TYPE_INSTRUCTIONAL_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_INSTRUCTIONAL_POS),

    /// Man Machine - Position
    APP_BAP_CONTEXT_TYPE_MAN_MACHINE_POS = 5,
    /// Man Machine - Bit\n
    /// Man machine communication as, for example, with voice recognition or virtual assistant
    APP_BAP_CONTEXT_TYPE_MAN_MACHINE_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_MAN_MACHINE_POS),

    /// Live - Position
    APP_BAP_CONTEXT_TYPE_LIVE_POS = 6,
    /// Live - Bit\n
    /// Live audio as from a microphone where audio is perceived both through a direct acoustic path and through
    /// an LE Audio Stream
    APP_BAP_CONTEXT_TYPE_LIVE_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_LIVE_POS),

    /// Sound Effects - Position
    APP_BAP_CONTEXT_TYPE_SOUND_EFFECTS_POS = 7,
    /// Sound Effects - Bit\n
    /// Sound effects including keyboard and touch feedback;
    /// menu and user interface sounds; and other system sounds
    APP_BAP_CONTEXT_TYPE_SOUND_EFFECTS_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_SOUND_EFFECTS_POS),

    /// Attention Seeking - Position
    APP_BAP_CONTEXT_TYPE_ATTENTION_SEEKING_POS = 8,
    /// Attention Seeking - Bit\n
    /// Attention seeking audio as, for example, in beeps signalling arrival of a message or keyboard clicks
    APP_BAP_CONTEXT_TYPE_ATTENTION_SEEKING_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_ATTENTION_SEEKING_POS),

    /// Ringtone - Position
    APP_BAP_CONTEXT_TYPE_RINGTONE_POS = 9,
    /// Ringtone - Bit\n
    /// Ringtone as in a call alert
    APP_BAP_CONTEXT_TYPE_RINGTONE_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_RINGTONE_POS),

    /// Immediate Alert - Position
    APP_BAP_CONTEXT_TYPE_IMMEDIATE_ALERT_POS = 10,
    /// Immediate Alert - Bit\n
    /// Immediate alerts as, for example, in a low battery alarm, timer expiry or alarm clock.
    APP_BAP_CONTEXT_TYPE_IMMEDIATE_ALERT_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_IMMEDIATE_ALERT_POS),

    /// Emergency Alert - Position
    APP_BAP_CONTEXT_TYPE_EMERGENCY_ALERT_POS = 11,
    /// Emergency Alert - Bit\n
    /// Emergency alerts as, for example, with fire alarms or other urgent alerts
    APP_BAP_CONTEXT_TYPE_EMERGENCY_ALERT_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_EMERGENCY_ALERT_POS),

    /// TV - Position
    APP_BAP_CONTEXT_TYPE_TV_POS = 12,
    /// TV - Bit\n
    /// Audio associated with a television program and/or with metadata conforming to the Bluetooth Broadcast TV
    /// profile
    APP_BAP_CONTEXT_TYPE_TV_BIT = APP_CO_BIT(APP_BAP_CONTEXT_TYPE_TV_POS),
};

/// Supported Audio Channel Counts Bitfield
/*
Example:
0x01 = 0b00000001: One channel supported.
0x02 = 0b00000010: Two channels supported.
0x27 = 0b00100111: One channel supported, two channels supported, three channels supported, and six channels supported.
*/
/// Supported Audio Channel Counts Bitfield
enum app_bap_chan_cnt_bf
{
    /// One channel supported
    APP_BAP_CHAN_CNT_1_POS    = 1,
    APP_BAP_CHAN_CNT_1_BIT = APP_CO_BIT(APP_BAP_CHAN_CNT_1_POS),

    /// two channels supported
    APP_BAP_CHAN_CNT_2_POS    = 2,
    APP_BAP_CHAN_CNT_2_BIT = APP_CO_BIT(APP_BAP_CHAN_CNT_2_POS),

    /// three channels supported
    APP_BAP_CHAN_CNT_3_POS    = 3,
    APP_BAP_CHAN_CNT_3_BIT = APP_CO_BIT(APP_BAP_CHAN_CNT_3_POS),

    /// four channels supported
    APP_BAP_CHAN_CNT_4_POS    = 4,
    APP_BAP_CHAN_CNT_4_BIT = APP_CO_BIT(APP_BAP_CHAN_CNT_4_POS),

    /// five channels supported
    APP_BAP_CHAN_CNT_5_POS    = 5,
    APP_BAP_CHAN_CNT_5_BIT = APP_CO_BIT(APP_BAP_CHAN_CNT_5_POS),

    /// six channels supported
    APP_BAP_CHAN_CNT_6_POS    = 6,
    APP_BAP_CHAN_CNT_6_BIT = APP_CO_BIT(APP_BAP_CHAN_CNT_6_POS),

    /// seven channels supported
    APP_BAP_CHAN_CNT_7_POS    = 7,
    APP_BAP_CHAN_CNT_7_BIT = APP_CO_BIT(APP_BAP_CHAN_CNT_7_POS),

    /// eight channels supported
    APP_BAP_CHAN_CNT_8_POS    = 8,
    APP_BAP_CHAN_CNT_8_BIT = APP_CO_BIT(APP_BAP_CHAN_CNT_8_POS),

};

/// Specify what PHY the Controller has changed for TX/RX. HCI:7.7.65.12
/*@TRACE*/
enum app_le_phy_value
{
    APP_PHY_UNDEF_VALUE    = 0,
    APP_PHY_1MBPS_VALUE    = 1,
    APP_PHY_2MBPS_VALUE    = 2,
    APP_PHY_CODED_VALUE    = 3,
#if (mHDT_SUPPORT)
    APP_PHY_4MBPS_VALUE    = 4,
#endif
};

/// Specify what PHY Host prefers to use for RX or TX HCI:7.8.48 / HCI:7.8.49
enum app_le_phy_mask
{
    /// The Host prefers to use the LE 1M transmitter/receiver PHY (possibly among others)
    APP_PHY_1MBPS_BIT      = (1<<0),
    APP_PHY_1MBPS_POS      = (0),
    /// The Host prefers to use the LE 2M transmitter/receiver PHY (possibly among others)
    APP_PHY_2MBPS_BIT      = (1<<1),
    APP_PHY_2MBPS_POS      = (1),
    /// The Host prefers to use the LE Coded transmitter/receiver PHY (possibly among others)
    APP_PHY_CODED_BIT      = (1<<2),
    APP_PHY_CODED_POS      = (2),
    /// The Host prefers to use the LE 4M transmitter/receiver PHY (possibly among others)
#if (mHDT_SUPPORT)
    APP_PHY_4MBPS_BIT      = (1<<3),
    APP_PHY_4MBPS_POS      = (3),
#endif
    /// The Host prefers to use the LE Coded transmitter/receiver PHY (possibly among others)
    APP_PHY_ALL        = (APP_PHY_1MBPS_BIT | APP_PHY_2MBPS_BIT | APP_PHY_CODED_BIT 
#if (mHDT_SUPPORT)
                        | APP_PHY_4MBPS_BIT
#endif
                        ),
};

/// Isochronous Group packing preference
enum app_iso_packing
{
    /// Sequential stream packing
    APP_ISO_PACKING_SEQUENTIAL = 0,
    /// Interleaved stream packing
    APP_ISO_PACKING_INTERLEAVED,

    APP_ISO_PACKING_MAX,
};

/// Isochronous PDU Framing mode
enum app_iso_frame
{
    /// Unframed mode
    APP_ISO_UNFRAMED_MODE = 0,
    /// Framed mode
    APP_ISO_FRAMED_MODE,

    APP_ISO_FRAME_MODE_MAX,
};

/// Context type values
enum app_bap_capa_context_type
{
    /// Supported Audio Contexts
    APP_BAP_CAPA_CONTEXT_TYPE_SUPP = 0,
    /// Available Audio Contexts
    APP_BAP_CAPA_CONTEXT_TYPE_AVA,

    APP_BAP_CAPA_CONTEXT_TYPE_MAX
};

/// Broadcast Group Parameters structure
typedef struct app_bap_bc_grp_param
{
    /// SDU interval in microseconds
    /// From 256us (0x00000100) to 1.048575s (0x000FFFFF)
    uint32_t sdu_intv_us;
    /// Maximum size of an SDU
    /// From 1 to 4095 bytes
    uint16_t max_sdu;
    /// Maximum time (in milliseconds) between the first transmission of an SDU to the end of the last transmission
    /// of the same SDU
    /// From 0ms to 4.095s (0x0FFF)
    uint16_t max_tlatency_ms;
    /// Sequential or Interleaved scheduling (see TODO [LT])
    uint8_t packing;
    /// Unframed or framed mode (see TODO [LT])
    uint8_t framing;
    /// Bitfield indicating PHYs that can be used by the controller for transmission of SDUs (see TODO [LT])
    uint8_t phy_bf;
    /// Number of times every PDU should be transmitted
    /// From 0 to 15
    uint8_t rtn;
} app_bap_bc_grp_param_t;

/// Advertising Parameters structure
typedef struct app_bap_bc_adv_param
{
    /// Minimum advertising interval in multiple of 0.625ms
    /// From 20ms (0x00000020) to 10485.759375s (0x00FFFFFF)
    uint32_t adv_intv_min_slot;
    /// Maximum advertising interval in multiple of 0.625ms
    /// From 20ms (0x00000020) to 10485.759375s (0x00FFFFFF)
    uint32_t adv_intv_max_slot;
    /// Channel Map (@see TODO [LT])
    uint8_t chnl_map;
    /// PHY for primary advertising (see #gap_phy_val enumeration)
    /// Only LE 1M and LE Codec PHYs are allowed
    uint8_t phy_prim;
    /// PHY for secondary advertising (see #gap_phy_val enumeration)
    uint8_t phy_second;
    /// Advertising SID
    /// From 0x00 to 0x0F
    uint8_t adv_sid;
} app_bap_bc_adv_param_t;

/// Periodic Advertising Parameters structure
typedef struct app_bap_bc_per_adv_param
{
    /// Minimum Periodic Advertising interval in multiple of 1.25ms
    /// Must be higher than 7.5ms (0x0006)
    uint16_t adv_intv_min_frame;
    /// Maximum Periodic Advertising interval in multiple of 1.25ms
    /// Must be higher than 7.5ms (0x0006)
    uint16_t adv_intv_max_frame;
} app_bap_bc_per_adv_param_t;

/// Codec Capabilities parameters structure
typedef struct app_bap_capa_param
{
    /// Supported Sampling Frequencies bit field (see #bap_sampling_freq_bf enumeration)\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// Mandatory for LC3
    uint16_t sampling_freq_bf;
    /// Supported Frame Durations bit field (see #bap_freq_dur_bf enumeration)\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// Mandatory for LC3
    uint8_t frame_dur_bf;
    /// Supported Audio Channel Counts\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// For LC3, absence in the Codec Specific Capabilities is equivalent to 1 channel supported (forced to 0x01
    /// on reception side)
    uint8_t chan_cnt_bf;
    /// Supported Octets Per Codec Frame - Minimum\n
    /// Not part of the Codec Specific Capabilities is equal to 0 and frame_octet_max also equal to 0\n
    /// Mandatory for LC3
    uint16_t frame_octet_min;
    /// Supported Octets Per Codec Frame - Maximum\n
    /// Not part of the Codec Specific Capabilities is equal to 0 and frame_octet_min also equal to 0\n
    /// Mandatory for LC3
    uint16_t frame_octet_max;
    /// Supported Maximum Codec Frames Per SDU\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// For LC3, absence in the Codec Specific Capabilities is equivalent to 1 Frame Per SDU (forced to 1 on
    /// reception side)
    uint8_t max_frames_sdu;
} app_bap_capa_param_t;

/// Codec Capabilities structure
typedef struct app_bap_capa
{
    /// Parameters structure
    app_bap_capa_param_t param;
    /// Additional Codec Capabilities (in LTV format)
    app_gaf_ltv_t add_capa;
} app_bap_capa_t;

/// Codec Capabilities Metadata parameters structure
typedef struct app_bap_capa_metadata_param
{
    /// Preferred Audio Contexts bit field (see #enum bap_context_type_bf enumeration)
    uint16_t context_bf;
} app_bap_capa_metadata_param_t;

/// Codec Capabilities Metadata structure
typedef struct app_bap_capa_metadata
{
    /// Parameters structure
    app_bap_capa_metadata_param_t param;
    /// Additional Metadata (in LTV format)
    app_gaf_ltv_t add_metadata;
} app_bap_capa_metadata_t;

typedef struct app_bap_vendor_specific_cfg
{
    uint8_t length;
    uint8_t type;
    uint16_t company_id;
    uint8_t s2m_encode_channel;
    uint8_t s2m_decode_channel;
} app_bap_vendor_specific_cfg_t;

/// Codec Configuration parameters structure
typedef struct app_bap_cfg_param
{
    /// Audio Locations of the Audio Channels being configured for the codec (i.e the number of codec frames per
    /// block) and their ordering within a single block of codec frames
    /// When transmitted, part of Codec Specific Configuration only if not equal to 0
    /// When received, 0 shall be interpreted as a single channel with no specified Audio Location
    uint32_t location_bf;
    /// Length of a codec frame in octets
    uint16_t frame_octet;
    /// Sampling Frequency (see #bap_sampling_freq enumeration)
    uint8_t sampling_freq;
    /// Frame Duration (see #bap_frame_dur enumeration)
    uint8_t frame_dur;
    /// Number of blocks of codec frames that shall be sent or received in a single SDU
    uint8_t frames_sdu;
} app_bap_cfg_param_t;

/// Codec Configuration structure
typedef struct app_bap_cfg
{
    /// Parameters structure
    app_bap_cfg_param_t param;
    /// Additional Codec Configuration (in LTV format)
    app_gaf_ltv_t add_cfg;
} app_bap_cfg_t;

/// Codec Configuration Metadata parameters structure
typedef struct app_bap_cfg_metadata_param
{
    /// Streaming Audio Contexts bit field (see #enum bap_context_type_bf enumeration)
    uint16_t context_bf;
} app_bap_cfg_metadata_param_t;

/// Codec Configuration Metadata structure
typedef struct app_bap_cfg_metadata
{
    /// Parameters structure
    app_bap_cfg_metadata_param_t param;
    /// Additional Metadata value (in LTV format)
    app_gaf_ltv_t add_metadata;
} app_bap_cfg_metadata_t;

/// Supported Audio Location Bitfield
enum gaf_bap_supported_locations_bf
{
    GAF_BAP_AUDIO_LOCATION_FRONT_LEFT               = 0x00000001,
    GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT              = 0x00000002,
    GAF_BAP_AUDIO_LOCATION_FRONT_CENTER             = 0x00000004,
    GAF_BAP_AUDIO_LOCATION_LOW_FREQ_EFFECTS_1       = 0x00000008,
    GAF_BAP_AUDIO_LOCATION_BACK_LEFT                = 0x00000010,
    GAF_BAP_AUDIO_LOCATION_BACK_RIGHT               = 0x00000020,
    GAF_BAP_AUDIO_LOCATION_FRONT_LEFT_OF_CENTER     = 0x00000040,
    GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT_OF_CENTER    = 0x00000080,
    GAF_BAP_AUDIO_LOCATION_BACK_CENTER              = 0x00000100,
    GAF_BAP_AUDIO_LOCATION_LOW_FREQ_EFFECTS_2       = 0x00000200,
    GAF_BAP_AUDIO_LOCATION_SIDE_LEFT                = 0x00000400,
    GAF_BAP_AUDIO_LOCATION_SIDE_RIGHT               = 0x00000800,
    GAF_BAP_AUDIO_LOCATION_TOP_FRONT_LEFT           = 0x00001000,
    GAF_BAP_AUDIO_LOCATION_TOP_FRONT_RIGHT          = 0x00002000,
    GAF_BAP_AUDIO_LOCATION_TOP_FRONT_CENTER         = 0x00004000,
    GAF_BAP_AUDIO_LOCATION_TOP_CENTER               = 0x00008000,
    GAF_BAP_AUDIO_LOCATION_TOP_BACK_LEFT            = 0x00010000,
    GAF_BAP_AUDIO_LOCATION_TOP_BACK_RIGHT           = 0x00020000,
    GAF_BAP_AUDIO_LOCATION_TOP_SIDE_LEFT            = 0x00040000,
    GAF_BAP_AUDIO_LOCATION_TOP_SIDE_RIGHT           = 0x00080000,
    GAF_BAP_AUDIO_LOCATION_TOP_BACK_CENTER          = 0x00100000,
    GAF_BAP_AUDIO_LOCATION_BOTTOM_FRONT_CENTER      = 0x00200000,
    GAF_BAP_AUDIO_LOCATION_BOTTOM_FRONT_LEFT        = 0x00400000,
    GAF_BAP_AUDIO_LOCATION_BOTTOM_FRONT_RIGHT       = 0x00800000,
    GAF_BAP_AUDIO_LOCATION_FRONT_LEFT_WIDE          = 0x01000000,
    GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT_WIDE         = 0x02000000,
    GAF_BAP_AUDIO_LOCATION_LEFT_SURROUND            = 0x04000000,
    GAF_BAP_AUDIO_LOCATION_RIGHT_SURROUND           = 0x08000000,

    GAF_BAP_AUDIO_LOCATION_RFU                      = 0xF0000000,
};

/// Context type bit field meaning
typedef enum gaf_bap_context_type_bf
{
    APP_BAP_CONTEXT_TYPE_UNSPECIFIED            = 0x0001,
    /// Conversation between humans as, for example, in telephony or video calls
    APP_BAP_CONTEXT_TYPE_CONVERSATIONAL         = 0x0002,
    /// Media as, for example, in music, public radio, podcast or video soundtrack.
    APP_BAP_CONTEXT_TYPE_MEDIA                  = 0x0004,
    /// Audio associated with video gaming, for example gaming media, gaming effects, music and in-game voice chat
    /// between participants; or a mix of all the above
    APP_BAP_CONTEXT_TYPE_GAME                   = 0x0008,
    /// Instructional audio as, for example, in navigation, traffic announcements or user guidance
    APP_BAP_CONTEXT_TYPE_INSTRUCTIONAL          = 0x0010,
    /// Man machine communication as, for example, with voice recognition or virtual assistant
    APP_BAP_CONTEXT_TYPE_MAN_MACHINE            = 0x0020,
    /// Live audio as from a microphone where audio is perceived both through a direct acoustic path and through
    /// an LE Audio Stream
    APP_BAP_CONTEXT_TYPE_LIVE                   = 0x0040,
   /// Sound effects including keyboard and touch feedback;
    /// menu and user interface sounds; and other system sounds
    APP_BAP_CONTEXT_TYPE_SOUND_EFFECT           = 0x0080,
    /// Attention seeking audio as, for example, in beeps signalling arrival of a message or keyboard clicks
    APP_BAP_CONTEXT_TYPE_ATTENTION_SEEKING      = 0x0100,
    /// Ringtone as in a call alert
    APP_BAP_CONTEXT_TYPE_RINGTONE               = 0x0200,
    /// Immediate alerts as, for example, in a low battery alarm, timer expiry or alarm clock.
    APP_BAP_CONTEXT_TYPE_IMMEDIATE_ALERT        = 0x0400,
    /// Emergency alerts as, for example, with fire alarms or other urgent alerts
    APP_BAP_CONTEXT_TYPE_EMERGENCY_ALERT        = 0x0800,
    /// Audio associated with a television program and/or with metadata conforming to the Bluetooth Broadcast TV
    /// profile
    APP_BAP_CONTEXT_TYPE_TV                     = 0x1000,
}app_bap_context_type_bf_t;

/// Target Latency values
enum app_bap_uc_tgt_latency
{
    APP_BAP_UC_TGT_LATENCY_MIN = 1,

    /// Target lower latency
    APP_BAP_UC_TGT_LATENCY_LOWER = APP_BAP_UC_TGT_LATENCY_MIN,
    /// Target balanced latency and reliability
    APP_BAP_UC_TGT_LATENCY_BALENCED,
    /// Target higher reliability
    APP_BAP_UC_TGT_LATENCY_RELIABLE,

    APP_BAP_UC_TGT_LATENCY_MAX,
};

typedef enum
{
    BAP_QOS_SETTING_MIN = 0,

    BAP_QOS_SETTING_LL_8_1_1 = BAP_QOS_SETTING_MIN,
    BAP_QOS_SETTING_LL_8_2_1,
    BAP_QOS_SETTING_LL_16_1_1,
    BAP_QOS_SETTING_LL_16_2_1,
    BAP_QOS_SETTING_LL_24_1_1,
    BAP_QOS_SETTING_LL_24_2_1,
    BAP_QOS_SETTING_LL_32_1_1,
    BAP_QOS_SETTING_LL_32_2_1,
    BAP_QOS_SETTING_LL_441_1_1,
    BAP_QOS_SETTING_LL_441_2_1,
    BAP_QOS_SETTING_LL_48_1_1,
    BAP_QOS_SETTING_LL_48_2_1,
    BAP_QOS_SETTING_LL_48_3_1,
    BAP_QOS_SETTING_LL_48_4_1,
    BAP_QOS_SETTING_LL_48_5_1,
    BAP_QOS_SETTING_LL_48_6_1,
    BAP_QOS_SETTING_LL_MAX,

    BAP_QOS_SETTING_HR_8_1_2 = BAP_QOS_SETTING_LL_MAX,
    BAP_QOS_SETTING_HR_8_2_2,
    BAP_QOS_SETTING_HR_16_1_2,
    BAP_QOS_SETTING_HR_16_2_2,
    BAP_QOS_SETTING_HR_24_1_2,
    BAP_QOS_SETTING_HR_24_2_2,
    BAP_QOS_SETTING_HR_32_1_2,
    BAP_QOS_SETTING_HR_32_2_2,
    BAP_QOS_SETTING_HR_441_1_2,
    BAP_QOS_SETTING_HR_441_2_2,
    BAP_QOS_SETTING_HR_48_1_2,
    BAP_QOS_SETTING_HR_48_2_2,
    BAP_QOS_SETTING_HR_48_3_2,
    BAP_QOS_SETTING_HR_48_4_2,
    BAP_QOS_SETTING_HR_48_5_2,
    BAP_QOS_SETTING_HR_48_6_2,

    BAP_QOS_SETTING_HR_MAX, 

#if defined (AOB_GMAP_ENABLED)
    BAP_QOS_SETTING_GMING_48_1_GC = BAP_QOS_SETTING_HR_MAX,
    BAP_QOS_SETTING_GMING_48_2_GC,
    BAP_QOS_SETTING_GMING_48_1_GR,
    BAP_QOS_SETTING_GMING_48_2_GR,
    BAP_QOS_SETTING_GMING_48_3_GR,
    BAP_QOS_SETTING_GMING_48_4_GR,
    BAP_QOS_SETTING_NUM_MAX,
#else
    BAP_QOS_SETTING_NUM_MAX = BAP_QOS_SETTING_HR_MAX,
#endif
}app_bap_qos_setting_e;

typedef enum
{
    BAP_AUD_CFG_MIN = 0,

    BAP_AUD_CFG_1 = BAP_AUD_CFG_MIN,
    BAP_AUD_CFG_2,
    BAP_AUD_CFG_3,
    BAP_AUD_CFG_4,
    BAP_AUD_CFG_5,
    BAP_AUD_CFG_6_i,
    BAP_AUD_CFG_6_ii,
    BAP_AUD_CFG_7_i,
    BAP_AUD_CFG_7_ii,
    BAP_AUD_CFG_8_i,
    BAP_AUD_CFG_8_ii,
    BAP_AUD_CFG_9_i,
    BAP_AUD_CFG_9_ii,
    BAP_AUD_CFG_10,
    BAP_AUD_CFG_11_i,
    BAP_AUD_CFG_11_ii,

    BAP_AUD_CFG_MAX
}app_bap_aud_cfg_e;

typedef struct BAP_QOS_CONFIG_SETTING
{
    uint8_t     Faming_type;
    uint8_t     Rtn_num;
    uint8_t     Max_trans_latency;
    uint8_t     Pres_Delay;
    uint8_t     Sdu_intval;
    uint16_t    Oct_max;
} bap_qos_setting_t;

typedef struct BAP_AUDIO_CONFIGURATION
{
    uint8_t     sink_supp_aud_chn_cnt_bf;
    uint8_t     src_supp_aud_chn_cnt_bf;
    uint8_t     sink_max_cfs_per_sdu;
    uint8_t     src_max_cfs_per_sdu;
    uint32_t    sink_aud_location_bf;
    uint32_t    src_aud_location_bf;
} bap_audio_cfg_t;

/// Data path configuration structure
typedef struct app_bap_uc_dp_cfg
{
    /// Datapath ID
    uint8_t dp_id;
    /// Controller Delay in microseconds
    uint32_t ctl_delay_us;
}app_bap_dp_cfg_t;

/// QoS Requirement structure
typedef struct app_bap_qos_req
{
    /// Presentation Delay minimum microseconds
    uint32_t pres_delay_min_us;
    /// Presentation Delay maximum in microseconds
    uint32_t pres_delay_max_us;
    /// Minimum preferred presentation delay in microseconds
    /// 0 means no preference
    /// If not equal to 0, shall be >= pres_delay_min_us
    uint32_t pref_pres_delay_min_us;
    /// Maximum preferred presentation delay in microseconds
    /// 0 means no preference
    /// If not equal to 0, shall be <= pres_delay_max_us
    uint32_t pref_pres_delay_max_us;
    /// Maximum Transport latency in milliseconds
    /// From 5ms (0x5) to 4000ms (0xFA0)
    uint16_t trans_latency_max_ms;
    /// PDU framing arrangement
    uint8_t framing;
    /// Preferred PHY bit field
    uint8_t phy_bf;
    /// Preferred maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 15
    uint8_t retx_nb;
} app_bap_qos_req_t;


/// QoS Configuration structure
typedef struct app_bap_qos_cfg
{
    /// PDU framing arrangement
    uint8_t framing;
    /// PHY
    uint8_t phy;
    /// Maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 15
    uint8_t retx_nb;
    /// Maximum SDU size
    /// From 0 to 4095 bytes (0xFFF)
    uint16_t max_sdu_size;
    /// Maximum Transport latency in milliseconds
    /// From 5ms (0x5) to 4000ms (0xFA0)
    uint16_t trans_latency_max_ms;
    /// Presentation Delay in microseconds
    uint32_t pres_delay_us;
    /// SDU interval in microseconds
    /// From 255us (0xFF) to 16777215us (0xFFFFFF)
    uint32_t sdu_intv_us;
} app_bap_qos_cfg_t;

/// Advertising Parameters structure
typedef struct app_bap_bc_adv_data
{
    /// Advertising data length
    uint8_t adv_data_len;
    /// Advertising data adv_type
    uint8_t adv_type;
    /// Advertising data
    uint8_t adv_data[0];
}app_bap_bc_adv_data_t;

typedef struct
{
    int m2s_bn;
    int m2s_nse;
    int m2s_ft;
    int s2m_bn;
    int s2m_nse;
    int s2m_ft;

    int frame_cnt_per_sdu;

    int iso_interval; // unit: 1.25 ms

} CIS_TIMING_CONFIGURATION_T;

bool app_bap_codec_is_lc3(const void* p_codec_id);
#ifdef LC3PLUS_SUPPORT
bool app_bap_codec_is_lc3plus(const void *p_codec_id);
#endif
uint8_t app_bap_frame_dur_us_to_frame_dur_enum(uint32_t frame_dur_us);
bool app_bap_get_specifc_ltv_data(app_gaf_ltv_t *add_data, uint8_t specific_ltv_type, void *p_cfg_out);
void app_bap_capa_param_print(app_bap_capa_param_t *capa_param);
void app_bap_capa_print(app_bap_capa_t* p_capa);
void app_bap_capa_metadata_print(app_bap_capa_metadata_t* p_capa_metadata);
void app_bap_cfg_print(app_bap_cfg_t* p_cfg);
void app_bap_cfg_metadata_print(app_bap_cfg_metadata_t* p_cfg_metadata);


#ifdef __cplusplus
extern "C" {
#endif
void app_bap_add_data_set(uint8_t *data, uint8_t data_len);
uint32_t app_bap_cmp_evt_handler(void const *param);
uint32_t app_bap_rsp_handler(void const *param);
uint32_t app_bap_ind_handler(void const *param);
uint32_t app_bap_req_ind_handler(void const *param);

#ifdef AOB_MOBILE_ENABLED
void app_bap_client_init(void);
void app_bap_start(uint8_t con_lid);
void app_bap_update_sdu_intv(uint32_t sdu_intv_m2s_us, uint32_t sdu_intv_s2m_us);
void app_bap_set_device_num_to_be_connected(uint8_t dev_num);
uint8_t app_bap_get_device_num_to_be_connected(void);
#endif

void app_bap_update_cis_timing(CIS_TIMING_CONFIGURATION_T* pTiming);
CIS_TIMING_CONFIGURATION_T* app_bap_get_cis_timing_config(void);
gaf_bap_activity_type_e app_bap_get_activity_type(void);
void app_bap_set_activity_type(gaf_bap_activity_type_e type);
uint32_t app_bap_get_role_bit_filed(void);

/**
 * @brief Frame_dur enumerate to frame_dur (us)
 * 
 * @param frame_duration_e 
 * @return uint32_t 
 */
uint32_t app_bap_frame_dur_enum_to_frame_dur_us(uint8_t frame_duration_e);

/**
 * @brief Input audio location bf, return L+R cnt
 * 
 * @param audio_location_bf 
 * @return uint8_t 
 */
uint8_t app_bap_get_audio_location_l_r_cnt(uint32_t audio_location_bf);

/**
 * @brief Input audio channel support chn bf, return max supp cnt
 * 
 * @param audio_channel_cnt_bf 
 * @return uint8_t 
 */
uint8_t app_bap_get_max_audio_channel_supp_cnt(uint32_t audio_channel_cnt_bf);
#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAP_H_

/// @} APP_BAP
