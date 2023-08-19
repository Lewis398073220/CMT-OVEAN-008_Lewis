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
// TX: Transimt process
// RX: Receive process
// 16k: base 25M/208M(1300,1302) base 28M/104M(1400,1402)
| ----- | ------------------- | --------------------------------- | --------- | ------------- | ---------------------- |
| TX/RX | Algo                | description                       | MIPS(M)   | RAM(kB)       | Note                   |
|       |                     |                                   | 16k    8k | 16k        8k |                        |
| ----- | ------------------- | --------------------------------- | --------- | ------------- | ---------------------- |
|       | SPEECH_TX_DC_FILTER | Direct Current filter             | 1~2    \  | 0.05          |
|       | SPEECH_TX_AEC       | Acoustic Echo Cancellation(old)   | 40     \  |               | HWFFT: 37              |
|       | SPEECH_TX_AEC2      | Acoustic Echo Cancellation(new)   | 39     \  |               | enable NLP             |
|       | SPEECH_TX_AEC3      | Acoustic Echo Cancellation(new)   | 14/18  \  |               | 2 filters/4 filters    |
|       | SPEECH_TX_AEC2FLOAT | Acoustic Echo Cancellation(new)   | 23/22  \  | 29.3          | nlp/af(blocks=1)       |
|       | SPEECH_TX_AEC2FLOAT | Acoustic Echo Cancellation(ns)    | 14/10  \  |               | banks=256/banks=128    |
|       | (ns_enabled)        |                                   | 8/6    \  |               | banks=64/banks=32      |
|       | SPEECH_TX_NS        | 1 mic noise suppress(old)         | 30     \  |               | HWFFT: 19              |
|       | SPEECH_TX_NS2       | 1 mic noise suppress(new)         | 16     \  |               | HWFFT: 12              |
|       | SPEECH_TX_NS3       | 1 mic noise suppress(new)         | 26     \  | 33.3          |                        |
|       | SPEECH_TX_NN_NS     | 1 mic noise suppress(new)         | 117/52 \  | 78.7/20.3     | 16ms; large/small      |
|       | SPEECH_TX_NN_NS2    | 1 mic noise suppress(new)         | 37        | 30.3          |
|       | SPEECH_TX_NS2FLOAT  | 1 mic noise suppress(new float)   | 12.5   \  |               | banks=64               |
| TX    | SPEECH_TX_2MIC_NS   | 2 mic noise suppres(long space)   | \         |               |                        |
|       | SPEECH_TX_2MIC_NS2  | 2 mic noise suppres(short space)  | 20        | 14.8          | delay_taps 5M          |
|       |                     |                                   |           |               | freq_smooth_enable 1.5 |
|       |                     |                                   |           |               | wnr_enable 1.5M        |
|       | SPEECH_TX_2MIC_NS4  | sensor mic noise suppress         | 31.5      |               |                        |
|       | SPEECH_TX_2MIC_NS3  | 2 mic noise suppres(far field)    | \         |               |                        |
|       | SPEECH_TX_2MIC_NS5  | 2 mic noise suppr(left right end) | \         |               |                        |
|       | SPEECH_TX_2MIC_NS6  | 2 mic noise suppr(far field)      | 70        |               |                        |
|       | SPEECH_TX_2MIC_NS7  | 2 mic noise suppr(Cohernt&RLS)    | 58.4      | 38.9          |                        |
|       | SPEECH_TX_2MIC_NS8  | 2 mic noise suppr(DSB)            | 3.5    \  |               |                        |
|       | SPEECH_TX_3MIC_NS   | 3 mic 2FF+FB NS(new)              | 80        | 33.1          | Wnr_enable = 0         |
|       |                     |                                   |           |               | include 3mic_preprocess|
|       | SPEECH_TX_3MIC_NS2  | 3 mic 2FF+FB NS (Cohernt&RLS)     | 62        | 58.7          |                        |
|       | SPEECH_TX_3MIC_NS4  | 3 mic 2FF+FB                      | 67        | 54.6          | Wnr_enable = 0         |
|       |                     |                                   |           |               | include 3mic_preprocess|
|       | SPEECH_TX_AGC       | Automatic Gain Control            | 3         | 0.4           |                        |
|       | SPEECH_TX_COMPEXP   | Compressor and expander           | 4         | 0.6           |                        |
|       | SPEECH_TX_EQ        | Default EQ                        | 0.5     \ | 1.1           | each section           |
| ----- | ------------------- | --------------------------------- | --------- | ------------- | ---------------------- |
|       | SPEECH_RX_NS        | 1 mic noise suppress(old)         | 30      \ |               |                        |
| RX    | SPEECH_RX_NS2       | 1 mic noise suppress(new)         | 16      \ |               |                        |
|       | SPEECH_RX_NS2FLOAT  | 1 mic noise suppress(new float)   | 12.5   \  | 17.9          | banks=64               |
|       | SPEECH_RX_AGC       | Automatic Gain Control            | 3         | 0.4           |                        |
|       | SPEECH_RX_EQ        | Default EQ                        | 0.5     \ | 1.1           | each section           |
| ----- | ------------------- | --------------------------------- | --------- | ------------- | ---------------------- |
**/
#include "plat_types.h"
#include "bt_sco_chain_cfg.h"

#if defined(SPEECH_TX_3MIC_NS)
static float ff_fb_h[128] = {
    -3.536209897e-02f,  2.747677356e-01f,  1.513736714e-02f, -3.359935819e-01f,  4.034027264e-01f,
     6.513950245e-03f, -2.627881564e-01f,  2.672419432e-01f,  9.828153003e-02f, -2.861677581e-01f,
     2.752002164e-01f,  4.486147071e-02f, -1.359003882e-01f,  1.168803495e-01f,  9.356983101e-02f,
    -1.273871040e-01f,  1.172692997e-01f,  3.059422825e-02f, -6.262034757e-02f,  4.522764052e-02f,
     4.260237025e-02f, -4.400530535e-02f,  3.561566399e-02f,  1.616197177e-02f, -2.782377434e-03f,
     2.145913071e-02f,  1.110486882e-02f, -1.310118800e-03f,  2.637226549e-02f, -2.502889552e-02f,
     3.213671311e-02f,  2.839047526e-03f, -2.156885228e-02f,  2.996687569e-02f,  7.337173318e-03f,
    -1.510900610e-02f,  3.582110770e-02f, -4.841064082e-03f, -6.173312899e-03f,  2.888213663e-02f,
    -7.430927886e-03f, -9.273929214e-03f,  2.273848439e-02f, -1.171324420e-02f, -1.839470348e-03f,
     1.601604885e-02f, -1.312372679e-02f, -1.278061176e-04f,  1.763266834e-02f, -1.704632049e-02f,
     7.495756647e-03f,  8.047851793e-03f, -6.666279401e-03f,  3.136248959e-03f,  1.354572924e-02f,
    -3.821910487e-03f,  1.633452290e-02f,  5.851211693e-03f,  9.540661549e-03f,  1.741216787e-02f,
     8.217053062e-03f,  8.871537244e-03f,  2.185619005e-02f,  2.249070034e-03f,  1.370508646e-02f,
     1.440164873e-02f,  4.291840081e-03f,  7.975772513e-03f,  1.755103858e-02f, -4.683693373e-03f,
     1.286872064e-02f,  7.227141113e-03f,  5.176660419e-03f,  1.080589373e-03f,  3.481592943e-03f,
     1.214236349e-02f, -8.802942643e-03f,  5.576199762e-03f,  1.020466700e-02f, -6.824169920e-03f,
    -5.418927680e-04f,  1.974044341e-02f, -1.360438069e-02f,  1.573132223e-03f,  1.683345093e-02f,
    -4.559245909e-03f, -1.056942248e-02f,  2.617508941e-02f, -1.019651616e-02f, -3.355556775e-03f,
     1.863869755e-02f, -2.244323682e-04f, -8.918964014e-03f,  2.250218648e-02f,  1.405531992e-03f,
    -8.320744308e-03f,  1.812432982e-02f,  6.968074796e-05f, -3.398145823e-03f,  1.209676789e-02f,
    -4.751500997e-03f,  4.700467157e-03f,  6.524508745e-03f, -1.480328725e-03f, -1.226391034e-03f,
     1.327179639e-02f, -1.009567624e-02f,  1.077835184e-02f, -1.224925391e-03f, -5.019549895e-04f,
     1.081839697e-02f, -5.189895998e-04f, -1.168307227e-02f,  2.594526482e-02f, -1.186961999e-02f,
    -4.287941075e-04f,  1.526968668e-02f, -4.362664821e-03f, -4.419208409e-03f,  1.778672418e-02f,
    -9.398356292e-03f,  4.981260812e-04f,  1.023442923e-02f, -5.920170431e-03f, -8.016932871e-04f,
     6.851569190e-03f, -6.727521805e-03f,  3.688725745e-03f,
};
#endif

#if defined(SPEECH_TX_2MIC_NS7) || defined(SPEECH_TX_3MIC_NS) || defined(SPEECH_TX_MIC_FIR_CALIBRATION)
static float mic_calib_filter[128] = {
     6.202919261e-01f,  8.037469076e-01f, -1.600362194e-01f,  3.098796151e-01f, -3.221007158e-01f, 
     6.939522926e-02f, -2.197326434e-01f,  2.967445051e-01f, -1.539558681e-01f,  1.568673645e-01f, 
    -1.535676006e-01f,  1.369199806e-01f, -2.429241633e-01f,  6.370278107e-02f, -2.454364959e-01f, 
     1.543613835e-01f, -4.580953075e-02f,  1.705936100e-01f, -6.333049682e-02f,  1.238209135e-01f, 
    -1.269522492e-01f,  1.090682257e-02f, -1.690590571e-01f, -6.945397953e-03f, -6.874834727e-02f, 
     8.313176183e-02f,  3.683071812e-02f,  1.182126965e-01f, -8.714021074e-02f,  3.599273165e-02f, 
    -6.635071944e-02f, -7.726752726e-03f, -8.982504885e-02f,  2.535089617e-02f,  1.904896891e-03f, 
     1.137365307e-02f,  1.579722023e-02f,  2.234187135e-03f, -1.259744181e-02f, -1.052641627e-02f, 
    -3.160779299e-03f, -4.308732781e-03f, -2.812431946e-03f, -2.865492236e-02f,  7.055175018e-03f, 
     1.632257853e-03f, -2.453137528e-02f, -2.026903600e-02f,  1.625059811e-02f, -1.554967366e-02f, 
     2.293516485e-02f, -3.462459914e-02f,  1.177648751e-02f, -1.107641958e-02f,  1.647448209e-02f, 
    -1.275720956e-02f,  1.870987238e-02f, -9.541430327e-03f, -2.279155082e-02f, -1.880658300e-02f, 
    -1.747780988e-03f, -2.118554372e-02f,  2.768046975e-02f, -6.465657026e-03f,  9.752833140e-03f, 
    -9.199369244e-04f,  1.034062978e-02f, -2.557081197e-02f, -1.801663383e-02f, -2.738124722e-02f, 
     1.130984266e-02f,  1.041767636e-02f,  1.666619542e-02f, -3.742378371e-03f,  4.828140526e-03f, 
    -3.255728256e-03f, -1.305059016e-02f, -1.090098809e-02f,  4.608301917e-03f, -5.172471726e-03f, 
     2.755225255e-02f, -1.551118604e-02f,  6.938741753e-03f, -7.030827325e-03f, -1.026446677e-02f, 
     6.106218967e-03f, -5.845722644e-03f, -5.205985339e-03f,  1.087467760e-02f,  5.145789063e-03f, 
    -6.800453577e-03f, -2.537260719e-03f, -1.509579397e-03f, -2.865543830e-02f, -2.042679265e-02f, 
     8.295039855e-02f,  1.629582054e-02f, -3.068044115e-02f, -2.584086633e-02f, -3.042421751e-02f, 
     1.623167179e-02f, -5.899060800e-02f,  1.060265049e-01f, -8.688999331e-02f,  6.031029718e-02f, 
     2.415070403e-02f, -1.989490836e-02f, -3.644763576e-02f, -1.889818182e-02f, -3.096998379e-02f, 
     2.044977544e-02f,  5.202091545e-02f, -6.494013764e-03f,  1.345467028e-03f,  2.218268903e-02f, 
    -1.594742541e-02f, -1.927969228e-02f,  1.264985783e-02f, -3.812827724e-02f,  8.355320499e-03f, 
    -6.805840437e-03f,  2.302310573e-02f, -3.559824605e-03f, -4.154173247e-03f, -2.753796987e-03f, 
    -3.451901209e-03f, -1.141899385e-02f, -5.590265047e-03f, 
};
#endif

#if defined(SPEECH_TX_MIC_CALIBRATION)
/****************************************************************************************************
 * Describtion:
 *     Mic Calibration Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     mic_num(1~4): the number of microphones. The filter num is (mic_num - 1)
 *     calib: {bypass, gain, num, {type, frequency, gain, q}}. Please refer to SPEECH_TX_EQ section
 *         in this file
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 0.5M/section;
 * Note:
 *     None
****************************************************************************************************/
const SpeechIirCalibConfig WEAK speech_tx_mic_calib_cfg =
{
    .bypass = 0,
    .mic_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM,
    .delay = 0,
    .calib = {
        {
            .bypass = 0,
            .gain = 0.f,
            .num = 1,
            .params = {
                {IIR_BIQUARD_LOWSHELF, {{150, -2.5, 0.707}}},
            }
        },
    },
};
#endif

#if defined(SPEECH_TX_MIC_FIR_CALIBRATION)
/****************************************************************************************************
 * Describtion:
 *     Mic Calibration Equalizer, implementation with fir filter
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     mic_num(1~4): the number of microphones. The filter num is (mic_num - 1)
 *     calib: {filter, filter_length, nfft}. 
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 0.5M/section;
 * Note:
 *     None
****************************************************************************************************/
float mic2_ft_caliration[256] = {1.f, };
const SpeechFirCalibConfig WEAK speech_tx_mic_fir_calib_cfg =
{
    .bypass = 0,
    .mic_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM,
    .delay = 2,
    .calib = {
        {
            .filter = mic_calib_filter,
            .filter_length = ARRAY_SIZE(mic_calib_filter),
        },
    },
};
#endif

const SpeechConfig WEAK speech_cfg_default = {

#if defined(SPEECH_TX_DC_FILTER)
    .tx_dc_filter = {
        .bypass                 = 0,
        .gain                   = 24,
    },
#endif

#if defined(SPEECH_TX_AEC)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay(>0): delay samples from mic to speak, unit(sample).
 *     leak_estimate(0~32767): echo supression value. This is a fixed mode. It has relatively large
 *         echo supression and large damage to speech signal.
 *     leak_estimate_shift(0~8): echo supression value. if leak_estimate == 0, then leak_estimate_shift
 *         can take effect. This is a adaptive mode. It has relatively small echo supression and also 
 *         small damage to speech signal.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 40M;
 * Note:
 *     If possible, use SPEECH_TX_AEC2FLOAT
****************************************************************************************************/
    .tx_aec = {
        .bypass                 = 0,
        .delay                  = 60,
        .leak_estimate          = 16383,
        .leak_estimate_shift    = 4,
    },
#endif

#if defined(SPEECH_TX_AEC2)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     .
 *     .
 *     .
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, enNlpEnable = 1, 39M;
 * Note:
 *     If possible, use SPEECH_TX_AEC2FLOAT
****************************************************************************************************/
    .tx_aec2 = {
        .bypass                 = 0,
        .enEAecEnable           = 1,
        .enHpfEnable            = 1,
        .enAfEnable             = 0,
        .enNsEnable             = 0,
        .enNlpEnable            = 1,
        .enCngEnable            = 0,
        .shwDelayLength         = 0,
        .shwNlpBandSortIdx      = 0.75f,
        .shwNlpBandSortIdxLow   = 0.5f,
        .shwNlpTargetSupp       = 3.0f,
        .shwNlpMinOvrd          = 1.0f,
    },
#endif

#if defined(SPEECH_TX_AEC2FLOAT)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     hpf_enabled(0/1): high pass filter enable or disable. Used to remove DC for Near and Ref signals.
 *     af_enabled(0/1): adaptive filter enable or disable. If the echo signal is very large, enable this
 *     adprop_enabled(0/1): update the filter coefficient according to the blocks enable or disable. If the number of blocks is bigger than 1, enable this.
 *     varistep_enabled(0/1): variable step size enable or disable. When dealing with the double-talk situation, enable this.
 *     nlp_enabled(0/1): non-linear process enable or disable. Enable this by default.
 *     clip_enabled(0/1): residual echo suppression enable or disable.
 *     stsupp_enabled(0/1): single-talk suppression enable or disable.
 *     hfsupp_enabled(0/1): high_frequency echo suppression enable or disable.
 *     constrain_enabled(0/1): constrain background noise enable or disable.
 *     ns_enabled(0/1): noise supression enable or disable. Enable this by default.
 *     cng_enabled(0/1):  comfort noise generator enable or disable.
 *     blocks(1~8): the length of adaptive filter = blocks * frame length
 *     delay(>0): delay samples from mic to speak, unit(sample).
 *     gamma(0~1): forgetting factor for psd estimation.
 *     echo_band_start(0~8000): start band for echo detection, unit(Hz)
 *     echo_band_end(echo_band_start~8000): end band for echo detection, unit(Hz)
 *     min_ovrd(1~6): supression factor, min_ovrd becomes larger and echo suppression becomes larger.
 *     target_supp(<0): target echo suppression, unit(dB)
 *     highfre_band_start(0~8000): start band for high_frequency echo suppression, unit(Hz)
 *     highfre_supp(>=0):high_frequency echo suppression, unit(dB) non-negative number!!!
 *     noise_supp(-30~0): noise suppression, unit(dB)
 *     cng_type(0/1): noise type(0: White noise; 1: Pink noise)
 *     cng_level(<0): noise gain, unit(dB)
 *     clip_threshold: compression threshold
 *     banks:Parameters for NS, bigger then better but cost more.
 * Resource consumption:
 *     RAM:     42K
 *     FLASH:   None
 *     MIPS:    30M(one block);62M(six blocks)    fs = 16kHz;
 * Note:
 *     This is the recommended AEC
****************************************************************************************************/
    .tx_aec2float = {
        .bypass         = 0,
        .hpf_enabled    = false,
        .af_enabled     = false,
        .adprop_enabled    = false,
        .varistep_enabled    = false,
        .nlp_enabled    = true,
        .clip_enabled   = false,
        .stsupp_enabled = false,
        .hfsupp_enabled = false,
        .constrain_enabled = false,
#if defined(SPEECH_TX_NS3) || defined(SPEECH_TX_NS4) || defined(SPEECH_TX_NS5) || defined(SPEECH_TX_DTLN)
        .ns_enabled     = false,
#else
        .ns_enabled     = true,
#endif
        .cng_enabled    = false,
        .blocks         = 1,
#if defined(SPEECH_TX_AEC_CODEC_REF)
        .delay          = 70,
#else
        .delay          = 139,
#endif
        .gamma          = 0.9,
        .echo_band_start = 300,
        .echo_band_end  = 1800,
        .min_ovrd       = 2,
        .target_supp    = -40,
        .highfre_band_start = 4000,
        .highfre_supp   = 8.f,
        .noise_supp     = -15,
        .cng_type       = 1,
        .cng_level      = -60,
        .clip_threshold = -20.f,
        .banks          = 64,
    },
#endif

#if defined(SPEECH_TX_AEC3)
    .tx_aec3 = {
        .bypass         = 0,
        .filter_size     = 16,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns = {
        .bypass             = 0,
        .alpha_h            = 0.8,
        .alpha_slow         = 0.9,
        .alpha_fast         = 0.6,
        .thegma             = 0.01,
        .thre_corr          = 0.2,
        .thre_filter_diff   = 0.2,
        .cal_left_gain      = 1.0,
        .cal_right_gain     = 1.0,
        .delay_mono_sample  = 6,
        .wnr_enable         = 0,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS2)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_taps(0~4): ceil{[d(MIC1~mouth) - d(MIC2~mouth)] / 2}.
 *         ceil: Returns the largest integer less than or equal to the specified data
 *         d(MIC~mouth): The dinstance from MIC to mouth
 *         e.g. 0: 0~2cm; 1: 2~4; 2: 5~6...
 *     freq_smooth_enable(1): Must enable
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 *     skip_freq_range_start(0~8000): skip dualmic process between skip_freq_range_start and skip_freq_range_end
 *     skip_freq_range_end(0~8000): skip dualmic process between skip_freq_range_start and skip_freq_range_end
 *     betalow(-0.1~0.1): suppression factor for frequency below 500Hz, large means more suppression
 *     betamid(-0.2~0.2): suppression factor for frequency between 500Hz and 2700Hz, large means more suppression
 *     betahigh(-0.3~0.3): suppression factor for frequency between 2700Hz and 8000Hz, large means more suppression
 *     vad_threshold(-1~0): threshold for vad, large means detection is more likely to be triggered
 *     vad_threshold(0~0.1): flux for threshold to avoid miss detection
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 32M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns2 = {
        .bypass             = 0,
        .delay_taps         = 0.f,  
        .dualmic_enable     = 1,
        .freq_smooth_enable = 1,
        .wnr_enable         = 1, 
        .skip_freq_range_start = 0,
        .skip_freq_range_end = 0,       // TODO: Jay: Audio developer has a bug, int type can not be negative
        .noise_supp         = -40,
        .betalow            = 0.07f, // 500
        .betamid            = -0.1f, // 2700
        .betahigh           = -0.2f, // above 2700
        .vad_threshold      = 0.f,
        .vad_threshold_flux = 0.05f,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS5)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_taps(0~4): ceil{[d(MIC1~mouth) - d(MIC2~mouth)] / 2}. Default as 0
 *         ceil: Returns the largest integer less than or equal to the specified data
 *         d(MIC~mouth): The dinstance from MIC to mouth
 *         e.g. 0: 0~2cm; 1: 2~4; 2: 5~6...
 *     freq_smooth_enable(1): Must enable
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 *     delay_enable(0/1): enable the delay_taps or not. Ideally, never need to enable the delay and
 *          delay_taps will be a useless parameter.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 32M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns5 = {
        .bypass             = 0,
        .delay_taps         = 0.0f,
        .freq_smooth_enable = 1,
        .wnr_enable         = 0, 
        .delay_enable       = 0,
    },
#endif


#if defined(SPEECH_TX_2MIC_NS6)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 *     denoise_dB : The minimum gain in the MMSE NS algorithm which is integrated in wind noise.
 * Resource consumption:
 *     RAM:     TBD
 *     FLASE:   TBD
 *     MIPS:    fs = 16kHz, TBD;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns6 = {
        .bypass             = 0,
        .wnr_enable         = 0, 
        .denoise_dB       = -12,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS4)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns4 = {
        .bypass             = 0,
        .wnr_enable =   1,
        .wind_thd =     0.8,
        .wnd_pwr_thd =  20,
        .wind_gamma =   0.7,
        .state_trans_frame_thd = 60,
        .af_enable =    1,
        .filter_gamma = 0.9,

        .vad_bin_start = 50,
        .vad_bin_end =  1000,
        .coef1_thd =    0.85F,
        .coef2_thd =    3.5F,

        .low_ram_enable=    0,
        .echo_supp_enable=  1,
        .ref_delay= 0,

        .post_supp_enable=  1,
        .denoise_db=    -20,

        .blend_en = 1,
        .comp_num = 4,
        .comp_freq = { 600, 1000, 3000, 4000},
        .comp_gaindB = { -5, 0, 8, 5 },
    },
#endif

#if defined(SPEECH_TX_2MIC_NS7)
/****************************************************************************************************
* Describtion:
*     High Performance 2 MICs BF Noise Suppression
*
* Parameters:
*     bypass(0/1): bypass enable or disable.
*     wnr_enable(0/1): bypass wind enhancment enable or disable.
*     wind_thd(0.1-0.95): wind threshold for wind enhancement function.
*     wnd_pwr_thd(1:32768): wind pwr threshold for wind enhancement function.
*     wind_gamma(0.1-0.95): wind convergence speed tunning key.
*     state_trans_frame_thd(1:32768): wind state threshold for wind enhancement function.
*     af_enable(0/1): bypass denoise enable or disable.
*     filter_gamma(0.1-0.99): denoise convergence speed tunning key.
*     vad_bin_start1(0-fs/2): denoise tunning para.
*     vad_bin_end1(0-fs/2): denoise tunning para.
*     vad_bin_start2(0-fs/2): denoise tunning para.
*     vad_bin_end2(0-fs/2): denoise tunning para.
*     vad_bin_start3(0-fs/2): denoise tunning para.
*     vad_bin_end3(0-fs/2): denoise tunning para.
*     coef1_thd(0.1-0.999): denoise tunning para.
*     coef2_thd(0.1-0.999): denoise tunning para.
*     coef3_thd(0.1-32768): denoise tunning para.
*     calib_enable(0/1): bypass calibration enable or disable.
*     calib_delay(0-512): delay para for calibration.
*     filter(float point): filter coef for calibration.
*     filter_len(16-256):filter length for calibration.
*     low_ram_enable(0/1): enable low ram mode to reduce RAM size & reduce MIPS
*     low_mips_enable(0/1): enable low mips mode to reduce MIPS
*     echo_supp_enable(0/1): enable echo suppression function
*     ref_delay(-240-240): delay for ref signal (no use so far, TBD)
*     post_supp_enable(0/1): enable post denoise function
*     denoise_db(0--50): max denoise dB for post denoise

* Resource consumption:
*-----If low_ram_enable == 0 && low_mips_enable == 0
*     Basic RAM = 40.3K, MIPS = 42M
*     IF set calib_enable = 1. Need more RAM += 4.3K & MIPS += 8M
*     IF set echo_supp_enable = 1. Need more RAM += 10.7K & MIPS += 19M
*     IF set post_supp_enable = 1. Need more RAM += 12K & MIPS += 22M
*     Total: RAM = 70K, MIPS = 91M

*-----If low_ram_enable == 1 && low_mips_enable == 0
*     Basic RAM = 21K, MIPS = 21M
*     IF set calib_enable = 1. Need more RAM += 2.2K & MIPS += 8M
*     IF set echo_supp_enable = 1. Need more RAM += 5.2K & MIPS += 9M
*     IF set post_supp_enable = 1. Need more RAM += 7.5K & MIPS += 12M
*     Total: RAM = 36K, MIPS = 50M

*-----If low_ram_enable == 0 && low_mips_enable == 1
*     Basic RAM = 40K, MIPS = 21M
*     IF set calib_enable = 1. Need more RAM += 4K & MIPS += 8M
*     IF set echo_supp_enable = 1. Need more RAM += 11K & MIPS += 9M
*     IF set post_supp_enable = 1. Need more RAM += 12K & MIPS += 12M
*     Total: RAM = 70K, MIPS = 50M

****************************************************************************************************/
    .tx_2mic_ns7 = {
        .bypass =       0,
        .wnr_enable =   1,
        .wind_thd =     0.7,
        .wnd_pwr_thd =  1600,
        .wind_gamma =   0.7,
        .state_trans_frame_thd = 60,
        .af_enable =    0,
        .filter_gamma = 0.9,
        .vad_bin_start1 =   50,
        .vad_bin_end1 =   3230,
        .vad_bin_start2 =   300,
        .vad_bin_end2 =   930,
        .vad_bin_start3 =   150,
        .vad_bin_end3 =   1500,
        .coef1_thd =    0.94F,
        .coef2_thd =    0.9F,
        .coef3_thd =    30.0F,
        .supp_times =   2,

        .calib_enable=  0,
        .calib_delay=   2,
        .filter=       mic_calib_filter,
        .filter_len=  ARRAY_SIZE(mic_calib_filter),

        .low_ram_enable=    0,
        .low_mips_enable=    0,

        .echo_supp_enable=  1,
        .ref_delay= 0,

        .post_supp_enable=  1,
        .denoise_db=    -10,
        .wdrc_enable = 1,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS8)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_taps(0~4): ceil{[d(MIC1~mouth) - d(MIC2~mouth)] / 2}. Default as 0
 *         ceil: Returns the largest integer less than or equal to the specified data
 *         d(MIC~mouth): The dinstance from MIC to mouth
 *         e.g. 0: 0~2cm; 1: 2~4; 2: 5~6...
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 32M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns8 = {
        .bypass             = 0,
        .dist               = 0.028f,
        .min_frequency      = 150.f,
        .max_frequency      = 3000.f,
        .energy_threshold   = 1.5f,
        .corr_threshold     = 0.9f,
        .wind_supp_enable   = 1,
        .echo_supp_enable   = 1,
        .post_supp_enable   = 1,
        .wdrc_enable        = 1,
        .pre_gain           = 10,
        .post_gain          = 14,
        .denoise_dB         = -15,
        .dnn_denoise_dB     = -35,
    },
#endif

#if defined(SPEECH_TX_3MIC_NS)
/****************************************************************************************************
 * Describtion:
 *     3 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_tapsM(0~4): MIC L/R delay samples. Refer to SPEECH_TX_2MIC_NS2 delay_taps
 *     delay_tapsS(0~4): MIC L/S delay samples. Refer to SPEECH_TX_2MIC_NS2 delay_taps
 *     freq_smooth_enable(1): Must enable
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_3mic_ns = {
        .bypass =       0,
        .wnr_enable =   1,
        .wind_thd =     0.7,
        .wnd_pwr_thd =  1600,
        .wind_gamma =   0.7,
        .state_trans_frame_thd = 30,
        .af_enable =    1,
        .filter_gamma = 0.9,
        .vad_bin_start1 =   50,
        .vad_bin_end1 =   3230,
        .vad_bin_start2 =   300,
        .vad_bin_end2 =   930,
        .vad_bin_start3 =   50,
        .vad_bin_end3 =   1100,
        .vad_bin_start4 = 150,
        .vad_bin_end4 = 1000,
        .coef1_thd =    0.94F,
        .coef2_thd =    0.9F,
        .coef3_thd =    25.0F,
        .coef4_thd =    0.80,

        .calib_enable=  1,
        .calib_delay=   2,
        .filter=       mic_calib_filter,
        .filter_len=  ARRAY_SIZE(mic_calib_filter),

        .low_ram_enable=    0,
        .low_mips_enable=    1,

        .echo_supp_enable=  0,
        .ref_delay= 0,

        .post_supp_enable=  1,
        .post_denoise_db = -20,
        .dnn_denoise_db = -25,
        .inner_denoise_db = -18,
        .spectral_smooth_enable = 0,

        .blend_en = 1,

        .ff_fb_coeff = ff_fb_h,
        .ff_fb_coeff_len = ARRAY_SIZE(ff_fb_h),

        .comp_num = 4,
        .comp_freq = { 450, 2000, 2800, 4000 },
        .comp_gaindB = { -8, -2, 4, 10 },
    },
#endif

#if defined(SPEECH_TX_3MIC_NS2)
/****************************************************************************************************
 * Describtion:
 *     3 MICs Noise Suppression2
 * Parameters:

 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_3mic_ns2 = {
		.bypass      = 0,
		.wnr_enable  = 1,
		.denoise_dB  = -6,
		.delay_taps  = 0.6,
		.freq_smooth_enable = 1,
		.crossover_freq = 1000,
	},
#endif

#if defined(SPEECH_TX_3MIC_NS3)
/****************************************************************************************************
 * Describtion:
 *     3 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_taps(0~4): ceil{[d(MIC1~mouth) - d(MIC2~mouth)] / 2}.
 *         ceil: Returns the largest integer less than or equal to the specified data
 *         d(MIC~mouth): The dinstance from MIC to mouth
 *         e.g. 0: 0~2cm; 1: 2~4; 2: 5~6...
 *     freq_smooth_enable(1): Must enable
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 32M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_3mic_ns3 = {
        .bypass             = 0,
        .endfire_enable     = 1,
        .broadside_enable   = 1,
        .delay_taps         = 0.7f,  
        .freq_smooth_enable = 1,
        .wnr_enable         = 0, 
    },
#endif

#if defined(SPEECH_TX_FB_AEC)
    .tx_fb_aec = {
        .bypass         = 0,
        .nfft_len       = 128;
        .af_enabled     = true,
        .nlp_enabled    = true,
        .blocks         = 1,
        .delay          = 70,
        .gamma          = 0.9,
        .echo_band_start = 300,
        .echo_band_end  = 1800,
        .min_ovrd       = 2,
        .target_supp    = -40,
    },
#endif
#if defined(SPEECH_TX_NS)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 30M;
 * Note:
 *     If possible, use SPEECH_TX_NS2 or SPEECH_TX_NS2FLOAT
****************************************************************************************************/
    .tx_ns = {
        .bypass     = 0,
        .denoise_dB = -12,
    },
#endif

#if defined(SPEECH_TX_NS2)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     denoise_dB(-30~0): noise suppression, unit(dB). 
 *         e.g. -15: Can reduce 15dB of stationary noise.
 * Resource consumption:
 *     RAM:     fs = 16k:   RAM = 8k; 
 *              fs = 8k:    RAM = 4k;
 *              RAM = frame_size * 30
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 16M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_ns2 = {
        .bypass     = 0,
        .denoise_dB = -15,
    },
#endif

#if defined(SPEECH_TX_NS2FLOAT)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     denoise_dB(-30~0): noise suppression, unit(dB). 
 *         e.g. -15: Can reduce 15dB of stationary noise.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     This is a 'float' version for SPEECH_TX_NS2. 
 *     It needs more MIPS and RAM, but can redece quantization noise.
****************************************************************************************************/
    .tx_ns2float = {
        .bypass     = 0,
        .denoise_dB = -15,
        .banks      = 64,
    },
#endif

#if defined(SPEECH_TX_NS3)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     mode: None
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_ns3 = {
        .bypass = 0,
        .denoise_dB = -18,
    },
#endif

#if defined(SPEECH_TX_NS4)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     COST:    FFT=512:  RAM = 22.3k, MIPS = 24M;
                FFT=256:  RAM = 11.3K, MIPS = 24M;(Need to open MACRO BT_SCO_LOW_RAM in bt_sco_chain.c)
 * Note:
****************************************************************************************************/
    .tx_ns4 = {
        .bypass     = 0,
        .denoise_dB = -18,
    },
#endif

#if defined(SPEECH_TX_NS5)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:

 * Note:
****************************************************************************************************/
    .tx_ns5 = {
        .bypass     = 0,
        .denoise_dB = -30,
    },
#endif

#if defined(SPEECH_TX_NS8)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:

 * Note:
****************************************************************************************************/
    .tx_ns8 = {
        .bypass     = 0,
        .denoise_dB = -10,
        .ref_delay = 148,
        .gamma = 0.9,
        .echo_band_start = 300,
        .echo_band_end = 7000,
        .min_ovrd = 2,
        .target_supp = -50,
        .ga_thr = 0.7,
        .en_thr = 100,
    },
#endif

#if defined(SPEECH_TX_WNR)
/****************************************************************************************************
 * Describtion:
 *     Wind Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     lpf_cutoff: lowpass filter for wind noise detection
 *     hpf_cutoff: highpass filter for wind noise suppression
 *     power_ratio_thr: ratio of the power spectrum of the lower frequencies over the total power
                        spectrum for all frequencies
 *     power_thr: normalized power of the low frequencies
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_wnr = {
        .bypass = 0,
        .lpf_cutoff   = 1000,
        .hpf_cutoff = 400,
        .power_ratio_thr = 0.9f,
        .power_thr = 1.f,
    },
#endif

#if defined(SPEECH_TX_NOISE_GATE)
/****************************************************************************************************
 * Describtion:
 *     Noise Gate
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     data_threshold(0~32767): distinguish between noise and speech, unit(sample).
 *     data_shift(1~3): 1: -6dB; 2: -12dB; 3: -18dB
 *     factor_up(float): attack time, unit(s)
 *     factor_down(float): release time, unit(s)
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_noise_gate = {
        .bypass         = 0,
        .data_threshold = 640,
        .data_shift     = 1,
        .factor_up      = 0.001f,
        .factor_down    = 0.5f,
    },
#endif

#if defined(SPEECH_TX_COMPEXP)
/****************************************************************************************************
 * Describtion:
 *     Compressor and expander
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     ...
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_compexp = {
        .bypass             = 0,
        .type               = 0,
        .comp_threshold     = -25.f,
        .comp_ratio         = 3.f,
        .expand_threshold   = -60.f,
        .expand_ratio       = 0.333f,
        .attack_time        = 0.008f,
        .release_time       = 0.06f,
        .makeup_gain        = 15,
        .delay              = 128,
        .tav                = 0.2f,
    },
#endif

#if defined(SPEECH_TX_AGC)
/****************************************************************************************************
 * Describtion:
 *     Automatic Gain Control
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     target_level(>0): signal can not exceed (-1 * target_level)dB.
 *     compression_gain(>0): excepted gain.
 *     limiter_enable(0/1): 0: target_level does not take effect; 1: target_level takes effect.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 3M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_agc = {
        .bypass             = 0,
        .target_level       = 3,
        .compression_gain   = 6,
        .limiter_enable     = 1,
    },
#endif

#if defined(SPEECH_TX_EQ)
/****************************************************************************************************
 * Describtion:
 *     Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     gain(float): normalized gain. It is usually less than or equal to 0
 *     num(0~6): the number of EQs
 *     params: {type, frequency, gain, q}. It supports a lot of types, please refer to iirfilt.h file
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 0.5M/section;
 * Note:
 *     None
****************************************************************************************************/
    .tx_eq = {
        .bypass     = 0,
        .gain       = 0.f,
        .num        = 1,
        .params = {
			//{IIR_BIQUARD_HIGHSHELF, {{4000, 0, 0.707}}}, //disable by jay
			{IIR_BIQUARD_PEAKINGEQ, {{5000, -5, 1.2}}},  //Modified by Jay
        },
    },
#endif

#if defined(SPEECH_TX_POST_GAIN)
/****************************************************************************************************
 * Describtion:
 *     Linear Gain
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_post_gain = {
        .bypass     = 0,
        .gain_dB    = 6.0f,
    },
#endif

// #if defined(SPEECH_CS_VAD)
// /****************************************************************************************************
//  * Describtion:
//  *     Voice Activity Detector
//  * Parameters:
//  *     bypass(0/1): bypass enable or disable.
//  * Resource consumption:
//  *     RAM:     None
//  *     FLASE:   None
//  *     MIPS:    fs = 16kHz;
//  * Note:
//  *     None
// ****************************************************************************************************/
//     .tx_vad = {
//         .snrthd     = 5.f,
//         .energythd  = 100.f,
//     },
// #endif

#if defined(SPEECH_RX_NS)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     Refer to SPEECH_TX_NS
 * Note:
 *     None
****************************************************************************************************/
    .rx_ns = {
        .bypass     = 0,
        .denoise_dB = -12,
    },
#endif

#if defined(SPEECH_RX_NS2)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     Refer to SPEECH_TX_NS2
 * Note:
 *     None
****************************************************************************************************/
    .rx_ns2 = {
        .bypass     = 0,
        .denoise_dB = -15,
    },
#endif

#if defined(SPEECH_RX_NS2FLOAT)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     Refer to SPEECH_TX_NS2FLOAT
 * Note:
 *     None
****************************************************************************************************/
    .rx_ns2float = {
        .bypass     = 0,
        .denoise_dB = -15,
        .banks      = 64,
        .wdrc_enable = 1,
        .expander_threshold = -70,
        .expander_ratio = 0.7,
    },
#endif

#if defined(SPEECH_RX_NS3)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     Refer to SPEECH_TX_NS3
 * Note:
 *     None
****************************************************************************************************/
    .rx_ns3 = {
        .bypass = 0,
        .mode   = NS3_SUPPRESSION_HIGH,
    },
#endif

#if defined(SPEECH_RX_NOISE_GATE)
/****************************************************************************************************
 * Describtion:
 *     Noise Gate
 * Parameters:
 *     Refer to SPEECH_TX_NOISE_GATE
 * Note:
 *     None
****************************************************************************************************/
    .rx_noise_gate = {
        .bypass         = 0,
        .data_threshold = 640,
        .data_shift     = 1,
        .factor_up      = 0.001f,
        .factor_down    = 0.5f,
    },
#endif

#if defined(SPEECH_RX_COMPEXP)
/****************************************************************************************************
 * Describtion:
 *     Compressor and expander
 * Parameters:
 *     Refer to SPEECH_TX_COMPEXP
 * Note:
 *     None
****************************************************************************************************/
    .rx_compexp = {
        .bypass = 0,
        .num = 2,
        .xover_freq = {5000},
        .order = 4,
        .params = {
            {
                .bypass             = 0,
                .type               = 0,
                .comp_threshold     = -10.f,
                .comp_ratio         = 2.f,
                .expand_threshold   = -60.f,
                .expand_ratio       = 0.5556f,
                .attack_time        = 0.001f,
                .release_time       = 0.006f,
                .makeup_gain        = 0,
                .delay              = 128,
                .tav                = 0.2f,
            },
            {
                .bypass             = 0,
                .type               = 0,
                .comp_threshold     = -10.f,
                .comp_ratio         = 2.f,
                .expand_threshold   = -60.f,
                .expand_ratio       = 0.5556f,
                .attack_time        = 0.001f,
                .release_time       = 0.006f,
                .makeup_gain        = 0,
                .delay              = 128,
                .tav                = 0.2f,
            }
        }
    },
#endif

#if defined(SPEECH_RX_AGC)
/****************************************************************************************************
 * Describtion:
 *      Automatic Gain Control
 * Parameters:
 *     Refer to SPEECH_TX_AGC
 * Note:
 *     None
****************************************************************************************************/
    .rx_agc = {
        .bypass             = 0,
        .target_level       = 3,
        .compression_gain   = 6,
        .limiter_enable     = 1,
    },
#endif

#if defined(SPEECH_RX_EQ)
/****************************************************************************************************
 * Describtion:
 *     Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     Refer to SPEECH_TX_EQ
 * Note:
 *     None
****************************************************************************************************/
    .rx_eq = {
        .bypass = 0,
        .gain   = 0.f,
        .num    = 1,
        .params = {
            //{IIR_BIQUARD_HPF, {{60, 0, 0.707f}}},
            //{IIR_BIQUARD_LOWSHELF, {{100, -6, 1.0}}},
            {IIR_BIQUARD_PEAKINGEQ, {{5600, -4, 2.0}}},  //Modified by Jay
            //{IIR_BIQUARD_PEAKINGEQ, {{900,-9, 1.3}}},
            //{IIR_BIQUARD_PEAKINGEQ, {{7000,-10, 0.6}}},
        },
    },
#endif

#if defined(SPEECH_RX_HW_EQ)
/****************************************************************************************************
 * Describtion:
 *     Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     Refer to SPEECH_TX_EQ
 * Note:
 *     None
****************************************************************************************************/
    .rx_hw_eq = {
        .bypass = 0,
        .gain   = 0.f,
        .num    = 1,
        .params = {
            {IIR_BIQUARD_HPF, {{60, 0, 0.707f}}},
        },
    },
#endif

#if defined(SPEECH_RX_DAC_EQ)
/****************************************************************************************************
 * Describtion:
 *     Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     Refer to SPEECH_TX_EQ
 * Note:
 *     None
****************************************************************************************************/
    .rx_dac_eq = {
        .bypass = 0,
        .gain   = 0.f,
        .num    = 1,
        .params = {
            {IIR_BIQUARD_HPF, {{60, 0, 0.707f}}},
        },
    },
#endif

#if defined(SPEECH_RX_POST_GAIN)
/****************************************************************************************************
 * Describtion:
 *     Linear Gain
 * Parameters:
 *     Refer to SPEECH_TX_POST_GAIN
 * Note:
 *     None
****************************************************************************************************/
    .rx_post_gain = {
        .bypass     = 0,
        .gain_dB    = 0.0f, //6.0f,  //Modified by Jay
    },
#endif

};
