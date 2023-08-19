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
#ifndef __BLE_GATT_COMMON_H__
#define __BLE_GATT_COMMON_H__
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

void bes_ble_gatt_bearer_mtu_set(uint8_t conidx, uint8_t bearer_lid, uint16_t mtu);

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_DP_COMMON_H__ */

