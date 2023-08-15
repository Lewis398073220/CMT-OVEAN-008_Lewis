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
#if defined(IBRT)
#include <stdio.h>
#include "string.h"
#include "cmsis_os.h"
#include "app_ibrt_debug.h"
#include "bluetooth_bt_api.h"
#include "btapp.h"
#include "app_bt.h"
#include "audio_policy.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_tws_ext_cmd.h"
#include "app_tws_ibrt_audio_analysis.h"
#include "app_tws_ibrt_audio_sync.h"
#include "a2dp_decoder.h"
#include "app_audio_active_device_manager.h"

typedef struct
{
    app_tws_ext_cmd_head_t head;
    bt_bdaddr_t playing_device_addr;
} app_tws_ext_cmd_a2dp_playing_device_t;

typedef struct
{
    app_tws_ext_cmd_head_t head;
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T audio_sync_trigger;
} app_tws_ext_cmd_mobile_playback_info_t;

typedef struct
{
    app_tws_ext_cmd_head_t head;
    bt_bdaddr_t active_device;
    uint8_t device_type;
}app_tws_ext_cmd_set_active_device_t;

typedef struct
{
    app_tws_ext_cmd_head_t head;
    uint8_t buff[APP_FOCOUS_DATA_SIZE];
}app_tws_ext_cmd_focus_sync_t;

void app_ibrt_send_ext_cmd_a2dp_playing_device(uint8_t curr_playing_device, bool is_response)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(curr_playing_device);
    app_tws_ext_cmd_a2dp_playing_device_t cmd;
    if (curr_device)
    {
        cmd.playing_device_addr = curr_device->remote;
        if (is_response)
        {
            app_ibrt_tws_send_ext_cmd_rsp(APP_TWS_EXT_CMD_A2DP_PLAYING_DEVICE, &cmd.head, sizeof(app_tws_ext_cmd_a2dp_playing_device_t));
        }
        else
        {
            app_ibrt_tws_send_ext_cmd(APP_TWS_EXT_CMD_A2DP_PLAYING_DEVICE, &cmd.head, sizeof(app_tws_ext_cmd_a2dp_playing_device_t));
        }
    }
}

static void app_ibrt_ext_cmd_a2dp_playing_device_handler(bool is_response, app_tws_ext_cmd_head_t *header, uint32_t length)
{
    app_tws_ext_cmd_a2dp_playing_device_t *cmd = (app_tws_ext_cmd_a2dp_playing_device_t *)header;
    uint8_t device_id = app_bt_get_device_id_byaddr(&cmd->playing_device_addr);
    if (device_id != BT_DEVICE_INVALID_ID)
    {
        app_bt_audio_receive_peer_a2dp_playing_device(is_response, device_id);
    }
}

void app_ibrt_send_ext_cmd_mobile_playback_info(uint8_t* param, uint16_t length)
{
    LOG_I("app_ibrt_send_ext_cmd_mobile_playback_info");
    app_tws_ext_cmd_mobile_playback_info_t cmd;
    memcpy(&(cmd.audio_sync_trigger), param, length);
    app_ibrt_tws_send_ext_cmd(APP_TWS_EXT_CMD_MOBILE_PLAYBACK_INFO, &cmd.head, sizeof(app_tws_ext_cmd_head_t) + length);
}

static void app_ibrt_ext_cmd_mobile_playback_info_handler(bool is_response, app_tws_ext_cmd_head_t *header, uint32_t length)
{
    if (header && (length  >= sizeof(app_tws_ext_cmd_head_t)))
    {
        app_tws_ext_cmd_mobile_playback_info_t *cmd = (app_tws_ext_cmd_mobile_playback_info_t *)header;
        app_ibrt_send_mobile_link_playback_info_handler(cmd->head.ext_cmdseq, (uint8_t*)(&cmd->audio_sync_trigger), length-sizeof(app_tws_ext_cmd_head_t));
    }
}

typedef struct
{
    app_tws_ext_cmd_head_t head;
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T audio_sync_trigger;
} app_tws_ext_cmd_set_trigger_t;

void app_ibrt_send_ext_cmd_set_trigger(uint8_t* param, uint16_t length)
{
    LOG_I("app_ibrt_send_ext_cmd_set_trigger");
    app_tws_ext_cmd_set_trigger_t cmd;
    memcpy(&(cmd.audio_sync_trigger), param, length);
    app_ibrt_tws_send_ext_cmd(APP_TWS_EXT_CMD_SET_TRIGGER_TIME,
        &cmd.head, sizeof(app_tws_ext_cmd_head_t) + length);
}

static void app_ibrt_ext_cmd_set_trigger_handler(bool is_response, app_tws_ext_cmd_head_t *header, uint32_t length)
{
    if (header && (length  >= sizeof(app_tws_ext_cmd_head_t)))
    {
        app_tws_ext_cmd_set_trigger_t *cmd = (app_tws_ext_cmd_set_trigger_t *)header;
        app_ibrt_cmd_set_trigger_handler(cmd->head.ext_cmdseq, (uint8_t*)(&cmd->audio_sync_trigger), length-sizeof(app_tws_ext_cmd_head_t));
    }
}
void app_ibrt_send_ext_cmd_set_active_device(uint8_t device_id, uint8_t device_type)
{
    LOG_I("app_ibrt_send_ext_cmd_set_trigger");
    struct BT_DEVICE_T *curr_device = NULL;
    curr_device = app_bt_get_device(device_id);
    app_tws_ext_cmd_set_active_device_t cmd;

    if(curr_device)
    {
        cmd.active_device = curr_device->remote;
        cmd.device_type = device_type;
        app_ibrt_tws_send_ext_cmd(APP_TWS_EXT_CMD_SET_ACTIVE_DEVICE, &cmd.head, sizeof(app_tws_ext_cmd_set_active_device_t));
    }
}

static void app_ibrt_set_active_device_handler(bool is_response, app_tws_ext_cmd_head_t *header, uint32_t length)
{
    uint8_t device_id = BT_DEVICE_INVALID_ID;
    if(header && (length >= sizeof(app_tws_ext_cmd_head_t)))
    {
        app_tws_ext_cmd_set_active_device_t *cmd = (app_tws_ext_cmd_set_active_device_t*)header;
        device_id = app_bt_get_device_id_byaddr(&cmd->active_device);
        LOG_I("app_adm:d%x set as active", device_id);
        if(BT_DEVICE_INVALID_ID == device_id)
        {
            return;
        }
        app_audio_adm_slave_set_active_device_handler(device_id, cmd->device_type);//handler
    }
}

void app_ibrt_send_ext_cmd_sync_focus_info(uint8_t* param, uint16_t length)
{
    app_tws_ext_cmd_focus_sync_t cmd;
    memcpy(&(cmd.buff), param, length);
    app_ibrt_tws_send_ext_cmd(APP_TWS_EXT_CMD_SYNC_FOCUS_INFO, &cmd.head, sizeof(app_tws_ext_cmd_head_t) + length);
}

static void app_tws_ext_cmd_sync_focus_info_handler(bool is_response, app_tws_ext_cmd_head_t *header, uint32_t length)
{
    app_tws_ext_cmd_focus_sync_t *cmd = (app_tws_ext_cmd_focus_sync_t *)header;
    app_bt_audio_handle_peer_focus_info((uint8_t*)(&cmd->buff), length-sizeof(app_tws_ext_cmd_head_t));  
}

static const app_tws_ext_cmd_handler_t g_app_ibrt_tws_ext_cmd_table[] =
{
    {
        APP_TWS_EXT_CMD_A2DP_PLAYING_DEVICE,
        "A2DP_PLAYING_DEVICE",
        app_ibrt_ext_cmd_a2dp_playing_device_handler,
    },
    {
        APP_TWS_EXT_CMD_MOBILE_PLAYBACK_INFO,
        "APP_TWS_CMD_MOBILE_PLAYBACK_INFO",
        app_ibrt_ext_cmd_mobile_playback_info_handler,
    },
    {
        APP_TWS_EXT_CMD_SET_TRIGGER_TIME,
        "APP_TWS_CMD_SET_TRIGGER_TIME",
        app_ibrt_ext_cmd_set_trigger_handler,
    },
    {
        APP_TWS_EXT_CMD_SET_ACTIVE_DEVICE,
        "APP_TWS_CMD_SET_ACTIVE_DEVICE",
        app_ibrt_set_active_device_handler,
    },
    {
        APP_TWS_EXT_CMD_SYNC_FOCUS_INFO,
        "SYNC_FOCUS_INFO",
        app_tws_ext_cmd_sync_focus_info_handler,
    },
};

void app_ibrt_tws_ext_cmd_init(void)
{
    app_ibrt_register_ext_cmd_table(g_app_ibrt_tws_ext_cmd_table, sizeof(g_app_ibrt_tws_ext_cmd_table)/sizeof(app_tws_ext_cmd_handler_t));
}

#endif

