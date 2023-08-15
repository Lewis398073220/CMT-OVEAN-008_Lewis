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

#ifndef __SBC_H__
#define __SBC_H__

#include "sbc_types.h"
#include "sbc_math.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(ROM_UTILS_ON) && defined(SBC_CHANNEL_SELECT_IN_DECODER_INSTANCE)
#define CHANNEL_SELECT_IN_DECODER_INST
#endif

#define SBC_DECODER SBC_ENABLED
#define SBC_ENCODER SBC_ENABLED
#define SBC_PCM_BIG_ENDIAN SBC_DISABLED
#define SBC_MATH_FUNCTIONS SBC_MATH_USE_FIXED_HI_RES

#define LEtoHost16(ptr)  (U16)(((U16) *((U8*)(ptr)+1) << 8) | \
        (U16) *((U8*)(ptr)))

#define StoreLE16(buff,num) ( ((buff)[1] = (U8) ((num)>>8)),    \
                              ((buff)[0] = (U8) (num)) )

#if SBC_PCM_BIG_ENDIAN == SBC_DISABLED

#define PCMtoHost16 LEtoHost16
#define StorePCM16 StoreLE16

#else /* SBC_PCM_BIG_ENDIAN == SBC_ENABLED */ 

#define PCMtoHost16 BEtoHost16
#define StorePCM16 StoreBE16

#endif /* else SBC_PCM_BIG_ENDIAN == SBC_ENABLED */ 

#define SBC_MAX_PCM_DATA  512

#define SBC_SYNC_WORD 0x9C

#define MSBC_SYNC_WORD	0xAD

#define MSBC_BLOCKS	15

#define SBC_MAX_NUM_BLK     16
#define SBC_MAX_NUM_SB       8
#define SBC_MAX_NUM_CHNL     2

#define SBC_PARSE_SYNC           0
#define SBC_PARSE_HEADER         1
#define SBC_PARSE_SCALE_FACTORS  2
#define SBC_PARSE_SAMPLES        3

typedef U8 sbc_sample_rate_t;

#define SBC_CHNL_SAMPLE_FREQ_16    0
#define SBC_CHNL_SAMPLE_FREQ_32    1
#define SBC_CHNL_SAMPLE_FREQ_44_1  2
#define SBC_CHNL_SAMPLE_FREQ_48    3

typedef U8 sbc_chnl_mode_t;

#define SBC_CHNL_MODE_MONO          0
#define SBC_CHNL_MODE_DUAL_CHNL     1
#define SBC_CHNL_MODE_STEREO        2
#define SBC_CHNL_MODE_JOINT_STEREO  3

typedef U8 sbc_method_alloc_t;

#define SBC_ALLOC_METHOD_LOUDNESS   0
#define SBC_ALLOC_METHOD_SNR        1

typedef struct _sbc_codec_information_t {

    U8   bitPool;

    sbc_sample_rate_t sampleFreq;

    sbc_chnl_mode_t channelMode;

    sbc_method_alloc_t allocMethod;

    U8   numBlocks;

    U8   numSubBands;

    U8   numChannels;

	U8   mSbcFlag;

    /* === Internal use only === */ 
    U16  bitOffset;
    U8   crc;
    U8   fcs;
    U8   join[SBC_MAX_NUM_SB];
    U8   scale_factors[SBC_MAX_NUM_CHNL][SBC_MAX_NUM_SB];
    S32  scaleFactors[SBC_MAX_NUM_CHNL][SBC_MAX_NUM_SB];
    U16  levels[SBC_MAX_NUM_CHNL][SBC_MAX_NUM_SB];
    S8   bitNeed0[SBC_MAX_NUM_SB];
    S8   bitNeed1[SBC_MAX_NUM_SB];
    U8   bits[SBC_MAX_NUM_CHNL][SBC_MAX_NUM_SB];
    REAL sbSample[SBC_MAX_NUM_BLK][SBC_MAX_NUM_CHNL][SBC_MAX_NUM_SB];
    U8   fcs_bak;
} sbc_codec_information_t;

typedef struct _sbc_codec_information_short_t {

    U8   bitPool;

    sbc_sample_rate_t sampleFreq;

    sbc_chnl_mode_t channelMode;

    sbc_method_alloc_t allocMethod;

    U8   numBlocks;

    U8   numSubBands;

    U8   numChannels;

	U8   mSbcFlag;

} sbc_codec_information_short_t;


#if SBC_ENCODER == SBC_ENABLED

typedef struct _sbc_encoder_t {
    sbc_codec_information_t streamInfo;

    /* === Internal use only === */ 
    U8   sFactorsJoint[SBC_MAX_NUM_CHNL][SBC_MAX_NUM_SB];
    REAL sbJoint[SBC_MAX_NUM_BLK][SBC_MAX_NUM_CHNL];
    S16  X0[80 * 2];
    S16  X1[80 * 2];
    U16  X0pos, X1pos;
    REAL Y[16];
} sbc_encoder_t;

#endif /* SBC_ENCODER == SBC_ENABLED */ 

typedef enum {
    SBC_DECODER_CHANNEL_SELECT_SELECT_STEREO = 0,
    SBC_DECODER_CHANNEL_SELECT_SELECT_LRMERGE,
    SBC_DECODER_CHANNEL_SELECT_LCHNL,
    SBC_DECODER_CHANNEL_SELECT_RCHNL,
} SBC_DECODER_CHANNEL_SELECT_E;

#if SBC_DECODER == SBC_ENABLED
typedef struct _sbc_decoder_t {

    sbc_codec_information_t streamInfo;

    U16  maxPcmLen;
    REAL V0[160];
    REAL V1[160];

#if defined(CHANNEL_SELECT_IN_DECODER_INST)
    SBC_DECODER_CHANNEL_SELECT_E resv_channel_select;
#endif
    
    struct {
        U8    stageBuff[SBC_MAX_PCM_DATA]; /* Staging buffer            */ 
        U16   stageLen;                    /* Length of staged data     */ 
        U16   curStageOff;                 /* Offset into staged data   */ 
        U8    *rxBuff;                     /* The Received buffer       */ 
        U16   rxSize;                      /* Remaining rx buff size    */ 
        U8    rxState;                     /* Parser state              */ 
    } parser;

} sbc_decoder_t;

#endif /* SBC_DECODER == SBC_ENABLED */ 

typedef struct _sbc_pcm_data_t {
    sbc_sample_rate_t  sampleFreq;
    U8             numChannels;
    U16            dataLen;
    U8            *data;
} sbc_pcm_data_t;

U16 sbc_frame_length_get(sbc_codec_information_t *StreamInfo);

#if SBC_DECODER == SBC_ENABLED

void sbc_decoder_init(sbc_decoder_t *Decoder);
sbc_ret_status_t sbc_frames_parser(sbc_decoder_t *Decoder, 
                                            U8       *Buff, 
                                            U16       Len, 
                                            U16      *BytesDecoded);
void sbc_decode_cfg_ch_mode(sbc_decoder_t *Decoder, U32 ch_select);
sbc_ret_status_t sbc_frames_decode(sbc_decoder_t *Decoder, 
                          U8         *Buff, 
                          U16         Len, 
                          U16        *BytesParsed, 
                          sbc_pcm_data_t *PcmData, 
                          U16         MaxPcmData,
                          float*       gains);
sbc_ret_status_t sbc_frames_decode_select_channel(sbc_decoder_t *Decoder, 
                          U8         *Buff, 
                          U16         Len, 
                          U16        *BytesParsed, 
                          sbc_pcm_data_t *PcmData, 
                          U16         MaxPcmData,
                          float*       gains, SBC_DECODER_CHANNEL_SELECT_E channel_select);
sbc_ret_status_t sbc_frames_decode_out_sbsamples(sbc_decoder_t *Decoder,
        U8		 *Buff,
        U16		  Len,
        U16		 *BytesDecoded,
        sbc_pcm_data_t *PcmData,
        U16		  MaxPcmData,
        float* gains,
        U8     ChooseDecChannel,
        REAL       *SBSamplesBuf,
        U32        SBSamplesBufLen,
        U32        *SBSamplesBufUsed,
        U8     ChooseSplitChannel);

#endif /* SBC_DECODER == SBC_ENABLED */ 

#if SBC_ENCODER == SBC_ENABLED

void sbc_encoder_init(sbc_encoder_t *Encoder);

#ifdef SBC_REDUCE_SIZE
void sbc_filter_8band_analysis(sbc_encoder_t *Encoder, sbc_pcm_data_t *PcmData);
#endif

sbc_ret_status_t sbc_frames_encode(sbc_encoder_t *Encoder, 
                          sbc_pcm_data_t *PcmData, 
                          U16        *BytesEncoded, 
                          U8         *Buff, 
                          U16        *Len, 
                          U16         MaxSbcData);
sbc_ret_status_t sbc_frames_encode_with_sbsamples(sbc_encoder_t *Encoder,
						  REAL       *SBSamplesBuf,
						  U32		 SBSamplesBufLen_bytes,
						  U32		 *SBSamplesBufUsed_bytes,
						  U8		 *Buff,
						  U16		 *Len,
						  U16		  MaxSbcData,
						  U8      *number_freame_encoded);
						  
#ifdef SBC_REDUCE_SIZE
void sbc_8band_synth_filter_part1(int* sample, int* v);
#endif

#endif  /* SBC_ENCODER == SBC_ENABLED */ 

#ifdef __SBC_FUNC_IN_ROM__
 
//typedef I8 sbc_ret_status_t;

typedef U16 (*__sbc_frame_length_get)(sbc_codec_information_t *StreamInfo);
typedef void (*__sbc_decoder_init)(sbc_decoder_t *Decoder);
typedef void  (*__sbc_4band_synth_filter)(sbc_decoder_t *Decoder, sbc_pcm_data_t *PcmData, float* gain);
typedef void (*__sbc_8band_synth_filter)(sbc_decoder_t *Decoder, sbc_pcm_data_t *PcmData, float* gain);
typedef sbc_ret_status_t (*__sbc_frames_decode)(sbc_decoder_t *Decoder, 
						  U8		 *Buff, 
						  U16		  Len, 
						  U16		 *BytesDecoded, 
						  sbc_pcm_data_t *PcmData, 
						  U16		  MaxPcmData,
						  float* gains);

typedef sbc_ret_status_t (*__sbc_frames_decode_out_sbsamples)(sbc_decoder_t *Decoder, 
        U8		 *Buff, 
        U16		  Len, 
        U16		 *BytesDecoded, 
        sbc_pcm_data_t *PcmData, 
        U16		  MaxPcmData,
        float* gains,
        U8     ChooseDecChannel,
        REAL       *SBSamplesBuf,
        U32        SBSamplesBufLen,
        U32        *SBSamplesBufUsed,
        U8     ChooseSplitChannel);


typedef struct{
    __sbc_frame_length_get sbc_frame_length_get;
    __sbc_decoder_init sbc_decoder_init;
    __sbc_4band_synth_filter sbc_4band_synth_filter;
    __sbc_8band_synth_filter sbc_8band_synth_filter;
    __sbc_frames_decode sbc_frames_decode;
    __sbc_frames_decode_out_sbsamples sbc_frames_decode_out_sbsamples;
}SBC_ROM_STRUCT;

extern SBC_ROM_STRUCT   SBC_ROM_FUNC;
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __SBC_H__ */ 
