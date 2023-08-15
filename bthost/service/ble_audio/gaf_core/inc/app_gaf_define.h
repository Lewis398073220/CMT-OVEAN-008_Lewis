/**
 ****************************************************************************************
 *
 * @file app_gaf_define.h
 *
 * @brief BLE Audio Generic Audio Framework
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
/**
 * NOTE: This header file defines the common used module for upper layer
 */

#ifndef APP_GAF_DEFINE_H_
#define APP_GAF_DEFINE_H_

#if BLE_AUDIO_ENABLED
/*****************************header include********************************/
#include <stdbool.h>       // standard boolean definitions
#include <stddef.h>        // standard definitions
#include <stdint.h>        // standard integer definitions
#include "app_bap.h"
#include "app_bap_bc_scan_msg.h"
#include "bap_capa_cli_msg.h"
#include "csis.h"
#include "bap_bc.h"
#include "acc_mcc.h"
#include "acc_tbc.h"

#ifdef __cplusplus
extern "C"{
#endif

/******************************macro defination*****************************/
/// Retrieves module id from event id.
#define GAF_ID_GET(event_id) (((event_id) & 0xF000) >> 12)
/// Retrieves event from event id.
#define GAF_EVENT_GET(event_id) ((event_id) & 0x0FFF)
/// Builds the identifier from the id and the event
#define GAF_BUILD_ID(id, event) ( (uint16_t)(((id) << 12)|(event)) )

#define AOB_CODEC_ID_LC3          (&codec_id_lc3)
#define AOB_CODEC_ID_LC3PLUS      (&codec_id_lc3plus)
/*****************************value declaration*****************************/
extern const app_gaf_codec_id_t codec_id_lc3;
extern const app_gaf_codec_id_t codec_id_lc3plus;
/******************************type defination******************************/
/// Error codes
enum app_gaf_err
{
    /// No Error
    APP_GAF_ERR_NO_ERROR = 0,
    /// Params Invalid
    APP_GAF_ERR_INVALID_PARAM,
    /// Mermory Malloced Failed
    APP_GAF_ERR_MALLOC_ERROR,
};

/// Codec Type values
typedef enum app_gaf_codec_type
{
    /// LC3 Codec
    APP_GAF_CODEC_TYPE_LC3    = 0x06,
    /// Maximum SIG Codec
    APP_GAF_CODEC_TYPE_SIG_MAX,
    /// Vendor Specific Codec
    APP_GAF_CODEC_TYPE_VENDOR = 0xFF,
} APP_GAF_CODEC_TYPE_T;

/// Audio Locations bit field meaning
enum app_gaf_loc_bf
{
    /// Front Left
    APP_GAF_LOC_FRONT_LEFT_POS = 0,
    APP_GAF_LOC_FRONT_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_LEFT_POS),
    /// Front Right
    APP_GAF_LOC_FRONT_RIGHT_POS = 1,
    APP_GAF_LOC_FRONT_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_RIGHT_POS),
    /// Front Center
    APP_GAF_LOC_FRONT_CENTER_POS = 2,
    APP_GAF_LOC_FRONT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_CENTER_POS),
    /// Low Frequency Effect 1
    APP_GAF_LOC_LFE1_POS = 3,
    APP_GAF_LOC_LFE1_BIT = APP_CO_BIT(APP_GAF_LOC_LFE1_POS),
    /// Back Left
    APP_GAF_LOC_BACK_LEFT_POS = 4,
    APP_GAF_LOC_BACK_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_BACK_LEFT_POS),
    /// Back Right
    APP_GAF_LOC_BACK_RIGHT_POS = 5,
    APP_GAF_LOC_BACK_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_BACK_RIGHT_POS),
    /// Front Left Center
    APP_GAF_LOC_FRONT_LEFT_CENTER_POS = 6,
    APP_GAF_LOC_FRONT_LEFT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_LEFT_CENTER_POS),
    /// Front Right Center
    APP_GAF_LOC_FRONT_RIGHT_CENTER_POS = 7,
    APP_GAF_LOC_FRONT_RIGHT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_RIGHT_CENTER_POS),
    /// Back Center
    APP_GAF_LOC_BACK_CENTER_POS = 8,
    APP_GAF_LOC_BACK_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_BACK_CENTER_POS),
    /// Low Frequency Effect 2
    APP_GAF_LOC_LFE2_POS = 9,
    APP_GAF_LOC_LFE2_BIT = APP_CO_BIT(APP_GAF_LOC_LFE2_POS),
    /// Side Left
    APP_GAF_LOC_SIDE_LEFT_POS = 10,
    APP_GAF_LOC_SIDE_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_SIDE_LEFT_POS),
    /// Side Right
    APP_GAF_LOC_SIDE_RIGHT_POS = 11,
    APP_GAF_LOC_SIDE_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_SIDE_RIGHT_POS),
    /// Top Front Left
    APP_GAF_LOC_TOP_FRONT_LEFT_POS = 12,
    APP_GAF_LOC_TOP_FRONT_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_FRONT_LEFT_POS),
    /// Top Front Right
    APP_GAF_LOC_TOP_FRONT_RIGHT_POS = 13,
    APP_GAF_LOC_TOP_FRONT_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_FRONT_RIGHT_POS),
    /// Top Front Center
    APP_GAF_LOC_TOP_FRONT_CENTER_POS = 14,
    APP_GAF_LOC_TOP_FRONT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_FRONT_CENTER_POS),
    /// Top Center
    APP_GAF_LOC_TOP_CENTER_POS = 15,
    APP_GAF_LOC_TOP_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_CENTER_POS),
    /// Top Back Left
    APP_GAF_LOC_TOP_BACK_LEFT_POS = 16,
    APP_GAF_LOC_TOP_BACK_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_BACK_LEFT_POS),
    /// Top Back Right
    APP_GAF_LOC_TOP_BACK_RIGHT_POS = 17,
    APP_GAF_LOC_TOP_BACK_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_BACK_RIGHT_POS),
    /// Top Side Left
    APP_GAF_LOC_TOP_SIDE_LEFT_POS = 18,
    APP_GAF_LOC_TOP_SIDE_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_SIDE_LEFT_POS),
    /// Top Side Right
    APP_GAF_LOC_TOP_SIDE_RIGHT_POS = 19,
    APP_GAF_LOC_TOP_SIDE_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_SIDE_RIGHT_POS),
    /// Top Back Center
    APP_GAF_LOC_TOP_BACK_CENTER_POS = 20,
    APP_GAF_LOC_TOP_BACK_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_BACK_CENTER_POS),
    /// Bottom Front Center
    APP_GAF_LOC_BOTTOM_FRONT_CENTER_POS = 21,
    APP_GAF_LOC_BOTTOM_FRONT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_BOTTOM_FRONT_CENTER_POS),
    /// Bottom Front Left
    APP_GAF_LOC_BOTTOM_FRONT_LEFT_POS = 22,
    APP_GAF_LOC_BOTTOM_FRONT_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_BOTTOM_FRONT_LEFT_POS),
    /// Bottom Front Right
    APP_GAF_LOC_BOTTOM_FRONT_RIGHT_POS = 23,
    APP_GAF_LOC_BOTTOM_FRONT_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_BOTTOM_FRONT_RIGHT_POS),
    /// Front Left Wide
    APP_GAF_LOC_FRONT_LEFT_WIDE_POS = 24,
    APP_GAF_LOC_FRONT_LEFT_WIDE_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_LEFT_WIDE_POS),
    /// Front Right Wide
    APP_GAF_LOC_FRONT_RIGHT_WIDE_POS = 25,
    APP_GAF_LOC_FRONT_RIGHT_WIDE_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_RIGHT_WIDE_POS),
    /// Left Surround
    APP_GAF_LOC_LEFT_SURROUND_POS = 26,
    APP_GAF_LOC_LEFT_SURROUND_BIT = APP_CO_BIT(APP_GAF_LOC_LEFT_SURROUND_POS),
    /// Right Surround
    APP_GAF_LOC_RIGHT_SURROUND_POS = 27,
    APP_GAF_LOC_RIGHT_SURROUND_BIT = APP_CO_BIT(APP_GAF_LOC_RIGHT_SURROUND_POS),
};

/// Volume Operation Code values
/// @see enum arc_vc_opcode
enum gaf_arc_vc_opcode
{
    /// Relative Volume Down
    GAF_ARC_VC_OPCODE_VOL_DOWN = 0,
    /// Relative Volume Up
    GAF_ARC_VC_OPCODE_VOL_UP,
    /// Unmute/Relative Volume Down
    GAF_ARC_VC_OPCODE_VOL_DOWN_UNMUTE,
    /// Unmute/Relative Volume Up
    GAF_ARC_VC_OPCODE_VOL_UP_UNMUTE,
    /// Set Absolute Volume
    GAF_ARC_VC_OPCODE_VOL_SET_ABS,
    /// Unmute
    GAF_ARC_VC_OPCODE_VOL_UNMUTE,
    /// Mute
    GAF_ARC_VC_OPCODE_VOL_MUTE,

    GAF_ARC_VC_OPCODE_MAX
};

/// Set type values
enum gaf_arc_voc_set_type
{
    /// Volume offset
    GAF_ARC_VOC_SET_TYPE_OFFSET = 0,
    /// Audio location
    GAF_ARC_VOC_SET_TYPE_LOCATION,

    GAF_ARC_VOC_SET_TYPE_MAX
};

/// Unicast group configuration structure (provided by controller after stream establisment)
typedef struct
{
    /// Group synchronization delay time in microseconds
    uint32_t sync_delay_us;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from master to slave
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t tlatency_m2s_us;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from slave to master
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t tlatency_s2m_us;
    /// ISO interval (1.25ms unit, range: 5ms to 4s)
    uint16_t iso_intv_frames;
} app_gaf_iap_ug_config_t;

/// Unicast stream configuration structure (provided by controller after stream establishment)
typedef struct
{
    /// Stream synchronization delay time in microseconds
    uint32_t sync_delay_us;
    /// Maximum size, in octets, of the payload from master to slave (Range: 0x00-0xFB)
    uint16_t max_pdu_m2s;
    /// Maximum size, in octets, of the payload from slave to master (Range: 0x00-0xFB)
    uint16_t max_pdu_s2m;
    /// Master to slave PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_m2s;
    /// Slave to master PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_s2m;
    /// The burst number for master to slave transmission (0x00: no isochronous data from the master to the slave, range 0x01-0x0F)
    uint8_t bn_m2s;
    /// The burst number for slave to master transmission (0x00: no isochronous data from the slave to the master, range 0x01-0x0F)
    uint8_t bn_s2m;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the master to the slave (Range: 0x01-0x1F)
    uint8_t ft_m2s;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the slave to the master (Range: 0x01-0x1F)
    uint8_t ft_s2m;
    /// Maximum number of subevents in each isochronous interval. From 0x1 to 0x1F
    uint8_t nse;
} app_gaf_iap_us_config_t;

/// Structure for BAP_UC_SRV_CIS_STATE indication message
typedef struct
{
    /// Indication code (@see enum bap_uc_srv_ind_code)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Connection local index of LE connection the CIS is bound with
    uint8_t con_lid;
    /// ASE local index for Sink direction
    uint8_t ase_lid_sink;
    /// ASE local index for Source direction
    uint8_t ase_lid_src;
    /// CIG ID
    uint8_t cig_id;
    /// CIS ID
    uint8_t cis_id;
    /// Connection handle allocated by the controller
    /// GAP_INVALID_CONHDL means that the CIS is not established
    uint16_t conhdl;
    /// Group configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    app_gaf_iap_ug_config_t cig_config;
    /// Stream configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    app_gaf_iap_us_config_t cis_config;
    /// status
    uint8_t status;
    /// reason
    uint8_t reason;
} app_gaf_uc_srv_cis_state_ind_t;

/// Structure for cis stream started indication message
typedef struct
{
    uint8_t ase_lid;
    uint16_t con_lid;
    uint16_t cis_hdl;
    uint8_t direction;
} app_gaf_ascs_cis_stream_started_t;

/// Structure for cis stream stopped indication message
typedef struct
{
    uint8_t con_lid;
    uint8_t ase_lid;
    uint16_t cis_hdl;
    uint8_t direction;
} app_gaf_ascs_cis_stream_stopped_t;

/// Structure for BAP_UC_SRV_CONFIGURE_CODEC request indication message
typedef struct
{
    /// Request indication code (set to #BAP_UC_SRV_CONFIGURE_CODEC_RI)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// ASE local index
    uint8_t ase_lid;
    /// Target Latency (see #bap_uc_tgt_latency enumeration)
    uint8_t tgt_latency;
    /// Target PHY (see #bap_uc_tgt_phy enumeration)
    uint8_t tgt_phy;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Codec Configuration structure
    app_bap_cfg_t cfg;
} app_gaf_uc_srv_configure_codec_req_ind_t;

/// Structure for BAP_UC_SRV_ENABLE request indication message
typedef struct
{
    /// Request indication code (@see enum bap_uc_srv_req_ind_code)
    ///  - BAP_UC_SRV_ENABLE
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata structure
    app_bap_cfg_metadata_t metadata;
} app_gaf_uc_srv_enable_req_ind_t;

/// Structure for BAP_UC_SRV_UPDATE_METADATA request indication message
typedef struct
{
    /// Request indication code (@see enum bap_uc_srv_req_ind_code)
    ///  - BAP_UC_SRV_UPDATE_METADATA_RI
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata structure
    app_bap_cfg_metadata_t metadata;
} app_gaf_uc_srv_update_metadata_req_ind_t;

/// Structure for BAP_UC_SRV_RELEASE request indication message
typedef struct
{
    /// Request indication code (@see enum bap_uc_srv_req_ind_code)
    ///  - BAP_UC_SRV_RELEASE_RI
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
} app_gaf_uc_srv_release_req_ind_t;

/// Structure for BAP_CAPA_SRV_LOCATION indication message
typedef struct
{
    /// Indication code (@see enum bap_capa_srv_ind_code)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Direction (@see enum gaf_direction)
    uint8_t direction;
    /// Location bit field
    uint32_t location_bf;
} app_gaf_capa_srv_location_ind_t;

typedef struct
{
    /// Indication code (shall be set to #BAP_CAPA_SRV_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field\n
    /// Each bit corresponds to a characteristic in the range [0, BAP_CAPA_CHAR_TYPE_PAC[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for Sink/Source PAC characteristic\n
    /// Each bit corresponds to an instance of the Sink/Source PAC characteristic (Sink placed first)
    uint16_t pac_cli_cfg_bf;
} app_gaf_capa_srv_bond_data_ind_t;

typedef struct
{
    /// Indication code (shall be set to #BAP_CAPA_SRV_LOCATION)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_capa_srv_cccd_written_ind_t;

/// Structure for scan report indication message
typedef struct
{
    uint8_t             scan_trigger_method;
    /// Broadcast ID
    bap_bcast_id_t      bcast_id;
    app_gaf_extend_adv_report_t adv_report;
} app_gaf_bc_scan_adv_report_t;

/// Structure for scan report indication message
typedef struct
{
    uint8_t   scan_trigger_method;
    // True if broadcast isochronous group is encrypted, False otherwise
    bool      encrypted;
} app_gaf_bc_scan_state_stream_t;

/// Structure for BAP_BC_SCAN_PA_REPORT indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_PA_REPORT
    uint16_t    ind_code;
    /// Periodic Advertising local index
    uint8_t     pa_lid;
    /// Length of advertising data
    uint8_t     length;
    /// Complete periodic advertising report
    uint8_t     data[__ARRAY_EMPTY];
} app_gaf_bc_scan_pa_report_ind_t;

/// Structure for BAP_BC_SCAN_PA_ESTABLISHED indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_PA_ESTABLISHED
    uint16_t            ind_code;
    /// Periodic Advertising local index
    uint8_t             pa_lid;
    /// Periodic advertising address information
    app_bap_adv_id_t   adv_addr;
    /// Only valid for a Periodic Advertising Sync Transfer, else ignore
    uint16_t            serv_data;
} app_gaf_bc_scan_pa_established_ind_t;

/// Structure for BAP_BC_SCAN_PA_TERMINATED indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_PA_TERMINATED
    uint16_t             ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Stop reason (@see enum gaf_err)
    uint16_t             reason;
} app_gaf_bc_scan_pa_terminated_ind_t;

/// Structure for BAP_BC_SCAN_GROUP_REPORT indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_GROUP_REPORT
    uint16_t             ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Number of subgroups in the Broadcast Group
    uint8_t              nb_subgroups;
    /// Total number of streams in the Broadcast Group
    uint8_t              nb_streams;
    /// Codec ID value
    app_gaf_codec_id_t       codec_id;
    /// Audio output presentation delay in microseconds
    uint32_t             pres_delay_us;
    /// Length of Codec Configuration value
    uint8_t              cfg_len;
    /// Length of Metadata value
    uint8_t              metadata_len;
    /// Codec Configuration value followed by Metadata value
    /// Both values are in LTV format
    uint8_t              val[__ARRAY_EMPTY];
} app_gaf_bc_scan_group_report_ind_t;

/// Structure for BAP_BC_SCAN_SUBGROUP_REPORT indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_SUBGROUP_REPORT
    uint16_t             ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Subgroup identifier
    uint8_t              sgrp_id;
    /// Codec ID value
    /// (Octet  = 0xFE if equals to group codec, surcharged otherwise)
    app_gaf_codec_id_t       codec_id;
    /// Stream position index bit field indicating for which streams are part of the subgroup
    uint32_t             stream_pos_bf;
    /// Length of Codec Configuration value
    uint8_t              cfg_len;
    /// Length of Metadata value
    uint8_t              metadata_len;
    /// Codec Configuration value followed by Metadata value
    /// Both values are in LTV format
    uint8_t              val[__ARRAY_EMPTY];
} app_gaf_bc_scan_subgroup_report_ind_t;

/// Structure for BAP_BC_SCAN_STREAM_REPORT indication message
typedef struct
{
    /// Indication code (set to #BAP_BC_SCAN_STREAM_REPORT)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Subgroup identifier
    uint8_t sgrp_id;
    /// Stream position in group
    uint8_t stream_pos;
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// Codec Configuration structure
    bap_cfg_t cfg;
} app_gaf_bc_scan_stream_report_ind_t;

/// BIG Info Report
typedef struct
{
    /// Value of the SDU interval in microseconds (Range 0x0000FF-0x0FFFFF)
    uint32_t  sdu_interval;
    /// Value of the ISO Interval (1.25 ms unit)
    uint16_t  iso_interval;
    /// Value of the maximum PDU size (Range 0x0000-0x00FB)
    uint16_t  max_pdu;
    /// VValue of the maximum SDU size (Range 0x0000-0x0FFF)
    uint16_t  max_sdu;
    /// Number of BIS present in the group (Range 0x01-0x1F)
    uint8_t   num_bis;
    /// Number of sub-events (Range 0x01-0x1F)
    uint8_t   nse;
    /// Burst number (Range 0x01-0x07)
    uint8_t   bn;
    /// Pre-transmit offset (Range 0x00-0x0F)
    uint8_t   pto;
    /// Initial retransmission count (Range 0x01-0x0F)
    uint8_t   irc;
    /// PHY used for transmission (0x01: 1M, 0x02: 2M, 0x03: Coded, All other values: RFU)
    uint8_t   phy;
    /// Framing mode (0x00: Unframed, 0x01: Framed, All other values: RFU)
    uint8_t   framing;
    /// True if broadcast isochronous group is encrypted, False otherwise
    bool      encrypted;
} app_gaf_big_info_t;

/// Structure for BAP_BC_SCAN_BIG_INFO_REPORT indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_BIG_INFO_REPORT
    uint16_t             ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// BIG Info Report
    app_gaf_big_info_t       report;
} app_gaf_bc_scan_big_info_report_ind_t;

/// Structure for BAP_BC_SCAN_PA_SYNCHRONIZE_RI request indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_req_ind_codes)
    /// - BAP_BC_SCAN_PA_SYNCHRONIZE_RI
    uint16_t             req_ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Source local index
    uint8_t              src_lid;
    /// Connection Local Identifier of requester
    uint8_t              con_lid;
    /// Targeted periodic advertiser
    app_gaf_per_adv_bdaddr_t addr;
} app_gaf_bc_scan_pa_synchronize_req_ind_t;

typedef struct
{
    /// Indication code (@see enum bap_bc_scan_req_ind_codes)
    /// - BAP_BC_SCAN_PA_TERMINATE_RI
    uint16_t             req_ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Source local index
    uint8_t              src_lid;
    /// Connection Local Identifier of requester
    uint8_t              con_lid;
} app_gaf_bc_scan_pa_terminate_req_ind_t;

/// Broadcast group configuration structure (provided by controller after stream establisment)
typedef struct
{
    /// The maximum delay time, in microseconds, for transmission of SDUs of all BISes
    /// (in us range 0x0000EA-0x7FFFFF)
    uint32_t tlatency_us;
    /// ISO interval in frames\n
    /// From 5ms to 4s
    uint16_t iso_interval_frames;
    /// The number of subevents in each BIS event in the BIG, range 0x01-0x1E
    uint8_t nse;
    /// The number of new payloads in each BIS event, range 0x01-0x07
    uint8_t bn;
    /// Offset used for pre-transmissions, range 0x00-0x0F
    uint8_t pto;
    /// The number of times a payload is transmitted in a BIS event, range 0x01-0x0F
    uint8_t irc;
    /// Maximum size of the payload in octets, range 0x00-0xFB
    uint8_t max_pdu;
} app_gaf_iap_bg_config_t;

/// Structure for BAP_BC_SINK_STATUS indication message
typedef struct
{
    /// Indication code (see #bap_bc_sink_msg_ind_code enumeration)
    /// - BAP_BC_SINK_STATUS
    uint16_t ind_code;
    /// Group local index
    uint8_t grp_lid;
    /// Broadcast Sink state (see #bap_bc_sink_state enumeration)
    uint8_t state;
    /// Stream position bit field indicating Stream with which synchronization is established
    /// Meaningful only if synchronization has been established
    uint32_t stream_pos_bf;
    /// When sync established, provides information about broadcast group else meaningless
    app_gaf_iap_bg_config_t bg_cfg;
    /// Number of BISes synchronization has been established with
    /// Meaningful only if synchronization has been established
    uint8_t nb_bis;
    /// List of Connection Handle values provided by the Controller (nb_bis elements)
    uint16_t conhdl[__ARRAY_EMPTY];
} app_gaf_bc_sink_status_ind_t;

/// Structure for BAP_BC_DELEG_SOLICITE_STOPPED indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_deleg_ind_code)
    ///  - BAP_BC_DELEG_SOLICITE_STOPPED
    uint16_t ind_code;
    /// Reason why sending of solicitation request has been stopped (@see enum bap_bc_deleg_reason)
    uint16_t reason;
} app_gaf_bc_deleg_solicite_stopped_ind_t;

/// Structure for BAP_BC_DELEG_BOND_REMOTE_SCAN indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_deleg_ind_code)
    ///  - BAP_BC_DELEG_BOND_REMOTE_SCAN
    uint16_t ind_code;
    /// Connection local index
    uint8_t  con_lid;
    /// Broadcast Assistant device scan state (@see enum bap_bc_deleg_scan_state)
    uint8_t  state;
} app_gaf_bc_deleg_bond_remote_scan_ind_t;

/// Structure for BAP_BC_DELEG_SOURCE_ADD_RI request indication message
typedef struct
{
    /// Request Indication code (set to #BAP_BC_DELEG_ADD_SOURCE_RI)
    uint16_t req_ind_code;
    /// Allocated Source local index
    uint8_t src_lid;
    /// Connection local index for device that has added the source
    uint8_t con_lid;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// Periodic Advertising interval in frames\n
    /// From 0x0006 to 0xFFFE\n
    /// BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
    uint16_t pa_intv_frames;
    /// Requested synchronization state for Periodic Advertising
    uint8_t pa_sync_req;
    /// Number of Subgroups
    uint8_t nb_subgroups;
    /// Subgroup information\n
    /// Must be casted as an array of #bap_bc_sgrp_info_t structures. Each structure has a variable size
    uint32_t subgroup_info[__ARRAY_EMPTY];
} app_gaf_bc_deleg_source_add_req_ind_t;

/// Structure for BAP_BC_DELEG_SOURCE_REMOVE_RI request indication message
typedef struct
{
    /// Request Indication code (@see enum bap_bc_deleg_req_ind_code)
    ///  - BAP_BC_DELEG_SOURCE_REMOVE_RI
    uint16_t             req_ind_code;
    /// Source local index
    uint8_t              src_lid;
    /// Connection local index
    uint8_t              con_lid;
} app_gaf_bc_deleg_source_remove_req_ind_t;

/// Structure for BAP_BC_DELEG_SOURCE_UPDATE_RI request indication message
typedef struct
{
    /// Request Indication code (@see enum bap_bc_deleg_req_ind_code)
    ///  - BAP_BC_DELEG_SOURCE_UPDATE_RI
    uint16_t             req_ind_code;
    /// Source local index
    uint8_t              src_lid;
    /// Connection local index
    uint8_t              con_lid;
    /// Metadata in LTV format
    app_gaf_ltv_t        metadata;
} app_gaf_bc_deleg_source_update_req_ind_t;

/// Structure for command complete event message
typedef struct
{
    /// Command code (@see enum acc_cmd_codes)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    union
    {
        /// Additional parameter
        uint8_t param;
        /// Characteristic type
        uint8_t char_type;
        /// Operation code
        uint8_t opcode;
    } u;
    /// Result
    uint8_t result;
} app_gaf_acc_mcc_cmp_evt_t;

/// Structure for ACC_MCC_TRACK_CHANGED indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
} app_gaf_acc_mcc_track_changed_ind_t;

/// Structure for ACC_MCC_VALUE indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Value
    union
    {
        /// Value
        uint32_t val;
        /// Playback speed
        int8_t playback_speed;
        /// Seeking speed
        int8_t seeking_speed;
        /// Media state
        uint8_t state;
        /// Supported media control operation codes bit field
        uint32_t opcodes_supp_bf;
        /// Playing Order
        uint8_t playing_order;
        /// Supported Playing Order bit field
        uint32_t playing_order_supp_bf;
        /// Track duration of the current track in 0.01 second resolution
        int32_t track_dur;
        /// Track position of the current track in 0.01 second resolution
        int32_t track_pos;
        /// Content Control ID
        uint8_t ccid;
    } val;
} app_gaf_acc_mcc_value_ind_t;

/// Structure for ACC_MCC_VALUE_LONG indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Length of value
    uint16_t val_len;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} app_gaf_acc_mcc_value_long_ind_t;

/// Structure for ACC_MCC_SVC_CHANGED indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_acc_mcc_svc_changed_ind_t;

/// Structure for ACC_MCC_BOND_DATA indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;

    acc_mcc_mcs_info_t mcs_info;
} app_gaf_acc_mcc_bond_data_ind_t;

/// Structure for command complete event message
typedef struct
{
    /// Command code (@see enum acc_cmd_codes)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    union
    {
        /// Value
        uint8_t val;
        /// Characteristic type
        uint8_t char_type;
        /// Operation code
        uint8_t opcode;
    } u;
    /// Call index
    uint8_t call_id;
    /// Result
    uint8_t result;
} app_gaf_acc_tbc_cmp_evt_t;

/// Structure for ACC_TBC_CALL_STATE indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t id;
    /// Call flags
    uint8_t flags;
    /// Call state
    uint8_t state;
} app_gaf_acc_tbc_call_state_ind_t;

/// Structure for ACC_TBC_CALL_STATE_LONG indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t id;
    /// Call flags
    uint8_t flags;
    /// Call state, @see enum acc_tb_call_state
    uint8_t state;
    /// Length of Incoming or Outgoing Call URI value
    uint8_t uri_len;
    /// Incoming or Outgoing Call URI value
    uint8_t uri[__ARRAY_EMPTY];
} app_gaf_acc_tbc_call_state_long_ind_t;

/// Structure for ACC_TBC_VALUE indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    /// Meaningful only for Termination Reason characteristic
    uint8_t call_id;
    /// Characteristic type
    uint8_t char_type;
    /// Value
    union
    {
        /// Value
        uint16_t val;
        /// Bearer Technology
        uint8_t techno;
        /// Signal Strength
        uint8_t sign_strength;
        /// Signal Strength Reporting Interval in seconds
        uint8_t sign_strength_intv_s;
        /// Content Control ID
        uint8_t ccid;
        /// Status Flags bit field
        uint16_t status_flags_bf;
        /// Call Control Point Optional Opcodes bit field
        uint16_t opt_opcodes_bf;
        /// Termination Reason
        uint8_t term_reason;
    } val;
} app_gaf_acc_tbc_value_ind_t;

/// Structure for ACC_TBC_VALUE_LONG indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t call_id;
    /// Characteristic type
    uint8_t char_type;
    /// Length of value
    uint16_t val_len;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} app_gaf_acc_tbc_value_long_ind_t;

/// Structure for ACC_TBC_SVC_CHANGED indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_acc_tbc_svc_changed_ind_t;

/// Structure for ACC_TBC_SVC_BOND_DATA indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;

    acc_tbc_tbs_info_t tbs_info;
} app_gaf_acc_tbc_bond_data_ind_t;

/// Structure for ARC_AICS_STATE indication message
typedef struct
{
    /// Indication code (@see enum arc_aics_ind_code)
    ///  - ARC_AICS_STATE
    uint16_t ind_code;
    /// Input local index
    uint8_t input_lid;
    /// Gain
    int8_t gain;
    /// Gain Mode
    uint8_t gain_mode;
    /// Mute
    uint8_t mute;
} app_gaf_arc_aics_state_ind_t;

/// Structure for ARC_MICS_MUTE indication message
typedef struct
{
    /// Indication code (@see enum arc_mics_ind_code)
    ///  - ARC_MICS_MUTE
    uint16_t ind_code;
    /// Mute value
    uint8_t mute;
} app_gaf_arc_mics_mute_ind_t;

/// Structure for ARC_MICC_MUTE indication message
typedef struct
{
    /// Indication code (@see enum arc_micc_ind_code)
    ///  - ARC_MICC_MUTE
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Mute value
    uint8_t mute;
} app_gaf_arc_micc_mute_ind_t;

/// Structure for ARC_VCS_VOLUME indication message
typedef struct
{
    /// Indication code (@see enum arc_vcs_ind_code)
    ///  - ARC_VCS_VOLUME
    uint16_t ind_code;
    /// Volume
    uint8_t volume;
    /// Mute
    uint8_t mute;
#ifdef BLE_STACK_PORTING_CHANGES
    /// Reason
    uint8_t reason;
#endif
} app_gaf_arc_vcs_volume_ind_t;

/// Structure for ARC_VCS_FLAGS indication message
typedef struct
{
    /// Indication code (@see enum arc_vcs_ind_code)
    ///  - ARC_VCS_FLAGS
    uint16_t ind_code;
    /// Volume Flags
    uint8_t flags;
} app_gaf_arc_vcs_flags_ind_t;

// @arc_vcs_bond_ind
typedef struct{
    /// Indication code (shall be set to #ARC_VCS_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
    // Characteristic type
    uint8_t char_type;
} app_gaf_arc_vcs_bond_data_ind_t;

/// Structure for ARC_VOCS_SET_LOCATION request indication message
typedef struct
{
    /// Request Indication code (@see enum arc_vocs_req_ind_code)
    ///  - ARC_VOCS_SET_LOCATION
    uint16_t req_ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Audio location
    uint8_t location;
} app_gaf_arc_vocs_set_location_req_ind_t;

/// Structure for ARC_VOCS_BOND_DATA indication message
typedef struct
{
    /// Indication code (@see enum arc_vocs_ind_code)
    ///  - ARC_VOCS_BOND_DATA
    uint16_t ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} app_gaf_arc_vocs_cfg_ind_t;

/// Structure for ARC_VOCS_OFFSET indication message
typedef struct
{
    /// Indication code (@see enum arc_vocs_ind_code)
    ///  - ARC_VOCS_OFFSET
    uint16_t ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Offset
    int16_t offset;
} app_gaf_arc_vocs_offset_ind_t;

/// Structure for CSISM_LOCK indication message
typedef struct
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// New lock state
    uint8_t lock;
    /// Connection local index of connection for which Coordinated Set has been locked
    /// or was locked
    uint8_t con_lid;
    /// Reason why Coordinated Set is not locked anymore
    uint8_t reason;
} app_gaf_atc_csism_lock_ind_t;

/// Structure for CSISM_AUTHORIZATION request indication message
typedef struct
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_atc_csism_ltk_req_ind_t;

typedef struct
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} app_gaf_atc_csism_bond_data_ind_t;

/// Structure CSISM_ADD/CSISM_SET_SIRK/CSISM_UPDATE_PSRI command complete event
typedef struct
{
    /// Command code
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Coordinated Set local index
    uint8_t set_lid;
} app_gaf_atc_csism_cmp_evt_t;

/// Structure for CSISM_RSI indication message
typedef struct
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// RSI value
    csis_rsi_t rsi;
} app_gaf_atc_sism_rsi_ind_t;

typedef struct {
    uint16_t ind_code;
    uint8_t con_lid;
    uint8_t char_type;
} app_gaf_atc_csism_ntf_sent_t;

typedef struct {
    uint16_t ind_code;
    uint8_t con_lid;
    uint8_t char_type;
    uint8_t data_len;
    uint8_t data[CSIS_SIRK_LEN_VALUE];
} app_gaf_atc_csism_read_rsp_sent_t;

/// Structure command complete event
typedef struct
{
    /// Request Indication code (see enum #dts_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dts_registerd_ind_t;

/// Structure for DTC_COC_CONNECTED indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t tx_mtu;
    /// Maximum packet size that the peer on the link can receive
    uint16_t tx_mps;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// initial credits
    uint16_t initial_credits;
} app_gaf_dts_coc_connected_ind_t;

/// Structure for DTC_COC_DISCONNECTED indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Disconnection reason
    uint16_t reason;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dts_coc_disconnected_ind_t;

/// Structure for DTC_COC_DATA indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// SDU data length
    uint16_t length;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// SDU data
    uint8_t sdu[__ARRAY_EMPTY];
} app_gaf_dts_coc_data_ind_t;

typedef struct
{
    /// Command code (see enum #ots_cmd_codes)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dts_cmp_evt_t;

typedef struct
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t peer_max_sdu;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dts_coc_connect_req_ind_t;

/// CIS stream state
typedef enum
{
    APP_GAF_CIS_STREAM_IDLE             = 0,
    APP_GAF_CIS_STREAM_CODEC_CONFIGURED = 1,
    APP_GAF_CIS_STREAM_QOS_CONFIGURED   = 2,
    APP_GAF_CIS_STREAM_ENABLING         = 3,
    APP_GAF_CIS_STREAM_DISABLING        = 4,
    APP_GAF_CIS_STREAM_STREAMING        = 5,
    APP_GAF_CIS_STREAM_RELEASING        = 6,

} APP_GAF_CIS_STREAM_STATE_E;

/// Structure for APP_GAF_CIS_CLI_STREAM_STATE_UPDATED and
/// APP_GAF_CIS_SRV_STREAM_STATE_UPDATED event indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// Former state
    APP_GAF_CIS_STREAM_STATE_E formerState;;
    /// Current state
    APP_GAF_CIS_STREAM_STATE_E currentState;
} app_gaf_cis_stream_state_updated_ind_t;

typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field for Audio Stream Control Service\n
    /// Each bit correspond to a characteristic in the range [0, BAP_UC_CHAR_TYPE_ASE[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for instances of the ASE characteristics\n
    /// Each bit correspond to an instance of the ASE characteristic
    uint16_t ase_cli_cfg_bf;
} app_gaf_bap_uc_srv_bond_data_ind_t;

typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_CIS_REJECTED)
    uint16_t ind_code;
    /// Connection handle of Connected Isochronous Stream
    uint16_t con_hdl;
    /// Reject reason
    uint8_t error;
} app_gaf_bap_uc_srv_cis_rejected_ind_t;

typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_CIG_TERMINATED)
    uint16_t ind_code;
    /// CIG ID
    uint8_t cig_id;
    /// Group local index
    uint8_t group_lid;
    /// Stream local index
    uint8_t stream_lid;
    /// Cig terminated reason
    uint8_t reason;
} app_gaf_bap_uc_srv_cig_terminated_ind_t;

typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_ASE_NTF_VALUE)
    uint16_t ind_code;
    /// Opcode of the client-initiated ASE Control operation causing this response
    uint8_t opcode;
    /// Total number of ASEs the server is providing a response for
    uint8_t nb_ases;
    /// ASE Local ID for this ASE
    uint8_t ase_lid;
    /// Response code
    uint8_t rsp_code;
    /// Reason
    uint8_t reason;
} app_gaf_bap_uc_srv_ase_ntf_value_ind_t;

/// Structure for response message
typedef struct
{
    /// Request code (@see enum bap_uc_cli_req_code)
    uint16_t req_code;
    /// Status
    uint16_t status;
    union
    {
        /// Local index
        uint8_t lid;
        /// Connection local index
        uint8_t con_lid;
        /// Group local index
        uint8_t grp_lid;
    } lid;
    /// ASE local index
    uint8_t ase_lid;
} app_gaf_uc_cli_rsp_t;

typedef enum {
    APP_GAF_ASCS_MODULE             = 0,
    APP_GAF_PACS_MODULE             = 1,
    APP_GAF_BIS_SCAN_MODULE         = 2,
    APP_GAF_BIS_SINK_MODULE         = 3,
    APP_GAF_DELEG_MODULE            = 4,
    APP_GAF_MCC_MODULE              = 5,
    APP_GAF_TBC_MODULE              = 6,
    APP_GAF_AICS_MODULE             = 7,
    APP_GAF_MICS_MODULE             = 8,
    APP_GAF_VCS_MODULE              = 9,
    APP_GAF_VOCS_MODULE             = 10,
    APP_GAF_CSISM_MODULE            = 11,
    APP_GAF_BC_SCAN_STATE_MODULE    = 12,
    APP_GAF_DTC_MODULE              = 13,
    APP_GAF_DTS_MODULE              = 14,

    APP_GAF_MAX_MODULE,
} app_gaf_evt_module_e;

/// earbuds event on GAF layer
// event only can use low 12 bits
typedef enum
{
    APP_GAF_EVENT_FIRST = 0x000,

    // ASCS Events
    APP_GAF_ASCS_CIS_ESTABLISHED_IND        = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x0),
    APP_GAF_ASCS_CIS_DISCONNETED_IND        = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x1),
    APP_GAF_ASCS_CIS_STREAM_STARTED_IND     = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x2),
    APP_GAF_ASCS_CIS_STREAM_STOPPED_IND     = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x3),
    APP_GAF_ASCS_CONFIGURE_CODEC_RI         = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x4),
    APP_GAF_ASCS_ENABLE_RI                  = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x5),
    APP_GAF_ASCS_UPDATE_METADATA_RI         = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x6),
    APP_GAF_ASCS_RELEASE_RI                 = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x7),
    APP_GAF_ASCS_CLI_STREAM_STATE_UPDATED   = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x8),
    APP_GAF_ASCS_ISO_LINK_QUALITY_EVT       = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x9),
    APP_GAF_ASCS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0xA),
    APP_GAF_ASCS_CIS_REJECTED_IND           = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0xB),
    APP_GAF_ASCS_CIG_TERMINATED_IND         = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0xC),
    APP_GAF_ASCS_ASE_NTF_VALUE_IND          = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0xD),


    // PACS Event
    APP_GAF_PACS_LOCATION_SET_IND           = GAF_BUILD_ID(APP_GAF_PACS_MODULE, 0x0),
    APP_GAF_PACS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_PACS_MODULE, 0x1),
    APP_GAF_PACS_CCCD_WRITTEN_IND           = GAF_BUILD_ID(APP_GAF_PACS_MODULE, 0x2),

    // BIS SCAN Events
    APP_GAF_SCAN_TIMEOUT_IND                = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x0),
    APP_GAF_SCAN_REPORT_IND                 = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x1),
    APP_GAF_SCAN_PA_REPORT_IND              = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x2),
    APP_GAF_SCAN_PA_ESTABLISHED_IND         = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x3),
    APP_GAF_SCAN_PA_TERMINATED_IND          = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x4),
    APP_GAF_SCAN_GROUP_REPORT_IND           = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x5),
    APP_GAF_SCAN_SUBGROUP_REPORT_IND        = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x6),
    APP_GAF_SCAN_STREAM_REPORT_IND          = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x7),
    APP_GAF_SCAN_BIGINFO_REPORT_IND         = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x8),
    APP_GAF_SCAN_PA_SYNC_REQ_IND            = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x9),
    APP_GAF_SCAN_PA_TERMINATED_REQ_IND      = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0xa),
    APP_GAF_SCAN_STOPPED_IND                = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0xb),

    // BIS Sink Events
    APP_GAF_SINK_BIS_STATUS_IND             = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x0),
    APP_GAF_SINK_BIS_SINK_ENABLED_IND       = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x1),
    APP_GAF_SINK_BIS_SINK_DISABLED_IND      = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x2),
    APP_GAF_SINK_BIS_STREAM_STARTED_IND     = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x3),
    APP_GAF_SINK_BIS_STREAM_STOPPED_IND     = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x4),

    // BIS Delegator Events
    APP_GAF_DELEG_SOLICITE_STARTED_IND      = GAF_BUILD_ID(APP_GAF_DELEG_MODULE, 0x0),
    APP_GAF_DELEG_SOLICITE_STOPPED_IND      = GAF_BUILD_ID(APP_GAF_DELEG_MODULE, 0x1),
    APP_GAF_DELEG_REMOTE_SCAN_STARTED_IND   = GAF_BUILD_ID(APP_GAF_DELEG_MODULE, 0x2),
    APP_GAF_DELEG_REMOTE_SCAN_STOPPED_IND   = GAF_BUILD_ID(APP_GAF_DELEG_MODULE, 0x3),
    APP_GAF_DELEG_SOURCE_ADD_RI             = GAF_BUILD_ID(APP_GAF_DELEG_MODULE, 0x4),
    APP_GAF_DELEG_SOURCE_REMOVE_RI          = GAF_BUILD_ID(APP_GAF_DELEG_MODULE, 0x5),
    APP_GAF_DELEG_SOURCE_UPDATE_RI          = GAF_BUILD_ID(APP_GAF_DELEG_MODULE, 0x6),

    // MCC Events
    APP_GAF_MCC_SVC_DISCOVERYED_IND         = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x0),
    APP_GAF_MCC_TRACK_CHANGED_IND           = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x1),
    APP_GAF_MCC_MEDIA_VALUE_IND             = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x2),
    APP_GAF_MCC_MEDIA_VALUE_LONG_IND        = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x3),
    APP_GAF_MCC_SVC_CHANGED_IND             = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x4),
    APP_GAF_MCC_BOND_DATA_IND               = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x5),

    // TBC Events
    APP_GAF_TBC_SVC_DISCOVERYED_IND         = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x0),
    APP_GAF_TBC_CALL_STATE_IND              = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x1),
    APP_GAF_TBC_CALL_STATE_LONG_IND         = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x2),
    APP_GAF_TBC_CALL_VALUE_IND              = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x3),
    APP_GAF_TBC_CALL_VALUE_LONG_IND         = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x4),
    APP_GAF_TBC_SVC_CHANGED_IND             = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x5),
    APP_GAF_TBC_CALL_ACTION_RESULT_IND      = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x6),
    APP_GAF_TBC_BOND_DATA_IND               = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x7),

    // AICS Event
    APP_GAF_AICS_STATE_IND                  = GAF_BUILD_ID(APP_GAF_AICS_MODULE, 0x0),
    APP_GAF_AICS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_AICS_MODULE, 0x1),

    // MICS Event
    APP_GAF_MICS_MUTE_IND                   = GAF_BUILD_ID(APP_GAF_MICS_MODULE, 0x0),
    APP_GAF_MICS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_MICS_MODULE, 0x1),

    // VCS Events
    APP_GAF_VCS_VOLUME_IND                  = GAF_BUILD_ID(APP_GAF_VCS_MODULE, 0x0),
    APP_GAF_VCS_FLAGS_IND                   = GAF_BUILD_ID(APP_GAF_VCS_MODULE, 0x1),
    APP_GAF_VCS_BOND_DATA_IND               = GAF_BUILD_ID(APP_GAF_VCS_MODULE, 0x2),

    // VOCS Events
    APP_GAF_VOCS_LOCATION_SET_RI            = GAF_BUILD_ID(APP_GAF_VOCS_MODULE, 0x0),
    APP_GAF_VOCS_OFFSET_IND                 = GAF_BUILD_ID(APP_GAF_VOCS_MODULE, 0x1),
    APP_GAF_VOCS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_VOCS_MODULE, 0x2),

    // CSISM Events
    APP_GAF_CSISM_LOCK_IND                  = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x0),
    APP_GAF_CSISM_LTK_RI                    = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x1),
    APP_GAF_CSISM_NEW_RSI_GENERATED_IND     = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x2),
    APP_GAF_CSISM_BOND_DATA_IND             = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x3),
    APP_GAF_CSISM_NTF_SENT_IND              = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x4),
    APP_GAF_CSISM_READ_RSP_SENT_IND         = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x5),

    // BIS scan state update
    APP_BAP_BC_SCAN_STATE_IDLE_IND          = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x0),
    APP_BAP_BC_SCAN_STATE_SCANNING_IND      = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x1),
    APP_BAP_BC_SCAN_STATE_SYNCHRONIZING_IND = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x2),
    APP_BAP_BC_SCAN_STATE_SYNCHRONIZED_IND  = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x3),
    APP_BAP_BC_SCAN_STATE_STREAMING_IND     = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x4),

    // DTS event
    APP_DTS_COC_REGISTERED_IND                  = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x0),
    APP_DTS_COC_CONNECTED_IND                   = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x1),
    APP_DTS_COC_DISCONNECTED_IND                = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x2),
    APP_DTS_COC_DATA_IND                        = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x3),
    APP_DTS_COC_SEND_IND                        = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x4),

    APP_GAF_EVENT_LAST                      = 0xFFFF
} app_gaf_evt_e;

/// mobile event on GAF layer
#ifdef AOB_MOBILE_ENABLED

typedef enum {
    APP_GAF_ASCC_MODULE         = 0,
    APP_GAF_PACC_MODULE         = 1,
    APP_GAF_ASSIST_MODULE       = 2,
    APP_GAF_MCS_MODULE          = 3,
    APP_GAF_TBS_MODULE          = 4,
    APP_GAF_AICC_MODULE         = 5,
    APP_GAF_MICC_MODULE         = 6,
    APP_GAF_BIS_SOURCE_MODULE   = 7,
    APP_GAF_VCC_MODULE          = 8,
    APP_GAF_VOCC_MODULE         = 9,
    APP_GAF_CSISC_MODULE        = 10,
    APP_GAF_MAX_MOBILE_EVENT_MODULE,
} app_gaf_mobile_evt_module_e;

// event only can use low 12 bits
typedef enum {
    APP_GAF_EVENT_MOBILE_FIRST              = 0x0000,

    // ASCC Events
    APP_GAF_ASCC_CIS_ESTABLISHED_IND        = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x0),
    APP_GAF_ASCC_CIS_DISCONNETED_IND        = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x1),
    APP_GAF_ASCC_CIS_STREAM_STARTED_IND     = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x2),
    APP_GAF_ASCC_CIS_STREAM_STOPPED_IND     = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x3),
    APP_GAF_ASCC_CIS_STREAM_STATE_UPDATED   = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x4),
    APP_GAF_ASCC_ASE_FOUND_IND              = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x5),
    APP_GAF_ASCC_CIS_GRP_STATE_IND          = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x6),
    APP_GAF_ASCC_CMD_CMP_IND                = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x7),

    APP_GAF_PACC_PAC_RECORD_IND             = GAF_BUILD_ID(APP_GAF_PACC_MODULE, 0x0),
    APP_GAF_PACC_LOCATION_IND               = GAF_BUILD_ID(APP_GAF_PACC_MODULE, 0x1),
    APP_GAF_PACC_OPERATION_CMP_IND          = GAF_BUILD_ID(APP_GAF_PACC_MODULE, 0x2),
    APP_GAF_PACC_CONTEXT_IND                = GAF_BUILD_ID(APP_GAF_PACC_MODULE, 0x3),

    APP_GAF_ASSIST_SCAN_TIMEOUT_IND         = GAF_BUILD_ID(APP_GAF_ASSIST_MODULE, 0x0),
    APP_GAF_ASSIST_SOLICITATION_IND         = GAF_BUILD_ID(APP_GAF_ASSIST_MODULE, 0x1),
    APP_GAF_ASSIST_SOURCE_STATE_IND         = GAF_BUILD_ID(APP_GAF_ASSIST_MODULE, 0x2),
    APP_GAF_ASSIST_BCAST_CODE_RI            = GAF_BUILD_ID(APP_GAF_ASSIST_MODULE, 0x3),
    APP_GAF_ASSIST_BOND_DATA_IND            = GAF_BUILD_ID(APP_GAF_ASSIST_MODULE, 0X4),

    APP_GAF_MCS_CONTROL_REQ_IND             = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x0),
    APP_GAF_MCS_SET_OBJ_ID_RI               = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x1),
    APP_GAF_MCS_CONTROL_RI                  = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x2),
    APP_GAF_MCS_SEARCH_RI                   = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x3),
    APP_GAF_MCS_GET_RI                      = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x4),
    APP_GAF_MCS_GET_POSITION_RI             = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x5),
    APP_GAF_MCS_SET_RI                      = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x6),

    APP_GAF_TBS_REPORT_INTV_IND             = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x0),
    APP_GAF_TBS_GET_RI                      = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x1),
    APP_GAF_TBS_CALL_OUTGOING_RI            = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x2),
    APP_GAF_TBS_CALL_ACTION_RI              = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x3),
    APP_GAF_TBS_CALL_JOIN_RI                = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x4),
    APP_GAF_TBS_BOND_DATA_IND               = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x5),
    APP_GAF_TBS_CALL_ACTION_REQ_RSP         = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x6),

    APP_GAF_AICC_GAIN_IND                   = GAF_BUILD_ID(APP_GAF_AICC_MODULE, 0x0),
    APP_GAF_AICC_GAIN_PROP_IND              = GAF_BUILD_ID(APP_GAF_AICC_MODULE, 0x1),
    APP_GAF_AICC_VALUE_IND                  = GAF_BUILD_ID(APP_GAF_AICC_MODULE, 0x2),

    APP_GAF_MICC_MUTE_IND                   = GAF_BUILD_ID(APP_GAF_MICC_MODULE, 0x0),

    // BIS Source Events
    APP_GAF_SRC_BIS_PA_ENABLED_IND          = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0X0),
    APP_GAF_SRC_BIS_PA_DISABLED_IND         = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0X1),
    APP_GAF_SRC_BIS_SRC_ENABLED_IND         = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0X2),
    APP_GAF_SRC_BIS_SRC_DISABLED_IND        = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0X3),
    APP_GAF_SRC_BIS_STREAM_STARTED_IND      = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0x4),
    APP_GAF_SRC_BIS_STREAM_STOPPED_IND      = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0x5),

    // VCC Events
    APP_GAF_VCC_VOLUME_IND                  = GAF_BUILD_ID(APP_GAF_VCC_MODULE, 0x0),

    // VOCC Events
    APP_GAF_VOCC_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_VOCC_MODULE, 0x0),
    APP_GAF_VOCC_VALUE_IND                  = GAF_BUILD_ID(APP_GAF_VOCC_MODULE, 0x1),

    // CSISC Events
    APP_GAF_CSISC_BOND_IND                  = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x0),
    APP_GAF_CSISC_SIRK_VALUE_IND            = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x1),
    APP_GAF_CSISC_CHAR_VALUE_RSULT_IND      = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x2),
    APP_GAF_CSISC_PSRI_RESOLVE_RESULT_IND   = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x3),
    APP_GAF_CSISC_SERVER_INIT_DONE_CMP_IND  = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x4),
    APP_GAF_CSISC_SIRK_ADD_RESULT_IND       = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x5),
    APP_GAF_CSISC_SIRK_REMOVE_RESULT_IND    = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x6),

    // DTC event
    APP_DTC_COC_CONNECTED_IND                   = GAF_BUILD_ID(APP_GAF_DTC_MODULE, 0x0),
    APP_DTC_COC_DISCONNECTED_IND                = GAF_BUILD_ID(APP_GAF_DTC_MODULE, 0x1),
    APP_DTC_COC_DATA_IND                        = GAF_BUILD_ID(APP_GAF_DTC_MODULE, 0x2),

    APP_GAF_EVENT_MOBILE_LAST               = 0xFFFF
} app_gaf_mobile_evt_e;

/// see @bap_uc_cli_cis_state_ind_t
typedef struct
{
    /// Indication code (set to #BAP_UC_CLI_CIS_STATE)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Connection local index of LE connection the CIS is bound with
    uint8_t con_lid;
    /// ASE local index for Sink direction
    uint8_t ase_lid_sink;
    /// ASE local index for Source direction
    uint8_t ase_lid_src;
    /// Group local index
    uint8_t grp_lid;
    /// CIS ID
    uint8_t cis_id;
    /// Connection handle allocated for the CIS by Controller
    /// GAP_INVALID_CONHDL indicates that CIS has been lost
    uint16_t conhdl;
    /// Event that has triggered update of CIS state (see #bap_uc_cli_cis_event enumeration)
    uint8_t event;
    /// Group configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    app_gaf_iap_ug_config_t cig_config;
    /// Stream configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    app_gaf_iap_us_config_t cis_config;
} app_gaf_uc_cli_cis_state_ind_t;

/// Structure for cis stream started indication message
typedef struct
{
    uint8_t con_lid;
    uint8_t ase_lid;
    uint16_t cis_hdl;
    uint8_t direction;
} app_gaf_ascc_cis_stream_started_t;

/// Structure for cis stream stopped indication message
typedef struct
{
    uint8_t con_lid;
    uint8_t ase_lid;
    uint16_t cis_hdl;
    uint8_t direction;
} app_gaf_ascc_cis_stream_stopped_t;

/// Structure for BAP_CAPA_CLI_RECORD indication message
typedef struct
{
    /// Indication code (@see enum bap_capa_cli_ind_code)
    ///  - BAP_CAPA_CLI_RECORD
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// PAC local index
    uint8_t pac_lid;
    /// Record local index
    uint8_t record_lid;
    /// Number of records
    uint8_t nb_records;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Length of Codec Capabilities value
    uint16_t capa_len;
    /// Codec Capabilities value
    uint8_t capa[__ARRAY_EMPTY];
} app_gaf_capa_cli_record_ind_t;


typedef struct
{
    uint8_t con_lid;
    uint16_t cmd_code;
} app_gaf_capa_operation_cmd_ind_t;

/// Structure for ACC_MCS_CONTROL request indication message
typedef struct
{
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Operation code
    uint8_t opcode;
} app_gaf_mcs_control_req_ind;

/// Structure for ARC_VOCC_BOND_DATA indication message
typedef struct
{
    /// Indication code (@see enum arc_vocc_ind_code)
    ///  - ARC_VOCC_BOND_DATA
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
} app_gaf_arc_vocc_bond_data_ind_t;

/// Structure for ARC_VOCC_VALUE indication message
typedef struct
{
    /// Indication code (@see enum arc_vocc_ind_code)
    ///  - ARC_VOCC_VALUE
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Characteristic type
    uint8_t char_type;
    union
    {
        /// Value
        uint32_t val;
        /// Volume offset
        int16_t offset;
        /// Audio location
        uint32_t location;
    } u;
} app_gaf_arc_vocc_value_ind_t;

/// Structure for BAP_CAPA_CLI_CONTEXT indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// Context type (@see enum bap_capa_context_type)
    uint8_t context_type;
    /// Context type bit field for Sink direction
    uint16_t context_bf_sink;
    /// Context type bit field for Source direction
    uint16_t context_bf_src;
} app_gaf_capa_cli_context_ind_t;

/// Structure for BAP_CAPA_CLI_LOCATION indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// Direction (@see enum gaf_direction)
    uint8_t direction;
    /// Location bit field
    uint32_t location_bf;
} app_gaf_capa_cli_location_ind_t;

/// Service information structure
typedef struct
{
    /// start handle
    uint16_t shdl;
    /// end handle
    uint16_t ehdl;
} app_gaf_prf_svc_t;

typedef struct
{
    /// Characteristic value handle
    uint16_t val_hdl;
    /// Client Characteristic Configuration descriptor handle
    uint16_t desc_hdl;
    /// ASE ID
    uint8_t ase_id;
} app_gaf_uc_cli_ascs_char_t;

/// Audio Stream Control Service content description structure
typedef struct
{
    /// Service description
    app_gaf_prf_svc_t svc_info;
    /// Number of discovered Sink ASE characteristics
    uint8_t nb_ases_sink;
    /// Number of discovered Source ASE characteristics
    uint8_t nb_ases_src;
    /// Characteristic information structures
    app_gaf_uc_cli_ascs_char_t char_info[__ARRAY_EMPTY];
} app_gaf_uc_cli_ascs_t;

/// Structure for BAP_UC_CLI_BOND_DATA indication message
typedef struct
{
    /// Indication code (@see enum bap_uc_cli_ind_code)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Audio Stream Control Service content description
    app_gaf_uc_cli_ascs_t ascs_info;
} app_gaf_uc_cli_bond_data_ind_t;

/// Structure for ARC_VCC_VOLUME indication message
typedef struct
{
    /// Indication code (@see enum arc_vcc_ind_code)
    ///  - ARC_VCC_VOLUME
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Volume
    uint8_t volume;
    /// Mute
    uint8_t mute;
} app_gaf_vcc_volume_ind_t;

/// Structure for CSISC_DISCOVER_SERVER_COMPLETE indication message
typedef struct
{
    uint8_t con_lid;
    uint8_t result;
} app_gaf_csisc_discover_cmp_ind_t;

/// Structure for DTC_COC_CONNECTED indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t tx_mtu;
    /// Maximum packet size that the peer on the link can receive
    uint16_t tx_mps;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dtc_coc_connected_ind_t;

/// Structure for DTC_COC_DISCONNECTED indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Disconnection reason
    uint16_t reason;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dtc_coc_disconnected_ind_t;

/// Structure for DTC_COC_DATA indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// SDU data length
    uint16_t length;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// SDU data
    uint8_t sdu[__ARRAY_EMPTY];
} app_gaf_dtc_coc_data_ind_t;

#endif

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_GAF_DEFINE_H_

/// @} APP_GAF
