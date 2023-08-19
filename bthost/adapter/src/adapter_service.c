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
#include "adapter_service.h"
#include "app_bt_media_manager.h"
#include "a2dp_api.h"
#include "avrcp_api.h"
#include "hfp_api.h"
#include "app_bt.h"
#include "cobuf.h"
#include "l2cap_api.h"
#include "l2cap_i.h"

void *ble_buffer_malloc_with_ca(uint16_t size, uint32_t ca, uint32_t line)
{
    return coheap_bt_malloc_with_ca(size, ca, line, false);
}

void ble_buffer_free_with_ca(void *buffer, uint32_t ca, uint32_t line)
{
    coheap_free_with_ca(buffer, ca, line);
}

#ifdef BT_APP_USE_COHEAP
void *bt_app_buffer_malloc_with_ca(uint16_t size, uint32_t ca, uint32_t line)
{
    return coheap_app_malloc_with_ca(size, ca, line);
}
void bt_app_buffer_free_with_ca(void *buffer, uint32_t ca, uint32_t line)
{
    coheap_free_with_ca(buffer, ca, line);
}
#endif

#if 1
#define BT_ADAPTER_TRACE(...) TRACE(__VA_ARGS__)
#else
#define BT_ADAPTER_TRACE(...)
#endif

osMutexDef(g_bt_adapter_mutex_def);
static struct BT_ADAPTER_MANAGER_T g_bt_adapter_manager;
int bt_adapter_event_callback(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param);

void bt_adapter_mutex_lock(void)
{
    if (g_bt_adapter_manager.adapter_lock)
    {
        osMutexWait(g_bt_adapter_manager.adapter_lock, osWaitForever);
    }
    else
    {
        TRACE(0, "bt_adapter_mutex_lock: invalid mutex");
    }
}

void bt_adapter_mutex_unlock(void)
{
    if (g_bt_adapter_manager.adapter_lock)
    {
        osMutexRelease(g_bt_adapter_manager.adapter_lock);
    }
}

static void bt_adapter_clear_device(struct BT_ADAPTER_DEVICE_T *curr_device)
{
    bt_adapter_mutex_lock();
    curr_device->acl_is_connected = false;
    curr_device->sco_is_connected = false;
    curr_device->hfp_is_connected = false;
    curr_device->a2dp_is_connected = false;
    curr_device->a2dp_is_streaming = false;
    curr_device->avrcp_is_connected = false;
    curr_device->acl_conn_hdl = BT_INVALID_CONN_HANDLE;
    curr_device->sco_handle = BT_INVALID_CONN_HANDLE;
    curr_device->sco_codec_type = BT_HFP_SCO_CODEC_CVSD;
    bt_adapter_mutex_unlock();
}

void bt_adapter_manager_init(void)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    memset(&g_bt_adapter_manager, 0, sizeof(g_bt_adapter_manager));

    if (g_bt_adapter_manager.adapter_lock == NULL)
    {
        g_bt_adapter_manager.adapter_lock = osMutexCreate(osMutex(g_bt_adapter_mutex_def));
    }

    ASSERT(sizeof(BT_CALLBACK_PARAM_T) == sizeof(void *), "BT_CALLBACK_PARAM_T error define");
    ASSERT(BT_EVENT_ACL_OPENED == 0x1000 && BT_EVENT_HF_OPENED == 0x1100, "bt event group error define");

#ifndef BT_BUILD_WITH_CUSTOMER_HOST
    bt_add_event_callback(bt_adapter_event_callback,
        BT_EVENT_MASK_LINK_GROUP |
        BT_EVENT_MASK_HFP_HF_GROUP |
        BT_EVENT_MASK_HFP_AG_GROUP |
        BT_EVENT_MASK_A2DP_SNK_GROUP |
        BT_EVENT_MASK_A2DP_SRC_GROUP |
        BT_EVENT_MASK_AVRCP_GROUP);
#endif

    for (int i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        bt_adapter_clear_device(curr_device);
        curr_device->device_id = i;
        INIT_LIST_HEAD(&curr_device->spp_list);
    }

    for (int i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        bt_adapter_clear_device(curr_device);
        curr_device->device_id = BT_SOURCE_DEVICE_ID_BASE + i;
        INIT_LIST_HEAD(&curr_device->spp_list);
    }

    curr_device = &g_bt_adapter_manager.bt_tws_device;
    bt_adapter_clear_device(curr_device);
    curr_device->device_id = BT_DEVICE_TWS_ID;
    INIT_LIST_HEAD(&curr_device->spp_list);
}

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_device(int device_id)
{
    if (device_id < BT_ADAPTER_MAX_DEVICE_NUM)
    {
        return g_bt_adapter_manager.bt_sink_device + device_id;
    }
    else if (device_id >= BT_SOURCE_DEVICE_ID_BASE && device_id < BT_SOURCE_DEVICE_ID_BASE + BT_ADAPTER_MAX_DEVICE_NUM)
    {
        return g_bt_adapter_manager.bt_source_device + (device_id - BT_SOURCE_DEVICE_ID_BASE);
    }
    else if (device_id == BT_DEVICE_TWS_ID)
    {
        return &g_bt_adapter_manager.bt_tws_device;
    }
    else
    {
        TRACE(0, "bt_adapter_get_device: invalid device %x ca=%p", device_id, __builtin_return_address(0));
        return NULL;
    }
}

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_by_id(int device_id)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    struct BT_ADAPTER_DEVICE_T *connected_device = NULL;

    bt_adapter_mutex_lock();

    curr_device = bt_adapter_get_device(device_id);

    if (curr_device && curr_device->acl_is_connected)
    {
        connected_device = curr_device;
    }

    bt_adapter_mutex_unlock();

    return connected_device;
}

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_by_connhdl(uint16_t connhdl)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    int i = 0;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->acl_is_connected && curr_device->acl_conn_hdl == connhdl)
        {
            bt_adapter_mutex_unlock();
            return curr_device;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->acl_is_connected && curr_device->acl_conn_hdl == connhdl)
        {
            bt_adapter_mutex_unlock();
            return curr_device;
        }
    }

    curr_device = &g_bt_adapter_manager.bt_tws_device;
    if (curr_device->acl_is_connected && curr_device->acl_conn_hdl == connhdl)
    {
        bt_adapter_mutex_unlock();
        return curr_device;
    }

    bt_adapter_mutex_unlock();
    return NULL;
}

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    int i = 0;

    if (remote == NULL)
    {
        return NULL;
    }

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->acl_is_connected && memcmp(&curr_device->remote, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            bt_adapter_mutex_unlock();
            return curr_device;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->acl_is_connected && memcmp(&curr_device->remote, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            bt_adapter_mutex_unlock();
            return curr_device;
        }
    }

    curr_device = &g_bt_adapter_manager.bt_tws_device;
    if (curr_device->acl_is_connected && memcmp(&curr_device->remote, remote, sizeof(bt_bdaddr_t)) == 0)
    {
        bt_adapter_mutex_unlock();
        return curr_device;
    }

    bt_adapter_mutex_unlock();
    return NULL;
}

int bt_adapter_get_device_id_by_connhdl(uint16_t connhdl)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    int device_id = BT_DEVICE_INVALID_ID;

    bt_adapter_mutex_lock();

    curr_device = bt_adapter_get_connected_device_by_connhdl(connhdl);
    if (curr_device)
    {
        device_id = curr_device->device_id;
    }

    bt_adapter_mutex_unlock();

    return device_id;
}

int bt_adapter_get_device_id_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    int device_id = BT_DEVICE_INVALID_ID;

    bt_adapter_mutex_lock();

    curr_device = bt_adapter_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        device_id = curr_device->device_id;
    }

    bt_adapter_mutex_unlock();

    return device_id;
}

uint8_t bt_adapter_get_a2dp_codec_type(int device_id)
{
#if 1
    return bes_aud_bt->aud_get_a2dp_codec_type(device_id);
#else
    uint8_t codec_type = BT_A2DP_CODEC_TYPE_SBC;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_device(device_id);
    if (curr_device)
    {
        codec_type = curr_device->a2dp_codec_type;
    }
    bt_adapter_mutex_unlock();

    return codec_type;
#endif
}

uint8_t bt_adapter_get_hfp_sco_codec_type(int device_id)
{
    uint8_t sco_codec = BT_HFP_SCO_CODEC_CVSD;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_device(device_id);
    if (curr_device)
    {
        sco_codec = curr_device->sco_codec_type;
    }
    bt_adapter_mutex_unlock();

    return sco_codec;
}

void bt_adapter_set_a2dp_codec_info(int device_id, uint8_t codec_type, uint8_t sample_rate, uint8_t sample_bit)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_device(device_id);
    if (curr_device)
    {
        curr_device->a2dp_codec_type = codec_type;
        curr_device->a2dp_sample_rate = sample_rate;
        TRACE(0, "(d%x) bt_adapter_set_a2dp_codec_info: type %d sample rate %02x bit %d",
            device_id, codec_type, sample_rate, sample_bit);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_set_hfp_sco_codec_type(int device_id, uint8_t sco_codec)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_device(device_id);
    if (curr_device)
    {
        curr_device->sco_codec_type = sco_codec;
        TRACE(0, "(d%x) bt_adapter_set_hfp_sco_codec_type: %d", device_id, sco_codec);
    }
    bt_adapter_mutex_unlock();
}

uint8_t bt_adapter_count_mobile_link(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->acl_is_connected)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_count_streaming_a2dp(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->a2dp_is_streaming)
        {
            count_device += 1;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->a2dp_is_streaming)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;

}

uint8_t bt_adapter_count_streaming_sco(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->sco_is_connected)
        {
            count_device += 1;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->sco_is_connected)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_has_incoming_call(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->hfp_is_connected && curr_device->hfp_callsetup_state == BT_HFP_CALL_SETUP_IN)
        {
            count_device += 1;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->hfp_is_connected && curr_device->hfp_callsetup_state == BT_HFP_CALL_SETUP_IN)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

bool bt_adapter_is_remote_tws_device(const bt_bdaddr_t *remote)
{
    return false;
}

uint8_t bt_adapter_count_connected_device(void)
{
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (int i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->acl_is_connected)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_count_connected_source_device(void)
{
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (int i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->acl_is_connected)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_create_new_device_id(const bt_bdaddr_t *remote, bool local_as_source)
{
    uint8_t device_id = BT_DEVICE_INVALID_ID;
    uint8_t count_device = 0;
    int i = 0;

    bt_adapter_mutex_lock();

    if (local_as_source)
    {
        count_device = bt_adapter_count_connected_source_device();

        if (count_device >= BT_ADAPTER_MAX_DEVICE_NUM)
        {
            device_id = BT_DEVICE_INVALID_ID;
            goto unlock_return;
        }

        for (i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_BASE + BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
        {
            if (!bt_adapter_get_device(i)->acl_is_connected)
            {
                device_id = i;
                goto unlock_return;
            }
        }

        device_id = count_device;
    }
    else
    {
        if (bt_adapter_is_remote_tws_device(remote))
        {
            device_id = BT_DEVICE_TWS_ID;
            goto unlock_return;
        }

        count_device = bt_adapter_count_connected_device();

        if (count_device >= BT_ADAPTER_MAX_DEVICE_NUM)
        {
            device_id = BT_DEVICE_INVALID_ID;
            goto unlock_return;
        }

        for (i = 0; i < count_device; i += 1)
        {
            if (!bt_adapter_get_device(i)->acl_is_connected)
            {
                device_id = i;
                goto unlock_return;
            }
        }

        device_id = count_device;
    }

unlock_return:
    bt_adapter_mutex_unlock();
    return device_id;
}

uint8_t bt_adapter_reset_device_id(struct BT_ADAPTER_DEVICE_T *curr_device, bool reset_to_source)
{
    uint8_t new_device_id = BT_DEVICE_INVALID_ID;
    struct BT_ADAPTER_DEVICE_T temp_device;
    struct BT_ADAPTER_DEVICE_T *new_device = NULL;

    bt_adapter_mutex_lock();

    if (!curr_device->acl_is_connected)
    {
        new_device_id = curr_device->device_id;
    }
    else if (curr_device->device_id == BT_DEVICE_TWS_ID)
    {
        new_device_id = BT_DEVICE_TWS_ID;
    }
    else if (curr_device->device_id < BT_ADAPTER_MAX_DEVICE_NUM)
    {
        if (reset_to_source)
        {
            new_device_id = bt_adapter_create_new_device_id(&curr_device->remote, true);
            if (new_device_id != BT_DEVICE_INVALID_ID)
            {
                temp_device = *curr_device;
                bt_adapter_clear_device(curr_device);
                new_device = bt_adapter_get_device(new_device_id);
                *new_device = temp_device;
                new_device->device_id = new_device_id; // keep device id untouched
            }
            else
            {
                new_device_id = curr_device->device_id;
                TRACE(0, "bt_adapter_reset_device_id: reset to source failed %x", new_device_id);
            }
        }
        else
        {
            new_device_id = curr_device->device_id;
            TRACE(0, "bt_adapter_reset_device_id: local already sink device %x", new_device_id);
        }
    }
    else if (curr_device->device_id >= BT_SOURCE_DEVICE_ID_BASE && curr_device->device_id < BT_SOURCE_DEVICE_ID_BASE + BT_ADAPTER_MAX_DEVICE_NUM)
    {
        if (reset_to_source)
        {
            new_device_id = curr_device->device_id;
            TRACE(0, "bt_adapter_reset_device_id: local already source device %x", new_device_id);
        }
        else
        {
            new_device_id = bt_adapter_create_new_device_id(&curr_device->remote, false);
            if (new_device_id != BT_DEVICE_INVALID_ID)
            {
                temp_device = *curr_device;
                bt_adapter_clear_device(curr_device);
                new_device = bt_adapter_get_device(new_device_id);
                *new_device = temp_device;
                new_device->device_id = new_device_id; // keep device id untouched
            }
            else
            {
                new_device_id = curr_device->device_id;
                TRACE(0, "bt_adapter_reset_device_id: reset to sink failed %x", new_device_id);
            }
        }
    }
    else
    {
        new_device_id = curr_device->device_id;
        TRACE(0, "bt_adapter_reset_device_id: invalid device %x", new_device_id);
    }

    bt_adapter_mutex_unlock();

    return new_device_id;
}

void bt_adapter_report_acl_connected(const bt_bdaddr_t *bd_addr, const bt_adapter_acl_opened_param_t *acl_con)
{
    uint8_t gen_device_id = BT_DEVICE_INVALID_ID;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    if (acl_con->error_code == BT_STS_ACL_ALREADY_EXISTS)
    {
        goto unlock_return;
    }

    if (acl_con->error_code == BT_STS_SUCCESS)
    {
        if (acl_con->device_id != BT_DEVICE_INVALID_ID)
        {
            gen_device_id = acl_con->device_id;
        }
        else
        {
            gen_device_id = bt_adapter_create_new_device_id(bd_addr, acl_con->local_is_source);
        }

        if (gen_device_id == BT_DEVICE_INVALID_ID)
        {
            TRACE(0, "bt_adapter_report_acl_connected: gen new device id failed %x %d %02x:%02x:*:*:*:%02x",
                acl_con->device_id, acl_con->local_is_source, bd_addr->address[0],
                bd_addr->address[1], bd_addr->address[5]);
            goto unlock_return;
        }

        curr_device = bt_adapter_get_device(gen_device_id);
        bt_adapter_clear_device(curr_device);
        curr_device->remote = *bd_addr;
        curr_device->acl_is_connected = true;
        curr_device->acl_link_mode = 0;
        curr_device->acl_bt_role = acl_con->acl_bt_role;
        curr_device->acl_conn_hdl = acl_con->conn_handle;

        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x connhdl %04x role %d d%x source %d",
            curr_device->device_id, __func__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
            acl_con->conn_handle, acl_con->acl_bt_role, acl_con->device_id, acl_con->local_is_source);
    }
    else
    {
        curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);

        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x connhdl %04x error %02x",
            curr_device ? curr_device->device_id : 0xd, __func__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
            acl_con->conn_handle, acl_con->error_code);

        if (acl_con->error_code != BT_STS_BT_CANCEL_PAGE && curr_device)
        {
            bt_adapter_clear_device(curr_device);
        }
    }

unlock_return:
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_acl_disconnected(const bt_bdaddr_t *bd_addr, const bt_adapter_acl_closed_param_t *acl_dis)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x error %02x reason %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], acl_dis->error_code, acl_dis->disc_reason);
    if (curr_device)
    {
        bt_adapter_clear_device(curr_device);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_sco_connected(const bt_bdaddr_t *bd_addr, const bt_adapter_sco_opened_param_t *sco_con)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x scohdl %04x codec %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], sco_con->sco_handle, sco_con->codec);
    if (curr_device && sco_con->error_code == BT_STS_SUCCESS)
    {
        curr_device->sco_is_connected = true;
        curr_device->sco_handle = sco_con->sco_handle;
        curr_device->sco_codec_type = sco_con->codec;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_sco_disconnected(const bt_bdaddr_t *bd_addr, const bt_adapter_sco_closed_param_t *sco_dis)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x error %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], sco_dis->error_code);
    if (curr_device)
    {
        curr_device->sco_is_connected = false;
        curr_device->sco_handle = BT_INVALID_CONN_HANDLE;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_access_change(const bt_bdaddr_t *bd_addr, const bt_adapter_access_change_param_t *access_change)
{
    BT_ADAPTER_TRACE(0, " %s %d", __func__, access_change->access_mode);
    g_bt_adapter_manager.access_mode = access_change->access_mode;
}

void bt_adapter_report_role_discover(const bt_bdaddr_t *bd_addr, const bt_adapter_role_discover_param_t *role_discover)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x role %d error %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        role_discover->acl_bt_role, role_discover->error_code);
    if (curr_device && role_discover->error_code == BT_STS_SUCCESS)
    {
        curr_device->acl_bt_role = role_discover->acl_bt_role;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_role_change(const bt_bdaddr_t *bd_addr, const bt_adapter_role_change_param_t *role_change)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x role %d error %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        role_change->acl_bt_role, role_change->error_code);
    if (curr_device && role_change->error_code == BT_STS_SUCCESS)
    {
        curr_device->acl_bt_role = role_change->acl_bt_role;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_mode_change(const bt_bdaddr_t *bd_addr, const bt_adapter_mode_change_param_t *mode_change)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x M %d T %d errno %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        mode_change->acl_link_mode, mode_change->sniff_interval, mode_change->error_code);
    if (curr_device && mode_change->error_code == BT_STS_SUCCESS)
    {
        curr_device->acl_link_mode = mode_change->acl_link_mode;
        if (curr_device->acl_link_mode == BT_MODE_SNIFF_MODE)
        {
            curr_device->sniff_interval = mode_change->sniff_interval;
        }
        else
        {
            curr_device->sniff_interval = 0;
        }
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_authenticated(const bt_bdaddr_t *bd_addr, const bt_adapter_authenticated_param_t *auth)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x error %02x", curr_device->device_id, __func__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], auth->error_code);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_enc_change(const bt_bdaddr_t *bd_addr, const bt_adapter_enc_change_param_t *enc_change)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x error %02x", curr_device->device_id, __func__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
            enc_change->error_code);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_inquiry_result(const bt_bdaddr_t *bd_addr, const bt_adapter_inquiry_result_param_t *inq_result)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(&inq_result->remote);
    if (curr_device)
    {
        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device->device_id, __func__,
            inq_result->remote.address[0], inq_result->remote.address[1], inq_result->remote.address[5]);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_inquiry_complete(const bt_bdaddr_t *bd_addr, const bt_adapter_inquiry_complete_param_t *inq_complete)
{

}

void bt_adapter_echo_register(void(*echo_req)(uint8_t device_id, uint16_t conhdl, uint8_t id, uint16_t len, uint8_t *data),
                           void(*echo_rsp)(uint8_t device_id, uint16_t conhdl, uint8_t *rxdata, uint16_t rxle))
{
    btif_custom_l2cap_echo_init(echo_req, echo_rsp);
}

void bt_adapter_echo_req_send(uint8_t device_id, void *conn, uint8_t *data, uint16_t data_len)
{
    btif_l2cap_fill_in_echo_req_data(device_id, conn, data, data_len);
}

void bt_adapter_echo_rsp_send(uint8_t device_id, uint16 conn_handle, uint8 sigid, uint16 len, const uint8* data)
{
    btif_l2cap_fill_in_enco_rsp_data(device_id, conn_handle, sigid, len, data);
}

#ifdef BT_HFP_SUPPORT

void bt_adapter_report_hfp_connected(const bt_bdaddr_t *bd_addr, const bt_hf_opened_param_t *hfp_conn)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->hfp_is_connected = true;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_disconnected(const bt_bdaddr_t *bd_addr, const bt_hf_closed_param_t *hfp_disc)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->hfp_is_connected = false;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_ring(const bt_bdaddr_t *bd_addr, const void *param)
{

}

void bt_adapter_report_hfp_clip_ind(const bt_bdaddr_t *bd_addr, const bt_hf_clip_ind_param_t *hfp_caller_ind)
{

}

void bt_adapter_report_hfp_call_state(const bt_bdaddr_t *bd_addr, const bt_hf_call_ind_param_t *hfp_call)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x call %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], hfp_call->call);
    if (curr_device)
    {
        curr_device->hfp_call_state = hfp_call->call;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_callsetup_state(const bt_bdaddr_t *bd_addr, const bt_hf_callsetup_ind_param_t *hfp_callsetup)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x callsetup %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], hfp_callsetup->callsetup);
    if (curr_device)
    {
        curr_device->hfp_callsetup_state = hfp_callsetup->callsetup;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_callhold_state(const bt_bdaddr_t *bd_addr, const bt_hf_callheld_ind_param_t *hfp_callhold)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x callhold %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], hfp_callhold->callheld);
    if (curr_device)
    {
        curr_device->hfp_callhold_state = hfp_callhold->callheld;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_volume_change(const bt_bdaddr_t *bd_addr, const bt_hf_volume_change_param_t *volume_change)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x type %d vol %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], volume_change->type, volume_change->volume);
    if (curr_device && volume_change->type == BT_HF_VOLUME_TYPE_SPK)
    {
        curr_device->hfp_speak_vol = volume_change->volume;
    }
    bt_adapter_mutex_unlock();
}

#endif /* BT_HFP_SUPPORT */

#ifdef BT_A2DP_SUPPORT

void bt_adapter_report_a2dp_connected(const bt_bdaddr_t *bd_addr, const bt_a2dp_opened_param_t *a2dp_conn)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x codec %d len %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        a2dp_conn->codec_type, a2dp_conn->codec_info_len);
    if (curr_device)
    {
        curr_device->a2dp_is_connected = true;
        curr_device->a2dp_codec_type = a2dp_conn->codec_type;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_disconnected(const bt_bdaddr_t *bd_addr, const bt_a2dp_closed_param_t *a2dp_disc)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->a2dp_is_connected = false;
        curr_device->a2dp_is_streaming = false;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_stream_start(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_start_param_t *a2dp_stream_start)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x codec %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        curr_device ? curr_device->a2dp_codec_type : 0xdd);
    if (curr_device)
    {
        curr_device->a2dp_is_streaming = true;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_stream_reconfig(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_reconfig_param_t *a2dp_stream_reconfig)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x codec %d len %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        a2dp_stream_reconfig->codec_type, a2dp_stream_reconfig->codec_info_len);
    if (curr_device)
    {
        curr_device->a2dp_codec_type = a2dp_stream_reconfig->codec_type;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_stream_suspend(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_suspend_param_t *a2dp_stream_suspend)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->a2dp_is_streaming = false;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_stream_close(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_close_param_t *a2dp_stream_close)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->a2dp_is_streaming = false;
    }
    bt_adapter_mutex_unlock();
}

#endif /* BT_A2DP_SUPPORT */

#ifdef BT_AVRCP_SUPPORT

void bt_adapter_report_avrcp_connected(const bt_bdaddr_t *bd_addr, const bt_avrcp_opened_t *avrcp_conn)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->avrcp_is_connected = true;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_avrcp_disconnected(const bt_bdaddr_t *bd_addr, const bt_avrcp_closed_t *avrcp_disc)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->avrcp_is_connected = false;
    }
    bt_adapter_mutex_unlock();
}

#endif /* BT_AVRCP_SUPPORT */


int bt_adapter_event_callback(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param)
{
    switch (event)
    {
        case BT_EVENT_ACL_OPENED:
            bt_adapter_report_acl_connected(bd_addr, param.bt.acl_opened);
            break;
        case BT_EVENT_ACL_CLOSED:
            bt_adapter_report_acl_disconnected(bd_addr, param.bt.acl_closed);
            break;
        case BT_EVENT_SCO_OPENED:
            bt_adapter_report_sco_connected(bd_addr, param.bt.sco_opened);
            break;
        case BT_EVENT_SCO_CLOSED:
            bt_adapter_report_sco_disconnected(bd_addr, param.bt.sco_closed);
            break;
        case BT_EVENT_ACCESS_CHANGE:
            bt_adapter_report_access_change(bd_addr, param.bt.access_change);
            break;
        case BT_EVENT_ROLE_DISCOVER:
            bt_adapter_report_role_discover(bd_addr, param.bt.role_discover);
            break;
        case BT_EVENT_ROLE_CHANGE:
            bt_adapter_report_role_change(bd_addr, param.bt.role_change);
            break;
        case BT_EVENT_MODE_CHANGE:
            bt_adapter_report_mode_change(bd_addr, param.bt.mode_change);
            break;
        case BT_EVENT_AUTHENTICATED:
            bt_adapter_report_authenticated(bd_addr, param.bt.authenticated);
            break;
        case BT_EVENT_ENC_CHANGE:
            bt_adapter_report_enc_change(bd_addr, param.bt.enc_change);
            break;
        case BT_EVENT_INQUIRY_RESULT:
            bt_adapter_report_inquiry_result(bd_addr, param.bt.inq_result);
            break;
        case BT_EVENT_INQUIRY_COMPLETE:
            bt_adapter_report_inquiry_complete(bd_addr, param.bt.inq_complete);
            break;
#ifdef BT_HFP_SUPPORT
        case BT_EVENT_HF_OPENED:
            bt_adapter_report_hfp_connected(bd_addr, param.hf.opened);
            break;
        case BT_EVENT_HF_CLOSED:
            bt_adapter_report_hfp_disconnected(bd_addr, param.hf.closed);
            break;
        case BT_EVENT_HF_RING_IND:
            bt_adapter_report_hfp_ring(bd_addr, NULL);
            break;
        case BT_EVENT_HF_CLIP_IND:
            bt_adapter_report_hfp_clip_ind(bd_addr, param.hf.clip_ind);
            break;
        case BT_EVENT_HF_CALL_IND:
            bt_adapter_report_hfp_call_state(bd_addr,param.hf.call_ind);
            break;
        case BT_EVENT_HF_CALLSETUP_IND:
            bt_adapter_report_hfp_callsetup_state(bd_addr, param.hf.callsetup_ind);
            break;
        case BT_EVENT_HF_CALLHELD_IND:
            bt_adapter_report_hfp_callhold_state(bd_addr, param.hf.callheld_ind);
            break;
        case BT_EVENT_HF_VOLUME_CHANGE:
            bt_adapter_report_hfp_volume_change(bd_addr, param.hf.volume_change);
            break;
#endif /* BT_HFP_SUPPORT */
#ifdef BT_A2DP_SUPPORT
        case BT_EVENT_A2DP_OPENED:
            bt_adapter_report_a2dp_connected(bd_addr, param.av.opened);
            break;
        case BT_EVENT_A2DP_CLOSED:
            bt_adapter_report_a2dp_disconnected(bd_addr, param.av.closed);
            break;
        case BT_EVENT_A2DP_STREAM_START:
            bt_adapter_report_a2dp_stream_start(bd_addr, param.av.stream_start);
            break;
        case BT_EVENT_A2DP_STREAM_RECONFIG:
            bt_adapter_report_a2dp_stream_reconfig(bd_addr, param.av.stream_reconfig);
            break;
        case BT_EVENT_A2DP_STREAM_SUSPEND:
            bt_adapter_report_a2dp_stream_suspend(bd_addr, param.av.stream_suspend);
            break;
        case BT_EVENT_A2DP_STREAM_CLOSE:
            bt_adapter_report_a2dp_stream_close(bd_addr, param.av.stream_close);
            break;
#endif /* BT_A2DP_SUPPORT */
#ifdef BT_AVRCP_SUPPORT
        case BT_EVENT_AVRCP_OPENED:
            bt_adapter_report_avrcp_connected(bd_addr, param.ar.opened);
            break;
        case BT_EVENT_AVRCP_CLOSED:
            bt_adapter_report_avrcp_disconnected(bd_addr, param.ar.closed);
            break;
#endif /* BT_AVRCP_SUPPORT */
        default:
            break;
    }

    return 0;
}

void bt_adapter_local_volume_up(void)
{
    app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_UP, 0);
}

void bt_adapter_local_volume_down(void)
{
    app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN, 0);
}

void bt_adapter_local_volume_up_with_callback(void (*cb)(uint8_t device_id))
{
     app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_UP, 0, cb);
}

void bt_adapter_local_volume_down_with_callback(void (*cb)(uint8_t device_id))
{
    app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN, 0, cb);
}

#ifdef BT_A2DP_SUPPORT

bt_status_t bt_a2dp_init(bt_a2dp_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_A2DP_SNK_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_connect(const bt_bdaddr_t *remote)
{
    app_bt_reconnect_a2dp_profile((bt_bdaddr_t *)remote);
    return BT_STS_PENDING;
}

bt_status_t bt_a2dp_disconnect(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_disconnect_a2dp_profile(curr_device->a2dp_connected_stream);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_a2dp_accept_custom_cmd(const bt_bdaddr_t *bd_addr, const bt_a2dp_custom_cmd_req_param_t *cmd, bool accept)
{
    bt_a2dp_signal_msg_header_t header;
    header.message_type = accept ? 2 : 3; // accept or reject
    header.packet_type = 0; // single packet
    header.transaction = cmd->trans_lable;
    header.signal_id = cmd->cmd_id;
    header.reserve = 0;
    return btif_a2dp_send_signal_message(bd_addr, &header, NULL, 0);
}

bt_status_t bt_a2dp_send_custom_cmd(const bt_bdaddr_t *bd_addr, uint8_t custom_cmd_id, const uint8_t *data, uint16_t len)
{
    bt_a2dp_signal_msg_header_t header;
    header.message_type = 0; // command
    header.packet_type = 0; // single packet
    header.transaction = 0;
    header.signal_id = custom_cmd_id;
    header.reserve = 0;
    return btif_a2dp_send_signal_message(bd_addr, &header, data, len);
}

#if defined(BT_SOURCE)

bt_status_t bt_a2dp_source_init(bt_a2dp_source_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_A2DP_SRC_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_source_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_source_connect(const bt_bdaddr_t *bd_addr)
{
    bt_source_reconnect_a2dp_profile(bd_addr);
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_source_disconnect(const bt_bdaddr_t *bd_addr)
{
    return bt_a2dp_disconnect(bd_addr);
}

bt_status_t bt_a2dp_source_start_stream(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    app_a2dp_source_start_stream(curr_device->device_id);
    return BT_STS_SUCCESS;
}

bt_status_t bt_a2dp_source_suspend_stream(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    app_a2dp_source_suspend_stream(curr_device->device_id);
    return BT_STS_SUCCESS;
}

#endif /* BT_SOURCE */
#endif /* BT_A2DP_SUPPORT */

#ifdef BT_AVRCP_SUPPORT

bt_status_t bt_avrcp_init(bt_avrcp_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_AVRCP_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_avrcp_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_avrcp_connect(const bt_bdaddr_t *remote)
{
    app_bt_reconnect_avrcp_profile((bt_bdaddr_t *)remote);
    return BT_STS_SUCCESS;
}

bt_status_t bt_avrcp_disconnect(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_disconnect_avrcp_profile(curr_device->avrcp_channel);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_avrcp_send_passthrough_cmd(const bt_bdaddr_t *bd_addr, bt_avrcp_key_code_t key)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    app_bt_a2dp_send_key_request(curr_device->device_id, key);
    return BT_STS_SUCCESS;
}

bt_status_t bt_avrcp_send_get_play_status(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_get_play_status(curr_device->avrcp_channel);
}

bt_status_t bt_avrcp_send_set_abs_volume(const bt_bdaddr_t *bd_addr, uint8_t volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_ct_set_absolute_volume(curr_device->avrcp_channel, volume);
}

bt_status_t bt_avrcp_send_get_play_status_rsp(const bt_bdaddr_t *bd_addr, bt_avrcp_play_status_t play_status, uint32_t song_len, uint32_t song_pos)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_avrcp_send_play_status_rsp(curr_device->avrcp_channel, song_len, song_pos, play_status);
}

bt_status_t bt_avrcp_send_volume_notify_rsp(const bt_bdaddr_t *bd_addr, uint8_t volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device || !curr_device->avrcp_conn_flag)
    {
        return BT_STS_FAILED;
    }
    if (curr_device->volume_report == BTIF_AVCTP_RESPONSE_INTERIM)
    {
        bt_status_t status = btif_avrcp_ct_send_volume_change_actual_rsp(curr_device->avrcp_channel, volume);
        if (BT_STS_FAILED != status)
        {
            curr_device->volume_report = BTIF_AVCTP_RESPONSE_CHANGED;
        }
        return status;
    }
    return BT_STS_FAILED;
}

bt_status_t bt_avrcp_report_status_change(const bt_bdaddr_t *bd_addr, bt_avrcp_status_change_event_t event_id, uint32_t param)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    if (event_id == BT_AVRCP_VOLUME_CHANGED)
    {
        return bt_avrcp_send_volume_notify_rsp(bd_addr, (uint8_t)param);
    }
    if (event_id == BT_AVRCP_PLAY_STATUS_CHANGED)
    {
        bt_avrcp_play_status_t play_status = (bt_avrcp_play_status_t)param;
        bt_avrcp_play_status_t prev_play_status = (bt_avrcp_play_status_t)curr_device->avrcp_palyback_status;
        if (curr_device->avrcp_conn_flag && play_status <= BT_AVRCP_PLAY_STATUS_PAUSED)
        {
            curr_device->avrcp_palyback_status = (uint8_t)play_status;
            if (curr_device->play_status_notify_registered && play_status != prev_play_status)
            {
                return btif_avrcp_send_play_status_change_actual_rsp(curr_device->avrcp_channel, (uint8_t)play_status);
            }
        }
        return BT_STS_FAILED;
    }
    TRACE(0, "bt_avrcp_send_notify_rsp: unsupported event %d", event_id);
    return BT_STS_FAILED;
}

#endif /* BT_AVRCP_SUPPORT */


#ifdef BT_HFP_SUPPORT

bt_status_t bt_hf_init(bt_hf_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_HFP_HF_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_hf_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_hf_connect(const bt_bdaddr_t *remote)
{
    app_bt_reconnect_hfp_profile((bt_bdaddr_t *)remote);
    return BT_STS_PENDING;
}

bt_status_t bt_hf_disconnect(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        app_bt_disconnect_hfp_profile(curr_device->hf_channel);
    }
    return BT_STS_PENDING;
}

bt_status_t bt_hf_connect_audio(const bt_bdaddr_t *bd_addr)
{
    uint8_t device_id = app_bt_get_device_id_byaddr((bt_bdaddr_t *)bd_addr);
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return BT_STS_FAILED;
    }
    return btif_hf_create_audio_link(curr_device->hf_channel);
}

bt_status_t bt_hf_disconnect_audio(const bt_bdaddr_t *bd_addr)
{
    uint8_t device_id = app_bt_get_device_id_byaddr((bt_bdaddr_t *)bd_addr);
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        return BT_STS_FAILED;
    }
    return btif_hf_disc_audio_link(curr_device->hf_channel);
}

bt_status_t bt_hf_start_voice_recognition(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_enable_voice_recognition(curr_device->hf_channel, true);
    }
    return status;
}

bt_status_t bt_hf_stop_voice_recognition(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_enable_voice_recognition(curr_device->hf_channel, false);
    }
    return status;
}

bt_status_t bt_hf_volume_control(const bt_bdaddr_t *bd_addr, bt_hf_volume_type_t type, int volume)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        if (type == BT_HF_VOLUME_TYPE_SPK)
        {
            btif_hf_report_speaker_volume(curr_device->hf_channel, (uint8_t)volume);
        }
        else
        {
            btif_hf_report_mic_volume(curr_device->hf_channel, (uint8_t)volume);
        }
        status = btif_hf_enable_voice_recognition(curr_device->hf_channel, false);
    }
    return status;
    
}

bt_status_t bt_hf_dial(const bt_bdaddr_t *bd_addr, const char *number)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        if (number == NULL)
        {
            status = btif_hf_redial_call(curr_device->hf_channel);
        }
        else
        {
            status = btif_hf_dial_number(curr_device->hf_channel, (uint8_t *)number, number ? strlen(number) : 0);
        }
    }
    return status;
}

bt_status_t bt_hf_dial_memory(const bt_bdaddr_t *bd_addr, int location)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_dial_memory(curr_device->hf_channel, location);
    }
    return status;
}

bt_status_t bt_hf_handle_call_action(const bt_bdaddr_t *bd_addr, bt_hf_call_action_t action, int idx)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return status;
    }
    if (action == BT_HF_CALL_ACTION_ATA)
    {
        return btif_hf_answer_call(curr_device->hf_channel);
    }
    if (action == BT_HF_CALL_ACTION_CHUP)
    {
        return btif_hf_hang_up_call(curr_device->hf_channel);
    }
    if (action == BT_HF_CALL_ACTION_REDIAL)
    {
        return btif_hf_redial_call(curr_device->hf_channel);
    }
    if (action >= BT_HF_CALL_ACTION_CHLD_0 && action <= BT_HF_CALL_ACTION_CHLD_4)
    {
        return btif_hf_call_hold(curr_device->hf_channel, (btif_hf_hold_call_t)(action - BT_HF_CALL_ACTION_CHLD_0), 0);
    }
    if (action == BT_HF_CALL_ACTION_CHLD_1x || action == BT_HF_CALL_ACTION_CHLD_2x)
    {
        return btif_hf_call_hold(curr_device->hf_channel,
            action == BT_HF_CALL_ACTION_CHLD_1x ? BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS : BTIF_HF_HOLD_HOLD_ACTIVE_CALLS,
            idx);
    }
    if (action >= BT_HF_CALL_ACTION_BTRH_0 && action <= BT_HF_CALL_ACTION_BTRH_2)
    {
        char at_cmd[] = {'A', 'T', '+', 'B', 'T', 'R', 'H', '=', '0' + (action - BT_HF_CALL_ACTION_BTRH_0), '\r', 0};
        return bt_hf_send_at_cmd(bd_addr, at_cmd);
    }
    return BT_STS_FAILED;
}

bt_status_t bt_hf_query_current_calls(const bt_bdaddr_t *bd_addr)
{
    return bt_hf_send_at_cmd(bd_addr, "AT+CLCC\r");
}

bt_status_t bt_hf_query_current_operator_name(const bt_bdaddr_t *bd_addr)
{
    return bt_hf_send_at_cmd(bd_addr, "AT+COPS?\r");
}

bt_status_t bt_hf_retrieve_subscriber_info(const bt_bdaddr_t *bd_addr)
{
    return bt_hf_send_at_cmd(bd_addr, "AT+CNUM\r");
}

bt_status_t bt_hf_send_dtmf(const bt_bdaddr_t *bd_addr, char code)
{
    char at_cmd[] = {'A', 'T', '+', 'V', 'T', 'S', '=', code, '\r', 0};
    return bt_hf_send_at_cmd(bd_addr, at_cmd);
}

bt_status_t bt_hf_request_last_voice_tag_number(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_attach_voice_tag(curr_device->hf_channel);
    }
    return status;
}

bt_status_t bt_hf_send_at_cmd(const bt_bdaddr_t *bd_addr, const char *at_cmd_str)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        status = btif_hf_send_at_cmd(curr_device->hf_channel, at_cmd_str);
        status = BT_STS_SUCCESS;
    }
    return status;
}

#if defined(BT_HFP_AG_ROLE)

bt_status_t bt_ag_init(bt_ag_callback_t callback)
{
    if (callback)
    {
        bt_add_event_callback((bt_event_callback_t)callback, BT_EVENT_MASK_HFP_AG_GROUP);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_cleanup(void)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_connect(const bt_bdaddr_t *bd_addr)
{
    bt_source_reconnect_hfp_profile(bd_addr);
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_disconnect(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return status;
    }
    return btif_ag_disconnect_service_link(curr_device->hf_channel);
}

bt_status_t bt_ag_connect_audio(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return status;
    }
    return btif_ag_create_audio_link(curr_device->hf_channel);
}

bt_status_t bt_ag_disconnect_audio(const bt_bdaddr_t *bd_addr)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return status;
    }
    return btif_ag_disc_audio_link(curr_device->hf_channel);
}

bt_status_t bt_ag_start_voice_recoginition(const bt_bdaddr_t *bd_addr)
{
    return bt_ag_send_at_result(bd_addr, "\r\nBVRA:1\r\n");
}

bt_status_t bt_ag_stop_voice_recoginition(const bt_bdaddr_t *bd_addr)
{
    return bt_ag_send_at_result(bd_addr, "\r\nBVRA:0\r\n");
}

bt_status_t bt_ag_volume_control(const bt_bdaddr_t *bd_addr, bt_hf_volume_type_t type, int volume)
{
    char result[32] = {0};
    snprintf(result, sizeof(result), (type == BT_HF_VOLUME_TYPE_SPK) ? "\r\n+VGS:%d\r\n" : "\r\n+VGM:%d\r\n", volume);
    return bt_ag_send_at_result(bd_addr, result);
}

bt_status_t bt_ag_set_curr_at_upper_handle(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_ag_set_curr_at_upper_handle(curr_device->hf_channel);
}

bt_status_t bt_ag_cops_response(const bt_bdaddr_t *bd_addr, const char *operator_name)
{
    char result[32] = {0};
    if (operator_name && operator_name[0])
    {
        snprintf(result, sizeof(result), "\r\n+COPS:1,0,\"%s\"\r\n", operator_name);
    }
    else
    {
        snprintf(result, sizeof(result), "\r\n+COPS:0\r\n");
    }
    return bt_ag_send_at_result(bd_addr, result);
}

bt_status_t bt_ag_clcc_response(const bt_bdaddr_t *bd_addr, const bt_ag_clcc_status_t *status)
{
    char clcc[128] = {0};
    snprintf(clcc, sizeof(clcc), "\r\n+CLCC:%d,%d,%d,%d,%d,\"%s\",%d\r\n",
        status->index, status->dir, status->state, status->mode, status->mpty, status->number, status->number_type);
    return bt_ag_send_at_result(bd_addr, clcc);
}

bt_status_t bt_ag_cind_response(const bt_bdaddr_t *bd_addr, const bt_ag_cind_status_t *status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    char result[32] = {0};
    bt_ag_ind_status_t ag_status;
    bt_hf_call_ind_t call = BT_HF_CALL_NO_CALLS_IN_PROGRESS;
    bt_hf_callsetup_ind_t callsetup = BT_HF_CALLSETUP_NONE;
    bt_hf_callheld_ind_t callheld = BT_HF_CALLHELD_NONE;
    if (status->num_active || status->num_held)
    {
        call = BT_HF_CALL_CALLS_IN_PROGRESS;
    }
    if (status->call_state == BT_HF_CALL_STATE_INCOMING || status->call_state == BT_HF_CALL_STATE_WAITING)
    {
        callsetup = BT_HF_CALLSETUP_INCOMING;
    }
    else if (status->call_state == BT_HF_CALL_STATE_DIALING)
    {
        callsetup = BT_HF_CALLSETUP_OUTGOING;
    }
    else if (status->call_state == BT_HF_CALL_STATE_ALERTING)
    {
        callsetup = BT_HF_CALLSETUP_ALERTING;
    }
    if (status->num_active && status->num_held)
    {
        callheld = BT_HF_CALLHELD_HOLD_AND_ACTIVE;
    }
    else if (status->num_held)
    {
        callheld = BT_HF_CALLHELD_HOLD;
    }
    ag_status.service = status->service;
    ag_status.call = call;
    ag_status.callsetup = callsetup;
    ag_status.callheld = callheld;
    ag_status.signal = status->signal;
    ag_status.roam = status->roam;
    ag_status.battchg = status->battery_level;
    snprintf(result, sizeof(result), "\r\n+CIND:%d,%d,%d,%d,%d,%d,%d\r\n",
        ag_status.service, ag_status.call, ag_status.callsetup, ag_status.callheld,
        ag_status.signal, ag_status.roam, ag_status.battchg);
    btif_ag_set_ind_status(curr_device->hf_channel, &ag_status);
    return bt_ag_send_at_result(bd_addr, result);
}

bt_status_t bt_ag_device_status_change(const bt_bdaddr_t *bd_addr, const bt_ag_device_status_t *status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    bt_ag_ind_status_t ag_status = btif_ag_get_ind_status(curr_device->hf_channel);
    if (status->service != ag_status.service)
    {
        btif_ag_send_service_status(curr_device->hf_channel, status->service);
    }
    if (status->signal != ag_status.signal)
    {
        btif_ag_send_mobile_signal_level(curr_device->hf_channel, status->signal);
    }
    if (status->roam != ag_status.roam)
    {
        btif_ag_send_mobile_roam_status(curr_device->hf_channel, status->roam);
    }
    if (status->battery_level != ag_status.battchg)
    {
        btif_ag_send_mobile_battery_level(curr_device->hf_channel, status->battery_level);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_phone_status_change(const bt_bdaddr_t *bd_addr, const bt_ag_phone_status_t *status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    bt_ag_ind_status_t ag_status = btif_ag_get_ind_status(curr_device->hf_channel);
    bt_hf_call_ind_t call = BT_HF_CALL_NO_CALLS_IN_PROGRESS;
    bt_hf_callsetup_ind_t callsetup = BT_HF_CALLSETUP_NONE;
    bt_hf_callheld_ind_t callheld = BT_HF_CALLHELD_NONE;
    if (status->num_active || status->num_held)
    {
        call = BT_HF_CALL_CALLS_IN_PROGRESS;
    }
    if (status->call_state == BT_HF_CALL_STATE_INCOMING || status->call_state == BT_HF_CALL_STATE_WAITING)
    {
        callsetup = BT_HF_CALLSETUP_INCOMING;
    }
    else if (status->call_state == BT_HF_CALL_STATE_DIALING)
    {
        callsetup = BT_HF_CALLSETUP_OUTGOING;
    }
    else if (status->call_state == BT_HF_CALL_STATE_ALERTING)
    {
        callsetup = BT_HF_CALLSETUP_ALERTING;
    }
    if (status->num_active && status->num_held)
    {
        callheld = BT_HF_CALLHELD_HOLD_AND_ACTIVE;
    }
    else if (status->num_held)
    {
        callheld = BT_HF_CALLHELD_HOLD;
    }
    if (callsetup == BT_HF_CALLSETUP_INCOMING)
    {
        if (ag_status.call)
        {
            btif_ag_send_calling_ring(curr_device->hf_channel, status->number);
        }
        else
        {
            btif_ag_send_call_waiting_notification(curr_device->hf_channel, status->number);
        }
    }
    if (call != ag_status.call)
    {
        btif_ag_send_call_active_status(curr_device->hf_channel, call);
    }
    if (callsetup != ag_status.callsetup)
    {
        btif_ag_send_callsetup_status(curr_device->hf_channel, callsetup);
    }
    if (callheld != ag_status.callheld)
    {
        btif_ag_send_callheld_status(curr_device->hf_channel, callheld);
    }
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_send_at_result(const bt_bdaddr_t *bd_addr, const char *at_result)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    return btif_ag_send_result_code(curr_device->hf_channel, at_result, at_result ? strlen(at_result) : 0);
}

bt_status_t bt_ag_send_response_code(const bt_bdaddr_t *bd_addr, bt_hf_at_response_t code, int cme_error)
{
    const char *at_result = NULL;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return BT_STS_FAILED;
    }
    if (code == BT_HF_AT_RESPONSE_ERROR_CME)
    {
        if (btif_ag_cmee_enabled(curr_device->hf_channel))
        {
            char result[32] = {0};
            snprintf(result, sizeof(result), "\r\n+CME ERROR:%d\r\n", cme_error);
            return bt_ag_send_at_result(bd_addr, result);
        }
        code = BT_HF_AT_RESPONSE_ERROR;
    }
    switch (code)
    {
        case BT_HF_AT_RESPONSE_OK: at_result = "\r\nOK\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR: at_result = "\r\nERROR\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_NO_CARRIER: at_result = "\r\nNO CARRIER\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_BUSY: at_result = "\r\nBUSY\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_NO_ANSWER: at_result = "\r\nNO ANSWER\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_DELAYED: at_result = "\r\nDELAYED\r\n"; break;
        case BT_HF_AT_RESPONSE_ERROR_BLACKLISTED: at_result = "\r\nBLACKLISTED\r\n"; break;
        default: TRACE(0, "bt_ag_send_response_code: invalid code %d", code); break;
    }
    if (at_result == NULL)
    {
        return BT_STS_FAILED;
    }
    return bt_ag_send_at_result(bd_addr, at_result);
}

bt_status_t bt_ag_set_sco_allowed(const bt_bdaddr_t *bd_addr, bool sco_enable)
{
    return BT_STS_SUCCESS;
}

bt_status_t bt_ag_send_bsir(const bt_bdaddr_t *bd_addr, bool in_band_ring_enable)
{
    char at_result[] = {'\r', '\n', '+', 'B', 'S', 'I', 'R', ':', in_band_ring_enable ? '1' : '0', '\r', '\n', 0};
    return bt_ag_send_at_result(bd_addr, at_result);
}

int bt_ag_is_noise_reduction_supported(const bt_bdaddr_t *bd_addr)
{
    uint32_t hf_features = 0;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return false;
    }
    hf_features = btif_ag_get_hf_features(curr_device->hf_channel);
    return (hf_features & BT_HF_FEAT_ECNR) != 0;
}

int bt_ag_is_voice_recognition_supported(const bt_bdaddr_t *bd_addr)
{
    uint32_t hf_features = 0;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);
    if (!curr_device)
    {
        return false;
    }
    hf_features = btif_ag_get_hf_features(curr_device->hf_channel);
    return (hf_features & BT_HF_FEAT_VR) != 0;
}

#endif /* BT_HFP_AG_ROLE */
#endif /* BT_HFP_SUPPORT */

