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
#include "cmsis.h"
#include "cmsis_os.h"
#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "app_rpc_api.h"
#include "rpc_speech_dsp.h"

rpc_speech_rsp_received_handler_t sco_dsp_process_done_handler = NULL;

static void sco_init_cmd_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_rpc_send_data_no_rsp(ACCEL_CORE, TASK_CMD_SCO_INIT_NO_RSP, ptr, len);
}

RPC_SPEECH_TASK_CMD_TO_ADD(TASK_CMD_SCO_INIT_NO_RSP,
                            "TASK_CMD_SCO_INIT_NO_RSP",
                            sco_init_cmd_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

static void sco_deinit_cmd_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_rpc_send_data_no_rsp(ACCEL_CORE, TASK_CMD_SCO_DEINIT_NO_RSP, ptr, len);
}

RPC_SPEECH_TASK_CMD_TO_ADD(TASK_CMD_SCO_DEINIT_NO_RSP,
                            "TASK_CMD_SCO_DEINIT_NO_RSP",
                            sco_deinit_cmd_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

static void sco_process_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "Warning %s", __func__);
}

static void sco_process_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    if (sco_dsp_process_done_handler) {
        sco_dsp_process_done_handler(ptr, len);
    }
}

void rpc_speech_rsp_received_cb_register(rpc_speech_rsp_received_handler_t handler)
{
    sco_dsp_process_done_handler = handler;
}

static void sco_capture_cmd_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_rpc_send_data_waiting_rsp(ACCEL_CORE, TASK_CMD_SCO_CAPTURE_RSP, ptr, len);
}


RPC_SPEECH_TASK_CMD_TO_ADD(TASK_CMD_SCO_CAPTURE_RSP,
                            "TASK_CMD_SCO_CAPTURE_RSP",
                            sco_capture_cmd_transmit_handler,
                            NULL,
                            RPC_CMD_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                            sco_process_cmd_wait_rsp_timeout,
                            sco_process_cmd_rsp_received_handler,
                            NULL);

static void sco_playback_cmd_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_rpc_send_data_waiting_rsp(ACCEL_CORE, TASK_CMD_SCO_PLAYBACK_RSP, ptr, len);
}


RPC_SPEECH_TASK_CMD_TO_ADD(TASK_CMD_SCO_PLAYBACK_RSP,
                            "TASK_CMD_SCO_PLAYBACK_RSP",
                            sco_playback_cmd_transmit_handler,
                            NULL,
                            RPC_CMD_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                            sco_process_cmd_wait_rsp_timeout,
                            sco_process_cmd_rsp_received_handler,
                            NULL);
