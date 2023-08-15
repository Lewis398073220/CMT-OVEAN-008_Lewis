#ifndef _BECO_DSPFUNCTIONS_H_
#define _BECO_DSPFUNCTIONS_H_



#ifdef __cplusplus
extern "C" {
#endif


#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"
beco_state beco_fir_q15(const q15_t *pSrc,
                        const uint16_t pSrcLen,
                        const q15_t *pCoeffs,
                        const uint16_t numTaps,
                        const uint16_t out_shift,
                        q15_t *pDst,
                        const uint16_t pDstLen);

#ifdef __cplusplus
}
#endif
#endif