/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

/*****************************header include********************************/
#include "string.h"
#include "co_math.h" // Common Maths Definition
#include "cmsis_os.h"
#include "ble_app_dbg.h"
#include "stdbool.h"
#include "bluetooth_bt_api.h"
#include "app_thread.h"
#include "app_utils.h"
#include "apps.h"
#include "gapm_msg.h"               // GAP Manager Task API
#include "gapc_msg.h"               // GAP Controller Task API
#include "app.h"
#include "app_sec.h"
#include "app_ble_include.h"
#include "nvrecord_bt.h"
#include "app_bt_func.h"
#include "hal_timer.h"
#include "app_bt.h"
#include "app_hfp.h"
#include "rwprf_config.h"
#include "nvrecord_ble.h"
#include "app_sec.h"

#ifdef IBRT
#if defined(IBRT_UI_V1)
#include "app_ibrt_ui.h"
#endif
#include "app_ibrt_if.h"
#endif

#if (BLE_AUDIO_ENABLED)
extern bool ble_audio_is_ux_mobile(void);
#endif

static void app_ble_customif_connect_event_handler(ble_event_t *event, void *output)
{
#ifdef TWS_SYSTEM_ENABLED
    app_ibrt_middleware_ble_connected_handler();
#endif

#if defined(IBRT) && defined(IBRT_UI_V1)
    bt_bdaddr_t *box_ble_addr = (bt_bdaddr_t *)app_ibrt_ui_get_box_ble_addr();
    if (app_ibrt_ui_get_snoop_via_ble_enable())
    {
        if(!memcmp(box_ble_addr, event->p.connect_handled.peer_bdaddr.addr, BTIF_BD_ADDR_SIZE))
        {
            app_ibrt_ui_set_ble_connect_index(event->p.connect_handled.conidx);
            app_ibrt_ui_set_box_connect_state(IBRT_BOX_CONNECT_MASTER, FALSE);
        }
    }
#endif
}

static void app_ble_customif_connect_bond_event_handler(ble_event_t *event, void *output)
{
    // Indicate that ble connection pairing complete
}

static void app_ble_customif_connect_nc_exch_event_handler(ble_event_t *event, void *output)
{
    // Indicate that ble connection numeric comparison - Exchange of Numeric Value
}

static void app_ble_customif_connect_encrypt_event_handler(ble_event_t *event, void *output)
{
    // Indicate that ble connection encrypt complete
}

static void app_ble_customif_stopped_connecting_event_handler(ble_event_t *event, void *output)
{
}

static void app_ble_customif_connecting_failed_event_handler(ble_event_t *event, void *output)
{
    // Indicate that ble start connecting failed. Mostly it's param invalid.
}

static void app_ble_customif_disconnect_event_handler(ble_event_t *event, void *output)
{
#if defined(IBRT) && defined(IBRT_UI_V1)
    if (app_ibrt_ui_get_snoop_via_ble_enable())
    {
        app_ibrt_ui_set_master_notify_flag(false);
        app_ibrt_ui_clear_box_connect_state(IBRT_BOX_CONNECT_MASTER, FALSE);
    }
#endif
}

static void app_ble_customif_conn_param_update_req_event_handler(ble_event_t *event, void *output)
{
}

static void app_ble_customif_conn_param_update_failed_event_handler(ble_event_t *event, void *output)
{
    // Indicate that ble connection param update failed
}

static void app_ble_customif_conn_param_update_successful_event_handler(ble_event_t *event, void *output)
{
    // Indicate that ble connection param update successful
}

static void app_ble_customif_set_random_bd_addr_event_handler(ble_event_t *event, void *output)
{
    // Indicate that a new random BD address set in lower layers
}

static void app_ble_customif_adv_started_event_handler(ble_event_t *event, void *output)
{
    // Indicate that adv has been started success
}

static void app_ble_customif_adv_starting_failed_event_handler(ble_event_t *event, void *output)
{
    // Indicate that adv starting failed
}

static void app_ble_customif_adv_stopped_event_handler(ble_event_t *event, void *output)
{
    // Indicate that adv has been stopped success
}

static void app_ble_customif_scan_started_event_handler(ble_event_t *event, void *output)
{
    // Indicate that scan has been started success
}

static void ble_adv_data_parse(ble_bdaddr_t *bleBdAddr,
                               int8_t rssi,
                               unsigned char *adv_buf,
                               unsigned char len)
{
#if defined(IBRT) && defined(IBRT_UI_V1)
    bt_bdaddr_t *box_ble_addr = (bt_bdaddr_t *)app_ibrt_ui_get_box_ble_addr();
    LOG_I("%s", __func__);
    //DUMP8("%02x ", (uint8_t *)box_ble_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", bleBdAddr, BT_ADDR_OUTPUT_PRINT_NUM);

    if (app_ibrt_ui_get_snoop_via_ble_enable())
    {
        if (!memcmp(box_ble_addr, bleBdAddr, BTIF_BD_ADDR_SIZE) && app_ibrt_ui_is_slave_scaning())
        {
            app_ibrt_ui_set_slave_scaning(FALSE);
            app_ble_stop_scan();
            app_ble_start_connect(bleBdAddr, APP_GAPM_STATIC_ADDR);
        }
    }
#endif
}

static void app_ble_customif_scan_data_report_event_handler(ble_event_t *event, void *output)
{
    // Scan data report
    ble_adv_data_parse((ble_bdaddr_t *)&event->p.scan_data_report_handled.trans_addr,
                           (int8_t)event->p.scan_data_report_handled.rssi,
                           event->p.scan_data_report_handled.data,
                           (unsigned char)event->p.scan_data_report_handled.length);
}

static void app_ble_customif_scan_starting_failed_event_handler(ble_event_t *event, void *output)
{
    // Indicate that scan starting failed
}

static void app_ble_customif_scan_stopped_event_handler(ble_event_t *event, void *output)
{
    // Indicate that scan has been stopped success
}

static void app_ble_customif_credit_based_conn_req_event_handler(ble_event_t *event, void *output)
{

}

static void app_ble_customif_encrypt_ltk_report_event_handler(ble_event_t *event, void *output)
{
    le_conn_encrypt_ltk_handled_t* le_ltk_info_cb = (le_conn_encrypt_ltk_handled_t*)&(event->p);
    if (le_ltk_info_cb)
        LOG_I("[%s] con_lid=%d ltk_found=%d", __func__, le_ltk_info_cb->conidx, le_ltk_info_cb->ltk_existed);
}

static const ble_event_handler_t app_ble_customif_event_handler_tab[] =
{
    {BLE_LINK_CONNECTED_EVENT, app_ble_customif_connect_event_handler},
    {BLE_CONNECT_BOND_EVENT, app_ble_customif_connect_bond_event_handler},
    {BLE_CONNECT_NC_EXCH_EVENT, app_ble_customif_connect_nc_exch_event_handler},
    {BLE_CONNECT_ENCRYPT_EVENT, app_ble_customif_connect_encrypt_event_handler},
    {BLE_CONNECTING_STOPPED_EVENT, app_ble_customif_stopped_connecting_event_handler},
    {BLE_CONNECTING_FAILED_EVENT, app_ble_customif_connecting_failed_event_handler},
    {BLE_DISCONNECT_EVENT, app_ble_customif_disconnect_event_handler},
    {BLE_CONN_PARAM_UPDATE_REQ_EVENT, app_ble_customif_conn_param_update_req_event_handler},
    {BLE_CONN_PARAM_UPDATE_FAILED_EVENT, app_ble_customif_conn_param_update_failed_event_handler},
    {BLE_CONN_PARAM_UPDATE_SUCCESSFUL_EVENT, app_ble_customif_conn_param_update_successful_event_handler},
    {BLE_SET_RANDOM_BD_ADDR_EVENT, app_ble_customif_set_random_bd_addr_event_handler},
    {BLE_ADV_STARTED_EVENT, app_ble_customif_adv_started_event_handler},
    {BLE_ADV_STARTING_FAILED_EVENT, app_ble_customif_adv_starting_failed_event_handler},
    {BLE_ADV_STOPPED_EVENT, app_ble_customif_adv_stopped_event_handler},
    {BLE_SCAN_STARTED_EVENT, app_ble_customif_scan_started_event_handler},
    {BLE_SCAN_DATA_REPORT_EVENT, app_ble_customif_scan_data_report_event_handler},
    {BLE_SCAN_STARTING_FAILED_EVENT, app_ble_customif_scan_starting_failed_event_handler},
    {BLE_SCAN_STOPPED_EVENT, app_ble_customif_scan_stopped_event_handler},
    {BLE_CREDIT_BASED_CONN_REQ_EVENT, app_ble_customif_credit_based_conn_req_event_handler},
    {BLE_ENCRYPT_LTK_REPORT_EVENT, app_ble_customif_encrypt_ltk_report_event_handler}
};

//handle the event that from ble lower layers
void app_ble_customif_global_handler_ind(ble_event_t *event, void *output)
{
    uint8_t evt_type = event->evt_type;
    uint16_t index = 0;
    const ble_event_handler_t *p_ble_event_hand = NULL;

    for (index = 0; index < ARRAY_SIZE(app_ble_customif_event_handler_tab); index++)
    {
        p_ble_event_hand = &app_ble_customif_event_handler_tab[index];
        if (p_ble_event_hand->evt_type == evt_type)
        {
            p_ble_event_hand->func(event, output);
            break;
        }
    }
}

void app_ble_customif_init(void)
{
#ifdef IS_BLE_CUSTOM_IF_ENABLED
    LOG_I("%s", __func__);
    app_ble_core_register_global_handler_ind(app_ble_customif_global_handler_ind);
#endif
}

