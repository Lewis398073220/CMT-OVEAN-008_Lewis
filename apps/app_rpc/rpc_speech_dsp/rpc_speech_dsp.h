/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __RPC_SPEECH_DSP_H__
#define __RPC_SPEECH_DSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SPEECH_ALGO_DSP_M55)
#include "app_dsp_m55.h"
#define ACCEL_CORE                  APP_RPC_CORE_BTH_M55
#define TASK_CMD_SCO_INIT_NO_RSP    MCU_DSP_M55_TASK_CMD_SCO_INIT_NO_RSP
#define TASK_CMD_SCO_DEINIT_NO_RSP  MCU_DSP_M55_TASK_CMD_SCO_DEINIT_NO_RSP
#define TASK_CMD_SCO_CAPTURE_RSP    MCU_DSP_M55_TASK_CMD_SCO_CAPTURE_RSP
#define TASK_CMD_SCO_PLAYBACK_RSP   MCU_DSP_M55_TASK_CMD_SCO_PLAYBACK_RSP
#define RPC_SPEECH_TASK_CMD_TO_ADD  CORE_BRIDGE_TASK_COMMAND_TO_ADD
#elif defined(SPEECH_ALGO_DSP_HIFI)
#define ACCEL_CORE                  APP_RPC_CORE_BTH_DSP
#define TASK_CMD_SCO_INIT_NO_RSP    BTH_DSP_TASK_CMD_SCO_INIT_NO_RSP
#define TASK_CMD_SCO_DEINIT_NO_RSP  BTH_DSP_TASK_CMD_SCO_DEINIT_NO_RSP
#define TASK_CMD_SCO_CAPTURE_RSP    BTH_DSP_TASK_CMD_SCO_CAPTURE_RSP
#define TASK_CMD_SCO_PLAYBACK_RSP   BTH_DSP_TASK_CMD_SCO_PLAYBACK_RSP
#define RPC_SPEECH_TASK_CMD_TO_ADD  RPC_BTH_DSP_TASK_CMD_TO_ADD
#elif defined(SPEECH_ALGO_DSP_SENS)
#include "app_sensor_hub.h"
#define ACCEL_CORE                      APP_RPC_CORE_MCU_SENSOR
#define TASK_CMD_SCO_INIT_NO_RSP        BTH_SENS_TASK_CMD_SCO_INIT_RSP
#define TASK_CMD_SCO_DEINIT_NO_RSP      BTH_SENS_TASK_CMD_SCO_DEINIT_RSP
#define TASK_CMD_SCO_CAPTURE_RSP        BTH_SENS_TASK_CMD_SCO_CAPTURE_NO_RSP
#define TASK_CMD_SCO_PLAYBACK_RSP       BTH_SENS_TASK_CMD_SCO_PLAYBACK_NO_RSP
#define RPC_SPEECH_TASK_CMD_TO_ADD      CORE_BRIDGE_TASK_COMMAND_TO_ADD
#else
#error "SPEECH_ALGO_DSP is invalid, expected M55、HIFI or SENS"
#endif

typedef void(*rpc_speech_rsp_received_handler_t)(unsigned char * ptr, short len);

void rpc_speech_rsp_received_cb_register(rpc_speech_rsp_received_handler_t handler);

#ifdef __cplusplus
}
#endif
#endif