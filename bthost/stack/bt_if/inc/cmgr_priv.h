/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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

#ifndef __CONMGR_PRIV_H__
#define __CONMGR_PRIV_H__

struct conn_handler {
    list_entry_t        node;
    struct bdaddr_t     remote;
    bool                use;
    btif_cmgr_callback  callback;
    btif_sniff_info_t   sniff_info;             /*record the sniff infomation               */
    btif_handler        btHandler;
    uint8_t             sniff_timer;            /*record the timer                          */
    uint32              sniff_timeout;          /* Timeout value of the sniff timer         */
    uint32              timer_start_time_tick;  /* record the time tick that sniff timer start  */
};

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CONMGR_PRIV_H__ */
