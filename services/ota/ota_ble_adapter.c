#ifdef __IAG_BLE_INCLUDE__
#include "cmsis_os.h"
#include "cmsis.h"
#include "ota_ble_adapter.h"
#include "cqueue.h"
#include "bluetooth_bt_api.h"
#include "bluetooth_ble_api.h"
#include "app_bt_func.h"
#include "ota_control.h"
#include "ota_bes.h"

#if defined(IBRT)
#include "app_ibrt_if.h"
#if defined(IBRT_UI_V1)
#include "app_tws_ibrt.h"
#else
#include "app_tws_ibrt_conn_api.h"
extern bool app_ibrt_if_is_any_mobile_connected(void); // TODO:
#endif
#endif

#if defined(BES_OTA)
#define OTA_BLE_RX_EVENT_MAX_MAILBOX    16
#define OTA_BLE_RX_BUF_SIZE             (2048)

static uint8_t ota_ble_rx_buf[OTA_BLE_RX_BUF_SIZE];
static CQueue ota_ble_rx_cqueue;

#define VOICEPATH_COMMON_OTA_BUFF_SIZE 4096

/// health thermometer application environment structure
struct ota_ble_env_info
{
    uint8_t connectionIndex;
    uint8_t isNotificationEnabled;
    uint16_t mtu[BLE_CONNECTION_MAX];
};

/// Health Thermomter Application environment
struct ota_ble_env_info ota_ble_env = {0};

uint8_t* app_voicepath_get_common_ota_databuf(void)
{
    static uint8_t voicepath_common_ota_buf[VOICEPATH_COMMON_OTA_BUFF_SIZE];
    return voicepath_common_ota_buf;
}

#ifdef BES_OTA
extern void ota_control_handle_received_data(uint8_t *otaBuf, bool isViaBle,uint16_t dataLenth);
#endif

#ifdef BLE_TOTA_ENABLED
extern uint8_t tota_ble_get_conidx(void);
extern void app_tota_handle_received_data(uint8_t* buffer, uint16_t maxBytes);
#endif

static osThreadId app_ble_rx_thread = NULL;
static bool ble_rx_thread_init_done = false;

extern void gsound_send_control_rx_cfm(uint8_t conidx);
extern void app_ota_send_rx_cfm(uint8_t conidx);

static void ota_ble_rx_handler_thread(const void *arg);

#ifndef OTA_BLE_RX_HANDLER_THREAD_SIZE
#define OTA_BLE_RX_HANDLER_THREAD_SIZE  (1024*3)
#endif

osThreadDef(ota_ble_rx_handler_thread, osPriorityNormal, 1, OTA_BLE_RX_HANDLER_THREAD_SIZE, "ota_ble_rx");

osMailQDef (ota_ble_rx_event_mailbox, OTA_BLE_RX_EVENT_MAX_MAILBOX, BLE_RX_EVENT_T);
static osMailQId ota_ble_rx_event_mailbox_id = NULL;

static int32_t ota_ble_rx_event_mailbox_init(void)
{
    ota_ble_rx_event_mailbox_id = osMailCreate(osMailQ(ota_ble_rx_event_mailbox), NULL);
    if (ota_ble_rx_event_mailbox_id == NULL) {
        TRACE(0, "Failed to Create ota_ble_rx_event_mailbox");
        return -1;
    }
    return 0;
}

static void ota_ble_update_init_state(bool state)
{
    ble_rx_thread_init_done = state;
}

static bool ota_ble_get_init_state(void)
{
    return ble_rx_thread_init_done;
}

static void ota_ble_rx_mailbox_free(BLE_RX_EVENT_T* rx_event)
{
    osStatus status;

    status = osMailFree(ota_ble_rx_event_mailbox_id, rx_event);
    ASSERT(osOK == status, "Free ble rx event mailbox failed!");
}
#if !defined(OTA_OVER_TOTA_ENABLED)
static void ota_ble_send_notification(uint8_t *data, uint32_t data_len)
{
    bes_ble_ota_send_notification(ota_ble_env.connectionIndex, data, data_len);
}

static void ota_ble_event_callback(bes_ble_ota_event_param_t *param)
{
    switch (param->event_type){
    case BES_BLE_OTA_CCC_CHANGED:
        TRACE(1,"ota data ccc changed to %d", param->param.ntf_en);
        ota_ble_env.isNotificationEnabled = (param->param.ntf_en > 0);

        if (ota_ble_env.isNotificationEnabled)
        {
            TRACE(2, "%s %d", __func__, param->conidx);
            ota_ble_env.connectionIndex = param->conidx;
            app_ota_connected(APP_OTA_CONNECTED);
            ota_control_set_datapath_type(DATA_PATH_BLE);
            ota_control_register_transmitter(ota_ble_send_notification);
            bes_ble_gap_conn_update_param(param->conidx, 10, 15, 20000, 0);

            if (ota_ble_env.mtu[param->conidx])
            {
                ota_control_update_MTU(ota_ble_env.mtu[param->conidx]);
            }
        }
        break;
    case BES_BLE_OTA_DISCONN:
        if (param->conidx == ota_ble_env.connectionIndex && \
            (DATA_PATH_BLE == ota_control_get_datapath_type()))
        {
            ota_ble_env.connectionIndex = INVALID_CONNECTION_INDEX;
            ota_ble_env.isNotificationEnabled = false;
            ota_ble_env.mtu[param->conidx] = 0;
            app_ota_disconnected(APP_OTA_DISCONNECTED, APP_OTA_LINK_TYPE_BLE);
        }
        break;
    case BES_BLE_OTA_RECEVICE_DATA:
        TRACE(2, "%s", __func__);
        ota_ble_push_rx_data(BLE_RX_DATA_SELF_OTA, ota_ble_env.connectionIndex,
                             param->param.receive_data.data, param->param.receive_data.data_len);
        break;
    case BES_BLE_OTA_MTU_UPDATE:
        if (param->conidx == ota_ble_env.connectionIndex)
        {
            ota_control_update_MTU(param->param.mtu);
        }
        else
        {
            ota_ble_env.mtu[param->conidx] = param->param.mtu;
        }
        break;
    case BES_BLE_OTA_SEND_DONE:
        break;
    default:
        break;
    }
}

uint8_t ota_ble_get_conidx(void)
{
    TRACE(2, "%s %d", __func__, ota_ble_env.connectionIndex);
    return ota_ble_env.connectionIndex;
}
#endif

void ota_ble_adapter_init(void)
{
    if (!ota_ble_get_init_state())
    {
        InitCQueue(&ota_ble_rx_cqueue, OTA_BLE_RX_BUF_SIZE, ( CQItemType * )ota_ble_rx_buf);
        ota_ble_rx_event_mailbox_init();

        app_ble_rx_thread = osThreadCreate(osThread(ota_ble_rx_handler_thread), NULL);
        ota_ble_update_init_state(true);

#if !defined(OTA_OVER_TOTA_ENABLED)
        // Reset the environment
        ota_ble_env.connectionIndex =  INVALID_CONNECTION_INDEX;
        ota_ble_env.isNotificationEnabled = false;
        memset((uint8_t *)&(ota_ble_env.mtu), 0, sizeof(ota_ble_env.mtu));
        bes_ble_ota_event_reg(ota_ble_event_callback);
#endif
    }
    else
    {
        TRACE(0, "rx already initialized");
    }
}

void ota_ble_push_rx_data(uint8_t flag, uint8_t conidx, uint8_t* ptr, uint16_t len)
{
    uint32_t lock = int_lock();
    int32_t ret = EnCQueue(&ota_ble_rx_cqueue, ptr, len);
    int_unlock(lock);
    if(CQ_OK != ret){
        TRACE(2,"BLE rx buffer overflow! %d,%d",AvailableOfCQueue(&ota_ble_rx_cqueue),len);
        return;
    }
    BLE_RX_EVENT_T* event = (BLE_RX_EVENT_T*)osMailAlloc(ota_ble_rx_event_mailbox_id, 0);
    event->flag = flag;
    event->conidx = conidx;
    event->ptr = ptr;
    event->len = len;
    osMailPut(ota_ble_rx_event_mailbox_id, event);
}

static int32_t ota_ble_rx_mailbox_get(BLE_RX_EVENT_T** rx_event)
{
    osEvent evt;
    evt = osMailGet(ota_ble_rx_event_mailbox_id, osWaitForever);
    if (evt.status == osEventMail) {
        *rx_event = (BLE_RX_EVENT_T *)evt.value.p;
        TRACE(0, "flag %d ptr %p len %d", (*rx_event)->flag,
            (*rx_event)->ptr, (*rx_event)->len);
        return 0;
    }
    return -1;
}

static void ota_ble_rx_handler_thread(void const *argument)
{
    while (true)
    {
        BLE_RX_EVENT_T* rx_event = NULL;
        if (!ota_ble_rx_mailbox_get(&rx_event))
        {
            uint8_t tmpData[512];
            uint32_t lock = int_lock();
            DeCQueue(&ota_ble_rx_cqueue, tmpData, rx_event->len);
            int_unlock(lock);
            switch (rx_event->flag)
            {
                #if defined(BES_OTA) && !defined(OTA_OVER_TOTA_ENABLED)
                case BLE_RX_DATA_SELF_OTA:
                    #if defined(IBRT)
                    #ifdef IBRT_OTA
                    #if defined(__GATT_OVER_BR_EDR__)
                    if (app_btgatt_over_br_edr_enabled() && btif_btgatt_is_connected_by_conidx(ota_ble_env.connectionIndex))
                    {
                        ota_control_handle_received_data(tmpData, false, rx_event->len);
                    }
                    else
                    #endif
                    {
                        ota_control_handle_received_data(tmpData, true, rx_event->len);
                    }

                    // TODO: freddie comment: should not send confirm here
                    bes_ble_ota_send_rx_cfm(ota_ble_env.connectionIndex);
                    #endif
                    #else
                        ota_bes_handle_received_data(tmpData, true,rx_event->len);
                    #endif
                    break;
                #endif
                #ifdef BLE_TOTA_ENABLED
                case BLE_RX_DATA_SELF_TOTA:
                #if !defined(TOTA_v2) // enabled when TOTA v2 BLE is ready
                    app_tota_handle_received_data(tmpData, rx_event->len);
                #endif
                    break;
                #if defined(TOTA_v2)
                case BLE_RX_DATA_SELF_TOTA_OTA:
                    #if defined(IBRT)
                    #ifdef IBRT_OTA
                    #if defined(__GATT_OVER_BR_EDR__)
                    if (app_btgatt_over_br_edr_enabled() && btif_btgatt_is_connected_by_conidx(tota_ble_get_conidx()))
                    {
                        ota_control_handle_received_data(tmpData, false, rx_event->len);
                    }
                    else
                    #endif
                    {
                        ota_control_handle_received_data(tmpData, true, rx_event->len);
                    }
                    #endif
                    #else
                        ota_bes_handle_received_data(tmpData, true, rx_event->len);
                    #endif
                #endif
                #endif

                default:
                    break;
            }
            ota_ble_rx_mailbox_free(rx_event);
        }
    }
}
#endif //BES_OTA
#endif //__IAG_BLE_INCLUDE__
