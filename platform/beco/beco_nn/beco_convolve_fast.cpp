/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"

#include "beco_convolve_fast.hpp"
#include "beco_nnfunctions.h"


beco_state beco_convolve_q7_fast(const q7_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const q7_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const q7_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    q7_t  *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y,
                                    const uint16_t d_coeff)
{

    beco_convolve_fast(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in, wt, ch_im_out,
                        dim_kernel_x, dim_kernel_y, padding_x, padding_y, stride_x,
                        stride_y, bias, bias_shift, out_shift, Im_out, dim_im_out_x,
                        dim_im_out_y, d_coeff);
    return BECO_OK;

}

beco_state beco_convolve_q7_out_q15_fast(const q7_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const q7_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const q7_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    q15_t  *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y,
                                    const uint16_t d_coeff)
{

    beco_convolve_fast(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in, wt, ch_im_out,
                        dim_kernel_x, dim_kernel_y, padding_x, padding_y, stride_x,
                        stride_y, bias, bias_shift, out_shift, Im_out, dim_im_out_x,
                        dim_im_out_y, d_coeff);

    return BECO_OK;
}


beco_state beco_convolve_q15_fast(const q15_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const q15_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const q15_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    q15_t  *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y,
                                    const uint16_t d_coeff)
{

    beco_convolve_fast(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in, wt, ch_im_out,
                        dim_kernel_x, dim_kernel_y, padding_x, padding_y, stride_x,
                        stride_y, bias, bias_shift, out_shift, Im_out, dim_im_out_x,
                        dim_im_out_y, d_coeff);
    return BECO_OK;
}

beco_state beco_convolve_q15_out_q31_fast(const q15_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const q15_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const q15_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    q31_t  *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y,
                                    const uint16_t d_coeff)
{

    beco_convolve_fast(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in, wt, ch_im_out,
                        dim_kernel_x, dim_kernel_y, padding_x, padding_y, stride_x,
                        stride_y, bias, bias_shift, out_shift, Im_out, dim_im_out_x,
                        dim_im_out_y, d_coeff);
    return BECO_OK;
}