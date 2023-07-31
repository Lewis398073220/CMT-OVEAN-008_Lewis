/**
 * @file aob_call_stm.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2021-05-24
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

/*****************************header include********************************/
#include "app_gaf_dbg.h"
#include "acc_tb.h"
#include "app_gaf_custom_api.h"
#include "aob_call_api.h"
#include "aob_mgr_gaf_evt.h"
#include "aob_media_api.h"
#include "ble_audio_earphone_info.h"
#include "app_status_ind.h"
#include "apps.h"

/* state machine action enumeration list */
typedef enum
{
    AOB_CALL_CLI_OUTGOING = 0,
    AOB_CALL_CLI_ACTION = 1,
    AOB_CALL_CLI_JOIN = 2,
    AOB_CALL_INCOMING_IND = 3,
    AOB_CALL_SRV_OUTGOING_IND = 4,
    AOB_CALL_SRV_ALTER_IND = 5,
    AOB_CALL_SRV_ANSWER_IND = 6,
    AOB_CALL_SRV_ACCEPT_IND = 7,
    AOB_CALL_SRV_TERMINATE_IND = 8,
    AOB_CALL_SRV_HELD_LOCAL_IND = 9,
    AOB_CALL_SRV_HELD_REMOTE_IND = 10,
    AOB_CALL_SRV_RETRIEVE_LOCAL_IND = 11,
    AOB_CALL_SRV_RETRIEVE_REMOTE_IND = 12,
    AOB_CALL_SRV_JOIN_IND = 13,
    AOB_CALL_CLI_MAX_NUM_ACTIONS
} AOB_CALL_CLI_ACTION_E;

#define AOB_CALL_CLIENT_IGNORE       AOB_CALL_CLI_MAX_NUM_ACTIONS

/* type for action functions */
typedef void (*AOB_CALL_CLI_ACTION_FUNC_T)(AOB_CALL_CLI_DATA_T *p_data);

void aob_call_cli_outgoing(AOB_CALL_CLI_DATA_T *p_data)
{
    app_acc_tbc_call_outgoing(p_data->api_call_outgoing.con_lid, p_data->api_call_outgoing.bearer_id,
                        p_data->api_call_outgoing.uri, p_data->api_call_outgoing.uri_len);
}

void aob_call_cli_action(AOB_CALL_CLI_DATA_T *p_data)
{
    app_acc_tbc_call_action(p_data->api_call_action.con_lid, p_data->api_call_action.bearer_id,
                        p_data->api_call_action.call_id, p_data->api_call_action.action_opcode);
}

void aob_call_cli_join(AOB_CALL_CLI_DATA_T *p_data)
{
    app_acc_tbc_call_join(p_data->api_call_join.con_lid, p_data->api_call_join.bearer_id,
                        p_data->api_call_join.nb_calls, p_data->api_call_join.call_ids);

}

void aob_call_terminate_ind(AOB_CALL_CLI_DATA_T *p_data)
{
    LOG_I("%s con_lid:%d call_id:%d reason:%d", __func__ ,p_data->api_call_srv_terminate_ind.con_lid, \
                p_data->api_call_srv_terminate_ind.call_id, p_data->api_call_srv_terminate_ind.terminate_reason);
}

/* action functions */
const AOB_CALL_CLI_ACTION_FUNC_T aob_call_cli_state_action_tbl[] =
{
    aob_call_cli_outgoing,
    aob_call_cli_action,
    aob_call_cli_join,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    aob_call_terminate_ind,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

/**********************private function declaration*************************/
/* state table information */
#define AOB_CALL_CLI_ACTIONS              1       /* number of actions */
#define AOB_CALL_CLI_NEXT_STATE           2       /* position of next state */
#define AOB_CALL_CLI_NUM_COLS             2       /* number of columns in state tables */

/* state table for init state */
static const uint8_t aob_call_st_idle[AOB_CALL_EARPHONE_MAX_NUM_EVT][AOB_CALL_CLI_NUM_COLS] =
{
/* Trigger Event                                           Action 1  */
{AOB_CALL_CLIENT_OUTGOING_USER_EVT,                 AOB_CALL_CLI_OUTGOING },
{AOB_CALL_CLIENT_ACCEPT_USER_EVT,                   AOB_CALL_CLIENT_IGNORE},
{AOB_CALL_CLIENT_TERMINATE_USER_EVT,                AOB_CALL_CLIENT_IGNORE},
{AOB_CALL_CLIENT_HOLD_USER_EVT,                     AOB_CALL_CLIENT_IGNORE},
{AOB_CALL_CLIENT_RETRIEVE_USER_EVT,                 AOB_CALL_CLIENT_IGNORE},
{AOB_CALL_CLIENT_JOIN_USER_EVT,                     AOB_CALL_CLIENT_IGNORE},
{AOB_CALL_SERVER_TERMINATE_IND_EVT,                 AOB_CALL_CLIENT_IGNORE},
};

static const uint8_t aob_call_st_incoming[AOB_CALL_EARPHONE_MAX_NUM_EVT][AOB_CALL_CLI_NUM_COLS] =
{
/* Trigger Event                                           Action 1 */
{AOB_CALL_CLIENT_OUTGOING_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_ACCEPT_USER_EVT,                   AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_TERMINATE_USER_EVT,                AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_HOLD_USER_EVT,                     AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_RETRIEVE_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_JOIN_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_SERVER_TERMINATE_IND_EVT,                 AOB_CALL_SRV_TERMINATE_IND},
};

static const uint8_t aob_call_st_dialing[AOB_CALL_EARPHONE_MAX_NUM_EVT][AOB_CALL_CLI_NUM_COLS] =
{
/* Trigger Event                                           Action 1 */
{AOB_CALL_CLIENT_OUTGOING_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_ACCEPT_USER_EVT,                   AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_TERMINATE_USER_EVT,                AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_HOLD_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_RETRIEVE_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_JOIN_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_SERVER_TERMINATE_IND_EVT,                 AOB_CALL_SRV_TERMINATE_IND},
};

static const uint8_t aob_call_st_altering[AOB_CALL_EARPHONE_MAX_NUM_EVT][AOB_CALL_CLI_NUM_COLS] =
{
/* Trigger Event                                           Action 1 */
{AOB_CALL_CLIENT_OUTGOING_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_ACCEPT_USER_EVT,                   AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_TERMINATE_USER_EVT,                AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_HOLD_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_RETRIEVE_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_JOIN_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_SERVER_TERMINATE_IND_EVT,                 AOB_CALL_SRV_TERMINATE_IND},
};

static const uint8_t aob_call_st_active[AOB_CALL_EARPHONE_MAX_NUM_EVT][AOB_CALL_CLI_NUM_COLS] =
{
/* Trigger Event                                           Action 1 */
{AOB_CALL_CLIENT_OUTGOING_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_ACCEPT_USER_EVT,                   AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_TERMINATE_USER_EVT,                AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_HOLD_USER_EVT,                     AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_RETRIEVE_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_JOIN_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_SERVER_TERMINATE_IND_EVT,                 AOB_CALL_SRV_TERMINATE_IND},
};

static const uint8_t aob_call_st_local_held[AOB_CALL_EARPHONE_MAX_NUM_EVT][AOB_CALL_CLI_NUM_COLS] =
{
/* Trigger Event                                           Action 1 */
{AOB_CALL_CLIENT_OUTGOING_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_ACCEPT_USER_EVT,                   AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_TERMINATE_USER_EVT,                AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_HOLD_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_RETRIEVE_USER_EVT,                 AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_JOIN_USER_EVT,                     AOB_CALL_CLI_JOIN         },
{AOB_CALL_SERVER_TERMINATE_IND_EVT,                 AOB_CALL_SRV_TERMINATE_IND},
};

static const uint8_t aob_call_st_remote_held[AOB_CALL_EARPHONE_MAX_NUM_EVT][AOB_CALL_CLI_NUM_COLS] =
{
/* Trigger Event                                           Action 1 */
{AOB_CALL_CLIENT_OUTGOING_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_ACCEPT_USER_EVT,                   AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_TERMINATE_USER_EVT,                AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_HOLD_USER_EVT,                     AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_RETRIEVE_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_JOIN_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_SERVER_TERMINATE_IND_EVT,                 AOB_CALL_SRV_TERMINATE_IND},
};

static const uint8_t aob_call_st_local_remote_held[AOB_CALL_EARPHONE_MAX_NUM_EVT][AOB_CALL_CLI_NUM_COLS] =
{
/* Trigger Event                                           Action 1 */
{AOB_CALL_CLIENT_OUTGOING_USER_EVT,                 AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_ACCEPT_USER_EVT,                   AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_TERMINATE_USER_EVT,                AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_HOLD_USER_EVT,                     AOB_CALL_CLIENT_IGNORE    },
{AOB_CALL_CLIENT_RETRIEVE_USER_EVT,                 AOB_CALL_CLI_ACTION       },
{AOB_CALL_CLIENT_JOIN_USER_EVT,                     AOB_CALL_CLI_JOIN         },
{AOB_CALL_SERVER_TERMINATE_IND_EVT,                 AOB_CALL_SRV_TERMINATE_IND},
};

/* type for state table */
typedef const uint8_t (*AOB_CALL_STATE_TBL)[AOB_CALL_CLI_NUM_COLS];

/* state table */
static const AOB_CALL_STATE_TBL aob_call_stm_tbl[] =
{
    aob_call_st_incoming,
    aob_call_st_dialing,
    aob_call_st_altering,
    aob_call_st_active,
    aob_call_st_local_held,
    aob_call_st_remote_held,
    aob_call_st_local_remote_held,
    aob_call_st_idle,
};

void aob_call_stm_execute(uint8_t call_id, uint16_t event, AOB_CALL_CLI_DATA_T *p_data)
{
    AOB_SINGLE_CALL_INFO_T*     call_info = ble_audio_earphone_info_find_call_info(0, call_id);
    AOB_CALL_STATE_TBL          state_table;
    AOB_CALL_STATE_E            current_state = AOB_CALL_STATE_IDLE;
    uint8_t                     action = AOB_CALL_CLI_MAX_NUM_ACTIONS;
    uint8_t                     i = 0;
    uint8_t                     evt_index = 0;

    if ((AOB_CALL_CLIENT_OUTGOING_USER_EVT != event) && (!call_info))
    {
        LOG_W("%s entry_event:%d, call_id:%d,callinfo null", __func__, event, call_id);
        return;
    }
    if (call_info)
    {
        LOG_I("%s entry_event:%d, state:%d call_id:%d", __func__, event, call_info->stm_state, call_id);
        current_state = call_info->state;
    }
    else
    {
        LOG_I("%s entry_event:0, call_id:%d", __func__, call_id);
    }

    event &= 0x00FF;
    if (event >= (AOB_CALL_EARPHONE_MAX_NUM_EVT & 0x00FF))
    {
        LOG_W("call evt out of range, ignoring...");
        return;
    }

    /* look up the state table for the current state */
    state_table = aob_call_stm_tbl[current_state];

    /* look up the trigger event index */
    for (evt_index = 0; evt_index < AOB_CALL_EARPHONE_MAX_NUM_EVT; evt_index++)
    {
        if (state_table[evt_index][0] == event)
        {
            break;
        }
    }

    if (evt_index != AOB_CALL_EARPHONE_MAX_NUM_EVT)
    {
        for (i = 1; i <= AOB_CALL_CLI_ACTIONS; i++)
        {
            action = state_table[evt_index][i];
            if ((action != AOB_CALL_CLIENT_IGNORE) && aob_call_cli_state_action_tbl[action])
            {
                LOG_I("aob_call_stm_execute call event action");
                (*aob_call_cli_state_action_tbl[action])(p_data);
            }
            else
            {
                LOG_W("call STM mismatch");
                break;
            }
        }
    }

    LOG_I("aob_call_stm_execute evt_index:%d action_index:%d state:%d", evt_index, action, current_state);
}

void aob_call_scb_init()
{
}

