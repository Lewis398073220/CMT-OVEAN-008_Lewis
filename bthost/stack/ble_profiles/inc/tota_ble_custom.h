/***************************************************************************
 *
 * Copyright 2023 Add by Jay
 * All rights reserved. All unpublished rights reserved.
 *
 ****************************************************************************/

#ifdef CMT_008_BLE_ENABLE

#include "tota_ble.h"



typedef enum
{
    SUCCESS_STATUS = 0x00,
    NOT_SUPPORT_STATUS,
    DISALLOW_STATUS,
    NO_RESOURCE_STATUS,
    FORMAT_ERROR_STATUS,
    PARAMETER_ERROR_STATUS,
    FAIL_STATUS = 0xFF,
    NO_NEED_STATUS_RESP = 0xEE,
} TOTA_BLE_STATUS_E;


#define TOTA_BLE_CMT_COMMAND_NOTIFY                          0x20
#define TOTA_BLE_CMT_COMMAND_GET                             0x40
#define TOTA_BLE_CMT_COMMAND_SET                             0x80


/* Command Header: 0x80, TOTA_BLE_CMT_COMMAND_SET. */
#define TOTA_BLE_CMT_COMMAND_SET_CLEAR_PAIRING_HISTORY               0x0001  /*Clear pairing history*/
#define TOTA_BLE_CMT_COMMAND_SET_DEVICE_FACTORY                      0x0002  /*Device reset to factory settings*/
#define TOTA_BLE_CMT_COMMAND_SET_HEADSET_LIGHT_MODE                  0x0004  /*Set headset light mode*/
#define TOTA_BLE_CMT_COMMAND_SET_IN_EAR_SWITCH                       0x0005  /*Set in-ear detection switch*/
#define TOTA_BLE_CMT_COMMAND_SET_NOISE_CANCELLING_MODE_AND_LEVEL     0x0006  /*Set noise cancelling mode and level*/
#define TOTA_BLE_CMT_COMMAND_SET_HEADSET_VOLUME                      0x0007  /*Set headset volume*/
#define TOTA_BLE_CMT_COMMAND_SET_LOW_LATENCY_MODE                    0x0008  /*Set low latency mode*/
#define TOTA_BLE_CMT_COMMAND_SET_TOUCH_SENSITIVITY                   0x0009  /*Set touch sensitivity*/
#define TOTA_BLE_CMT_COMMAND_SET_TOUCH_TEACHING_MODE                 0x000A  /*Set touch teaching mode*/
#define TOTA_BLE_CMT_COMMAND_SET_ENTER_OTA_MODE                      0x000B  /*Set enter OTA mode*/
#define TOTA_BLE_CMT_COMMAND_SET_TIME_SYNC                           0x000C  /*Set time synchronization*/
#define TOTA_BLE_CMT_COMMAND_SET_TOUCH_FUNC_ON_OFF                   0x000D  /*Set touch function state enable or disable*/
#define TOTA_BLE_CMT_COMMAND_SET_L_R_CHANNEL_BALANCE                 0x0010  /*Set left and right channel balance*/ 
#define TOTA_BLE_CMT_COMMAND_SET_MULTIPOINT_SWITCH                   0x0011  /*Set multipoint switch*/
#define TOTA_BLE_CMT_COMMAND_SET_CHANGE_DEVICE_NAME                  0x0012  /*Set change device name*/
#define TOTA_BLE_CMT_COMMAND_SET_SWITCHING_SOUND_PROMPTS             0x0013  /*Set switching sound prompts*/
#define TOTA_BLE_CMT_COMMAND_SET_SOUND_PROMPTS_LEVEL                 0x0016  /*Set sound prompts level*/
#define TOTA_BLE_CMT_COMMAND_SET_SHUTDOWN_TIME                       0x0017  /*Set shutdown time*/
#define TOTA_BLE_CMT_COMMAND_SET_CAMERA_SWITCH                       0x0018  /*Set camera switch*/
#define TOTA_BLE_CMT_COMMAND_SET_STANDBY_TIME                        0x0019  /*Set standby time*/
#define TOTA_BLE_CMT_COMMAND_SET_EQ_MODE                             0x001A  /*Set EQ mode*/
#define TOTA_BLE_CMT_COMMAND_SET_USER_DEFINED_EQ                     0x001B  /*Set user defined EQ*/
#define TOTA_BLE_CMT_COMMAND_SET_SEND_MUSIC_EVENT                    0x001C  /*Sends music event*/
#define TOTA_BLE_CMT_COMMAND_SET_SEND_CALL_EVENT                     0x001D  /*Sends call event*/
#define TOTA_BLE_CMT_COMMAND_SET_SNED_BUTTON_EVENT                   0x001E  /*Sends button event*/
#define TOTA_BLE_CMT_COMMAND_SET_LANGUAGE                            0x0020  /*Set language*/
#define TOTA_BLE_CMT_COMMAND_SET_SIDETONE_CONTROL_STATUS             0x0025  /*Set sidetone control status(disable or enable)*/
#define TOTA_BLE_CMT_COMMAND_SET_STANDBY_MODE_ACTIVELY               0x0026  /*Set standby mode actively*/
#define TOTA_BLE_CMT_COMMAND_SET_KEY_REDEFINITION                    0x0027  /*Set keys redefinition*/
#define TOTA_BLE_CMT_COMMAND_SET_VOICE_NOISE_REDUCTION_MODE          0x0028  /*Set Voice Noise Reduction Mode*/
#define TOTA_BLE_CMT_COMMAND_SET_VOICE_ASSISTANT_CONTROL             0x0031  /*Set voice assistant Control*/
#define TOTA_BLE_CMT_COMMAND_SET_FLASHING_LIGHTS                     0x0032  /*Set flashing lights(find my)*/
#define TOTA_BLE_CMT_COMMAND_SET_HARSH_SOUND                         0x0033  /*Set harsh sound(find my)*/
#define TOTA_BLE_CMT_COMMAND_SET_VOKALEN_SOUND                       0x0034  /*Set vokalen sound*/
#define TOTA_BLE_CMT_COMMAND_SET_DEFAULT_SETTING                     0x0035  /*Set restore default settings*/


/* Command Header: 0x40, TOTA_BLE_CMT_COMMAND_GET. */
#define TOTA_BLE_CMT_COMMAND_GET_HEADSET_LIGHT_MODE                  0x0004  /*Get headset light mode*/
#define TOTA_BLE_CMT_COMMAND_GET_IN_EAR_SWITCH                       0x0005  /*Get in-ear detection switch*/
#define TOTA_BLE_CMT_COMMAND_GET_NOISE_CANCELLING_MODE_AND_LEVEL     0x0006  /*Get noise cancelling mode and level*/
#define TOTA_BLE_CMT_COMMAND_GET_HEADSET_VOLUME                      0x0007  /*Get headset volume*/
#define TOTA_BLE_CMT_COMMAND_GET_LOW_LATENCY_MODE                    0x0008  /*Get low latency mode*/
#define TOTA_BLE_CMT_COMMAND_GET_TOUCH_SENSITIVITY                   0x0009  /*Get touch sensitivity*/
#define TOTA_BLE_CMT_COMMAND_GET_ENTER_OTA_MODE_STATUS               0x000B  /*Get enter OTA mode status*/
#define TOTA_BLE_CMT_COMMAND_GET_TOUCH_FUNC_ON_OFF                   0x000D  /*Get touch function state enable or disable*/
#define TOTA_BLE_CMT_COMMAND_GET_HEADSET_ADDRESS                     0x000F  /*Get headset address*/
#define TOTA_BLE_CMT_COMMAND_GET_L_R_CHANNEL_BALANCE                 0x0010  /*Get left and right channel balance*/
#define TOTA_BLE_CMT_COMMAND_GET_MULTIPOINT_SWITCH                   0x0011  /*Get multipoint switch*/
#define TOTA_BLE_CMT_COMMAND_GET_DEVICE_NAME                         0x0012  /*Get device name*/
#define TOTA_BLE_CMT_COMMAND_GET_IDENTIFILER_SOUND_PROMPTS           0x0013  /*Get identifier of sound prompts*/
#define TOTA_BLE_CMT_COMMAND_GET_AUDIO_CODEC_FORMAT                  0x0015  /*Get audio codec format*/
#define TOTA_BLE_CMT_COMMAND_GET_SOUND_PROMPTS_LEVEL                 0x0016  /*Get sound prompts level*/
#define TOTA_BLE_CMT_COMMAND_GET_SHUTDOWN_TIME                       0x0017  /*Get shutdown time*/
#define TOTA_BLE_CMT_COMMAND_GET_CAMERA_SWITCH_STATE                 0x0018  /*Get camera switch state*/
#define TOTA_BLE_CMT_COMMAND_GET_STANDBY_TIME                        0x0019  /*Get standby time*/
#define TOTA_BLE_CMT_COMMAND_GET_EQ_MODE                             0x001A  /*Get EQ mode*/
#define TOTA_BLE_CMT_COMMAND_GET_USER_DEFINED_EQ                     0x001B  /*Get user defined EQ*/
#define TOTA_BLE_CMT_COMMAND_GET_BATTERY_LEVEL                       0x001F  /*Get battery level*/
#define TOTA_BLE_CMT_COMMAND_GET_PCBA_VER                            0x0021  /*Get PCBA Version*/
#define TOTA_BLE_CMT_COMMAND_GET_API_VER                             0x0022  /*Get API Version*/
#define TOTA_BLE_CMT_COMMAND_GET_SALES_REGION                        0x0023  /*Get sales region*/
#define TOTA_BLE_CMT_COMMAND_GET_CHIPSET_INFO                        0x0024  /*Get chipset info*/
#define TOTA_BLE_CMT_COMMAND_GET_SIDETONE_CONTROL_STATUS             0x0025  /*Get sidetone control status(disable or enable)*/
#define TOTA_BLE_CMT_COMMAND_GET_STANDBY_MODE_ACTIVELY               0x0026  /*Get standby mode actively*/
#define TOTA_BLE_CMT_COMMAND_GET_KEY_REDEFINITION                    0x0027  /*Get keys redefinition*/
#define TOTA_BLE_CMT_COMMAND_GET_VOICE_NOISE_REDUCTION_MODE          0x0028  /*Get Voice Noise Reduction Mode*/
#define TOTA_BLE_CMT_COMMAND_GET_FIRMWARE_VERSION                    0x0029  /*Get firmware version*/
#define TOTA_BLE_CMT_COMMAND_GET_PRODUCT_INFO                        0x0030  /*Get product info*/
#define TOTA_BLE_CMT_COMMAND_GET_VOICE_ASSISTANT_CONTROL             0x0031  /*Get voice assistant Control*/
#define TOTA_BLE_CMT_COMMAND_GET_VOKALEN_SOUND                       0x0034  /*Get vokalen sound*/


/* Command Header: 0x20, TOTA_BLE_CMT_COMMAND_NOTIFY. */
#define TOTA_BLE_CMT_COMMAND_NOTIFY_HEADSET_LIGHT_MODE               0x0004  /*Notify headset light mode*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_IN_EAR_SWITCH                    0x0005  /*Notify in-ear detection switch*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_NOISE_CANCELLING_MODE_AND_LEVEL  0x0006  /*Notify noise cancelling mode and level*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_HEADSET_VOLUME                   0x0007  /*Notify headset volume*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_LOW_LATENCY_MODE                 0x0008  /*Notify low latency mode*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_TOUCH_SENSITIVITY                0x0009  /*Notify touch sensitivity*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_SOUND_PROMPTS_LEVEL              0x0016  /*Notify sound prompts level*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_EQ_MODE                          0x001A  /*Notify EQ mode*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_BATTERY_LEVEL                    0x001F  /*Notify battery level*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_SIDETONE_CONTROL_STATUS          0x0025  /*Notify sidetone control status(disable or enable)*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_STANDBY_MODE_ACTIVELY            0x0026  /*Notify standby mode actively*/
#define TOTA_BLE_CMT_COMMAND_NOTIFY_KEY_REDEFINITION                 0x0027  /*Notify keys redefinition*/


#ifdef __cplusplus
extern "C" {
#endif

void custom_tota_ble_data_handle(uint8_t* ptrData, uint32_t length);
bool custon_tota_ble_send_response(TOTA_BLE_STATUS_E rsp_status, uint8_t* ptrData, uint32_t ptrData_len);
bool custon_tota_ble_send_notify_response(TOTA_BLE_STATUS_E rsp_status, uint8_t* ptrData, uint32_t ptrData_len);

#ifdef __cplusplus
}
#endif

#endif /* CMT_008_BLE_ENABLE */
