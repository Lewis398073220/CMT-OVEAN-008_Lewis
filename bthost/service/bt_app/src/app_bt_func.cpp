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
#include "string.h"
#include "hal_trace.h"

#include "bluetooth.h"
#include "besbt.h"
#include "app_bt_func.h"
#include "hfp_api.h"
#include "app_bt.h"

#ifdef __IAG_BLE_INCLUDE__
#include "app_ble_mode_switch.h"
#endif
#if defined(IBRT) && defined(FREEMAN_ENABLED_STERO)
#include "app_ibrt_if.h"
#endif

#ifdef __GATT_OVER_BR_EDR__
#include "btgatt_api.h"
#endif

extern "C" void OS_NotifyEvm(void);
static const char * const app_bt_func_table_str[] =
{
    "Me_switch_sco_req",
    "ME_SwitchRole_req",
    "MeDisconnectLink_req",
    "ME_StopSniff_req",
    "ME_SetAccessibleMode_req",
    "Me_SetLinkPolicy_req",
    "CMGR_SetSniffTimer_req",
    "CMGR_SetSniffInofToAllHandlerByRemDev_req",
    "A2DP_OpenStream_req",
    "A2DP_CloseStream_req",
    "HF_CreateServiceLink_req",
    "HF_DisconnectServiceLink_req",
    "HF_CreateAudioLink_req",
    "HF_DisconnectAudioLink_req",
    "BT_Control_SleepMode_req",
    "BT_Custom_Func_req",
    "BT_Thread_Defer_Func_req",
    "ME_StartSniff_req",
    "DIP_QuryService_req",
    "A2DP_Force_OpenStream_req",
    "HF_Force_CreateServiceLink_req",
    "BT_Red_Ccmp_Client_Open",
    "BT_Set_Access_Mode_Test",
    "BT_Set_Adv_Mode_Test",
    "Write_Controller_Memory_Test",
    "Read_Controller_Memory_Test",
    "GATT_Connect_Req",
    "GATT_Disconnect_Req",
};

#define APP_BT_MAILBOX_MAX (40)
osMailQDef (app_bt_mailbox, APP_BT_MAILBOX_MAX, APP_BT_MAIL);
static osMailQId app_bt_mailbox = NULL;

static btif_accessible_mode_t gBT_DEFAULT_ACCESS_MODE = BTIF_BAM_NOT_ACCESSIBLE;
static uint8_t bt_access_mode_set_pending = 0;
void app_set_accessmode(btif_accessible_mode_t mode)
{
#if !defined(IBRT)
    const btif_access_mode_info_t info = { BTIF_BT_DEFAULT_INQ_SCAN_INTERVAL,
                                    BTIF_BT_DEFAULT_INQ_SCAN_WINDOW,
                                    BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL,
                                    BTIF_BT_DEFAULT_PAGE_SCAN_WINDOW };
    bt_status_t status;

    gBT_DEFAULT_ACCESS_MODE = mode;

    status =   btif_me_set_accessible_mode(mode, &info);
    TRACE(1,"app_set_accessmode status=0x%x",status);

    if(status == BT_STS_IN_PROGRESS)
        bt_access_mode_set_pending = 1;
    else
        bt_access_mode_set_pending = 0;
#endif
}

bool app_is_access_mode_set_pending(void)
{
    return bt_access_mode_set_pending;
}

extern "C" void app_bt_accessmode_set(btif_accessible_mode_t mode);
void app_set_pending_access_mode(void)
{
    if (bt_access_mode_set_pending)
    {
        TRACE(1,"Pending for change access mode to %d", gBT_DEFAULT_ACCESS_MODE);
        bt_access_mode_set_pending = 0;
        app_bt_accessmode_set(gBT_DEFAULT_ACCESS_MODE);
    }
}

void app_retry_setting_access_mode(void)
{
    TRACE(0,"Former setting access mode failed, retry it.");
    app_bt_accessmode_set(gBT_DEFAULT_ACCESS_MODE);
}

#define PENDING_SET_LINKPOLICY_REQ_BUF_CNT  5
static BT_SET_LINKPOLICY_REQ_T pending_set_linkpolicy_req[PENDING_SET_LINKPOLICY_REQ_BUF_CNT];

static uint8_t pending_set_linkpolicy_in_cursor = 0;
static uint8_t pending_set_linkpolicy_out_cursor = 0;

static void app_bt_print_pending_set_linkpolicy_req(void)
{
    TRACE(0,"Pending set link policy requests:");
    uint8_t index = pending_set_linkpolicy_out_cursor;
    while (index != pending_set_linkpolicy_in_cursor)
    {
        TRACE(3,"index %d RemDev %p LinkPolicy %d", index,
            pending_set_linkpolicy_req[index].remDev,
            pending_set_linkpolicy_req[index].policy);
        index++;
        if (PENDING_SET_LINKPOLICY_REQ_BUF_CNT == index)
        {
            index = 0;
        }
    }
}

static void app_bt_push_pending_set_linkpolicy(btif_remote_device_t *remDev, btif_link_policy_t policy)
{
    // go through the existing pending list to see if the remDev is already in
    uint8_t index = pending_set_linkpolicy_out_cursor;
    while (index != pending_set_linkpolicy_in_cursor)
    {
        if (remDev == pending_set_linkpolicy_req[index].remDev)
        {
            pending_set_linkpolicy_req[index].policy = policy;
            return;
        }
        index++;
        if (PENDING_SET_LINKPOLICY_REQ_BUF_CNT == index)
        {
            index = 0;
        }
    }

    pending_set_linkpolicy_req[pending_set_linkpolicy_in_cursor].remDev = remDev;
    pending_set_linkpolicy_req[pending_set_linkpolicy_in_cursor].policy = policy;
    pending_set_linkpolicy_in_cursor++;
    if (PENDING_SET_LINKPOLICY_REQ_BUF_CNT == pending_set_linkpolicy_in_cursor)
    {
        pending_set_linkpolicy_in_cursor = 0;
    }

    app_bt_print_pending_set_linkpolicy_req();
}

BT_SET_LINKPOLICY_REQ_T* app_bt_pop_pending_set_linkpolicy(void)
{
    if (pending_set_linkpolicy_out_cursor == pending_set_linkpolicy_in_cursor)
    {
        return NULL;
    }

    BT_SET_LINKPOLICY_REQ_T* ptReq = &pending_set_linkpolicy_req[pending_set_linkpolicy_out_cursor];
    pending_set_linkpolicy_out_cursor++;
    if (PENDING_SET_LINKPOLICY_REQ_BUF_CNT == pending_set_linkpolicy_out_cursor)
    {
        pending_set_linkpolicy_out_cursor = 0;
    }

    app_bt_print_pending_set_linkpolicy_req();
    return ptReq;
}

void app_bt_set_linkpolicy(btif_remote_device_t *remDev, btif_link_policy_t policy)
{
    if (btif_me_get_remote_device_state(remDev) == BTIF_BDS_CONNECTED)
    {
        bt_status_t ret = btif_me_set_link_policy(remDev, policy);
        TRACE(3,"%s policy %d returns %d", __FUNCTION__, policy, ret);

        if (BT_STS_IN_PROGRESS == ret)
        {
            app_bt_push_pending_set_linkpolicy(remDev, policy);
        }
    }
}

#define COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE  8
static btif_remote_device_t* pendingRemoteDevToExitSniffMode[COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE];
static uint8_t  maskOfRemoteDevPendingForExitingSniffMode = 0;
void app_check_pending_stop_sniff_op(void)
{
    if (maskOfRemoteDevPendingForExitingSniffMode > 0)
    {
        for (uint8_t index = 0;index < COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE;index++)
        {
            if (maskOfRemoteDevPendingForExitingSniffMode & (1 << index))
            {
                btif_remote_device_t* remDev = pendingRemoteDevToExitSniffMode[index];
                if (btif_me_get_remote_device_state(remDev) == BTIF_BDS_CONNECTED){
                    if (btif_me_get_current_mode(remDev) == BTIF_BLM_SNIFF_MODE){
                        TRACE(1,"!!! stop sniff currmode:%d\n",  btif_me_get_current_mode(remDev));
                        bt_status_t ret = btif_me_stop_sniff(remDev);
                        TRACE(1,"Return status %d", ret);
                        if (BT_STS_IN_PROGRESS != ret)
                        {
                            maskOfRemoteDevPendingForExitingSniffMode &= (~(1<<index));
                            break;
                        }
                    }
                }
            }
        }

        if (maskOfRemoteDevPendingForExitingSniffMode > 0)
        {
            osapi_notify_evm();
        }
    }
}

static void app_add_pending_stop_sniff_op(btif_remote_device_t* remDev)
{
    for (uint8_t index = 0;index < COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE;index++)
    {
        if (maskOfRemoteDevPendingForExitingSniffMode & (1 << index))
        {
            if (pendingRemoteDevToExitSniffMode[index] == remDev)
            {
                return;
            }
        }
    }

    for (uint8_t index = 0;index < COUNT_OF_PENDING_REMOTE_DEV_TO_EXIT_SNIFF_MODE;index++)
    {
        if (0 == (maskOfRemoteDevPendingForExitingSniffMode & (1 << index)))
        {
            pendingRemoteDevToExitSniffMode[index] = remDev;
            maskOfRemoteDevPendingForExitingSniffMode |= (1 << index);
        }
    }
}

struct app_bt_alloc_param_t {
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
};

extern "C" bt_status_t app_tws_ibrt_set_access_mode(btif_accessible_mode_t mode);

#ifdef FPGA
void app_start_ble_adv_for_test(void);
#endif

static inline int app_bt_mail_process(APP_BT_MAIL* mail_p)
{
    bt_status_t status = BT_STS_FAILED;
    if (mail_p->request_id != CMGR_SetSniffTimer_req &&
        mail_p->request_id != BT_Custom_Func_req &&
        mail_p->request_id != BT_Thread_Defer_Func_req)
    {
        TRACE(3,"[BT_FUNC] src_thread:0x%08x call request_id=%x->:%s", mail_p->src_thread, mail_p->request_id,app_bt_func_table_str[mail_p->request_id]);
    }
    switch (mail_p->request_id) {
        case Me_switch_sco_req:
            status = btif_me_switch_sco(mail_p->param.Me_switch_sco_param.scohandle);
            break;
        case ME_SwitchRole_req:
            status = btif_me_switch_role(mail_p->param.ME_SwitchRole_param.remDev);
            break;
        case MeDisconnectLink_req:
            status = btif_me_force_disconnect_link_with_reason(NULL, mail_p->param.MeDisconnectLink_param.remDev, BTIF_BEC_USER_TERMINATED, TRUE);
            break;
        case ME_StopSniff_req:
        {
            if (btif_me_get_remote_device_state(mail_p->param.ME_StopSniff_param.remDev) == BTIF_BDS_CONNECTED)
            {
                status = btif_me_stop_sniff(mail_p->param.ME_StopSniff_param.remDev);
                if (BT_STS_IN_PROGRESS == status)
                {
                    app_add_pending_stop_sniff_op(mail_p->param.ME_StopSniff_param.remDev);
                }
            }
            break;
        }
        case ME_StartSniff_req:
        {
            btif_cmgr_handler_t    *cmgrHandler;
            if (btif_me_get_remote_device_state(mail_p->param.ME_StartSniff_param.remDev) == BTIF_BDS_CONNECTED) {
                cmgrHandler = btif_cmgr_get_acl_handler(mail_p->param.ME_StartSniff_param.remDev);
                status = btif_me_start_sniff(mail_p->param.ME_StartSniff_param.remDev,
                                            &(mail_p->param.ME_StartSniff_param.sniffInfo));
                if (BT_STS_PENDING != status) {
                    if (mail_p->param.ME_StartSniff_param.sniffInfo.maxInterval == 0) {
                        status = btif_cmgr_set_sniff_timer((btif_cmgr_handler_t *)cmgrHandler,
                                                        NULL,
                                                        BTIF_CMGR_SNIFF_TIMER);
                    } else {
                        status = btif_cmgr_set_sniff_timer((btif_cmgr_handler_t *)cmgrHandler,
                                                        &(mail_p->param.ME_StartSniff_param.sniffInfo),
                                                        BTIF_CMGR_SNIFF_TIMER);
                    }
                }
            }
            break;
        }
        case BT_Control_SleepMode_req:
        {
            bt_adapter_write_sleep_enable(mail_p->param.ME_BtControlSleepMode_param.isEnable);
            break;
        }
        case ME_SetAccessibleMode_req:
#if defined(IBRT)
            app_tws_ibrt_set_access_mode(mail_p->param.ME_SetAccessibleMode_param.mode);
#else
            app_set_accessmode(mail_p->param.ME_SetAccessibleMode_param.mode);
#endif
            break;
        case Me_SetLinkPolicy_req:
            app_bt_set_linkpolicy(mail_p->param.Me_SetLinkPolicy_param.remDev,
                                      mail_p->param.Me_SetLinkPolicy_param.policy);
            break;
        case CMGR_SetSniffTimer_req:
            if (mail_p->param.CMGR_SetSniffTimer_param.SniffInfo.maxInterval == 0){
                status = btif_cmgr_set_sniff_timer(mail_p->param.CMGR_SetSniffTimer_param.Handler,
                                            NULL,
                                            mail_p->param.CMGR_SetSniffTimer_param.Time);
            }else{
                status = btif_cmgr_set_sniff_timer(mail_p->param.CMGR_SetSniffTimer_param.Handler,
                                            &mail_p->param.CMGR_SetSniffTimer_param.SniffInfo,
                                            mail_p->param.CMGR_SetSniffTimer_param.Time);
            }
            break;
        case CMGR_SetSniffInofToAllHandlerByRemDev_req:
            status = btif_cmgr_set_sniff_info_by_remdev(&mail_p->param.CMGR_SetSniffInofToAllHandlerByRemDev_param.SniffInfo,
                                                            mail_p->param.CMGR_SetSniffInofToAllHandlerByRemDev_param.RemDev);
            break;
#ifdef BT_A2DP_SUPPORT
        case A2DP_OpenStream_req:
            status = btif_a2dp_open_stream((btif_avdtp_codec_t*)mail_p->param.A2DP_OpenStream_param.Stream,
                                        &mail_p->param.A2DP_OpenStream_param.Addr);
            if ((BT_STS_NO_RESOURCES == status) || (BT_STS_IN_PROGRESS == status))
            {
#if defined(IBRT) && defined(FREEMAN_ENABLED_STERO)
                app_ibrt_if_set_access_mode(IBRT_BAM_CONNECTABLE_ONLY);
#else
                app_bt_accessmode_set(BTIF_BAM_CONNECTABLE_ONLY);
#endif
            }
            else
            {
#if defined(IBRT) && defined(FREEMAN_ENABLED_STERO)
                app_ibrt_if_set_access_mode(IBRT_BAM_NOT_ACCESSIBLE_MODE);
#else            
                app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);
#endif
            }
            break;
        case A2DP_CloseStream_req:
            status = btif_a2dp_close_stream(mail_p->param.A2DP_CloseStream_param.Stream);
            break;
#endif /* BT_A2DP_SUPPORT */
#ifdef BT_HFP_SUPPORT
        case HF_CreateServiceLink_req:
            status = btif_hf_create_service_link(&mail_p->param.HF_CreateServiceLink_param.Addr);
            if ((BT_STS_NO_RESOURCES == status) || (BT_STS_IN_PROGRESS == status))
            {
                app_bt_accessmode_set(BTIF_BAM_CONNECTABLE_ONLY);
            }
            else
            {
                app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);
            }
            break;
        case HF_DisconnectServiceLink_req:
            status = btif_hf_disconnect_service_link(mail_p->param.HF_DisconnectServiceLink_param.Chan);
            break;
        case HF_CreateAudioLink_req:
            status = btif_hf_create_audio_link(mail_p->param.HF_CreateAudioLink_param.Chan);
            break;
        case HF_DisconnectAudioLink_req:
            status = btif_hf_disc_audio_link(mail_p->param.HF_DisconnectAudioLink_param.Chan);
            break;
#endif /* BT_HFP_SUPPORT */
#ifdef BT_DIP_SUPPORT
        case DIP_QuryService_req:
            status = btif_dip_query_for_service(mail_p->param.DIP_QuryService_param.dip_client,
                              mail_p->param.DIP_QuryService_param.remDev);
            break;
#endif
#ifdef FPGA
        case BT_Set_Access_Mode_Test:
#if defined(IBRT)
            app_tws_ibrt_set_access_mode(mail_p->param.ME_SetAccessibleMode_param.mode);
#else
            app_set_accessmode(mail_p->param.ME_SetAccessibleMode_param.mode);
#endif
            break;
        case BT_Set_Adv_Mode_Test:
#if defined(IBRT)
        app_start_ble_adv_for_test();
#endif
            break;
        case Write_Controller_Memory_Test:
        {
            status = btif_me_write_controller_memory(mail_p->param.Me_writecontrollermem_param.addr,
                                                     mail_p->param.Me_writecontrollermem_param.memval,
                                                     mail_p->param.Me_writecontrollermem_param.type);
            break;
        }
        case Read_Controller_Memory_Test:
        {
            status = btif_me_read_controller_memory(mail_p->param.Me_readcontrollermem_param.addr,
                                                     mail_p->param.Me_readcontrollermem_param.len,
                                                     mail_p->param.Me_readcontrollermem_param.type);
            break;
        }
#endif
        case BT_Custom_Func_req:
            if (mail_p->param.CustomFunc_param.func_ptr){
#ifndef BT_SOURCE
                TRACE(3,"func:0x%08x,param0:0x%08x, param1:0x%08x",
                      mail_p->param.CustomFunc_param.func_ptr,
                      mail_p->param.CustomFunc_param.param0,
                      mail_p->param.CustomFunc_param.param1);
#endif
                ((APP_BT_REQ_CUSTOMER_CALl_CB_T)(mail_p->param.CustomFunc_param.func_ptr))(
                    (void *)mail_p->param.CustomFunc_param.param0,
                    (void *)mail_p->param.CustomFunc_param.param1,
                    (void *)mail_p->param.CustomFunc_param.param2,
                    (void *)mail_p->param.CustomFunc_param.param3);
            }
            break;
        case BT_Thread_Defer_Func_req:
            if (mail_p->param.CustomFunc_param.func_ptr)
            {
                struct app_bt_alloc_param_t *alloc_param = NULL;
                alloc_param = (struct app_bt_alloc_param_t *)(uintptr_t)mail_p->param.CustomFunc_param.param0;
                ((APP_BT_REQ_CUSTOMER_CALl_CB_T)(mail_p->param.CustomFunc_param.func_ptr))(
                    (void *)alloc_param->param0,
                    (void *)alloc_param->param1,
                    (void *)alloc_param->param2,
                    (void *)alloc_param->param3);
                btif_cobuf_free((uint8_t *)alloc_param);
            }
            break;
#ifdef __GATT_OVER_BR_EDR__
        case GATT_Connect_Req:
        {
            uint8_t device_id = app_bt_get_device_id_byaddr(&(mail_p->param.GATT_param.Addr));
            if(!btif_btgatt_is_connected(device_id))
            {
                btif_btgatt_client_create(app_bt_get_remoteDev(device_id));
            }
            break;
        }
        case GATT_Disconnect_Req:
        {
            uint8_t device_id = app_bt_get_device_id_byaddr(&(mail_p->param.GATT_param.Addr));
            if(btif_btgatt_is_connected(device_id))
            {
                btif_btgatt_disconnect(device_id);
            }
            break;
        }
#endif
    }

    if (mail_p->request_id != CMGR_SetSniffTimer_req &&
        mail_p->request_id != BT_Custom_Func_req &&
        mail_p->request_id != BT_Thread_Defer_Func_req)
    {
        TRACE(2,"[BT_FUNC] exit request_id:%d :status:%d", mail_p->request_id, status);
    }
    return 0;
}

static inline int app_bt_mail_alloc(APP_BT_MAIL** mail)
{
    *mail = (APP_BT_MAIL*)osMailAlloc(app_bt_mailbox, 0);
    ASSERT(*mail, "app_bt_mail_alloc error");
    return 0;
}

static inline int app_bt_mail_send(APP_BT_MAIL* mail)
{
    osStatus status;

    ASSERT(mail, "osMailAlloc NULL");
    status = osMailPut(app_bt_mailbox, mail);
    ASSERT(osOK == status, "osMailAlloc Put failed");

    osapi_notify_evm();

    return (int)status;
}

static inline int app_bt_mail_free(APP_BT_MAIL* mail_p)
{
    osStatus status;

    status = osMailFree(app_bt_mailbox, mail_p);
    ASSERT(osOK == status, "osMailAlloc Put failed");

    return (int)status;
}

static inline int app_bt_mail_get(APP_BT_MAIL** mail_p)
{
    osEvent evt;
    evt = osMailGet(app_bt_mailbox, 0);
    if (evt.status == osEventMail) {
        *mail_p = (APP_BT_MAIL *)evt.value.p;
        return 0;
    }
    return -1;
}

static void app_bt_mail_poll(void)
{
    APP_BT_MAIL *mail_p = NULL;
    if (!app_bt_mail_get(&mail_p)){
        app_bt_mail_process(mail_p);
        app_bt_mail_free(mail_p);
        osapi_notify_evm();
    }
}

int app_bt_mail_init(void)
{
    app_bt_mailbox = osMailCreate(osMailQ(app_bt_mailbox), NULL);
    if (app_bt_mailbox == NULL)  {
        TRACE(0,"Failed to Create app_mailbox\n");
        return -1;
    }
    Besbt_hook_handler_set(BESBT_HOOK_USER_1, app_bt_mail_poll);

    return 0;
}

int app_bt_Me_switch_sco(uint16_t  scohandle)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = Me_switch_sco_req;
    mail->param.Me_switch_sco_param.scohandle = scohandle;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_SwitchRole(btif_remote_device_t* remDev)
{
#if !defined(IBRT) && !defined(BT_DISABLE_INITIAL_ROLE_SWITCH)
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = ME_SwitchRole_req;
    mail->param.ME_SwitchRole_param.remDev = remDev;
    app_bt_mail_send(mail);
#endif
    return 0;
}

int app_bt_MeDisconnectLink(btif_remote_device_t* remDev)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = MeDisconnectLink_req;
    mail->param.MeDisconnectLink_param.remDev = remDev;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_StopSniff(btif_remote_device_t *remDev)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = ME_StopSniff_req;
    mail->param.ME_StopSniff_param.remDev = remDev;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_StartSniff(btif_remote_device_t *remDev, btif_sniff_info_t* sniffInfo)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = ME_StartSniff_req;
    mail->param.ME_StartSniff_param.remDev = remDev;
    mail->param.ME_StartSniff_param.sniffInfo = *sniffInfo;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_ControlSleepMode(bool isEnabled)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = BT_Control_SleepMode_req;
    mail->param.ME_BtControlSleepMode_param.isEnable = isEnabled;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_SetAccessibleMode(btif_accessible_mode_t mode)
{
#if defined(BLE_ONLY_ENABLED)
    return 0;
#endif

    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = ME_SetAccessibleMode_req;
    mail->param.ME_SetAccessibleMode_param.mode = mode;
    app_bt_mail_send(mail);
    return 0;
}

#ifdef FPGA
int app_bt_ME_SetAccessibleMode_Fortest(btif_accessible_mode_t mode, const btif_access_mode_info_t *info)
{
#if defined(BLE_ONLY_ENABLED)
    return 0;
#endif

    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = BT_Set_Access_Mode_Test;
    mail->param.ME_SetAccessibleMode_param.mode = mode;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_Set_Advmode_Fortest(uint8_t en)
{


    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = BT_Set_Adv_Mode_Test;
    mail->param.ME_BtSetAdvMode_param.isEnable = en;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_Write_Controller_Memory_Fortest(uint32_t addr,uint32_t val,uint8_t type)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = Write_Controller_Memory_Test;
    mail->param.Me_writecontrollermem_param.addr = addr;
    mail->param.Me_writecontrollermem_param.memval = val;
    mail->param.Me_writecontrollermem_param.type = type;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_ME_Read_Controller_Memory_Fortest(uint32_t addr,uint32_t len,uint8_t type)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = Read_Controller_Memory_Test;
    mail->param.Me_readcontrollermem_param.addr = addr;
    mail->param.Me_readcontrollermem_param.len = len;
    mail->param.Me_readcontrollermem_param.type = type;
    app_bt_mail_send(mail);
    return 0;
}

#endif

int app_bt_Me_SetLinkPolicy(btif_remote_device_t *remDev, btif_link_policy_t policy)
{
#if !defined(IBRT)
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = Me_SetLinkPolicy_req;
    mail->param.Me_SetLinkPolicy_param.remDev = remDev;
    mail->param.Me_SetLinkPolicy_param.policy = policy;
    app_bt_mail_send(mail);
#endif
    return 0;
}

int app_bt_CMGR_SetSniffTimer(   btif_cmgr_handler_t *Handler,
                                                btif_sniff_info_t* SniffInfo,
                                                TimeT Time)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = CMGR_SetSniffTimer_req;
    mail->param.CMGR_SetSniffTimer_param.Handler = Handler;
    if (SniffInfo){
        memcpy(&mail->param.CMGR_SetSniffTimer_param.SniffInfo, SniffInfo, sizeof(btif_sniff_info_t));
    }else{
        memset(&mail->param.CMGR_SetSniffTimer_param.SniffInfo, 0, sizeof(btif_sniff_info_t));
    }
    mail->param.CMGR_SetSniffTimer_param.Time = Time;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_CMGR_SetSniffInfoToAllHandlerByRemDev(btif_sniff_info_t* SniffInfo,
                                                                 btif_remote_device_t *RemDev)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = CMGR_SetSniffInofToAllHandlerByRemDev_req;
    memcpy(&mail->param.CMGR_SetSniffInofToAllHandlerByRemDev_param.SniffInfo, SniffInfo, sizeof(btif_sniff_info_t));
    mail->param.CMGR_SetSniffInofToAllHandlerByRemDev_param.RemDev = RemDev;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_A2DP_OpenStream(a2dp_stream_t *Stream, bt_bdaddr_t *Addr)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = A2DP_OpenStream_req;
    mail->param.A2DP_OpenStream_param.Stream = Stream;
    mail->param.A2DP_OpenStream_param.Addr = *Addr;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_A2DP_CloseStream(a2dp_stream_t *Stream)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = A2DP_CloseStream_req;
    mail->param.A2DP_CloseStream_param.Stream = Stream;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_HF_CreateServiceLink(btif_hf_channel_t* Chan, bt_bdaddr_t *Addr)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = HF_CreateServiceLink_req;
    mail->param.HF_CreateServiceLink_param.Chan = Chan;
    mail->param.HF_CreateServiceLink_param.Addr = *Addr;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_HF_DisconnectServiceLink(btif_hf_channel_t* Chan)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = HF_DisconnectServiceLink_req;
    mail->param.HF_DisconnectServiceLink_param.Chan = Chan;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_HF_CreateAudioLink(btif_hf_channel_t* Chan)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = HF_CreateAudioLink_req;
    mail->param.HF_CreateAudioLink_param.Chan = Chan;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_HF_DisconnectAudioLink(btif_hf_channel_t* Chan)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = HF_DisconnectAudioLink_req;
    mail->param.HF_DisconnectAudioLink_param.Chan = Chan;
    app_bt_mail_send(mail);
    return 0;
}

#ifdef BT_DIP_SUPPORT
int app_bt_dip_QuryService(btif_dip_client_t *client, btif_remote_device_t* rem)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = DIP_QuryService_req;
    mail->param.DIP_QuryService_param.remDev = rem;
    mail->param.DIP_QuryService_param.dip_client = client;
    app_bt_mail_send(mail);
    return 0;
}
#endif

int app_bt_start_custom_function_in_bt_thread(
    uint32_t param0, uint32_t param1, uint32_t funcPtr)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = BT_Custom_Func_req;
    mail->param.CustomFunc_param.func_ptr = funcPtr;
    mail->param.CustomFunc_param.param0 = param0;
    mail->param.CustomFunc_param.param1 = param1;
    mail->param.CustomFunc_param.param2 = 0;
    mail->param.CustomFunc_param.param3 = 0;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_call_func_in_bt_thread(
    uint32_t param0, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t funcPtr)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = BT_Custom_Func_req;
    mail->param.CustomFunc_param.func_ptr = funcPtr;
    mail->param.CustomFunc_param.param0 = param0;
    mail->param.CustomFunc_param.param1 = param1;
    mail->param.CustomFunc_param.param2 = param2;
    mail->param.CustomFunc_param.param3 = param3;
    app_bt_mail_send(mail);
    return 0;
}

int app_bt_defer_call_in_bt_thread(uintptr_t func, struct app_bt_alloc_param_t *param)
{
    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = BT_Thread_Defer_Func_req;
    mail->param.CustomFunc_param.func_ptr = (uint32_t)(uintptr_t)func;
    mail->param.CustomFunc_param.param0 = (uint32_t)(uintptr_t)param;
    app_bt_mail_send(mail);
    return 0;
}

struct bt_defer_param_t bt_fixed_param_impl(uint32_t param)
{
    struct bt_defer_param_t data;
    data.param = param;
    data.is_fixed_param = true;
    data.alloc_size = 0;
    return data;
}

struct bt_defer_param_t bt_alloc_param_size(const void *data_ptr, uint16_t alloc_size)
{
    struct bt_defer_param_t data;
    data.param = (uint32_t)(uintptr_t)data_ptr;
    data.is_fixed_param = false;
    data.alloc_size = alloc_size;
    return data;
}

static uint32_t bt_defer_get_param(void *alloc_param, uint16_t *curr_len, struct bt_defer_param_t *defer)
{
    if (defer->is_fixed_param)
    {
        return defer->param;
    }
    else
    {
        uint8_t *param = ((uint8_t *)alloc_param) + *curr_len;
        memcpy(param, (void *)(uintptr_t)defer->param, defer->alloc_size);
        *curr_len += co_round_size(defer->alloc_size);
        return (uint32_t)(uintptr_t)param;
    }
}

static void bt_defer_direct_call_func(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uintptr_t func)
{
    ((APP_BT_REQ_CUSTOMER_CALl_CB_T)((void *)func))((void *)a, (void *)b, (void *)c, (void *)d);
}

bool bt_defer_curr_func_0_impl(uintptr_t func, bool direct_call_func)
{
    if (app_bt_is_besbt_thread())
    {
        if (direct_call_func)
        {
            bt_defer_direct_call_func(0, 0, 0, 0, func);
        }
        return false;
    }
    app_bt_call_func_in_bt_thread(0, 0, 0, 0, (uint32_t)(uintptr_t)func);
    return true;
}

bool bt_defer_curr_func_1_impl(uintptr_t func, struct bt_defer_param_t param0, bool direct_call_func)
{
    uint16_t curr_len = sizeof(struct app_bt_alloc_param_t);
    if (app_bt_is_besbt_thread())
    {
        if (direct_call_func)
        {
            bt_defer_direct_call_func(param0.param, 0, 0, 0, func);
        }
        return false;
    }
    if (param0.is_fixed_param)
    {
        app_bt_call_func_in_bt_thread(param0.param, 0, 0, 0, (uint32_t)(uintptr_t)func);
    }
    else
    {
        struct app_bt_alloc_param_t *param = NULL;
        param = (struct app_bt_alloc_param_t *)btif_cobuf_malloc(curr_len + co_round_size(param0.alloc_size));
        param->param0 = bt_defer_get_param(param, &curr_len, &param0);
        param->param1 = param->param2 = param->param3 = 0;
        app_bt_defer_call_in_bt_thread(func, param);
    }
    return true;
}

bool bt_defer_curr_func_2_impl(uintptr_t func, struct bt_defer_param_t param0,
        struct bt_defer_param_t param1, bool direct_call_func)
{
    uint16_t curr_len = sizeof(struct app_bt_alloc_param_t);
    if (app_bt_is_besbt_thread())
    {
        if (direct_call_func)
        {
            bt_defer_direct_call_func(param0.param, param1.param, 0, 0, func);
        }
        return false;
    }
    if (param0.is_fixed_param && param1.is_fixed_param)
    {
        app_bt_call_func_in_bt_thread(param0.param, param1.param, 0, 0, (uint32_t)(uintptr_t)func);
    }
    else
    {
        struct app_bt_alloc_param_t *param = NULL;
        param = (struct app_bt_alloc_param_t *)btif_cobuf_malloc(curr_len + co_round_size(param0.alloc_size) +
            co_round_size(param1.alloc_size));
        param->param0 = bt_defer_get_param(param, &curr_len, &param0);
        param->param1 = bt_defer_get_param(param, &curr_len, &param1);
        param->param2 = param->param3 = 0;
        app_bt_defer_call_in_bt_thread(func, param);
    }
    return true;
}

bool bt_defer_curr_func_3_impl(uintptr_t func, struct bt_defer_param_t param0, struct bt_defer_param_t param1,
        struct bt_defer_param_t param2, bool direct_call_func)
{
    uint16_t curr_len = sizeof(struct app_bt_alloc_param_t);
    if (app_bt_is_besbt_thread())
    {
        if (direct_call_func)
        {
            bt_defer_direct_call_func(param0.param, param1.param, param2.param, 0, func);
        }
        return false;
    }
    if (param0.is_fixed_param && param1.is_fixed_param && param2.is_fixed_param)
    {
        app_bt_call_func_in_bt_thread(param0.param, param1.param, param2.param, 0, (uint32_t)(uintptr_t)func);
    }
    else
    {
        struct app_bt_alloc_param_t *param = NULL;
        param = (struct app_bt_alloc_param_t *)btif_cobuf_malloc(curr_len + co_round_size(param0.alloc_size) +
            co_round_size(param1.alloc_size) + co_round_size(param2.alloc_size));
        param->param0 = bt_defer_get_param(param, &curr_len, &param0);
        param->param1 = bt_defer_get_param(param, &curr_len, &param1);
        param->param2 = bt_defer_get_param(param, &curr_len, &param2);
        param->param3 = 0;
        app_bt_defer_call_in_bt_thread(func, param);
    }
    return true;
}

bool bt_defer_curr_func_4_impl(uintptr_t func, struct bt_defer_param_t param0,
        struct bt_defer_param_t param1, struct bt_defer_param_t param2,
        struct bt_defer_param_t param3, bool direct_call_func)
{
    uint16_t curr_len = sizeof(struct app_bt_alloc_param_t);
    if (app_bt_is_besbt_thread())
    {
        if (direct_call_func)
        {
            bt_defer_direct_call_func(param0.param, param1.param, param2.param, param3.param, func);
        }
        return false;
    }
    if (param0.is_fixed_param && param1.is_fixed_param && param2.is_fixed_param && param3.is_fixed_param)
    {
        app_bt_call_func_in_bt_thread(param0.param, param1.param, param2.param, param3.param, (uint32_t)(uintptr_t)func);
    }
    else
    {
        struct app_bt_alloc_param_t *param = NULL;
        param = (struct app_bt_alloc_param_t *)btif_cobuf_malloc(curr_len + co_round_size(param0.alloc_size) +
            co_round_size(param1.alloc_size) + co_round_size(param2.alloc_size) + co_round_size(param3.alloc_size));
        param->param0 = bt_defer_get_param(param, &curr_len, &param0);
        param->param1 = bt_defer_get_param(param, &curr_len, &param1);
        param->param2 = bt_defer_get_param(param, &curr_len, &param2);
        param->param3 = bt_defer_get_param(param, &curr_len, &param3);
        app_bt_defer_call_in_bt_thread(func, param);
    }
    return true;
}

int app_bt_GATT_Connect(bt_bdaddr_t *Addr)
{
#ifdef __GATT_OVER_BR_EDR__
    TRACE(1,"%s send gatt connect req",__func__);

    APP_BT_MAIL* mail;
    app_bt_mail_alloc(&mail);
    mail->src_thread = (uint32_t)osThreadGetId();
    mail->request_id = GATT_Connect_Req;
    mail->param.GATT_param.Addr = *Addr;
    app_bt_mail_send(mail);
#endif

    return 0;
}

