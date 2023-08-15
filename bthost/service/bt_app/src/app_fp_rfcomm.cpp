#ifdef GFPS_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bluetooth.h"
#include "bt_if.h"
#include "app_bt.h"
#include "spp_api.h"
#include "sdp_api.h"
#include "app_bt_func.h"
#include "app_fp_rfcomm.h"
#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_if.h"
#endif
#include "app_media_player.h"
#ifdef SASS_ENABLED
#include "encrypt/aes.h"
#include "gfps_sass.h"
#include "gfps_crypto.h"
#include "nvrecord_fp_account_key.h"
#endif

#ifdef SPOT_ENABLED
#include "nvrecord_fp_account_key.h"
#include "bt_drv_interface.h"
#include "app_gfps.h"
#endif

#define FP_RFCOMM_TX_PKT_CNT 10
#define FP_ACCUMULATED_DATA_BUF_SIZE    128
// update this value if the maximum possible tx data size is bigger than current value
#define FP_RFCOMM_TX_BUF_CHUNK_SIZE 64
#define FP_RFCOMM_TX_BUF_CHUNK_CNT FP_RFCOMM_TX_PKT_CNT
#define FP_RFCOMM_TX_BUF_SIZE (FP_RFCOMM_TX_BUF_CHUNK_CNT * FP_RFCOMM_TX_BUF_CHUNK_SIZE)

static uint8_t fp_accumulated_data_buf[FP_ACCUMULATED_DATA_BUF_SIZE];
static uint16_t fp_accumulated_data_size = 0;

/* 128 bit UUID in Big Endian df21fe2c-2515-4fdb-8886-f12c4d67927c */
static const uint8_t FP_RFCOMM_UUID_128[16] = {
    0x7C, 0x92, 0x67, 0x4D, 0x2C, 0xF1, 0x86, 0x88, 0xDB, 0x4F, 0x15, 0x25, 0x2C, 0xFE, 0x21, 0xDF};

static fpRfcommSrvEnv_t fpRfEnv = {0};

fpRfcommEnv_t *fp_rfcomm_get_handler(bt_spp_channel_t *chnl)
{
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(fpRfEnv.fp_rfcomm_service[i].spp_chan == chnl)
        {
            return &(fpRfEnv.fp_rfcomm_service[i]);
        }
    }
    return NULL;
}

fpRfcommEnv_t *fp_rfcomm_get_handler_by_id(uint8_t device_id)
{
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(fpRfEnv.fp_rfcomm_service[i].devId == device_id)
        {
            return &(fpRfEnv.fp_rfcomm_service[i]);
        }
    }
    return NULL;
}

fpRfcommEnv_t *fp_rfcomm_get_free_handler()
{
    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(fpRfEnv.fp_rfcomm_service[i].spp_chan == NULL)
        {          
            return &(fpRfEnv.fp_rfcomm_service[i]);
        }
    }
    return NULL;
}

static void fp_rfcomm_reset_data_accumulator(void)
{
    fp_accumulated_data_size = 0;
    memset(fp_accumulated_data_buf, 0, sizeof(fp_accumulated_data_buf));
}

static void app_fp_disconnect_rfcomm_handler(uint8_t device_id)
{
    fpRfcommEnv_t *env = fp_rfcomm_get_handler_by_id(device_id);
    if(env && env->isConnected)
    {
        bt_spp_disconnect(env->spp_chan->rfcomm_handle, 0);
    }
}

void app_fp_disconnect_rfcomm(uint8_t device_id)
{
    app_bt_start_custom_function_in_bt_thread(device_id,
                                           0,
                                           ( uint32_t )app_fp_disconnect_rfcomm_handler);
}

bool app_fp_rfcomm_send(uint8_t device_id, uint8_t *ptrData, uint32_t length)
{
    fpRfcommEnv_t *env = fp_rfcomm_get_handler_by_id(device_id);

    if ((env == NULL) || !env->isConnected)
    {
        TRACE(0,"Fast pair rfcomm has not connected!");
        return false;
    }

    ASSERT(length < FP_RFCOMM_TX_BUF_CHUNK_SIZE,
           "FP_RFCOMM_TX_BUF_CHUNK_SIZE is %d which is smaller than %d, need to increase!",
           FP_RFCOMM_TX_BUF_CHUNK_SIZE,
           length);

    if (BT_STS_FAILED == bt_spp_write(env->spp_chan->rfcomm_handle, ptrData, (uint16_t)length))
    {
        return false;
    }
    else
    {
        return true;
    }
}

static int app_fp_rfcomm_accept_channel_request(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_accept_t *accept)
{
    bool ret = true;
    uint8_t server_channel = accept->local_server_channel;
    fpRfcommEnv_t * env = fp_rfcomm_get_handler_by_id(accept->device_id);
    if (env && (server_channel == RFCOMM_CHANNEL_FP) && (env->isConnected == true))
    {
        TRACE(3,"%s server_channel %d, env %p", __func__, server_channel, env);
        ret = false;
    }
    return ret;
}

static void fp_rfcomm_data_accumulator(uint8_t device_id, uint8_t* ptr, uint16_t len)
{
    if((fp_accumulated_data_size + len) > sizeof(fp_accumulated_data_buf))
    {
        return;
    }
    ASSERT((fp_accumulated_data_size + len) < sizeof(fp_accumulated_data_buf),
        "fp accumulcate buffer is overflow!");

    memcpy(&fp_accumulated_data_buf[fp_accumulated_data_size], ptr, len);
    fp_accumulated_data_size += len;

    uint16_t dataLen;
    while (fp_accumulated_data_size > 0)
    {
        FP_EVENT_PARAM_T evtParam;
        evtParam.devId = SET_BT_ID(device_id);
        evtParam.p.data.pBuf= fp_accumulated_data_buf;
        evtParam.p.data.len = fp_accumulated_data_size;
        dataLen = fpRfEnv.cb(FP_EVENT_DATA_IND, &evtParam);
        if (dataLen > 0)
        {
            fp_accumulated_data_size -= dataLen;
            memmove(fp_accumulated_data_buf, &fp_accumulated_data_buf[dataLen],
                fp_accumulated_data_size);
        }
        else
        {
            break;
        }
    }
}

static int fp_rfcomm_data_received(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    TRACE(1,"%s",__func__);
    uint8_t device_id = param->device_id;
    uint8_t *pData = (uint8_t *)param->rx_data_ptr;
    uint16_t dataLen = param->rx_data_len;
    fp_rfcomm_data_accumulator(device_id, pData, dataLen);
    return 0;
}

static void fp_rfcomm_connected_handler(bt_spp_channel_t *spp_chan, uint8_t instanceIndex, uint8_t *pBtAddr)
{
    uint8_t device_id = spp_chan->device_id;
    fpRfcommEnv_t *env = fp_rfcomm_get_free_handler();
    if (env && (!env->isConnected))
    {
        FP_EVENT_PARAM_T evtParam;
        evtParam.devId = SET_BT_ID(device_id);
        memcpy(evtParam.p.addr.address, pBtAddr, sizeof(bt_bdaddr_t));

        env->isConnected = true;
        env->spp_chan = spp_chan;
        env->devId = device_id;       
        fp_rfcomm_reset_data_accumulator();
        fpRfEnv.cb(FP_EVENT_CONNECTED, &evtParam);
     }
}

static void fp_rfcomm_disconnected_handler(bt_spp_channel_t *spp_chan, uint8_t *pBtAddr)
{
    TRACE(1,"%s",__func__);
    fpRfcommEnv_t *env = fp_rfcomm_get_handler(spp_chan);
    if(env)
    {
        FP_EVENT_PARAM_T evtParam;
        evtParam.devId = SET_BT_ID(env->devId);
        memcpy(evtParam.p.addr.address, pBtAddr, sizeof(bt_bdaddr_t));

        env->devId = 0xFF;
        env->spp_chan = NULL;
        env->isConnected = false;
        fpRfEnv.cb(FP_EVENT_DISCONNECTED, &evtParam);
    }
}

static int fp_rfcomm_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    bt_spp_channel_t *spp_chan = param->spp_chan;
    bt_spp_port_t *port = spp_chan->port;
    uint8_t instanceIndex = port->server_instance_id;
    uint16_t connHandle = spp_chan->connhdl;
    uint8_t* pBtAddr = spp_chan->remote.address;

    TRACE(2,"%s,event is %d",__func__,event);
    switch (event)
    {
        case BT_SPP_EVENT_ACCEPT:
        {
            TRACE(0,"Connected Indication RFComm device info:");
            TRACE(2,"hci handle is 0x%x service index %d",
                connHandle, instanceIndex);
            if (pBtAddr)
            {
                TRACE(0,"Bd addr is:");
                DUMP8("%02x ", pBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);
            }

            fp_rfcomm_connected_handler(spp_chan, instanceIndex, pBtAddr);
            break;
        }
        case BT_SPP_EVENT_OPENED:
        {
            if (pBtAddr)
            {
              TRACE(0,"Bd addr is:");
              DUMP8("%02x ", pBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);
            }

            fp_rfcomm_connected_handler(spp_chan, instanceIndex, pBtAddr);
            break;
        }
        case BT_SPP_EVENT_CLOSED:
        {
            TRACE(0,"Disconnected Rfcomm device info:");
            TRACE(0,"Bd addr is:");
            DUMP8("%02x ", pBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);
            TRACE(1,"hci handle is 0x%x", connHandle);

            TRACE(1,"::BT_SPP_EVENT_CLOSED %d", event);

            fp_rfcomm_disconnected_handler(spp_chan, pBtAddr);
            break;
        }
        case BT_SPP_EVENT_TX_DONE:
        {
            break;
        }
        case BT_SPP_EVENT_RX_DATA:
        {
            fp_rfcomm_data_received(remote, event, param);
            break;
        }
        default:
        {
            TRACE(1,"Unkown rfcomm event %d", event);
            break;
        }
    }

    return BT_STS_SUCCESS;
}

void app_fp_rfcomm_register_callback(fp_event_cb callback)
{
    fpRfEnv.cb = callback;
}

bt_status_t app_fp_rfcomm_init(void)
{
    TRACE(1,"%s",__func__);
    bt_spp_server_config_t config = {0};
    int i = 0;

    config.local_server_channel = RFCOMM_CHANNEL_FP;
    config.local_spp_128bit_uuid = FP_RFCOMM_UUID_128;
    config.app_spp_callback = fp_rfcomm_callback;
    config.accept_callback = app_fp_rfcomm_accept_channel_request;

    bt_spp_server_listen(&config);

    for (; i < BT_DEVICE_NUM; i++)
    {
        if (!fpRfEnv.fp_rfcomm_service[i].isRfcommInitialized)
        {
            fpRfEnv.fp_rfcomm_service[i].isRfcommInitialized = true;
            fpRfEnv.fp_rfcomm_service[i].isConnected = false;
            fpRfEnv.fp_rfcomm_service[i].spp_chan = NULL;
            fpRfEnv.fp_rfcomm_service[i].devId = 0xFF;
        }
    }

    return BT_STS_SUCCESS;
}

#endif
