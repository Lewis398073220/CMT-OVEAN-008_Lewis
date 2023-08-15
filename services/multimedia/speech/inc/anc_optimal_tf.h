#ifndef ANC_OPTIMAL_TF_H
#define ANC_OPTIMAL_TF_H

#include <stdint.h>
#include "custom_allocator.h"

typedef enum
{
    ANC_OPTIMAL_TF_STAGE_IDLE = 0,
    ANC_OPTIMAL_TF_STAGE_PNC,
    ANC_OPTIMAL_TF_STAGE_WAITING_ANC_ON,
    ANC_OPTIMAL_TF_STAGE_ANC,
    ANC_OPTIMAL_TF_STAGE_WAITING_ANC_OFF,
    ANC_OPTIMAL_TF_STAGE_NUM,
} ANC_OPTIMAL_TF_STAGE;

struct anc_optimal_tf_inst_;
typedef struct anc_optimal_tf_inst_ anc_optimal_tf_inst;

#ifdef __cplusplus
extern "C" {
#endif

anc_optimal_tf_inst* anc_optimal_tf_create(int32_t sample_rate, int32_t frame_size, int32_t blocks, custom_allocator* allocator);

void anc_optimal_tf_destroy(anc_optimal_tf_inst* inst);

int32_t anc_optimal_tf_process(anc_optimal_tf_inst* inst, float* ff_buf, float* fb_buf, float* ref_buf, uint32_t pcm_len, ANC_OPTIMAL_TF_STAGE stage);

void anc_optimal_tf_get_Pz(anc_optimal_tf_inst* inst, float* Pz);

void anc_optimal_tf_get_Sz(anc_optimal_tf_inst* inst, float* Sz);

float* anc_optimal_tf_get_TF(anc_optimal_tf_inst* inst);

float anc_optimal_tf_get_TF_feature(anc_optimal_tf_inst* inst);

#ifdef __cplusplus
}
#endif

#endif
