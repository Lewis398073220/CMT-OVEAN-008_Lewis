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

#ifndef __SBC_TYPES_H__
#define __SBC_TYPES_H__

typedef char sbc_ret_status_t;

#ifndef TRUE
#define TRUE  (1==1)
#endif /* TRUE */

#ifndef FALSE
#define FALSE (0==1)
#endif /* FALSE */

#ifndef BOOL_DEFINED
typedef unsigned int BOOL;   /* IGNORESTYLE */
#endif


typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char  U8;

typedef int S32;
typedef short S16;
typedef char  S8;

#ifndef U32_PTR_DEFINED
typedef U32* U32_PTR;
#define U32_PTR_DEFINED
#endif /* U32_PTR_DEFINED */

typedef unsigned long  I32;

#ifndef XA_INTEGER_SIZE
#define XA_INTEGER_SIZE    4
#endif

#if XA_INTEGER_SIZE == 4
typedef unsigned long  I16;
typedef unsigned long  I8;
#elif XA_INTEGER_SIZE == 2
typedef unsigned short I16;
typedef unsigned short I8;
#elif XA_INTEGER_SIZE == 1
typedef unsigned short I16;
typedef unsigned char  I8;
#else
#error No XA_INTEGER_SIZE specified!
#endif

typedef void (*PFV) (void);

#define SBC_RET_STATUS_SUCCESS         0
#define SBC_RET_STATUS_FAILED          1
#define SBC_RET_STATUS_NO_RESOURCES      12
#define SBC_RET_STATUS_NOT_FOUND         13
#define SBC_RET_STATUS_INVALID_PARM      18
#define SBC_RET_STATUS_CONTINUE          24

#define SBC_ENABLED 1
#define SBC_DISABLED 0

#endif /* __SBC_TYPES__ */
