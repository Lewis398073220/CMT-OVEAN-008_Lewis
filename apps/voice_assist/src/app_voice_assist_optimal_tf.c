/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "app_voice_assist_optimal_tf.h"
#include "anc_optimal_tf.h"
#include "anc_assist.h"
#include "app_anc.h"
#include "heap_api.h"
#include "hal_codec.h"

#define STAGE_NUM (10)
const static float faet_thd[STAGE_NUM-1] = {-10.75, -9.6, -8.45, -7.3, -6.15, -5.0, -3.85, -2.7, -1.55};

float Sz_estimate[120] = {1.0,0.0};
typedef struct
{
    anc_optimal_tf_inst *anc_inst;
    ANC_OPTIMAL_TF_STAGE last_stage;
    ANC_OPTIMAL_TF_STAGE stage;
    uint32_t frame_count;
    uint32_t pnc_total_frame;
    uint32_t switch_total_frame;
    uint32_t anc_total_frame;
} voice_assist_optimal_tf_inst;

static voice_assist_optimal_tf_inst ctx;

voice_assist_optimal_tf_inst *voice_assist_get_ctx(void)
{
    return &ctx;
}

static int32_t _voice_assist_optimal_tf_anc_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_optimal_tf_anc_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_OPTIMAL_TF_ANC, _voice_assist_optimal_tf_anc_callback);

    return 0;
}

int32_t app_voice_assist_optimal_tf_anc_open(void)
{
    voice_assist_optimal_tf_inst *ctx = voice_assist_get_ctx();

    syspool_init();

    med_heap_init(syspool_start_addr(), syspool_total_size());

    ctx->anc_inst = anc_optimal_tf_create(16000, 120, 1, default_allocator());
    ctx->stage = ANC_OPTIMAL_TF_STAGE_IDLE;
    ctx->frame_count = 0;
    ctx->pnc_total_frame = 500 * 2 / 15;
    ctx->switch_total_frame = 700 * 2 / 15;
    ctx->anc_total_frame = 500 * 2 / 15;

    app_anc_assist_open(ANC_ASSIST_USER_OPTIMAL_TF_ANC);

    return 0;
}

int32_t app_voice_assist_optimal_tf_anc_close(void)
{
    voice_assist_optimal_tf_inst *ctx = voice_assist_get_ctx();

    anc_optimal_tf_get_Sz(ctx->anc_inst , Sz_estimate);

    anc_optimal_tf_destroy(ctx->anc_inst);

    app_anc_assist_close(ANC_ASSIST_USER_OPTIMAL_TF_ANC);

    return 0;
}

int32_t app_voice_assist_optimal_tf_choose_mode(void)
{
    voice_assist_optimal_tf_inst *ctx = voice_assist_get_ctx();
    int32_t best_mode = STAGE_NUM;

    float feat = anc_optimal_tf_get_TF_feature(ctx->anc_inst);

    for (int32_t i = 1; i < STAGE_NUM; i++){
        if(feat < faet_thd[i-1]){
            best_mode = i;
            break;
        }
    }

    TRACE(2, "[%s] ---------feat = %de-2 best_mode = mode %d--------", __FUNCTION__, (int)(feat * 100), best_mode);
    // app_anc_switch(best_mode);
    return 0;
}

static char *stage_desc[ANC_OPTIMAL_TF_STAGE_NUM] = {
    "IDLE",
    "PNC",
    "WAITING ANC ON",
    "ANC",
    "WAITING ANC OFF",
};

static int32_t _voice_assist_optimal_tf_anc_callback(void * buf, uint32_t len, void *other)
{
    float **input_data = buf;
    float *ff_data = input_data[0];  // error
    float *fb_data = input_data[1];  // error
    float *ref_data = input_data[2];

     static enum HAL_CODEC_ECHO_PATH_T path_def = HAL_CODEC_ECHO_PATH_QTY;

    voice_assist_optimal_tf_inst *ctx = voice_assist_get_ctx();

    anc_optimal_tf_process(ctx->anc_inst, ff_data, fb_data, ref_data, 120, ctx->stage);

    ctx->frame_count += 1;

    if (ctx->stage == ANC_OPTIMAL_TF_STAGE_IDLE) {
        ctx->stage = ANC_OPTIMAL_TF_STAGE_PNC;
        ctx->frame_count = 0;
    } else if (ctx->stage == ANC_OPTIMAL_TF_STAGE_PNC) {
        if (ctx->frame_count == ctx->pnc_total_frame) {
            app_anc_switch(APP_ANC_MODE1);
            ctx->stage = ANC_OPTIMAL_TF_STAGE_WAITING_ANC_ON;
            ctx->frame_count = 0;
        }
    } else if (ctx->stage == ANC_OPTIMAL_TF_STAGE_WAITING_ANC_ON) {
        if (ctx->frame_count == ctx->switch_total_frame) {
            path_def = hal_codec_get_echo_path();
            hal_codec_set_echo_path(HAL_CODEC_ECHO_PATH_ALL);
            ctx->stage = ANC_OPTIMAL_TF_STAGE_ANC;
            ctx->frame_count = 0;
        }
    } else if (ctx->stage == ANC_OPTIMAL_TF_STAGE_ANC) {
        if (ctx->frame_count == ctx->anc_total_frame) {
            ctx->stage = ANC_OPTIMAL_TF_STAGE_WAITING_ANC_OFF;
            app_voice_assist_optimal_tf_choose_mode();
            app_voice_assist_optimal_tf_anc_close();
            hal_codec_set_echo_path(path_def);
        }
    }

    if (ctx->frame_count == 0) {
        TRACE(2, "[%s] switch to stage %s", __FUNCTION__, stage_desc[ctx->stage]);
    }

    return 0;
}
