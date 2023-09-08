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
#include "tgt_hardware.h"
#include "pmu.h"
#include "hal_timer.h"
#include "hal_gpadc.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_chipid.h"
#include "app_thread.h"
#include "app_battery.h"
#include "audio_policy.h"
#include "app_bt.h"
#include "apps.h"
#include "app_hfp.h"

#ifdef CMT_008_BLE_ENABLE
//#include "../../bthost/stack/ble_profiles/inc/toat_ble_custom.h"
//#include "toat_ble_custom.h"
#endif /*CMT_008_BLE_ENABLE*/
#include "app_user.h"

#ifdef APP_BATTERY_ENABLE
#include "app_status_ind.h"
#include "bluetooth_bt_api.h"
#include "app_media_player.h"
#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#endif
#include <stdlib.h>

#ifdef __INTERCONNECTION__
#include "app_ble_mode_switch.h"
#endif

#if (defined(BTUSB_AUDIO_MODE) || defined(BTUSB_AUDIO_MODE))
extern "C" bool app_usbaudio_mode_on(void);
#endif

#ifdef MORE_THAN_ONE_TYPE_OF_CHARGER
#include CHIP_SPECIFIC_HDR(charger)
#endif

#define APP_BATTERY_TRACE(s,...)
// TRACE(s, ##__VA_ARGS__)

#ifndef APP_BATTERY_GPADC_CH_NUM
#define APP_BATTERY_GPADC_CH_NUM (HAL_GPADC_CHAN_BATTERY)
#endif

#ifndef APP_BATTERY_ERR_MV
#define APP_BATTERY_ERR_MV (2800)
#endif

#ifndef APP_BATTERY_MIN_MV
#ifdef CMT_008_BATTERY_LOW
#define APP_BATTERY_MIN_MV (3450) //Modifed by Jay, changed from 3200 to 3450. 
#else /*CMT_008_BATTERY_LOW*/
#define APP_BATTERY_MIN_MV (3200)
#endif /*CMT_008_BATTERY_LOW*/
#endif

#ifndef APP_BATTERY_MAX_MV
#define APP_BATTERY_MAX_MV (4200)
#endif

#ifndef APP_BATTERY_PD_MV
#ifdef CMT_008_BATTERY_LOW
#define APP_BATTERY_PD_MV   (3400) //Modifed by Jay, changed from 3100 to 3400. 
#else /*CMT_008_BATTERY_LOW*/
#define APP_BATTERY_PD_MV   (3100)
#endif /*CMT_008_BATTERY_LOW*/

#endif

#ifndef APP_BATTERY_CHARGE_TIMEOUT_MIN
#define APP_BATTERY_CHARGE_TIMEOUT_MIN (210) //Modifed by Jay, changed from 90 to 210.
#endif

#ifndef APP_BATTERY_CHARGE_OFFSET_MV
#define APP_BATTERY_CHARGE_OFFSET_MV (50) //Modifed by Jay, changed from 20 to 50.
#endif

#ifndef CHARGER_PLUGINOUT_RESET
#define CHARGER_PLUGINOUT_RESET (1)
#endif

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_MS
#define CHARGER_PLUGINOUT_DEBOUNCE_MS (50)
#endif

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_CNT
#define CHARGER_PLUGINOUT_DEBOUNCE_CNT (3)
#endif

static APP_BATTERY_MV_T batterylevel[]={4030,3920,3820,3740,3670,3620,3570,3510,3440};//copy by pang

#define APP_BATTERY_CHARGING_PLUGOUT_DEDOUNCE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<500?3:1)

#define APP_BATTERY_CHARGING_EXTPIN_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<2*1000?2*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_EXTPIN_DEDOUNCE_CNT (6)

#define APP_BATTERY_CHARGING_OVERVOLT_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<2*1000?2*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_OVERVOLT_DEDOUNCE_CNT (3)

#define APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<20*1000?20*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT (6)


#define APP_BATTERY_REPORT_INTERVAL (5)

#define APP_BATTERY_MV_BASE ((APP_BATTERY_MAX_MV-APP_BATTERY_PD_MV)/(APP_BATTERY_LEVEL_NUM))

#define APP_BATTERY_STABLE_COUNT (5)
#define APP_BATTERY_MEASURE_PERIODIC_FAST_MS (200)
#ifdef BLE_ONLY_ENABLED
#define APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS (25000)
#else
#define APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS (10000)
#endif
#define APP_BATTERY_CHARGING_PERIODIC_MS (APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS)

#define APP_BATTERY_SET_MESSAGE(appevt, status, volt) (appevt = (((uint32_t)status&0xffff)<<16)|(volt&0xffff))
#define APP_BATTERY_GET_STATUS(appevt, status) (status = (appevt>>16)&0xffff)
#define APP_BATTERY_GET_VOLT(appevt, volt) (volt = appevt&0xffff)
#define APP_BATTERY_GET_PRAMS(appevt, prams) ((prams) = appevt&0xffff)

enum APP_BATTERY_MEASURE_PERIODIC_T
{
    APP_BATTERY_MEASURE_PERIODIC_FAST = 0,
    APP_BATTERY_MEASURE_PERIODIC_NORMAL,
    APP_BATTERY_MEASURE_PERIODIC_CHARGING,

    APP_BATTERY_MEASURE_PERIODIC_QTY,
};

struct APP_BATTERY_MEASURE_CHARGER_STATUS_T
{
    HAL_GPADC_MV_T prevolt;
    int32_t slope_1000[APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT];
    int slope_1000_index;
    int cnt;
};


typedef void (*APP_BATTERY_EVENT_CB_T)(enum APP_BATTERY_STATUS_T, APP_BATTERY_MV_T volt);

struct APP_BATTERY_MEASURE_T
{
    uint32_t start_time;
    enum APP_BATTERY_STATUS_T status;
#ifdef __INTERCONNECTION__
    uint8_t currentBatteryInfo;
    uint8_t lastBatteryInfo;
    uint8_t isMobileSupportSelfDefinedCommand;
#else
    uint8_t currlevel;
#endif
    APP_BATTERY_MV_T currvolt;
    APP_BATTERY_MV_T lowvolt;
    APP_BATTERY_MV_T highvolt;
    APP_BATTERY_MV_T pdvolt;
    uint32_t chargetimeout;
    enum APP_BATTERY_MEASURE_PERIODIC_T periodic;
    HAL_GPADC_MV_T voltage[APP_BATTERY_STABLE_COUNT];
    uint16_t index;
    struct APP_BATTERY_MEASURE_CHARGER_STATUS_T charger_status;
    APP_BATTERY_EVENT_CB_T cb;
    APP_BATTERY_CB_T user_cb;
};

#ifdef IS_BES_BATTERY_MANAGER_ENABLED

#ifdef CMT_008_NTC_DETECT
/* ntc 30k ok */
#define CHARGE_HIGH_TEMPERATURE          225     // 45C
#define CHARGE_LOW_TEMPERATURE           949    // 0C
#define CHARGE_HIGH_TEMPERATURE_RECOVER  256    // 41C
#define CHARGE_LOW_TEMPERATURE_RECOVER   840    // 4C

#define DISCHARGE_HIGH_TEMPERATURE       193    // 50C
#define DISCHARGE_LOW_TEMPERATURE        1266   // -10C

#define TEMPERATURE_ERROT_COUNT          5

static int8_t charge_temperature_error_num=0;
static int8_t charge_temperature_valid_num=0;
static bool charge_protection_flag=0;
static int8_t discharge_temperature_error_num=0;

static void app_create_ntc_timer_open(void);
static void app_stop_ntc_timer(void);
#endif /*CMT_008_NTC_DETECT*/

static enum APP_BATTERY_CHARGER_T app_battery_charger_forcegetstatus(void);

static void app_battery_pluginout_debounce_start(void);
static void app_battery_pluginout_debounce_handler(void const *param);
osTimerDef (APP_BATTERY_PLUGINOUT_DEBOUNCE, app_battery_pluginout_debounce_handler);
static osTimerId app_battery_pluginout_debounce_timer = NULL;
static uint32_t app_battery_pluginout_debounce_ctx = 0;
static uint32_t app_battery_pluginout_debounce_cnt = 0;

static enum HAL_GPADC_CHAN_T app_vbat_ch = APP_BATTERY_GPADC_CH_NUM;
#ifdef CHARGER_SPICIAL_CHAN
static uint8_t app_vbat_volt_div = GPADC_VBAT_VOLT_DIV;
#endif
static void app_battery_timer_handler(void const *param);
osTimerDef (APP_BATTERY, app_battery_timer_handler);
static osTimerId app_battery_timer = NULL;
static struct APP_BATTERY_MEASURE_T app_battery_measure;

static int app_battery_charger_handle_process(void);

#ifdef __INTERCONNECTION__
uint8_t* app_battery_get_mobile_support_self_defined_command_p(void)
{
    return &app_battery_measure.isMobileSupportSelfDefinedCommand;
}
#endif


void app_battery_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    uint8_t i;
    uint32_t meanBattVolt = 0;
    HAL_GPADC_MV_T vbat = volt;
    APP_BATTERY_TRACE(2,"%s %d",__func__, vbat);
    TRACE(2,"%s, volt:[%d],  volt<<2:[%d], level[%d]", __func__, vbat, vbat<<2, app_battery_current_level());
 #ifdef CHARGER_SPICIAL_CHAN
J    if ((vbat == HAL_GPADC_BAD_VALUE) || ((vbat * app_vbat_volt_div) <= APP_BATTERY_ERR_MV))
#else
    if ((vbat == HAL_GPADC_BAD_VALUE) || ((vbat<<2) <= APP_BATTERY_ERR_MV))
#endif
    {
        app_battery_measure.cb(APP_BATTERY_STATUS_INVALID, vbat);
        return;
    }

#if (defined(BTUSB_AUDIO_MODE) || defined(BTUSB_AUDIO_MODE))
    if(app_usbaudio_mode_on()) return ;
#endif
#ifdef CHARGER_SPICIAL_CHAN
    app_battery_measure.voltage[app_battery_measure.index++%APP_BATTERY_STABLE_COUNT] = vbat * app_vbat_volt_div;
#else
    app_battery_measure.voltage[app_battery_measure.index++%APP_BATTERY_STABLE_COUNT] = vbat<<2;
#endif 
    if (app_battery_measure.index > APP_BATTERY_STABLE_COUNT)
    {
        for (i=0; i<APP_BATTERY_STABLE_COUNT; i++)
        {
            meanBattVolt += app_battery_measure.voltage[i];
        }
        meanBattVolt /= APP_BATTERY_STABLE_COUNT;
        if (app_battery_measure.cb)
        {
            TRACE(3, "highvolt[%d], lowvolt[%d], pdvolt[%d],", app_battery_measure.highvolt, app_battery_measure.lowvolt, app_battery_measure.pdvolt);
            if (meanBattVolt>app_battery_measure.highvolt) //more than 4200mV.
            {
                TRACE(2, "%s   OVER   BattVolt[%d]", __func__, meanBattVolt);
                app_battery_measure.cb(APP_BATTERY_STATUS_OVERVOLT, meanBattVolt);
            }
            /* BattVolt > 3400mV && BattVolt < 3450mV, now is low battery state. */
            else if((meanBattVolt>app_battery_measure.pdvolt) && (meanBattVolt<app_battery_measure.lowvolt))
            {
                TRACE(2, "%s   UNDER   BattVolt[%d]", __func__, meanBattVolt);
                app_battery_measure.cb(APP_BATTERY_STATUS_UNDERVOLT, meanBattVolt);
            }
            else if(meanBattVolt<=app_battery_measure.pdvolt) //lenss than or equal to 3400mV.
            {
                TRACE(2, "%s   PD   BattVolt[%d]", __func__, meanBattVolt);
                app_battery_measure.cb(APP_BATTERY_STATUS_PDVOLT, meanBattVolt);
            }
            else
            {
                TRACE(2, "%s   NORMAL   BattVolt[%d]", __func__, meanBattVolt);
                app_battery_measure.cb(APP_BATTERY_STATUS_NORMAL, meanBattVolt);
            }
        }
    }
    else
    {
        int8_t level = 0;
#ifdef CHARGER_SPICIAL_CHAN
J         meanBattVolt = vbat * app_vbat_volt_div;
#else
         //meanBattVolt = vbat<<2;

        /* Get average meanBattVolt */
        for (i=0; i<app_battery_measure.index; i++)
        {
            meanBattVolt += app_battery_measure.voltage[i];
        }
        meanBattVolt /= app_battery_measure.index;

#endif
        //level = (meanBattVolt-APP_BATTERY_PD_MV)/APP_BATTERY_MV_BASE;

        for(i=0;i<9;i++)
        {
            if(meanBattVolt>=batterylevel[i])
                break;
        }
        level=10-i;

        if (level<APP_BATTERY_LEVEL_MIN)
            level = APP_BATTERY_LEVEL_MIN;
        if (level>APP_BATTERY_LEVEL_MAX)
            level = APP_BATTERY_LEVEL_MAX;

        app_battery_measure.currvolt = meanBattVolt;
#ifdef __INTERCONNECTION__
        APP_BATTERY_INFO_T* pBatteryInfo = (APP_BATTERY_INFO_T*)&app_battery_measure.currentBatteryInfo;
        pBatteryInfo->batteryLevel = level;
#else
        app_battery_measure.currlevel = level;
#endif
    }
}

static void app_battery_timer_start(enum APP_BATTERY_MEASURE_PERIODIC_T periodic)
{
    uint32_t periodic_millisec = 0;

    if (app_battery_measure.periodic != periodic){
        app_battery_measure.periodic = periodic;
        switch (periodic)
        {
            case APP_BATTERY_MEASURE_PERIODIC_FAST:
                periodic_millisec = APP_BATTERY_MEASURE_PERIODIC_FAST_MS;
                break;
            case APP_BATTERY_MEASURE_PERIODIC_CHARGING:
                periodic_millisec = APP_BATTERY_CHARGING_PERIODIC_MS;
                break;
            case APP_BATTERY_MEASURE_PERIODIC_NORMAL:
                periodic_millisec = APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS;
            default:
                break;
        }
        osTimerStop(app_battery_timer);
        osTimerStart(app_battery_timer, periodic_millisec);
    }
}

static void app_battery_timer_handler(void const *param)
{
#ifdef CHARGER_1802
    charger_vbat_div_adc_enable(true);
J    hal_gpadc_open(HAL_GPADC_CHAN_5, HAL_GPADC_ATP_ONESHOT, app_battery_irqhandler);
#else
    hal_gpadc_open(app_vbat_ch, HAL_GPADC_ATP_ONESHOT, app_battery_irqhandler);
#endif
}

static void app_battery_event_process(enum APP_BATTERY_STATUS_T status, APP_BATTERY_MV_T volt)
{
    uint32_t app_battevt;
    APP_MESSAGE_BLOCK msg;

    APP_BATTERY_TRACE(3,"%s %d,%d",__func__, status, volt);
    msg.mod_id = APP_MODUAL_BATTERY;
#if defined(USE_BASIC_THREADS)
    msg.mod_level = APP_MOD_LEVEL_2;
#endif
    APP_BATTERY_SET_MESSAGE(app_battevt, status, volt);
    msg.msg_body.message_id = app_battevt;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_mailbox_put(&msg);

}

#ifdef BT_BUILD_WITH_CUSTOMER_HOST
int app_status_battery_report(uint8_t level)
{
    return 0;
}
#else
int app_status_battery_report(uint8_t level)
{
#ifdef CMT_008_BLE_ENABLE
    user_custom_battery_level_notify(level+1);
#endif /*CMT_008_BLE_ENABLE*/

#if defined(__BTIF_EARPHONE__)
    app_10_second_timer_check();
#endif

    if (app_is_stack_ready())
    {
// #if (HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_BATTERY_REPORT) || (HF_SDK_FEATURES & HF_FEATURE_HF_INDICATORS)
#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
#if defined(IBRT)
        uint8_t hfp_device = app_bt_audio_get_curr_hfp_device();
        struct BT_DEVICE_T *curr_device = app_bt_get_device(hfp_device);
        if (curr_device->hf_conn_flag)
        {
            app_hfp_set_battery_level(level);
        }
#else
        app_hfp_set_battery_level(level);
#endif
#else
        TRACE(1,"[%s] Can not enable SUPPORT_BATTERY_REPORT", __func__);
#endif
        osapi_notify_evm();
    }
    return 0;
}
#endif

int app_battery_handle_process_normal(uint32_t status,  union APP_BATTERY_MSG_PRAMS prams)
{
    int8_t level = 0;
#ifdef CMT_008_BATTERY_LOW
    static uint8_t battery_low_play_time = 0;
    uint8_t i = 0;
#endif /*CMT_008_BATTERY_LOW*/

    switch (status)
    {
        case APP_BATTERY_STATUS_UNDERVOLT:
            TRACE(1,"UNDERVOLT:%d", prams.volt);
#ifdef CMT_008_BATTERY_LOW
            if(!battery_low_play_time && !app_battery_is_charging())
            {
                media_PlayAudio(AUD_ID_BT_BATTERY_LOW, 0);
            }

            battery_low_play_time ++;
            /* About 1min to play once 'battery low' prompts, since every 10sec to handle once of battery timer. */
            if(battery_low_play_time >= 7)
            {
                media_PlayAudio(AUD_ID_BT_BATTERY_LOW, 0);
                battery_low_play_time = 1;
            }
#else /*CMT_008_BATTERY_LOW*/
            app_status_indication_set(APP_STATUS_INDICATION_CHARGENEED);
#endif /*CMT_008_BATTERY_LOW*/

#ifdef MEDIA_PLAYER_SUPPORT
#if defined(IBRT)

#else
            media_PlayAudio(AUD_ID_BT_CHARGE_PLEASE, 0);
#endif
#endif
            // FALLTHROUGH
        case APP_BATTERY_STATUS_NORMAL:
        case APP_BATTERY_STATUS_OVERVOLT:
            app_battery_measure.currvolt = prams.volt;
            level = (prams.volt-APP_BATTERY_PD_MV)/APP_BATTERY_MV_BASE;

            if (level<APP_BATTERY_LEVEL_MIN)
                level = APP_BATTERY_LEVEL_MIN;
            if (level>APP_BATTERY_LEVEL_MAX)
                level = APP_BATTERY_LEVEL_MAX;
#ifdef __INTERCONNECTION__
            APP_BATTERY_INFO_T* pBatteryInfo;
            pBatteryInfo = (APP_BATTERY_INFO_T*)&app_battery_measure.currentBatteryInfo;
            pBatteryInfo->batteryLevel = level;
            if(level == APP_BATTERY_LEVEL_MAX)
            {
                level = 9;
            }
            else
            {
                level /= 10;
            }
#else
            //app_battery_measure.currlevel = level;

            /* Get average meanBattVolt */
            for(i=0;i<9;i++)
            {
                if(app_battery_measure.currvolt>=batterylevel[i])
                    break;
            }

            level=9-i;	
#endif
            app_status_battery_report(level);
            break;
        case APP_BATTERY_STATUS_PDVOLT:
#ifndef BT_USB_AUDIO_DUAL_MODE
            TRACE(1,"PDVOLT-->POWEROFF:%d", prams.volt);
            TRACE(0,"Low battery POWEROFF");
            media_PlayAudio(AUD_ID_BT_BATTERY_LOW, 0); //Add by Jay, play 'battery low' prompts.
            osDelay(2000);
            osTimerStop(app_battery_timer);
            app_shutdown();
#endif
            break;
        case APP_BATTERY_STATUS_CHARGING:
            TRACE(1,"CHARGING-->APP_BATTERY_CHARGER :%d", prams.charger);
            if (prams.charger == APP_BATTERY_CHARGER_PLUGIN)
            {
#ifdef BT_USB_AUDIO_DUAL_MODE
                TRACE(1,"%s:PLUGIN.", __func__);
                btusb_switch(BTUSB_MODE_USB);
#else
#if CHARGER_PLUGINOUT_RESET
                app_reset();
#else
                app_battery_measure.status = APP_BATTERY_STATUS_CHARGING;
#endif
#endif
            }
            break;
        case APP_BATTERY_STATUS_INVALID:
        default:
            break;
    }

    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_NORMAL);
    return 0;
}

int app_battery_handle_process_charging(uint32_t status,  union APP_BATTERY_MSG_PRAMS prams)
{
    switch (status)
    {
        case APP_BATTERY_STATUS_OVERVOLT:
        case APP_BATTERY_STATUS_NORMAL:
        case APP_BATTERY_STATUS_UNDERVOLT:
            app_battery_measure.currvolt = prams.volt;
            app_status_battery_report(prams.volt);
            break;
        case APP_BATTERY_STATUS_CHARGING:
            TRACE(1,"CHARGING:%d", prams.charger);
            if (prams.charger == APP_BATTERY_CHARGER_PLUGOUT)
            {
#ifndef BT_USB_AUDIO_DUAL_MODE
#if CHARGER_PLUGINOUT_RESET
                TRACE(0,"CHARGING-->RESET");
                osTimerStop(app_battery_timer);
                app_shutdown();
#else
                /* Disable by jay */
                //app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;

                /* Add by jay*/
                
                osTimerStop(app_battery_timer);
                //app_reset();
                //app_shutdown();
                /* Note: can not call 'app_shutdown()' and 'app_reset()',
                 * Otherwise have happen crash, here. I don't know why.
                 * Add by Jay.
                 */
                pmu_reboot();
                /* Add by jay, end. */
#endif
#endif
            }
            else if (prams.charger == APP_BATTERY_CHARGER_PLUGIN)
            {
#if defined(BT_USB_AUDIO_DUAL_MODE)
                TRACE(1,"%s:PLUGIN.", __func__);
                btusb_switch(BTUSB_MODE_USB);
#endif
            }
            break;
        case APP_BATTERY_STATUS_INVALID:
        default:
            break;
    }

    if (app_battery_charger_handle_process()<=0)
    {
        if (app_status_indication_get() != APP_STATUS_INDICATION_FULLCHARGE)
        {
            TRACE(1,"FULL_CHARGING:%d", app_battery_measure.currvolt);
            app_status_indication_set(APP_STATUS_INDICATION_FULLCHARGE);
#ifdef MEDIA_PLAYER_SUPPORT
#if defined(BT_USB_AUDIO_DUAL_MODE) || defined(IBRT)
#else
            media_PlayAudio(AUD_ID_BT_CHARGE_FINISH, 0);
#endif
#endif
            /* Add by Jay, disable charger pin, since now battery full. */
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin);
            }
            app_battery_stop();
#ifdef  CMT_008_NTC_DETECT
            app_stop_ntc_timer();
#endif /*CMT_008_NTC_DETECT*/
            /* Add by Jay end. */
        }
    }

    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_CHARGING);

    return 0;
}

extern "C"     bool pmu_ana_volt_is_high(void);
static int app_battery_handle_process(APP_MESSAGE_BODY *msg_body)
{
    uint8_t status;
    union APP_BATTERY_MSG_PRAMS msg_prams;

    APP_BATTERY_GET_STATUS(msg_body->message_id, status);
    APP_BATTERY_GET_PRAMS(msg_body->message_id, msg_prams.prams);

    uint32_t generatedSeed = hal_sys_timer_get();
    for (uint8_t index = 0; index < sizeof(bt_global_addr); index++)
    {
        generatedSeed ^= (((uint32_t)(bt_global_addr[index])) << (hal_sys_timer_get()&0xF));
    }
    srand(generatedSeed);

    if (status == APP_BATTERY_STATUS_PLUGINOUT){
        app_battery_pluginout_debounce_start();
    }
    else
    {
        switch (app_battery_measure.status)
        {
            case APP_BATTERY_STATUS_NORMAL:
                TRACE(1,"[ %s ]",__func__);
                app_battery_handle_process_normal((uint32_t)status, msg_prams);
#if defined(CHIP_BEST1501P)
                if(pmu_ana_volt_is_high() == false){
                    pmu_ntc_capture_start(NULL);
                }else{
                    TRACE(1,"%s stop ana check",__func__);
                }
#endif
                break;

            case APP_BATTERY_STATUS_CHARGING:
                app_battery_handle_process_charging((uint32_t)status, msg_prams);
                break;

            default:
                break;
        }
    }
    if (NULL != app_battery_measure.user_cb)
    {
        uint8_t batteryLevel;
#ifdef __INTERCONNECTION__
        APP_BATTERY_INFO_T* pBatteryInfo;
        pBatteryInfo = (APP_BATTERY_INFO_T*)&app_battery_measure.currentBatteryInfo;
        pBatteryInfo->chargingStatus = ((app_battery_measure.status == APP_BATTERY_STATUS_CHARGING)? 1:0);
        batteryLevel = pBatteryInfo->batteryLevel;

#else
        batteryLevel = app_battery_measure.currlevel;
#endif
        app_battery_measure.user_cb(app_battery_measure.currvolt,
                                    batteryLevel, app_battery_measure.status,status,msg_prams);
    }

    return 0;
}

int app_battery_register(APP_BATTERY_CB_T user_cb)
{
    if(NULL == app_battery_measure.user_cb)
    {
        app_battery_measure.user_cb = user_cb;
        return 0;
    }
    return 1;
}

int app_battery_get_info(APP_BATTERY_MV_T *currvolt, uint8_t *currlevel, enum APP_BATTERY_STATUS_T *status)
{
    if (currvolt)
    {
        *currvolt = app_battery_measure.currvolt;
    }

    if (currlevel)
    {
#ifdef __INTERCONNECTION__
        *currlevel = app_battery_measure.currentBatteryInfo;
#else
        *currlevel = app_battery_measure.currlevel;
#endif
    }

    if (status)
    {
        *status = app_battery_measure.status;
    }

    return 0;
}

#ifdef MORE_THAN_ONE_TYPE_OF_CHARGER
static void app_battery_gpadc_configuration_init(void)
{
    if (charger_package_type_get() == CHARGER_PACKAGE_TYPE_SIP_1620)
    {
        app_vbat_ch = HAL_GPADC_CHAN_BATTERY;
        app_vbat_volt_div = 4;
    }
}
#endif

int app_battery_open(void)
{
    APP_BATTERY_TRACE(3,"%s batt range:%d~%d",__func__, APP_BATTERY_MIN_MV, APP_BATTERY_MAX_MV);
    int nRet = APP_BATTERY_OPEN_MODE_INVALID;

#ifdef MORE_THAN_ONE_TYPE_OF_CHARGER
    app_battery_gpadc_configuration_init();
#endif

    if (app_battery_timer == NULL)
        app_battery_timer = osTimerCreate (osTimer(APP_BATTERY), osTimerPeriodic, NULL);

    if (app_battery_pluginout_debounce_timer == NULL)
        app_battery_pluginout_debounce_timer = osTimerCreate (osTimer(APP_BATTERY_PLUGINOUT_DEBOUNCE), osTimerOnce, &app_battery_pluginout_debounce_ctx);

    app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
#ifdef __INTERCONNECTION__
    app_battery_measure.currentBatteryInfo = APP_BATTERY_DEFAULT_INFO;
    app_battery_measure.lastBatteryInfo = APP_BATTERY_DEFAULT_INFO;
    app_battery_measure.isMobileSupportSelfDefinedCommand = 0;
#else
    app_battery_measure.currlevel = APP_BATTERY_LEVEL_MAX;
#endif
    app_battery_measure.currvolt = APP_BATTERY_MAX_MV;
    app_battery_measure.lowvolt = APP_BATTERY_MIN_MV;
    app_battery_measure.highvolt = APP_BATTERY_MAX_MV;
    app_battery_measure.pdvolt = APP_BATTERY_PD_MV;
    app_battery_measure.chargetimeout = APP_BATTERY_CHARGE_TIMEOUT_MIN;

    app_battery_measure.periodic = APP_BATTERY_MEASURE_PERIODIC_QTY;
    app_battery_measure.cb = app_battery_event_process;
    app_battery_measure.user_cb = NULL;

    app_battery_measure.charger_status.prevolt = 0;
    app_battery_measure.charger_status.slope_1000_index = 0;
    app_battery_measure.charger_status.cnt = 0;

    app_set_threadhandle(APP_MODUAL_BATTERY, app_battery_handle_process);

    /* Add by Jay */
    if(app_battery_full_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *) &app_battery_full_charger_detecter_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_full_charger_detecter_cfg.pin, HAL_GPIO_DIR_IN, 0);
    }
    /* Add by Jay, end. */

    if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_detecter_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_IN, 1);
    }

    if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        /* Modified by Jay */
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_enable_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 0);
    }

    if (app_battery_charger_indication_open() == APP_BATTERY_CHARGER_PLUGIN)
    {
        TRACE(2,"[%s]       status [ APP_BATTERY_CHARGER_PLUGIN ]", __func__);
        app_battery_measure.status = APP_BATTERY_STATUS_CHARGING;
        app_battery_measure.start_time = hal_sys_timer_get();
        //pmu_charger_plugin_config();
        if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
        {
            hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_OUT, 0);
        }

#if (CHARGER_PLUGINOUT_RESET == 0)
        nRet = APP_BATTERY_OPEN_MODE_CHARGING_PWRON;
        TRACE(2," [%s] , if [%d]",__func__,nRet);
#else
        nRet = APP_BATTERY_OPEN_MODE_CHARGING;
        TRACE(2," [%s] , else [%d]",__func__,nRet);
#endif
    }
    else
    {
        app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
        //pmu_charger_plugout_config();
        nRet = APP_BATTERY_OPEN_MODE_NORMAL;
        TRACE(2," [%s] , _else_ [%d]",__func__,nRet);
    }

#ifdef CMT_008_NTC_DETECT
    app_create_ntc_timer_open();
#endif /*CMT_008_NTC_DETECT*/

    return nRet;
}

int app_battery_start(void)
{
    APP_BATTERY_TRACE(2,"%s %d",__func__, APP_BATTERY_MEASURE_PERIODIC_FAST_MS);

    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_FAST);

    return 0;
}

int app_battery_stop(void)
{
    osTimerStop(app_battery_timer);

    return 0;
}

int app_battery_close(void)
{
    hal_gpadc_close(HAL_GPADC_CHAN_BATTERY);

    return 0;
}


static int32_t app_battery_charger_slope_calc(int32_t t1, int32_t v1, int32_t t2, int32_t v2)
{
    int32_t slope_1000;
    slope_1000 = (v2-v1)*1000/(t2-t1);
    return slope_1000;
}

static int app_battery_charger_handle_process(void)
{
    int nRet = 1;
    int8_t i=0,cnt=0;
    uint32_t slope_1000 = 0;
    uint32_t charging_min;
    static uint8_t overvolt_full_charge_cnt = 0;
    static uint8_t ext_pin_full_charge_cnt = 0;

    charging_min = hal_sys_timer_get() - app_battery_measure.start_time;
    charging_min = TICKS_TO_MS(charging_min)/1000/60;
    if (charging_min >= app_battery_measure.chargetimeout)
    {
        // TRACE(0,"TIMEROUT-->FULL_CHARGING");
        nRet = -1;
        goto exit;
    }

    if ((app_battery_measure.charger_status.cnt++%APP_BATTERY_CHARGING_OVERVOLT_MEASURE_CNT) == 0)
    {
        if (app_battery_measure.currvolt>=(app_battery_measure.highvolt+APP_BATTERY_CHARGE_OFFSET_MV))
        {
            overvolt_full_charge_cnt++;
        }
        else
        {
            overvolt_full_charge_cnt = 0;
        }
        if (overvolt_full_charge_cnt>=APP_BATTERY_CHARGING_OVERVOLT_DEDOUNCE_CNT)
        {
            TRACE(0,"OVERVOLT-->FULL_CHARGING");
            nRet = -1;
            goto exit;
        }
    }

    if ((app_battery_measure.charger_status.cnt++%APP_BATTERY_CHARGING_EXTPIN_MEASURE_CNT) == 0)
    {
        if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
        {
            if (!hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin)) //Modified by Jay
            {
                ext_pin_full_charge_cnt++;
            }
            else
            {
                ext_pin_full_charge_cnt = 0;
            }
            if (ext_pin_full_charge_cnt>=APP_BATTERY_CHARGING_EXTPIN_DEDOUNCE_CNT)
            {
                TRACE(0,"EXT PIN-->FULL_CHARGING");
                nRet = -1;
                goto exit;
            }
        }
    }

    if ((app_battery_measure.charger_status.cnt++%APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT) == 0)
    {
        if (!app_battery_measure.charger_status.prevolt)
        {
            app_battery_measure.charger_status.slope_1000[app_battery_measure.charger_status.slope_1000_index%APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT] = slope_1000;
            app_battery_measure.charger_status.prevolt = app_battery_measure.currvolt;
            for (i=0; i<APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT; i++)
            {
                app_battery_measure.charger_status.slope_1000[i]=100;
            }
        }
        else
        {
            slope_1000 = app_battery_charger_slope_calc(0, app_battery_measure.charger_status.prevolt,
                         APP_BATTERY_CHARGING_PERIODIC_MS*APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT/1000, app_battery_measure.currvolt);
            app_battery_measure.charger_status.slope_1000[app_battery_measure.charger_status.slope_1000_index%APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT] = slope_1000;
            app_battery_measure.charger_status.prevolt = app_battery_measure.currvolt;
            for (i=0; i<APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT; i++)
            {
                if (app_battery_measure.charger_status.slope_1000[i]>0)
                    cnt++;
                else
                    cnt--;
                TRACE(3,"slope_1000[%d]=%d cnt:%d", i,app_battery_measure.charger_status.slope_1000[i], cnt);
            }
            TRACE(3,"app_battery_charger_slope_proc slope*1000=%d cnt:%d nRet:%d", slope_1000, cnt, nRet);
            if (cnt>1)
            {
                nRet = 1;
            }/*else (3>=cnt && cnt>=-3){
                nRet = 0;
            }*/else
            {
                if (app_battery_measure.currvolt>=(app_battery_measure.highvolt-APP_BATTERY_CHARGE_OFFSET_MV))
                {
                    TRACE(0,"SLOPE-->FULL_CHARGING");
                    nRet = -1;
                }
            }
        }
        app_battery_measure.charger_status.slope_1000_index++;
    }
exit:
    return nRet;
}

static enum APP_BATTERY_CHARGER_T app_battery_charger_forcegetstatus(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;
    enum PMU_CHARGER_STATUS_T charger;

    charger = pmu_charger_get_status();

    if (charger == PMU_CHARGER_PLUGIN)
    {
        status = APP_BATTERY_CHARGER_PLUGIN;
        // TRACE(0,"force APP_BATTERY_CHARGER_PLUGIN");
    }
    else
    {
        status = APP_BATTERY_CHARGER_PLUGOUT;
        // TRACE(0,"force APP_BATTERY_CHARGER_PLUGOUT");
    }

    return status;
}

static void app_battery_charger_handler(enum PMU_CHARGER_STATUS_T status)
{
    TRACE(2,"%s: status=%d", __func__, status);
    pmu_charger_set_irq_handler(NULL);
    app_battery_event_process(APP_BATTERY_STATUS_PLUGINOUT,
                              (status == PMU_CHARGER_PLUGIN) ? APP_BATTERY_CHARGER_PLUGIN : APP_BATTERY_CHARGER_PLUGOUT);
}

static void app_battery_pluginout_debounce_start(void)
{
    TRACE(1,"%s", __func__);
#if defined(BT_USB_AUDIO_DUAL_MODE)
    btusb_switch(BTUSB_MODE_BT);
#endif
    app_battery_pluginout_debounce_ctx = (uint32_t)app_battery_charger_forcegetstatus();
    app_battery_pluginout_debounce_cnt = 1;
    osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
}

static void app_battery_pluginout_debounce_handler(void const *param)
{
    enum APP_BATTERY_CHARGER_T status_charger = app_battery_charger_forcegetstatus();

    if(app_battery_pluginout_debounce_ctx == (uint32_t) status_charger){
        app_battery_pluginout_debounce_cnt++;
    }
    else
    {
        TRACE(2,"%s dithering cnt %u", __func__, app_battery_pluginout_debounce_cnt);
        app_battery_pluginout_debounce_cnt = 0;
        app_battery_pluginout_debounce_ctx = (uint32_t)status_charger;
    }

    if (app_battery_pluginout_debounce_cnt >= CHARGER_PLUGINOUT_DEBOUNCE_CNT){
        TRACE(2,"%s %s", __func__, status_charger == APP_BATTERY_CHARGER_PLUGOUT ? "PLUGOUT" : "PLUGIN");
        if (status_charger == APP_BATTERY_CHARGER_PLUGIN)
        {
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                //hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_OUT, 0);  //Disable by Jay
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 1);      //Add by Jay
            }
            app_battery_measure.start_time = hal_sys_timer_get();
        }
        else
        {
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                //hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_OUT, 1);  //Disable by Jay
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 0);      //Add by Jay
            }
        }
        app_battery_event_process(APP_BATTERY_STATUS_CHARGING, status_charger);
        pmu_charger_set_irq_handler(app_battery_charger_handler);
        osTimerStop(app_battery_pluginout_debounce_timer);
    }else{
        osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
    }
}

/* Add by Jay */
#ifdef CMT_008_NTC_DETECT

#define APP_NTC_DETECT_TIMER_MS  1000  // timer time

static osTimerId app_ntc_timer = NULL; // timer ID 

/* Software timer hanlde func */
static void app_ntc_timer_handle(void const *param)
{
    // do anything
    ntc_capture_open();
}

/* Define software timer. */
osTimerDef (APP_NTC_SW_TIMER, app_ntc_timer_handle);

/* Create a software timer task */
static void app_create_ntc_timer_open(void)
{
    /* If it hasn't been created, it will be created. */
    if (app_ntc_timer == NULL)
    {
        /* Create a software Timer and return a Timer ID for later indexing. */
        app_ntc_timer = osTimerCreate(osTimer(APP_NTC_SW_TIMER), osTimerPeriodic, NULL);

        /* One-shot timer.*/
        //app_ntc_timer = osTimerCreate(osTimer(APP_NTC_SW_TIMER), osTimerOnce, NULL);
    }

    /* The following are the Stop and Start Timer tasks,
     * which take the parameters of the created Timer ID and time. */  
    osTimerStop(app_ntc_timer);
    osTimerStart(app_ntc_timer, APP_NTC_DETECT_TIMER_MS);

    //hal_gpio_pin_set((enum HAL_GPIO_PIN_T)Cfg_ntc_volt_ctr.pin);
}

static void app_stop_ntc_timer(void)
{
    osTimerStop(app_ntc_timer);
}

#endif /*CMT_008_NTC_DETECT*/

int app_battery_charger_indication_open(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;
    uint8_t cnt = 0;

    APP_BATTERY_TRACE(1,"[%s]__",__func__);

    pmu_charger_init();

    do
    {
        status = app_battery_charger_forcegetstatus();
        if (status == APP_BATTERY_CHARGER_PLUGIN)
            break;
        osDelay(20);
    }
    while(cnt++<5);

    if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        if (!hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin))
        {
            status = APP_BATTERY_CHARGER_PLUGIN;
        }
    }

    pmu_charger_set_irq_handler(app_battery_charger_handler);

    return status;
}

int8_t app_battery_current_level(void)
{
#ifdef __INTERCONNECTION__
    return app_battery_measure.currentBatteryInfo & 0x7f;
#else
    return app_battery_measure.currlevel;
#endif
}

int8_t app_battery_is_charging(void)
{
    return (APP_BATTERY_STATUS_CHARGING == app_battery_measure.status);
}
typedef uint16_t NTP_VOLTAGE_MV_T;
typedef uint16_t NTP_TEMPERATURE_C_T;

#define NTC_CAPTURE_STABLE_COUNT (5)
#define NTC_CAPTURE_TEMPERATURE_STEP (4)
#define NTC_CAPTURE_TEMPERATURE_REF (15)
#define NTC_CAPTURE_VOLTAGE_REF (1100)

typedef void (*NTC_CAPTURE_MEASURE_CB_T)(NTP_TEMPERATURE_C_T);

struct NTC_CAPTURE_MEASURE_T
{
    NTP_TEMPERATURE_C_T temperature;
    NTP_VOLTAGE_MV_T currvolt;
    NTP_VOLTAGE_MV_T voltage[NTC_CAPTURE_STABLE_COUNT];
    uint16_t index;
    NTC_CAPTURE_MEASURE_CB_T cb;
};

static struct NTC_CAPTURE_MEASURE_T ntc_capture_measure;

void ntc_capture_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
#if 0
    TRACE(1,"++++++++++++++++++++++++ enable_pin[%d], full_dete[%d], detecter_pin[%d]",\
    hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T) app_battery_ext_charger_enable_cfg.pin),\
    hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T) app_battery_full_charger_detecter_cfg.pin),\
    hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T) app_battery_ext_charger_detecter_cfg.pin));
#endif
    uint32_t meanVolt = 0;
    TRACE(3,"%s %d irq:0x%04x",__func__, volt, irq_val);

    if (volt == HAL_GPADC_BAD_VALUE)
    {
        return;
    }

    ntc_capture_measure.voltage[ntc_capture_measure.index++%NTC_CAPTURE_STABLE_COUNT] = volt;

    if (ntc_capture_measure.index > NTC_CAPTURE_STABLE_COUNT)
    {
        for (uint8_t i=0; i<NTC_CAPTURE_STABLE_COUNT; i++)
        {
            meanVolt += ntc_capture_measure.voltage[i];
        }
        meanVolt /= NTC_CAPTURE_STABLE_COUNT;
        ntc_capture_measure.currvolt = meanVolt;
    }
    else if (!ntc_capture_measure.currvolt)
    {
        ntc_capture_measure.currvolt = volt;
    }
    ntc_capture_measure.temperature = ((int32_t)ntc_capture_measure.currvolt - NTC_CAPTURE_VOLTAGE_REF)/NTC_CAPTURE_TEMPERATURE_STEP + NTC_CAPTURE_TEMPERATURE_REF;
    pmu_ntc_capture_disable();

#ifdef CMT_008_NTC_DETECT
    static bool status_indication = FALSE;

    if(app_battery_is_charging())
    {
        //TODO: Jay
        if(!status_indication)
        {
            status_indication = TRUE;
            app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
        }
        
        discharge_temperature_error_num=0;

        /* If current temperature more than 45 degrees celsius or less than 0 degrees celsius. */
        if((ntc_capture_measure.currvolt<CHARGE_HIGH_TEMPERATURE)||(ntc_capture_measure.currvolt>CHARGE_LOW_TEMPERATURE))
        {
            charge_temperature_error_num++;
            //charge_temperature_valid_num=0;
        }
        else
        {
            charge_temperature_error_num=0;
            //charge_temperature_valid_num++;
        }

        //charge over-temperature protection
        if(charge_temperature_error_num>TEMPERATURE_ERROT_COUNT)
        {
            charge_temperature_error_num=TEMPERATURE_ERROT_COUNT+1;
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                /* Disable charger pin */
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 0);
                charge_protection_flag=1;
                //app_pwm_clear();//disable pwm
                //app_status_indication_set(APP_STATUS_INDICATION_FULLCHARGE);
            }
        }

        //charge recover
        if(charge_protection_flag)
        {
            if((ntc_capture_measure.currvolt<CHARGE_HIGH_TEMPERATURE_RECOVER)||(ntc_capture_measure.currvolt>CHARGE_LOW_TEMPERATURE_RECOVER))
                charge_temperature_valid_num=0;
            else
                charge_temperature_valid_num++;
        }
        else
        {
            charge_temperature_valid_num=0;
        }

        if(charge_temperature_valid_num>30)
        {
            charge_temperature_valid_num=30+1;
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                //if(0==charge_full_flag)
                {
                    /* Enable charger pin */
                    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin, HAL_GPIO_DIR_OUT, 1);
                    charge_protection_flag=0;
                    app_status_indication_set(APP_STATUS_INDICATION_CHARGING);

                    //Todo: LED indication. jay
                    //apps_pwm_set(RED_PWM_LED, 1); //enable pwm
                }
            }
        }
    }
    else
    {
        if(status_indication)
        {
            status_indication = FALSE;
            app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);
        }
        
        //TODO: Jay
        charge_temperature_error_num=0;
        charge_temperature_valid_num=0;
        if((ntc_capture_measure.currvolt<DISCHARGE_HIGH_TEMPERATURE)||(ntc_capture_measure.currvolt>DISCHARGE_LOW_TEMPERATURE))
            discharge_temperature_error_num++;
        else
            discharge_temperature_error_num=0;

        if(discharge_temperature_error_num>TEMPERATURE_ERROT_COUNT)
        {
            discharge_temperature_error_num=TEMPERATURE_ERROT_COUNT+1;

            osTimerStop(app_battery_timer);
            osTimerStop(app_ntc_timer);
            TRACE(1, "[%s] Temp abnormal need shutdown.", __func__);
            media_PlayAudio(AUD_ID_POWER_OFF, 0);
            osDelay(500);
            app_shutdown();
        }
    }
#endif /*CMT_008_NTC_DETECT*/
}

int ntc_capture_open(void)
{

    ntc_capture_measure.currvolt = 0;
    ntc_capture_measure.index = 0;
    ntc_capture_measure.temperature = 0;
    ntc_capture_measure.cb = NULL;

    pmu_ntc_capture_enable();
#ifdef CMT_008_NTC_DETECT
    hal_gpadc_open(HAL_GPADC_CHAN_4, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);
#else /*CMT_008_NTC_DETECT*/
    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);
#endif /*CMT_008_NTC_DETECT*/
    return 0;
}

int ntc_capture_start(void)
{
    pmu_ntc_capture_enable();
#ifdef CMT_008_NTC_DETECT
    hal_gpadc_open(HAL_GPADC_CHAN_4, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);
#else /*CMT_008_NTC_DETECT*/
    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);
#endif /*CMT_008_NTC_DETECT*/
    return 0;
}
#else

#define IS_USE_SOC_PMU_PLUGINOUT

#ifdef IS_USE_SOC_PMU_PLUGINOUT

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_MS
#define CHARGER_PLUGINOUT_DEBOUNCE_MS (50)
#endif

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_CNT
#define CHARGER_PLUGINOUT_DEBOUNCE_CNT (3)
#endif

static void app_battery_pluginout_debounce_start(void);
static void app_battery_pluginout_debounce_handler(void const *param);
osTimerDef (APP_BATTERY_PLUGINOUT_DEBOUNCE, app_battery_pluginout_debounce_handler);
static osTimerId app_battery_pluginout_debounce_timer = NULL;
static uint32_t app_battery_pluginout_debounce_ctx = 0;
static uint32_t app_battery_pluginout_debounce_cnt = 0;

static int app_battery_handle_process(APP_MESSAGE_BODY *msg_body)
{
    uint8_t status;

    APP_BATTERY_GET_STATUS(msg_body->message_id, status);

    if (status == APP_BATTERY_STATUS_PLUGINOUT){
        app_battery_pluginout_debounce_start();
    }

    return 0;
}

static void app_battery_event_process(enum APP_BATTERY_STATUS_T status, APP_BATTERY_MV_T volt)
{
    uint32_t app_battevt;
    APP_MESSAGE_BLOCK msg;

    APP_BATTERY_TRACE(3,"%s %d,%d",__func__, status, volt);
    msg.mod_id = APP_MODUAL_BATTERY;
#if defined(USE_BASIC_THREADS)
    msg.mod_level = APP_MOD_LEVEL_2;
#endif
    APP_BATTERY_SET_MESSAGE(app_battevt, status, volt);
    msg.msg_body.message_id = app_battevt;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_mailbox_put(&msg);
}

static void app_battery_charger_handler(enum PMU_CHARGER_STATUS_T status)
{
    TRACE(2,"%s: status=%d", __func__, status);
    pmu_charger_set_irq_handler(NULL);
    app_battery_event_process(APP_BATTERY_STATUS_PLUGINOUT,
                              (status == PMU_CHARGER_PLUGIN) ? APP_BATTERY_CHARGER_PLUGIN : APP_BATTERY_CHARGER_PLUGOUT);
}

static enum APP_BATTERY_CHARGER_T app_battery_charger_forcegetstatus(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;
    enum PMU_CHARGER_STATUS_T charger;

    charger = pmu_charger_get_status();

    if (charger == PMU_CHARGER_PLUGIN)
    {
        status = APP_BATTERY_CHARGER_PLUGIN;
    }
    else
    {
        status = APP_BATTERY_CHARGER_PLUGOUT;
    }

    return status;
}

static void app_battery_pluginout_debounce_start(void)
{
    TRACE(1,"%s", __func__);

    app_battery_pluginout_debounce_ctx = (uint32_t)app_battery_charger_forcegetstatus();
    app_battery_pluginout_debounce_cnt = 1;
    osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
}

static void app_battery_pluginout_event_callback(enum APP_BATTERY_CHARGER_T event);

static void app_battery_pluginout_debounce_handler(void const *param)
{
    enum APP_BATTERY_CHARGER_T status_charger = app_battery_charger_forcegetstatus();

    if(app_battery_pluginout_debounce_ctx == (uint32_t) status_charger){
        app_battery_pluginout_debounce_cnt++;
    }
    else
    {
        TRACE(2,"%s dithering cnt %u", __func__, app_battery_pluginout_debounce_cnt);
        app_battery_pluginout_debounce_cnt = 0;
        app_battery_pluginout_debounce_ctx = (uint32_t)status_charger;
    }

    if (app_battery_pluginout_debounce_cnt >= CHARGER_PLUGINOUT_DEBOUNCE_CNT){
        TRACE(2,"%s %s", __func__, status_charger == APP_BATTERY_CHARGER_PLUGOUT ? "PLUGOUT" : "PLUGIN");

        app_battery_pluginout_event_callback(status_charger);
        pmu_charger_set_irq_handler(app_battery_charger_handler);
        osTimerStop(app_battery_pluginout_debounce_timer);
    }else{
        osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
    }
}

int app_battery_charger_indication_open(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;

    APP_BATTERY_TRACE(1,"[%s]",__func__);

    pmu_charger_init();

    pmu_charger_set_irq_handler(app_battery_charger_handler);

    return status;
}
#endif

int app_battery_open(void)
{
    app_battery_opened_callback();
#ifdef IS_USE_SOC_PMU_PLUGINOUT
    if (app_battery_pluginout_debounce_timer == NULL)
    {
        app_battery_pluginout_debounce_timer =
            osTimerCreate (osTimer(APP_BATTERY_PLUGINOUT_DEBOUNCE),
            osTimerOnce, &app_battery_pluginout_debounce_ctx);
    }

    app_set_threadhandle(APP_MODUAL_BATTERY, app_battery_handle_process);

    app_battery_charger_indication_open();
#endif

    // initialize the custom battery manager here
    // returned value could be:
    // #define APP_BATTERY_OPEN_MODE_INVALID        (-1)
    // #define APP_BATTERY_OPEN_MODE_NORMAL         (0)
    // #define APP_BATTERY_OPEN_MODE_CHARGING       (1)
    // #define APP_BATTERY_OPEN_MODE_CHARGING_PWRON (2)
    return APP_BATTERY_OPEN_MODE_NORMAL;
}

int app_battery_start(void)
{
    // start battery measurement timer here
    return 0;
}

int app_battery_stop(void)
{
    // stop battery measurement timer here
    return 0;
}

int app_battery_get_info(APP_BATTERY_MV_T *currvolt, uint8_t *currlevel, enum APP_BATTERY_STATUS_T *status)
{
    // should just return battery level via currlevel for hfp battery level indication
    *currlevel = APP_BATTERY_LEVEL_MAX;
    return 0;
}

#ifdef IS_USE_SOC_PMU_PLUGINOUT
static void app_battery_pluginout_event_callback(enum APP_BATTERY_CHARGER_T event)
{
    if (APP_BATTERY_CHARGER_PLUGOUT == event)
    {
        TRACE(0, "Charger plug out.");
    }
    else if (APP_BATTERY_CHARGER_PLUGIN == event)
    {
        TRACE(0, "Charger plug in.");
    }
}
#endif

int app_battery_register(APP_BATTERY_CB_T user_cb)
{
    // register the battery level update event callback
    return 0;
}

#endif

WEAK void app_battery_opened_callback(void)
{

}

#else
int app_battery_open(void)
{
    return 0;
}

int app_battery_start(void)
{
    return 0;
}
#endif
