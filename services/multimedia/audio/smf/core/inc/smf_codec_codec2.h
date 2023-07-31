#pragma once
#ifndef __SMF_CODEC_CODEC2_H__
#define __SMF_CODEC_CODEC2_H__

#include "smf_api.h"

enum smf_codec2_bitrate_mode_e {
    CODEC2_BR_MODE_3200 = 0,
};

typedef struct{
    smf_media_info_t media;
    unsigned int bitRateMode;
    uint32_t vad_mode; //0,1,2,3,4; 0:off
}smf_codec2_enc_open_param_t;

typedef struct {
    smf_media_info_t media;
    unsigned int bitRateMode;
}smf_codec2_dec_open_param_t;

/**
 * register codec2 decoder
 */
EXTERNC void smf_codec2_decoder_register(void);

/**
 * register codec2 encoder
 */
EXTERNC void smf_codec2_encoder_register(void);

#endif
