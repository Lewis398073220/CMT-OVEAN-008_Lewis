/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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

#include "crc32_c.h"
#include "export_fn_rom.h"

unsigned long crc32_c(unsigned long crc, const unsigned char *buf, unsigned int len)
{
#ifdef CHIP_BEST1306
    return export_fn_rom->crc32(crc, buf, len);
#else
    return __export_fn_rom.crc32(crc, buf, len);
#endif
}

