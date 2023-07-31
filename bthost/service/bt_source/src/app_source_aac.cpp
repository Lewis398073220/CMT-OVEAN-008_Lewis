/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef BT_A2DP_SUPPORT
#ifdef A2DP_SOURCE_AAC_ON
#include "app_source_codec.h"
#include "a2dp_codec_aac.h"
#ifdef A2DP_AAC_ON
#include "aacenc_lib.h"
#endif
#include "hal_location.h"

#define AAC_OUT_SIZE 1024*2
static HANDLE_AACENCODER SRAM_BSS_LOC aacEnc_handle = NULL;
static int8_t SRAM_BSS_LOC aacout_buffer[AAC_OUT_SIZE];

#define AAC_MEMPOLL_SIZE (70*1024)
static uint8_t  SRAM_BSS_LOC aac_pool_buff[AAC_MEMPOLL_SIZE];
static uint8_t * SRAM_DATA_LOC  source_aac_mempoll = NULL;

static char SRAM_BSS_LOC a2dp_aac_transmit_buffer[A2DP_AAC_TRANS_SIZE];

uint8_t *a2dp_source_aac_frame_buffer(void)
{
    return (uint8_t *)a2dp_aac_transmit_buffer;
}

extern heap_handle_t aac_memhandle;
static int source_aac_meminit(void)
{
    source_aac_mempoll = aac_pool_buff;

    if(aac_memhandle == NULL)
        aac_memhandle = heap_register(source_aac_mempoll, AAC_MEMPOLL_SIZE);

    return 1;
}
#define MAX_SOURCE_AAC_BITRATE 128000

#if 0 ///for test 
static int _aac_count=0;
static int _aac_time_ms0=0;
static int _aac_sample_rate=0;
static int _aac_sample_ch=0;
#endif
int aacenc_init(void)
{
    if (aacEnc_handle == NULL) {
        int  sample_rate, channels;
        int aot = 129;//2;//- 129: MPEG-2 AAC Low Complexity.
        int afterburner = 0;
        int eld_sbr = 0;
        int vbr = 0;
        CHANNEL_MODE mode;
        AACENC_InfoStruct info = { 0 };
        int bitrate =MAX_SOURCE_AAC_BITRATE;
        ///get media param from curr device
        uint8_t device_id = app_bt_source_get_streaming_a2dp();
        struct BT_SOURCE_DEVICE_T *curr_device = NULL;	
        if (device_id == BT_SOURCE_DEVICE_INVALID_ID){
            return false;
        }	
        curr_device = app_bt_source_get_device(device_id);
        ///set media param from curr device
        channels = curr_device->base_device->a2dp_channel_num;
        sample_rate = curr_device->aud_sample_rate;
        vbr = curr_device->base_device->vbr_support;
        bitrate = curr_device->aud_bit_rate;
        int sample_bits = curr_device->base_device->sample_bit;
        TRACE(2,"\n@@@aacenc_init:rate=%d,ch=%d,br=%d,vbr=%d,bits=%d,%dms\n"
            ,sample_rate,channels
            ,(int)bitrate
            ,(int)vbr
            ,(int)sample_bits	
            ,(int)1024000/sample_rate
        );
#if 0 ///for test
        _aac_sample_rate = sample_rate;
        _aac_sample_ch = channels;
#endif
        switch (channels) {
        case 1: mode = MODE_1;		 break;
        case 2: mode = MODE_2;		 break;
        case 3: mode = MODE_1_2;	 break;
        case 4: mode = MODE_1_2_1;	 break;
        case 5: mode = MODE_1_2_2;	 break;
        case 6: mode = MODE_1_2_2_1; break;
        default:
            TRACE(0,"ERROR!");
            return 1;
        }
        TRACE(1,"start aacEncOpen sample_rate=%d",sample_rate);
        
        int aacret = 0;
        if (source_aac_meminit() < 0)
        {
            TRACE(0,"aac_meminit error\n");
            return 1;
        }
        if ((aacret = aacEncOpen(&aacEnc_handle, 1, channels)) != AACENC_OK) {
            TRACE(1,"Unable to open encoder aacret=%d\n",aacret);
            return 1;
        }
        TRACE(0,"start aacEncoder_SetParam!");
        if (aacEncoder_SetParam(aacEnc_handle, AACENC_AOT, aot) != AACENC_OK) {
            TRACE(0,"Unable to set the AOT\n");
            return 1;
        }
        if (aot == 39 && eld_sbr) {
            if (aacEncoder_SetParam(aacEnc_handle, AACENC_SBR_MODE, 1) != AACENC_OK) {
                TRACE(0,"Unable to set SBR mode for ELD\n");
                return 1;
            }
        }
        if (aacEncoder_SetParam(aacEnc_handle, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
            TRACE(0,"Unable to set the AOT\n");
            return 1;
        }
        if (aacEncoder_SetParam(aacEnc_handle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
            TRACE(0,"Unable to set the channel mode\n");
            return 1;
        }
        if (aacEncoder_SetParam(aacEnc_handle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
            TRACE(0,"Unable to set the wav channel order\n");
            return 1;
        }
        if (vbr) {
            if (aacEncoder_SetParam(aacEnc_handle, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
                TRACE(0,"Unable to set the VBR bitrate mode\n");
                return 1;
            }
        } else {
            if (aacEncoder_SetParam(aacEnc_handle, AACENC_BITRATE, bitrate) != AACENC_OK) {
                TRACE(0,"Unable to set the bitrate\n");
                return 1;
            }
        }
        if (aacEncoder_SetParam(aacEnc_handle, AACENC_TRANSMUX, TT_MP4_LATM_MCP1) != AACENC_OK) {
            TRACE(0,"Unable to set the ADTS transmux\n");
            return 1;
        } 
        if (aacEncoder_SetParam(aacEnc_handle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
            TRACE(0,"Unable to set the afterburner mode\n");
            return 1;
        }
        ///add aac LATM header every frame.
        if (aacEncoder_SetParam(aacEnc_handle, AACENC_HEADER_PERIOD, 1) != AACENC_OK) {
            TRACE(0,"Unable to set the aac header-period\n");
            return 1;
        }
        if (aacEncEncode(aacEnc_handle, NULL, NULL, NULL, NULL) != AACENC_OK) {
            TRACE(0,"Unable to initialize the encoder\n");
            return 1;
        }
        if (aacEncInfo(aacEnc_handle, &info) != AACENC_OK) {
            TRACE(0,"Unable to get the encoder info\n");
            return 1;
        }
    }
    return 0;
}

int aacenc_deinit(void)
{
    if (aacEnc_handle != NULL) {
        aacEncClose(&aacEnc_handle);
        aacEnc_handle = NULL;
    }
    return 0;
}

bool a2dp_source_encode_aac_packet(a2dp_source_packet_t *source_packet)
{
    uint32_t byte_encoded = A2DP_AAC_TRANS_SIZE;
    AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
    AACENC_InArgs in_args = { 0 };
    AACENC_OutArgs out_args = { 0 };
    int in_identifier = IN_AUDIO_DATA;
    int in_elem_size;
    int out_identifier = OUT_BITSTREAM_DATA;
    int out_elem_size;
    int out_size = AAC_OUT_SIZE;
    AACENC_ERROR err;
    in_elem_size = 2;
    void *in_ptr, *out_ptr;

    btif_a2dp_sbc_packet_t *packet = &(source_packet->packet);

    in_ptr = a2dp_aac_transmit_buffer;
    in_args.numInSamples = byte_encoded/2;
    in_buf.numBufs = 1;
    in_buf.bufs = (void **)&in_ptr;
    in_buf.bufferIdentifiers = &in_identifier;
    in_buf.bufSizes = (int *)&byte_encoded;
    in_buf.bufElSizes = &in_elem_size;
    out_elem_size = 1;
    out_ptr = aacout_buffer;
    out_buf.numBufs = 1;
    out_buf.bufs = (void **)&out_ptr;
    out_buf.bufferIdentifiers = &out_identifier;
    out_buf.bufSizes = &out_size;
    out_buf.bufElSizes = &out_elem_size;

    int lock = int_lock();
    
    aacenc_init();
#if 0 ///for test
    unsigned t0 = hal_fast_sys_timer_get();   
#endif
    err = aacEncEncode(aacEnc_handle, &in_buf, &out_buf, &in_args, &out_args);
#if 0 ///for test
	unsigned t1 = hal_fast_sys_timer_get();
#endif
    int_unlock(lock);
#if 0 ///for test
    {
        unsigned ms = FAST_TICKS_TO_MS(t0);
        if(_aac_time_ms0==0)_aac_time_ms0=ms;	
        ms-=_aac_time_ms0;
        TRACE(1,"%dms,%d,%u(%u,%u)(%u,%u)",FAST_TICKS_TO_MS(t1-t0),(int)out_args.numOutBytes,_aac_count
            ,ms,_aac_count*1024000u/_aac_sample_rate
            ,_aac_count?(ms/_aac_count):0,1024000u/_aac_sample_rate
        );
        _aac_count++;
        DUMP8("%02x ",aacout_buffer,out_args.numOutBytes);
    }
#endif
    if(err != AACENC_OK || out_args.numOutBytes <= 0)
    {
        TRACE(2,"aacEncEncode err %x len %d", err, out_args.numOutBytes);
        return false;
    }

    packet->dataLen = out_args.numOutBytes;
    packet->frameSize = out_args.numInSamples;

    ASSERT(packet->dataLen < packet->reserved_data_size, "aac encodec packet length too long");

    memcpy(packet->data, aacout_buffer, packet->dataLen);

    source_packet->codec_type = BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC;
    return true;
}

static btif_avdtp_codec_t a2dp_source_aac_avdtpcodec;

#if BT_SOURCE_DEVICE_NUM > 1
static const unsigned char a2dp_source_codec_aac_elements[A2DP_AAC_OCTET_NUMBER] = {
    A2DP_AAC_OCTET0_MPEG2_AAC_LC,
    A2DP_AAC_OCTET1_SAMPLING_FREQUENCY_44100,
    A2DP_AAC_OCTET2_CHANNELS_2,
    /* A2DP_AAC_OCTET3_VBR_SUPPORTED |*/ ((MAX_SOURCE_AAC_BITRATE >> 16) & 0x7f),
    (MAX_SOURCE_AAC_BITRATE >> 8) & 0xff,
    (MAX_SOURCE_AAC_BITRATE) & 0xff
};
#else
static const unsigned char a2dp_source_codec_aac_elements[A2DP_AAC_OCTET_NUMBER] = {
    A2DP_AAC_OCTET0_MPEG2_AAC_LC,
    A2DP_AAC_OCTET1_SAMPLING_FREQUENCY_44100,
    A2DP_AAC_OCTET2_CHANNELS_1 | A2DP_AAC_OCTET2_CHANNELS_2 /*| A2DP_AAC_OCTET2_SAMPLING_FREQUENCY_48000*/,
    /* A2DP_AAC_OCTET3_VBR_SUPPORTED |*/ ((MAX_SOURCE_AAC_BITRATE >> 16) & 0x7f),
    (MAX_SOURCE_AAC_BITRATE >> 8) & 0xff,
    (MAX_SOURCE_AAC_BITRATE) & 0xff
};
#endif

void a2dp_source_register_aac_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback)
{
    a2dp_source_aac_avdtpcodec.codecType = BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC;
    a2dp_source_aac_avdtpcodec.discoverable = 1;
    a2dp_source_aac_avdtpcodec.elements = (U8 *)&a2dp_source_codec_aac_elements;
    a2dp_source_aac_avdtpcodec.elemLen  = sizeof(a2dp_source_codec_aac_elements);

    btif_a2dp_register(btif_a2dp, BTIF_A2DP_STREAM_TYPE_SOURCE, &a2dp_source_aac_avdtpcodec, sep_cp, sep_priority, callback);
}

#endif /* A2DP_SOURCE_AAC_ON */
#endif /* BT_A2DP_SUPPORT */