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
#ifdef VOICE_ASSIST_FF_FIR_LMS
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "app_voice_assist_fir_lms.h"
#include "anc_ff_fir_lms.h"
#include "anc_assist.h"
#include "anc_process.h"
#include "audio_dump.h"
#include "app_utils.h"
#include "app_media_player.h"

static ANCFFFirLmsSt * fir_st= NULL;

static ANC_FF_FIR_LMS_CFG_T cfg ={
    .max_cnt = 150,
    .period_cnt = 2,
};

static int stop_flag = 0;
int32_t *fir_coeff_cache;


static int32_t _voice_assist_fir_lms_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_fir_lms_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_FIR_LMS, _voice_assist_fir_lms_callback);
    return 0;
}

int32_t app_voice_assist_fir_lms_reset(void)
{
    anc_ff_fir_lms_reset(fir_st, 0);
    return 0;
}

void set_fixed_fir_filter(void)
{
    anc_ff_fir_lms_reset(fir_st, 1);
}


int32_t app_voice_assist_fir_lms_open(void)
{
    TRACE(0, "[%s] fir lms start stream", __func__);
    media_PlayAudio(AUD_ID_ANC_PROMPT, 0);
    stop_flag = 0;
    app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_208M);
    
    fir_st = anc_ff_fir_lms_create(16000, 120, &cfg);
    app_anc_assist_open(ANC_ASSIST_USER_FIR_LMS);
    fir_coeff_cache = fir_lms_coeff_cache(fir_st);
    
    // close fb anc for adaptive anc, it is better not to open it during the init state
    anc_set_gain(0, 0, ANC_FEEDBACK);

    return 0;
}

int32_t app_voice_assist_fir_lms_close(void)
{
    TRACE(0, "[%s] fir lms close stream", __func__);
    anc_ff_fir_lms_destroy(fir_st);
    app_anc_assist_close(ANC_ASSIST_USER_FIR_LMS);
    anc_set_gain(512, 512, ANC_FEEDBACK);
    app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
#ifdef FIR_LMS_DUMP
    audio_dump_deinit();
#endif
    return 0;
}



static int32_t _voice_assist_fir_lms_callback(void * buf, uint32_t len, void *other)
{

    if (stop_flag == 0) {
        float ** input_data = buf;
        float * ff_data = input_data[0];  // error
        float * fb_data = input_data[1];  // error
        float * ref_data = input_data[2];

        int32_t res = anc_ff_fir_lms_process(fir_st, ff_data, fb_data, ref_data, 120);
        if (res == 1) {
            stop_flag = 1;
            fir_coeff_cache = fir_lms_coeff_cache(fir_st);
            app_voice_assist_fir_lms_close();
        } else if (res == 2) {      // speaking
            stop_flag = 1;
            app_voice_assist_fir_lms_close();
        } else if (res == 3) {     // large leak
            stop_flag = 1;
            app_voice_assist_fir_lms_close();
        }
    } else {
        return 0;
    }
    return 0;
    
}
#endif