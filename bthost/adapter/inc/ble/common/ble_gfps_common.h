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
#ifndef __BLE_GFPS_COMMON_H__
#define __BLE_GFPS_COMMON_H__
#include "ble_common_define.h"
#ifdef BLE_HOST_SUPPORT
#ifdef GFPS_ENABLED
#ifdef __cplusplus
extern "C" {
#endif

#define BLE_FASTPAIR_NORMAL_ADVERTISING_INTERVAL (160)
#define BLE_FASTPAIR_FAST_ADVERTISING_INTERVAL (48)

#define APP_GFPS_RANDOM_RESOLVABLE_DATA_TYPE    (0x06)

typedef void (*gfps_get_battery_info_handler)(uint8_t* batteryValueCount, uint8_t* batteryValue);

#ifdef __cplusplus
}
#endif
#endif
#endif
#endif /* __BLE_GFPS_COMMON_H__ */
