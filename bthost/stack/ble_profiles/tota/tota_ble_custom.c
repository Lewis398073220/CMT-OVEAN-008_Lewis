/***************************************************************************
 *
 * Copyright 2023 Add by Jay
 * Handle custom application
 * All rights reserved. All unpublished rights reserved.
 *
 ****************************************************************************/

#ifdef CMT_008_BLE_ENABLE

#include "tota_ble_custom.h"
#include "rwip_config.h"
#include "gap.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "co_utils.h"
#include "bluetooth_bt_api.h"




static bool custom_tota_ble_send_ind_ntf_generic(bool isNotification, uint8_t conidx, uint16_t handle, const uint8_t* ptrData, uint32_t length)
{
    TRACE(1, "[%s]  conidx:[%d]", __func__, conidx);
    enum gatt_evt_type evtType = isNotification?GATT_NOTIFY:GATT_INDICATE;

    PRF_ENV_T(tota) *tota_env = PRF_ENV_GET(TOTA, tota);

    if ((tota_env->ntfIndEnableFlag[conidx])&(1 << (uint8_t)evtType)) 
    {

        co_buf_t* p_buf = NULL;
        prf_buf_alloc(&p_buf, length);

        uint8_t* p_data = co_buf_data(p_buf);
        memcpy(p_data, ptrData, length);

        // Dummy parameter provided to GATT
        uint16_t dummy = 0;

        // Inform the GATT that notification must be sent
        uint16_t ret = gatt_srv_event_send(conidx, tota_env->srv_user_lid, dummy, evtType,
                            tota_env->shdl + handle, p_buf);

        // Release the buffer
        co_buf_release(p_buf);

        return (GAP_ERR_NO_ERROR == ret);
    }
    else
    {
        return false;
    }
}

static bool custom_tota_ble_send_notification(uint16_t handle, uint8_t* ptrData, uint32_t length)
{
    return custom_tota_ble_send_ind_ntf_generic(true, false, handle, ptrData, length);
}

bool custon_tota_ble_send_response(TOTA_BLE_STATUS_E rsp_status, uint8_t* ptrData, uint32_t ptrData_len)
{
    /* Add the last byte in data for response status. */
    ptrData[ptrData_len] = rsp_status;
    ptrData_len ++;

    return custom_tota_ble_send_notification(TOTA_IDX_VAL, ptrData, ptrData_len);
}

bool custon_tota_ble_send_notify_response(TOTA_BLE_STATUS_E rsp_status, uint8_t* ptrData, uint32_t ptrData_len)
{
    /* Add the last byte in data for response status. */
    ptrData[ptrData_len] = rsp_status;
    ptrData_len ++;

    return custom_tota_ble_send_notification(TOTA_IDX1_VAL, ptrData, ptrData_len);
}

static void custom_tota_ble_command_set_handle(uint8_t* data, uint32_t data_len)
{
    switch (data[1])
    {
        case TOTA_BLE_CMT_COMMAND_SET_CLEAR_PAIRING_HISTORY:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_DEVICE_FACTORY:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_HEADSET_LIGHT_MODE:     
        break;

        case TOTA_BLE_CMT_COMMAND_SET_IN_EAR_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_NOISE_CANCELLING_MODE_AND_LEVEL:    
        break;

        case TOTA_BLE_CMT_COMMAND_SET_HEADSET_VOLUME:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_LOW_LATENCY_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_TOUCH_SENSITIVITY:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_TOUCH_TEACHING_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_ENTER_OTA_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_TIME_SYNC:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_TOUCH_FUNC_ON_OFF:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_L_R_CHANNEL_BALANCE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_MULTIPOINT_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_CHANGE_DEVICE_NAME:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SWITCHING_SOUND_PROMPTS:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SOUND_PROMPTS_LEVEL:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SHUTDOWN_TIME:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_CAMERA_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_STANDBY_TIME:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_EQ_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_USER_DEFINED_EQ:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SEND_MUSIC_EVENT:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SEND_CALL_EVENT:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SNED_BUTTON_EVENT:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_LANGUAGE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SIDETONE_CONTROL_STATUS:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_STANDBY_MODE_ACTIVELY:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_KEY_REDEFINITION:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_VOICE_NOISE_REDUCTION_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_VOICE_ASSISTANT_CONTROL:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_FLASHING_LIGHTS:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_HARSH_SOUND:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_VOKALEN_SOUND:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_DEFAULT_SETTING:
        break;

        default:
        break;
    }
}

static void custom_tota_ble_command_get_handle(uint8_t* data, uint32_t data_len)
{
    switch (data[1])
    {
        case TOTA_BLE_CMT_COMMAND_GET_HEADSET_LIGHT_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_IN_EAR_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_NOISE_CANCELLING_MODE_AND_LEVEL:     
        break;

        case TOTA_BLE_CMT_COMMAND_GET_HEADSET_VOLUME:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_LOW_LATENCY_MODE:    
        break;

        case TOTA_BLE_CMT_COMMAND_GET_TOUCH_SENSITIVITY:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_ENTER_OTA_MODE_STATUS:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_TOUCH_FUNC_ON_OFF:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_HEADSET_ADDRESS:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_L_R_CHANNEL_BALANCE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_MULTIPOINT_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_DEVICE_NAME:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_IDENTIFILER_SOUND_PROMPTS:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_AUDIO_CODEC_FORMAT:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_SOUND_PROMPTS_LEVEL:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_SHUTDOWN_TIME:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_CAMERA_SWITCH_STATE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_STANDBY_TIME:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_EQ_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_USER_DEFINED_EQ:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_BATTERY_LEVEL:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_PCBA_VER:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_API_VER:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_SALES_REGION:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_CHIPSET_INFO:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_SIDETONE_CONTROL_STATUS:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_STANDBY_MODE_ACTIVELY:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_KEY_REDEFINITION:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_VOICE_NOISE_REDUCTION_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_FIRMWARE_VERSION:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_PRODUCT_INFO:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_VOICE_ASSISTANT_CONTROL:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_VOKALEN_SOUND:
        break;

        default:
        break;
    }
}

static void custom_tota_ble_command_notify_handle(uint8_t* data, uint32_t data_len)
{
    switch (data[1])
    {
        case TOTA_BLE_CMT_COMMAND_NOTIFY_HEADSET_LIGHT_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_IN_EAR_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_NOISE_CANCELLING_MODE_AND_LEVEL:     
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_HEADSET_VOLUME:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_LOW_LATENCY_MODE:    
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_TOUCH_SENSITIVITY:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_SOUND_PROMPTS_LEVEL:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_EQ_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_BATTERY_LEVEL:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_SIDETONE_CONTROL_STATUS:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_STANDBY_MODE_ACTIVELY:
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY_KEY_REDEFINITION:
        break;

        default:
        break;
    }
}

// TODO: Jay
void custom_tota_ble_data_handle(uint8_t* ptrData, uint32_t length)
{
    switch (ptrData[0])
    {
        case TOTA_BLE_CMT_COMMAND_SET:
            TRACE(1 ,"[%s]  TOTA_BLE_CMT_COMMAND_SET", __func__);
            custom_tota_ble_command_set_handle(ptrData, length);
        break;

        case TOTA_BLE_CMT_COMMAND_GET:
            TRACE(1 ,"[%s]  TOTA_BLE_CMT_COMMAND_GET", __func__);
            custom_tota_ble_command_get_handle(ptrData, length);
        break;

        case TOTA_BLE_CMT_COMMAND_NOTIFY:
            TRACE(1 ,"[%s]  TOTA_BLE_CMT_COMMAND_NOTIFY", __func__);
            custom_tota_ble_command_notify_handle(ptrData, length);
        break;
        default:
            TRACE(1 ,"[%s]  Invalid command setting", __func__);
            custon_tota_ble_send_response(NOT_SUPPORT_STATUS, ptrData, length);
            custon_tota_ble_send_notify_response(NOT_SUPPORT_STATUS, ptrData, length); //TODO: will be removed it.
            return;
    }
}

#endif /* CMT_008_BLE_ENABLE */
