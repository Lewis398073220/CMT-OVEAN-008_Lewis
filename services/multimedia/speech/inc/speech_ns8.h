#ifndef SPEECH_NS8_H
#define SPEECH_NS8_H

#include <stdint.h>
#include "custom_allocator.h"

typedef struct {
    int32_t     bypass;
    float       denoise_dB;
    bool        echo_supp_enable;
    int32_t     ref_delay;
    float       gamma;
    int32_t     echo_band_start;
    int32_t     echo_band_end;
    float       min_ovrd;
    float       target_supp;
    float       ga_thr;
    float       en_thr;
} SpeechNs8Config;

struct SpeechNs8State_;

typedef struct SpeechNs8State_ SpeechNs8State;

#ifdef __cplusplus
extern "C" {
#endif

int32_t speech_ns8_process(SpeechNs8State *st, int16_t *pcm_buf, int16_t *ref_buf, uint32_t pcm_len);
void speech_ns8_destory(SpeechNs8State *st);
SpeechNs8State* speech_ns8_create(uint32_t sample_rate, uint32_t frame_size,  const SpeechNs8Config *cfg);
int32_t speech_ns8_set_config(SpeechNs8State *st, const SpeechNs8Config *cfg);

#ifdef __cplusplus
}
#endif

#endif
