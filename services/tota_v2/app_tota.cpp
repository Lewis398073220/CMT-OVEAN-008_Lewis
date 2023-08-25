/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_audio.h"
#include "app_utils.h"
#ifndef CMT_008_SPP_TOTA_V2
#include "hal_aud.h"
#else /*CMT_008_SPP_TOTA_V2*/
#include "app_anc.h"
#endif /*CMT_008_SPP_TOTA_V2*/
#include "hal_norflash.h"
#include "pmu.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "cmsis_os.h"
#include "app_tota.h"
#include "app_tota_cmd_code.h"
#include "app_tota_cmd_handler.h"
#include "app_spp_tota.h"
#include "cqueue.h"
#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#include "ota_ble_adapter.h"
#endif
#include "bluetooth_bt_api.h"
#include "app_bt_audio.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "app_thread.h"
#include "cqueue.h"
#include "hal_location.h"
#include "app_hfp.h"
#include "bt_drv_reg_op.h"
#if defined(IBRT)
#include "app_tws_ibrt.h"
#endif
#include "cmsis.h"
#include "app_battery.h"
#include "crc32_c.h"
#include "factory_section.h"
#include "app_ibrt_rssi.h"
#include "tota_stream_data_transfer.h"
#if (BLE_APP_TOTA)
#include "app_tota_ble.h"
#endif
#if defined(OTA_OVER_TOTA_ENABLED)
#include "ota_control.h"
#endif
#include "app_tota_encrypt.h"
#include "app_tota_flash_program.h"
#include "app_tota_audio_dump.h"
#include "app_tota_mic.h"
#include "app_tota_anc.h"
#include "app_tota_general.h"
#include "app_tota_custom.h"
#include "app_tota_conn.h"
#include "encrypt/aes.h"
#include "app_tota_audio_EQ.h"
#include "app_tota_common.h"
#include "audio_cfg.h"
#include "app_bt_cmd.h"

#include <map>

#include "beslib_info.h"

using namespace std;

#define DONGLE_DATA_FRAME 624

APP_TOTA_CMD_PAYLOAD_T *payload = NULL;
/*call back sys for modules*/
static map<APP_TOTA_MODULE_E, const app_tota_callback_func_t *>     s_module_map;
static const app_tota_callback_func_t                             *s_module_func;
static APP_TOTA_MODULE_E                                    s_module;

#ifdef BLE_TOTA_ENABLED
/// health thermometer application environment structure
struct ble_tota_env_info
{
    uint8_t  connectionIndex;
    uint8_t  isNotificationEnabled;
    uint16_t mtu[BLE_CONNECTION_MAX];
};

struct ble_tota_env_info ble_tota_env;
#endif

/* register callback module */
void app_tota_callback_module_register(APP_TOTA_MODULE_E module,
                                const app_tota_callback_func_t *tota_callback_func)
{
    map<APP_TOTA_MODULE_E, const app_tota_callback_func_t *>::iterator it = s_module_map.find(module);
    if ( it == s_module_map.end() )
    {
        TOTA_LOG_DBG(0, "add to map");
        s_module_map.insert(make_pair(module, tota_callback_func));
    }
    else
    {
        TOTA_LOG_DBG(0, "already exist, not add");
    }
}
/* set current module */
void app_tota_callback_module_set(APP_TOTA_MODULE_E module)
{
    map<APP_TOTA_MODULE_E, const app_tota_callback_func_t *>::iterator it = s_module_map.find(module);
    if ( it != s_module_map.end() )
    {
        s_module = module;
        s_module_func = it->second;
        TOTA_LOG_DBG(1, "set %d module success", module);
    }
    else
    {
        TOTA_LOG_DBG(0, "not find callback func by module");
    }
}

/* get current module */
APP_TOTA_MODULE_E app_tota_callback_module_get()
{
    return s_module;
}
/*-------------------------------------------------------------------*/

static void s_app_tota_connected();
static void s_app_tota_disconnected();
static void s_app_tota_tx_done();
static void s_app_tota_rx(uint8_t * cmd_buf, uint16_t len);
#ifdef CMT_008_SPP_TOTA_V2
static void app_tota_vendor_cmd_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);
#endif

static void s_app_tota_connected()
{
    TOTA_LOG_DBG(0,"Tota connected.");
    app_tota_store_reset();
    tota_set_connect_status(TOTA_CONNECTED);
    tota_set_connect_path(APP_TOTA_VIA_SPP);
    if (s_module_func && s_module_func->connected_cb != NULL)
    {
        s_module_func->connected_cb();
    }
}

static void s_app_tota_disconnected()
{
    TOTA_LOG_DBG(0,"Tota disconnected.");
    app_tota_store_reset();
    app_tota_ctrl_reset();
    tota_rx_queue_deinit();

#if (OTA_OVER_TOTA_ENABLED)
    ota_control_set_datapath_type(NO_OTA_CONNECTION)
    Bes_exit_ota_state();
#ifdef IBRT
    if (APP_TOTA_VIA_NOTIFICATION == tota_get_connect_path())
    {
        uint8_t disconnected_parm = APP_OTA_LINK_TYPE_BLE;
        tws_ctrl_send_cmd(IBRT_OTA_TWS_MOBILE_DISC_CMD, &disconnected_parm, 1);
    }
#endif
#endif

    tota_set_connect_status(TOTA_DISCONNECTED);
    tota_set_connect_path(APP_TOTA_PATH_IDLE);

    if (s_module_func && s_module_func->disconnected_cb != NULL)
    {
        s_module_func->disconnected_cb();
    }
}

static void s_app_tota_tx_done()
{
    TOTA_LOG_DBG(0,"Tota tx done.");
    if (s_module_func && s_module_func->tx_done_cb != NULL)
    {
        s_module_func->tx_done_cb();
    }
}

#ifdef BUDSODIN2_TOTA
#define BUDSODIN2_VENDOR_CODE 0xfd
bool app_tota_if_customers_access_valid(uint8_t access_code)
{
    bool valid =  false;
    if(BUDSODIN2_VENDOR_CODE == access_code)
    {
        valid = true;
    }

    return valid;
}

#define BUDSODIN2_DATA_LEN_LIMIT 40
#define BUDSODIN2_RUBBISH_CODE_LEN 3
#define BUDSODIN2_HEADER_LEN 4
uint8_t g_custom_tota_data[BUDSODIN2_DATA_LEN_LIMIT]= {0};
uint8_t * app_tota_custom_refactor_tota_data(uint8_t* ptrData, uint32_t dataLength)
{
    TOTA_LOG_DBG(0,"TOTA custom hijack! data len=%d", dataLength);
    do{
        if(dataLength >= BUDSODIN2_DATA_LEN_LIMIT || dataLength < 4)
        {
            break;
        }
        memset(g_custom_tota_data, 0 , BUDSODIN2_DATA_LEN_LIMIT);
        //refacor tota data
        APP_TOTA_CMD_PAYLOAD_T* pPayload = (APP_TOTA_CMD_PAYLOAD_T *)&g_custom_tota_data;
        pPayload->cmdCode = OP_TOTA_STRING;
        pPayload->paramLen = dataLength - BUDSODIN2_HEADER_LEN - BUDSODIN2_RUBBISH_CODE_LEN;

        memcpy(pPayload->param, &ptrData[BUDSODIN2_HEADER_LEN], pPayload->paramLen);
    }while(0);

    return g_custom_tota_data;
}
#endif

#ifdef BLE_TOTA_ENABLED
static void tota_ble_event_ccc_change_handler(uint8_t conidx, uint8_t ntf_en)
{
    if (ntf_en)
    {
        if (!ble_tota_env.isNotificationEnabled)
        {
            ble_tota_env.isNotificationEnabled = ntf_en;
            ble_tota_env.connectionIndex = conidx;
            app_tota_store_reset();
            tota_set_connect_status(TOTA_CONNECTED);
            tota_set_connect_path(APP_TOTA_VIA_NOTIFICATION);
            TOTA_LOG_DBG(3,"[%s], condix = %d connectionIndex = %d",__func__,conidx, ble_tota_env.connectionIndex);
#if (BLE_TOTA_ENABLED) && (OTA_OVER_TOTA_ENABLED)
            ota_control_register_transmitter(ota_tota_send_notification);
            bes_ble_gap_conn_update_param(conidx, 10, 15, 20000, 0);
            ota_control_set_datapath_type(DATA_PATH_BLE);
#endif

            if (ble_tota_env.mtu[conidx])
            {
                tota_set_trans_MTU(ble_tota_env.mtu[conidx]);
#if defined(BLE_TOTA_ENABLED) && defined(OTA_OVER_TOTA_ENABLED)
                ota_control_update_MTU(ble_tota_env.mtu[conidx] - TOTA_PACKET_VERIFY_SIZE);
#endif
            }
        }
    }
    else
    {
        if(ble_tota_env.isNotificationEnabled)
        {
            ble_tota_env.isNotificationEnabled = ntf_en;
            tota_set_connect_path(APP_TOTA_PATH_IDLE);
            app_tota_ble_disconnected();
        }
    }
}

static void tota_ble_event_callback(bes_ble_tota_event_param_t *param)
{
    switch(param->event_type) {
    case BES_BLE_TOTA_CCC_CHANGED:
        TOTA_LOG_DBG(3,"[%s], condix = %d  %p",__func__, param->conidx,(void *)param);
        tota_ble_event_ccc_change_handler(param->conidx, param->param.ntf_en);
        break;
    case BES_BLE_TOTA_DIS_CONN_EVENT:
        TOTA_LOG_DBG(3,"[%s], condix = %d Index = %d",__func__, param->conidx, ble_tota_env.connectionIndex);
        if (param->conidx == ble_tota_env.connectionIndex)
        {
            ble_tota_env.connectionIndex = INVALID_CONNECTION_INDEX;
            ble_tota_env.isNotificationEnabled = false;
            ble_tota_env.mtu[param->conidx] = 0;
            tota_set_connect_path(APP_TOTA_PATH_IDLE);
            app_tota_ble_disconnected();     
        }
        break;
    case BES_BLE_TOTA_RECEVICE_DATA:
        app_tota_handle_received_data(param->param.receive_data.data, param->param.receive_data.data_len);
        break;
    case BES_BLE_TOTA_MTU_UPDATE:
        TOTA_LOG_DBG(1,"[%s]TOTA",__func__);
        if (param->conidx == ble_tota_env.connectionIndex)
        {
            ble_tota_env.mtu[ble_tota_env.connectionIndex] = param->param.mtu;
            TRACE(1,"updated data packet size is %d", ble_tota_env.mtu[ble_tota_env.connectionIndex]);
        }
        else
        {
            ble_tota_env.mtu[param->conidx] = param->param.mtu;
        }
        break;
    case BES_BLE_TOTA_SEND_DONE:
        break;
    default:
        TOTA_LOG_DBG(0,"TOTA ble not find event %x", param->event_type);
        break;
    }
}

uint8_t tota_ble_get_conidx(void)
{
    return ble_tota_env.connectionIndex;
}
static void tota_ble_init(void)
{
    ble_tota_env.connectionIndex =  INVALID_CONNECTION_INDEX;
    ble_tota_env.isNotificationEnabled = false;
    memset((uint8_t *)&(ble_tota_env.mtu), 0, sizeof(ble_tota_env.mtu));

    bes_ble_tota_event_reg(tota_ble_event_callback);
}

#endif

static bool app_tota_send_via_datapath(uint8_t * pdata, uint16_t dataLen);

static void s_app_tota_rx(uint8_t * cmd_buf, uint16_t len)
{
    TOTA_LOG_DBG(0,"Tota rx.");
    uint8_t * buf = cmd_buf;

    //sanity check
    if(buf == NULL)
    {
        TOTA_LOG_DBG(0,"[%s] cmd_buf is null", __func__);
        return;
    }

    uint8_t ptrData[3]={0x17,0x18,0x19};
    uint16_t length =3;
    app_tota_send_via_datapath(ptrData, length);

#ifdef CMT_008_SPP_TOTA_V2
    uint16_t i=0;
    for(i=0; i<len; i++)
    {
        TOTA_LOG_DBG(1,"spp_data = 0x%x", buf[i]);
    }

    if(buf[0] == 0x92 && buf[1] == 0x00 && len == 0x06)
    {
        if(buf[2] != 0x00 || buf[3] != 0x02)
            return;
        app_tota_vendor_cmd_handler(OP_TOTA_CMT008_SPP_TEST_CMD, buf, (uint32_t) len);
        return;
    }
#endif /*CMT_008_SPP_TOTA_V2*/

    int ret = 0;
#ifdef BUDSODIN2_TOTA
    //hijack the customers tota data
    if(app_tota_if_customers_access_valid(buf[0]))
    {
        buf = app_tota_custom_refactor_tota_data(buf, len);
    }
#endif
    ret = tota_rx_queue_push(buf, len);
    ASSERT(ret == 0, "tota rx queue FULL!!!!!!");

    uint16_t queueLen= tota_rx_queue_length();
    while (TOTA_PACKET_VERIFY_SIZE < queueLen)
    {
        TOTA_LOG_DBG(1,"queueLen_1 = 0x%x", queueLen);
        ret = app_tota_rx_unpack(buf, queueLen); //jay
        TOTA_LOG_DBG(1," ret: [%d] ", ret);
        if (ret)
        {
            break;
        }
        queueLen= tota_rx_queue_length();

        TOTA_LOG_DBG(1,"queueLen_2 = 0x%x",queueLen);
    }

}

static const tota_callback_func_t app_tota_cb = {
    s_app_tota_connected,
    s_app_tota_disconnected,
    s_app_tota_tx_done,
    s_app_tota_rx
};


void app_tota_init(void)
{
    TOTA_LOG_DBG(0, "tota %s", BESLIB_INFO_STR);
    TOTA_LOG_DBG(0, "Init application test over the air.");

    app_spp_tota_init(&app_tota_cb);

#ifdef STREAM_DATA_OVER_TOTA    
    /* initialize stream thread */
    app_tota_stream_data_transfer_init();
#endif

    // app_tota_cmd_handler_init();
    /* register callback modules */

#ifdef DATA_DUMP_TOTA    
    app_tota_audio_dump_init();
#endif

    /* set module to access spp callback */
    app_tota_callback_module_set(APP_TOTA_AUDIO_DUMP);

    tota_common_init();

#if defined(BLE_TOTA_ENABLED)
    tota_ble_init();
#if defined(OTA_OVER_TOTA_ENABLED)
    ota_ble_adapter_init();
#endif
#endif

}

void app_tota_handle_received_data(uint8_t* buffer, uint16_t maxBytes)
{
    TOTA_LOG_DBG(2,"[%s]data receive data length = %d",__func__,maxBytes);
    TOTA_LOG_DUMP("[0x%x]",buffer,(maxBytes>20 ? 20 : maxBytes));
    s_app_tota_rx(buffer,maxBytes);
}

void app_tota_ble_disconnected(void)
{
    s_app_tota_disconnected();
}

static bool app_tota_send_via_datapath(uint8_t * pdata, uint16_t dataLen)
{
    //dataLen = app_tota_tx_pack(pdata, dataLen);  /* Disable by Jay */
    if (0 == dataLen)
    {
        return false;
    }

    switch (tota_get_connect_path())
    {
        case APP_TOTA_VIA_SPP:
            return app_spp_tota_send_data(pdata, dataLen);
#ifdef BLE_TOTA_ENABLED
        case APP_TOTA_VIA_NOTIFICATION:
            TOTA_LOG_DBG(2 ,"[%s]   BLEconnectionIndex:[%d]",__func__, ble_tota_env.connectionIndex);
            return bes_ble_tota_send_notification(ble_tota_env.connectionIndex, pdata, dataLen);
#endif
        default:
            return false;
    }
}

bool app_tota_send(uint8_t * pdata, uint16_t dataLen, APP_TOTA_CMD_CODE_E opCode)
{
    if ( opCode == OP_TOTA_NONE )
    {
        TOTA_LOG_DBG(0, "Send pure data");
        /* send pure data */
        return app_tota_send_via_datapath(pdata, dataLen);
    }

    APP_TOTA_CMD_PAYLOAD_T payload;
    /* sanity check: opcode is valid */
    if (opCode >= OP_TOTA_COMMAND_COUNT)
    {
        TOTA_LOG_DBG(0, "Warning: opcode not found");
        return false;
    }
    /* sanity check: data length */
    if (dataLen > sizeof(payload.param))
    {
        TOTA_LOG_DBG(0, "Warning: the length of the data is too lang");
        return false;
    }
    /* sanity check: opcode entry */
    // becase some cmd only for one side
    uint16_t entryIndex = app_tota_cmd_handler_get_entry_index_from_cmd_code(opCode);
    if (INVALID_TOTA_ENTRY_INDEX == entryIndex)
    {
        TOTA_LOG_DBG(0, "Warning: cmd not registered");
        return false;
    }

    payload.cmdCode = opCode;
    payload.paramLen = dataLen;
    memcpy(payload.param, pdata, dataLen);

    /* if is string, direct send */
    if ( opCode == OP_TOTA_STRING )
    {
        return app_tota_send_via_datapath((uint8_t*)&payload, dataLen+4);
    }
#if TOTA_ENCODE
    /* cmd filter */
    if ((TOTA_SHAKE_HANDED  == tota_get_connect_status()) && (opCode > OP_TOTA_CONN_CONFIRM))
    {
        // encode here
        TOTA_LOG_DBG(0, "do encode");
        uint16_t len = tota_encrypt(totaEnv->codeBuf, (uint8_t*)&payload, dataLen+4);
        if (app_tota_send_via_datapath(totaEnv->codeBuf, len))
        {
            APP_TOTA_CMD_INSTANCE_T* pInstance = TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex);
            if (pInstance->isNeedResponse)
            {
                app_tota_cmd_handler_add_waiting_rsp_timeout_supervision(entryIndex);
            }
            return true;
        }
        else
        {
            return false;
        }
    }
#endif
    TOTA_LOG_DBG(0, "send normal cmd");
    if (app_tota_send_via_datapath((uint8_t*)&payload, dataLen+4))
    {
        APP_TOTA_CMD_INSTANCE_T* pInstance = TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex);
        if (pInstance->isNeedResponse)
        {
            app_tota_cmd_handler_add_waiting_rsp_timeout_supervision(entryIndex);
        }
    }

    return true;
}

bool app_tota_send_rsp(APP_TOTA_CMD_CODE_E rsp_opCode, APP_TOTA_CMD_RET_STATUS_E rsp_status, uint8_t * pdata, uint16_t dataLen)
{
    TOTA_LOG_DBG(3,"[%s] opCode=0x%x, status=%d",__func__, rsp_opCode, rsp_status);
    // check responsedCmdCode's validity
    if ( rsp_opCode >= OP_TOTA_COMMAND_COUNT || rsp_opCode < OP_TOTA_STRING)
    {
        return false;
    }
    APP_TOTA_CMD_RSP_T* pResponse = (APP_TOTA_CMD_RSP_T *)(payload->param);

    // check parameter length
    if (dataLen > sizeof(pResponse->rspData))
    {
        return false;
    }
    pResponse->cmdCodeToRsp = rsp_opCode;
    pResponse->cmdRetStatus = rsp_status;
    pResponse->rspDataLen   = dataLen;
    memcpy(pResponse->rspData, pdata, dataLen);

    payload->cmdCode = OP_TOTA_RESPONSE_TO_CMD;
    payload->paramLen = 3*sizeof(uint16_t) + dataLen;

#if TOTA_ENCODE
    uint16_t len = tota_encrypt(totaEnv->codeBuf, (uint8_t*)payload, payload->paramLen+4);
    return app_tota_send_via_datapath(totaEnv->codeBuf, len);
#else
    return app_tota_send_via_datapath((uint8_t*)payload, payload->paramLen+4);
#endif
}

bool app_tota_send_data(APP_TOTA_CMD_CODE_E opCode, uint8_t * data, uint32_t dataLen)
{
    uint16_t trans_MTU = tota_get_trans_MTU();
    TOTA_LOG_DBG(1, "trans_MTU:%d",trans_MTU);

    /* sanity check: opcode is valid */
    if (opCode >= OP_TOTA_COMMAND_COUNT)
    {
        TOTA_LOG_DBG(0, "Warning: opcode not found");
        return false;
    }

    /* sanity check: opcode entry */
    // becase some cmd only for one side
    uint16_t entryIndex = app_tota_cmd_handler_get_entry_index_from_cmd_code(opCode);
    if (INVALID_TOTA_ENTRY_INDEX == entryIndex)
    {
        TOTA_LOG_DBG(0, "Warning: cmd not registered");
        return false;
    }

    uint8_t *pData = data;
    uint8_t *sendBuf = (uint8_t *)payload;
    uint32_t sendBytes = 0;
    uint32_t leftBytes = dataLen;
    payload->cmdCode = opCode;
    payload->paramLen = dataLen;
    sendBytes = 4;
    leftBytes += 4;

    do
    {
        TOTA_LOG_DBG(2, "leftBytes=%d,sendBytes=%d", leftBytes, sendBytes);

        if (leftBytes <= DONGLE_DATA_FRAME)
        {
            memcpy(sendBuf+sendBytes, pData, (leftBytes-sendBytes));
            pData += (leftBytes-sendBytes);
            sendBytes = leftBytes;
        }
        else
        {
            memcpy(sendBuf+sendBytes, pData, DONGLE_DATA_FRAME-sendBytes);
            pData += DONGLE_DATA_FRAME-sendBytes;
            sendBytes = DONGLE_DATA_FRAME;
        }

        leftBytes -= sendBytes;

        TOTA_LOG_DBG(2, "leftBytes=%d,sendBytes=%d",leftBytes,sendBytes);
#if TOTA_ENCODE
        if (TOTA_SHAKE_HANDED == tota_get_connect_status())
        {
            // encode here
            TOTA_LOG_DBG(0, "do encode");
            sendBytes = tota_encrypt(totaEnv->codeBuf, sendBuf, sendBytes);
            sendBuf = totaEnv->codeBuf;
        }
        else
        {
            return false;
        }
#else
        TOTA_LOG_DBG(0, "send normal cmd");
#endif

        app_tota_send_via_datapath(sendBuf, sendBytes);

        APP_TOTA_CMD_INSTANCE_T* pInstance = TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex);
        if (pInstance->isNeedResponse)
        {
            app_tota_cmd_handler_add_waiting_rsp_timeout_supervision(entryIndex);
        }

        sendBytes = 0;
        sendBuf = (uint8_t *)payload;

    }while(leftBytes > 0);

    return true;
}


#if defined(OTA_OVER_TOTA_ENABLED)
void app_ota_over_tota_receive_data(uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(1,"[%s] datapath %d", __func__, tota_get_connect_path());
    switch (tota_get_connect_path())
    {
        case APP_TOTA_VIA_SPP:
            ota_control_handle_received_data(ptrParam, false, paramLen);
            break;
#ifdef BLE_TOTA_ENABLED
        case APP_TOTA_VIA_NOTIFICATION:
            ota_ble_push_rx_data(BLE_RX_DATA_SELF_TOTA_OTA, tota_ble_get_conidx(), ptrParam, paramLen);
            break;
#endif
        default:
            break;
    }
}

#if (BLE_TOTA_ENABLED)
void ota_tota_send_notification(uint8_t* ptrData, uint32_t length)
{
    app_tota_send_data(OP_TOTA_OTA, ptrData, length);
}
#endif

void ota_spp_tota_send_data(uint8_t* ptrData, uint32_t length)
{
    app_tota_send_data(OP_TOTA_OTA, ptrData, length);
}
#endif

/*---------------------------------------------------------------------------------------------------------------------------*/
#ifdef IS_TOTA_LOG_PRINT_ENABLED
static char strBuf[MAX_SPP_PACKET_SIZE-4];

char *tota_get_strbuf(void)
{
    return strBuf;
}

void tota_printf(const char * format, ...)
{
    app_spp_tota_send_data((uint8_t*)format, strlen(format));
}

void tota_print(const char * format, ...)
{
    app_tota_send((uint8_t*)format, strlen(format), OP_TOTA_STRING);
}
#else
void tota_printf(const char * format, ...)
{

}

void tota_print(const char * format, ...)
{

}
#endif

static void app_tota_demo_cmd_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    switch(funcCode)
    {
        case OP_TOTA_STRING:
#if defined(APP_TRACE_RX_ENABLE) || defined(APP_RX_API_ENABLE)
            app_bt_cmd_line_handler((char *)ptrParam, paramLen);
#endif
            break;
        default:
            break;
    }
}

#ifdef CMT_008_SPP_TOTA_V2

#define MIC_SELECT_TEST   0x50
#define ANC_MODE_TEST     0x51
#define FUNCTION_TEST     0x52
#define LEN_OF_ARRAY      512 /* Copy from 'LEN_OF_IMAGE_TAIL_TO_FINDKEY_WORD'*/


typedef enum 
{
    TALK_MIC = 1,
    L_FF_MIC,
    R_FF_MIC,
    L_FB_MIC,
    R_FB_MIC,
    MIC_RESET,
    ALL_SELECT_MIC = 0xFF
}MIC_SELECT_E;

typedef enum 
{
    ANC_MODE_CMD = 1,
    AWARENESS_MODE_CMD,
    ANC_OFF_CMD,
    ANC_NONE = 0xFF
}ANC_MODE_E;

typedef enum 
{
    POWER_OFF_CMD = 1,
    DISCONNECT_BT_CMD,
    ANC_WIRELESS_DEBUG_EN,
    ANC_WIRELESS_DEBUG_DIS,
    GET_FIRMWARE_VER,
    GET_FIRMWARE_VER_BUILD_DATE,
    FUNCTION_TEST_NONE = 0xFF
}FUNCTION_TEST_E;

static AUD_IO_PATH_T mic_select_spp_cmd = AUD_INPUT_PATH_MAINMIC;
static const char* image_info_build_data = "BUILD_DATE=";
extern const char sys_build_info[];

AUD_IO_PATH_T current_select_mic(void)
{
    return mic_select_spp_cmd;
}

static bool app_tota_send_response(APP_TOTA_CMD_RET_STATUS_E rsp_status, uint8_t * pdata, uint16_t dataLen)
{
    TOTA_LOG_DBG(0, "Response send data");

    /* Disable by Jay, only response data as same the RX.*/
    //dataLen = app_tota_tx_pack(pdata, dataLen);
    if (0 == dataLen)
    {
        return false;
    }

    if(rsp_status != TOTA_CMT_008_NOT_NEED_STATUS)
    {
        /* This is response one byte of status. */
        pdata[dataLen] = rsp_status;
        dataLen ++;
    }

    switch (tota_get_connect_path())
    {
        case APP_TOTA_VIA_SPP:
            return app_spp_tota_send_data(pdata, dataLen);

        default:
            return false;
    }
}

static int32_t find_key_word(uint8_t* targetArray, uint32_t targetArrayLen,
    uint8_t* keyWordArray,
    uint32_t keyWordArrayLen)
{
    if ((keyWordArrayLen > 0) && (targetArrayLen >= keyWordArrayLen))
    {
        uint32_t index = 0, targetIndex = 0;
        for (targetIndex = 0;targetIndex < targetArrayLen;targetIndex++)
        {
            for (index = 0;index < keyWordArrayLen;index++)
            {
                if (targetArray[targetIndex + index] != keyWordArray[index])
                {
                    break;
                }
            }

            if (index == keyWordArrayLen)
            {
                return targetIndex;
            }
        }

        return -1;
    }
    else
    {
        return -1;
    }
}

static APP_TOTA_CMD_RET_STATUS_E app_tota_get_buidl_data(uint8_t *buildData) 
{
    if(NULL == buildData)
    {
        return TOTA_CMD_HANDLING_FAILED;
    }

    int32_t found = find_key_word((uint8_t*)&sys_build_info,\
                                    LEN_OF_ARRAY,\
                                    (uint8_t*)image_info_build_data,\
                                    strlen(image_info_build_data));

    if (-1 == found)
    {
        return TOTA_CMD_HANDLING_FAILED;
    }

    memcpy(buildData, (uint8_t*)&sys_build_info+found+strlen(image_info_build_data), 20);

    TRACE(1,"[%s]buildData is 0x%s", __func__, buildData);

    return TOTA_NO_ERROR;
}

static void app_tota_vendor_cmd_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(2,"Func code 0x%x, param len %d", funcCode, paramLen);
    TOTA_LOG_DBG(0,"Param content:");
    DUMP8("%02x ", ptrParam, paramLen);

    if(funcCode != OP_TOTA_CMT008_SPP_TEST_CMD)
        return;

    switch (funcCode)
    {
        case OP_TOTA_CMT008_SPP_TEST_CMD:
            switch (ptrParam[4])
            {
                case MIC_SELECT_TEST:
                    switch (ptrParam[5])
                    {
                        case TALK_MIC:
                            TOTA_LOG_DBG(0,"TALK MIC");
                            mic_select_spp_cmd = AUD_INPUT_PATH_MAINMIC;
                        break;
                        case L_FF_MIC:
                            TOTA_LOG_DBG(0,"L FF MIC");
                            mic_select_spp_cmd = AUD_INPUT_PATH_LFFMIC_SPP;
                        break;
                        case R_FF_MIC:
                            TOTA_LOG_DBG(0,"R FF MIC");
                            mic_select_spp_cmd = AUD_INPUT_PATH_RFFMIC_SPP;
                        break;
                        case L_FB_MIC:
                            TOTA_LOG_DBG(0,"L FB MIC");
                            mic_select_spp_cmd = AUD_INPUT_PATH_LFBMIC_SPP;
                        break;
                        case R_FB_MIC:
                            TOTA_LOG_DBG(0,"R FB MIC");
                            mic_select_spp_cmd = AUD_INPUT_PATH_RFBMIC_SPP;
                        break;
                        case MIC_RESET:
                            TOTA_LOG_DBG(0,"Reset into TALK MIC");
                            mic_select_spp_cmd = AUD_INPUT_PATH_MAINMIC;
                        break;
                        default:
                            TOTA_LOG_DBG(0,"Unsupported command ID of 'Mic select'");
                            app_tota_send_response(TOTA_INVALID_CMD, ptrParam, paramLen);
                            return;
                    }
                break;

                case ANC_MODE_TEST:
                    switch (ptrParam[5])
                    {
                        case ANC_MODE_CMD:
                            TOTA_LOG_DBG(0,"'ANC MODE'");
                            app_anc_switch(APP_ANC_MODE1);
                        break;
                        case AWARENESS_MODE_CMD:
                            TOTA_LOG_DBG(0,"'Awareness MODE'");
                            app_anc_switch(APP_ANC_MODE2);
                        break;
                        case ANC_OFF_CMD:
                            TOTA_LOG_DBG(0,"'ANC OFF'");
                            app_anc_switch(APP_ANC_MODE_OFF);
                        break;
                        default:
                            TOTA_LOG_DBG(0,"Unsupported command ID of 'ANC mode test'");
                            app_tota_send_response(TOTA_INVALID_CMD, ptrParam, paramLen);
                            return;
                    }
                break;

                case FUNCTION_TEST:
                    switch (ptrParam[5])
                    {
                        case POWER_OFF_CMD:
                            TOTA_LOG_DBG(0,"'Shut down'");
                            /* Send response before asking to shutdown, to ensure response gets sent. */
                            app_tota_send_response(TOTA_NO_ERROR, ptrParam, paramLen);
                            app_shutdown();
                            return;
                        break;
                        case DISCONNECT_BT_CMD:
                            TOTA_LOG_DBG(0,"'Disconnect BT'");
                            /* Send response before asking to disconnect, to ensure response gets sent. */
                            app_tota_send_response(TOTA_NO_ERROR, ptrParam, paramLen);
                            app_ibrt_if_event_entry(APP_UI_EV_DOCK);
                            app_ibrt_if_event_entry(APP_UI_EV_CASE_CLOSE);
                            return;
                        break;
                        case ANC_WIRELESS_DEBUG_EN:
                            TOTA_LOG_DBG(0,"ANC_WIRELESS_DEBUG_EN");
                            // TODO: this whether need? to check it. Add by Jay.
                            app_tota_send_response(TOTA_INVALID_CMD, ptrParam, paramLen);
                        break;
                        case ANC_WIRELESS_DEBUG_DIS:
                            TOTA_LOG_DBG(0,"ANC_WIRELESS_DEBUG_DIS");
                            // TODO: this whether need? to check it. Add by Jay.
                            app_tota_send_response(TOTA_INVALID_CMD, ptrParam, paramLen);
                        break;
#ifdef CMT_008_SPP_GET_FW
                        case GET_FIRMWARE_VER:
                            {
                                TOTA_LOG_DBG(0,"'Get Firmware version'");
                                uint16_t data_len = strlen(app_tota_get_fw_version());
                                app_tota_send_response(TOTA_CMT_008_NOT_NEED_STATUS, (uint8*) app_tota_get_fw_version(), data_len);
                                return;
                            }
                        break;
                        case GET_FIRMWARE_VER_BUILD_DATE:
                            {
                                TOTA_LOG_DBG(0,"'Get Firmware version build date'");
                                uint8_t build_date[32];
                                if(app_tota_get_buidl_data(build_date) == TOTA_NO_ERROR)
                                    app_tota_send_response(TOTA_CMT_008_NOT_NEED_STATUS, build_date, sizeof(build_date));
                                else
                                    app_tota_send_response(TOTA_CMD_HANDLING_FAILED, ptrParam, paramLen);
                                return;
                            }
                        break;
#endif /*CMT_008_SPP_GET_FW*/
                        default:
                            TOTA_LOG_DBG(0,"Unsupported command ID of 'function test'");
                            app_tota_send_response(TOTA_INVALID_CMD, ptrParam, paramLen);
                            return;
                    }
                break;

                default:
                    TOTA_LOG_DBG(0,"Invalid command");
                    app_tota_send_response(TOTA_INVALID_CMD, ptrParam, paramLen);
                    return;
            }
            app_tota_send_response(TOTA_NO_ERROR, ptrParam, paramLen);
        break;

        default:
            TOTA_LOG_DBG(0,"Invalid TOTA command");
            app_tota_send_response(TOTA_INVALID_CMD, ptrParam, paramLen);
            return;
    }
}
#endif /*CMT_008_SPP_TOTA_V2*/

TOTA_COMMAND_TO_ADD(OP_TOTA_STRING, app_tota_demo_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_DEMO_CMD, app_tota_demo_cmd_handler, false, 0, NULL );
