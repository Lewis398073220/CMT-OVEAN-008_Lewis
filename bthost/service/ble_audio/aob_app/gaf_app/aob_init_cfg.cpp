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

/*****************************header include********************************/
#include "aob_csip_api.h"
#include "app_tws_ibrt.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "app_trace_rx.h"
#include "plat_types.h"
#include "ble_audio_dbg.h"
#include "heap_api.h"
#include "tgt_hardware.h"

#include "aob_gaf_api.h"
#include "aob_pacs_api.h"
#include "app_bap.h"
#include "app_ibrt_if.h"
#include "gaf_media_stream.h"
#include "ble_audio_earphone_info.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/****************************PACS init CFG********************************/
#ifndef LC3PLUS_SUPPORT
#define CUSTOM_CAPA_NB_PAC_SINK                  (1)    // for lc3 sink
#define CUSTOM_CAPA_NB_PAC_SRC                   (1)    // for lc3 src
#else
#define CUSTOM_CAPA_NB_PAC_SINK                  (2)    // one for lc3 sink one for lc3plus
#define CUSTOM_CAPA_NB_PAC_SRC                   (2)    // for lc3 src one for lc3plus
#endif // LC3PLUS_SUPPORT
/****************************LC3 Codec CFG*********************************/
#define CUSTOM_CAPA_CODEC_ID                     "\x06\x00\x00\x00\x00"
#define CUSTOM_CAPA_ALL_SAMPLING_FREQ_BF         (AOB_SUPPORTED_SAMPLE_FREQ_48000 | AOB_SUPPORTED_SAMPLE_FREQ_24000 |\
                                                  AOB_SUPPORTED_SAMPLE_FREQ_32000 | AOB_SUPPORTED_SAMPLE_FREQ_16000 |\
                                                  AOB_SUPPORTED_SAMPLE_FREQ_8000)
#define CUSTOM_CAPA_SINK_CONTEXT_BF              (APP_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_BAP_CONTEXT_TYPE_MEDIA |\
                                                  APP_BAP_CONTEXT_TYPE_UNSPECIFIED | APP_BAP_CONTEXT_TYPE_LIVE |\
                                                  APP_BAP_CONTEXT_TYPE_GAME | APP_BAP_CONTEXT_TYPE_UNSPECIFIED)
#define CUSTOM_CAPA_SRC_CONTEXT_BF               (APP_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_BAP_CONTEXT_TYPE_MEDIA |\
                                                  APP_BAP_CONTEXT_TYPE_MAN_MACHINE | APP_BAP_CONTEXT_TYPE_LIVE |\
                                                  APP_BAP_CONTEXT_TYPE_GAME | APP_BAP_CONTEXT_TYPE_UNSPECIFIED)
#define CUSTOM_CAPA_PREFERRED_CONTEXT_BF         (APP_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_BAP_CONTEXT_TYPE_MEDIA)
#ifdef BLE_AUDIO_FRAME_DUR_7_5MS
#define CUSTOM_CAPA_FRAME_DURATION_BF            (AOB_SUPPORTED_FRAME_DURATION_7_5MS | AOB_SUPPORTED_FRAME_DURATION_10MS\
                                                    | AOB_PREFERRED_FRAME_DURATION_7_5MS)
#else
#define CUSTOM_CAPA_FRAME_DURATION_BF            (AOB_SUPPORTED_FRAME_DURATION_7_5MS | AOB_SUPPORTED_FRAME_DURATION_10MS)
#endif // BLE_AUDIO_FRAME_DUR_7_5MS

#ifdef LC3PLUS_SUPPORT
/**************************LC3Plus Codec CFG******************************/
#define CUSTOM_CAPA_LC3PLUS_CODEC_ID             "\xFF\x08\xA9\x00\x02"
#define CUSTOM_CAPA_LC3PLUS_SAMPLING_FREQ_BF     (AOB_SUPPORTED_SAMPLE_FREQ_48000 | AOB_SUPPORTED_SAMPLE_FREQ_96000)
#define CUSTOM_CAPA_LC3PLUS_FRAME_OCT_MIN        (20)
#define CUSTOM_CAPA_LC3PLUS_FRAME_OCT_MAX        (190)
#define CUSTOM_CAPA_LC3PLUS_PREFERRED_CONTEXT_BF (APP_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_BAP_CONTEXT_TYPE_MEDIA)
#define CUSTOM_CAPA_LC3PLUS_FRAME_DURATION_BF    (AOB_SUPPORTED_FRAME_DURATION_10MS | AOB_SUPPORTED_FRAME_DURATION_5MS\
                                                    |AOB_SUPPORTED_FRAME_DURATION_2_5MS)
/****************************LC3Plus Qos CFG******************************/
#define QOS_SETTING_LC3PLUS_MAX_TRANS_DELAY      (100)
#define QOS_SETTING_LC3PLUS_MAX_RTX_NUMER        (13)
#define QOS_SETTING_LC3PLUS_DFT_PRESDELAY        (40000)
#endif // LC3PLUS_SUPPORT

/*****************************USB DONGLE**********************************/
#if defined(BLE_USB_AUDIO_SUPPORT) || defined(AOB_LOW_LATENCY_MODE)
#define APP_BAP_DFT_ASCS_RTN                     (13)
#define APP_BAP_DFT_ASCS_MIN_PRES_DELAY_US       (5000)
#define APP_BAP_DFT_ASCS_MAX_PRES_DELAY_US       (5000)
#define APP_BAP_DFT_ASCS_MAX_TRANS_LATENCY_MS    (100)
#define APP_BAP_DFT_ASCS_FRAMING_TYPE            (APP_ISO_UNFRAMED_MODE)
#define APP_BAP_DFT_ASCS_PHY_BF                  (APP_PHY_2MBPS_VALUE)
#endif // BLE_USB_AUDIO_SUPPORT

/// GMAP defines 10000 to includes in [codec cfg-qos req ntf] presdelay range
#if defined (AOB_GMAP_ENABLED)
#define QOS_SETTING_DEFAULT_MIN_PRESDELAY       (10000)
#else
#define QOS_SETTING_DEFAULT_MIN_PRESDELAY       (40000)
#endif

#define QOS_SETTING_DEFAULT_MAX_PRESDELAY       (40000)

/***************************AUDIO CONFIGURATION****************************/
#define AOB_AUD_LOCATION_WITH_ROLE               (0xFFFF)
#define AOB_AUD_LOCATION_LEFT                    (AOB_SUPPORTED_LOCATION_FRONT_LEFT | AOB_SUPPORTED_LOCATION_SIDE_LEFT)
#define AOB_AUD_LOCATION_RIGHT                   (AOB_SUPPORTED_LOCATION_SIDE_RIGHT | AOB_SUPPORTED_LOCATION_FRONT_RIGHT)
#define AOB_AUD_LOCATION_ALL_SUPP                (AOB_AUD_LOCATION_LEFT | AOB_AUD_LOCATION_RIGHT)
#define AOB_AUD_CHAN_CNT_MONO                    (0x1)
#define AOB_AUD_CHAN_CNT_STEREO_ONLY             (0x2)
#define AOB_AUD_CHAN_CNT_ALL_SUPP                (AOB_AUD_CHAN_CNT_MONO | AOB_AUD_CHAN_CNT_STEREO_ONLY)

#if defined (POWER_ON_ENTER_FREEMAN_PAIRING_ENABLED) || defined(FREEMAN_ENABLED_STERO)
#if (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT == 1)
#define AOB_AUD_CFG_MODE_SELECT                  (AOB_AUD_CFG_FREEMAN_STEREO_ONE_CIS)
#elif (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT == 2)
#define AOB_AUD_CFG_MODE_SELECT                  (AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS)
#else
#define AOB_AUD_CFG_MODE_SELECT                  (AOB_AUD_CFG_TWS_MONO)
#endif

#else // (POWER_ON_ENTER_TWS_PAIRING_ENABLED)
#if (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT == 1)
#define AOB_AUD_CFG_MODE_SELECT                  (AOB_AUD_CFG_TWS_STEREO_ONE_CIS)
#elif (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT == 2)
#define AOB_AUD_CFG_MODE_SELECT                  (AOB_AUD_CFG_TWS_STEREO_TWO_CIS)
#else
#define AOB_AUD_CFG_MODE_SELECT                  (AOB_AUD_CFG_TWS_MONO)
#endif
#endif

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
typedef enum AOB_AUDIO_CONFIGURATION
{
    AOB_AUD_CFG_MIN = 0,
    /// TWS mode
    AOB_AUD_CFG_TWS_MONO = AOB_AUD_CFG_MIN,
    /// TWS Spatial audio
    AOB_AUD_CFG_TWS_STEREO_ONE_CIS,
    AOB_AUD_CFG_TWS_STEREO_TWO_CIS,//(may not be used)
    /// FREEMAN mode (Not single ear usage)
    AOB_AUD_CFG_FREEMAN_STEREO_ONE_CIS,
    AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS,
    /// Audio configuration max
    AOB_AUD_CFG_MAX,
} aob_audio_cfg_e;

/*
 * External Functions declaration
 ****************************************************************************************
 */
extern void aob_gaf_api_get_qos_req_info_cb_init(void* cb_func);

/*
 * Internal Functions declaration
 ****************************************************************************************
 */
static void aob_init_add_pac_record(void);

static bool aob_init_fill_qos_req_for_codec_cfg_cb_func(uint8_t direction, const app_gaf_codec_id_t *codec_id, uint8_t tgt_latency,
                                                        app_bap_cfg_t *codec_cfg_req, app_bap_qos_req_t *ntf_qos_req);

/*
 * Internal Values
 ****************************************************************************************
 */

/*LC3 Audio Configuration V1.0*/
static const bap_audio_cfg_t aob_audio_conf_v_1[AOB_AUD_CFG_MAX] = \
{
    /*AOB_AUD_CFG_TWS_MONO*/
    {AOB_AUD_CHAN_CNT_MONO, AOB_AUD_CHAN_CNT_MONO, 1, 1, AOB_AUD_LOCATION_WITH_ROLE, AOB_AUD_LOCATION_WITH_ROLE},
    /*AOB_AUD_CFG_TWS_STEREO_ONE_CIS*/
    {AOB_AUD_CHAN_CNT_ALL_SUPP, AOB_AUD_CHAN_CNT_ALL_SUPP, 2, 2, AOB_AUD_LOCATION_WITH_ROLE, AOB_AUD_LOCATION_WITH_ROLE},
    /*AOB_AUD_CFG_TWS_STEREO_TWO_CIS*/
    {AOB_AUD_CHAN_CNT_MONO, AOB_AUD_CHAN_CNT_MONO, 1, 1, AOB_AUD_LOCATION_WITH_ROLE, AOB_AUD_LOCATION_WITH_ROLE},
    /*AOB_AUD_CFG_FREEMAN_STEREO_ONE_CIS*/
    {AOB_AUD_CHAN_CNT_ALL_SUPP, AOB_AUD_CHAN_CNT_ALL_SUPP, 2, 2, AOB_AUD_LOCATION_ALL_SUPP, AOB_AUD_LOCATION_ALL_SUPP},
    /*AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS*/
    {AOB_AUD_CHAN_CNT_MONO, AOB_AUD_CHAN_CNT_MONO, 1, 1, AOB_AUD_LOCATION_ALL_SUPP, AOB_AUD_LOCATION_ALL_SUPP},
};

/*LC3 QOS SETTING BAP V1.0*/
static const bap_qos_setting_t bap_qos_setting_v_1[BAP_QOS_SETTING_NUM_MAX] = \
{
     /// Low Latency
     /*            {frame_type, rtn, max_trans, max_pres, sdu_intv, max_oxts,}*//*
    "8_1_1"      */{APP_ISO_UNFRAMED_MODE, 2,  8,  40, 75,  26, },/*
    "8_2_1"      */{APP_ISO_UNFRAMED_MODE, 2,  10, 40, 100, 30, },/*
    "16_1_1"     */{APP_ISO_UNFRAMED_MODE, 2,  8,  40, 75,  30, },/*
    "16_2_1"     */{APP_ISO_UNFRAMED_MODE, 2,  10, 40, 100, 40, },/*
    "24_1_1"     */{APP_ISO_UNFRAMED_MODE, 2,  8,  40, 75,  45, },/*
    "24_2_1"     */{APP_ISO_UNFRAMED_MODE, 2,  10, 40, 100, 60, },/*
    "32_1_1"     */{APP_ISO_UNFRAMED_MODE, 2,  8,  40, 75,  60, },/*
    "32_2_1"     */{APP_ISO_UNFRAMED_MODE, 2,  10, 40, 100, 80, },/*
    "441_1_1"    */{APP_ISO_FRAMED_MODE,   5,  24, 40, 75,  97, },/*
    "441_2_1"    */{APP_ISO_FRAMED_MODE,   5,  31, 40, 100, 20, },/*
    "48_1_1"     */{APP_ISO_UNFRAMED_MODE, 5,  15, 40, 75,  75, },/*
    "48_2_1"     */{APP_ISO_UNFRAMED_MODE, 5,  20, 40, 100, 100,},/*
    "48_3_1"     */{APP_ISO_UNFRAMED_MODE, 5,  15, 40, 75,  90, },/*
    "48_4_1"     */{APP_ISO_UNFRAMED_MODE, 5,  20, 40, 100, 120,},/*
    "48_5_1"     */{APP_ISO_UNFRAMED_MODE, 5,  15, 40, 75,  117,},/*
    "48_6_1"     */{APP_ISO_UNFRAMED_MODE, 5,  20, 40, 100, 155,},/*
     /// High Reliable
    "8_1_2"      */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  26, },/*
    "8_2_2"      */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 30, },/*
    "16_1_2"     */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  30, },/*
    "16_2_2"     */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 40, },/*
    "24_1_2"     */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  45, },/*
    "24_2_2"     */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 60, },/*
    "32_1_2"     */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  60, },/*
    "32_2_2"     */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 80, },/*
    "441_1_2"    */{APP_ISO_FRAMED_MODE,   13, 80, 40, 75,  97, },/*
    "441_2_2"    */{APP_ISO_FRAMED_MODE,   13, 85, 40, 100, 130,},/*
    "48_1_2",    */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  75, },/*
    "48_2_2",    */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 100,},/*
    "48_3_2",    */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  90, },/*
    "48_4_2",    */{APP_ISO_UNFRAMED_MODE, 13, 100, 40, 100, 120,},/*
    "48_5_2",    */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  117,},/*
    "48_6_2",    */{APP_ISO_UNFRAMED_MODE, 13, 100, 40, 100, 155,},
#if defined (AOB_GMAP_ENABLED)
    /*
    "48_1_GC"    */{APP_ISO_UNFRAMED_MODE,  2,  8, 40, 75,  75, }, /*
    "48_2_GC"    */{APP_ISO_UNFRAMED_MODE,  2, 10, 40, 100, 100,}, /*
    "48_1_GR"    */{APP_ISO_UNFRAMED_MODE,  2, 13, 10, 75,  75, }, /*
    "48_2_GR"    */{APP_ISO_UNFRAMED_MODE,  2, 16, 10, 100, 100,}, /*
    "48_3_GR"    */{APP_ISO_UNFRAMED_MODE,  2, 13, 10, 75,  90,}, /*
    "48_4_GR"    */{APP_ISO_UNFRAMED_MODE,  2, 17, 10, 100, 120,},
#endif
};

/*
 * Public functions
 ****************************************************************************************
 */

/**
 ************************************************************************************************
 * @brief @brief main thread system call init for tws earbuds
 *
 ************************************************************************************************
 **/
void ble_audio_tws_init(void)
{
#if BLE_AUDIO_ENABLED
    // Below are some configuration about BLE Audio Profiles, sunch as PACS and only PACS
    aob_gaf_capa_info_t gaf_pacs_info;
    uint32_t role_bf = 0;
    /// Check for select audio configuration location sink
    if (aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_aud_location_bf == AOB_AUD_LOCATION_WITH_ROLE)
    {
        if (BLE_AUDIO_TWS_MASTER == ble_audio_get_tws_nv_role())
        {
            gaf_pacs_info.sink_location_bf = AOB_AUD_LOCATION_RIGHT;
        }
        else
        {
            gaf_pacs_info.sink_location_bf = AOB_AUD_LOCATION_LEFT;
        }
    }
    else
    {
        gaf_pacs_info.sink_location_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_aud_location_bf;
    }
    /// Check for select audio configuration location src
    if (aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_aud_location_bf == AOB_AUD_LOCATION_WITH_ROLE)
    {
        if (BLE_AUDIO_TWS_MASTER == ble_audio_get_tws_nv_role())
        {
            gaf_pacs_info.src_location_bf = AOB_AUD_LOCATION_RIGHT;
        }
        else
        {
            gaf_pacs_info.src_location_bf = AOB_AUD_LOCATION_LEFT;
        }
    }
    else
    {
        gaf_pacs_info.src_location_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_aud_location_bf;
    }

    LOG_I("%s audio location bf sink:[0x%x] src:[0x%x]", __func__,
          gaf_pacs_info.sink_location_bf, gaf_pacs_info.src_location_bf);
    /// Init supp and ava context
    gaf_pacs_info.sink_context_bf_supp = CUSTOM_CAPA_SINK_CONTEXT_BF;
    gaf_pacs_info.sink_ava_bf = CUSTOM_CAPA_SINK_CONTEXT_BF;
    gaf_pacs_info.src_context_bf_supp = CUSTOM_CAPA_SRC_CONTEXT_BF;
    gaf_pacs_info.src_ava_bf = CUSTOM_CAPA_SRC_CONTEXT_BF;

    /// Init pac record num for adding pac records
    gaf_pacs_info.sink_nb_pacs = CUSTOM_CAPA_NB_PAC_SINK;
    gaf_pacs_info.src_nb_pacs = CUSTOM_CAPA_NB_PAC_SRC;

    // The role bf is use to inidcate which BLE Audio Profile should be enbaled
    SETB(role_bf, BAP_ROLE_SUPP_CAPA_SRV, 1);
    SETB(role_bf, BAP_ROLE_SUPP_UC_SRV, 1);

    SETB(role_bf, BAP_ROLE_SUPP_BC_SINK, 1);
    SETB(role_bf, BAP_ROLE_SUPP_BC_SCAN, 1);
    SETB(role_bf, BAP_ROLE_SUPP_BC_DELEG, 1);

    /// Init BLE Audio Profiles and Services sunch as BAP
    // Init csip set info fill callback, must set it before earbuds init
    aob_csip_if_user_parameters_init();//aob_csip_if_register_sets_config_cb(cb);
    // Only PACS need a configuration when init
    aob_gaf_earbuds_init(&gaf_pacs_info, role_bf);
    // After PACS is init done, add pac records for earbuds support codec capabilties
    aob_init_add_pac_record();
    /*
    Init for codec cfg req_ind - cfm qos req callback function register
    custom may use @see aob_media_ascs_register_codec_req_handler_cb
    */
    aob_gaf_api_get_qos_req_info_cb_init((void *)aob_init_fill_qos_req_for_codec_cfg_cb_func);
#endif
}

/**
 ************************************************************************************************
 * @brief Add pac record in this fucntion, sunch as LC3/LC3Plus
 *
 * ATTENTION: call it after PACS first init is called  @see aob_gaf_earbuds_init
 ************************************************************************************************
 **/
static void aob_init_add_pac_record(void)
{
    aob_codec_capa_t p_codec_capa;
    aob_codec_id_t codec_id;
    app_bap_vendor_specific_cfg_t vendor_specific_cfg;
    uint32_t capa_data_len = sizeof(app_bap_vendor_specific_cfg_t);;
    uint32_t metadata_data_len = 0;

    // Prepare for codec capability add use
    p_codec_capa.capa = (app_bap_capa_t *)ke_malloc(sizeof(app_bap_capa_t) + capa_data_len, KE_MEM_ENV);
    p_codec_capa.metadata = (app_bap_capa_metadata_t *)ke_malloc(sizeof(app_bap_capa_metadata_t) + metadata_data_len, KE_MEM_ENV);

    if ((!p_codec_capa.capa) || (!p_codec_capa.metadata))
    {
        ASSERT(0,"aob_codec_capa_t codec cfg init malloc failed");
    }

    memset(p_codec_capa.capa, 0, sizeof(app_bap_capa_t) + capa_data_len);
    memset(p_codec_capa.metadata, 0, sizeof(app_bap_capa_metadata_t)+ metadata_data_len);

    p_codec_capa.metadata->add_metadata.len = metadata_data_len;//only merge aob without app_bap from le_audio_dev,avoid build error

    /*******************************Vendor Specific use***************************/
    /// Vendor specified metadata
    vendor_specific_cfg.length = sizeof(app_bap_vendor_specific_cfg_t)-1;
    //vendor specific type is 0xff
    vendor_specific_cfg.type = 0xff;
    //Add finished product suppliers company id
    vendor_specific_cfg.company_id = 0xffff;
    vendor_specific_cfg.s2m_decode_channel = GAF_AUDIO_STREAM_PLAYBACK_CHANNEL_NUM;
    vendor_specific_cfg.s2m_encode_channel = GAF_AUDIO_STREAM_CAPTURE_CHANNEL_NUM;
    memcpy(&p_codec_capa.capa->add_capa.data[0] + p_codec_capa.capa->add_capa.len,
           &vendor_specific_cfg, sizeof(app_bap_vendor_specific_cfg_t));
    p_codec_capa.capa->add_capa.len += sizeof(app_bap_vendor_specific_cfg_t);

    /*********************************LC3 CODEC CFG*******************************/
    /// LC3 CAPA
    memcpy(&codec_id.codec_id[0], CUSTOM_CAPA_CODEC_ID, AOB_CODEC_ID_LEN);
    p_codec_capa.capa->param.frame_dur_bf = CUSTOM_CAPA_FRAME_DURATION_BF;
    p_codec_capa.metadata->param.context_bf = CUSTOM_CAPA_PREFERRED_CONTEXT_BF;

#if !defined (AOB_SPLIT_PAC_RECORD_INTO_FOUR_RECORDS)
    /// 8_1_1 -> 48_6_2
    p_codec_capa.capa->param.sampling_freq_bf = CUSTOM_CAPA_ALL_SAMPLING_FREQ_BF;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_8_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_48_6_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);
    /*******************************************************************************/
#else
    // Do not need vendor capa
    p_codec_capa.capa->add_capa.len = 0;
    /// 8_1_1 -> 8_2_2
    p_codec_capa.capa->param.sampling_freq_bf = AOB_SUPPORTED_SAMPLE_FREQ_8000;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_8_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_8_2_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);

    /// 16_1_1 -> 16_2_2
    p_codec_capa.capa->param.sampling_freq_bf = AOB_SUPPORTED_SAMPLE_FREQ_16000;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_16_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_16_2_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);

    /// 32_1_1 -> 32_2_2
    p_codec_capa.capa->param.sampling_freq_bf = AOB_SUPPORTED_SAMPLE_FREQ_32000;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_32_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_32_2_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);

    /// 48_1_1 -> 48_6_2
    p_codec_capa.capa->param.sampling_freq_bf = AOB_SUPPORTED_SAMPLE_FREQ_48000;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_48_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_48_6_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

#if defined (AOB_GMAP_ENABLED)
    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);
#endif
    /*******************************************************************************/
#endif

#ifdef LC3PLUS_SUPPORT
    /*******************************LC3Plus CODEC CFG*******************************/
    /// LC3Plus capa
    memcpy(&codec_id.codec_id[0], &CUSTOM_CAPA_LC3PLUS_CODEC_ID, AOB_CODEC_ID_LEN);

    p_codec_capa.capa->param.frame_dur_bf           = CUSTOM_CAPA_LC3PLUS_FRAME_DURATION_BF;
    p_codec_capa.metadata->param.context_bf         = CUSTOM_CAPA_LC3PLUS_PREFERRED_CONTEXT_BF;
    p_codec_capa.capa->param.sampling_freq_bf       = CUSTOM_CAPA_LC3PLUS_SAMPLING_FREQ_BF;
    p_codec_capa.capa->param.frame_octet_min        = CUSTOM_CAPA_LC3PLUS_FRAME_OCT_MIN;
    p_codec_capa.capa->param.frame_octet_max        = CUSTOM_CAPA_LC3PLUS_FRAME_OCT_MAX;

    p_codec_capa.capa->param.chan_cnt_bf            = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu         = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf            = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu         = aob_audio_conf_v_1[AOB_AUD_CFG_MODE_SELECT].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);
    /******************************************************************************/
#endif // LC3PLUS_SUPPORT

    // Free allocated buffer
    ke_free(p_codec_capa.capa);
    ke_free(p_codec_capa.metadata);
}

/**
 ************************************************************************************************
 * @brief A callback function example for Codec cfg request to fill qos req value 
 *        @see app_bap_qos_req_t or you can modify codec_cfg_req @see app_bap_cfg_t
 *        in this function
 *
 * ATTENTION: Do not delete this callback function before using a new callback.
 *            If no callback function is set, codec cfg req - cfm will use dft.
 *
 * view more information @see get_qos_req_cfg_info_cb
 *
 *************************************************************************************************
 **/
static bool aob_init_fill_qos_req_for_codec_cfg_cb_func(uint8_t direction, const app_gaf_codec_id_t *codec_id, 
                                                        uint8_t tgt_latency, app_bap_cfg_t *codec_cfg_req, app_bap_qos_req_t *ntf_qos_req)
{
    if (!codec_cfg_req || !ntf_qos_req)
    {
        LOG_E("%s Err params!!!", __func__);
        return false;
    }

    LOG_I("%s codec id is %d", __func__, codec_id->codec_id[0]);
    uint8_t qos_setting_idx = BAP_QOS_SETTING_MIN;

    /*************************Using app_bap_cfg_t to choose qos setting****************************/
#ifdef LC3PLUS_SUPPORT
    if (app_bap_codec_is_lc3plus(codec_id))
    {
        /// @see LC3plus High Resolution Spec
        // Max transmission Latency
        ntf_qos_req->trans_latency_max_ms = QOS_SETTING_LC3PLUS_MAX_TRANS_DELAY;
        // Retransmission Number
        ntf_qos_req->retx_nb = QOS_SETTING_LC3PLUS_MAX_RTX_NUMER;
        // Prefer PresDelay
        ntf_qos_req->pref_pres_delay_min_us = QOS_SETTING_LC3PLUS_DFT_PRESDELAY;
        ntf_qos_req->pref_pres_delay_max_us = QOS_SETTING_LC3PLUS_DFT_PRESDELAY;
        // PresDelay
        ntf_qos_req->pres_delay_min_us = QOS_SETTING_LC3PLUS_DFT_PRESDELAY;
        ntf_qos_req->pres_delay_max_us = QOS_SETTING_LC3PLUS_DFT_PRESDELAY;
        // Framing type
        ntf_qos_req->framing = APP_ISO_UNFRAMED_MODE;
        // PHY
        ntf_qos_req->phy_bf = APP_PHY_2MBPS_VALUE;
    }
    else if (app_bap_codec_is_lc3(codec_id))
    {
#endif
    // QOS SETTING LOW LATENCY
    if (tgt_latency == APP_BAP_UC_TGT_LATENCY_LOWER)
    {
        switch (codec_cfg_req->param.sampling_freq)
        {
        case APP_BAP_SAMPLING_FREQ_48000HZ:
        {
#if defined (AOB_GMAP_ENABLED)
            if (direction == APP_GAF_DIRECTION_SRC)
            {
                for (qos_setting_idx = BAP_QOS_SETTING_GMING_48_1_GC; qos_setting_idx <= BAP_QOS_SETTING_GMING_48_2_GC; qos_setting_idx++)
                {
                    if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                    {
                        LOG_I("GMAP qos setting %d found!!!", qos_setting_idx);
                        break;
                    }
                }
            }
            else //if (direction == APP_GAF_DIRECTION_SINK)
            {
                for (qos_setting_idx = BAP_QOS_SETTING_GMING_48_1_GR; qos_setting_idx <= BAP_QOS_SETTING_GMING_48_4_GR; qos_setting_idx++)
                {
                    if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                    {
                        LOG_I("GMAP qos setting %d found!!!", qos_setting_idx);
                        break;
                    }
                }
            }
            /// Could not find target GMAP qos setting
            if (qos_setting_idx == (BAP_QOS_SETTING_GMING_48_2_GC + 1) ||
                qos_setting_idx == (BAP_QOS_SETTING_GMING_48_4_GR + 1))
#endif
            for (qos_setting_idx = BAP_QOS_SETTING_LL_48_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_48_6_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_BAP_SAMPLING_FREQ_32000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_LL_32_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_32_2_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_BAP_SAMPLING_FREQ_24000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_LL_24_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_24_2_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_BAP_SAMPLING_FREQ_16000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_LL_16_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_16_2_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_BAP_SAMPLING_FREQ_8000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_LL_8_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_8_2_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        default:
            break;
        }

        if (qos_setting_idx == BAP_QOS_SETTING_LL_MAX)
        {
            qos_setting_idx = BAP_QOS_SETTING_LL_48_6_1;
        }
    }
    // QOS SETTING HIGH RELIABLE
    else if ((tgt_latency == APP_BAP_UC_TGT_LATENCY_BALENCED || tgt_latency == APP_BAP_UC_TGT_LATENCY_RELIABLE))
    {
        switch (codec_cfg_req->param.sampling_freq)
        {
        case APP_BAP_SAMPLING_FREQ_48000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_48_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_48_6_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_BAP_SAMPLING_FREQ_32000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_32_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_32_2_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_BAP_SAMPLING_FREQ_24000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_24_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_24_2_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_BAP_SAMPLING_FREQ_16000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_16_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_16_2_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_BAP_SAMPLING_FREQ_8000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_8_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_8_2_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        default:
            break;
        }

        if (qos_setting_idx == BAP_QOS_SETTING_HR_MAX)
        {
            qos_setting_idx = BAP_QOS_SETTING_HR_48_6_2;
        }
    }
    else
    {
        LOG_E("No matched Qos setting latency for Codec cfg!!!");
        /// use dft qos setting
        qos_setting_idx = BAP_QOS_SETTING_HR_48_6_2;
    }

    
#if defined(BLE_USB_AUDIO_SUPPORT) || defined(AOB_LOW_LATENCY_MODE)
    // Max transmission Latency
    ntf_qos_req->trans_latency_max_ms = APP_BAP_DFT_ASCS_MAX_TRANS_LATENCY_MS;
    // Retransmission Number
    ntf_qos_req->retx_nb = APP_BAP_DFT_ASCS_RTN;
    // Prefer PresDelay
    ntf_qos_req->pref_pres_delay_min_us = APP_BAP_DFT_ASCS_MIN_PRES_DELAY_US;
    ntf_qos_req->pref_pres_delay_max_us = APP_BAP_DFT_ASCS_MAX_PRES_DELAY_US;
    // PresDelay
    ntf_qos_req->pres_delay_min_us = APP_BAP_DFT_ASCS_MIN_PRES_DELAY_US;
    ntf_qos_req->pres_delay_max_us = APP_BAP_DFT_ASCS_MAX_PRES_DELAY_US;
    // Framing type
    ntf_qos_req->framing = APP_BAP_DFT_ASCS_FRAMING_TYPE;
    // PHY
    ntf_qos_req->phy_bf = APP_BAP_DFT_ASCS_PHY_BF;
#else
    // Max transmission Latency
    ntf_qos_req->trans_latency_max_ms = bap_qos_setting_v_1[qos_setting_idx].Max_trans_latency;
    // Retransmission Number
    ntf_qos_req->retx_nb = bap_qos_setting_v_1[qos_setting_idx].Rtn_num;
    // Prefer PresDelay
    ntf_qos_req->pref_pres_delay_min_us = QOS_SETTING_DEFAULT_MIN_PRESDELAY;
    ntf_qos_req->pref_pres_delay_max_us = QOS_SETTING_DEFAULT_MAX_PRESDELAY;
    // PresDelay
    ntf_qos_req->pres_delay_min_us = QOS_SETTING_DEFAULT_MIN_PRESDELAY;
    ntf_qos_req->pres_delay_max_us = QOS_SETTING_DEFAULT_MAX_PRESDELAY;
    // Framing type
    ntf_qos_req->framing = bap_qos_setting_v_1[qos_setting_idx].Faming_type;
    // PHY
    ntf_qos_req->phy_bf = APP_PHY_2MBPS_VALUE;
#endif

    LOG_I("%s use qos setting label %d", __func__, qos_setting_idx);
#ifdef LC3PLUS_SUPPORT
    }
#endif
     /*******************************Modify codec_cfg_req here*******************************/
    // codec_cfg_req->param.frame_octet = some value custom difine

    // TRUE Means accept this codec req, will cfm after this callback
    return true;
}
