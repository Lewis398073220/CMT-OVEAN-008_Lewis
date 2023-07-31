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
#ifndef __APP_UI_TWS_FSM_H__
#define __APP_UI_TWS_FSM_H__

typedef enum {
    TWS_STATE_DISCONNECTED = 0,
    TWS_STATE_CONNECTING,
    TWS_STATE_CONNECTED,
} tws_fsm_states;

typedef void (*submit_msg_to_super)(app_ui_evt_t evt, app_ui_evt_t extra_evt);

typedef struct
{
    tws_fsm_states curr_state;
    app_ui_evt_t active_event;
    osTimerId nv_slave_delay_tws_timer_id;
    osTimerId tws_reconnect_timer_id;

    bool is_running;
    submit_msg_to_super submit_msg_func;

    uint8_t tws_reconnect_times;
    uint8_t tws_reconnect_max_time;
    app_ui_evt_t reconnect_event;
} app_ui_tws_sm_t;

const char* tws_sm_state_to_string(tws_fsm_states state);
void app_ui_tws_sm_init(submit_msg_to_super submit_msg_cb);
app_ui_tws_sm_t* app_ui_tws_sm_ctx();
void app_ui_tws_sm_handle_event(app_ui_evt_t evt, uint8_t reasonCode = 0);
bool app_ui_tws_need_delay_mob_reconn();
void app_ui_clear_reconnect_timer(void);
void app_ui_cancel_tws_connect();

#endif /*__APP_UI_TWS_FSM_H__ */
