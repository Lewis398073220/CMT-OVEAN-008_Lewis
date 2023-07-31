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
#ifndef __BLE_ACC_COMMON_H__
#define __BLE_ACC_COMMON_H__
#include "ble_common_define.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define APP_ACC_DFT_TECHNO_VAL ACC_TB_TECHNO_4G
#define APP_ACC_DFT_SIGN_STRENGTH_VAL (100)
#define APP_ACC_DFT_PROV_NAME "CHINA MOBILE"
#define APP_ACC_DFT_URI_SCHEMES_LIST "tel,sip,skype"
#define APP_ACC_DFT_CALL_INCOMING_URI "tel:10086"
#define APP_ACC_DFT_CALL_INCOMING_TGT_URI "tel:10086"
#define APP_ACC_DFT_CALL_INCOMING_FRIENDLY_NAME "INCOMING CALL"
#define APP_ACC_DFT_CALL_OUTGOING_URI "tel:10086"
#define APP_ACC_DFT_CALL_OUTGOING_FRIENDLY_NAME "OUTGOING CALL"

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_ACC_COMMON_H__ */
