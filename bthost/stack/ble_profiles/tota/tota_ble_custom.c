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
#include "besbt.h"
#include "tota_ble.h"
#include "app_battery.h"
#include "app_bt_stream.h"
#include "factory_section.h"
#include "app_ibrt_if.h"
#include "app_user.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "string.h"
#include "hal_trace.h"
#include "apps.h"
#include "app_bt.h"
#include "app_thread.h"
#include "tgt_hardware.h"
#include "app_bt_stream.h"
#include "bt_sco_chain.h"
#include "hal_codec.h"
#include "app_hfp.h"
#include "cst_capacitive_tp_hynitron_cst0xx.h"


static bool custom_tota_ble_send_ind_ntf_generic(bool isNotification, bool enable, uint8_t conidx, uint16_t handle, const uint8_t* ptrData, uint32_t length)
{
    TRACE(1, "[%s]  notify:[%d], conidx:[%d], handle:[%d]", __func__, isNotification, conidx, handle);
    enum gatt_evt_type evtType = enable?GATT_NOTIFY:GATT_INDICATE;

    PRF_ENV_T(tota) *tota_env = PRF_ENV_GET(TOTA, tota);

    if(isNotification & (1 << (uint8_t)evtType))
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

bool custon_tota_ble_send_response(TOTA_BLE_STATUS_E rsp_status, uint8_t* ptrData, uint32_t ptrData_len)
{
    if(rsp_status != NO_NEED_STATUS_RESP)
    {
        /* Add the last byte in data for response status. */
        ptrData[ptrData_len] = rsp_status;
        ptrData_len ++;
    }

    return custom_tota_ble_send_ind_ntf_generic(user_custom_get_notify_enable_idx(),\
                                                                        true,\
                                                                        false,\
                                                                        TOTA_IDX_VAL,\
                                                                        ptrData,\
                                                                        ptrData_len);
}

bool custon_tota_ble_send_notify_response(TOTA_BLE_STATUS_E rsp_status, uint8_t* ptrData, uint32_t ptrData_len)
{
    if(rsp_status != NO_NEED_STATUS_RESP)
    {
        /* Add the last byte in data for response status. */
        ptrData[ptrData_len] = rsp_status;
        ptrData_len ++;
    }

    return custom_tota_ble_send_ind_ntf_generic(user_custom_get_notify_enable_idx1(),\
                                                                        true,\
                                                                        false,\
                                                                        TOTA_IDX1_VAL,\
                                                                        ptrData,\
                                                                        ptrData_len);
}

static void custom_tota_ble_command_set_handle(uint8_t* data, uint32_t data_len)
{
    TOTA_BLE_STATUS_E rsp_status;

    switch (data[1])
    {
        case TOTA_BLE_CMT_COMMAND_SET_CLEAR_PAIRING_HISTORY:
            if(data[2] == 0x01 && data[3] == 0x01 && data_len == 0x04)
            {
                app_ibrt_if_nvrecord_delete_all_mobile_record();
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_DEVICE_FACTORY:
            if(data[2] == 0x01 && data[3] == 0x01 && data_len == 0x03)
            {
                user_custom_factory_reset();
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_HEADSET_LIGHT_MODE:     
        break;

        case TOTA_BLE_CMT_COMMAND_SET_IN_EAR_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_NOISE_CANCELLING_MODE_AND_LEVEL:    
        break;

        case TOTA_BLE_CMT_COMMAND_SET_HEADSET_VOLUME:
            if(data[2] == 0x01 && data_len == 0x04 && (data[3] >= 0x00 || data[3] <= 0x0F))
            {
                app_bt_stream_volumeset(data[3]);
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_LOW_LATENCY_MODE:
            if(data[2] == 0x01 && data_len == 0x04 && (data[3] == 0x00 || data[3] == 0x01))
            {
                user_custom_gaming_mode_set(data[3]);
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
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
            if(data[2] == 0x01 && (data[3] == 0x00 || data[3] == 0x01) && data_len == 0x04)
            {
                bool touch_lock = data[3] ? 1 : 0;
                user_custom_set_touch_clock(touch_lock);
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_L_R_CHANNEL_BALANCE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_MULTIPOINT_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_CHANGE_DEVICE_NAME:
            //set_bt_name_len = data[2];
            //memcpy(set_bt_name, &data[3], set_bt_name_len);

            user_custom_nvrecord_set_bt_name((char*) &data[3], data[2]);

            data[2] = 0x01;
            data_len = 0x04;
            custon_tota_ble_send_response(SUCCESS_STATUS, data, data_len);

            //factory_section_set_bt_name((char*) &data[3], (int) data[2]);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SWITCHING_SOUND_PROMPTS:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SOUND_PROMPTS_LEVEL:
            if(data[2] == 0x01 && data_len == 0x04)
            {
                user_custom_set_sound_prompt(data[3]);
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SHUTDOWN_TIME:
            if(data[2] == 0x04 && data[5] == 0x00 && data[6] == 0x00 && data_len == 0x07)
            {
                uint16_t shutdown_time = (data[3] << 8) | data[4];
                user_custom_set_shutdown_time(shutdown_time);
                data[2] = 0x01;
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_CAMERA_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_STANDBY_TIME:
            if(data[2] == 0x02 && data_len == 0x05)
            {
                uint16_t standby_time = (data[3] << 8) | data[4];
                user_custom_set_standby_time(standby_time);
                data[2] = 0x01;
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_EQ_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_USER_DEFINED_EQ:
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SEND_MUSIC_EVENT:
            if(data[2] == 0x01 && data_len == 0x04)
            {
                cst816s_ble_custom_set_event(data[3]); //TODO: Select media music event.
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SEND_CALL_EVENT:
            if(data[2] == 0x01 && data_len == 0x04)
            {
                cst816s_ble_custom_set_event(data[3]); //TODO: Select call status event.
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_SET_SNED_BUTTON_EVENT:
            if(data[2] == 0x01 && data_len == 0x04)
            {
                //cst816s_ble_custom_set_event(data[3]); //TODO: Select button event.
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
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
            if(data[2] == 0x01 && data_len == 0x04 && (data[3] == 0x00 || data[3] == 0x01))
            {
                app_hfp_siri_voice((bool) data[3]);
                data_len = 0x03;
                rsp_status = SUCCESS_STATUS;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
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
    TOTA_BLE_STATUS_E rsp_status;

    switch (data[1])
    {
        case TOTA_BLE_CMT_COMMAND_GET_HEADSET_LIGHT_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_IN_EAR_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_NOISE_CANCELLING_MODE_AND_LEVEL:     
        break;

        case TOTA_BLE_CMT_COMMAND_GET_HEADSET_VOLUME:
            if(data[2] == 0x00 && data_len == 0x03)
            {
                data[2] = 0x01;
                data[3] = app_bt_stream_local_volume_get();
                data_len = 0x04;
                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_GET_LOW_LATENCY_MODE:
            if(data[2] == 0x00 && data_len == 0x03)
            {
                data[2] = 0x01;
                data[3] = user_custom_gaming_mode_get();
                data_len = 0x04;
                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_GET_TOUCH_SENSITIVITY:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_ENTER_OTA_MODE_STATUS:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_TOUCH_FUNC_ON_OFF:
            if(data[2] == 0x00 && data_len == 0x03)
            {
                data[2] = 0x01;
                data[3] = user_custom_get_touch_clock();
                data_len = 0x04;
                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_GET_HEADSET_ADDRESS:
            if(data[2] == 0 && data_len == 3)
            {
                data[2] = 0x06;
                memcpy(&data[3], (uint8*) bt_get_local_address(), data[2]);
                data_len += data[2];
                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_GET_L_R_CHANNEL_BALANCE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_MULTIPOINT_SWITCH:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_DEVICE_NAME:
            {
                if(data[2] == 0 && data_len == 3)
                {
                    uint32_t resp_data_len = strlen(bt_get_local_name());
                    data[2] = resp_data_len;
                    memcpy(&data[3], (uint8*) bt_get_local_name(), resp_data_len);
                    data_len += resp_data_len;

                    rsp_status = NO_NEED_STATUS_RESP;
                }
                else
                {
                    rsp_status = NOT_SUPPORT_STATUS;
                }
                custon_tota_ble_send_response(rsp_status, data, data_len);
            }
        break;

        case TOTA_BLE_CMT_COMMAND_GET_IDENTIFILER_SOUND_PROMPTS:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_AUDIO_CODEC_FORMAT:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_SOUND_PROMPTS_LEVEL:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_SHUTDOWN_TIME:
            if(data[2] == 0x00 && data_len == 0x03)
            {
                data[3] = 0x00;
                data[4] = 0x00;
                
                uint16_t shutdown_time = user_custom_get_remaining_shutdown_time();
                data[5] = (shutdown_time & 0xFF00) >> 8;
                data[6] = shutdown_time & 0xFF;
                rsp_status = NO_NEED_STATUS_RESP;
                data_len = 0x07;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_GET_CAMERA_SWITCH_STATE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_STANDBY_TIME:
            if(data[2] == 0x00 && data_len == 0x03)
            {
                uint16_t standby_time = user_custom_get_standby_time();
                data[3] = (standby_time & 0xFF00) >> 8;
                data[4] = standby_time & 0xFF;
                data_len = 0x05;
                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_GET_EQ_MODE:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_USER_DEFINED_EQ:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_BATTERY_LEVEL:
            if(data[2] == 0 && data_len == 3)
            {
                uint32_t resp_data_len = sizeof(int8_t);
                data[2] = resp_data_len;
                data[3] = app_battery_current_level() * 10;
                data_len += resp_data_len;

                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
            {
                rsp_status = NOT_SUPPORT_STATUS;
            }
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_GET_PCBA_VER:
            if(data[2] == 0 && data_len == 3)
            {
                uint32_t resp_data_len = strlen(app_tota_get_pcba_version());
                data[2] = resp_data_len;
                memcpy(&data[3], (uint8*) app_tota_get_pcba_version(), resp_data_len);
                data_len += resp_data_len;

                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;
            custon_tota_ble_send_response(rsp_status, data, data_len);
        break;

        case TOTA_BLE_CMT_COMMAND_GET_API_VER:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_SALES_REGION:
        break;

        case TOTA_BLE_CMT_COMMAND_GET_CHIPSET_INFO:
            /* define chip info.
             *  +---------------+------------+------------+------+ +-----+
             *  |  +     byte4  |            |            |      | |     |
             *  |       +       |    0x01    |    0x02    | 0x03 | |  N  |
             *  |byte3       +  |            |            |      | |     |
             *  +---------------+------------+------------+------+ +-----+
             *  |     0x01      |  QCC3071   |  QCC3072   |      | |     |
             *  +---------------+------------+------------+------+ +-----+
             *  |     0x02      | BES2600IHC |  BES2700H  |      | |     |
             *  +---------------+------------+------------+------+ +-----+
             *  |     0x03      |            |            |      | |     |
             *  +---------------+------------+------------+------+ +-----+
             *  +---------------+------------+------------+------+ +-----+
             *  |       N       |            |            |      | |     |
             *  +---------------+------------+------------+------+ +-----+
             */	

            if(data[2] == 0x00 && data_len == 0x03)
            {
                data[2] = 0x02;
                data[3] = 0x02;
                data[4] = 0x02;
                data_len = 0x05;
                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
                rsp_status = NOT_SUPPORT_STATUS;

            custon_tota_ble_send_response(rsp_status, data, data_len);
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
            if(data[2] == 0 && data_len == 3)
            {
                uint32_t resp_data_len = strlen(app_tota_get_fw_version());
                data[2] = resp_data_len;
                memcpy(&data[3], (uint8*) app_tota_get_fw_version(), resp_data_len);
                data_len += resp_data_len;

                rsp_status = NO_NEED_STATUS_RESP;
            }
            else
            {
                rsp_status = NOT_SUPPORT_STATUS;
            }
            custon_tota_ble_send_response(rsp_status, data, data_len);
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
            //custon_tota_ble_send_notify_response(NOT_SUPPORT_STATUS, ptrData, length); //TODO: will be removed it.
            return;
    }
}

#endif /* CMT_008_BLE_ENABLE */
