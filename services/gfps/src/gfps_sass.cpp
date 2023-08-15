/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#ifdef SASS_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bt_if.h"
#include "app_bt.h"
#include "app_bt_func.h"
#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_if.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#endif
#include "bluetooth.h"
#include "app_media_player.h"
#include "gfps_sass.h"
#include "app_ble_mode_switch.h"
#include "app_hfp.h"
#include "app_gfps.h"
#include "app_bt_media_manager.h"
#include "app_fp_rfcomm.h"
#include "gfps_crypto.h"
#include "../../utils/encrypt/aes.h"
#include "nvrecord_fp_account_key.h"

SassConnInfo sassInfo;
SassAdvInfo sassAdv;

#if defined(IBRT) && !defined(FREEMAN_ENABLED)
void gfps_sass_get_sync_info(uint8_t *buf, uint16_t *len)
{
    SassConnInfo *info = (SassConnInfo *)buf;
    memcpy((uint8_t *)info, (uint8_t *)&sassInfo, sizeof(SassConnInfo));
    *len = sizeof(SassConnInfo);
}

void gfps_sass_set_sync_info(uint8_t *buf, uint16_t len)
{
    SassConnInfo *info = (SassConnInfo *)buf;
    SassBtInfo temp[BT_DEVICE_NUM];

    info->headState = sassInfo.headState;
    memcpy((uint8_t *)&temp, (uint8_t *)&(sassInfo.connInfo), sizeof(SassBtInfo) * BT_DEVICE_NUM);
    memcpy((uint8_t *)&sassInfo, buf, sizeof(SassConnInfo));

    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if (info->connInfo[i].connId != 0xFF)
        {
            for(int j= 0; j < BT_DEVICE_NUM; j++)
            {
                if (!memcmp((void *)temp[j].btAddr.address, \
                    (void *)info->connInfo[i].btAddr.address, sizeof(bt_bdaddr_t)))
                {
                    TRACE(3, "%s master id:%d, slave id:%d", __func__, sassInfo.connInfo[j].connId, temp[j].connId);
                    sassInfo.connInfo[j].connId = temp[j].connId;
                    break;
                }
            }
        }
    }
}

void gfps_sass_sync_info()
{
    SassConnInfo info = {0};
    uint16_t len = 0;
    gfps_sass_get_sync_info((uint8_t *)&info, &len);
    tws_ctrl_send_cmd(APP_TWS_CMD_SEND_SASS_INFO, (uint8_t *)&info, len);

    TRACE(2, "%s len:%d", __func__, len);
}

void gfps_sass_role_switch_prepare()
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
    {
        gfps_sass_sync_info();
    }
}
#endif

void gfps_sass_init()
{
    memset((void *)&sassInfo, 0, sizeof(SassConnInfo));
    sassInfo.reconnInfo.evt = 0xFF;
    sassInfo.activeId = 0xFF;
    sassInfo.headState = SASS_HEAD_STATE_OFF;
    sassInfo.connAvail = SASS_CONN_AVAILABLE;
    sassInfo.preference = SASS_HFP_VS_A2DP_BIT;
    sassInfo.hunId = 0xFF;
#ifdef IBRT_V2_MULTIPOINT
    sassInfo.isMulti = true;
#endif
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        sassInfo.connInfo[i].connId = 0xFF;
        sassInfo.connInfo[i].devType = SASS_DEV_TYPE_INVALID;
    }
    INIT_LIST_HEAD(&sassInfo.hDevHead);

    //for (int i = 0; i < HISTORY_DEV_NUM; ++i) {
    //    colist_addto_head(&(sassInfo.historyDev[i].node), &sassInfo.hDevHead);
    //}

    memset((void *)&sassAdv, 0, sizeof(SassAdvInfo));
    sassAdv.lenType = (3 << 4) + SASS_CONN_STATE_TYPE;
    SET_SASS_STATE(sassAdv.state, HEAD_ON, 1);
    SET_SASS_STATE(sassAdv.state, CONN_AVAILABLE, 1);

    TRACE(1, "%s", __func__);
}

void gfps_sass_gen_session_nonce(uint8_t device_id)
{
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId == device_id)
        {
            for(int n = 0; n < SESSION_NOUNCE_NUM; n++)
            {
                 sassInfo.connInfo[i].session[n] = (uint8_t)rand();
            }
            TRACE(1, "sass dev %d session nounce is:", device_id);
            DUMP8("%02x ", sassInfo.connInfo[i].session, 8);
            break;
        }
    }
}

bool gfps_sass_get_session_nonce(uint8_t device_id, uint8_t *session)
{
    bool ret = false;
    uint8_t zero[SESSION_NOUNCE_NUM] = {0};
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if ((sassInfo.connInfo[i].connId == device_id) && (memcmp(sassInfo.connInfo[i].session, zero, SESSION_NOUNCE_NUM)))
        {
            memcpy(session, sassInfo.connInfo[i].session, SESSION_NOUNCE_NUM);
            ret = true;
            break;
        }
    }

    return ret;
}

SassBtInfo *gfps_sass_get_free_handler()
{
    SassBtInfo *handler = NULL;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId == 0xFF)
        {
            handler = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return handler;
}

SassBtInfo *gfps_sass_get_connected_dev(uint8_t id)
{
    SassBtInfo *handler = NULL;
    if (id == 0xFF)
    {
        return NULL;
    }

    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId == id)
        {
            handler = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return handler;
}

SassBtInfo *gfps_sass_get_connected_dev_by_addr(const bt_bdaddr_t *addr)
{
    SassBtInfo *handler = NULL;
    if (addr == NULL)
    {
        return NULL;
    }

    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if (!memcmp(sassInfo.connInfo[i].btAddr.address, addr->address, sizeof(bt_bdaddr_t)))
        {
            handler = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return handler;
}

SassBtInfo *gfps_sass_get_other_connected_dev(uint8_t id)
{
    SassBtInfo *info = NULL;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if ((sassInfo.connInfo[i].connId != id) && (sassInfo.connInfo[i].connId != 0xFF))
        {
            info = (SassBtInfo *)&(sassInfo.connInfo[i]);
            break;
        }
    }
    return info;
}

bool gfps_sass_is_other_sass_dev(uint8_t id)
{
    bool ret = false;
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(id);
    if (otherInfo)
    {
        ret = gfps_sass_is_sass_dev(otherInfo->connId);
    }
    return ret;
}

void gfps_sass_get_adv_data(uint8_t *buf, uint8_t *len)
{
    *len =  (sassAdv.lenType >> 4) + 1;
    memcpy(buf, (uint8_t *)&sassAdv, *len);
    TRACE(1, "sass adv data, len:%d", *len);
    DUMP8("%02x ", buf, *len);
}

#ifdef SASS_SECURE_ENHACEMENT
void gfps_sass_encrypt_adv_data(uint8_t *iv,  uint8_t *inUseKey, 
                                               uint8_t *outputData, uint8_t *dataLen, bool LT)
{
    uint8_t sassBuf[SASS_ADV_LEN_MAX] = {0};//{0x35, 0x85, 0x38, 0x09};
    uint8_t sassLen = 0;
    uint8_t hkdfKey[16] = {0};
    char str[13] = "SASS-RRD-KEY";

    inUseKey[0] = NONE_IN_USET_ACCOUNT_KEY_HEADER;
    // HKDF(accountkey, NULL, UTF8("SASS-RRD-KEY"), 16);
    gfps_hdkf(NULL, 0, inUseKey, 16, (uint8_t *)str, 12, hkdfKey, 16);
    TRACE(0, "gfps_hdkf is:");
    DUMP8("%2x ", hkdfKey, 16);

    gfps_sass_get_adv_data(sassBuf, &sassLen);
    if (LT)
    {
        AES128_CTR_encrypt_buffer(sassBuf, sassLen, hkdfKey, iv, &(outputData[1]));
        outputData[0] = (sassLen << 4) + APP_GFPS_RANDOM_RESOLVABLE_DATA_TYPE;
        *dataLen = (sassLen + 1);
    }
    else
    {
        AES128_CTR_encrypt_buffer(sassBuf + 1, sassLen-1, hkdfKey, iv, outputData);
        *dataLen = (sassLen -1);
    }
    TRACE(1, "len is:%d, encrypt connection state is:", *dataLen);
    DUMP8("%2x ", outputData, *dataLen);

}
#else
void gfps_sass_encrypt_adv_data(uint8_t *FArray, uint8_t sizeOfFilter, uint8_t *inUseKey, 
                                               uint8_t *outputData, uint8_t *dataLen)
{
    uint8_t sassBuf[4] = {0};//{0x35, 0x85, 0x38, 0x09};
    uint8_t outBuf[16 + 1] = {0};
    uint8_t iv[16] = {0}; //{0x8c, 0xa9, 0X0c, 0x08, 0x1c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sassLen = 0;
    uint8_t tempLen = *dataLen;
    gfps_sass_get_adv_data(sassBuf, &sassLen);
    memcpy(iv, FArray, sizeOfFilter);
    AES128_CTR_encrypt_buffer(sassBuf, sassLen, inUseKey, iv, outBuf + 1);
    TRACE(0, "encrypt connection state is:");
    DUMP8("%2x ", outBuf + 1, sassLen);
    outBuf[0] = (sassLen << 4) + APP_GFPS_RANDOM_RESOLVABLE_DATA_TYPE;
    memcpy(outputData + tempLen, outBuf, sassLen + 1);
    tempLen += (sassLen + 1);
    *dataLen = tempLen;
}

#endif

void gfps_sass_update_adv_data()
{
    uint8_t tempDev = 0;
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId != 0xFF)
        {
            TRACE(3, "%s device %d devtype:0x%0x", __func__, i, sassInfo.connInfo[i].devType);
            if ((tempDev & SASS_DEV_TYPE_PHONEA) && \
                (sassInfo.connInfo[i].devType == SASS_DEV_TYPE_PHONEA))
            {
                tempDev |= SASS_DEV_TYPE_PHONEB;
            }
            else if (sassInfo.connInfo[i].devType != SASS_DEV_TYPE_INVALID)
            {
                tempDev |= sassInfo.connInfo[i].devType;
            }else
            {
            }
        }
    }
    sassAdv.devBitMap = tempDev;

    sassAdv.state = 0;
    SET_SASS_STATE(sassAdv.state, HEAD_ON, sassInfo.headState);
    SET_SASS_STATE(sassAdv.state, CONN_AVAILABLE, sassInfo.connAvail);
    SET_SASS_STATE(sassAdv.state, FOCUS_MODE, sassInfo.focusMode);
    SET_SASS_STATE(sassAdv.state, AUTO_RECONN, sassInfo.autoReconn);
    SET_SASS_CONN_STATE(sassAdv.state, CONN_STATE, sassInfo.connState);
}

void gfps_sass_set_sass_mode(uint8_t device_id, uint16_t sassVer, bool state)
{
    SassBtInfo *sInfo;
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        sInfo = &(sassInfo.connInfo[i]);
        if (sInfo->connId == device_id)
        {
            sInfo->sassVer = sassVer;
            sInfo->sassState = state;
            TRACE(2, "sass ver:0x%4x, state:%d", sassVer, state);
            if (!state)
            {
                if (!memcmp(sassInfo.inUseAddr.address, sInfo->btAddr.address, sizeof(bt_bdaddr_t)))
                {
                    uint8_t keyCount = nv_record_fp_account_key_count();
                    NV_FP_ACCOUNT_KEY_ENTRY_T keyInfo;
                    for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
                    {
                        nv_record_fp_account_key_info(keyCount - 1 - keyIndex, &keyInfo);
                        if (memcmp(keyInfo.addr, sInfo->btAddr.address, sizeof(bt_bdaddr_t)))
                        {
                            gfps_sass_set_inuse_acckey(keyInfo.key, (bt_bdaddr_t *)&(keyInfo.addr));
                            break;
                        }
                    }
                }
            }
            break;
        }
   }
}

void gfps_sass_set_multi_status(const bt_bdaddr_t               *addr, bool isMulti)
{
    SassEvtParam evtParam;
    SASS_CONN_AVAIL_E availstate;
    uint8_t num = app_ibrt_if_get_connected_mobile_count();
    uint8_t total = isMulti ? BT_DEVICE_NUM : 1;
    TRACE(3, "%s num:%d, total:%d", __func__, num, total);
    sassInfo.isMulti = isMulti;
    if (addr)
    {
        evtParam.devId = SET_BT_ID(app_bt_get_device_id_byaddr((bt_bdaddr_t *)addr));
    }
    else
    {
        evtParam.devId = 0;
    }

    evtParam.event = SASS_EVT_UPDATE_MULTI_STATE;
    if (num >= total)
    {
        availstate = SASS_CONN_NONE_AVAILABLE;
    }
    else
    {
        availstate = SASS_CONN_AVAILABLE;
    }
    evtParam.state.connAvail = availstate;
    gfps_sass_update_state(&evtParam);
}

bool gfps_sass_get_multi_status()
{
    TRACE(1, "sass is in multi-point state ? %d", sassInfo.isMulti);
    return sassInfo.isMulti;
}

void gfps_sass_switch_max_link(uint8_t device_id, uint8_t type)
{
    bool leAud = false;
#if BLE_AUDIO_ENABLED
    leAud = true;
#endif
    bt_bdaddr_t *mobile_addr = &(app_bt_get_device(GET_BT_ID(device_id))->remote);

    TRACE(3,"%s, type:%d, id:0x%0x", __func__, type, device_id);
    if ((type == SASS_LINK_SWITCH_TO_SINGLE_POINT) && (app_ibrt_if_get_connected_mobile_count() > 1))
    {
        struct BT_DEVICE_T *revDevice = NULL;
        for(int i = 0; i < BT_DEVICE_NUM; i++)
        {
            revDevice = app_bt_get_device(i);
            if (revDevice->acl_is_connected)
            {
                AppIbrtCallStatus callState;
                AppIbrtA2dpState a2dpState;
                app_ibrt_if_get_a2dp_state(&(revDevice->remote), &a2dpState);
                app_ibrt_if_get_hfp_call_status(&(revDevice->remote), &callState);
                if ((callState != APP_IBRT_IF_NO_CALL) || (a2dpState == APP_IBRT_IF_A2DP_STREAMING))
                {
                    mobile_addr = &(revDevice->remote);
                    break;
                }
            }
        }
    }
    
    if (type == SASS_LINK_SWITCH_TO_MULTI_POINT)
    {
        app_ui_change_mode_ext(leAud, true, mobile_addr);
    }
    else
    {
        app_ui_change_mode_ext(leAud, false, mobile_addr);
    }
}

bool gfps_sass_is_any_dev_connected()
{
    bool ret = false;
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId != 0xFF) {
            ret = true;
            break;
        }
    }
    return ret;
}

bool gfps_sass_is_sass_dev(uint8_t device_id)
{
    bool ret = false;
    uint8_t key[FP_ACCOUNT_KEY_SIZE] = {0};
    SassBtInfo *sInfo;

    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        sInfo = &(sassInfo.connInfo[i]);
        if (sInfo->connId == device_id)
        {
            if (sInfo->sassVer && (sInfo->sassState ||  memcmp(sInfo->accKey, key, FP_ACCOUNT_KEY_SIZE)))
            {
                ret = true;
            }
            break;
        }
    }
    return ret;
}

bool gfps_sass_is_there_sass_dev()
{
    bool ret = false;
    SassBtInfo *sInfo;
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        sInfo = &(sassInfo.connInfo[i]);
        if (sInfo->sassVer && \
            (sInfo->sassState ||  (sInfo->accKey[0] == SASS_NOT_IN_USE_ACCOUNT_KEY)))
        {
            ret = true;
            break;
        }
    }

    TRACE(1, "there is a sass dev ? %d", ret);
    return ret;
}

void gfps_sass_set_custom_data(uint8_t data)
{
    sassAdv.cusData = data;
}

void gfps_sass_get_cap(uint8_t *buf, uint32_t *len)
{
    uint16_t sass_ver = SASS_VERSION;
    uint16_t capbility = 0;

    capbility |= SASS_STATE_ON_BIT;
#ifdef IBRT_V2_MULTIPOINT
    capbility |= SASS_MULTIPOINT_BIT;
#endif
    if (gfps_sass_get_multi_status())
    {
        capbility |= SASS_MULTIPOINT_ON_BIT;
    }
    capbility |= SASS_ON_HEAD_BIT;
    if (gfps_sass_get_head_state())
    {
        capbility |= SASS_ON_HEAD_ON_BIT;
    }
    *len = NTF_CAP_LEN;
    buf[0] = (sass_ver >> 8) & 0xFF;
    buf[1] = (sass_ver & 0xFF);
    buf[2] = (capbility >> 8) & 0xFF;
    buf[3] = (capbility & 0xFF);
}

void gfps_sass_set_switch_pref(uint8_t pref)
{
    sassInfo.preference = pref;
}

uint8_t gfps_sass_get_switch_pref()
{
    TRACE(1, "set reconnect dev %d", sassInfo.preference);

    return sassInfo.preference;
}

void gfps_sass_set_active_dev(uint8_t device_id)
{
    if (sassInfo.activeId != device_id)
    {
        sassInfo.activeId = device_id;
    }
}

void gfps_sass_update_active_info(uint8_t device_id)
{
    uint8_t acc[FP_ACCOUNT_KEY_SIZE] = {0};
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);

    gfps_sass_set_active_dev(device_id);
    if (sInfo && memcmp(sInfo->accKey, acc, FP_ACCOUNT_KEY_SIZE) && \
        gfps_sass_is_sass_dev(device_id))
    {
        gfps_sass_set_inuse_acckey(sInfo->accKey, &(sInfo->btAddr));
    }
}

uint8_t gfps_sass_get_active_dev()
{
    return sassInfo.activeId;
}

SassBtInfo *gfps_sass_get_inactive_dev()
{
    SassBtInfo *sInfo = NULL;
    for (int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if ((sassInfo.connInfo[i].connId != 0xFF) && \
            (sassInfo.activeId != sassInfo.connInfo[i].connId))
        {
            sInfo = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return sInfo;
}


void gfps_sass_set_reconnecting_dev(bt_bdaddr_t *addr, uint8_t evt)
{
    memcpy(sassInfo.reconnInfo.reconnAddr.address, addr->address, sizeof(bt_bdaddr_t));
    sassInfo.reconnInfo.evt = evt;
    TRACE(0, "set reconnect dev");
}

bool gfps_sass_is_reconn_dev(bt_bdaddr_t *addr)
{
    bool ret = false;
    if (!memcmp(sassInfo.reconnInfo.reconnAddr.address, addr->address, sizeof(bt_bdaddr_t)))
    {
        ret = true;
    }
    TRACE(1, "sass is reconnect dev ? %d", ret);
    return ret;
}

uint8_t gfps_sass_get_hun_flag()
{
    return sassInfo.hunId;
}

void gfps_sass_set_hun_flag(uint8_t device_id)
{
    sassInfo.hunId = device_id;
}

bool gfps_sass_is_need_resume(bt_bdaddr_t *addr)
{
    bool ret = false;
    if (!memcmp(sassInfo.reconnInfo.reconnAddr.address, addr->address, sizeof(bt_bdaddr_t)) && \
        (sassInfo.reconnInfo.evt == SASS_EVT_SWITCH_BACK_AND_RESUME))
    {
        ret = true;
    }
    return ret;
}

bool gfps_sass_is_profile_connected(bt_bdaddr_t *addr)
{
    bool ret = false;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if (!memcmp(sassInfo.connInfo[i].btAddr.address, addr->address, sizeof(bt_bdaddr_t)))
        {
            if (GET_PROFILE_STATE(sassInfo.connInfo[i].audState, CONNECTION, A2DP) && \
                GET_PROFILE_STATE(sassInfo.connInfo[i].audState, CONNECTION, HFP) && \
                GET_PROFILE_STATE(sassInfo.connInfo[i].audState, CONNECTION, AVRCP))
            {
                ret = true;
            }
            break;
        }
    }

    TRACE(1, "sass is profile connected ? %d", ret);
    return ret;
}

void gfps_sass_reset_reconn_info()
{
    memset((uint8_t *)&(sassInfo.reconnInfo), 0, sizeof(sassInfo.reconnInfo));
    TRACE(0, "reset sass reconn info");
    return;
}

void gfps_sass_update_last_dev(bt_bdaddr_t *addr)
{
    if (addr)
    {
        memcpy(sassInfo.lastDev.address, addr->address, sizeof(bt_bdaddr_t));
    }
}

void gfps_sass_clear_last_dev()
{
    memset(sassInfo.lastDev.address, 0, sizeof(bt_bdaddr_t));
}

void gfps_sass_get_last_dev(bt_bdaddr_t *lastAddr)
{
    memcpy(lastAddr->address, sassInfo.lastDev.address, sizeof(bt_bdaddr_t));
}

void gfps_sass_set_pending_proc(SassPendingProc *pending)
{
    memcpy(&sassInfo.pending, pending, sizeof(SassPendingProc));
}

void gfps_sass_get_pending_proc(SassPendingProc *pending)
{
    memcpy(pending, &sassInfo.pending, sizeof(SassPendingProc));
}

void gfps_sass_clear_pending_proc()
{
    memset(&sassInfo.pending, 0, sizeof(SassPendingProc));
}

void gfps_sass_switch_a2dp(uint8_t awayId, uint8_t destId, bool update)
{
    SassBtInfo *awayInfo = gfps_sass_get_connected_dev(awayId);
    if (awayInfo && GET_PROFILE_STATE(awayInfo->audState, AUDIO, AVRCP))
    {
        app_ibrt_if_a2dp_send_pause(GET_BT_ID(awayId));
    }
    else
    {
        app_ibrt_if_switch_streaming_a2dp();
    }
    gfps_sass_update_active_info(destId);

    if (update)
    {
        SassEvtParam evtParam;
        evtParam.devId = awayId;
        evtParam.event = SASS_EVT_UPDATE_ACTIVE_DEV;
        gfps_sass_update_state(&evtParam);
    }
}

SASS_CONN_STATE_E gfps_sass_get_conn_state()
{
    return sassInfo.connState;
}

void gfps_sass_set_conn_state(SASS_CONN_STATE_E state)
{
    TRACE(1, "set sass conn state:0x%0x", state);

    sassInfo.connState = state;
}

SASS_HEAD_STATE_E gfps_sass_get_head_state()
{
    return sassInfo.headState;
}

void gfps_sass_set_head_state(SASS_HEAD_STATE_E headstate)
{
    sassInfo.headState = headstate;
}

void gfps_sass_set_conn_available(SASS_CONN_AVAIL_E available)
{
    sassInfo.connAvail= available;
}

void gfps_sass_set_focus_mode(SASS_FOCUS_MODE_E focus)
{
    sassInfo.focusMode = focus;
}

void gfps_sass_set_auto_reconn(SASS_AUTO_RECONN_E focus)
{
    sassInfo.autoReconn = focus;
}

void gfps_sass_set_init_conn(uint8_t device_id, bool bySass)
{
    SassBtInfo *info = gfps_sass_get_connected_dev(device_id);
    if (info)
    {
        info->initbySass = bySass;
    }
}

void gfps_sass_set_inuse_acckey(uint8_t *accKey, bt_bdaddr_t *addr)
{
    if (accKey)
    {
        memcpy(sassInfo.inuseKey, accKey, FP_ACCOUNT_KEY_SIZE);
    }

    if (addr)
    {
        memcpy(sassInfo.inUseAddr.address, addr->address, sizeof(bt_bdaddr_t));
    }
}

void gfps_sass_set_inuse_acckey_by_dev(uint8_t device_id, uint8_t *accKey)
{
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        gfps_sass_set_inuse_acckey(accKey, &(sInfo->btAddr));

        memcpy(sInfo->accKey, accKey, FP_ACCOUNT_KEY_SIZE);
        sInfo->updated = true;
    }
    TRACE(1, "sass dev %d inuse acckey is:", device_id);
    DUMP8("0x%2x ", accKey, FP_ACCOUNT_KEY_SIZE);
}

bool gfps_sass_get_inuse_acckey(uint8_t *accKey)
{
    if (sassInfo.inuseKey[0] != SASS_NOT_IN_USE_ACCOUNT_KEY)
    {
        return false;
    }
    else
    {
        memcpy(accKey, sassInfo.inuseKey, FP_ACCOUNT_KEY_SIZE);
        return true;
    }
}

bool gfps_sass_get_inuse_acckey_by_id(uint8_t device_id, uint8_t *accKey)
{
    bool ret = false;
    uint8_t key[FP_ACCOUNT_KEY_SIZE] = {0};
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo && memcmp(sInfo->accKey, key, FP_ACCOUNT_KEY_SIZE))
    {
        memcpy(accKey, sInfo->accKey, FP_ACCOUNT_KEY_SIZE);
        ret = true;
    }
    TRACE(1, "get sass dev %d inuse acckey is:", device_id);
    DUMP8("0x%2x ", accKey, FP_ACCOUNT_KEY_SIZE);
    return ret;
}

void gfps_sass_set_drop_dev(uint8_t device_id)
{
    SassBtInfo *info = gfps_sass_get_connected_dev(device_id);
    memcpy(sassInfo.dropDevAddr.address, info->btAddr.address, sizeof(bt_bdaddr_t));
}

SASS_DEV_TYPE_E gfps_sass_get_dev_type_by_cod(uint8_t *cod)
{
    SASS_DEV_TYPE_E type;
    uint32_t devCod = cod[0] + (cod[1] << 8) + (cod[2] << 16);
    TRACE(4, "%s type: 0x%02x%02x%02x", __func__, cod[0], cod[1], cod[2]);

    if ((devCod & COD_TYPE_PHONE) == COD_TYPE_PHONE)
    {
        type = SASS_DEV_TYPE_PHONEA;
    }
    else if ((devCod & COD_TYPE_LAPTOP) == COD_TYPE_LAPTOP)
    {
        type = SASS_DEV_TYPE_LAPTOP;
    }
    else if ((devCod & COD_TYPE_TABLET) == COD_TYPE_TABLET)
    {
        type = SASS_DEV_TYPE_TABLET;
    }
    else if ((devCod & COD_TYPE_TV) == COD_TYPE_TV)
    {
        type = SASS_DEV_TYPE_TV;
    }
    else
    {
        type = SASS_DEV_TYPE_PHONEA;//SASS_DEV_TYPE_INVALID;
    }

    return type;
}

SASS_CONN_STATE_E gfps_sass_get_state(uint8_t devId, SASS_CONN_STATE_E entry)
{
    SASS_CONN_STATE_E state;
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(devId);
    TRACE(3, "activeId:%d, devId:%d, entry:%d", sassInfo.activeId, devId, entry);

    if (otherInfo)
    {
        if ((entry == SASS_STATE_NO_CONNECTION) && (sassInfo.activeId == devId))
        {
            sassInfo.activeId = otherInfo->connId;
            TRACE(2, "change activeId to:%d as devId:%d disconnect", sassInfo.activeId, devId);
        }

        if (sassInfo.activeId == otherInfo->connId) {
            state = otherInfo->state;
        }
        else if (sassInfo.activeId == devId) {
            state = entry;
        }
        else if (otherInfo->state > SASS_STATE_NO_AUDIO || entry > SASS_STATE_NO_AUDIO)
        {
            if (otherInfo->state > entry)
            {
                state = otherInfo->state;
                sassInfo.activeId = otherInfo->connId;
            }else {
                state = entry;
                sassInfo.activeId = devId;
            }
        }
        else
        {
            state = (otherInfo->state > entry) ? otherInfo->state : entry;
        }
    }
    else
    {
        state = entry;
    }

    return state;
}

SASS_CONN_STATE_E gfps_sass_update_conn_state(uint8_t devId)
{
    SASS_CONN_STATE_E connState = SASS_STATE_NO_CONNECTION;
    SassBtInfo *info = gfps_sass_get_connected_dev(devId);
    if (info)
    {
        if (GET_PROFILE_STATE(info->audState, AUDIO, HFP) && \
            GET_PROFILE_STATE(info->audState, CONNECTION, HFP)) {
            info->state = SASS_STATE_HFP;
        }else if (GET_PROFILE_STATE(info->audState, AUDIO, AVRCP) && \
            GET_PROFILE_STATE(info->audState, CONNECTION, AVRCP)) {
            info->state = SASS_STATE_A2DP_WITH_AVRCP;
        }else if (GET_PROFILE_STATE(info->audState, AUDIO, A2DP) && \
            GET_PROFILE_STATE(info->audState, CONNECTION, A2DP)) {
            info->state = SASS_STATE_ONLY_A2DP;
        }else if (GET_PROFILE_STATE(info->audState, CONNECTION, HFP) || \
            GET_PROFILE_STATE(info->audState, CONNECTION, A2DP)  || \
            GET_PROFILE_STATE(info->audState, CONNECTION, AVRCP)) {
            info->state = SASS_STATE_NO_DATA;
        }else {
            info->state = SASS_STATE_NO_CONNECTION;
        }
    }else {
        info->state = SASS_STATE_NO_CONNECTION;
    }
    connState = gfps_sass_get_state(devId, info->state);
    TRACE(2, "update profile state:0x%0x, info->state:%d", connState, info->state);
    return connState;
}

void gfps_sass_check_if_need_reconnect()
{
    uint8_t invalidAddr[6] = {0};
    bt_bdaddr_t *reconnAddr = &(sassInfo.reconnInfo.reconnAddr);
#ifdef IBRT
    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
#endif
    {
        if (memcmp(reconnAddr->address, invalidAddr, sizeof(bt_bdaddr_t)) && \
            !app_bt_is_acl_connected_byaddr(reconnAddr))
        {
            TRACE(1, "%s try to reconnect dev", __func__);
            DUMP8("%02x ", reconnAddr->address, 6);
            app_ui_choice_mobile_connect((bt_bdaddr_t *)&(sassInfo.reconnInfo.reconnAddr));
        }
    }
}

void gfps_sass_check_if_need_hun(uint8_t device_id, uint8_t event)
{
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(device_id);
    if (otherInfo == NULL)
    {
        return;
    }

    uint8_t activeId = gfps_sass_get_active_dev();

    if ((activeId != 0xFF) && (activeId == otherInfo->connId))
    {
        switch(event)
        {
            case BTIF_HF_EVENT_AUDIO_CONNECTED:
                if (GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP))
                {
                    gfps_sass_set_hun_flag(device_id);
                }
                break;

            default:
                break;
        }
    }
}

void gfps_sass_get_acckey_from_nv(uint8_t *addr, uint8_t *key)
{
    nv_record_fp_get_key_by_addr(addr, key);
}

void gfps_sass_set_need_ntf_status(uint8_t device_id, bool en)
{
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        sInfo->needNtfStatus = en;
    }
}

bool gfps_sass_is_need_ntf_status(uint8_t device_id)
{
    bool ret = false;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        ret = sInfo->needNtfStatus;
    }
    return ret;
}

void gfps_sass_set_need_ntf_switch(uint8_t device_id, bool en)
{
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        sInfo->needNtfSwitch = en;
    }
}

bool gfps_sass_is_need_ntf_switch(uint8_t device_id)
{
    bool ret = false;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        ret = sInfo->needNtfSwitch;
    }
    return ret;
}

void gfps_sass_send_session_nonce(uint8_t device_id)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_DEVICE_INFO, FP_MSG_DEVICE_INFO_SESSION_NONCE, 0, 8};
    gfps_sass_get_session_nonce(device_id, req.data);
    gfps_send(device_id, (uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+8);
}

void gfps_sass_get_capability(uint8_t device_id)
{
    TRACE(1,"%s",__func__);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_GET_CAPBILITY,
         0,
         0};
    gfps_send(device_id, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN);
}

void gfps_sass_ntf_capability(uint8_t device_id)
{
    TRACE(2,"%s %d",__func__, device_id);
    uint32_t dataLen = 0;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_CAPBILITY};
    gfps_sass_get_cap(req.data, &dataLen);
    req.dataLenHighByte = (uint8_t)(dataLen & 0xFF00);
    req.dataLenLowByte = (uint8_t)(dataLen & 0xFF);
    gfps_send(device_id, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + dataLen);
}

void gfps_sass_set_capability(uint8_t device_id,uint8_t *data)
{
    TRACE(1,"%s",__func__);
    uint16_t sassVer =( data[0] << 8) | data[1];
    bool state = (data[2] & 0x80) ? true: false;
    gfps_sass_set_sass_mode(device_id, sassVer, state);
}

void gfps_sass_set_multipoint_hdl(uint8_t device_id, uint8_t *data)
{
    uint8_t enable = data[0];
    if(enable) {
        gfps_sass_switch_max_link(device_id, SASS_LINK_SWITCH_TO_MULTI_POINT);
    }else {
        gfps_sass_switch_max_link(device_id, SASS_LINK_SWITCH_TO_SINGLE_POINT);
    }

    TRACE(2,"%s swtich:%d",__func__, enable);
}

void gfps_sass_set_switch_pref_hdl(uint8_t device_id, uint8_t *data)
{
    uint8_t pref = data[0];
    TRACE(2,"%s pref:0x%0x",__func__, pref);
    gfps_sass_set_switch_pref(pref);
}

void gfps_sass_ntf_switch_pref(uint8_t device_id)
{
    TRACE(1,"%s",__func__);
    uint16_t dataLen = 2;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_SWITCH_PREFERENCE,
         0,
         2};
    req.data[0] = gfps_sass_get_switch_pref();
    req.data[1] = 0;
    gfps_send(device_id, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + dataLen);
}

bool gfps_sass_ntf_switch_evt(uint8_t device_id, uint8_t reason)
{
    uint32_t nameLen = 0;
    bool ret = true;
    uint8_t *namePtr = NULL;
    uint8_t activeId = gfps_sass_get_active_dev();

    namePtr = nv_record_fp_get_name_ptr(&nameLen);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_SWITCH_EVT,
         0,
         (uint8_t)(2+nameLen)};

    req.data[0] = reason;
    req.data[1] = (device_id == activeId) ? SASS_DEV_THIS_DEVICE : SASS_DEV_ANOTHER;
    if(namePtr)
    {
        DUMP8("%02x ", namePtr, nameLen);
        memcpy(req.data + 2, namePtr, nameLen);
    }
    TRACE(3,"sass_ntf_switch id:%d, activeId:%d, reason:%d", device_id, activeId, reason);

    ret = gfps_send(device_id, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + 2 + nameLen);
    if (ret == false)
    {
        gfps_sass_set_need_ntf_switch(device_id, true);
    }
    else
    {
        gfps_sass_set_need_ntf_switch(device_id, false);
    }
    return ret;
}

bool gfps_sass_ntf_conn_status(uint8_t device_id)
{
    uint8_t advlen, len, activeId;
    uint8_t account[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t outBuf[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t iv[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t memNonce[SESSION_NOUNCE_NUM] = {0};
    bool ret = true;
    SassBtInfo * otherInfo = NULL;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_CONN_STATUS,
         0,
         0};

    if (!gfps_sass_get_session_nonce(device_id, iv))
    {
        ret = false;
        goto NTF_EXIT;
    }

    if (!gfps_sass_get_inuse_acckey(account))
    {
        ret = false;
        goto NTF_EXIT;
    }
    TRACE(0, "in use account key is:");
    DUMP8("0x%2x ", account, FP_ACCOUNT_KEY_SIZE);

    for(int i = 0; i < SESSION_NOUNCE_NUM; i++)
    {
         memNonce[i] = (uint8_t)rand();
         iv[SESSION_NOUNCE_NUM + i] = memNonce[i];
    }

    activeId = gfps_sass_get_active_dev();
#ifdef SASS_SECURE_ENHACEMENT
    gfps_sass_encrypt_adv_data(iv, account, outBuf, &advlen, false);
#else
    uint8_t adv[5] = {0};
    gfps_sass_get_adv_data(adv, &advlen);
    AES128_CTR_encrypt_buffer(adv+1, advlen-1, account, iv, outBuf);
    advlen--;
#endif
    len = advlen + 1 + SESSION_NOUNCE_NUM;
    req.dataLenLowByte = len;

    otherInfo = gfps_sass_get_other_connected_dev(device_id);
    if (!otherInfo || activeId == device_id)
    {
        req.data[0] = SASS_DEV_IS_ACTIVE;
    }
    else if ((otherInfo && gfps_sass_is_sass_dev(otherInfo->connId)) || (activeId == 0xFF))
    {
        req.data[0] = SASS_DEV_IS_PASSIVE;
    }
    else
    {
        req.data[0] = SASS_DEV_IS_PSSIVE_WITH_NONSASS;
    }

    memcpy(req.data + 1, outBuf, advlen);
    memcpy(req.data + advlen + 1, memNonce, SESSION_NOUNCE_NUM);
    ret = gfps_send(device_id, (uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + len);  
    TRACE(3,"%s dev:%d ret:%d",__func__, device_id, ret);

NTF_EXIT:
    if (ret == false)
    {
        gfps_sass_set_need_ntf_status(device_id, true);
    }
    else
    {
        gfps_sass_set_need_ntf_status(device_id, false);
    }
    return ret;
}

SASS_REASON_E gfps_sass_get_switch_reason(SASS_CONN_STATE_E state)
{
    SASS_REASON_E reason;
    if (state == SASS_STATE_ONLY_A2DP || state == SASS_STATE_A2DP_WITH_AVRCP){
        reason = SASS_REASON_A2DP;
    }else if (state == SASS_STATE_HFP) {
        reason = SASS_REASON_HFP;
    }else {
        reason = SASS_REASON_UNSPECIFIED;
    }
    return reason;
}

void gfps_sass_update_switch_dev(uint8_t dev_id)
{
    SASS_CONN_STATE_E state = gfps_sass_get_conn_state();
    SASS_REASON_E reason = gfps_sass_get_switch_reason(state);
    SassBtInfo *otherInfo  = gfps_sass_get_other_connected_dev(dev_id);
    gfps_sass_ntf_switch_evt(dev_id, reason);

    if (otherInfo && (otherInfo->connId != 0xFF))
    {
        gfps_sass_ntf_switch_evt(otherInfo->connId, reason);
    }
}

void gfps_sass_get_conn_hdl(uint8_t device_id)
{
    gfps_sass_ntf_conn_status(device_id);
    TRACE(1,"%s",__func__);
}

void gfps_sass_set_init_conn(uint8_t device_id, uint8_t *data)
{
    bool isSass = data[0];
    gfps_sass_set_init_conn(device_id, isSass);
    TRACE(1,"connection is triggered by sass? %d", isSass);
}

void gfps_sass_ind_inuse_acckey(uint8_t device_id, uint8_t *data)
{
    char str[8] = "in-use";
    uint8_t auth[8] = {0};
    uint8_t nonce[16] = {0};
    uint8_t keyCount = 0;
    uint8_t accKey[16] = {0};
    uint8_t output[8] = {0};
    if (memcmp(data, str, 6))
    {
        TRACE(1, "%s data error!", __func__);
        return;
    }

    gfps_sass_get_session_nonce(device_id, nonce);
    memcpy(nonce + 8, data + 6, 8);
    memcpy(auth, data + 14, 8);

    keyCount = nv_record_fp_account_key_count();
    for (int i = 0; i < keyCount; i++)
    {
        nv_record_fp_account_key_get_by_index(i, accKey);
        gfps_encrypt_messasge(accKey, nonce, data, 6, output);
        if (!memcmp(output, auth, 8))
        {
            TRACE(2, "%s find account key index:%d", __func__, i);
            bt_bdaddr_t btAddr;
            uint8_t initAcc[FP_ACCOUNT_KEY_SIZE];
            gfps_sass_get_inuse_acckey(initAcc);
            gfps_sass_set_inuse_acckey_by_dev(device_id, accKey);
            if (app_bt_get_device_bdaddr(GET_BT_ID(device_id), btAddr.address))
            {
                nv_record_fp_update_addr(i, btAddr.address);
            }

            if (!gfps_sass_is_other_sass_dev(device_id) || \
                memcmp(initAcc, accKey, FP_ACCOUNT_KEY_SIZE) || \
                gfps_sass_is_need_ntf_status(device_id))
            {
                SassEvtParam evtParam;
                evtParam.devId = device_id;
                evtParam.event = SASS_EVT_UPDATE_INUSE_ACCKEY;
                gfps_sass_update_state(&evtParam);
            }

            if (gfps_sass_is_need_ntf_switch(device_id) && gfps_sass_get_multi_status())
            {
                gfps_sass_update_switch_dev(device_id);
            }
            break;
        }
    }
}

void gfps_sass_set_custom_data(uint8_t device_id, uint8_t *data)
{
    uint8_t param = data[0];
    uint8_t activeDev = gfps_sass_get_active_dev();
    if ((activeDev != 0xFF) && (activeDev != device_id))
    {
        return;
    }

    SassEvtParam evtParam;
    evtParam.event = SASS_EVT_UPDATE_CUSTOM_DATA;
    evtParam.devId = device_id;
    evtParam.state.cusData = param;
    gfps_sass_update_state(&evtParam);
}

void gfps_sass_set_drop_dev(uint8_t device_id, uint8_t *data)
{
    //drop this device
    if(data[0] == 1)
    {
        gfps_sass_set_drop_dev(device_id);
    }
}

void gfps_sass_switched_callback(uint8_t selectedId)
{
    SassBtInfo *sInfo = gfps_sass_get_other_connected_dev(selectedId);
    if (sInfo && (sInfo->connId != 0xFF) && app_bt_is_a2dp_streaming(GET_BT_ID(sInfo->connId)))
    {
       //app_ibrt_if_a2dp_send_pause(sInfo->connId);
       app_bt_ibrt_audio_pause_a2dp_stream(GET_BT_ID(sInfo->connId));
    }
}

uint8_t gfps_sass_switch_src(uint8_t device_id, uint8_t evt)
{
    uint8_t reason = SASS_STATUS_OK;
    uint8_t awayId, switchId, currentId, otherId = 0xFF;
    bool isDisconnect = false;
    SassBtInfo *info = gfps_sass_get_other_connected_dev(device_id);
    if (info)
    {
        otherId = info->connId;
    }

    if (evt & SASS_SWITCH_TO_CURR_DEV_BIT)
    {
        switchId = device_id;
        awayId = otherId;
    }
    else
    {
        switchId = otherId;
        awayId = device_id;
    }

    if (switchId != 0xFF)
    {
        currentId = SET_BT_ID(app_bt_audio_get_curr_playing_a2dp());
        TRACE(3, "sass switch src to %d from %d, other:%d, evt:0x%0x a2dp curr id:%d", switchId, awayId, otherId, evt, currentId);

        if (currentId == switchId) {
            return SASS_STATUS_REDUNTANT;
        } else {
            if (evt & SASS_DISCONN_ON_AWAY_DEV_BIT)
            {
                SassBtInfo *awayinfo = gfps_sass_get_connected_dev(awayId);
                if (awayinfo)
                {
                    gfps_sass_update_last_dev(&(awayinfo->btAddr));
                    app_ibrt_if_disconnet_moblie_device((bt_bdaddr_t *)&(awayinfo->btAddr));
                }
                isDisconnect = true;
            }
            else if(app_bt_is_a2dp_streaming(GET_BT_ID(switchId)))
            {
                uint8_t oldActive = gfps_sass_get_active_dev();
                gfps_sass_switch_a2dp(currentId, switchId, true);
                if (gfps_sass_get_multi_status() && (oldActive != 0xFF) && (switchId != oldActive))
                {
                    gfps_sass_update_switch_dev(switchId);
                }
                goto set_hf;
            }
            else
            {
            }

            if (evt & SASS_RESUME_ON_SWITCH_DEV_BIT)
            {
                //app_bt_ibrt_audio_pause_a2dp_stream(awayId);
                //app_bt_ibrt_audio_play_a2dp_stream(switchId);
                //app_bt_resume_music_player(switchId);
                if (!isDisconnect && (0xFF != awayId) && app_bt_is_a2dp_streaming(GET_BT_ID(awayId)))
                {
                    //app_ibrt_if_a2dp_send_pause(awayId);
                    app_bt_ibrt_audio_pause_a2dp_stream(GET_BT_ID(awayId));
                }
                //app_ibrt_if_a2dp_send_play(switchId);
                app_bt_ibrt_audio_play_a2dp_stream(GET_BT_ID(switchId));
            }
            else
            {
                SassPendingProc pending;
                pending.proc = SASS_PROCESS_SWITCH_ACTIVE_SRC;
                pending.param.activeId = switchId;
                gfps_sass_set_pending_proc(&pending);
                //app_bt_audio_stop_a2dp_playing(currentId);
            }
       }
    }
    else
    {
        if ((awayId != 0xFF) && app_bt_is_a2dp_streaming(GET_BT_ID(awayId)))
        {
            app_bt_audio_stop_a2dp_playing(GET_BT_ID(awayId));
        }
    }

set_hf:
    if (evt & SASS_REJECT_SCO_ON_AWAY_DEV_BIT)
    {
        app_bt_hf_set_reject_dev(GET_BT_ID(awayId));
    }
    return reason;
}

uint8_t gfps_sass_switch_back(uint8_t device_id, uint8_t evt)
{
    bt_bdaddr_t currAddr, lastAddr;
	uint8_t empty[6] = {0};

    if ((evt != SASS_EVT_SWITCH_BACK) && (evt != SASS_EVT_SWITCH_BACK_AND_RESUME))
    {
        return  SASS_STATUS_FAIL;
    }

    app_bt_get_device_bdaddr(GET_BT_ID(device_id), currAddr.address);
    gfps_sass_get_last_dev(&lastAddr);

    if (!memcmp(lastAddr.address, empty, sizeof(bt_bdaddr_t)))
    {
        TRACE(0, "sass switch back hdl last dev is NULL!");
	}
    else if (memcmp(lastAddr.address, currAddr.address, sizeof(bt_bdaddr_t)))
    {
        TRACE(0, "sass switch back to dev:");
        DUMP8("%02x ", lastAddr.address, 6);
        if (!app_bt_is_acl_connected_byaddr((bt_bdaddr_t *)&lastAddr))
        {
            uint8_t maxLink = gfps_sass_get_multi_status() ? BT_DEVICE_NUM : 1;            
            if (app_bt_count_connected_device() >= maxLink)
            {
                SassBtInfo *sInfo = gfps_sass_get_inactive_dev();
                bt_bdaddr_t *disAddr;
                gfps_sass_set_reconnecting_dev(&lastAddr, evt);
                if (sInfo)
                {                
                    disAddr = &(sInfo->btAddr);
                }
                else
                {
                    disAddr = &currAddr;
                }
                app_ibrt_if_disconnet_moblie_device(disAddr);
                TRACE(0, "disconnect dev:");
                DUMP8("%02x ", disAddr, 6);
            }
            else
            {
                app_ui_choice_mobile_connect((bt_bdaddr_t *)&lastAddr);
            }
        }
        else if (evt == SASS_EVT_SWITCH_BACK_AND_RESUME && gfps_sass_is_profile_connected(&lastAddr))
        {
            uint8_t lastId = app_bt_get_device_id_byaddr((bt_bdaddr_t *)&lastAddr);
            app_bt_resume_music_player(GET_BT_ID(lastId));
        }else {
            TRACE(0, "sass switch back already connect!!");
        }
    }
    else
    {
        TRACE(0, "sass switch back hdl disconnect itself");
        app_ibrt_if_disconnet_moblie_device((bt_bdaddr_t *)&currAddr);
    }

    return SASS_STATUS_OK;
}

void gfps_sass_switch_src_hdl(uint8_t device_id, uint8_t *data)
{
    uint8_t evt = data[0];
    uint8_t reason = SASS_STATUS_OK;
#ifdef IBRT
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
    {
        reason = gfps_sass_switch_src(device_id, evt);
    }
#endif

    if (reason == SASS_STATUS_OK)
    {
        gfps_send_msg_ack(device_id, FP_MSG_GROUP_SASS, FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }
    else if (reason == SASS_STATUS_REDUNTANT)
    {
        gfps_send_msg_nak(device_id, FP_MSG_NAK_REASON_REDUNDANT_ACTION, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }
    else
    {
        gfps_send_msg_nak(device_id, FP_MSG_NAK_REASON_NOT_ALLOWED, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }
    TRACE(3,"%s evt:0x%0x, reason:%d",__func__, evt, reason);
}

void gfps_sass_switch_back_hdl(uint8_t device_id, uint8_t *data)
{
    uint8_t evt = data[0];
    uint8_t reason = SASS_STATUS_OK;
#ifdef IBRT
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
    {
        reason = gfps_sass_switch_back(device_id, evt);
    }
#endif

    if (reason == SASS_STATUS_OK)
    {
        gfps_send_msg_ack(device_id, FP_MSG_GROUP_SASS, FP_MSG_SASS_SWITCH_BACK);
    }
    else
    {
        gfps_send_msg_nak(device_id, FP_MSG_NAK_REASON_NOT_ALLOWED, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_BACK);
    }

    //gfps_sass_ntf_switch_evt(device_id, SASS_REASON_UNSPECIFIED);
    TRACE(3,"%s evt:0x%0x, reason:%d",__func__, evt, reason);
}

void gfps_sass_profile_event_handler(SASS_PROFILE_ID_E pro, uint8_t devId, uint8_t event, uint8_t *param)
{
    bt_bdaddr_t addr;
    bool needUpdate = true;
    bool needResume = false;
    bool devIssass = false;
    bool otherIssass = false;
    SASS_CONN_STATE_E tempState;
    SASS_CONN_STATE_E updateState;
    uint8_t acc[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t oldActive = gfps_sass_get_active_dev();
    SassBtInfo *oldActInfo = gfps_sass_get_connected_dev(oldActive);
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(devId);
    if (!sInfo)
    {
        return;
    }
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(devId);
    if (otherInfo)
    {
        otherIssass = gfps_sass_is_sass_dev(otherInfo->connId);
    }
    app_bt_get_device_bdaddr(GET_BT_ID(devId), addr.address);
    needResume = gfps_sass_is_need_resume(&addr);
    devIssass = gfps_sass_is_sass_dev(devId);

    TRACE(5,"%s id:%d event:%d pro:%d avtiveId:%d", __func__, devId, event,pro,oldActive);

    if (pro == SASS_PROFILE_A2DP)
    {
        //SassBtInfo *otherInfo = NULL;
        SassPendingProc pending;
        uint8_t currId = SET_BT_ID(app_bt_audio_get_curr_playing_a2dp());
        switch(event)
        {
            case BTIF_A2DP_EVENT_STREAM_OPEN:
            case BTIF_A2DP_EVENT_STREAM_OPEN_MOCK:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, A2DP, 1);
                if (gfps_sass_get_active_dev() == 0xFF || \
                    (otherInfo && !GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) && \
                    !GET_PROFILE_STATE(otherInfo->audState, AUDIO, HFP) && \
                    !GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP)))
                {
                    //gfps_sass_set_active_dev(devId);
                }
                break;

            case BTIF_A2DP_EVENT_STREAM_CLOSED:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, A2DP, 0);
                if (oldActive == devId)
                {
                    gfps_sass_set_active_dev(0xFF);
                }
                break;

            case BTIF_A2DP_EVENT_STREAM_STARTED:
            case BTIF_A2DP_EVENT_STREAM_STARTED_MOCK:
                if (!GET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP))
                {
                    SET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP, 1);
                    gfps_sass_get_pending_proc(&pending);
                    if ((currId != BT_DEVICE_INVALID_ID) && (currId != devId))
                        /*|| \
                        ((GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP) ||  \
                        !GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP)) && devIssass)*/
                    {
                        if (devIssass && (pending.proc == SASS_PROCESS_SWITCH_ACTIVE_SRC) && \
                            (pending.param.activeId == devId))
                        {
                            gfps_sass_switch_a2dp(currId, devId, false);
                            //app_bt_ibrt_audio_pause_a2dp_stream(currId);
                            gfps_sass_clear_pending_proc();
                            //gfps_sass_update_active_info(devId);
                        }
                        else if (!devIssass)
                        {
                            app_bt_ibrt_audio_pause_a2dp_stream(GET_BT_ID(devId));
                        }
                    }
                    else if (otherInfo && (GET_PROFILE_STATE(otherInfo->audState, AUDIO, HFP) || \
                        (!devIssass && GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP))))
                    {
                        app_bt_ibrt_audio_pause_a2dp_stream(GET_BT_ID(devId));
                        //app_ibrt_if_a2dp_send_pause(devId);
                    }
                    else if (otherInfo && GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) && \
                        GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP))
                    {
                        TRACE(1, "%s media cannot switch to game!!", __func__);
                    }
                    else
                    {
                        gfps_sass_update_active_info(devId);
                    }
                }
                break;

            case BTIF_A2DP_EVENT_STREAM_SUSPENDED:
                SET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP, 0);
                //if (oldActive == devId)
                //{
                //    gfps_sass_set_active_dev(0xFF);
                //}
                break;
            default:
                needUpdate = false;
                break;
        }
    }
    else if (pro == SASS_PROFILE_HFP)
    {
        switch(event)
        {
            case BTIF_HF_EVENT_CALL_IND:
            case BTIF_HF_EVENT_AUDIO_CONNECTED:
            case BTIF_HF_EVENT_RING_IND:
                if (((event == BTIF_HF_EVENT_CALL_IND) && param && param[0]) \
                    || (event != BTIF_HF_EVENT_CALL_IND))
                {
                    SET_PROFILE_STATE(sInfo->audState, AUDIO, HFP, 1);
                    if (otherInfo)
                    {
                        if (GET_PROFILE_STATE(otherInfo->audState, AUDIO, HFP))
                        {
                            app_ibrt_if_disconnet_moblie_device((bt_bdaddr_t *)&(sInfo->btAddr));
                        }
                        else if (GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) || \
                            GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP))
                        {
                            //app_ibrt_if_a2dp_send_pause(otherInfo->connId);
                            app_bt_ibrt_audio_pause_a2dp_stream(GET_BT_ID(otherInfo->connId));
                            gfps_sass_update_active_info(devId);
                        }
                        else
                        {
                             gfps_sass_update_active_info(devId);
                        }
                    }
                }
                else
                {
                    SET_PROFILE_STATE(sInfo->audState, AUDIO, HFP, 0);
                }
                break;

            case BTIF_HF_EVENT_AUDIO_DISCONNECTED:
                SET_PROFILE_STATE(sInfo->audState, AUDIO, HFP, 0);
                if (otherInfo && GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP))
                {
                    gfps_sass_set_active_dev(otherInfo->connId);
                    if (otherIssass && gfps_sass_get_inuse_acckey_by_id(otherInfo->connId, acc))
                    {
                        gfps_sass_set_inuse_acckey(acc, &addr);
                    }
                }
                break;

            case BTIF_HF_EVENT_SERVICE_DISCONNECTED:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, HFP, 0);
                if (oldActive == devId)
                {
                    gfps_sass_set_active_dev(0xFF);
                }
                break;

            case BTIF_HF_EVENT_SERVICE_CONNECTED:
            case BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, HFP, 1);
                break;

            default:
            needUpdate = false;
            break;
        }
    }
    else if (pro == SASS_PROFILE_AVRCP)
    {
        switch(event)
        {
            case BTIF_AVCTP_CONNECT_EVENT:
            case BTIF_AVCTP_CONNECT_EVENT_MOCK:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, AVRCP, 1);
                break;

            case BTIF_AVCTP_DISCONNECT_EVENT:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, AVRCP, 0);
                break;

            case BTIF_AVRCP_EVENT_ADV_NOTIFY:
            case BTIF_AVRCP_EVENT_ADV_RESPONSE:
                TRACE(2, "%s avrcp state:%d", __func__, *param);
                if (*param == BTIF_AVRCP_MEDIA_PLAYING) {
                    SET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP, 1);             
                    if (otherInfo && (GET_PROFILE_STATE(otherInfo->audState, AUDIO, HFP) || \
                        ((otherIssass != devIssass) && GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP))))
                    {
                        app_bt_ibrt_audio_pause_a2dp_stream(GET_BT_ID(devId));
                        //app_ibrt_if_a2dp_send_pause(devId);
                    }
                }else if ((*param == BTIF_AVRCP_MEDIA_PAUSED || *param == BTIF_AVRCP_MEDIA_STOPPED) \
                    && GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP))
                {
                    SET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP, 0);
                }else {
                    needUpdate = false;
                }
                break;

            default:
                needUpdate = false;
                break;
        }
    }
    else
    {
        needUpdate = false;
        TRACE(1,"%s sass profile update error", __func__);
    }

    if (needUpdate)
    {
        uint8_t newActive = gfps_sass_get_active_dev();
        SassEvtParam evtParam;
        evtParam.devId = devId;

        tempState = gfps_sass_get_conn_state();
        updateState = gfps_sass_update_conn_state(devId);
        TRACE(4,"%s sass audState: 0x%0x, temp:0x%0x, state:0x%0x", __func__, sInfo->audState, tempState, updateState);

        if (tempState != updateState)
        {
            evtParam.event = SASS_EVT_UPDATE_CONN_STATE;
            evtParam.state.connState = updateState;
            gfps_sass_update_state(&evtParam);
        }
        else if ((tempState == updateState) && (oldActive != newActive))
        {
            evtParam.event = SASS_EVT_UPDATE_ACTIVE_DEV;
            gfps_sass_update_state(&evtParam);
        }
        else
        {
        }

        if (gfps_sass_get_multi_status() && (oldActive != newActive) && \
            (oldActive != 0xFF) && (newActive != 0xFF) && oldActInfo && \
            (GET_PROFILE_STATE(oldActInfo->audState, AUDIO, HFP) || \
            GET_PROFILE_STATE(oldActInfo->audState, AUDIO, A2DP) || \
            gfps_sass_get_hun_flag() == newActive))
        {
             gfps_sass_update_switch_dev(devId);
            if (gfps_sass_get_hun_flag() == newActive)
            {
                gfps_sass_set_hun_flag(0xFF);
            }
        }
    }

    if (needResume && GET_PROFILE_STATE(sInfo->audState, CONNECTION, A2DP) && \
        GET_PROFILE_STATE(sInfo->audState, CONNECTION, AVRCP))
    {
        app_bt_resume_music_player(GET_BT_ID(devId));
        memset(sassInfo.reconnInfo.reconnAddr.address, 0, sizeof(bt_bdaddr_t));
        sassInfo.reconnInfo.evt = 0xFF;
    }
}

void gfps_sass_add_dev_handler(uint8_t devId, bt_bdaddr_t *addr)
{
    uint8_t cod[3];
    uint8_t connNum = 0;
    SassBtInfo *btHdl;

    if (sassInfo.isMulti) {
        connNum = BT_DEVICE_NUM;
    }else {
        connNum = 1;
    }

    btHdl = gfps_sass_get_connected_dev_by_addr(addr);
    if (btHdl)
    {
        return;
    }
    else
    {
        btHdl = gfps_sass_get_free_handler();
        if (!btHdl)
        {
            return;
        }
    }

    memset(btHdl, 0, sizeof(SassBtInfo));
    app_bt_get_remote_cod_by_addr(addr, cod);
    memcpy(btHdl->btAddr.address, addr, sizeof(bt_bdaddr_t));
    //gfps_sass_get_acckey_from_nv((uint8_t *)addr, btHdl->accKey);
    btHdl->devType = gfps_sass_get_dev_type_by_cod(cod);
    btHdl->connId = devId;
    TRACE(3,"%s btHdl->devType:0x%0x num:%d", __func__, btHdl->devType, sassInfo.connNum);
    sassInfo.connNum++;

    if (sassInfo.connNum == 1) {
        sassInfo.connState = SASS_STATE_NO_DATA;
        sassInfo.focusMode = SASS_CONN_NO_FOCUS;
    }

    if (sassInfo.connNum < connNum){
        sassInfo.connAvail = SASS_CONN_AVAILABLE;
    }else{
        sassInfo.connAvail = SASS_CONN_NONE_AVAILABLE;
    }

    if (!memcmp(sassInfo.reconnInfo.reconnAddr.address, addr, sizeof(bt_bdaddr_t)))
    {
        sassInfo.autoReconn = SASS_AUTO_RECONNECTED;
        if (sassInfo.reconnInfo.evt != SASS_EVT_SWITCH_BACK_AND_RESUME)
        {
            sassInfo.reconnInfo.evt = 0xFF;
        }
    }
}

void gfps_sass_remove_dev_handler(bt_bdaddr_t *addr)
{
    uint8_t maxLink = gfps_sass_get_multi_status() ? 2 : 1;
    for(int i= 0; i < BT_DEVICE_NUM; i++)
    {
        if (!memcmp(sassInfo.connInfo[i].btAddr.address, (uint8_t *)addr, sizeof(bt_bdaddr_t)))
        {
            if (sassInfo.activeId == sassInfo.connInfo[i].connId)
            {
                SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(sassInfo.connInfo[i].connId);
                if (otherInfo && (GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP) || \
                    GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) || \
                    GET_PROFILE_STATE(otherInfo->audState, AUDIO, HFP)))
                {
                    gfps_sass_set_active_dev(otherInfo->connId);
                }
                else
                {
                    gfps_sass_set_active_dev(0xFF);
                }
            }
            memset((void *)&(sassInfo.connInfo[i]), 0, sizeof(SassBtInfo));
            sassInfo.connInfo[i].connId = 0xFF;
            sassInfo.connNum--;
            break;
        }
    }

    if (!memcmp(sassInfo.reconnInfo.reconnAddr.address, (uint8_t *)addr, sizeof(bt_bdaddr_t)))
    {
        gfps_sass_reset_reconn_info();
    }

    if (sassInfo.connNum >= maxLink)
    {
        sassInfo.connAvail = SASS_CONN_NONE_AVAILABLE;
    }
    else
    {
        sassInfo.connAvail = SASS_CONN_AVAILABLE;
        if (sassInfo.connNum == 0) {
            sassInfo.connState = SASS_STATE_NO_CONNECTION;
            sassInfo.focusMode = SASS_CONN_NO_FOCUS;
        }
    }
    TRACE(1, "%s acckey is:", __func__);
    DUMP8("%02x ", sassInfo.connInfo[0].accKey, 16);
    DUMP8("%02x ", sassInfo.connInfo[1].accKey, 16);   
}

void gfps_sass_del_dev_handler(bt_bdaddr_t *addr, uint8_t reason)
{
    gfps_sass_remove_dev_handler(addr);
    if (reason == BTIF_BEC_MAX_CONNECTIONS || reason == BTIF_BEC_LOCAL_TERMINATED)
    {
        gfps_sass_update_last_dev(addr);
    }
    else if (!gfps_sass_is_any_dev_connected())
    {
        gfps_sass_clear_last_dev();
    }
    else
    {
        TRACE(1,"%s don't update last dev", __func__);
    }
    app_bt_accessmode_set(BTIF_BAM_CONNECTABLE_ONLY);
    gfps_sass_check_if_need_reconnect();
    TRACE(2,"%s connNum: %d", __func__, sassInfo.connNum);
}

void gfps_sass_connect_handler(uint8_t device_id, const bt_bdaddr_t *addr)
{
    SassEvtParam evtParam;
    evtParam.event = SASS_EVT_ADD_DEV;
    evtParam.devId = device_id;
    memcpy(evtParam.addr.address, addr, sizeof(bt_bdaddr_t));
    gfps_sass_update_state(&evtParam);
}

void gfps_sass_update_head_state(SASS_HEAD_STATE_E state)
{
    SassEvtParam evtParam;
    evtParam.devId = 0;
    evtParam.event = SASS_EVT_UPDATE_HEAD_STATE;
    evtParam.state.headState = state;
    gfps_sass_update_state(&evtParam);
}

void gfps_sass_disconnect_handler(uint8_t device_id,const bt_bdaddr_t *addr, uint8_t errCode)
{
    SassEvtParam evtParam;
    evtParam.event = SASS_EVT_DEL_DEV;
    evtParam.devId = device_id;
    evtParam.reason = errCode;
    memcpy(evtParam.addr.address, addr, sizeof(bt_bdaddr_t));
    gfps_sass_update_state(&evtParam);
}

void gfps_sass_update_state(SassEvtParam *evtParam)
{
    uint8_t devId = evtParam->devId;
    bool needUpdate = true;
    SassBtInfo *otherInfo = NULL;

    TRACE(2, "sass update state evt:%d, devId:%d", evtParam->event, devId);
    switch(evtParam->event)
    {
        case SASS_EVT_ADD_DEV:
        {
            gfps_sass_add_dev_handler(devId, &(evtParam->addr));
            break;
        }

        case SASS_EVT_DEL_DEV:
        {
            gfps_sass_del_dev_handler(&(evtParam->addr), evtParam->reason);
            break;
        }

        case SASS_EVT_UPDATE_CONN_STATE:
        {
            gfps_sass_set_conn_state(evtParam->state.connState);
            break;
        }

        case SASS_EVT_UPDATE_HEAD_STATE:
        {
            SASS_HEAD_STATE_E headstate = evtParam->state.headState;
            gfps_sass_set_head_state(headstate);
            break;
        }

        case SASS_EVT_UPDATE_FOCUS_STATE:
        {
            SASS_FOCUS_MODE_E focusstate = evtParam->state.focusMode;
            gfps_sass_set_focus_mode(focusstate);
            break;
        }

        case SASS_EVT_UPDATE_RECONN_STATE:
        {
            SASS_AUTO_RECONN_E reconnstate = evtParam->state.autoReconn;
            gfps_sass_set_auto_reconn(reconnstate);
            break;
        }

        case SASS_EVT_UPDATE_CUSTOM_DATA:
        {
            gfps_sass_set_custom_data(evtParam->state.cusData);
            break;
        }

        case SASS_EVT_UPDATE_MULTI_STATE:
        {
            gfps_sass_set_conn_available(evtParam->state.connAvail);
            break;
        }

        case SASS_EVT_UPDATE_ACTIVE_DEV:
        {
            needUpdate = false;
            break;
        }

        case SASS_EVT_UPDATE_INUSE_ACCKEY:
        {
#if defined(IBRT) && !defined(FREEMAN_ENABLED)
            if (TWS_UI_MASTER == app_ibrt_if_get_ui_role())
            {
                gfps_sass_sync_info();
            }
#endif
            break;
        }
        default:
        break;
    }

    //gfps_sass_get_adv_data(newAdv, &advLen);
    //if (memcmp(newAdv, oldAdv, advLen))
    {   
        gfps_sass_update_adv_data();
        gfps_sass_ntf_conn_status(devId);

        otherInfo  = gfps_sass_get_other_connected_dev(devId);
        TRACE(3, "%s otherInfo:%p, id:%d", __func__, otherInfo, otherInfo ? otherInfo->connId : 0xFF);
        if (otherInfo && (otherInfo->connId != 0xFF))
        {
            uint8_t acckeyA[FP_ACCOUNT_KEY_SIZE];
            gfps_sass_get_inuse_acckey_by_id(devId, acckeyA);
            if ((evtParam->event == SASS_EVT_UPDATE_HEAD_STATE) || !gfps_sass_is_sass_dev(devId) || \
                (acckeyA[0] == 4 && !memcmp(acckeyA, otherInfo->accKey, FP_ACCOUNT_KEY_SIZE)))
            {
                gfps_sass_ntf_conn_status(otherInfo->connId);
            }
        }

        if (needUpdate)
        {
            app_ble_refresh_adv_state(BLE_FASTPAIR_NORMAL_ADVERTISING_INTERVAL);
        }
    }
}

void gfps_sass_handler(uint8_t device_id, uint8_t evt, void *param)
{
    TRACE(3,"%s id:%d evt:0x%0x", __func__, device_id, evt);

    switch(evt)
    {
        case FP_MSG_SASS_GET_CAPBILITY:
        {  
            gfps_sass_ntf_capability(device_id);
            break;
        }

        case FP_MSG_SASS_NTF_CAPBILITY:
        {
            gfps_sass_set_capability(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SET_MULTIPOINT_STATE:
        {
            gfps_sass_set_multipoint_hdl(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SET_SWITCH_PREFERENCE:
        {
            gfps_sass_set_switch_pref_hdl(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_GET_SWITCH_PREFERENCE:
        {
            gfps_sass_ntf_switch_pref(device_id);
            break;
        }
        case FP_MSG_SASS_SWITCH_ACTIVE_SOURCE:
        {
            gfps_sass_switch_src_hdl(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SWITCH_BACK:
        {
            gfps_sass_switch_back_hdl(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_GET_CONN_STATUS:
        {
            gfps_sass_get_conn_hdl(device_id);
            break;
        }
        case FP_MSG_SASS_NTF_INIT_CONN:
        {
            gfps_sass_set_init_conn(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_IND_INUSE_ACCOUNT_KEY:
        {
            gfps_sass_ind_inuse_acckey(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SEND_CUSTOM_DATA:
        {
            gfps_sass_set_custom_data(device_id, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SET_DROP_TGT:
        {
            break;
        }
        default:
        break;
    }
}

#endif

