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

#ifndef __SBC_MATH_H__
#define __SBC_MATH_H__

#if (__ARM_FEATURE_MVE & 2) 
//#define KEIL_SIMULATION 1
#define VFAST 1
#endif

#if (KEIL_SIMULATION||VFAST)
#define M55HEAD
#endif
#ifdef M55HEAD
#include "RTE_Components.h"
#include CMSIS_device_header
#define asm __ASM
#endif

#include "sbc_types.h"

#ifndef INLINE
#define INLINE __inline
#endif

#define SBC_MATH_USE_FIXED_HI_RES  1

#define SBC_MATH_USE_FIXED_LO_RES  2

#define SBC_MATH_USE_FIXED_ASM     3

#define SBC_MATH_USE_FLOAT         4

#ifndef SBC_MATH_FUNCTIONS
#define SBC_MATH_FUNCTIONS  SBC_MATH_USE_FIXED_HI_RES
#endif

#if SBC_MATH_FUNCTIONS == SBC_MATH_USE_FIXED_LO_RES
/* Fixed Point enabled, Fixed Macros enabled, Asm disabled, HiRes disabled */
#define SBC_USE_HIRES_MACROS SBC_DISABLED
#elif SBC_MATH_FUNCTIONS == SBC_MATH_USE_FIXED_ASM
/* Fixed Point enabled, Fixed Macros disabled, Asm enabled, HiRes - N/A */
#define SBC_USE_FIXED_MACROS SBC_DISABLED
#elif SBC_MATH_FUNCTIONS == SBC_MATH_USE_FLOAT
/* Fixed Point disabled, Fixed Macros - N/A, Asm - N/A, HiRes - N/A */
#define SBC_USE_FIXED_POINT  SBC_DISABLED
#endif

/* Fixed-point used */
#ifndef SBC_USE_FIXED_POINT
#define SBC_USE_FIXED_POINT SBC_ENABLED
#endif

/* Fixed-point macros used */
#ifndef SBC_USE_FIXED_MACROS
#define SBC_USE_FIXED_MACROS SBC_ENABLED
#endif

/* High resolution macros used */
#ifndef SBC_USE_HIRES_MACROS
#define SBC_USE_HIRES_MACROS SBC_ENABLED
#endif

#if SBC_USE_FIXED_POINT == SBC_ENABLED
typedef S32 REAL;
#else
typedef float REAL;
#endif

typedef signed char INT8;

REAL S16toReal(S16 Integer);
S16 RealtoS16(REAL Real);
U16 RealtoU16(REAL Integer);
S32 RealtoS32(REAL Real);
REAL Mul(REAL x, REAL y);
REAL MulP(REAL x, REAL y);
REAL MulPI(REAL x, REAL y);
REAL dMulP(REAL x, REAL y);

#if SBC_USE_FIXED_POINT == SBC_ENABLED

/****************************************************************************
 *
 * Fixed point macros
 *
 ****************************************************************************/

/* Some useful contants */
#define ONE_F       (0x00008000)
#define ONE_F_P     (ONE_F << 15) /* For higher precision calculation */

/* Signed 16 to REAL */
#define S16toReal(x) ((REAL)(((S32)(x)) << 15))

/* Unsigned 16 to REAL */
#define U16toReal(x) ((REAL)(((U32)(x)) << 15))

/* Clip a positive signed number */
#define ClipPos(x) (((x) > 0x3FFF8000) ? 0x3FFF8000 : (x))

/* Clip a negative signed number */
#define ClipNeg(x) (((x) < 0xC0000000) ? 0xC0000000 : (x))

/* Clip a value to largest or smallest 16 bit signed value */
#define Clip(x) (((x) < 0) ? ClipNeg(x) : ClipPos(x))

/* Clip a positive signed number (decoder only) */
#define ClipPosD(x) (((x) > 0x1FFFC000) ? 0x1FFFC000: (x))
 
/* Clip a negative signed number (decoder only) */
#define ClipNegD(x) (((x) < 0xE0000000) ? 0xE0000000 : (x))
 
/* Clip a value to largest or smallest 16 bit signed value (decoder only) */
#define ClipD(x) (((x) < 0) ? ClipNegD(x) : ClipPosD(x))
 
/* REAL to signed 16 bit value */
//#define RealtoS16(x) ((S16)((REAL)(ClipD(x)) >> 14))


#define RealtoS16(x) SATURATE32((x>>14),16)

#if defined(__GNUC__) && defined(__arm__)
static inline S32 SATURATE32(S32 ARG1, S32 ARG2)
{
  S32 __RES, __ARG1 = (ARG1); 
  ARG2=16;
  asm volatile ("ssat %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) ); 
  return(__RES);
}
#else
static inline S32 SATURATE32(S32 ARG1, S32 ARG2)
{
    S32 smax = (1 << (ARG2 - 1)) - 1;
    S32 smin = -1 * (1 << (ARG2 - 1));

    return (ARG1 > smax ? smax : ARG1 < smin ? smin : ARG1);
}
#endif


/* REAL to unsigned 16 bit value */
#define RealtoU16(x) ((U16)((REAL)(Clip(x)) >> 15))

/* REAL to signed 32-bit value */
#define RealtoS32(x) ((S32)((REAL)(x) >> 15))

#if SBC_USE_FIXED_MACROS == SBC_ENABLED

#ifndef Mul
/* Macros for multiplying 17:15 by 17:15 (high word of b == 0) */
#define MulHiLo(a,b) ((((a) >> 16) * ((b) & 0x0000FFFF)) << 1)
#ifndef KEIL_SIMULATION
#define MulLoLo(a,b) ((((a) & 0x0000FFFF) * ((b) & 0x0000FFFF)) >> 15)
/* Multiply a REAL by a REAL */
#define Mul(x,y) (REAL)(MulHiLo(x,y) + MulLoLo(x,y))
#endif
#endif

#ifndef MulP
/* Macros for multiplying 17:15 by 2:30 */
#if SBC_USE_HIRES_MACROS == SBC_ENABLED
#define MulPHiHi(a,b) ((((a) >> 15) * ((b) >> 15)))
#define MulPHiLo(a,b) ((((a) >> 15) * ((b) & 0x00007FFF)) >> 15)
#define MulPLoHi(a,b) ((((a) & 0x00007FFF) * ((b) >> 15)) >> 15)
/* Multiply a REAL by a REAL with high precision */
#define MulP(x,y) (REAL)(MulPHiHi(x,y) + MulPHiLo(x,y) + MulPLoHi(x,y))
#else
/* Multiply a REAL by a REAL with low precision */
#define MulP(x,y) (REAL)((((x) >> 13) * ((y) >> 17)))
#endif
#endif

#ifndef MulPI
/* Macros that assume the first parameter is an integer */
#if SBC_USE_HIRES_MACROS == SBC_ENABLED
#define MulPIHiHi(a,b) (((a) * ((b) >> 15)))
#define MulPIHiLo(a,b) (((a) * ((b) & 0x00007FFF)) >> 15)
/* Multiply an S16 by a REAL with high precision */
#define MulPI(x,y) (REAL)(MulPIHiHi(x,y) + MulPIHiLo(x,y))
#else
/* Multiply an S16 by a REAL with low precision */
#define MulPI(x,y) (REAL)(((x) * ((y) >> 12)) >> 3)
#endif
#endif
#ifndef KEIL_SIMULATION
#ifndef dMulP
/* Decoder only macros for multiplying 17:15 by 2:30 */
#if SBC_USE_HIRES_MACROS == SBC_ENABLED
/* Multiply a REAL by a REAL with high precision */
#define dMulP(x,y) (REAL)((((x) >> 13) * (y)))
#else
/* Multiply a REAL by a REAL with low precision */
#define dMulP(x,y) (REAL)((((x) >> 13) * (y)))
#endif
#endif
#endif
#endif /* SBC_USE_FIXED_MACROS == SBC_ENABLED */

#else /* SBC_USE_FIXED_POINT == SBC_ENABLED */

/****************************************************************************
 *
 * Floating point macros
 *
 ****************************************************************************/

/* Some useful contants */
#define ONE_F      (REAL)(1.0)
#define ONE_F_P    (REAL)(1.0)

/* Signed 16 to REAL */
#define S16toReal(x) ((REAL)((S32)(x)))

/* Clip a positive signed number (decoder only) */
#define ClipPos(x) (((x) > 32767) ? 32767: (x))
 
/* Clip a negative signed number (decoder only) */
#define ClipNeg(x) (((x) < -32767) ? -32767 : (x))
 
/* Clip a value to largest or smallest 16 bit signed value (decoder only) */
#define Clip(x) (((x) < 0) ? ClipNeg(x) : ClipPos(x))
 
/* REAL to signed 16 bit value */
#define RealtoS16(x) ((S16)((REAL)(Clip(x*2))))

/* REAL to unsigned 16 bit value */
#define RealtoU16(x) ((U16)(Clip(x)))

/* REAL to signed 32-bit value */
#define RealtoS32(x) ((S32)(x))

/* Multiply a REAL by a REAL  */
#define Mul(x,y) ((REAL)(x)*(REAL)(y))

/* Multiply a REAL by a REAL with high precision */
#define MulP(x,y) ((REAL)(x)*(REAL)(y))

/* Multiply a signed integer by a REAL with high precision */
#define MulPI(x,y) ((REAL)(x)*(REAL)(y))

/* Multiply a REAL by a REAL with high precision (decoder only) */
#define dMulP(x,y) ((REAL)(x)*(REAL)(y))

#endif /* SBC_USE_FIXED_POINT == SBC_ENABLED */

#endif /* __SBC_MATH_H__ */