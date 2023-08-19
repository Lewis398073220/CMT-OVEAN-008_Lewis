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


#ifndef __ANC_FF_FIR_LMS_H__
#define __ANC_FF_FIR_LMS_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "stdbool.h"
#include "stdint.h"

#define H_spk_fir_len (120)
#define LOCAL_FIR_LEN (220)

typedef struct {
    int32_t max_cnt;
    int32_t period_cnt;

} ANC_FF_FIR_LMS_CFG_T;

typedef struct ANCFFFirLmsSt_ ANCFFFirLmsSt;

ANCFFFirLmsSt * anc_ff_fir_lms_create(int32_t sample_rate, int32_t frame_len, ANC_FF_FIR_LMS_CFG_T * cfg);
void anc_ff_fir_lms_destroy(ANCFFFirLmsSt* st);
int32_t anc_ff_fir_lms_reset(ANCFFFirLmsSt* st,int32_t status);

int32_t anc_ff_fir_lms_process(ANCFFFirLmsSt* st,float *ff, float *fb, float *ref, int frame_len);
int32_t *fir_lms_coeff_cache(ANCFFFirLmsSt* st);




#ifdef __cplusplus
}
#endif

#endif