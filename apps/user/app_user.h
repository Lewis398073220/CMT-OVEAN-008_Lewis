/*!
\copyright  Copyright (c) 2023 Add by Jay, copy from pang .\n
\file       app_user.h
\brief    
*/

#ifndef __USER_APPS_H__
#define __USER_APPS_H__

#if defined(__PWM_LED_CTL__)
#include "hal_cmu.h"
#include "hal_pwm.h"

#define RED_PWM_LED 	HAL_PWM_ID_1
#define GREEN_PWM_LED 	HAL_PWM_ID_3

#define RED_LED 	0
#define BLUE_LED 	1
#define GREEN_LED 	2

#define PWM_IDLE 0
#define PWM_EN 1
#define BlINK_1x 2
#define BLINK_1S 3
#define BLINK_5S 4
#define BlINK_2x 5

#define LED_ON  2
#define LED_OFF_1S 5
#define LED_OFF_3S 15
#define LED_OFF_5S 25
#endif

#if defined(__USE_MOTOR_CTL__)
#define MOTOR_POWER_ON  500
#define MOTOR_POWER_OFF 500
#define MOTOR_PAIRING   200
#define MOTOR_CALLING   200
#define MOTOR_ENDCALL   500
#define MOTOR_MUTE      200
#define MOTOR_UNMUTE    200
#endif


#if defined(__EVRCORD_USER_DEFINE__)
#include "nvrecord_env.h"

#define DEMO_MODE 0xEE

#define SLEEP_TIME_3MIN  24
#define SLEEP_TIME_5MIN  39//39
#define SLEEP_TIME_10MIN 77
#define SLEEP_TIME_PERM  255

#define DEFAULT_SLEEP_TIME SLEEP_TIME_5MIN

extern IIR_CFG_T eq_custom_para;
extern IIR_CFG_T eq_custom_para_ancoff;
#if defined(AUDIO_LINEIN)
extern IIR_CFG_T eq_custom_para_linein;
#endif

#endif

#if defined(__USE_3_5JACK_CTR__)
extern bool reconncect_null_by_user;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool notify_enable_idx_cfg;
    bool notify_enable_idx1_cfg;

    uint8_t user_set_bt_name_len;
    char user_set_bt_name[27]; /* 27 = CLASSIC_BTNAME_LEN */
    bool touch_lock;
    uint8_t suond_prompt_level;
    uint16_t standby_time;
    uint16_t standby_time_count;
    bool current_gaming_mode_state;
    bool sidetone_state;
    uint8_t channel_balance_value;
} app_user_custom_data_t;

float user_custom_return_balance_value_db(uint8_t index);
uint8_t user_custom_get_channel_balance_value(void);
void user_custom_set_channel_balance(uint8_t balance_value);
bool user_custom_get_sidetone_status(void);
void user_custom_set_sidetone(bool en);
bool user_custom_gaming_mode_get(void);
void user_custom_gaming_mode_set(bool enable);
void user_custom_factory_reset(void);

void user_custom_default_reset(void);
void user_custom_reset_standby_time(void);
void user_custom_set_standby_time(uint16_t time);
uint16_t user_custom_get_standby_time(void);
bool user_custom_get_active_standby_time(void);
void user_custom_set_shutdown_time(uint16_t time);
uint16_t user_custom_get_remaining_shutdown_time(void);
bool user_custom_get_notify_enable_idx(void);
bool user_custom_get_notify_enable_idx1(void);
void user_custom_battery_level_notify(uint8_t level);
void user_custom_get_notify_flag(bool flag, uint8_t notify_enable);
void user_custom_set_sound_prompt(uint8_t level);
bool user_custom_get_touch_clock(void);
void user_custom_set_touch_clock(bool lock);
const char *user_custom_get_bt_name(void);
bool user_custom_get_bt_name_len(void);
void user_custom_nvrecord_data_get(void);
void user_custom_nvrecord_set_bt_name(char* data, uint8_t len);

int app_user_event_open_module(void);
void app_user_event_close_module(void);

#if defined(__USE_3_5JACK_CTR__)
bool apps_3p5_jack_get_val(void);
bool  apps_3p5jack_plugin_check(void);	
bool app_apps_3p5jack_plugin_flag(bool clearcount);
void app_jack_start_timer(void);
void app_jack_stop_timer(void);
void apps_jack_event_process(void);
#endif

#if defined(__USE_IR_CTL__)
void app_ir_start_timer(void);
void app_ir_stop_timer(void);
void apps_ir_event_process(void);
#endif

#if defined(__PWM_LED_CTL__)
void app_pwm_disable(void);
void app_pwm_start_timer(void);
void apps_pwm_event_process(void);
void app_pwm_clear(void);
void apps_pwm_set(enum HAL_PWM_ID_T id, uint8_t enable);
void apps_blink_nx_set(uint8_t id, uint8_t enable, uint8_t period_on, uint8_t period_off);
void apps_blink_1x_set(uint8_t id,uint8_t period_on);
bool app_pwm_idle(void);
#endif

#if defined(__USE_MOTOR_CTL__)
void app_motor_init_timer(void);
void app_motor_timer_set(uint8_t vibrate_num, uint32_t vibrate_ms,uint16_t delay_ms);
void app_motor_timer_close(void);
#endif

#if defined(__USE_AMP_MUTE_CTR__)
void app_amp_open_start_timer(void);
void app_amp_open_stop_timer(void);
void apps_amp_switch_event_process(void);
#endif

#if defined(__AUDIO_FADEIN__)
void app_audio_fadein_start(uint8_t audio_type);
void apps_audio_fadein_event_process(void);
#endif

#if defined(__EVRCORD_USER_DEFINE__)
uint8_t app_eq_index_get(void);
void app_nvrecord_eq_set(uint8_t eq_index);
//void app_eq_para_get(uint8_t *p);
void app_eq_custom_para_get(uint8_t customization_eq_value[6]);
void app_nvrecord_eq_param_set(uint8_t customization_eq_value[6]);
uint8_t app_nvrecord_anc_get(void);
uint8_t app_nvrecord_anc_table_get(void);
void app_nvrecord_anc_set(uint8_t nc);
void app_nvrecord_demo_mode_set(uint8_t mod);
uint8_t app_nvrecord_demo_mode_get(void);
uint8_t app_get_sleep_time(void);
uint8_t get_sleep_time(void);
void app_nvrecord_sleep_time_set(uint8_t sltime);
uint8_t app_get_vibrate_mode(void);
void app_nvrecord_vibrate_mode_set(uint8_t mod);
uint8_t app_get_monitor_level(void);
uint8_t app_get_focus(void);
void app_nvrecord_monitor_level_set(uint8_t level);
void app_nvrecord_focus_set(uint8_t focus);
uint8_t app_get_focus(void);
void app_nvrecord_sensor_set(uint8_t on);
uint8_t app_get_touchlock(void);
void app_nvrecord_touchlock_set(uint8_t on);
uint8_t app_get_auto_poweroff(void);
void app_auto_poweroff_set(uint8_t pftime);
uint8_t app_get_sidetone(void);
void app_nvrecord_sidetone_set(uint8_t on);
uint8_t app_get_fota_flag(void);
void app_nvrecord_fotaflag_set(uint8_t on);
uint8_t app_get_new_multipoint_flag(void);
uint8_t app_get_multipoint_flag(void);
void app_nvrecord_multipoint_set(uint8_t on);
void app_nvrecord_para_get(void);
uint8_t app_get_TalkMicLed_flag(void);
void app_nvrecord_TalkMicLed_set(uint8_t on);

#endif

#if defined(TPV_DEVICE_API)
uint8_t app_get_device_color(void);
#endif


#ifdef __cplusplus
	}//extern "C" {
#endif

#endif
