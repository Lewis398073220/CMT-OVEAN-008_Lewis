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
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "audio_dump.h"
#include "audioflinger.h"
#include "cqueue.h"
#include "hal_dma.h"
#include "hal_aud.h"
#include "hal_trace.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "app_audio.h"

#include "app.h"
#include "app_gaf_dbg.h"
#include "gaf_media_pid.h"
#include "gaf_media_sync.h"
#include "gaf_bis_media_stream.h"
#include "app_bap_bc_src_msg.h"
#include "app_bap_bc_sink_msg.h"
#include "app_gaf_custom_api.h"
#include "isoohci_int.h"
#include "co_hci.h"
#include "app_bap_data_path_itf.h"
#include "app_gaf_custom_api.h"
#include "app_overlay.h"
#include "gaf_codec_lc3.h"
#include "app_bt_sync.h"
#include "ble_audio_earphone_info.h"
#ifdef DSP_HIFI4
#include "mcu_dsp_m55_app.h"
#endif
#ifdef GAF_CODEC_CROSS_CORE
#include "mcu_dsp_m55_app.h"
#include "app_dsp_m55.h"
#include "gaf_codec_cc_common.h"
#include "gaf_codec_cc_bth.h"
#endif

/************************private macro defination***************************/
//#define BAP_DUMP_AUDIO_DATA
//#define BAP_CALCULATE_CODEC_MIPS

#define BT_AUDIO_CACHE_2_UNCACHE(addr) \
    ((unsigned char *)((unsigned int)addr & ~(0x04000000)))

#define LC3_BPS                             AUD_BITS_16
#define LC3_FRAME_MS                        10

//audio
#define LC3_AUDIO_CHANNEL_NUM               AUD_CHANNEL_NUM_1

//local source audio play
#define BIS_AUDIO_CHANNEL_NUM               AUD_CHANNEL_NUM_2
#define BIS_SAMPLE_RATE                     AUD_SAMPRATE_48000

#define TIME_CALCULATE_REVERSAL_THRESHOLD   0x7F000000
#define BIS_AUDIO_PLAY_DLAY_US              2000   //trigger delay time
#define BIS_AUDIO_PLAY_INTERVAL_US          10000   //af DMA irq interval
#define LC3_ALGORITH_CODEC_DELAY_US         2500    //lc3 encode decode delay
#define BIS_DIFF_ANCHOR_US                  4000    //current time diff to bis send anchor


/************************private type defination****************************/

/************************extern function declearation***********************/

/**********************private function declearation************************/

/************************private variable defination************************/

osMutexDef(gaf_bis_src_decoded_buffer_mutex);
osMutexDef(gaf_bis_src_encoded_buffer_mutex);
osMutexDef(gaf_decoder_buffer_mutex);
#ifdef GAF_CODEC_CROSS_CORE
osMutexDef(gaf_m55_encoder_buffer_mutex);
#endif
extern struct hci_le_create_big_cmp_evt big_created_info;
uint32_t expected_play_time = 0;

#ifdef GAF_CODEC_CROSS_CORE
GAF_AUDIO_STREAM_ENV_T* pLocalBisStreamEnvPtr = NULL;
#endif
GAF_AUDIO_STREAM_ENV_T gaf_bis_src_audio_stream_env[GAF_AUDIO_CONTEXT_NUM_MAX];
GAF_AUDIO_STREAM_ENV_T gaf_bis_audio_stream_env[GAF_AUDIO_CONTEXT_NUM_MAX];

const static GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T gaf_bis_sink_stream_types_op_rule =
{
    GAF_AUDIO_STREAM_TYPE_PLAYBACK,
    GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM,
    1,
    0,
};

const static GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T gaf_bis_src_stream_types_op_rule =
{
    GAF_AUDIO_STREAM_TYPE_PLAYBACK | GAF_AUDIO_STREAM_TYPE_CAPTURE,
    GAF_AUDIO_TRIGGER_BY_CAPTURE_STREAM,
    1,
    1,
};

#ifdef AOB_MOBILE_ENABLED
#ifdef GAF_CODEC_CROSS_CORE
/**
 ****************************************************************************************
 * @brief When bth bis send the deinit signal to m55 before bth deinit m55 core
 *
 * @param[in] NONE                 NONE
 * @param[in] NONE                 NONE
 *
 * @param[out] NONE                NONE
 ****************************************************************************************
 */
static void gaf_bis_src_audio_send_deinit_signal_to_m55(void)
{
    GAF_AUDIO_M55_DEINIT_T p_deinit_req;
    p_deinit_req.con_lid         = gaf_m55_deinit_status.con_lid;
    p_deinit_req.context_type    = gaf_m55_deinit_status.context_type;
    p_deinit_req.is_bis          = gaf_m55_deinit_status.is_bis;
    p_deinit_req.is_bis_src      = gaf_m55_deinit_status.is_bis_src;
    p_deinit_req.capture_deinit  = gaf_m55_deinit_status.capture_deinit;
    p_deinit_req.playback_deinit = gaf_m55_deinit_status.playback_deinit;
    p_deinit_req.is_mobile_role  = gaf_m55_deinit_status.is_mobile_role;

    if (true == gaf_m55_deinit_status.playback_deinit){
        app_dsp_m55_bridge_send_cmd(
            CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP,
            (uint8_t*)&p_deinit_req,
            sizeof(GAF_AUDIO_M55_DEINIT_T));
    }

    if (true == gaf_m55_deinit_status.capture_deinit){
        app_dsp_m55_bridge_send_cmd(
            CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP, \
            (uint8_t*)&p_deinit_req, \
            sizeof(GAF_AUDIO_M55_DEINIT_T));
    }

    return ;
}
#endif

static GAF_MEDIA_DWELLING_INFO_T gaf_bis_src_media_dwelling_info;
/****************************function defination****************************/
static uint8_t *gaf_bis_src_media_data_get_packets(uint32_t len)
{
    gaf_stream_buff_list_t *list = &gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA].stream_context.playback_buff_list[0].buff_list;
    uint8_t *pcm_buff = NULL;
    uint8_t *read_pcm_buff = NULL;

    while (gaf_list_length(list))
    {
        list->node = gaf_list_begin(list);
        pcm_buff = (uint8_t *)gaf_list_node(list);
        break;
    }

    read_pcm_buff = (uint8_t *)gaf_stream_heap_malloc(len);
    if (pcm_buff)
    {
        memcpy(read_pcm_buff, pcm_buff, len);
        gaf_list_remove(list, pcm_buff);
    }

    return read_pcm_buff;
}

POSSIBLY_UNUSED static int gaf_bis_src_store_pcm_buffer(uint8_t *buffer, uint32_t len)
{
    gaf_stream_buff_list_t *list = &gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA].stream_context.playback_buff_list[0].buff_list;
    int nRet = 0;

    LOG_D("%s, data_len %d list len %d ", __func__, len, gaf_list_length(list));
    if (gaf_list_length(list) < 5)
    {
        uint8_t *pcm_buffer = (uint8_t *)gaf_stream_heap_malloc(len);

        if (len)
        {
            memcpy(pcm_buffer, buffer, len);
        }
        gaf_list_append(list, pcm_buffer);
    }
    else
    {
        LOG_I("%s list full current list_len:%d data_len:%d", __func__, gaf_list_length(list), len);
        nRet = -1;
    }

    return nRet;
}

static void gaf_bis_src_audio_media_buf_init(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    uint8_t* heapBufStartAddr = NULL;

#ifndef AOB_CODEC_CP
    lc3_alloc_data_free();
#endif
    app_audio_mempool_init_with_specific_size(app_audio_mempool_size());

    uint32_t audioCacheHeapSize = pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount*
        pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize;
    audioCacheHeapSize += 2 * (pStreamEnv->stream_info.captureInfo.maxCachedEncodedAudioPacketCount*
        pStreamEnv->stream_info.captureInfo.maxEncodedAudioPacketSize);
    app_audio_mempool_get_buff(&heapBufStartAddr, audioCacheHeapSize);
    gaf_stream_heap_init(heapBufStartAddr, audioCacheHeapSize);

    app_audio_mempool_get_buff(&(pStreamEnv->stream_info.playbackInfo.dmaBufPtr),
        pStreamEnv->stream_info.playbackInfo.dmaChunkSize*2);

    gaf_list_new(&pStreamEnv->stream_context.playback_buff_list[0].buff_list,
                    (osMutex(gaf_bis_src_decoded_buffer_mutex)),
                    gaf_stream_heap_free,
                    gaf_stream_heap_cmalloc,
                    gaf_stream_heap_free);

    pStreamEnv->func_list->decoder_func_list->decoder_init_buf_func(pStreamEnv,
        gaf_bis_src_stream_types_op_rule.playback_ase_count);

    app_audio_mempool_get_buff(&(pStreamEnv->stream_info.captureInfo.dmaBufPtr),
        pStreamEnv->stream_info.captureInfo.dmaChunkSize*2);

#ifdef GAF_CODEC_CROSS_CORE
    gaf_list_new(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list,
                    (osMutex(gaf_m55_encoder_buffer_mutex)),
                    gaf_m55_stream_encoder_data_free,
                    gaf_stream_heap_cmalloc,
                    gaf_m55_stream_encoder_heap_free);
#else
    gaf_list_new(&pStreamEnv->stream_context.capture_buff_list,
                    (osMutex(gaf_bis_src_encoded_buffer_mutex)),
                    gaf_stream_data_free,
                    gaf_stream_heap_cmalloc,
                    gaf_stream_heap_free);
#endif

    pStreamEnv->func_list->encoder_func_list->encoder_init_buf_func(pStreamEnv);

    gaf_bis_src_media_dwelling_info.playback_ase_id[0] = GAF_INVALID_ASE_INDEX;
    gaf_bis_src_media_dwelling_info.capture_ase_id = GAF_INVALID_ASE_INDEX;
    gaf_bis_src_media_dwelling_info.startedStreamTypes = 0;
}

static int gaf_bis_src_audio_media_stream_start_handler(void* _pStreamEnv)
{
    uint32_t trigger_bt_time = 0,current_bt_time = 0,bis_anchor_time = 0,diff_time = 0;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    pLocalCaptureBisStreamEnvPtr = pStreamEnv;
#endif
    if (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_208M);
        af_set_priority(AF_USER_AI, osPriorityHigh);

        struct AF_STREAM_CONFIG_T stream_cfg;

        // capture stream
        memset((void *)&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.captureInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.captureInfo.num_channels);
        stream_cfg.channel_map  = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1);

        stream_cfg.io_path      = AUD_INPUT_PATH_LINEIN;
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.captureInfo.sample_rate;

        // TODO: get vol from VCC via ase_lid
        stream_cfg.vol          = TGT_VOLUME_LEVEL_7;

        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.captureInfo.dmaChunkSize);

        pStreamEnv->func_list->stream_func_list.init_stream_buf_func(pStreamEnv);

        pStreamEnv->func_list->encoder_func_list->encoder_init_func(pStreamEnv);

        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.captureInfo.dmaBufPtr);

        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.capture_dma_irq_handler_func;

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

        // playback stream
        memset((void *)&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.playbackInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.playbackInfo.num_channels);

        stream_cfg.io_path      = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.playbackInfo.sample_rate;

        // TODO: get vol from VCC via ase_lid
        stream_cfg.vol          = TGT_VOLUME_LEVEL_7;

        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.playbackInfo.dmaChunkSize);

        pStreamEnv->func_list->decoder_func_list->decoder_init_func(pStreamEnv,
            gaf_bis_src_stream_types_op_rule.playback_ase_count);

        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.playbackInfo.dmaBufPtr);

        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.playback_dma_irq_handler_func;

        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

        // trigger set-up
        pStreamEnv->stream_context.playbackTriggerChannel = app_bt_sync_get_avaliable_trigger_channel();

        gaf_media_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

        af_codec_sync_device_config(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
        af_codec_sync_device_config(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
        af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, pStreamEnv->stream_context.playbackTriggerChannel);

        gaf_media_pid_init(&(pStreamEnv->stream_context.playback_pid_env));

        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);

        pStreamEnv->stream_context.captureTriggerChannel = app_bt_sync_get_avaliable_trigger_channel();

        gaf_media_prepare_capture_trigger(pStreamEnv->stream_context.captureTriggerChannel);
        gaf_media_pid_init(&(pStreamEnv->stream_context.capture_pid_env));
        gaf_media_pid_update_threshold(&(pStreamEnv->stream_context.capture_pid_env),
            pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2);

        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_INITIALIZED);

        if (GAF_AUDIO_TRIGGER_BY_CAPTURE_STREAM ==
            gaf_bis_src_stream_types_op_rule.trigger_stream_type)
        {
            current_bt_time = gaf_media_sync_get_curr_time();
            bis_anchor_time = btdrv_reg_op_big_anchor_timestamp(big_created_info.conhdl[0]&0xFF);
            LOG_I("current_bt_time %d bis_anchor_time %d big_sync_delay %d",current_bt_time, bis_anchor_time,big_created_info.big_sync_delay);

            if(current_bt_time >= bis_anchor_time)
            {
                diff_time = bis_anchor_time + pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs - current_bt_time;
                if(diff_time >= BIS_DIFF_ANCHOR_US){
                    if(bis_anchor_time + big_created_info.big_sync_delay > current_bt_time){
                        current_bt_time = bis_anchor_time + big_created_info.big_sync_delay;
                    }
                }
                else{
                    current_bt_time = bis_anchor_time + pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
                    osDelay(diff_time/1000);
                }
            }
            else
            {
                diff_time = bis_anchor_time - current_bt_time;
                if(diff_time >= BIS_DIFF_ANCHOR_US){
                    if(bis_anchor_time + big_created_info.big_sync_delay - pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs > current_bt_time){
                        current_bt_time = bis_anchor_time + big_created_info.big_sync_delay - pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
                    }
                }
                else{
                    current_bt_time = bis_anchor_time;
                    osDelay(diff_time/1000);
                }
            }

            trigger_bt_time = current_bt_time + big_created_info.big_trans_latency -
                pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs + LC3_ALGORITH_CODEC_DELAY_US;
            expected_play_time = trigger_bt_time + pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
            gaf_stream_common_set_capture_trigger_time(pStreamEnv, trigger_bt_time);
        }

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

        return 0;
    }

    return -1;
}

static uint32_t gaf_bis_src_playback_dma_irq_handler(uint8_t *buf, uint32_t len)
{
    uint32_t af_dma_time = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    int32_t diff_bt_time = 0;
    //float ratio = 0;
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint64_t revlersal_expected_time = 1,revlersal_current_time = 1;
    POSSIBLY_UNUSED uint8_t* read_buf = NULL;
    memset(buf, 0, len);

    //GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA];

    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        af_dma_time = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    diff_bt_time = af_dma_time - expected_play_time;

    if(abs(diff_bt_time) > TIME_CALCULATE_REVERSAL_THRESHOLD)
    {
        if(af_dma_time > expected_play_time)
        {
            revlersal_expected_time <<= 32;
            revlersal_expected_time += expected_play_time;
            revlersal_current_time = af_dma_time;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        else
        {
            revlersal_current_time <<= 32;
            revlersal_expected_time = expected_play_time;
            revlersal_current_time += af_dma_time;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        LOG_I("WARNIN:time has revleral expected_play_time %d af_dma_time %d diff_bt_time %d ",expected_play_time, af_dma_time,diff_bt_time);
    }

    //ratio = gaf_media_pid_adjust(&(pStreamEnv->stream_context.playback_pid_env), diff_bt_time);

    //af_codec_tune(AUD_STREAM_PLAYBACK, ratio);
    expected_play_time += BIS_AUDIO_PLAY_INTERVAL_US;

    read_buf = gaf_bis_src_media_data_get_packets(len);
    memcpy(buf, read_buf, len);
    gaf_stream_heap_free(read_buf);
    LOG_I("%s length %d", __func__, len);

    return 0;
}

static int gaf_bis_src_audio_media_stream_stop_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (GAF_CAPTURE_STREAM_IDLE != pStreamEnv->stream_context.capture_stream_state)
    {
        uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
        uint32_t dma_base;
        // source
        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_IDLE);
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

        // sink
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_IDLE);
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);

        pStreamEnv->func_list->decoder_func_list->decoder_deinit_func();

        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);

        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_AI, osPriorityAboveNormal);

        pStreamEnv->func_list->encoder_func_list->encoder_deinit_func(pStreamEnv);
        pStreamEnv->func_list->stream_func_list.deinit_stream_buf_func(pStreamEnv);

        return 0;
    }

    return -1;
}

static void gaf_bis_src_capture_dma_irq_handler_send(void* pStreamEnv_,void *payload,
    uint32_t payload_size, uint32_t ref_time)
{
    uint8_t *output_buf[2];
    output_buf[0] = (uint8_t*)payload;
    output_buf[1] = NULL;
    app_bap_bc_src_iso_send_data_to_all_channel(output_buf, payload_size, ref_time);
}

static uint32_t gaf_bis_src_capture_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    uint32_t dmaIrqHappeningTimeUs = 0;
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA];

    if ((!pStreamEnv) ||
        (GAF_CAPTURE_STREAM_START_TRIGGERING > pStreamEnv->stream_context.capture_stream_state)) {
        memset(ptrBuf, 0x00, length);
        return length;
    } else if (GAF_CAPTURE_STREAM_START_TRIGGERING == pStreamEnv->stream_context.capture_stream_state) {
        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_STREAMING_TRIGGERED);
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);
    }

    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((GAF_CAPTURE_STREAM_STREAMING_TRIGGERED ==
        pStreamEnv->stream_context.capture_stream_state) &&
        (dmaIrqHappeningTimeUs ==
        pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs))
    {
        LOG_W("accumulated irq messages happen!");
        return length;
    }

    //gaf_bis_src_store_pcm_buffer(ptrBuf, length);
    gaf_stream_common_capture_timestamp_checker(pStreamEnv, dmaIrqHappeningTimeUs);

    dmaIrqHappeningTimeUs += (uint32_t)pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs;
    LOG_I("length %d encoded_len %d filled timestamp %d", length,
        pStreamEnv->stream_info.captureInfo.encoded_frame_size,
        dmaIrqHappeningTimeUs);

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    do{
peek_again:
        if (gaf_m55_deinit_status.capture_deinit == true)
        {
            break;
        }
        bool is_accessed = false;
        is_accessed = gaf_stream_common_store_received_pcm_packet((void *)pStreamEnv, dmaIrqHappeningTimeUs, ptrBuf, length);

        if (pStreamEnv->stream_context.isUpStreamingStarted)
        {
            uint8_t capture_list_length = 0;
            capture_list_length = gaf_list_length(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list);
            if ((false == is_accessed) && (capture_list_length > GAF_ENCODER_PCM_DATA_BUFF_LIST_MAX_LENGTH)) {
                goto peek_again;
            }

            // malloc output_buf to cached encoded data
            uint8_t *output_buf = NULL;
            uint32_t lc3_encoded_frame_len = (uint32_t)(pStreamEnv->stream_info.captureInfo.encoded_frame_size);
            output_buf = (uint8_t *)gaf_stream_heap_cmalloc(lc3_encoded_frame_len);

            // bth fetch encoded data
            gaf_encoder_core_fetch_encoded_data(pStreamEnv, output_buf, lc3_encoded_frame_len, dmaIrqHappeningTimeUs);

            // bth send out encoded data
            gaf_bis_src_capture_dma_irq_handler_send(pStreamEnv, output_buf, lc3_encoded_frame_len, dmaIrqHappeningTimeUs);

            gaf_stream_heap_free(output_buf);
        }
    } while(0);
#else

    pStreamEnv->func_list->encoder_func_list->encoder_encode_frame_func(pStreamEnv, dmaIrqHappeningTimeUs,
        length, ptrBuf, &pStreamEnv->stream_context.codec_alg_context[0],
        &gaf_bis_src_capture_dma_irq_handler_send);
#endif
    return 0;
}

static void gaf_bis_src_audio_media_buf_deinit(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->stream_info.captureInfo.dmaBufPtr = NULL;
#ifdef GAF_CODEC_CROSS_CORE
    gaf_list_free(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list);
#else
    gaf_list_free(&pStreamEnv->stream_context.capture_buff_list);
#endif

    pStreamEnv->stream_info.playbackInfo.dmaBufPtr = NULL;
    gaf_list_free(&pStreamEnv->stream_context.playback_buff_list[0].buff_list);

    pStreamEnv->func_list->decoder_func_list->decoder_deinit_buf_func(pStreamEnv,
        gaf_bis_src_stream_types_op_rule.playback_ase_count);
    pStreamEnv->func_list->encoder_func_list->encoder_deinit_buf_func(pStreamEnv);
}

static GAF_AUDIO_FUNC_LIST_T gaf_bis_src_audio_media_stream_func_list =
{
    {
        .start_stream_func = gaf_bis_src_audio_media_stream_start_handler,
        .init_stream_buf_func = gaf_bis_src_audio_media_buf_init,
        .stop_stream_func = gaf_bis_src_audio_media_stream_stop_handler,
        .playback_dma_irq_handler_func = gaf_bis_src_playback_dma_irq_handler,
        .capture_dma_irq_handler_func = gaf_bis_src_capture_dma_irq_handler,
        .deinit_stream_buf_func = gaf_bis_src_audio_media_buf_deinit,
    },
};

static GAF_AUDIO_STREAM_ENV_T *gaf_bis_src_audio_stream_update_stream_env_from_grp_info(uint8_t grp_lid)
{
    app_bap_bc_src_grp_t *p_grp_t = app_bap_bc_src_get_big_info_by_big_idx(grp_lid);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA];
    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo;
    pCommonInfo = &(pStreamEnv->stream_info.captureInfo);

    pCommonInfo->aseChInfo[0].iso_channel_hdl = BLE_BISHDL_TO_ACTID(p_grp_t->stream_info->bis_hdl);
    pCommonInfo->aseChInfo[0].ase_handle = p_grp_t->stream_info->bis_hdl;
    pCommonInfo->num_channels = BIS_AUDIO_CHANNEL_NUM;
    pCommonInfo->bits_depth = LC3_BPS;

    switch (p_grp_t->p_sgrp_infos->codec_id.codec_id[0])
    {
        case APP_GAF_CODEC_TYPE_LC3:
        {
            app_bap_cfg_t* p_lc3_cfg = (app_bap_cfg_t*)p_grp_t->stream_info->p_cfg;
            pCommonInfo->frame_ms =
                gaf_stream_common_frame_duration_parse(p_lc3_cfg->param.frame_dur);
            pCommonInfo->sample_rate =
                gaf_stream_common_sample_freq_parse(p_lc3_cfg->param.sampling_freq);
            pCommonInfo->encoded_frame_size = p_lc3_cfg->param.frame_octet;
            pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER;
            pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size();
            pCommonInfo->dmaChunkIntervalUs = (uint32_t)(pCommonInfo->frame_ms*1000);
            pCommonInfo->dmaChunkSize =
                (uint32_t)((pCommonInfo->sample_rate*
                (pCommonInfo->bits_depth/8)*
                (pCommonInfo->dmaChunkIntervalUs/1000)*
                pCommonInfo->num_channels)/1000);
            memcpy(&(pStreamEnv->stream_info.playbackInfo), pCommonInfo, sizeof(GAF_AUDIO_STREAM_COMMON_INFO_T));
            gaf_audio_lc3_update_codec_func_list(pStreamEnv);
            break;
        }
        default:
            ASSERT(false, "unknown codec type!");
            return NULL;
    }

    return pStreamEnv;
}

static void _gaf_bis_src_audio_stream_start_handler(uint8_t grp_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_bis_src_audio_stream_update_stream_env_from_grp_info(grp_lid);
    if (pStreamEnv)
    {
        bt_adapter_write_sleep_enable(0);
        pStreamEnv->func_list->stream_func_list.start_stream_func(pStreamEnv);
    }
}

void gaf_bis_src_audio_stream_start_handler(uint8_t grp_lid)
{
    LOG_I("%s", __func__);
    app_bt_start_custom_function_in_bt_thread((uint32_t)grp_lid,
                                              0,
                                              (uint32_t)_gaf_bis_src_audio_stream_start_handler);
}

static void _gaf_bis_src_audio_stream_stop_handler(uint8_t grp_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_bis_src_audio_stream_update_stream_env_from_grp_info(grp_lid);

    if (pStreamEnv)
    {
        pStreamEnv->func_list->stream_func_list.stop_stream_func(pStreamEnv);
        bt_adapter_write_sleep_enable(1);
#ifdef GAF_CODEC_CROSS_CORE
        /// for bis src, the conlid and context_type is not needed,
        /// so it could be setted 0
        gaf_m55_deinit_status.con_lid         = 0;
        gaf_m55_deinit_status.context_type    = 0;
        gaf_m55_deinit_status.is_bis          = true;
        gaf_m55_deinit_status.is_bis_src      = true;
        gaf_m55_deinit_status.playback_deinit = true;
        gaf_m55_deinit_status.capture_deinit  = true;
        gaf_m55_deinit_status.is_mobile_role  = false;

        gaf_bis_src_audio_send_deinit_signal_to_m55();
#endif

    }
}

void gaf_bis_src_audio_stream_stop_handler(uint8_t grp_lid)
{
    LOG_I("%s", __func__);
    app_bt_start_custom_function_in_bt_thread((uint32_t)grp_lid,
                                              0,
                                              (uint32_t)_gaf_bis_src_audio_stream_stop_handler);
}

void gaf_bis_src_audio_stream_init(void)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA];
    memset((uint8_t *)pStreamEnv, 0, sizeof(GAF_AUDIO_STREAM_ENV_T));
    pStreamEnv->stream_context.playback_stream_state = GAF_PLAYBACK_STREAM_IDLE;
    pStreamEnv->stream_context.capture_stream_state = GAF_CAPTURE_STREAM_IDLE;
    pStreamEnv->stream_info.playbackInfo.aseChInfo[0].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    pStreamEnv->stream_info.captureInfo.aseChInfo[0].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    pStreamEnv->stream_info.playbackInfo.presDelayUs = GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;
    pStreamEnv->stream_info.captureInfo.presDelayUs = GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;

    GAF_AUDIO_STREAM_INFO_T* pStreamInfo;

    pStreamInfo = &(gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA].stream_info);
    pStreamInfo->contextType = GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA;
    gaf_stream_common_register_func_list(&gaf_bis_src_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA],
        &gaf_bis_src_audio_media_stream_func_list);
}
#endif

#ifdef GAF_CODEC_CROSS_CORE
/**
 ****************************************************************************************
 * @brief When bth bis received the decdoer deinit signal from m55, bth will deinit m55 core
 *
 * @param[in] con_id                       connection index
 * @param[in] context_type                 ASE context type
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
void _gaf_audio_bis_bth_decoder_received_deinit_signal_from_m55(uint8_t con_id, uint32_t context_type)
{
    LOG_D("gaf_audio_bth_bis_received_deinit_signal_from_m55");
    app_dsp_m55_deinit(APP_DSP_M55_USER_AUDIO_DECODER);
    return ;
}

/**
 ****************************************************************************************
 * @brief When bth bis received the encoder deinit signal from m55, bth will deinit m55 core
 *
 * @param[in] con_id                       connection index
 * @param[in] context_type                 ASE context type
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
void _gaf_audio_bis_bth_encoder_received_deinit_signal_from_m55(uint8_t con_id, uint32_t context_type)
{
    LOG_D("gaf_audio_bth_bis_received_deinit_signal_from_m55");
    app_dsp_m55_deinit(APP_DSP_M55_USER_AUDIO_ENCODER);
    return ;
}
#endif

//bis sink audio steam
static GAF_AUDIO_STREAM_ENV_T* gaf_bis_audio_get_stream_from_channel_index(uint8_t channel)
{
    for (uint32_t type = GAF_AUDIO_STREAM_CONTEXT_TYPE_MIN; type < GAF_AUDIO_CONTEXT_NUM_MAX; type++)
    {
        if ((gaf_bis_audio_stream_env[type].stream_info.playbackInfo.aseChInfo[0].iso_channel_hdl == channel) ||
            (gaf_bis_audio_stream_env[type].stream_info.captureInfo.aseChInfo[0].iso_channel_hdl == channel))
        {
            return &gaf_bis_audio_stream_env[type];
        }
    }

    ASSERT(0, "Receive a BIS packet before cooresponding stream is ready!");
    return NULL;
}

static void gaf_bis_stream_receive_data(uint16_t conhdl, GAF_ISO_PKT_STATUS_E pkt_status)
{
    // map to gaf stream context
    // TODO: get the correct stream context based on active stream type.
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = NULL;
    uint8_t channel = BLE_ISOHDL_TO_ACTID(conhdl);

    pStreamEnv = gaf_bis_audio_get_stream_from_channel_index(channel);
#ifdef GAF_CODEC_CROSS_CORE
    if (NULL != pStreamEnv)
    {
        pLocalBisStreamEnvPtr = pStreamEnv;
    }
#endif
    uint16_t frame_size = pStreamEnv->stream_info.playbackInfo.encoded_frame_size;
    uint8_t  select_channel = pStreamEnv->stream_info.playbackInfo.select_channel;

    uint32_t current_bt_time = 0;
    uint32_t trigger_bt_time = 0;
    gaf_media_data_t decoder_frame_info;
    gaf_media_data_t *p_decoder_frame = NULL;
    gaf_media_data_t* storedFramePointer = NULL;
    uint32_t sinktransdelay = 0;
    uint32_t audio_play_delay = 0;

    while ((p_decoder_frame = (gaf_media_data_t *)app_bap_dp_itf_get_rx_data(conhdl, NULL)))
    {
        ASSERT(p_decoder_frame->data_len <= pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize,
            "%s len %d %d, channel:%d, playbackInfo:%p", __func__, p_decoder_frame->data_len,
            pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize, channel, &(pStreamEnv->stream_info.playbackInfo));

        if ((pStreamEnv->stream_context.playback_stream_state >=
            GAF_PLAYBACK_STREAM_START_TRIGGERING) ||
            ((GAF_ISO_PKT_STATUS_VALID == p_decoder_frame->pkt_status) &&
            (p_decoder_frame->data_len > 0)))
        {
        #ifdef GAF_CODEC_CROSS_CORE
            if (gaf_m55_deinit_status.playback_deinit == true)
            {
                gaf_stream_data_free(p_decoder_frame);
                break;
            }
        #endif

            p_decoder_frame->data_len = frame_size;

            if((frame_size < p_decoder_frame->data_len)
                && ((frame_size * select_channel) < p_decoder_frame->data_len))
            {
                p_decoder_frame->sdu_data += frame_size * select_channel; // move sdu_data up to channel data position
            }
            storedFramePointer = gaf_stream_common_store_received_packet(pStreamEnv,
                                                                         GAF_AUDIO_DFT_PLAYBACK_LIST_IDX,
                                                                         p_decoder_frame);
        }

        decoder_frame_info = *p_decoder_frame;
        if (storedFramePointer == NULL)
        {
            gaf_stream_data_free(p_decoder_frame);
        }

        if (pStreamEnv->stream_context.playback_stream_state < GAF_PLAYBACK_STREAM_START_TRIGGERING)
        {
            if (GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM ==
                gaf_bis_sink_stream_types_op_rule.trigger_stream_type)
            {
                if ((GAF_PLAYBACK_STREAM_INITIALIZED == pStreamEnv->stream_context.playback_stream_state)
                    && (GAF_ISO_PKT_STATUS_VALID == decoder_frame_info.pkt_status)
                    && (decoder_frame_info.data_len > 0))
                {
                    current_bt_time = gaf_media_sync_get_curr_time();

                    // TODO: not necessary, should always use present delay
                    sinktransdelay = ble_audio_earphone_info_get_sink_sync_ref_offset();

                    ble_audio_earphone_info_update_sink_audio_paly_delay(BIS_AUDIO_PLAY_DLAY_US + pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs);

                    audio_play_delay = ble_audio_earphone_info_get_sink_audio_paly_delay();

                    LOG_I("%s expected play us %u current us %u seq 0x%x", __func__,
                        decoder_frame_info.time_stamp, current_bt_time, decoder_frame_info.pkt_seq_nb);
                    pStreamEnv->stream_info.playbackInfo.presDelayUs = 
                        sinktransdelay+audio_play_delay;

                    trigger_bt_time = decoder_frame_info.time_stamp + pStreamEnv->stream_info.playbackInfo.presDelayUs -
                        (uint32_t)(pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs);
                    LOG_I("calculated trigger ticks %u ", trigger_bt_time);
                    if (current_bt_time < trigger_bt_time)
                    {
                        LOG_I("Starting playback seq num 0x%x", decoder_frame_info.pkt_seq_nb);
                        pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX] = decoder_frame_info.pkt_seq_nb;
                        gaf_stream_common_set_playback_trigger_time(pStreamEnv, trigger_bt_time);
                    }
                    else
                    {
                        LOG_I("time_stamp error");
                        if (storedFramePointer)
                        {
                            gaf_list_remove(&pStreamEnv->stream_context.playback_buff_list[0].buff_list,
                                storedFramePointer);
                        }
                    }
                }
            }
        }
    }
}

static int gaf_bis_audio_media_stream_start_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state)
    {
        // TODO: shall use reasonable cpu frequency
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_104M);
        af_set_priority(AF_USER_AI, osPriorityHigh);

        struct AF_STREAM_CONFIG_T stream_cfg;

        memset((void *)&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.playbackInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.playbackInfo.num_channels);

        stream_cfg.io_path      = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.playbackInfo.sample_rate;

        // TODO: get vol from VCC
        stream_cfg.vol          = TGT_VOLUME_LEVEL_7;

        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.playbackInfo.dmaChunkSize);

        pStreamEnv->func_list->stream_func_list.init_stream_buf_func(pStreamEnv);
        pStreamEnv->func_list->decoder_func_list->decoder_init_func(pStreamEnv,
            gaf_bis_sink_stream_types_op_rule.playback_ase_count);

        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.playbackInfo.dmaBufPtr);

        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.playback_dma_irq_handler_func;

        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

        pStreamEnv->stream_context.playbackTriggerChannel = app_bt_sync_get_avaliable_trigger_channel();
        gaf_media_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

        // put PID env into stream context
        gaf_media_pid_init(&(pStreamEnv->stream_context.playback_pid_env));

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);

        app_bap_dp_itf_data_come_callback_register((void *)gaf_bis_stream_receive_data);

        return 0;
    }

    return -1;
}

static void gaf_bis_audio_media_playback_buf_init(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    uint8_t* heapBufStartAddr = NULL;

#ifndef AOB_CODEC_CP
    lc3_alloc_data_free();
#endif
    app_audio_mempool_init_with_specific_size(app_audio_mempool_size());

    uint32_t audioCacheHeapSize = pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount*
        pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize;

    app_audio_mempool_get_buff(&heapBufStartAddr, audioCacheHeapSize);
    gaf_stream_heap_init(heapBufStartAddr, audioCacheHeapSize);
    app_audio_mempool_get_buff(&(pStreamEnv->stream_info.playbackInfo.dmaBufPtr),
        pStreamEnv->stream_info.playbackInfo.dmaChunkSize*2);

    gaf_list_new(&pStreamEnv->stream_context.playback_buff_list[0].buff_list,
                    (osMutex(gaf_decoder_buffer_mutex)),
                    gaf_stream_data_free,
                    gaf_stream_heap_cmalloc,
                    gaf_stream_heap_free);

    pStreamEnv->func_list->decoder_func_list->decoder_init_buf_func(pStreamEnv,
        gaf_bis_sink_stream_types_op_rule.playback_ase_count);
}

static int gaf_bis_audio_media_stream_stop_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (GAF_PLAYBACK_STREAM_IDLE != pStreamEnv->stream_context.playback_stream_state)
    {
        uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
        uint32_t dma_base;
        app_bap_dp_itf_data_come_callback_deregister();
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_IDLE);
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);

        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);

        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_AI, osPriorityAboveNormal);

        pStreamEnv->func_list->decoder_func_list->decoder_deinit_func();
        pStreamEnv->func_list->stream_func_list.deinit_stream_buf_func(pStreamEnv);

        return 0;
    }

    return -1;

}

static uint32_t gaf_bis_stream_media_dma_irq_handler(uint8_t *buf, uint32_t len)
{
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint32_t dmaIrqHappeningTimeUs = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    uint32_t audio_play_delay = ble_audio_earphone_info_get_sink_audio_paly_delay();
    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA];

    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((!pStreamEnv) ||
        (GAF_PLAYBACK_STREAM_START_TRIGGERING > pStreamEnv->stream_context.playback_stream_state) ||
        ((GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED == pStreamEnv->stream_context.playback_stream_state) &&
        (dmaIrqHappeningTimeUs == pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs)))
    {
        memset(buf, 0, len);
        return len;
    }

    gaf_stream_common_updated_expeceted_playback_seq_and_time(pStreamEnv, GAF_AUDIO_DFT_PLAYBACK_LIST_IDX, dmaIrqHappeningTimeUs);

    if (GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED !=
        pStreamEnv->stream_context.playback_stream_state)
    {
        gaf_stream_common_update_playback_stream_state(pStreamEnv,
            GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED);
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
        pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]--;
        LOG_D("Trigger ticks %d Update playback seq to %d",
            dmaIrqHappeningTimeUs,
            pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]);
    }

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    uint32_t sink_sync_ref_offset = ble_audio_earphone_info_get_sink_sync_ref_offset();

    if (is_support_ble_audio_mobile_m55_decode == false)
    {
            bool ret = gaf_bis_decoder_core_fetch_pcm_data(pStreamEnv, pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX],
                buf, len, dmaIrqHappeningTimeUs, sink_sync_ref_offset, audio_play_delay);
            if (false == ret)
            {
                memset(buf, 0, len);
            }
    }
#else
    int32_t diff_bt_time = 0;
    uint8_t *bis_lc3_encode_buf_temp = NULL;

#ifdef ADVANCE_FILL_ENABLED
    // Broadcast sink prefill is not ready yet
    dmaIrqHappeningTimeUs -= pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
#endif

    uint64_t revlersal_expected_time = 1;
    uint64_t revlersal_current_time = 1;
    gaf_media_data_t *decoder_frame_p = NULL;

    decoder_frame_p = gaf_stream_common_get_packet(pStreamEnv, GAF_AUDIO_DFT_PLAYBACK_LIST_IDX, dmaIrqHappeningTimeUs);
#ifdef ADVANCE_FILL_ENABLED
    dmaIrqHappeningTimeUs += pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
#endif
    diff_bt_time = GAF_AUDIO_CLK_32_BIT_DIFF(decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs) - pStreamEnv->stream_info.playbackInfo.presDelayUs;

    if(abs(diff_bt_time) > TIME_CALCULATE_REVERSAL_THRESHOLD)
    {
        decoder_frame_p->time_stamp += audio_play_delay;
        dmaIrqHappeningTimeUs += pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
        if(dmaIrqHappeningTimeUs > decoder_frame_p->time_stamp)
        {
            revlersal_expected_time <<= 32;
            revlersal_expected_time += decoder_frame_p->time_stamp;
            revlersal_current_time = dmaIrqHappeningTimeUs;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        else
        {
            revlersal_current_time <<= 32;
            revlersal_expected_time = decoder_frame_p->time_stamp;
            revlersal_current_time += dmaIrqHappeningTimeUs;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        LOG_I("WARNIN:time has revleral time_stamp %d dmaIrqHappeningTimeUs %d diff_bt_time %d ", decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs,diff_bt_time);
    }

    gaf_media_pid_adjust(AUD_STREAM_PLAYBACK, &(pStreamEnv->stream_context.playback_pid_env), diff_bt_time);

    bis_lc3_encode_buf_temp = decoder_frame_p->sdu_data;
    if (bis_lc3_encode_buf_temp)
    {
        int ret = pStreamEnv->func_list->decoder_func_list->decoder_decode_frame_func
                (pStreamEnv, decoder_frame_p->data_len, bis_lc3_encode_buf_temp, &pStreamEnv->stream_context.codec_alg_context[0], buf);
        LOG_I("seq 0x%02x expected play time %u local time %u dt_time %d dec ret %d", decoder_frame_p->pkt_seq_nb,
                    decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs, diff_bt_time, ret);
    }
    else
    {
        memset(buf, 0, len);
    }
    gaf_stream_data_free(decoder_frame_p);
#endif

    return 0;
}

static void gaf_bis_audio_media_playback_buf_deinit(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->stream_info.playbackInfo.dmaBufPtr = NULL;
    gaf_list_free(&pStreamEnv->stream_context.playback_buff_list[0].buff_list);

    pStreamEnv->func_list->decoder_func_list->decoder_deinit_buf_func(pStreamEnv,
        gaf_bis_sink_stream_types_op_rule.playback_ase_count);
}

static GAF_AUDIO_FUNC_LIST_T gaf_audio_media_stream_func_list =
{
    {
        .start_stream_func = gaf_bis_audio_media_stream_start_handler,
        .init_stream_buf_func = gaf_bis_audio_media_playback_buf_init,
        .stop_stream_func = gaf_bis_audio_media_stream_stop_handler,
        .playback_dma_irq_handler_func = gaf_bis_stream_media_dma_irq_handler,
        .deinit_stream_buf_func = gaf_bis_audio_media_playback_buf_deinit,
    },
};

static GAF_AUDIO_STREAM_ENV_T *gaf_bis_audio_stream_update_stream_env_from_grp_info(uint8_t stream_lid)
{
    AOB_BIS_GROUP_INFO_T *aob_bis_group_info = ble_audio_earphone_info_get_bis_group_info();

    if (NULL == aob_bis_group_info)
    {
        ASSERT(0, "%s bc sink can not find earbud info", __func__);
    }
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA];

    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo;
    pCommonInfo = &(pStreamEnv->stream_info.playbackInfo);

    pStreamEnv->stream_info.playbackInfo.aseChInfo[0].iso_channel_hdl = BLE_BISHDL_TO_ACTID(aob_bis_group_info->bis_hdl[stream_lid]);
    pCommonInfo->aseChInfo[0].ase_handle = aob_bis_group_info->bis_hdl[stream_lid];
    pStreamEnv->stream_info.playbackInfo.num_channels = LC3_AUDIO_CHANNEL_NUM;
    pStreamEnv->stream_info.playbackInfo.bits_depth = LC3_BPS;

    switch (aob_bis_group_info->sink_audio_straming_info->codec_id.codec_id[0])
    {
        case APP_GAF_CODEC_TYPE_LC3:
        {
            app_bap_cfg_t* p_lc3_cfg = (app_bap_cfg_t*)aob_bis_group_info->sink_audio_straming_info->cfg;

            /// A bis contains multiple channels. The data of which channel is played is selected according
            /// to the p_lc3_cfg->param.location_bf parameter. At present, the audio data of the first channel
            /// is played by default
            pStreamEnv->stream_info.playbackInfo.select_channel = 0;

            pCommonInfo->frame_ms =
                gaf_stream_common_frame_duration_parse(p_lc3_cfg->param.frame_dur);
            pCommonInfo->sample_rate =
                gaf_stream_common_sample_freq_parse(p_lc3_cfg->param.sampling_freq);
            pCommonInfo->encoded_frame_size = p_lc3_cfg->param.frame_octet;
            pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER;
            pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size();
            pCommonInfo->dmaChunkIntervalUs = (uint32_t)(pCommonInfo->frame_ms*1000);
            pCommonInfo->dmaChunkSize =
                (uint32_t)((pCommonInfo->sample_rate*
                (pCommonInfo->bits_depth/8)*
                (pCommonInfo->dmaChunkIntervalUs)*
                pCommonInfo->num_channels)/1000/1000);
            gaf_audio_lc3_update_codec_func_list(pStreamEnv);
            break;
        }
        default:
            ASSERT(false, "unknown codec type!");
            return NULL;
    }

    return pStreamEnv;
}

#ifdef GAF_CODEC_CROSS_CORE
/**
 ****************************************************************************************
 * @brief When bth bis send the deinit signal to m55 before bth deinit m55 core
 *
 * @param[in] NONE                 NONE
 * @param[in] NONE                 NONE
 *
 * @param[out] NONE                NONE
 ****************************************************************************************
 */
static void gaf_bis_audio_send_deinit_signal_to_m55(void)
{
    GAF_AUDIO_M55_DEINIT_T p_deinit_req;
    p_deinit_req.con_lid         = gaf_m55_deinit_status.con_lid;
    p_deinit_req.context_type    = gaf_m55_deinit_status.context_type;
    p_deinit_req.is_bis          = gaf_m55_deinit_status.is_bis;
    p_deinit_req.is_bis_src      = gaf_m55_deinit_status.is_bis_src;
    p_deinit_req.capture_deinit  = gaf_m55_deinit_status.capture_deinit;
    p_deinit_req.playback_deinit = gaf_m55_deinit_status.playback_deinit;
    p_deinit_req.is_mobile_role  = gaf_m55_deinit_status.is_mobile_role;

    if (true == gaf_m55_deinit_status.playback_deinit){
        app_dsp_m55_bridge_send_cmd(
            CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP,
            (uint8_t*)&p_deinit_req,
            sizeof(GAF_AUDIO_M55_DEINIT_T));
    }
    return ;
}
#endif

static void _gaf_bis_audio_stream_stop_handler(uint8_t stream_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_bis_audio_stream_update_stream_env_from_grp_info(stream_lid);

    if (pStreamEnv)
    {
        pStreamEnv->func_list->stream_func_list.stop_stream_func(pStreamEnv);
        bt_adapter_write_sleep_enable(1);
#ifdef GAF_CODEC_CROSS_CORE
        gaf_m55_deinit_status.con_lid         = 0;
        gaf_m55_deinit_status.context_type    = 0;
        gaf_m55_deinit_status.is_bis          = true;
        gaf_m55_deinit_status.is_bis_src      = false;
        gaf_m55_deinit_status.playback_deinit = true;
        gaf_m55_deinit_status.capture_deinit  = false;
        gaf_m55_deinit_status.is_mobile_role  = false;

        gaf_bis_audio_send_deinit_signal_to_m55();
#endif
    }
#ifdef DSP_SMF
    dsp_close();
#endif
}

void gaf_bis_audio_stream_stop_handler(uint8_t stream_lid)
{
    LOG_I("%s", __func__);
    app_bt_start_custom_function_in_bt_thread((uint32_t)stream_lid,
                                              0,
                                              (uint32_t)_gaf_bis_audio_stream_stop_handler);
}

static void _gaf_bis_audio_stream_start_handler(uint8_t stream_lid)
{
#ifdef DSP_SMF
    dsp_open();
    osDelay(150);
#endif
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_bis_audio_stream_update_stream_env_from_grp_info(stream_lid);

    if (pStreamEnv)
    {
        bt_adapter_write_sleep_enable(0);
        pStreamEnv->func_list->stream_func_list.start_stream_func(pStreamEnv);
    }
}

void gaf_bis_audio_stream_start_handler(uint8_t stream_lid)
{
    // TODO: triggered by bt media manager
    LOG_I("%s", __func__);
    app_bt_start_custom_function_in_bt_thread((uint32_t)stream_lid,
                                              0,
                                              (uint32_t)_gaf_bis_audio_stream_start_handler);
}

void gaf_bis_audio_stream_init(void)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_bis_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA];
    memset((uint8_t *)pStreamEnv, 0, sizeof(GAF_AUDIO_STREAM_ENV_T));
    pStreamEnv->stream_context.playback_stream_state = GAF_PLAYBACK_STREAM_IDLE;
    pStreamEnv->stream_context.capture_stream_state = GAF_CAPTURE_STREAM_IDLE;
    pStreamEnv->stream_info.playbackInfo.aseChInfo[0].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    pStreamEnv->stream_info.captureInfo.aseChInfo[0].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    pStreamEnv->stream_info.playbackInfo.presDelayUs = GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US;

    GAF_AUDIO_STREAM_INFO_T* pStreamInfo;

    pStreamInfo = &(gaf_bis_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA].stream_info);
    pStreamInfo->contextType = GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA;
    gaf_stream_common_register_func_list(&gaf_bis_audio_stream_env[GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA],
        &gaf_audio_media_stream_func_list);

    ble_audio_earphone_info_update_sink_audio_paly_delay(BIS_AUDIO_PLAY_DLAY_US);
}

/// @} APP
#endif
