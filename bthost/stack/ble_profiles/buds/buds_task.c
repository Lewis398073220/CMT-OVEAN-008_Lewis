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
 
/**
 ****************************************************************************************
 * @addtogroup BUDSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "gapc_bt_msg.h"
#include "gapc_le_msg.h"

#if (BLE_BUDS)
#include "gap.h"
#include "buds.h"
#include "buds_task.h"

#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    buds_env_t *buds_env = PRF_ENV_GET(BUDS, buds);
    uint8_t conidx = param->conidx;
    buds_env->ntfIndEnableFlag[conidx] = 0;

    TRACE(1, "buds profile disconnected.");
    return KE_MSG_CONSUMED;
}

static int gapc_bt_connection_req_ind_handler(ke_msg_id_t const msgid,
                                           struct gapc_bt_connection_req_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    TRACE(1, "buds profile connected.");
    return KE_MSG_CONSUMED;
}

static int gapc_le_connection_req_ind_handler(ke_msg_id_t const msgid,
                                          struct gapc_le_connection_req_ind const *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
   TRACE(1, "buds profile connected.");
   return KE_MSG_CONSUMED;
}


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/* Default State handlers definition. */
KE_MSG_HANDLER_TAB(buds)
{
    {GAPC_BT_CONNECTION_REQ_IND,   (ke_msg_func_t)gapc_bt_connection_req_ind_handler},
    {GAPC_LE_CONNECTION_REQ_IND,   (ke_msg_func_t)gapc_le_connection_req_ind_handler},
    {GAPC_DISCONNECT_IND,       (ke_msg_func_t)gapc_disconnect_ind_handler},
};

void buds_task_init(struct ke_task_desc *task_desc, void *p_env)
{
    // Get the address of the environment
    buds_env_t *buds_env = (buds_env_t *)p_env;

    task_desc->msg_handler_tab = buds_msg_handler_tab;
    task_desc->msg_cnt         = ARRAY_LEN(buds_msg_handler_tab);
    task_desc->state           = &(buds_env->state);
    task_desc->idx_max         = BLE_CONNECTION_MAX;
}

#endif /* #if (BLE_BUDS) */

/// @} BUDSTASK
