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
#include "cmsis_os.h"
#include <string.h>
#include "hal_trace.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "nvrecord_bt.h"
#include "nvrecord_ble.h"
#include "export_fn_rom.h"
#include "app_ibrt_if.h"
#include "app_ibrt_if_internal.h"
#include "a2dp_decoder.h"
#include "app_ibrt_nvrecord.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "bluetooth_bt_api.h"
#include "btapp.h"
#include "app_tws_ibrt_cmd_sync_a2dp_status.h"
#include "app_ibrt_a2dp.h"
#include "app_tws_ibrt_cmd_sync_hfp_status.h"
#include "app_ibrt_hf.h"
#include "app_audio_control.h"
#include "app_bt_stream.h"
#include "app_tws_ctrl_thread.h"
#include "app_bt_media_manager.h"
#include "app_bt_func.h"
#include "app_bt.h"
#include "app_a2dp.h"
#include "app_tws_ibrt_audio_analysis.h"
#include "app_ibrt_keyboard.h"
#include "factory_section.h"
#include "apps.h"
#include "nvrecord_env.h"
#include "hal_bootmode.h"
#include "ddbif.h"
#include "audio_policy.h"
#include "app_ibrt_customif_cmd.h"
#include "app_tws_ibrt_conn.h"
#include "app_ibrt_debug.h"


#if defined(BT_HID_DEVICE)
#include "app_bt_hid.h"
#endif

#if defined(__BT_SYNC__)
#include "app_bt_sync.h"
#endif

#ifdef __GMA_VOICE__
#include "gma_crypto.h"
#endif
#include "app_battery.h"
#include "app_hfp.h"
#if defined(BISTO_ENABLED) || defined(__AI_VOICE__)
#include "app_ai_if.h"
#include "app_ai_tws.h"
#endif
#ifdef IBRT_UI_V2
#include "app_ui_evt.h"
#include "app_ui_api.h"
#endif

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#ifdef BES_OTA
#include "ota_control.h"
extern void ota_control_send_start_role_switch(void);
#endif

#ifdef BISTO_ENABLED
#include "app_ai_tws.h"
#endif

#if BLE_AUDIO_ENABLED
#include "ble_audio_core_api.h"
#endif

#if defined(IBRT)

#include "app_tws_besaud.h"
#include "app_custom_api.h"
#if defined(IBRT_UI_V2)
#include "app_ibrt_customif_ui.h"
#endif

#ifdef BLE_AUDIO_DOLPHIN_COMPATIBLE_SUPPORT
#include "nvrecord_ble.h"
#endif

#ifdef GFPS_ENABLED
#include "gfps.h"
#endif

typedef struct
{
    ibrt_config_t ibrt_config;
    nvrec_btdevicerecord rec_mobile;
    nvrec_btdevicerecord rec_peer;
    uint8_t reserved __attribute__((aligned(4)));
    uint32_t crc;
} ibrt_config_ram_bak_t;

ibrt_config_ram_bak_t REBOOT_CUSTOM_PARAM_LOC ibrt_config_ram_bak;
void app_ibrt_ui_start_perform_a2dp_switching(void);

#ifdef IBRT_UI_V2

ibrt_link_status_changed_cb_t* client_cb = NULL;
void app_ibrt_if_register_client_callback(APP_IBRT_IF_LINK_STATUS_CHANGED_CALLBACK* cbs)
{
    client_cb = cbs;
}

void app_ibrt_if_register_custom_ui_callback(APP_IBRT_IF_LINK_STATUS_CHANGED_CALLBACK* cbs)
{
    app_ui_register_link_status_callback(cbs);
}

void app_ibrt_if_register_vender_handler_ind(APP_IBRT_IF_VENDER_EVENT_HANDLER_IND handler)
{
    app_ui_register_vender_event_update_ind(handler);
}

uint8_t app_ibrt_if_get_connected_remote_dev_count()
{
    return app_ui_get_connected_remote_dev_count();
}

void app_ibrt_if_disconnet_moblie_device(const bt_bdaddr_t* remote_addr)
{
    app_ui_send_mobile_disconnect_event(remote_addr);
}

void app_ibrt_if_reconnect_moblie_device(const bt_bdaddr_t* addr)
{
    app_ui_send_mobile_reconnect_event_by_addr(addr);
}

void app_ibrt_if_event_entry(app_ui_evt_t event)
{
    app_ui_event_entry(event);
}
#endif

static void app_ibrt_pre_pairing_disconnecting_check_timer_handler(void const *param);
osTimerDef(app_ibrt_pre_pairing_disconnecting_check_timer,
    app_ibrt_pre_pairing_disconnecting_check_timer_handler);
static osTimerId app_ibrt_pre_pairing_disconnecting_check_timer_id = NULL;
static uint32_t app_ibrt_pre_pairing_disconnecting_check_counter = 0;

#define APP_IBRT_PRE_PAIRING_DISCONNECTING_STATUS_CHECK_PERIOD_MS   100
#define APP_IBRT_PRE_PAIRING_DISCONNECTING_TIMEOUT_MS               10000
#define APP_IBRT_PRE_PAIRING_DISCONNECTING_TIMEMOUT_CHECK_COUNT    \
    ((APP_IBRT_PRE_PAIRING_DISCONNECTING_TIMEOUT_MS)/(APP_IBRT_PRE_PAIRING_DISCONNECTING_STATUS_CHECK_PERIOD_MS))

static void app_ibrt_pre_pairing_disconnecting_check_timer_handler(void const *param)
{
    if (app_ibrt_pre_pairing_disconnecting_check_counter >=
        APP_IBRT_PRE_PAIRING_DISCONNECTING_TIMEMOUT_CHECK_COUNT)
    {
        osTimerStop(app_ibrt_pre_pairing_disconnecting_check_timer_id);
        LOG_I("Pre pairing disconnecting timeout!");
        // TODO: put the registered callback API here
        return;
    }

    if (app_bt_is_any_connection())
    {
        app_ibrt_pre_pairing_disconnecting_check_counter++;
    }
    else
    {
        osTimerStop(app_ibrt_pre_pairing_disconnecting_check_timer_id);
        #ifdef IBRT_UI_V2
        app_ibrt_if_event_entry(APP_UI_EV_TWS_PAIRING);
        #endif
    }
}

static void app_ibrt_if_start_pre_pairing_disconnecting(void)
{
    if (NULL == app_ibrt_pre_pairing_disconnecting_check_timer_id)
    {
        app_ibrt_pre_pairing_disconnecting_check_timer_id =
            osTimerCreate(osTimer(app_ibrt_pre_pairing_disconnecting_check_timer),
                osTimerPeriodic, NULL);
    }

    app_ibrt_pre_pairing_disconnecting_check_counter = 0;

    osTimerStart(app_ibrt_pre_pairing_disconnecting_check_timer_id,
        APP_IBRT_PRE_PAIRING_DISCONNECTING_STATUS_CHECK_PERIOD_MS);

    app_bt_start_custom_function_in_bt_thread(0, 0,
        (uint32_t)app_disconnect_all_bt_connections);
}

static void app_ibrt_if_tws_free_pairing_entry(ibrt_role_e role, uint8_t* pMasterAddr, uint8_t *pSlaveAddr)
{
#ifdef IBRT_RIGHT_MASTER
    app_tws_ibrt_reconfig_role(role, pMasterAddr, pSlaveAddr, true);
#else
    app_tws_ibrt_reconfig_role(role, pMasterAddr, pSlaveAddr, false);
#endif

    app_tws_ibrt_use_the_same_bd_addr();
    if (app_tws_ibrt_is_connected_with_wrong_peer())
    {
        app_ibrt_if_start_pre_pairing_disconnecting();
    }
    else
    {
        #ifdef IBRT_UI_V2
        app_ibrt_if_event_entry(APP_UI_EV_TWS_PAIRING);
        #endif
    }
}

void app_ibrt_if_big_little_switch(uint8_t *in, uint8_t *out, uint8_t len)
{
    for (int i = 0; i < len; i++)
    {
        out[i] = in[len - i - 1];
    }
}

void app_ibrt_if_start_tws_pairing(ibrt_role_e role, uint8_t* peerAddr)
{
    LOG_I("start tws pairing as role %d, peer addr:", role);
    DUMP8("%02x ", peerAddr, BT_ADDR_OUTPUT_PRINT_NUM);

    bt_bdaddr_t local_addr;
    factory_section_original_btaddr_get(local_addr.address);
    LOG_I("   jay [ %s ] ", __func__);
    if (IBRT_MASTER == role)
    {
        app_ibrt_if_tws_free_pairing_entry(role, local_addr.address,
            peerAddr);
    }
    else if (IBRT_SLAVE == role)
    {
        app_ibrt_if_tws_free_pairing_entry(role, peerAddr,
            local_addr.address);
    }
    else
    {
        LOG_I("API %s doesn't accept unknown tws role", __FUNCTION__);
    }
}

void app_ibrt_if_update_tws_pairing_info(ibrt_role_e role, uint8_t* peerAddr)
{
    bt_bdaddr_t tmpBtAddr;
    app_ibrt_if_big_little_switch(peerAddr, tmpBtAddr.address, 6);
    peerAddr = tmpBtAddr.address;

    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    memset((uint8_t *)&(nvrecord_env->ibrt_mode), 0xFF, sizeof(nvrecord_env->ibrt_mode));
    nv_record_env_set(nvrecord_env);

    bt_bdaddr_t local_addr;
    factory_section_original_btaddr_get(local_addr.address);
    LOG_I("   jay [ %s ] ", __func__);
    bool isRightMasterSidePolicy = true;
#ifdef IBRT_RIGHT_MASTER
    isRightMasterSidePolicy = true;
#else
    isRightMasterSidePolicy = false;
#endif

    if (IBRT_MASTER == role)
    {
        app_tws_ibrt_reconfig_role(role, local_addr.address,
            peerAddr, isRightMasterSidePolicy);
    }
    else if (IBRT_SLAVE == role)
    {
        app_tws_ibrt_reconfig_role(role, peerAddr,
            local_addr.address, isRightMasterSidePolicy);
    }
    else
    {
        LOG_I("API %s doesn't accept unknown tws role", __FUNCTION__);
    }

    nv_record_flash_flush();

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_CUSTOM_OP2_AFTER_REBOOT);
}
#ifdef IBRT_UI_V2
void app_ibrt_if_config(app_ui_config_t *ui_config)
{

    app_ui_reconfig_env(ui_config);

}
#endif
void app_ibrt_if_nvrecord_config_load(void *config)
{
    app_ibrt_nvrecord_config_load(config);
}

void app_ibrt_if_nvrecord_update_ibrt_mode_tws(bool status)
{
    app_ibrt_nvrecord_update_ibrt_mode_tws(status);
}

int app_ibrt_if_nvrecord_get_latest_mobiles_addr(bt_bdaddr_t *mobile_addr1, bt_bdaddr_t* mobile_addr2)
{
    return app_ibrt_nvrecord_get_latest_mobiles_addr(mobile_addr1,mobile_addr2);
}

int app_ibrt_if_config_keeper_clear(void)
{
    memset(&ibrt_config_ram_bak, 0, sizeof(ibrt_config_ram_bak));
    return 0;
}

int app_ibrt_if_config_keeper_flush(void)
{
#ifdef CHIP_BEST1306
    if(export_fn_rom->crc32 != NULL)
    {
        ibrt_config_ram_bak.crc = export_fn_rom->crc32(0,(uint8_t *)(&ibrt_config_ram_bak),(sizeof(ibrt_config_ram_bak_t)-sizeof(uint32_t)));
#else
    if(__export_fn_rom.crc32 != NULL)
    {
        ibrt_config_ram_bak.crc = __export_fn_rom.crc32(0,(uint8_t *)(&ibrt_config_ram_bak),(sizeof(ibrt_config_ram_bak_t)-sizeof(uint32_t)));
#endif
        LOG_I("%s crc:%08x", __func__, ibrt_config_ram_bak.crc);
    }
    return 0;
}

int app_ibrt_if_volume_ptr_update_v2(bt_bdaddr_t *addr)
{
    if (addr)
    {
        app_bt_stream_volume_ptr_update((uint8_t *)addr);
    }

    return 0;
}

int app_ibrt_if_config_keeper_mobile_update(bt_bdaddr_t *addr)
{
    nvrec_btdevicerecord *nv_record = NULL;
    nvrec_btdevicerecord *rambak_record = NULL;

    rambak_record = &ibrt_config_ram_bak.rec_mobile;

    if (!nv_record_btdevicerecord_find(addr,&nv_record))
    {
        LOG_I("%s success", __func__);
        DUMP8("%02x ", nv_record->record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
        DUMP8("%02x ", nv_record->record.linkKey, sizeof(nv_record->record.linkKey));
        ibrt_config_ram_bak.ibrt_config.mobile_addr = *addr;
        *rambak_record = *nv_record;
        app_ibrt_if_config_keeper_flush();
        app_ibrt_if_volume_ptr_update_v2(addr);
    }
    else
    {
        LOG_I("%s failure", __func__);
    }
#ifdef __GMA_VOICE__
    if(app_tws_ibrt_compare_btaddr())
    {
        gma_secret_key_send();
    }
#endif
    return 0;
}

#ifdef __GMA_VOICE__
void app_ibrt_gma_exchange_ble_key()
{
    if(app_tws_ibrt_compare_btaddr())
    {
        gma_secret_key_send();
    }
}
#endif

int app_ibrt_if_config_keeper_tws_update(bt_bdaddr_t *addr)
{
    nvrec_btdevicerecord *nv_record = NULL;
    nvrec_btdevicerecord *rambak_record = NULL;

    rambak_record = &ibrt_config_ram_bak.rec_peer;

    if (!nv_record_btdevicerecord_find(addr,&nv_record))
    {
        LOG_I("%s success", __func__);
        DUMP8("%02x ", nv_record->record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
        DUMP8("%02x ", nv_record->record.linkKey, sizeof(nv_record->record.linkKey));
        ibrt_config_ram_bak.ibrt_config.peer_addr = *addr;
        *rambak_record = *nv_record;
        app_ibrt_if_config_keeper_flush();
        app_ibrt_if_volume_ptr_update_v2(addr);
    }
    else
    {
        LOG_I("%s failure", __func__);
    }

    return 0;
}

/*
 *tws switch stable timer, beacuse tws switch complete event is async
 *so need wait a stable time to let both device syncable
*/
static osTimerId  ibrt_ui_tws_switch_prepare_timer_id = NULL;
static void app_ibrt_ui_tws_switch_prepare_timer_cb(void const *n)
{
    app_ibrt_conn_notify_prepare_complete();
}

osTimerDef (IBRT_UI_TWS_SWITCH_PREPARE_TIMER, app_ibrt_ui_tws_switch_prepare_timer_cb);

static void app_ibrt_ui_start_tws_switch_prepare_supervising(uint32_t timeoutMs)
{
    if (NULL == ibrt_ui_tws_switch_prepare_timer_id)
    {
        ibrt_ui_tws_switch_prepare_timer_id =
            osTimerCreate(osTimer(IBRT_UI_TWS_SWITCH_PREPARE_TIMER), \
                          osTimerOnce, NULL);
    }

    osTimerStart(ibrt_ui_tws_switch_prepare_timer_id, timeoutMs);
}

bool app_ibrt_if_get_bes_ota_state(void)
{
#if BES_OTA
    return app_get_bes_ota_state();
#else
    return false;
#endif
}

void app_ibrt_if_ble_role_switch_start(void)
{
#ifdef __IAG_BLE_INCLUDE__
        bes_ble_roleswitch_start();
#endif
}

void app_ibrt_if_ai_role_switch_prepare(uint32_t *wait_ms)
{
#if defined(__AI_VOICE__) || defined(BISTO_ENABLED)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    p_ibrt_ctrl->ibrt_ai_role_switch_handle = app_ai_tws_role_switch_prepare(wait_ms);
    if (p_ibrt_ctrl->ibrt_ai_role_switch_handle)
    {
        p_ibrt_ctrl->ibrt_role_switch_handle_user |= IBRT_ROLE_SWITCH_USER_AI;
    }
#endif
}

void app_ibrt_if_gfps_role_switch_prepare()
{
#ifdef GFPS_ENABLE
    gfps_role_switch_prepare();
#endif
}

/*
* tws preparation before tws switch if needed
*/
bool app_ibrt_if_tws_switch_prepare_needed(uint32_t *wait_ms)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (p_ibrt_ctrl->ibrt_role_switch_handle_user)
    {
        // this means another mobile link sm had reached here and
        // the tws role switch preparation is already on-going.
        // so we can just let the very mobile link wait for role switch
        // preparation complete event to be informed to both mobile links
        *wait_ms = 800;
        return true;
    }

    bool ret = false;

    // If local device is slave and launch role switch, open uplink firstly.
    // After role switch finished, local device will be master. So open uplink firstly.
    extern int bt_sco_chain_set_master_role(bool is_master);
    if (app_ibrt_if_get_ui_role() == TWS_UI_SLAVE) {
        bt_sco_chain_set_master_role(true);
    }

    app_ibrt_if_ble_role_switch_start();
    app_ibrt_if_ai_role_switch_prepare(wait_ms);
#if defined(GFPS_ENABLED) && !defined(FREEMAN_ENABLED)
    app_ibrt_if_gfps_role_switch_prepare();
#endif

    if (app_ibrt_if_get_bes_ota_state())
    {
        p_ibrt_ctrl->ibrt_role_switch_handle_user |= IBRT_ROLE_SWITCH_USER_OTA;
        *wait_ms = 800;
    }

    if (p_ibrt_ctrl->ibrt_role_switch_handle_user)
    {
        ret = true;
    }

    app_ibrt_middleware_role_switch_started_handler();

    LOG_I("tws_switch_prepare_needed %d wait_ms %d handle 0x%x 0x%x", ret, *wait_ms,
                                                p_ibrt_ctrl->ibrt_ai_role_switch_handle,
                                                p_ibrt_ctrl->ibrt_role_switch_handle_user);
    return ret;
}

void app_ibrt_if_ai_role_switch_handle(void)
{
#if defined(BISTO_ENABLED)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (p_ibrt_ctrl->ibrt_ai_role_switch_handle & (1 << AI_SPEC_GSOUND))
    {
        app_ai_tws_role_switch();
    }
#endif
}

void app_ibrt_if_ota_role_switch_handle(void)
{
#ifdef BES_OTA
    ibrt_ctrl_t *ota_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if ((app_ibrt_if_get_bes_ota_state()) &&\
        (ota_ibrt_ctrl->ibrt_role_switch_handle_user & IBRT_ROLE_SWITCH_USER_OTA))
    {
        app_set_ota_role_switch_initiator(true);
        bes_ota_send_role_switch_req();
    }
#endif
}

/*
* tws preparation before tws switch
*/
void app_ibrt_if_tws_swtich_prepare(uint32_t timeoutMs)
{
    app_ibrt_ui_start_tws_switch_prepare_supervising(timeoutMs);
    app_ibrt_if_ai_role_switch_handle();
    app_ibrt_if_ota_role_switch_handle();
}

/*
* notify UI SM tws preparation done
*/
static void app_ibrt_if_tws_switch_prepare_done(IBRT_ROLE_SWITCH_USER_E user, uint32_t role)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    LOG_I("tws_switch_prepare_done switch_handle 0x%x", \
            p_ibrt_ctrl->ibrt_role_switch_handle_user);

    if (p_ibrt_ctrl->ibrt_role_switch_handle_user)
    {
#if defined(__AI_VOICE__) || defined(BISTO_ENABLED)
        LOG_I("ai_handle 0x%x role %d%s", \
              p_ibrt_ctrl->ibrt_ai_role_switch_handle, \
              role, \
              ai_spec_type2str((AI_SPEC_TYPE_E)role));

        if (user == IBRT_ROLE_SWITCH_USER_AI)
        {
            if (role >= AI_SPEC_COUNT)
            {
                LOG_I("%s role error", __func__);
                return;
            }
            if (!p_ibrt_ctrl->ibrt_ai_role_switch_handle)
            {
                LOG_I("%s ai_handle is 0", __func__);
                return;
            }

            p_ibrt_ctrl->ibrt_ai_role_switch_handle &= ~(1 << role);
            if (!p_ibrt_ctrl->ibrt_ai_role_switch_handle)
            {
                p_ibrt_ctrl->ibrt_role_switch_handle_user &= ~IBRT_ROLE_SWITCH_USER_AI;
            }
        }
        else
#endif
        {
            p_ibrt_ctrl->ibrt_role_switch_handle_user &= ~user;
        }

        if (!p_ibrt_ctrl->ibrt_role_switch_handle_user)
        {
            osTimerStop(ibrt_ui_tws_switch_prepare_timer_id);
            app_ibrt_conn_notify_prepare_complete();
        }
    }
}

void app_ibrt_if_tws_switch_prepare_done_in_bt_thread(IBRT_ROLE_SWITCH_USER_E user, uint32_t role)
{
    app_bt_start_custom_function_in_bt_thread(user,
            role,
            (uint32_t)app_ibrt_if_tws_switch_prepare_done);
}

int app_ibrt_if_config_keeper_resume(ibrt_config_t *config)
{
    uint32_t crc;
    nvrec_btdevicerecord *nv_record = NULL;
    nvrec_btdevicerecord *rambak_record = NULL;
    bool mobile_check_ok = false;
    bool peer_check_ok = false;
    bool flash_need_flush = false;
    bt_bdaddr_t zeroAddr = {0,0,0,0,0,0};

#ifdef CHIP_BEST1306
    crc = export_fn_rom->crc32(0,(uint8_t *)(&ibrt_config_ram_bak),(sizeof(ibrt_config_ram_bak_t)-sizeof(uint32_t)));
#else
    crc = __export_fn_rom.crc32(0,(uint8_t *)(&ibrt_config_ram_bak),(sizeof(ibrt_config_ram_bak_t)-sizeof(uint32_t)));
#endif

    LOG_I("%s start crc:%08x/%08x", __func__, ibrt_config_ram_bak.crc, crc);
    if (crc == ibrt_config_ram_bak.crc)
    {
        LOG_I("%s success", __func__);
        LOG_I("%s config loc", __func__);
        DUMP8("%02x ", config->local_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
        LOG_I("%s config mobile", __func__);
        DUMP8("%02x ", config->mobile_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
        LOG_I("%s config peer", __func__);
        DUMP8("%02x ", config->peer_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);

        rambak_record = &ibrt_config_ram_bak.rec_mobile;
        if (!nv_record_btdevicerecord_find(&config->mobile_addr,&nv_record))
        {
            LOG_I("%s  find mobile", __func__);
            DUMP8("%02x ", nv_record->record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
            DUMP8("%02x ", nv_record->record.linkKey, sizeof(nv_record->record.linkKey));
            if (!memcmp(rambak_record->record.linkKey, nv_record->record.linkKey, sizeof(nv_record->record.linkKey)))
            {
                LOG_I("%s  check mobile success", __func__);
                mobile_check_ok = true;
            }
        }
        if (!mobile_check_ok)
        {
            LOG_I("%s  check mobile failure", __func__);
            DUMP8("%02x ", rambak_record->record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
            DUMP8("%02x ", rambak_record->record.linkKey, sizeof(rambak_record->record.linkKey));
            if (memcmp(rambak_record->record.bdAddr.address, zeroAddr.address, sizeof(zeroAddr)))
            {
                nv_record_add(section_usrdata_ddbrecord, rambak_record);
                config->mobile_addr = rambak_record->record.bdAddr;
                flash_need_flush = true;
                LOG_I("%s resume mobile", __func__);
            }
        }

        rambak_record = &ibrt_config_ram_bak.rec_peer;
        if (!nv_record_btdevicerecord_find(&config->peer_addr,&nv_record))
        {
            LOG_I("%s  find tws peer", __func__);
            DUMP8("%02x ", nv_record->record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
            DUMP8("%02x ", nv_record->record.linkKey, sizeof(nv_record->record.linkKey));
            if (!memcmp(rambak_record->record.linkKey, nv_record->record.linkKey, sizeof(nv_record->record.linkKey)))
            {
                LOG_I("%s  check tws peer success", __func__);
                peer_check_ok = true;
            }
        }
        if (!peer_check_ok)
        {
            LOG_I("%s  check tws peer failure", __func__);
            DUMP8("%02x ", rambak_record->record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
            DUMP8("%02x ", rambak_record->record.linkKey, sizeof(rambak_record->record.linkKey));
            if (memcmp(rambak_record->record.bdAddr.address, zeroAddr.address, sizeof(zeroAddr)))
            {
                nv_record_add(section_usrdata_ddbrecord, rambak_record);
                config->peer_addr = rambak_record->record.bdAddr;
                flash_need_flush = true;
                LOG_I("%s resume tws peer", __func__);
            }
        }

    }

    ibrt_config_ram_bak.ibrt_config = *config;
    rambak_record = &ibrt_config_ram_bak.rec_mobile;
    if (!nv_record_btdevicerecord_find(&config->mobile_addr,&nv_record))
    {
        *rambak_record = *nv_record;
    }
    else
    {
        memset(rambak_record, 0, sizeof(nvrec_btdevicerecord));
    }
    rambak_record = &ibrt_config_ram_bak.rec_peer;
    if (!nv_record_btdevicerecord_find(&config->peer_addr,&nv_record))
    {
        *rambak_record = *nv_record;
    }
    else
    {
        memset(rambak_record, 0, sizeof(nvrec_btdevicerecord));
    }
    app_ibrt_if_config_keeper_flush();
    if (flash_need_flush)
    {
        nv_record_flash_flush();
    }
    LOG_I("%s end crc:%08x", __func__, ibrt_config_ram_bak.crc);

    LOG_I("%s mobile", __func__);
    DUMP8("%02x ", ibrt_config_ram_bak.rec_mobile.record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", ibrt_config_ram_bak.rec_mobile.record.linkKey, sizeof(ibrt_config_ram_bak.rec_mobile.record.linkKey));
    LOG_I("%s peer", __func__);
    DUMP8("%02x ", ibrt_config_ram_bak.rec_peer.record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", ibrt_config_ram_bak.rec_peer.record.linkKey, sizeof(ibrt_config_ram_bak.rec_peer.record.linkKey));
    return 0;
}

void app_ibrt_if_set_access_mode(ibrt_if_access_mode_enum mode)
{
    app_bt_ME_SetAccessibleMode((btif_accessible_mode_t)mode);
}

static bool g_ibrt_if_bluetooth_is_enabling = false;

void app_ibrt_if_stack_is_ready(void)
{
    LOG_I("%s", __func__);
    if (g_ibrt_if_bluetooth_is_enabling)
    {
        app_custom_ui_notify_bluetooth_enabled();
        g_ibrt_if_bluetooth_is_enabling = false;
    }
}

void app_ibrt_if_enable_bluetooth(void)
{
    g_ibrt_if_bluetooth_is_enabling = false;

    if (app_is_stack_ready())
    {
        ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        if (p_ibrt_ctrl->init_done)
        {
            app_ibrt_conn_set_ui_role((TWS_UI_ROLE_E)(p_ibrt_ctrl->nv_role));
        }

#if defined(IBRT_UI_V2)
        ibrt_global_state_change_event evt;
        evt.header.type = IBRT_CONN_EVENT_GLOBAL_STATE;
        evt.state = IBRT_BLUETOOTH_ENABLED;
        app_ibrt_customif_global_state_callback(&evt);
#else
        app_custom_ui_notify_bluetooth_enabled();
#endif
    }
    else
    {
        LOG_I("%s waiting", __func__);
        g_ibrt_if_bluetooth_is_enabling = true;
    }
}

static bool g_ibrt_if_bluetooth_is_disabling = false;

void app_ibrt_if_case_is_closed_complete(void)
{
    /* case is closed */
}

void app_ibrt_if_link_disconnected(void)
{
    if (g_ibrt_if_bluetooth_is_disabling)
    {
        if (!app_bt_manager.tws_conn.acl_is_connected && !app_bt_count_connected_device())
        {
            app_custom_ui_notify_bluetooth_disabled();
            g_ibrt_if_bluetooth_is_disabling = false;
        }
    }
}

void app_ibrt_if_disable_bluetooth(void)
{
    app_disconnect_all_bt_connections();
    app_custom_ui_notify_bluetooth_disabled();

    g_ibrt_if_bluetooth_is_disabling = false;

    if (app_bt_count_connected_device() || app_tws_ibrt_tws_link_connected())
    {
        app_bt_start_custom_function_in_bt_thread(
            (uint32_t)NULL, (uint32_t)NULL, (uint32_t)(uintptr_t)app_disconnect_all_bt_connections);
        g_ibrt_if_bluetooth_is_disabling = true;
    }
    else
    {
        app_custom_ui_notify_bluetooth_disabled();
    }
}

void app_ibrt_if_set_extended_inquiry_response(ibrt_if_extended_inquiry_response_t *data)
{
    static ibrt_if_extended_inquiry_response_t tmpEIR;
    tmpEIR = *data;
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)tmpEIR.eir, sizeof(tmpEIR.eir), (uint32_t)(uintptr_t)btif_set_extended_inquiry_response);
}

#if defined(BT_DIP_SUPPORT)
ibrt_if_pnp_info* app_ibrt_if_get_pnp_info(bt_bdaddr_t *remote)
{
    return (ibrt_if_pnp_info*)btif_dip_get_device_info(remote);
}
#endif

/*
* only used for factory.
* this function is only for freeman pairing,no tws link or ibrt link should be connected
* when mobile link or tws link exist, this function will disconnect mobile link and tws link
*/
void app_ibrt_if_enter_freeman_pairing(void)
{
#ifdef IBRT_UI_V2
    uint8_t* currentBtAddr = factory_section_get_bt_address();
    ASSERT(currentBtAddr, "%s: Bad bt address", __func__);

    // disable access mode
    scanMgrReset();
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    memcpy(p_ibrt_ctrl->local_addr.address, currentBtAddr, BD_ADDR_LEN);

    app_tws_update_local_bt_addr(currentBtAddr);

    app_ibrt_conn_set_freeman_enable();
    app_ibrt_if_init_open_box_state_for_evb();
    app_ibrt_if_event_entry(APP_UI_EV_FREE_MAN_MODE);
#endif
}

app_ibrt_if_ctrl_t *app_ibrt_if_get_bt_ctrl_ctx(void)
{
    return (app_ibrt_if_ctrl_t *)app_tws_ibrt_get_bt_ctrl_ctx();
}


void app_ibrt_if_write_bt_local_address(uint8_t* btAddr)
{
    bt_bdaddr_t tmpBtAddr;
    app_ibrt_if_big_little_switch(btAddr, tmpBtAddr.address, 6);
    btAddr = tmpBtAddr.address;

    uint8_t* currentBtAddr = factory_section_get_bt_address();
    ASSERT(currentBtAddr, "%s: Bad bt address", __func__);
    if (memcmp(currentBtAddr, btAddr, BD_ADDR_LEN))
    {
        nv_record_rebuild(NV_REBUILD_SDK_ONLY);
        ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        memcpy(p_ibrt_ctrl->local_addr.address, btAddr, BD_ADDR_LEN);
        factory_section_set_bt_address(btAddr);

        app_tws_update_local_bt_addr(btAddr);

        nv_record_flash_flush();
    }
}

void app_ibrt_if_write_ble_local_address(uint8_t* bleAddr)
{
    bt_bdaddr_t tmpBleAddr;
    app_ibrt_if_big_little_switch(bleAddr, tmpBleAddr.address, 6);
    bleAddr = tmpBleAddr.address;

    uint8_t* currentBleAddr = factory_section_get_ble_address();
    ASSERT(currentBleAddr, "%s: Bad ble address", __func__);
    if (memcmp(currentBleAddr, bleAddr, BD_ADDR_LEN))
    {
        nv_record_rebuild(NV_REBUILD_SDK_ONLY);
        ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        memcpy(p_ibrt_ctrl->local_addr.address, bleAddr, BD_ADDR_LEN);
        factory_section_set_ble_address(bleAddr);

        app_tws_update_local_ble_addr(bleAddr);

        nv_record_flash_flush();
    }
}

static bt_bdaddr_t tmpLocalBtAddr;
uint8_t *app_ibrt_if_get_bt_local_address(void)
{
    uint8_t* localBtAddr = factory_section_get_bt_address();
    app_ibrt_if_big_little_switch(localBtAddr, tmpLocalBtAddr.address, 6);
    return tmpLocalBtAddr.address;
}

static bt_bdaddr_t tmpLocalBleAddr;
uint8_t *app_ibrt_if_get_ble_local_address(void)
{
    uint8_t* localBleAddr = factory_section_get_ble_address();
    app_ibrt_if_big_little_switch(localBleAddr, tmpLocalBleAddr.address, 6);
    return tmpLocalBleAddr.address;
}

static bt_bdaddr_t tmpPeerBtAddr;
uint8_t *app_ibrt_if_get_bt_peer_address(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    app_ibrt_if_big_little_switch(nvrecord_env->ibrt_mode.record.bdAddr.address, tmpPeerBtAddr.address, 6);
    return tmpPeerBtAddr.address;
}

/*
* this function is only for tws ibrt pairing mode
* when tws earphone is in the nearby, tws link will be connected firstly
* when mobile link exist, this function will disconnect mobile link
*/
void app_ibrt_if_enter_pairing_after_tws_connected(void)
{
    #ifdef IBRT_UI_V2
    app_ibrt_if_event_entry(APP_UI_EV_TWS_PAIRING);
    #endif
}

extern void app_ibrt_mgr_pairing_mode_test(void);
void app_ibrt_if_enter_pairing_after_power_on(void)
{
    app_ibrt_mgr_pairing_mode_test();
}

void app_ibrt_if_ctx_checker(void)
{
    #ifdef IBRT_UI_V2
    ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
    bud_box_state  box_state = app_ui_get_local_box_state();

    LOG_I("checker: nv_role:%d current_role:%s access_mode:%d",
          p_ibrt_ctrl->nv_role,
          app_bt_get_device_current_roles(),
          p_ibrt_ctrl->access_mode);

    if(app_ibrt_conn_any_mobile_connected())
    {
        rx_agc_t link_agc_info = {0};
        //IBRT_MASTER trace FA rssi info
        bool ret = bt_drv_reg_op_read_fa_rssi_in_dbm(&link_agc_info);
        if (ret)
        {
            LOG_I("FA RSSI=%d,RX gain =%d", link_agc_info.rssi,link_agc_info.rxgain);
        }
    }
    DUMP8("%02x ", p_ibrt_ctrl->local_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", p_ibrt_ctrl->peer_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);

    LOG_I("checker: box_state:%s",app_ui_box_state_to_string(box_state));

    if (p_ibrt_ctrl->tws_mode == IBRT_ACTIVE_MODE)
    {
        bt_drv_reg_op_bt_info_checker();
    }
    #endif
}

void app_ibrt_if_init_open_box_state_for_evb(void)
{
#ifdef IBRT_UI_V2
    LOG_I("app_ibrt_if_init_open_box_state_for_evb");
    app_ibrt_if_event_entry(APP_UI_EV_CASE_OPEN);
    osDelay(50);
#endif
}

static void app_ibrt_tws_sync_retrigger_a2dp_handler(void)
{
    uint8_t device_id = app_bt_audio_get_curr_playing_a2dp();
    if (device_id == BT_DEVICE_INVALID_ID){
        LOG_I("(d%d)retrigger_a2dp_handler, not in playing status", device_id);
        return;
    }

    a2dp_audio_retrigger_set_on_process(true);
    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP, BT_STREAM_MUSIC,
        device_id, MAX_RECORD_NUM);
    app_bt_manager.curr_playing_a2dp_id = device_id;
    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START, BT_STREAM_MUSIC,
        device_id, MAX_RECORD_NUM);
}

static void app_ibrt_tws_sync_retrigger_a2dp_status_notify(uint32_t opCode,
    bool triStatus, bool triInfoSentStatus)
{
    if ((!triStatus) && (APP_BT_SYNC_OP_RETRIGGER == opCode)) {
        // bt sync supervision timeout
        app_ibrt_tws_sync_retrigger_a2dp_handler();
    }
}

APP_BT_SYNC_COMMAND_TO_ADD(APP_BT_SYNC_OP_RETRIGGER, app_ibrt_tws_sync_retrigger_a2dp_handler,
    app_ibrt_tws_sync_retrigger_a2dp_status_notify);

int app_ibrt_if_force_audio_retrigger(uint8_t retriggerType)
{
#if defined(__AUDIO_RETRIGGER_REPORT__)
    app_ibrt_if_report_audio_retrigger((uint8_t)retriggerType);
#endif
    if (!app_tws_ibrt_audio_retrigger()) {
        app_ibrt_tws_sync_retrigger_a2dp_handler();
    }
    return 0;
}

void app_ibrt_if_audio_mute(void)
{

}

void app_ibrt_if_audio_recover(void)
{

}

bool app_ibrt_if_ota_is_in_progress(void)
{
#ifdef BES_OTA
    return ota_is_in_progress();
#else
    return false;
#endif
}

bool app_ibrt_if_start_ibrt_onporcess(const bt_bdaddr_t *addr)
{
    return !app_ibrt_conn_is_ibrt_idle(addr);
}

void app_ibrt_if_get_tws_conn_state_test(void)
{
    if(app_tws_ibrt_tws_link_connected())
    {
        LOG_I("ibrt_ui_log:TWS CONNECTED");
    }
    else
    {
        LOG_I("ibrt_ui_log:TWS DISCONNECTED");
    }
}

bt_status_t app_tws_if_ibrt_write_link_policy(const bt_bdaddr_t *p_addr, btif_link_policy_t policy)
{
    return app_tws_ibrt_write_link_policy(p_addr, policy);
}

struct app_ibrt_profile_req
{
    bool error_status;
    bt_bdaddr_t remote;
    app_ibrt_profile_id_enum profile_id;
    uint32_t extra_data;
};

static void app_ibrt_if_connect_profile_handler(bool is_ibrt_slave_receive_request, uint8_t *p_buff, uint16_t length)
{
    struct app_ibrt_profile_req *profile_req = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;
    struct BT_DEVICE_T *curr_device = NULL;

    profile_req = (struct app_ibrt_profile_req *)p_buff;
    device_id = app_bt_get_device_id_byaddr(&profile_req->remote);

    LOG_I("%s d%x profile %x status %d", __func__, device_id, profile_req->profile_id, profile_req->error_status);

    curr_device = app_bt_get_device(device_id);

    if (curr_device == NULL || !curr_device->acl_is_connected)
    {
        profile_req->error_status = true;
        return;
    }

    if (app_bt_manager.config.keep_only_one_stream_close_connected_a2dp && profile_req->profile_id == APP_IBRT_A2DP_PROFILE_ID)
    {
        if (is_ibrt_slave_receive_request)
        {
            uint8_t another_streaming_device = app_bt_audio_select_another_streaming_a2dp(device_id);
            if (another_streaming_device != BT_DEVICE_INVALID_ID)
            {
                struct BT_DEVICE_T *another_device = app_bt_get_device(another_streaming_device);
                if (BTIF_AVRCP_MEDIA_PLAYING == another_device->avrcp_palyback_status && app_ibrt_conn_is_profile_exchanged(&another_device->remote))
                {
                    LOG_I("%s d%x skip due to another device d%x streaming", __func__, device_id, another_streaming_device);
                    profile_req->error_status = true;
                    curr_device->this_is_closed_bg_a2dp = true;
                    return;
                }
            }
        }
        else
        {
            if (profile_req->error_status)
            {
                LOG_I("%s d%x skip a2dp connect", __func__, device_id);
                curr_device->this_is_closed_bg_a2dp = true;
                return;
            }
        }
    }

    if (profile_req->error_status)
    {
        LOG_I("%s d%x skip due to remote error_status", __func__, device_id);
        return;
    }

    btif_me_reset_l2cap_sigid(&curr_device->remote);

    switch (profile_req->profile_id)
    {
        case APP_IBRT_HFP_PROFILE_ID:
            app_bt_reconnect_hfp_profile(&curr_device->remote);
            break;
        case APP_IBRT_A2DP_PROFILE_ID:
            app_bt_reconnect_a2dp_profile(&curr_device->remote);
            curr_device->a2dp_disc_on_process = 0;
            curr_device->this_is_closed_bg_a2dp = false;
            break;
        case APP_IBRT_AVRCP_PROFILE_ID:
            app_bt_reconnect_avrcp_profile(&curr_device->remote);
            break;
        case APP_IBRT_HID_PROFILE_ID:
#if defined(BT_HID_DEVICE)
            app_bt_hid_profile_connect(&curr_device->remote, profile_req->extra_data ? true : false);
#endif
            break;
        default:
            break;
    }
}

static void app_ibrt_if_disconnect_profile_handler(uint8_t *p_buff, uint16_t length)
{
    struct app_ibrt_profile_req *profile_req = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;
    struct BT_DEVICE_T *curr_device = NULL;
    profile_req = (struct app_ibrt_profile_req *)p_buff;
    device_id = app_bt_get_device_id_byaddr(&profile_req->remote);
    LOG_I("%s d%x profile %x", __func__, device_id, profile_req->profile_id);
    if (device_id == BT_DEVICE_INVALID_ID)
    {
        return;
    }
    curr_device = app_bt_get_device(device_id);
    switch (profile_req->profile_id)
    {
        case APP_IBRT_HFP_PROFILE_ID:
            app_bt_disconnect_hfp_profile(curr_device->hf_channel);
            break;
        case APP_IBRT_A2DP_PROFILE_ID:
            app_bt_disconnect_a2dp_profile(curr_device->a2dp_connected_stream);
            curr_device->ibrt_disc_a2dp_profile_only = true;
            curr_device->a2dp_disc_on_process = 1;
            if (!curr_device->this_is_closed_bg_a2dp)
            {
                curr_device->this_is_closed_bg_a2dp = true;
            }
            break;
        case APP_IBRT_AVRCP_PROFILE_ID:
            app_bt_disconnect_avrcp_profile(curr_device->avrcp_channel);
            break;
        case APP_IBRT_HID_PROFILE_ID:
#if defined(BT_HID_DEVICE)
            app_bt_hid_profile_disconnect(&profile_req->remote);
#endif
            break;
        default:
            break;
    }
}

struct app_ibrt_rfcomm_req
{
    uint8_t reason;
    uint32_t sppdev_ptr;
};

static void app_ibrt_if_disconnect_rfcomm_handler(uint8_t *p_buff)
{
    struct app_ibrt_rfcomm_req *rfcomm_req = NULL;
    bt_spp_channel_t *spp_chan = NULL;
    rfcomm_req = (struct app_ibrt_rfcomm_req *)p_buff;
    spp_chan = (bt_spp_channel_t *)rfcomm_req->sppdev_ptr;
    bt_spp_disconnect(spp_chan->rfcomm_handle, rfcomm_req->reason);
}

#define APP_IBRT_PROFILE_CONN_PROTECT_TIME 3000
#define APP_IBRT_PROFILE_DISC_PROTECT_TIME 2000
#define APP_IBRT_PROFILE_DELAY_TIME_MS 500

static RS_TASK_ID app_ibrt_profile_connect_get_rs_task_id(uint8_t device_id, int profile_id)
{
    RS_TASK_ID task_id[BT_DEVICE_NUM][APP_IBRT_MAX_PROFILE_ID] = {
            {RS_TASK_INVALID_ID, RS_TASK_HFP_CONN_D0, RS_TASK_A2DP_CONN_D0, RS_TASK_AVRCP_CONN_D0},
            {RS_TASK_INVALID_ID, RS_TASK_HFP_CONN_D1, RS_TASK_A2DP_CONN_D1, RS_TASK_AVRCP_CONN_D1},
        };
    if (device_id < BT_DEVICE_NUM && profile_id < APP_IBRT_MAX_PROFILE_ID) {
        return task_id[device_id][profile_id];
    } else {
        return RS_TASK_INVALID_ID;
    }
}

static RS_TASK_ID app_ibrt_profile_disconnect_get_rs_task_id(uint8_t device_id, int profile_id)
{
    RS_TASK_ID task_id[BT_DEVICE_NUM][APP_IBRT_MAX_PROFILE_ID] = {
            {RS_TASK_INVALID_ID, RS_TASK_HFP_DISC_D0, RS_TASK_A2DP_DISC_D0, RS_TASK_AVRCP_DISC_D0},
            {RS_TASK_INVALID_ID, RS_TASK_HFP_DISC_D1, RS_TASK_A2DP_DISC_D1, RS_TASK_AVRCP_DISC_D1},
        };
    if (device_id < BT_DEVICE_NUM && profile_id < APP_IBRT_MAX_PROFILE_ID) {
        return task_id[device_id][profile_id];
    } else {
        return RS_TASK_INVALID_ID;
    }
}

void app_ibrt_set_profile_connect_protect(uint8_t device_id, int profile_id)
{
    RS_TASK_ID task_id = app_ibrt_profile_connect_get_rs_task_id(device_id, profile_id);
    if (task_id != RS_TASK_INVALID_ID) {
        LOG_I("app_ibrt_set_profile_connect_protect: %d time %d", task_id, APP_IBRT_PROFILE_CONN_PROTECT_TIME);
        app_ibrt_conn_rs_task_set(task_id, APP_IBRT_PROFILE_CONN_PROTECT_TIME);
    }
}

void app_ibrt_clear_profile_connect_protect(uint8_t device_id, int profile_id)
{
    RS_TASK_ID task_id = app_ibrt_profile_connect_get_rs_task_id(device_id, profile_id);
    if (task_id != RS_TASK_INVALID_ID) {
        LOG_I("app_ibrt_clear_profile_connect_protect: %d", task_id);
        app_ibrt_conn_rs_task_clr(task_id);
    }
}

void app_ibrt_set_profile_disconnect_protect(uint8_t device_id, int profile_id)
{
    RS_TASK_ID task_id = app_ibrt_profile_disconnect_get_rs_task_id(device_id, profile_id);
    if (task_id != RS_TASK_INVALID_ID) {
        LOG_I("app_ibrt_set_profile_disconnect_protect: %d time %d", task_id, APP_IBRT_PROFILE_DISC_PROTECT_TIME);
        app_ibrt_conn_rs_task_set(task_id, APP_IBRT_PROFILE_DISC_PROTECT_TIME);
    }
}

void app_ibrt_clear_profile_disconnect_protect(uint8_t device_id, int profile_id)
{
    RS_TASK_ID task_id = app_ibrt_profile_disconnect_get_rs_task_id(device_id, profile_id);
    if (task_id != RS_TASK_INVALID_ID) {
        LOG_I("app_ibrt_clear_profile_disconnect_protect: %d", task_id);
        app_ibrt_conn_rs_task_clr(task_id);
    }
}

static void app_ibrt_hfp_delay_conn_handler(void const *ctx)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T *)ctx;
    if (curr_device && curr_device->acl_is_connected && !curr_device->hf_conn_flag) {
        app_ibrt_if_profile_connect(curr_device->device_id, APP_IBRT_HFP_PROFILE_ID, 0);
    }
}

static void app_ibrt_a2dp_delay_conn_handler(void const *ctx)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T *)ctx;
    if (curr_device && curr_device->acl_is_connected && !curr_device->a2dp_conn_flag) {
        app_ibrt_if_profile_connect(curr_device->device_id, APP_IBRT_A2DP_PROFILE_ID, 0);
    }
}

static void app_ibrt_avrcp_delay_conn_handler(void const *ctx)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T *)ctx;
    if (curr_device && curr_device->acl_is_connected && !curr_device->avrcp_conn_flag) {
        app_ibrt_if_profile_connect(curr_device->device_id, APP_IBRT_AVRCP_PROFILE_ID, 0);
    }
}

static void app_ibrt_hfp_delay_disc_handler(void const *ctx)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T *)ctx;
    if (curr_device && curr_device->acl_is_connected && curr_device->hf_conn_flag) {
        app_ibrt_if_profile_disconnect(curr_device->device_id, APP_IBRT_HFP_PROFILE_ID);
    }
}

static void app_ibrt_a2dp_delay_disc_handler(void const *ctx)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T *)ctx;
    if (curr_device && curr_device->acl_is_connected && curr_device->a2dp_conn_flag) {
        app_ibrt_if_profile_disconnect(curr_device->device_id, APP_IBRT_A2DP_PROFILE_ID);
    }
}

static void app_ibrt_avrcp_delay_disc_handler(void const *ctx)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T *)ctx;
    if (curr_device && curr_device->acl_is_connected && curr_device->avrcp_conn_flag) {
        app_ibrt_if_profile_disconnect(curr_device->device_id, APP_IBRT_AVRCP_PROFILE_ID);
    }
}

osTimerDef(APP_IBRT_HFP_DELAY_CONN_TIMER0, app_ibrt_hfp_delay_conn_handler);
osTimerDef(APP_IBRT_A2DP_DELAY_CONN_TIMER0, app_ibrt_a2dp_delay_conn_handler);
osTimerDef(APP_IBRT_AVRCP_DELAY_CONN_TIMER0, app_ibrt_avrcp_delay_conn_handler);
osTimerDef(APP_IBRT_HFP_DELAY_DISC_TIMER0, app_ibrt_hfp_delay_disc_handler);
osTimerDef(APP_IBRT_A2DP_DELAY_DISC_TIMER0, app_ibrt_a2dp_delay_disc_handler);
osTimerDef(APP_IBRT_AVRCP_DELAY_DISC_TIMER0, app_ibrt_avrcp_delay_disc_handler);
#if BT_DEVICE_NUM > 1
osTimerDef(APP_IBRT_HFP_DELAY_CONN_TIMER1, app_ibrt_hfp_delay_conn_handler);
osTimerDef(APP_IBRT_A2DP_DELAY_CONN_TIMER1, app_ibrt_a2dp_delay_conn_handler);
osTimerDef(APP_IBRT_AVRCP_DELAY_CONN_TIMER1, app_ibrt_avrcp_delay_conn_handler);
osTimerDef(APP_IBRT_HFP_DELAY_DISC_TIMER1, app_ibrt_hfp_delay_disc_handler);
osTimerDef(APP_IBRT_A2DP_DELAY_DISC_TIMER1, app_ibrt_a2dp_delay_disc_handler);
osTimerDef(APP_IBRT_AVRCP_DELAY_DISC_TIMER1, app_ibrt_avrcp_delay_disc_handler);
#endif

void app_ibrt_profile_protect_timer_init(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; i += 1) {
        curr_device = app_bt_get_device(i);
        if (i == 0) {
            curr_device->hfp_delay_conn_timer =
                osTimerCreate(osTimer(APP_IBRT_HFP_DELAY_CONN_TIMER0), osTimerOnce, curr_device);
            curr_device->a2dp_delay_conn_timer =
                osTimerCreate(osTimer(APP_IBRT_A2DP_DELAY_CONN_TIMER0), osTimerOnce, curr_device);
            curr_device->avrcp_delay_conn_timer =
                osTimerCreate(osTimer(APP_IBRT_AVRCP_DELAY_CONN_TIMER0), osTimerOnce, curr_device);
            curr_device->hfp_delay_disc_timer =
                osTimerCreate(osTimer(APP_IBRT_HFP_DELAY_DISC_TIMER0), osTimerOnce, curr_device);
            curr_device->a2dp_delay_disc_timer =
                osTimerCreate(osTimer(APP_IBRT_A2DP_DELAY_DISC_TIMER0), osTimerOnce, curr_device);
            curr_device->avrcp_delay_disc_timer =
                osTimerCreate(osTimer(APP_IBRT_AVRCP_DELAY_DISC_TIMER0), osTimerOnce, curr_device);
        }
#if BT_DEVICE_NUM > 1
        else if (i == 1) {
            curr_device->hfp_delay_conn_timer =
                osTimerCreate(osTimer(APP_IBRT_HFP_DELAY_CONN_TIMER1), osTimerOnce, curr_device);
            curr_device->a2dp_delay_conn_timer =
                osTimerCreate(osTimer(APP_IBRT_A2DP_DELAY_CONN_TIMER1), osTimerOnce, curr_device);
            curr_device->avrcp_delay_conn_timer =
                osTimerCreate(osTimer(APP_IBRT_AVRCP_DELAY_CONN_TIMER1), osTimerOnce, curr_device);
            curr_device->hfp_delay_disc_timer =
                osTimerCreate(osTimer(APP_IBRT_HFP_DELAY_DISC_TIMER1), osTimerOnce, curr_device);
            curr_device->a2dp_delay_disc_timer =
                osTimerCreate(osTimer(APP_IBRT_A2DP_DELAY_DISC_TIMER1), osTimerOnce, curr_device);
            curr_device->avrcp_delay_disc_timer =
                osTimerCreate(osTimer(APP_IBRT_AVRCP_DELAY_DISC_TIMER1), osTimerOnce, curr_device);
        }
#endif
    }
}

void app_ibrt_stop_profile_protect_timer(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL) {
        return;
    }
    if (curr_device->hfp_delay_conn_timer) {
        osTimerStop(curr_device->hfp_delay_conn_timer);
    }
    if (curr_device->a2dp_delay_conn_timer) {
        osTimerStop(curr_device->a2dp_delay_conn_timer);
    }
    if (curr_device->avrcp_delay_conn_timer) {
        osTimerStop(curr_device->avrcp_delay_conn_timer);
    }
    if (curr_device->hfp_delay_disc_timer) {
        osTimerStop(curr_device->hfp_delay_disc_timer);
    }
    if (curr_device->a2dp_delay_disc_timer) {
        osTimerStop(curr_device->a2dp_delay_disc_timer);
    }
    if (curr_device->avrcp_delay_disc_timer) {
        osTimerStop(curr_device->avrcp_delay_disc_timer);
    }
}

void app_ibrt_start_profile_connect_delay_timer(uint8_t device_id, int profile_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    LOG_I("(d%x) app_ibrt_start_profile_connect_delay_timer: profile %d", device_id, profile_id);
    if (curr_device && curr_device->acl_is_connected) {
        switch (profile_id) {
            case APP_IBRT_HFP_PROFILE_ID:
                if (curr_device->hfp_delay_conn_timer) {
                    osTimerStop(curr_device->hfp_delay_conn_timer);
                    osTimerStart(curr_device->hfp_delay_conn_timer, APP_IBRT_PROFILE_DELAY_TIME_MS);
                }
                break;
            case APP_IBRT_A2DP_PROFILE_ID:
                if (curr_device->a2dp_delay_conn_timer) {
                    osTimerStop(curr_device->a2dp_delay_conn_timer);
                    osTimerStart(curr_device->a2dp_delay_conn_timer, APP_IBRT_PROFILE_DELAY_TIME_MS);
                }
                break;
            case APP_IBRT_AVRCP_PROFILE_ID:
                if (curr_device->avrcp_delay_conn_timer) {
                    osTimerStop(curr_device->avrcp_delay_conn_timer);
                    osTimerStart(curr_device->avrcp_delay_conn_timer, APP_IBRT_PROFILE_DELAY_TIME_MS);
                }
                break;
            default:
                break;
        }
    }
}

void app_ibrt_start_profile_disconnect_delay_timer(uint8_t device_id, int profile_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    LOG_I("(d%x) app_ibrt_start_profile_disconnect_delay_timer: profile %d", device_id, profile_id);
    if (curr_device && curr_device->acl_is_connected) {
        switch (profile_id) {
            case APP_IBRT_HFP_PROFILE_ID:
                if (curr_device->hfp_delay_disc_timer) {
                    osTimerStop(curr_device->hfp_delay_disc_timer);
                    osTimerStart(curr_device->hfp_delay_disc_timer, APP_IBRT_PROFILE_DELAY_TIME_MS);
                }
                break;
            case APP_IBRT_A2DP_PROFILE_ID:
                if (curr_device->a2dp_delay_disc_timer) {
                    osTimerStop(curr_device->a2dp_delay_disc_timer);
                    osTimerStart(curr_device->a2dp_delay_disc_timer, APP_IBRT_PROFILE_DELAY_TIME_MS);
                }
                break;
            case APP_IBRT_AVRCP_PROFILE_ID:
                if (curr_device->avrcp_delay_disc_timer) {
                    osTimerStop(curr_device->avrcp_delay_disc_timer);
                    osTimerStart(curr_device->avrcp_delay_disc_timer, APP_IBRT_PROFILE_DELAY_TIME_MS);
                }
                break;
            default:
                break;
        }
    }
}

void app_ibrt_if_profile_connect(uint8_t device_id, int profile_id, uint32_t extra_data)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct app_ibrt_profile_req profile_req = {false, {0}};
#ifdef IBRT_UI_V2
    bud_box_state  box_state = app_ui_get_local_box_state();
#endif
    if (curr_device == NULL || !curr_device->acl_is_connected)
    {
        LOG_I("%s d%x profile %d curr_device NULL", __func__, device_id, profile_id);
        return;
    }
#ifdef IBRT_UI_V2
    else if(box_state == IBRT_IN_BOX_CLOSED)
    {
        LOG_I("######app_ibrt_if_profile_connect failed,box is closed!!!");
        return;
    }
#endif
    LOG_I("%s d%x profile %d", __func__, device_id, profile_id);
    if (profile_id == APP_IBRT_A2DP_PROFILE_ID)
    {
        curr_device->ibrt_disc_a2dp_profile_only = false;
        curr_device->a2dp_disc_on_process = 0;
        curr_device->this_is_closed_bg_a2dp = false;
    }

    profile_req.remote = curr_device->remote;
    profile_req.profile_id = (app_ibrt_profile_id_enum)profile_id;
    profile_req.extra_data = extra_data;

    if (tws_besaud_is_connected())
    {
        uint8_t ibrt_role = app_tws_get_ibrt_role(&curr_device->remote);
        LOG_I("(d%x) %s role %d", device_id, __func__, ibrt_role);

        if (IBRT_MASTER == ibrt_role)
        {
            if (app_ibrt_conn_is_profile_exchanged(&curr_device->remote))
            {
                if (app_ibrt_conn_any_role_switch_running()) {
                    app_ibrt_clear_profile_connect_protect(device_id, profile_id);
                    app_ibrt_start_profile_connect_delay_timer(device_id, profile_id);
                    return;
                } else {
                    app_ibrt_set_profile_connect_protect(device_id, profile_id);
                }
                tws_ctrl_send_cmd(APP_TWS_CMD_CONN_PROFILE_REQ, (uint8_t*)&profile_req, sizeof(struct app_ibrt_profile_req));
            }
            else
            {
                app_ibrt_if_connect_profile_handler(false, (uint8_t*)&profile_req, sizeof(struct app_ibrt_profile_req));
            }
        }
        else if (IBRT_SLAVE == ibrt_role)
        {
            app_ibrt_if_start_user_action_v2(device_id, IBRT_ACTION_TELL_MASTER_CONN_PROFILE, profile_req.profile_id, profile_req.extra_data);
        }
    }
    else
    {
        app_ibrt_if_connect_profile_handler(false, (uint8_t*)&profile_req, sizeof(struct app_ibrt_profile_req));
    }
}

bool app_ibrt_if_role_unified()
{
    uint8_t ibrt_role;
    uint8_t pre_ibrt_role = IBRT_UNKNOW;
    bool role_unified = false;
    struct BT_DEVICE_T * curr_device = NULL;
    for(uint8_t i=0; i< BT_DEVICE_NUM; i++)
    {
        curr_device = app_bt_get_device(i);
        ibrt_role = app_tws_get_ibrt_role(&curr_device->remote);
        role_unified = (pre_ibrt_role == ibrt_role) ? true:false;
    }
    return role_unified;
}

struct switch_background_req
{
    bt_bdaddr_t remote;
    uint8_t hold_state;
    uint32_t trigger_clk;
};

void app_ibrt_if_hold_background_switch()
{
    uint8_t current_playing_sco = app_bt_audio_get_curr_playing_sco();
    struct BT_DEVICE_T *curr_device = app_bt_get_device(current_playing_sco);
    struct switch_background_req switch_bg_req ={0};
    uint32_t trigger_btclk = app_bt_audio_trigger_switch_mute_streaming_sco(0);
    if ((curr_device == NULL) || (!curr_device->acl_is_connected))
    {
        LOG_I("%s curr_device is not valid", __func__);
        return;
    }
    if(!curr_device->hf_conn_flag)
    {
        LOG_I("(d%x)dont need switch background",current_playing_sco);
        return;
    }
    switch_bg_req.remote = curr_device->remote;
    switch_bg_req.hold_state = curr_device->hf_callheld;
    switch_bg_req.trigger_clk = trigger_btclk;
    if(tws_besaud_is_connected())
    {
        if(app_ibrt_conn_any_role_switch_running() || app_ibrt_if_role_unified())
        {
            LOG_I("disallow switch bg due to role mismatch");
            return;
        }
        tws_ctrl_send_cmd(APP_TWS_CMD_SWITCH_BACKGROUND, (uint8_t*)&switch_bg_req, sizeof(struct switch_background_req));
#if 0
        if(current_role == IBRT_MASTER)
        {
            tws_ctrl_send_cmd(APP_TWS_CMD_SWITCH_BACKGROUND, (uint8_t*)&switch_bg_req, sizeof(struct switch_background_req));
            //send tws cmd notify ready
        }
        else
        {
            app_ibrt_if_start_user_action_v2(app_bt_audio_get_curr_playing_sco(), IBRT_ACTION_HOLD_SWITCH, switch_bg_req.hold_state, 0);
            //tell master
        }
#endif
    }
    else
    {
        app_bt_audio_trigger_switch_mute_streaming_sco(bt_syn_get_curr_ticks(curr_device->acl_conn_hdl));
    }
}

void app_ibrt_if_switch_background_handler(uint8_t *p_buff, uint16_t length)
{
    struct switch_background_req *switch_bg_rsp = (struct switch_background_req*)p_buff;
    if(!switch_bg_rsp)
    {
        LOG_I("%s error", __func__);
    }
    uint8_t device_id = app_bt_get_device_id_byaddr(&switch_bg_rsp->remote);
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(curr_device && switch_bg_rsp->trigger_clk < bt_syn_get_curr_ticks(curr_device->acl_conn_hdl))
    {
        LOG_I("###switch failed time passed");
    }
    if(curr_device && curr_device->hf_callheld == switch_bg_rsp->hold_state
        && app_bt_audio_select_another_call_active_hfp(device_id) != BT_DEVICE_INVALID_ID){
        LOG_I("d%x start request switch background!!!", device_id);
        app_bt_audio_trigger_switch_mute_streaming_sco(switch_bg_rsp->trigger_clk);
    }
    LOG_I("%s",__func__);
}

void app_ibrt_if_profile_disconnect(uint8_t device_id, int profile_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct app_ibrt_profile_req profile_req = {false, {0}};

    if ((curr_device == NULL) || (!curr_device->acl_is_connected))
    {
        LOG_I("(d%x)%s profile %d curr_device:%p", device_id, __func__, profile_id, curr_device);
        return;
    }

    profile_req.remote = curr_device->remote;
    profile_req.profile_id = (app_ibrt_profile_id_enum)profile_id;

    if (profile_id == APP_IBRT_A2DP_PROFILE_ID)
    {
        curr_device->ibrt_disc_a2dp_profile_only = true;
    }

    if (tws_besaud_is_connected())
    {
        bool disLocally = app_tws_ibrt_mobile_link_connected(&curr_device->remote);
        LOG_I("(d%x)%s disLocal:%d", device_id, __func__, disLocally);

        if (disLocally)
        {
            if (app_ibrt_conn_is_profile_exchanged(&curr_device->remote))
            {
                if (app_ibrt_conn_any_role_switch_running()) {
                    app_ibrt_clear_profile_disconnect_protect(device_id, profile_id);
                    app_ibrt_start_profile_disconnect_delay_timer(device_id, profile_id);
                    return;
                } else {
                    app_ibrt_set_profile_disconnect_protect(device_id, profile_id);
                }
                tws_ctrl_send_cmd(APP_TWS_CMD_DISC_PROFILE_REQ, (uint8_t*)&profile_req, sizeof(struct app_ibrt_profile_req));
            }
            else
            {
                app_ibrt_if_disconnect_profile_handler((uint8_t*)&profile_req, sizeof(struct app_ibrt_profile_req));
            }
        } else {
            app_ibrt_if_start_user_action_v2(device_id, IBRT_ACTION_TELL_MASTER_DISC_PROFILE, profile_req.profile_id, 0);
        }
    }
    else
    {
        app_ibrt_if_disconnect_profile_handler((uint8_t*)&profile_req, sizeof(struct app_ibrt_profile_req));
    }
}

void app_ibrt_if_master_connect_hfp_profile(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (tws_besaud_is_connected() && app_tws_get_ibrt_role(&curr_device->remote) != IBRT_MASTER)
    {
        LOG_I("(d%x) %s not master", device_id, __func__);
        return;
    }
    app_ibrt_if_profile_connect(device_id, APP_IBRT_HFP_PROFILE_ID, 0);
}

void app_ibrt_if_master_connect_a2dp_profile(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (tws_besaud_is_connected() && app_tws_get_ibrt_role(&curr_device->remote) != IBRT_MASTER)
    {
        LOG_I("(d%x) %s not master", device_id, __func__);
        return;
    }
    app_ibrt_if_profile_connect(device_id, APP_IBRT_A2DP_PROFILE_ID, 0);
}

void app_ibrt_if_master_connect_avrcp_profile(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (tws_besaud_is_connected() && app_tws_get_ibrt_role(&curr_device->remote) != IBRT_MASTER)
    {
        LOG_I("(d%x) %s not master", device_id, __func__);
        return;
    }
    app_ibrt_if_profile_connect(device_id, APP_IBRT_AVRCP_PROFILE_ID, 0);
}

void app_ibrt_if_master_disconnect_hfp_profile(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (tws_besaud_is_connected() && app_tws_get_ibrt_role(&curr_device->remote) != IBRT_MASTER)
    {
        LOG_I("(d%x) %s not master", device_id, __func__);
        return;
    }
    app_ibrt_if_profile_disconnect(device_id, APP_IBRT_HFP_PROFILE_ID);
}

void app_ibrt_if_master_disconnect_a2dp_profile(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (tws_besaud_is_connected() && app_tws_get_ibrt_role(&curr_device->remote) != IBRT_MASTER)
    {
        LOG_I("(d%x) %s not master", device_id, __func__);
        return;
    }
    app_ibrt_if_profile_disconnect(device_id, APP_IBRT_A2DP_PROFILE_ID);
}

void app_ibrt_if_master_disconnect_avrcp_profile(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (tws_besaud_is_connected() && app_tws_get_ibrt_role(&curr_device->remote) != IBRT_MASTER)
    {
        LOG_I("(d%x) %s not master", device_id, __func__);
        return;
    }
    app_ibrt_if_profile_disconnect(device_id, APP_IBRT_AVRCP_PROFILE_ID);
}

void app_ibrt_if_connect_hfp_profile(uint8_t device_id)
{
    app_ibrt_if_profile_connect(device_id, APP_IBRT_HFP_PROFILE_ID, 0);
}

void app_ibrt_if_connect_a2dp_profile(uint8_t device_id)
{
    app_ibrt_if_profile_connect(device_id, APP_IBRT_A2DP_PROFILE_ID, 0);
}

void app_ibrt_if_connect_avrcp_profile(uint8_t device_id)
{
    app_ibrt_if_profile_connect(device_id, APP_IBRT_AVRCP_PROFILE_ID, 0);
}

void app_ibrt_if_disconnect_hfp_profile(uint8_t device_id)
{
    app_ibrt_if_profile_disconnect(device_id, APP_IBRT_HFP_PROFILE_ID);
}

void app_ibrt_if_disconnect_a2dp_profile(uint8_t device_id)
{
    app_ibrt_if_profile_disconnect(device_id, APP_IBRT_A2DP_PROFILE_ID);
}

void app_ibrt_if_disconnect_avrcp_profile(uint8_t device_id)
{
    app_ibrt_if_profile_disconnect(device_id, APP_IBRT_AVRCP_PROFILE_ID);
}

void app_ibrt_if_register_ibrt_cbs()
{
    static const app_ibrt_if_cbs_t app_ibrt_if_cbs  =
    {
        .keyboard_request_handler = app_ibrt_keyboard_request_handler_v2,
        .ui_perform_user_action = app_ibrt_ui_perform_user_action_v2,
        .conn_profile_handler = app_ibrt_if_connect_profile_handler,
        .disc_profile_handler = app_ibrt_if_disconnect_profile_handler,
        .disc_rfcomm_handler = app_ibrt_if_disconnect_rfcomm_handler,
        .ibrt_if_sniff_prevent_need = app_ibrt_if_customer_prevent_sniff,
        .tws_switch_prepare_needed = app_ibrt_if_tws_switch_prepare_needed,
        .tws_swtich_prepare = app_ibrt_if_tws_swtich_prepare,
        .switch_background_handler = app_ibrt_if_switch_background_handler,
    };

    app_ibrt_conn_reg_ibrt_if_cb(&app_ibrt_if_cbs);
}

bool app_ibrt_if_is_any_mobile_connected(void)
{
    return app_ibrt_conn_any_mobile_connected();
}

bool app_ibrt_if_is_maximum_mobile_dev_connected(void)
{
    return app_ibrt_conn_support_max_mobile_dev();
}

#ifdef IBRT_UI_V2
void app_ibrt_if_dump_ui_status()
{
    app_ui_dump_status();
}
#endif

btif_connection_role_t app_ibrt_if_get_tws_current_bt_role(void)
{
    btif_connection_role_t tws_bt_role = BTIF_BCR_UNKNOWN;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if(p_ibrt_ctrl->p_tws_remote_dev != NULL)
    {
        tws_bt_role = btif_me_get_current_role(p_ibrt_ctrl->p_tws_remote_dev);
    }
    return tws_bt_role;
}


/* API for Customer*/
AppIbrtStatus app_ibrt_if_get_a2dp_state(bt_bdaddr_t *addr, AppIbrtA2dpState *a2dp_state)
{
    uint8_t device_id = btif_me_get_device_id_from_addr(addr);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if(0xff != device_id){
        if(a2dp_state == NULL)
        {
            return APP_IBRT_IF_STATUS_ERROR_INVALID_PARAMETERS;
        }
        if(app_tws_ibrt_mobile_link_connected(addr) || app_tws_ibrt_slave_ibrt_link_connected(addr))
        {
            if(btif_a2dp_get_stream_state(curr_device->btif_a2dp_stream->a2dp_stream) > BT_A2DP_STREAM_STATE_IDLE)
            {
                *a2dp_state = (AppIbrtA2dpState)btif_a2dp_get_stream_state(curr_device->btif_a2dp_stream->a2dp_stream);
            }
            else
            {
                *a2dp_state = APP_IBRT_IF_A2DP_IDLE;
            }
        }
        else
        {
            *a2dp_state = APP_IBRT_IF_A2DP_IDLE;
        }
    }
    else{
        *a2dp_state = APP_IBRT_IF_A2DP_IDLE;
    }
    return APP_IBRT_IF_STATUS_SUCCESS;
}

AppIbrtStatus app_ibrt_if_get_avrcp_state(bt_bdaddr_t *addr,AppIbrtAvrcpState *avrcp_state)
{
    if(NULL == avrcp_state)
    {
        return APP_IBRT_IF_STATUS_ERROR_INVALID_PARAMETERS;
    }
    uint8_t device_id = btif_me_get_device_id_from_addr(addr);
    if(0xff != device_id)
    {
        struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
        if(BTIF_AVRCP_STATE_DISCONNECTED != curr_device->avrcp_conn_flag)
        {
            if(avrcp_state != NULL)
            {
                if(curr_device->avrcp_palyback_status == BTIF_AVRCP_MEDIA_PLAYING)
                {
                    *avrcp_state = APP_IBRT_IF_AVRCP_PLAYING;
                }
                else if(curr_device->avrcp_palyback_status == BTIF_AVRCP_MEDIA_PAUSED)
                {
                    *avrcp_state = APP_IBRT_IF_AVRCP_PAUSED;
                }
                else
                {
                    *avrcp_state = APP_IBRT_IF_AVRCP_CONNECTED;
                }
            }
        }
        else
        {
            *avrcp_state = APP_IBRT_IF_AVRCP_DISCONNECTED;
        }
    }
    else
    {
        *avrcp_state = APP_IBRT_IF_AVRCP_DISCONNECTED;
    }
    return APP_IBRT_IF_STATUS_SUCCESS;
}

AppIbrtStatus app_ibrt_if_get_hfp_state(bt_bdaddr_t *addr, AppIbrtHfpState *hfp_state)
{
    uint8_t device_id = btif_me_get_device_id_from_addr(addr);
    if(NULL== hfp_state)
    {
        return APP_IBRT_IF_STATUS_ERROR_INVALID_PARAMETERS;
    }
    if(0xff != device_id)
    {
        struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
        if(btif_get_hf_chan_state(curr_device->hf_channel) == BT_HFP_CHAN_STATE_OPEN)
        {
            *hfp_state = APP_IBRT_IF_HFP_SLC_OPEN;
        }
        else
        {
            *hfp_state = APP_IBRT_IF_HFP_SLC_DISCONNECTED;
        }
    }
    else
    {
        *hfp_state = APP_IBRT_IF_HFP_SLC_DISCONNECTED;
    }
    return APP_IBRT_IF_STATUS_SUCCESS;
}

AppIbrtStatus app_ibrt_if_get_hfp_call_status(bt_bdaddr_t *addr, AppIbrtCallStatus *call_status)
{
    if(call_status == NULL)
    {
        return APP_IBRT_IF_STATUS_ERROR_INVALID_PARAMETERS;
    }

    uint8_t device_id = btif_me_get_device_id_from_addr(addr);
    if(0xff != device_id)
    {
        struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
        if (curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN)
        {
            *call_status = APP_IBRT_IF_SETUP_INCOMMING;
        }
        else if(curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_OUT)
        {
            *call_status = APP_IBRT_IF_SETUP_OUTGOING;
        }
        else if(curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_ALERT)
        {
            *call_status = APP_IBRT_IF_SETUP_ALERT;
        }
        else if((curr_device->hfchan_call == BTIF_HF_CALL_ACTIVE) \
                && (curr_device->hf_audio_state == BTIF_HF_AUDIO_CON))
        {
            *call_status = APP_IBRT_IF_CALL_ACTIVE;
        }
        else if(curr_device->hf_callheld == BTIF_HF_CALL_HELD_ACTIVE)
        {
            *call_status = APP_IBRT_IF_HOLD;
        }
        else
        {
            *call_status = APP_IBRT_IF_NO_CALL;
        }
    }
    else
    {
        *call_status = APP_IBRT_IF_NO_CALL;
    }

    return APP_IBRT_IF_STATUS_SUCCESS;
}

uint8_t app_ibrt_if_get_mobile_connected_dev_list(bt_bdaddr_t *addr_list)
{
    return app_ibrt_conn_get_connected_mobile_list(addr_list);
}

bool app_ibrt_if_is_tws_role_switch_on(void)
{
    return app_ibrt_conn_any_role_switch_running();
}

void app_ibrt_if_tws_role_switch_request(void)
{
#ifdef TWS_RS_WITHOUT_MOBILE
    app_bt_start_custom_function_in_bt_thread((uint32_t)0, (uint32_t)0,
        (uint32_t)app_ibrt_conn_enhanced_rs);
#else
    app_bt_start_custom_function_in_bt_thread((uint32_t)0, (uint32_t)0,
        (uint32_t)app_ibrt_conn_all_dev_start_tws_role_switch);
#endif
}

bool app_ibrt_if_nvrecord_get_mobile_addr(bt_bdaddr_t mobile_addr_list[],uint8_t *count)
{
    bt_status_t result = BT_STS_SUCCESS;

    result = app_ibrt_nvrecord_get_mobile_addr(mobile_addr_list,count);
    if(result == BT_STS_FAILED)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void app_ibrt_if_nvrecord_delete_all_mobile_record(void)
{
    app_ibrt_nvrecord_delete_all_mobile_record();
}

bool app_ibrt_if_nvrecord_get_mobile_paired_dev_list(nvrec_btdevicerecord *nv_record,uint8_t *count)
{
    bt_status_t result = BT_STS_SUCCESS;

    result = app_ibrt_nvrecord_get_mobile_paired_dev_list(nv_record,count);
    if(result != BT_STS_FAILED)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool app_ibrt_if_is_tws_link_connected(void)
{
    return app_ibrt_conn_is_tws_connected();
}
#ifdef IBRT_UI_V2
uint8_t app_ibrt_if_get_connected_mobile_count(void)
{
    return app_ibrt_conn_get_local_connected_mobile_count();
}
#endif
uint8_t app_ibrt_if_is_in_freeman_mode(void)
{
    return app_ibrt_conn_is_freeman_mode();
}

ibrt_status_t app_ibrt_if_tws_disconnect_request(void)
{
    return app_ibrt_conn_tws_disconnect();
}

#if defined(USE_SAFE_DISCONNECT)
ibrt_status_t app_ibrt_if_all_disconnect_request(void)
{
    app_bt_call_func_in_bt_thread(0, 0, 0, 0,\
            (uint32_t) app_custom_ui_all_safe_disconnect);
    return IBRT_STATUS_SUCCESS;
}
#endif

ibrt_status_t app_ibrt_if_mobile_disconnect_request(const bt_bdaddr_t *addr,ibrt_post_func post_func)
{
    if (!app_ibrt_conn_mobile_link_connected(addr)) {
        return IBRT_STATUS_ERROR_NO_CONNECTION;
    }

    return app_ibrt_conn_remote_dev_disconnect_request(addr, post_func);
}

bool app_ibrt_if_is_tws_in_pairing_state(void)
{
    return app_ibrt_conn_is_tws_in_pairing_state();
}

uint8_t app_ibrt_if_get_support_max_remote_link()
{
    return app_ibrt_conn_support_max_mobile_dev();
}

void app_ibrt_if_a2dp_send_play(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_PLAY, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_a2dp_send_pause(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_PAUSE, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_a2dp_send_forward(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_FORWARD, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_a2dp_send_backward(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_BACKWARD, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_a2dp_send_volume_up(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_AVRCP_VOLUP, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_a2dp_send_volume_down(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_AVRCP_VOLDN, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_a2dp_send_set_abs_volume(uint8_t device_id, uint8_t volume)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_AVRCP_ABS_VOL, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_hf_create_audio_link(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_HFSCO_CREATE, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_hf_disc_audio_link(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_HFSCO_DISC, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_hf_call_redial(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_REDIAL, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_hf_call_answer(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_ANSWER, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_hf_call_hangup(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_HANGUP, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_hf_call_hold(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_HOLD_ACTIVE_CALL, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_hf_3way_hungup_incoming(uint8_t device_id)
{
    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_3WAY_HUNGUP_INCOMING, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}
void app_ibrt_if_hf_battery_report_ext(uint8_t level, uint8_t min, uint8_t max)
{
        if (level <= min)
        {
            level = 0;
        }
        else if (level >= max)
        {
            level = 9;
        }
        else
        {
            level = (uint8_t)((float)(level - min) / (max - min) * 9 + 0.5);
        }

        app_ibrt_if_hf_battery_report(level);
}

void app_ibrt_if_hf_3way_hungup_active_accept_incomming(uint8_t device_id)
{

    app_bt_call_func_in_bt_thread((uint32_t) device_id, (uint32_t) IBRT_ACTION_RELEASE_ACTIVE_ACCEPT_ANOTHER, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_hf_battery_report(uint8_t level)
{
    app_bt_call_func_in_bt_thread((uint8_t) level ,0,0,0,(int)app_hfp_battery_report);
}

void app_ibrt_if_set_local_volume_up(void)
{
    app_bt_call_func_in_bt_thread((uint32_t) BT_DEVICE_ID_1, (uint32_t) IBRT_ACTION_LOCAL_VOLUP, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_set_local_volume_down(void)
{
    app_bt_call_func_in_bt_thread((uint32_t) BT_DEVICE_ID_1, (uint32_t) IBRT_ACTION_LOCAL_VOLDN, 0, 0,\
        (uint32_t) app_ibrt_if_start_user_action_v2);
}

void app_ibrt_if_set_afh_assess_en(bool status)
{
    app_bt_call_func_in_bt_thread((uint32_t) status, 0, 0, 0,\
        (uint32_t) bt_drv_reg_op_afh_assess_en);
}

void app_ibrt_if_write_tws_link(const uint16_t connhandle)
{
    app_bt_call_func_in_bt_thread(connhandle, 0, 0, 0,\
        (uint32_t) btif_me_write_tws_link);
}

void app_ibrt_if_write_btpcm_en(bool en)
{
    app_bt_call_func_in_bt_thread(en, 0, 0, 0,\
        (uint32_t) btif_me_write_btpcm_en);
}

extern void bt_key_handle_func_key(enum APP_KEY_EVENT_T event);

void app_ibrt_if_key_event(enum APP_KEY_EVENT_T event)
{
    bt_key_handle_func_key(event);
}

static void app_ibrt_start_switching_sco(uint32_t playing_sco)
{
    app_ibrt_if_start_user_action_v2(playing_sco, IBRT_ACTION_SWITCH_SCO, 0, 0);
}

void app_ibrt_if_switch_streaming_sco(void)
{
    uint8_t playing_sco = app_bt_audio_get_curr_playing_sco();
    uint8_t sco_count = app_bt_audio_count_connected_sco();
    uint8_t another_call_active_device = BT_DEVICE_INVALID_ID;

    if (playing_sco == BT_DEVICE_INVALID_ID)
    {
        playing_sco = app_bt_audio_select_call_active_hfp();
    }

    if (playing_sco != BT_DEVICE_INVALID_ID)
    {
        another_call_active_device = app_bt_audio_select_another_device_to_create_sco(playing_sco);
    }

    LOG_I("%s playing sco d%x sco count %d active call d%x", __func__, playing_sco, sco_count, another_call_active_device);

    if (playing_sco != BT_DEVICE_INVALID_ID && (sco_count > 1 || another_call_active_device != BT_DEVICE_INVALID_ID))
    {
        app_bt_start_custom_function_in_bt_thread(playing_sco, (uint32_t)NULL, (uint32_t)(uintptr_t)app_ibrt_start_switching_sco);
    }
}

void app_ibrt_if_switch_streaming_a2dp(void)
{
    uint8_t playing_sco = app_bt_audio_get_curr_playing_sco();
    uint8_t sco_count = app_bt_audio_count_connected_sco();
    uint8_t a2dp_count = app_bt_audio_count_streaming_a2dp();

    LOG_I("%s %d %d %d", __func__, playing_sco, sco_count, a2dp_count);

    if (playing_sco == BT_DEVICE_INVALID_ID && sco_count == 0 && a2dp_count > 1)
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)NULL, (uint32_t)NULL, (uint32_t)(uintptr_t)app_ibrt_ui_start_perform_a2dp_switching);
    }
}

#ifdef TOTA_v2
#ifdef IS_TOTA_LOG_PRINT_ENABLED
char *tota_get_strbuf(void);
void app_ibrt_if_tota_printf(const char * format, ...)
{
    char *strbuf = tota_get_strbuf();
    va_list vlist;
    va_start(vlist, format);
    vsprintf(strbuf, format, vlist);
    va_end(vlist);
    app_ibrt_if_start_user_action_v2(BT_DEVICE_ID_1, IBRT_ACTION_SEND_TOTA_DATA, (uint32_t)(uintptr_t)strbuf, strlen(strbuf));
}
void app_ibrt_if_tota_printf_by_device(uint8_t device_id, const char * format, ...)
{
    char *strbuf = tota_get_strbuf();
    va_list vlist;
    va_start(vlist, format);
    vsprintf(strbuf, format, vlist);
    va_end(vlist);
    app_ibrt_if_start_user_action_v2(device_id, IBRT_ACTION_SEND_TOTA_DATA, (uint32_t)(uintptr_t)strbuf, strlen(strbuf));
}
#endif
#endif

#ifdef IBRT_UI_V2
const char* app_ibrt_if_ui_event_to_string(app_ui_evt_t type)
{
    return app_ui_event_to_string(type);
}

bool app_ibrt_if_event_has_been_queued(const bt_bdaddr_t* remote_addr,app_ui_evt_t event)
{
    return app_ui_event_has_been_queued(remote_addr,event);
}

app_ui_evt_t app_ibrt_if_get_remote_dev_active_event(const bt_bdaddr_t* remote_addr)
{
    return app_ui_get_active_event(remote_addr);
}
#endif

#ifdef PRODUCTION_LINE_PROJECT_ENABLED
void app_ibrt_if_test_open_box(void)
{
#if defined(IBRT_UI_V2)
    app_ui_get_config()->reconnect_tws_max_times = 0xFFFF;
    app_ui_get_config()->open_reconnect_tws_max_times = 0xFFFF;

    app_ibrt_if_event_entry(APP_UI_EV_CASE_OPEN);
#endif
}

void app_ibrt_if_test_close_box(void)
{
#if defined(IBRT_UI_V2)
    app_ui_get_config()->reconnect_tws_max_times = 0;
    app_ui_get_config()->open_reconnect_tws_max_times = 0;
    app_ibrt_if_event_entry(APP_UI_EV_CASE_CLOSE);
#endif
}
#endif

bool app_ibrt_if_post_custom_reboot_handler(void)
{
    if (hal_sw_bootmode_get()&HAL_SW_BOOTMODE_CUSTOM_OP1_AFTER_REBOOT)
    {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_CUSTOM_OP1_AFTER_REBOOT);
        LOG_I("%s [Enter pairing jay] ", __func__);
        app_ibrt_if_enter_freeman_pairing();
        return false;
    }
    else if (hal_sw_bootmode_get()&HAL_SW_BOOTMODE_CUSTOM_OP2_AFTER_REBOOT)
    {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_CUSTOM_OP2_AFTER_REBOOT);
        #ifdef IBRT_UI_V2
        app_ibrt_if_event_entry(APP_UI_EV_CASE_OPEN);
        #endif
        return false;
    }
    else if(hal_sw_bootmode_get()&HAL_SW_BOOTMODE_TEST_NORMAL_MODE)
    {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_NORMAL_MODE);
        app_ibrt_reconect_mobile_after_factorty_test();
        return false;
    }
    else
    {
        return true;
    }
}

ibrt_status_t app_ibrt_if_get_local_name(uint8_t* name_buf, uint8_t max_size)
{
    const char *name = (char *)factory_section_get_bt_name();

    int name_len = name ? strlen(name) : 0;
    if (name_len && max_size)
    {
        name_len = (name_len < max_size - 1) ? name_len : (max_size - 1);
        memcpy(name_buf, name, name_len);
        name_buf[name_len] = 0;
        return IBRT_STATUS_SUCCESS;
    }
    else
    {
        return IBRT_STATUS_ERROR_INVALID_STATE;
    }
}

void app_ibrt_if_get_mobile_bt_link_key(uint8_t *linkKey1, uint8_t *linkKey2)
{
    btif_device_record_t record;
    struct BT_DEVICE_T *curr_device = NULL;
    bt_bdaddr_t *mobile_addr = NULL;
    bt_status_t status = BT_STS_FAILED;
    uint8_t *linkKeyTemp = NULL;

    memset(linkKey1, 0x00, 16);
    memset(linkKey2, 0x00, 16);

    for (uint8_t i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        mobile_addr = &curr_device->remote;
        status = ddbif_find_record(mobile_addr, &record);

        if ((NULL != mobile_addr) &&  (BT_STS_SUCCESS == status)) {
            linkKeyTemp = (i == BT_DEVICE_ID_1) ? linkKey1 : linkKey2;
            memcpy(linkKeyTemp, record.linkKey, 16);
        }
    }
}

void app_ibrt_if_get_tws_bt_link_key(uint8_t *linkKey)
{
    btif_device_record_t record;
    bt_bdaddr_t *tws_addr = NULL;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    memset(linkKey, 0x00, 16);

    tws_addr = btif_me_get_remote_device_bdaddr(p_ibrt_ctrl->p_tws_remote_dev);

    if ((tws_addr != NULL) && ddbif_find_record(tws_addr, &record) == BT_STS_SUCCESS) {
        memcpy(linkKey, record.linkKey, 16);
    }
}

bool app_ibrt_if_get_all_paired_bt_link_key(ibrt_if_paired_bt_link_key_info *linkKey)
{
    if (linkKey == NULL) {
        LOG_I("%s null pointer,pls check", __func__);
        return false;
    }

    bt_status_t retStatus = BT_STS_SUCCESS;
    btif_device_record_t record;
    int32_t index= 0;

    memset(linkKey, 0x00, sizeof(ibrt_if_paired_bt_link_key_info));
    linkKey->pairedDevNum = nv_record_get_paired_dev_count();

    for (index = 0; index < linkKey->pairedDevNum; index++)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS == retStatus)
        {
            memcpy(&linkKey->linkKey[index].btAddr, record.bdAddr.address, BTIF_BD_ADDR_SIZE);
            memcpy(&linkKey->linkKey[index].linkKey, record.linkKey , 16);
        }
    }
    return true;
}

bool app_ibrt_if_get_all_paired_ble_ltk(ibrt_if_paired_ble_ltk_info* leLTK)
{
    if (leLTK == NULL) {
        LOG_I("%s fail! param pointer is null", __func__);
        return false;
    }

    memset(leLTK, 0x00, sizeof(ibrt_if_paired_ble_ltk_info));

    uint8_t nvBleInfoListNum = nv_record_get_paired_ble_dev_info_list_num();
    if (nvBleInfoListNum == 0)
    {
        return false;
    }

    for (int32_t index = 0; index < nvBleInfoListNum; index++)
    {
        BleDevicePairingInfo nvBleInfo;
        bool isGetInfoDone= nv_record_get_ble_pairing_info_through_list_index(index, &nvBleInfo);
        if (isGetInfoDone)
        {
            memcpy(&(leLTK->bleLtk[index].bleAddr), &(nvBleInfo.peer_addr.addr), BLE_ADDR_SIZE);
            memcpy(&(leLTK->bleLtk[index].bleLTK), &(nvBleInfo.LTK), BLE_LTK_SIZE);
            leLTK->blePairedDevNum++;
        }
    }

    return true;
}

bool app_ibrt_if_is_audio_active(uint8_t device_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    if (device_id > BT_DEVICE_NUM) {
        return false;
    }

    if(!curr_device) {
        return false;
    }

    return (curr_device->a2dp_streamming || \
            app_bt_audio_get_curr_playing_sco() == device_id);
}

bool app_ibrt_if_get_active_device(bt_bdaddr_t* device)
{
    bool addr_valid = false;
    struct BT_DEVICE_T* curr_device = NULL;

    if(!device)
    {
        return false;
    }

    for(uint8_t index = 0;index < BT_DEVICE_NUM;index++)
    {
        if(app_ibrt_if_is_audio_active(index))
        {
            curr_device = app_bt_get_device(index);
            if(!curr_device)
            {
                return false;
            }

            memcpy((uint8_t*)&device->address[0],(uint8_t*)&curr_device->remote.address[0],BD_ADDR_LEN);
            LOG_I("custom_ui:d%x is service active ",index);
            addr_valid = true;
        }
    }

    if (addr_valid == false)
    {
        uint8_t device_id = app_bt_audio_select_connected_device();
        if (device_id != BT_DEVICE_INVALID_ID)
        {
            LOG_I("custom_ui:select d%x as service active ",device_id);
            *device = app_bt_get_device(device_id)->remote;
            addr_valid = true;
        }
    }

    return addr_valid;
}

static void app_ibrt_if_update_mobile_link_qos_handler(uint8_t device_id, uint8_t tpoll_slot)
{
    struct BT_DEVICE_T* pBtdev = app_bt_get_device(device_id);
    bt_status_t ret = BT_STS_SUCCESS;

    if (pBtdev && (pBtdev->acl_is_connected) && (app_ibrt_conn_get_ui_role() != TWS_UI_SLAVE))
    {
        ret = btif_me_qos_setup_with_tpoll_generic(pBtdev->acl_conn_hdl, tpoll_slot, QOS_SETUP_SERVICE_TYPE_BEST_EFFORT);
        LOG_I("btif_me_qos_setup_with_tpoll_generic:handle:%04x %d", pBtdev->acl_conn_hdl, ret);
    }
}

void app_ibrt_if_update_mobile_link_qos(uint8_t device_id, uint8_t tpoll_slot)
{
    app_bt_call_func_in_bt_thread((uint32_t)device_id, (uint32_t)tpoll_slot,
        0, 0, (uint32_t) app_ibrt_if_update_mobile_link_qos_handler);
}

bool app_ibrt_if_is_tws_addr(const uint8_t* pBdAddr)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (!memcmp(pBdAddr, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) ||
        !memcmp(pBdAddr, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static osMutexId setConnectivityLogMutexId = NULL;
osMutexDef(setConnectivityLog);
static reportConnectivityLogCallback_t ibrt_if_report_connectivity_log_callback = NULL;
static reportDisconnectReasonCallback_t ibrt_if_report_disconnect_reason_callback = NULL;
static connectivity_log_t Connectivity_log;
static disconnect_reason_t disconnect_reason;
static uint32_t bt_clkoffset = 0;
connectivity_log_report_intersys_api ibrt_if_report_intersys_callback = NULL;

void app_ibrt_if_report_connectivity_log_init(void)
{
    if (!setConnectivityLogMutexId){
        setConnectivityLogMutexId = osMutexCreate((osMutex(setConnectivityLog)));
    }

    ibrt_if_report_intersys_callback = app_ibrt_if_acl_data_packet_check_handler;

    memset(&Connectivity_log, 0, sizeof(connectivity_log_t));
    Connectivity_log.retriggerType = RETRIGGER_MAX;
    LOG_I("%s, %d", __func__, sizeof(connectivity_log_t));
}

void app_ibrt_if_register_report_connectivity_log_callback(reportConnectivityLogCallback_t callback)
{
    ibrt_if_report_connectivity_log_callback = callback;
}

void app_ibrt_if_register_report_disonnect_reason_callback(reportDisconnectReasonCallback_t callback)
{
    ibrt_if_report_disconnect_reason_callback = callback;
}

void app_ibrt_if_save_bt_clkoffset(uint32_t clkoffset, uint8_t device_id)
{
    if (BT_DEVICE_NUM > device_id)
    {
        bt_clkoffset = clkoffset;
    }
}

void app_ibrt_if_disconnect_event(btif_remote_device_t *rem_dev, bt_bdaddr_t *addr, uint8_t disconnectReason, uint8_t activeConnection)
{
    uint8_t device_id = btif_me_get_device_id_from_rdev(rem_dev);
    uint16_t conhandle = INVALID_HANDLE;

    conhandle = btif_me_get_remote_device_hci_handle(rem_dev);
    bt_drv_reg_op_get_pkt_ts_rssi(conhandle, (CLKN_SER_NUM_T *)disconnect_reason.rxClknRssi);
    disconnect_reason.lcState = bt_drv_reg_op_force_get_lc_state(conhandle);

    if (BT_DEVICE_NUM > device_id)
    {
        for (uint8_t i = 0; i < RECORD_RX_NUM; i++)
        {
            disconnect_reason.rxClknRssi[i].clkn = (disconnect_reason.rxClknRssi[i].clkn + bt_clkoffset) & 0xFFFFFFFF;
        }
        bt_clkoffset = 0;
    }

    disconnect_reason.disconnectObject = device_id;
    disconnect_reason.disconnectReson = disconnectReason;
    disconnect_reason.activeConnection = activeConnection;
    memcpy(disconnect_reason.addr, addr, 6);

    if (ibrt_if_report_disconnect_reason_callback)
    {
        ibrt_if_report_disconnect_reason_callback(&disconnect_reason);
    }

    LOG_I("%s, Device_id[d%x] Disconnect_reson : 0x%02x", __func__, disconnect_reason.disconnectObject, disconnect_reason.disconnectReson);
    LOG_I("REMOTE DEVICE ADDR IS: ");
    DUMP8("%02x ", addr, BT_ADDR_OUTPUT_PRINT_NUM);
}

void app_ibrt_if_save_curr_mode_to_disconnect_info(uint8_t currMode, uint32_t interval, bt_bdaddr_t *addr)
{
    uint8_t device_id = btif_me_get_device_id_from_addr(addr);

    if (BT_DEVICE_NUM > device_id)
    {
        disconnect_reason.mobileCurrMode[device_id] = currMode;
        disconnect_reason.mobileSniffInterval[device_id] = interval;
    }
    else if (BT_DEVICE_INVALID_ID == device_id)
    {
        disconnect_reason.twsCurrMode = currMode;
        disconnect_reason.twsSniffInterval = interval;
    }
}

static void app_ibrt_if_get_newest_rssi_and_chlMap(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (NULL == curr_device)
    {
        return;
    }

    ibrt_mobile_info_t *p_mobile_info = NULL;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    rx_agc_t link_agc_info = {0};
    //uint8_t chlMap[10] = {0};
    //char strChlMap[32];
    //int pos = 0;

    p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
    osMutexWait(setConnectivityLogMutexId, osWaitForever);

    if (p_mobile_info)
    {
        if (p_mobile_info->p_mobile_remote_dev)
        {
            if (bt_drv_reg_op_read_rssi_in_dbm(btif_me_get_remote_device_hci_handle(p_mobile_info->p_mobile_remote_dev),&link_agc_info))
            {
                Connectivity_log.mobile_rssi[device_id] = link_agc_info.rssi;
                LOG_I("[MOBILE] [d%x] RSSI = %d, RX gain = %d",
                  device_id,
                  link_agc_info.rssi,
                  link_agc_info.rxgain);
                  LOG_I("MOBILE ADDR IS: ");
                  DUMP8("%02x ", curr_device->remote.address, BT_ADDR_OUTPUT_PRINT_NUM);
            }

#ifdef __UPDATE_CHNMAP__
            if (0 == bt_drv_reg_op_acl_chnmap(btif_me_get_remote_device_hci_handle(p_mobile_info->p_mobile_remote_dev), chlMap, sizeof(chlMap)))
            {
                memcpy(&Connectivity_log.mobileChlMap[device_id], chlMap, 10);
                for (uint8_t i = 0; i < 10; i++)
                {
                    pos += format_string(strChlMap + pos, sizeof(strChlMap) - pos, "%02x ", Connectivity_log.mobileChlMap[device_id][i]);
                }
                LOG_I("%s chlMap %s", "[MOBILE] chlMap", strChlMap);
            }
#endif
        }
    }

    if (p_ibrt_ctrl->p_tws_remote_dev)
    {
        if (bt_drv_reg_op_read_rssi_in_dbm(btif_me_get_remote_device_hci_handle(p_ibrt_ctrl->p_tws_remote_dev),&link_agc_info))
        {
            Connectivity_log.tws_rssi = link_agc_info.rssi;
            bt_bdaddr_t *remote = btif_me_get_remote_device_bdaddr(p_ibrt_ctrl->p_tws_remote_dev);
            if (remote)
            {
                LOG_I("[TWS] RSSI = %d, RX gain = %d",
                      Connectivity_log.tws_rssi,
                      link_agc_info.rxgain);
                LOG_I("PEER TWS ADDR IS: ");
                DUMP8("%02x ",remote->address, BT_ADDR_OUTPUT_PRINT_NUM);
            }
        }
    }

    osMutexRelease(setConnectivityLogMutexId);
}

void app_ibrt_if_update_rssi_info(const char* tag, rx_agc_t link_agc_info, uint8_t device_id)
{
    osMutexWait(setConnectivityLogMutexId, osWaitForever);

    if (!memcmp(tag, "PEER TWS", sizeof(*tag)))
    {
        Connectivity_log.tws_rssi = link_agc_info.rssi;
    }
    else if(!memcmp(tag, "MASTER MOBILE", sizeof(*tag)) || !memcmp(tag, "SNOOP MOBILE", sizeof(*tag)))
    {
        Connectivity_log.mobile_rssi[device_id] = link_agc_info.rssi;
    }

    osMutexRelease(setConnectivityLogMutexId);
}

void app_ibrt_if_update_chlMap_info(const char* tag, uint8_t *chlMap, uint8_t device_id)
{
#ifdef __UPDATED_CHNMAP__
    if(!memcmp(tag, "MASTER MOBILE", sizeof(*tag)) || !memcmp(tag, "SNOOP MOBILE", sizeof(*tag)))
    {
        osMutexWait(setConnectivityLogMutexId, osWaitForever);
        memcpy(&Connectivity_log.mobileChlMap[device_id], chlMap, 10);
        osMutexRelease(setConnectivityLogMutexId);
    }
#endif
}

static const char* app_ibrt_if_retrigger_type_to_str(uint8_t retrigger_type)
{
    switch(retrigger_type)
    {
        case RETRIGGER_BY_ROLE_SWITCH:
            return "RETRIGGER_BY_ROLE_SWITCH";
        case RETRIGGER_BY_DECODE_ERROR:
            return "RETRIGGER_BY_DECODE_ERROR";
        case RETRIGGER_BY_DECODE_STATUS_ERROR:
            return "RETRIGGER_BY_DECODE_STATUS_ERROR";
        case RETRIGGER_BY_ROLE_MISMATCH:
            return "RETRIGGER_BY_ROLE_MISMATCH";
        case RETRIGGER_BY_TRIGGER_FAIL:
            return "RETRIGGER_BY_TRIGGER_FAIL";
        case RETRIGGER_BY_L_R_SYNC_DETECT:
            return "RETRIGGER_BY_L_R_SYNC_DETECT";
        case RETRIGGER_BY_SYNCHRONIZE_CNT_LIMIT:
            return "RETRIGGER_BY_SYNCHRONIZE_CNT_LIMIT";
        case RETRIGGER_BY_LOW_BUFFER:
            return "RETRIGGER_BY_LOW_BUFFER";
        case RETRIGGER_BY_SEQ_MISMATCH:
            return "RETRIGGER_BY_SEQ_MISMATCH";
        case RETRIGGER_BY_AUTO_SYNC_NO_SUPPORT:
            return "RETRIGGER_BY_AUTO_SYNC_NO_SUPPORT";
        case RETRIGGER_BY_PLAYER_NOT_ACTIVE:
            return "RETRIGGER_BY_PLAYER_NOT_ACTIVE";
        case RETRIGGER_BY_STATUS_ERROR:
            return "RETRIGGER_BY_STATUS_ERROR";
        case RETRIGGER_BY_STREAM_RESTART:
            return "RETRIGGER_BY_STREAM_RESTART";
        case RETRIGGER_BY_SYNC_MISMATCH:
            return "RETRIGGER_BY_SYNC_MISMATCH";
        case RETRIGGER_BY_SYNC_FAIL:
            return "RETRIGGER_BY_SYNC_FAIL";
        case RETRIGGER_BY_SYS_BUSY:
            return "RETRIGGER_BY_SYS_BUSY";
        case RETRIGGER_BY_SYNC_TARGET_CNT_ERROR:
            return "RETRIGGER_BY_SYNC_TARGET_CNT_ERROR";
        case RETRIGGER_BY_AI_STREAM:
            return "RETRIGGER_BY_AI_STREAM";
        case RETRIGGER_BY_UNKNOW:
            return "RETRIGGER_BY_UNKNOW";
        case RETRIGGER_MAX:
            return "RETRIGGER_MAX";
        default:
            ASSERT(0,"UNKNOWN retrigger_type = 0x%x",retrigger_type);
            break;
    }
}

uint32_t app_ibrt_if_get_curr_ticks(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (NULL == curr_device)
    {
        return 0;
    }

    uint32_t curr_ticks = 0;
    uint16_t conhandle = INVALID_HANDLE;

    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote))
    {
        conhandle = APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote);
        curr_ticks = bt_syn_get_curr_ticks(conhandle);
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        conhandle = APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote);
        curr_ticks = bt_syn_get_curr_ticks(conhandle);
    }

    return curr_ticks;
}

void app_ibrt_if_report_audio_retrigger(uint8_t retriggerType)
{
    uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (NULL == curr_device)
    {
        return;
    }

    uint32_t curr_ticks = app_ibrt_if_get_curr_ticks(device_id);
    app_ibrt_if_get_newest_rssi_and_chlMap(device_id);

    LOG_I("[%s] RETRIGGER TYPE[%s]",__func__,app_ibrt_if_retrigger_type_to_str(retriggerType));
    osMutexWait(setConnectivityLogMutexId, osWaitForever);
    if (btif_me_is_in_sniff_mode(&curr_device->remote))
    {
        Connectivity_log.retriggerType = RETRIGGER_BY_IN_SNIFF;
    }
    else
    {
        Connectivity_log.retriggerType = retriggerType;
    }
    Connectivity_log.clock = curr_ticks;
    LOG_I("Retrigger Curr ticke = 0x%08x", Connectivity_log.clock);

    osMutexRelease(setConnectivityLogMutexId);
}

void app_ibrt_if_update_link_monitor_info(uint8_t *ptr)
{
    uint32_t *p = (uint32_t *)ptr + HEC_ERR;
    uint8_t tmp[sizeof(Connectivity_log.ll_monitor_info)/sizeof(uint8_t) - 14];
    //uint8_t device_id = app_bt_audio_get_curr_a2dp_device();

    for (uint8_t i = 0; i < (sizeof(Connectivity_log.ll_monitor_info)/sizeof(uint8_t) - 14); i++)
    {
        tmp[i] = ((uint8_t *)p)[0];
        p += 1;
    }

    osMutexWait(setConnectivityLogMutexId, osWaitForever);

    Connectivity_log.ll_monitor_info.rev_fa_cnt = 0;
    memcpy(&Connectivity_log.ll_monitor_info, tmp, sizeof(tmp));

    p += 6;
    Connectivity_log.ll_monitor_info.rx_seq_err_cnt = ((uint8_t *)p)[0];

    p = (uint32_t *)ptr + 12;
    for (uint8_t i = 12; i <= 23; i++)
    {
        Connectivity_log.ll_monitor_info.rev_fa_cnt += ((uint8_t *)p)[0];
        if (i == RX_DM1)
        {
            Connectivity_log.ll_monitor_info.rx_dm1 = ((uint8_t *)p)[0];
        }
        else if (i == RX_2DH1)
        {
            Connectivity_log.ll_monitor_info.rx_2dh1 = ((uint8_t *)p)[0];
        }
        else if (i == RX_2DH3)
        {
            Connectivity_log.ll_monitor_info.rx_2dh3 = ((uint8_t *)p)[0];
        }
        else if (i == RX_2DH5)
        {
            Connectivity_log.ll_monitor_info.rx_2dh5 = ((uint8_t *)p)[0];
        }
        p++;
    }
    Connectivity_log.ll_monitor_info.last_ticks = Connectivity_log.ll_monitor_info.curr_ticks;
    Connectivity_log.ll_monitor_info.curr_ticks = app_ibrt_if_get_curr_ticks(app_bt_audio_get_curr_a2dp_device());

    if (RETRIGGER_MAX != Connectivity_log.retriggerType)
    {
        if (ibrt_if_report_connectivity_log_callback)
        {
            ibrt_if_report_connectivity_log_callback(&Connectivity_log);
        }
        Connectivity_log.retriggerType = RETRIGGER_MAX;
    }

    osMutexRelease(setConnectivityLogMutexId);

    //LOG_I("%s curr tick 0x%x", __func__, Connectivity_log.ll_monitor_info.curr_ticks);
    //DUMP8("0x%02x ", &Connectivity_log.ll_monitor_info, sizeof(Connectivity_log.ll_monitor_info)/sizeof(uint8_t));
}

static uint32_t lastAclPacketTimeMs = 0;
static uint32_t lasttwsAclPacketTimeMs = 0;

void app_ibrt_if_reset_acl_data_packet_check(void)
{
    osMutexWait(setConnectivityLogMutexId, osWaitForever);

    memset(&Connectivity_log.acl_packet_interval, 0, sizeof(Connectivity_log.acl_packet_interval));
    lastAclPacketTimeMs = 0;
    osMutexRelease(setConnectivityLogMutexId);
}

static void app_ibrt_if_fill_acl_data_packet_interval(uint32_t intervalMs, int8_t currRssi, uint8_t lastRssi)
{
    char strAclPacketInterval[50];
    int len = 0;
    int8_t index = 0;
    uint32_t currBtclk = app_ibrt_if_get_curr_ticks(app_bt_audio_get_curr_a2dp_device());

    osMutexWait(setConnectivityLogMutexId, osWaitForever);

    for (index = TOP_ACL_PACKET_INTERVAL_CNT - 1; index > 0; index--)
    {
        memmove((uint8_t *)&Connectivity_log.acl_packet_interval[index],
                (uint8_t *)&Connectivity_log.acl_packet_interval[index-1],
                sizeof(acl_packet_interval_t));
    }
    index = 0;
    Connectivity_log.acl_packet_interval[index].AclPacketInterval = intervalMs;
    Connectivity_log.acl_packet_interval[index].currentReceivedRssi = currRssi;
    Connectivity_log.acl_packet_interval[index].lastReceivedRssi = lastRssi;
    Connectivity_log.acl_packet_interval[index].AclPacketBtclock = currBtclk;

    osMutexRelease(setConnectivityLogMutexId);

    for (index = 0;index < TOP_ACL_PACKET_INTERVAL_CNT;index++)
    {
        len += format_string(strAclPacketInterval + len, sizeof(strAclPacketInterval) - len, "%08x ",
                        Connectivity_log.acl_packet_interval[index].AclPacketInterval);
    }

    LOG_I("ACL DATA INTERVAL %s, curr ticks is %d curr btclk is 0x%x", strAclPacketInterval, lastAclPacketTimeMs, currBtclk);
}

void app_ibrt_if_check_acl_data_packet_during_a2dp_streaming(void)
{
    uint32_t passedTimerMs = 0;
    uint32_t currentTimerMs = GET_CURRENT_MS();

    if (0 == lastAclPacketTimeMs)
    {
        lastAclPacketTimeMs = currentTimerMs;
    }
    else
    {
        if (currentTimerMs >= lastAclPacketTimeMs)
        {
            passedTimerMs = currentTimerMs - lastAclPacketTimeMs;
        }
        else
        {
            passedTimerMs = TICKS_TO_MS(0xFFFFFFFF) - lastAclPacketTimeMs + currentTimerMs + 1;
        }

        lastAclPacketTimeMs = currentTimerMs;
    }

    uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (NULL == curr_device)
    {
        return;
    }

    ibrt_mobile_info_t *p_mobile_info = NULL;
    rx_agc_t link_agc_info = {0};
    static int8_t currRssi = 0;
    static int8_t lastRssi = 0;
    p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);

    if (p_mobile_info)
    {
        if (p_mobile_info->p_mobile_remote_dev)
        {
            if (bt_drv_reg_op_read_rssi_in_dbm(btif_me_get_remote_device_hci_handle(p_mobile_info->p_mobile_remote_dev),&link_agc_info))
            {
                lastRssi = currRssi;
                currRssi = link_agc_info.rssi;
            }
        }
    }

    if (passedTimerMs >= ACL_PACKET_INTERVAL_THRESHOLD_MS)
    {
        LOG_I("%s passedTimerMs = %d ", __func__, passedTimerMs);
        app_ibrt_if_fill_acl_data_packet_interval(passedTimerMs, currRssi, lastRssi);
    }
}

void app_ibrt_if_reset_tws_acl_data_packet_check(void)
{
    //When needed, use it
}

static void app_ibrt_if_fill_tws_acl_data_packet_interval(uint32_t intervalMs)
{
    //When needed, use it
}

void app_ibrt_if_check_tws_acl_data_packet(void)
{
    uint32_t passedTimerMs = 0;
    uint32_t currentTimerMs = GET_CURRENT_MS();

    if (0 == lasttwsAclPacketTimeMs)
    {
        lasttwsAclPacketTimeMs = currentTimerMs;
    }
    else
    {
        if (currentTimerMs >= lasttwsAclPacketTimeMs)
        {
            passedTimerMs = currentTimerMs - lasttwsAclPacketTimeMs;
        }
        else
        {
            passedTimerMs = TICKS_TO_MS(0xFFFFFFFF) - lasttwsAclPacketTimeMs + currentTimerMs + 1;
        }

        lasttwsAclPacketTimeMs = currentTimerMs;
    }

    if (passedTimerMs >= ACL_PACKET_INTERVAL_THRESHOLD_MS)
    {
        LOG_I("%s passedTimerMs = %d", __func__, passedTimerMs);
        app_ibrt_if_fill_tws_acl_data_packet_interval(passedTimerMs);
    }
}

bool app_ibrt_if_is_mobile_connhandle(uint16_t connhandle)
{
    struct BT_DEVICE_T *curr_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    bt_bdaddr_t *mobile_addr = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);

        if (curr_device)
        {
            mobile_addr = &curr_device->remote;
            p_mobile_info = app_ibrt_conn_get_mobile_info_by_addr(mobile_addr);
            if (p_mobile_info)
            {
                if (p_mobile_info->mobile_conhandle == connhandle || p_mobile_info->ibrt_conhandle == connhandle)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
    return false;
}

bool app_ibrt_if_is_tws_connhandle(uint16_t connhandle)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (p_ibrt_ctrl->tws_conhandle == connhandle)
    {
        return true;
    }

    return false;
}

void app_ibrt_if_acl_data_packet_check_handler(uint8_t *data)
{
    if ((0x02 == data[0]) && (0x80&data[1]))
    {
        if (a2dp_is_music_ongoing() && app_ibrt_if_is_mobile_connhandle(data[1]))
        {
            app_ibrt_if_check_acl_data_packet_during_a2dp_streaming();
        }
        else if(app_ibrt_if_is_tws_connhandle((uint16_t)data[1]))
        {
            app_ibrt_if_check_tws_acl_data_packet();
        }
    }
}

static uint8_t LatestNewlyPairedMobileBtAddr[BTIF_BD_ADDR_SIZE] = {0, 0, 0, 0, 0, 0};

static void app_ibrt_if_new_mobile_paired_callback(const uint8_t* btAddr)
{
    if (app_ibrt_if_is_tws_addr(btAddr))
    {
        LOG_I("New device paired: skip, local address!");
    }
    else
    {
        LOG_I("New mobile paired: %02x:%02x:*:*:*:%02x", btAddr[0], btAddr[1], btAddr[5]);
        memcpy(LatestNewlyPairedMobileBtAddr, btAddr, sizeof(LatestNewlyPairedMobileBtAddr));

#if defined (BLE_AUDIO_DOLPHIN_COMPATIBLE_SUPPORT) && (BLE_AUDIO_ENABLED > 0)
        if (!app_ibrt_conn_mobile_link_connected((bt_bdaddr_t*)btAddr))
        {
            LOG_I("Slave mobile connected:");
            return;
        }

        BleDevicePairingInfo ble_record;
        BLE_ADDR_INFO_T ble_connect_addr;
        ble_connect_addr.addr_type = ADDR_PUBLIC;
        memcpy(ble_connect_addr.addr, btAddr, BD_ADDR_LEN);
        if(nv_record_blerec_get_paired_dev_from_addr(&ble_record, &ble_connect_addr))
        {
            LOG_I("%s-->%d", __func__, __LINE__);
            if (!ble_audio_is_mobile_link_connected((ble_bdaddr_t *)&ble_connect_addr)) {
                LOG_I("The LE link not connected, delete old record");
                nv_record_ble_del_nv_data_entry((uint8_t *)btAddr);
                app_ui_notify_bt_nv_recored_changed((bt_bdaddr_t *)btAddr);
            }
        }
        else
        {
            LOG_I("%s-->%d", __func__, __LINE__);
        }
#endif
    }
}

static void app_ibrt_if_new_tws_paired_callback(const uint8_t* btAddr)
{
}

uint8_t* app_ibrt_if_get_latest_paired_mobile_bt_addr(void)
{
    return LatestNewlyPairedMobileBtAddr;
}

void app_ibrt_if_clear_newly_paired_mobile(void)
{
    memset(LatestNewlyPairedMobileBtAddr, 0, sizeof(LatestNewlyPairedMobileBtAddr));
}

void app_ibrt_if_init_newly_paired_mobile_callback(void)
{
    nv_record_bt_device_register_newly_paired_device_callback(NEW_DEVICE_CB_MOBILE,
        app_ibrt_if_new_mobile_paired_callback);
}

void app_ibrt_if_init_newly_paired_tws_callback(void)
{
    nv_record_bt_device_register_newly_paired_device_callback(NEW_DEVICE_CB_TWS,
        app_ibrt_if_new_tws_paired_callback);
}

uint32_t app_ibrt_if_get_tws_mtu_size(void)
{
    return tws_ctrl_get_mtu_size();
}

static BSIR_event_callback_t ibrt_if_bsir_command_event_callback = NULL;
void app_ibrt_if_register_BSIR_command_event_callback(BSIR_event_callback_t callback)
{
    ibrt_if_bsir_command_event_callback = callback;
}

void app_ibrt_if_BSIR_command_event(uint8_t is_in_band_ring)
{
    if (ibrt_if_bsir_command_event_callback)
    {
        ibrt_if_bsir_command_event_callback(is_in_band_ring);
    }
}

#ifdef IBRT_CORE_V2_ENABLE
void app_ibrt_if_reg_disallow_role_switch_callback(APP_IBRT_IF_ROLE_SWITCH_IND cb)
{
    app_ibrt_conn_reg_disallow_role_switch_callback(cb);
}
#endif

static sco_disconnect_event_callback_t ibrt_if_sco_disconnect_event_callback = NULL;
void app_ibrt_if_register_sco_disconnect_event_callback(sco_disconnect_event_callback_t callback)
{
    ibrt_if_sco_disconnect_event_callback = callback;
}

void app_ibrt_if_sco_disconnect(uint8_t *addr, uint8_t disconnect_rseason)
{
    if (ibrt_if_sco_disconnect_event_callback)
    {
        ibrt_if_sco_disconnect_event_callback(addr, disconnect_rseason);
    }
}

void app_ibrt_if_a2dp_set_delay(a2dp_stream_t *Stream, uint16_t delayMs)
{
    app_bt_call_func_in_bt_thread((uint32_t) Stream, (uint32_t) delayMs, 0, 0,\
            (uint32_t) btif_a2dp_set_sink_delay);
}

#ifdef IBRT_UI_V2
bool app_ibrt_if_is_earbud_in_pairing_mode(void)
{
    return app_ui_in_pairing_mode();
}

void app_ibrt_if_choice_mobile_connect(const bt_bdaddr_t* bt_addr)
{
    app_ui_choice_mobile_connect(bt_addr);
}

void app_ibrt_if_change_ui_mode(bool enable_leaudio, bool enable_multipoint, const bt_bdaddr_t *addr)
{
    app_ui_change_mode_ext(enable_leaudio, enable_multipoint, addr);
}
#endif

void app_ibrt_if_enable_hfp_voice_assistant(bool isEnable)
{
    app_bt_call_func_in_bt_thread((uint32_t)isEnable, 0, 0, 0,\
            (uint32_t) app_hfp_siri_voice);
}

void app_ibrt_if_disonnect_rfcomm(bt_spp_channel_t *spp_chan, uint8_t reason)
{
    struct app_ibrt_rfcomm_req rfcomm_req = {0, 0};

    rfcomm_req.reason = reason;
    rfcomm_req.sppdev_ptr = (uint32_t)spp_chan;

    if (tws_besaud_is_connected())
    {
        uint8_t ibrt_role = app_tws_get_ibrt_role(&spp_chan->remote);
        LOG_I("(d%x) %s role %d", spp_chan->device_id, __func__, ibrt_role);

        if (IBRT_MASTER == ibrt_role)
        {
            if (app_ibrt_conn_is_profile_exchanged(&spp_chan->remote))
            {
                tws_ctrl_send_cmd(APP_TWS_CMD_DISC_RFCOMM_REQ, (uint8_t*)&rfcomm_req, sizeof(struct app_ibrt_rfcomm_req));
            }
            else
            {
                app_ibrt_if_disconnect_rfcomm_handler((uint8_t*)&rfcomm_req);
            }
        }
        else if (IBRT_SLAVE == ibrt_role)
        {
            app_ibrt_if_start_user_action_v2(spp_chan->device_id, IBRT_ACTION_TELL_MASTER_DISC_RFCOMM, (uint32_t)spp_chan, reason);
        }
    }
    else
    {
        app_ibrt_if_disconnect_rfcomm_handler((uint8_t*)&rfcomm_req);
    }
}


void app_ibrt_if_enter_non_signalingtest_mode(void)
{
    app_bt_call_func_in_bt_thread(0, 0, 0, 0, (uint32_t)app_enter_non_signalingtest_mode);
}

void app_ibrt_if_bt_stop_inqury(void)
{
    app_bt_call_func_in_bt_thread(0, 0, 0, 0, (uint32_t)app_bt_stop_inquiry);
}

void app_ibrt_if_bt_start_inqury(void)
{
    app_bt_call_func_in_bt_thread(0, 0, 0, 0, (uint32_t)app_bt_start_inquiry);
}


void app_ibrt_if_bt_set_local_dev_name(const uint8_t *dev_name, unsigned char len)
{
    static uint8_t local_bt_name[BTM_NAME_MAX_LEN];
    memcpy(local_bt_name, dev_name, len);
    app_bt_call_func_in_bt_thread((uint32_t)local_bt_name, (uint32_t)len, 0, 0, (uint32_t)bt_set_local_dev_name);
}

void app_ibrt_if_disconnect_all_bt_connections(void)
{
    app_bt_call_func_in_bt_thread(0, 0, 0, 0, (uint32_t)app_disconnect_all_bt_connections);
}

static void app_ibrt_if_spp_write_handler(bt_spp_channel_t *spp_chan, char *buff, uint16_t length, app_ibrt_spp_write_result_callback_t func)
{
    bool isSuccessful = true;
    bt_status_t ret = bt_spp_write(spp_chan->rfcomm_handle, (uint8_t *)buff, length);
    if (ret == BT_STS_FAILED)
    {
        isSuccessful = false;
    }

    if (func)
    {
        func(buff, length, isSuccessful);
    }
}

void app_ibrt_if_spp_write(bt_spp_channel_t *spp_chan, char *buff, uint16_t length, app_ibrt_spp_write_result_callback_t func)
{
    app_bt_call_func_in_bt_thread((uint32_t)spp_chan, (uint32_t)buff, (uint32_t)length, (uint32_t)func,
            (uint32_t)app_ibrt_if_spp_write_handler);
}

void app_ibrt_if_write_page_timeout(uint16_t timeout)
{
    app_bt_call_func_in_bt_thread((uint16_t)timeout, 0, 0, 0, (uint32_t)btif_me_write_page_timeout);
}

void app_ibrt_if_conn_tws_connect_request(bool isInPairingMode, uint32_t timeout)
{
    app_bt_call_func_in_bt_thread((uint16_t)isInPairingMode, (uint32_t)timeout, 0, 0, (uint32_t)app_ibrt_conn_tws_connect_request);
}

void app_ibrt_if_conn_remote_dev_connect_request(const bt_bdaddr_t *addr,connection_direction_t direction,bool request_connect, uint32_t timeout)
{
    static bt_bdaddr_t remote_bt_addr;
    memcpy(remote_bt_addr.address, addr->address, sizeof(bt_bdaddr_t));
    app_bt_call_func_in_bt_thread((uint32_t)&remote_bt_addr, (uint32_t)direction, (uint32_t)request_connect, (uint32_t)timeout, (uint32_t)app_ibrt_conn_remote_dev_connect_request);
}

bool app_ibrt_if_is_peer_mobile_link_exist_but_local_not_on_tws_connected(void)
{
    return app_tws_ibrt_get_bt_ctrl_ctx()->isPeerMobileLinkExistButLocalNotOnTwsConnected;
}

void app_ibrt_if_clear_flag_peer_mobile_link_exist_but_local_not_on_tws_connected(void)
{
    app_tws_ibrt_get_bt_ctrl_ctx()->isPeerMobileLinkExistButLocalNotOnTwsConnected = false;
}

void app_ibrt_if_register_is_reject_new_mobile_connection_query_callback(new_connection_callback_t callback)
{
    app_tws_ibrt_register_is_reject_new_mobile_connection_callback(callback);
}

static void app_ibrt_if_set_a2dp_current_abs_volume_handler(int device_id, uint8_t volume)
{
    app_a2dp_reject_sniff_start(device_id, APP_A2DP_REJECT_SNIFF_TIMEOUT);
    app_bt_set_a2dp_current_abs_volume(device_id, volume);
    app_ibrt_if_sync_volume_info_v2(device_id);
}

void app_ibrt_if_set_a2dp_current_abs_volume(int device_id, uint8_t volume)
{
    app_bt_call_func_in_bt_thread((uint32_t)device_id, (uint32_t)volume,
        0, 0, (uint32_t)app_ibrt_if_set_a2dp_current_abs_volume_handler);
}

void app_ibrt_if_write_page_scan_setting(uint16_t window_slots, uint16_t interval_slots)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)window_slots, (uint32_t)interval_slots,
        (uint32_t)app_ibrt_conn_pscan_setting);
}

void app_ibrt_if_register_is_reject_tws_connection_callback(new_connection_callback_t callback)
{
    app_tws_ibrt_register_is_reject_tws_connection_callback(callback);
}

uint8_t app_ibrt_if_get_max_support_link(void)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    return p_ibrt_ctrl->custom_config.support_max_remote_link;
}

void app_ibrt_if_ui_allow_resume_a2dp_callback(app_bt_audio_ui_allow_resume_request allow_resume_request_cb)
{
    app_bt_audio_ui_allow_resume_a2dp_register(allow_resume_request_cb);
}

/**
 * \brief           hfp hold result calback
 * \param           typedef void(*hfp_hold_result_callback)(uint8_t device_id, uint8_t result);
 * \return          device_id:0/1
 *                  result:0-sucess/other-failed.
 */
void app_ibrt_if_register_hold_result_callback(hfp_hold_result_callback result_cb)
{
    btif_hfp_hold_result_callback(result_cb);
}

void app_ibrt_if_register_report_remoteDevice_name_callback(remoteDevice_name_Callback_t callback)
{
    app_tws_ibrt_register_report_remoteDevice_name_callback(callback);
}

static bool app_ibrt_if_role_switch_needed(void)
{
    bud_box_state local_box_state = app_ui_get_local_box_state();
    bud_box_state peer_box_state = app_ui_get_peer_box_state();
    if ((local_box_state == IBRT_OUT_BOX_WEARED) &&
        (peer_box_state == IBRT_OUT_BOX_WEARED))
    {
        // both ear was wear up,so need to switch
        return true;
    }
    else
    {
        LOG_I("tws switch fail by box state l_b:%d, p_b:%d",
            local_box_state, peer_box_state);
        return false;
    }
    return false;
}

void app_ibrt_if_customer_role_switch()
{
    bool switch2master = true;

    if (TWS_UI_MASTER == app_ui_get_current_role())
    {
        switch2master = false;
    }
    else if(TWS_UI_SLAVE == app_ui_get_current_role())
    {
        switch2master = true;
    }
    else
    {
        LOG_I("%s unknown role fail ", __func__);
    }

    LOG_I("%s change to master %d", __func__, switch2master);

    if ((!app_ibrt_if_is_tws_role_switch_on())
        && app_ibrt_if_role_switch_needed())
    {
        app_ui_user_role_switch(switch2master);
    }
    else
    {
        LOG_I("%s fail", __func__);
    }
}

#ifdef __IAG_BLE_INCLUDE__
void app_ibrt_if_set_nonConn_adv_data(uint8_t len, uint8_t *adv_data, uint16_t interval, bool enable)
{
    bes_ble_gap_cus_adv_param_t adv_param;

    adv_param.actv_user           = BLE_ADV_ACTIVITY_USER_0;
    adv_param.is_custom_adv_flags = enable;
    adv_param.type                = BLE_ADV_PRIVATE_STATIC;
    adv_param.local_addr          = (uint8_t *)bt_get_ble_local_address();
    adv_param.adv_interval        = interval;
    adv_param.adv_type            = ADV_TYPE_NON_CONN_SCAN;
    adv_param.adv_mode            = ADV_MODE_LEGACY;
    adv_param.tx_power_dbm        = -5;
    adv_param.adv_data            = adv_data;
    adv_param.adv_data_size       = len;
    adv_param.scan_rsp_data       = NULL;
    adv_param.scan_rsp_data_size  = 0;
    bes_ble_gap_custom_adv_write_data(&adv_param);
}

void app_ibrt_if_stop_ble_adv(void)
{
    bes_ble_gap_custom_adv_stop(BLE_ADV_ACTIVITY_USER_0);
}
#endif /*__IAG_BLE_INCLUDE__*/

#ifdef IS_TWS_IBRT_DEBUG_SYSTEM_ENABLED
void app_ibrt_if_register_link_loss_universal_info_received_cb(app_tws_ibrt_analysing_info_received_cb callback)
{
    app_tws_ibrt_register_link_loss_universal_info_received_cb(callback);
}

void app_ibrt_if_register_link_loss_clock_info_received_cb(app_tws_ibrt_analysing_info_received_cb callback)
{
    app_tws_ibrt_register_link_loss_clock_info_received_cb(callback);
}

void app_ibrt_if_register_a2dp_sink_info_received_cb(app_tws_ibrt_analysing_info_received_cb callback)
{
    app_tws_ibrt_register_a2dp_sink_info_received_cb(callback);
}


tws_ibrt_link_loss_info_t* app_ibrt_if_get_wireless_signal_link_loss_info(void)
{
    return app_tws_ibrt_get_wireless_signal_link_loss_total_info();
}

tws_ibrt_a2dp_sink_info_t* app_ibrt_if_get_wireless_signal_a2dp_sink_info(void)
{
    return app_tws_ibrt_get_wireless_signal_a2dp_sink_total_info();
}
#endif

#ifdef CUSTOM_BITRATE
#include "app_ibrt_customif_cmd.h"
//#include "product_config.h"
ibrt_custome_codec_t a2dp_custome_config;
static bool user_a2dp_info_set = false;
extern void a2dp_avdtpcodec_aac_user_configure_set(uint32_t bitrate,uint8_t user_configure);
extern void a2dp_avdtpcodec_sbc_user_configure_set(uint32_t bitpool,uint8_t user_configure);
extern void app_audio_dynamic_update_dest_packet_mtu_set(uint8_t codec_index, uint8_t packet_mtu, uint8_t user_configure);

#define RECONNECT_MOBILE_INTERVAL      (5000)
static void app_ibrt_reconnect_mobile_timehandler(void const *param);
osTimerDef (IBRT_RECONNECT_MOBILE, app_ibrt_reconnect_mobile_timehandler);
static osTimerId  app_ibrt_mobile_timer = NULL;
typedef struct{
    bool used;
    bt_bdaddr_t mobile_addr;
}manual_reconnect_device_info_t;

static manual_reconnect_device_info_t mobile_info[BT_DEVICE_NUM];

static void app_ibrt_reconnect_mobile_timehandler(void const *param)
{
    for (int i = 0; i < app_ibrt_conn_support_max_mobile_dev(); ++i){
        if(mobile_info[i].used == true){
            mobile_info[i].used = false;
            app_ibrt_if_reconnect_moblie_device((const bt_bdaddr_t*)(&mobile_info[i].mobile_addr));
        }else{
            continue;
        }
    }
}

static void app_ibrt_if_mannual_disconnect_mobile_then_reconnect(void)
{
    for (int i = 0; i < app_ibrt_conn_support_max_mobile_dev(); ++i){
        struct BT_DEVICE_T *curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected){
            app_ibrt_if_disconnet_moblie_device(&curr_device->remote);
            if (mobile_info[i].used == false){
                mobile_info[i].used = true;
                memcpy((uint8_t*)&(mobile_info[i].mobile_addr), (uint8_t*)(&curr_device->remote), sizeof(bt_bdaddr_t));
            }
            if (app_ibrt_mobile_timer == NULL){
                app_ibrt_mobile_timer = osTimerCreate(osTimer(IBRT_RECONNECT_MOBILE), osTimerOnce, NULL);
            }
            if (app_ibrt_mobile_timer != NULL){
                osTimerStart(app_ibrt_mobile_timer, RECONNECT_MOBILE_INTERVAL);
            }
        }
        else{
            LOG_I("acl is not connect");
        }
    }
}

void app_ibrt_if_set_codec_param(uint32_t aac_bitrate,uint32_t sbc_boitpool,uint32_t audio_latency)
{
    LOG_I("%s aac=%d sbc=%d latency=%d", __func__, aac_bitrate, sbc_boitpool, audio_latency);
    if(app_bt_ibrt_has_mobile_link_connected()
#ifndef FREEMAN_ENABLED_STERO
	&&(app_tws_ibrt_tws_link_connected())
#endif
	)
	{
        a2dp_custome_config.aac_bitrate = aac_bitrate;
        a2dp_custome_config.sbc_bitpool = sbc_boitpool;
        a2dp_custome_config.audio_latentcy = audio_latency;
        uint32_t lock = nv_record_pre_write_operation();
        nv_record_get_extension_entry_ptr()->codec_user_info.aac_bitrate = a2dp_custome_config.aac_bitrate;
        nv_record_get_extension_entry_ptr()->codec_user_info.sbc_bitpool = a2dp_custome_config.sbc_bitpool;
        nv_record_get_extension_entry_ptr()->codec_user_info.audio_latentcy = a2dp_custome_config.audio_latentcy;
        nv_record_post_write_operation(lock);
        nv_record_flash_flush();
        user_a2dp_info_set = true;
#ifndef FREEMAN_ENABLED_STERO
        app_ibrt_user_a2dp_info_sync_tws_share_cmd_send((uint8_t *)&a2dp_custome_config, sizeof(ibrt_custome_codec_t));
#endif
        app_ibrt_if_mannual_disconnect_mobile_then_reconnect();
    }
}

void app_ibrt_user_a2dp_codec_info_action(void)
{
    LOG_I("%s %d",__func__,user_a2dp_info_set);
    if(user_a2dp_info_set){
        user_a2dp_info_set = false;
    }
    else{
        return;
    }

#ifndef FREEMAN_ENABLED_STERO
    if(app_ibrt_if_get_ui_role() == IBRT_MASTER)
#endif
    {
        a2dp_avdtpcodec_sbc_user_configure_set(a2dp_custome_config.sbc_bitpool, true);
        a2dp_avdtpcodec_aac_user_configure_set(a2dp_custome_config.aac_bitrate, true);
        app_audio_dynamic_update_dest_packet_mtu_set(A2DP_AUDIO_CODEC_TYPE_SBC, (a2dp_custome_config.audio_latentcy-USER_CONFIG_SBC_AUDIO_LATENCY_ERROR)/3, true);//sbc
        app_audio_dynamic_update_dest_packet_mtu_set(A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC, (a2dp_custome_config.audio_latentcy-USER_CONFIG_AAC_AUDIO_LATENCY_ERROR)/23, true);//aac
    }
}
#endif
#endif /*IBRT*/

