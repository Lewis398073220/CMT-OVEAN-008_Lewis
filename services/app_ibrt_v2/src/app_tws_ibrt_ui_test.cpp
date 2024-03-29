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
#if defined(IBRT_CORE_V2_ENABLE)
#include <stdlib.h>
#include <string.h>
#include "hal_trace.h"
#include "apps.h"
#include "app_key.h"
#include "app_anc.h"
#include "bluetooth_bt_api.h"
#include "app_bt_cmd.h"
#include "btapp.h"
#include "factory_section.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_ble.h"
#include "ddbif.h"
#include "app_tws_ibrt_conn_api.h"
#include "app_tws_ibrt_ui_test.h"
#include "app_custom_api.h"
#include "app_tws_ibrt.h"
#include "app_ibrt_keyboard.h"
#include "app_ibrt_if.h"
#include "app_bt.h"
#include "btapp.h"
#include "app_ibrt_nvrecord.h"
#include "app_ibrt_customif_cmd.h"
#include "app_audio_active_device_manager.h"
#include "app_audio_control.h"
#include "app_audio_bt_device.h"
#include "app_ui_api.h"
#include "app_bt_media_manager.h"
#include "app_ibrt_debug.h"
#include "app_user.h" //Add by Jay

#ifdef CUSTOMER_APP_BOAT
#include "app_tota_general.h"
#endif
#if defined(BISTO_ENABLED)
#include "gsound_custom_actions.h"
#endif

#include "app_media_player.h"
#include "a2dp_decoder.h"

#ifdef RECORDING_USE_SCALABLE
#include "voice_compression.h"
#endif

#ifdef APP_BT_SPEAKER
#include "btspeaker_main.h"
#endif

#ifdef __AI_VOICE__
#include "app_ai_if.h"
#endif
//#include "app_ui_queues.h"

#if defined(A2DP_SBC_PLC_ENABLED)
#include "hal_location.h"
#endif

#if BLE_AUDIO_ENABLED
#include "app_audio_focus_stack.h"
#include "app_audio_active_device_manager.h"
#include "app_ble_mode_switch.h"
#include "ble_audio_earphone_info.h"
#include "app.h"
#include "app_audio_control.h"
#endif

#include "app_audio_control.h"

#ifdef AUDIO_MANAGER_TEST_ENABLE
#include "app_audio_active_device_manager_ut.h"
#include "app_audio_focus_control_test.h"
#include "app_audio_focus_stack_ut.h"
#endif
#include "app_ai_manager_api.h"

typedef struct
{
    bt_bdaddr_t master_bdaddr;
    bt_bdaddr_t slave_bdaddr;
} ibrt_pairing_info_t;

static const ibrt_pairing_info_t g_ibrt_pairing_info[] =
{
    {{0x51, 0x33, 0x33, 0x22, 0x11, 0x11},{0x50, 0x33, 0x33, 0x22, 0x11, 0x11}},
    {{0x53, 0x33, 0x33, 0x22, 0x11, 0x11},{0x52, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*LJH*/
    {{0x61, 0x33, 0x33, 0x22, 0x11, 0x11},{0x60, 0x33, 0x33, 0x22, 0x11, 0x11}},
    {{0x71, 0x33, 0x33, 0x22, 0x11, 0x11},{0x70, 0x33, 0x33, 0x22, 0x11, 0x11}},
    {{0x81, 0x33, 0x33, 0x22, 0x11, 0x11},{0x80, 0x33, 0x33, 0x22, 0x11, 0x11}},
    {{0x91, 0x33, 0x33, 0x22, 0x11, 0x11},{0x90, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*Customer use*/
    {{0x05, 0x33, 0x33, 0x22, 0x11, 0x11},{0x04, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*Rui*/
    {{0x07, 0x33, 0x33, 0x22, 0x11, 0x11},{0x06, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*zsl*/
    {{0x88, 0xaa, 0x33, 0x22, 0x11, 0x11},{0x87, 0xaa, 0x33, 0x22, 0x11, 0x11}}, /*Lufang*/
    {{0x77, 0x22, 0x66, 0x22, 0x11, 0x11},{0x77, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*xiao*/
    {{0xAA, 0x22, 0x66, 0x22, 0x11, 0x11},{0xBB, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*LUOBIN*/
    {{0x08, 0x33, 0x66, 0x22, 0x11, 0x11},{0x07, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*Yangbin1*/
    {{0x0B, 0x33, 0x66, 0x22, 0x11, 0x11},{0x0A, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*Yangbin2*/
    {{0x35, 0x33, 0x66, 0x22, 0x11, 0x11},{0x34, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*Lulu*/
    {{0xF8, 0x33, 0x66, 0x22, 0x11, 0x11},{0xF7, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*jtx*/
    {{0xd3, 0x53, 0x86, 0x42, 0x71, 0x31},{0xd2, 0x53, 0x86, 0x42, 0x71, 0x31}}, /*shhx*/
    {{0xd5, 0x53, 0x86, 0x42, 0x71, 0x31},{0xd4, 0x53, 0x86, 0x42, 0x71, 0x31}}, /*shhx*/
    {{0xcc, 0xaa, 0x99, 0x88, 0x77, 0x66},{0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66}}, /*mql*/
    {{0x95, 0x33, 0x69, 0x22, 0x11, 0x11},{0x94, 0x33, 0x69, 0x22, 0x11, 0x11}}, /*wyl*/
    {{0x82, 0x35, 0x68, 0x24, 0x19, 0x17},{0x81, 0x35, 0x68, 0x24, 0x19, 0x17}}, /*rhy*/
    {{0x66, 0x66, 0x88, 0x66, 0x66, 0x88},{0x65, 0x66, 0x88, 0x66, 0x66, 0x88}}, /*xdl*/
    {{0x61, 0x66, 0x66, 0x66, 0x66, 0x81},{0x16, 0x66, 0x66, 0x66, 0x66, 0x18}}, /*test1*/
    {{0x62, 0x66, 0x66, 0x66, 0x66, 0x82},{0x26, 0x66, 0x66, 0x66, 0x66, 0x28}}, /*test2*/
    {{0x63, 0x66, 0x66, 0x66, 0x66, 0x83},{0x36, 0x66, 0x66, 0x66, 0x66, 0x38}}, /*test3*/
    {{0x64, 0x66, 0x66, 0x66, 0x66, 0x84},{0x46, 0x66, 0x66, 0x66, 0x66, 0x48}}, /*test4*/
    {{0x65, 0x66, 0x66, 0x66, 0x66, 0x85},{0x56, 0x66, 0x66, 0x66, 0x66, 0x58}}, /*test5*/
    {{0xaa, 0x66, 0x66, 0x66, 0x66, 0x86},{0xaa, 0x66, 0x66, 0x66, 0x66, 0x68}}, /*test6*/
    {{0x67, 0x66, 0x66, 0x66, 0x66, 0x87},{0x76, 0x66, 0x66, 0x66, 0x66, 0x78}}, /*test7*/
    {{0x68, 0x66, 0x66, 0x66, 0x66, 0xa8},{0x86, 0x66, 0x66, 0x66, 0x66, 0x8a}}, /*test8*/
    {{0x69, 0x66, 0x66, 0x66, 0x66, 0x89},{0x86, 0x66, 0x66, 0x66, 0x66, 0x18}}, /*test9*/
    {{0x67, 0x66, 0x66, 0x22, 0x11, 0x11},{0x66, 0x66, 0x66, 0x22, 0x11, 0x11}}, /*anonymous*/
    {{0x93, 0x33, 0x33, 0x33, 0x33, 0x33},{0x92, 0x33, 0x33, 0x33, 0x33, 0x33}}, /*gxl*/
    {{0x07, 0x13, 0x66, 0x22, 0x11, 0x11},{0x06, 0x13, 0x66, 0x22, 0x11, 0x11}}, /*yangguo*/
    {{0x02, 0x15, 0x66, 0x22, 0x11, 0x11},{0x01, 0x15, 0x66, 0x22, 0x11, 0x11}}, /*mql fpga*/
    {{0x31, 0x21, 0x68, 0x93, 0x52, 0x70},{0x30, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial0*/
    {{0x33, 0x21, 0x68, 0x93, 0x52, 0x70},{0x32, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial1*/
    {{0x35, 0x21, 0x68, 0x93, 0x52, 0x70},{0x34, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial2*/
    {{0x37, 0x21, 0x68, 0x93, 0x52, 0x70},{0x36, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial3*/
    {{0x39, 0x21, 0x68, 0x93, 0x52, 0x70},{0x38, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial4*/
    {{0x41, 0x21, 0x68, 0x93, 0x52, 0x70},{0x40, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial5*/
    {{0x43, 0x21, 0x68, 0x93, 0x52, 0x70},{0x42, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial6*/
    {{0x45, 0x21, 0x68, 0x93, 0x52, 0x70},{0x44, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial7*/
    {{0x47, 0x21, 0x68, 0x93, 0x52, 0x70},{0x46, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial8*/
    {{0x49, 0x21, 0x68, 0x93, 0x52, 0x70},{0x48, 0x21, 0x68, 0x93, 0x52, 0x70}}, /*xinyin serial9*/
    {{0x32, 0x15, 0x66, 0x22, 0x11, 0x11},{0x31, 0x15, 0x66, 0x22, 0x11, 0x11}}, /*dengcong*/
    {{0x33, 0x33, 0x33, 0x22, 0x11, 0x11},{0x32, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*zhanghao*/
    {{0x77, 0x77, 0x33, 0x22, 0x11, 0x11},{0x76, 0x77, 0x33, 0x22, 0x11, 0x11}}, /* jiaqizhu */
    {{0x38, 0x77, 0x33, 0x22, 0x11, 0x11},{0x39, 0x77, 0x33, 0x22, 0x11, 0x11}}, /* zhanghaoyu */
    {{0x10, 0x77, 0x33, 0x22, 0x11, 0x11},{0x11, 0x77, 0x33, 0x22, 0x11, 0x11}}, /* zhangyanyang */
    {{0x99, 0x77, 0x33, 0x22, 0x11, 0x11},{0x98, 0x77, 0x33, 0x22, 0x11, 0x11}}, /*zhaochunyu*/

    {{0x81, 0x33, 0x33, 0x23, 0x22, 0x11},{0x80, 0x33, 0x33, 0x23, 0x22, 0x11}}, /*dual mode,test 2*/
    {{0x91, 0x44, 0x33, 0x23, 0x22, 0x11},{0x90, 0x44, 0x33, 0x23, 0x22, 0x11}}, /*dual mode,test 4*/
    {{0x51, 0x33, 0x33, 0x23, 0x22, 0x11},{0x50, 0x33, 0x33, 0x23, 0x22, 0x11}}, /* ziqiangli */
    {{0x6b, 0x33, 0x33, 0x23, 0x22, 0x11},{0x6a, 0x33, 0x33, 0x23, 0x22, 0x11}},
    {{0x91, 0x33, 0x33, 0x23, 0x22, 0x11},{0x90, 0x33, 0x33, 0x23, 0x22, 0x11}}, /* liujunhong*/
    {{0x91, 0x33, 0x33, 0x23, 0x22, 0x11},{0x90, 0x33, 0x33, 0x23, 0x22, 0x11}}, /*dual mode,test 4*/

    {{0x25, 0x66, 0x66, 0x66, 0x66, 0x27},{0x52, 0x66, 0x66, 0x66, 0x66, 0x72}}, //customer01
    {{0x39, 0x33, 0x33, 0x23, 0x22, 0x11},{0x38, 0x33, 0x33, 0x23, 0x22, 0x11}}, //customer02
    {{0x44, 0x33, 0x33, 0x23, 0x22, 0x11},{0x43, 0x33, 0x33, 0x23, 0x22, 0x11}}, //customer03
    {{0x87, 0x33, 0x33, 0x23, 0x22, 0x11},{0x86, 0x33, 0x33, 0x23, 0x22, 0x11}}, //customer04
    {{0x90, 0x33, 0x33, 0x23, 0x22, 0x11},{0x89, 0x33, 0x33, 0x23, 0x22, 0x11}}, //customer05
    {{0x39, 0x31, 0x31, 0x23, 0x22, 0x11},{0x37, 0x31, 0x31, 0x23, 0x22, 0x11}}, //customer06
    {{0x44, 0x31, 0x31, 0x23, 0x22, 0x11},{0x42, 0x31, 0x31, 0x23, 0x22, 0x11}}, //customer07
    {{0x48, 0x31, 0x31, 0x23, 0x22, 0x11},{0x47, 0x31, 0x31, 0x23, 0x22, 0x11}}, //customer08
    {{0x52, 0x31, 0x31, 0x23, 0x22, 0x11},{0x51, 0x31, 0x31, 0x23, 0x22, 0x11}}, //customer09
    {{0x56, 0x31, 0x31, 0x23, 0x22, 0x11},{0x55, 0x31, 0x31, 0x23, 0x22, 0x11}}, //customer10

    {{0x60, 0x31, 0x31, 0x23, 0x22, 0x11},{0x59, 0x31, 0x31, 0x23, 0x22, 0x11}}, //BES_01
    {{0x64, 0x31, 0x31, 0x23, 0x22, 0x11},{0x63, 0x31, 0x31, 0x23, 0x22, 0x11}}, //BES_02
    {{0x68, 0x31, 0x31, 0x23, 0x22, 0x11},{0x67, 0x31, 0x31, 0x23, 0x22, 0x11}}, //BES_03
    {{0x72, 0x31, 0x31, 0x23, 0x22, 0x11},{0x71, 0x31, 0x31, 0x23, 0x22, 0x11}}, //BES_04
    {{0x76, 0x31, 0x31, 0x23, 0x22, 0x11},{0x75, 0x31, 0x31, 0x23, 0x22, 0x11}}, //BES_05
    {{0x80, 0x31, 0x31, 0x23, 0x22, 0x11},{0x79, 0x31, 0x31, 0x23, 0x22, 0x11}}, //BES_06
    {{0x22, 0x66, 0x66, 0x66, 0x66, 0x24},{0x22, 0x66, 0x66, 0x66, 0x66, 0x42}}, //BES_07
    {{0x24, 0x66, 0x66, 0x66, 0x66, 0x26},{0x42, 0x66, 0x66, 0x66, 0x66, 0x62}}, //BES_08
    {{0x28, 0x66, 0x66, 0x66, 0x66, 0x30},{0x82, 0x66, 0x66, 0x66, 0x66, 0x03}}, //BES_09
    {{0x30, 0x66, 0x66, 0x66, 0x66, 0x32},{0x03, 0x66, 0x66, 0x66, 0x66, 0x23}}, //BES_10
    {{0x38, 0x66, 0x66, 0x66, 0x66, 0x40},{0x83, 0x66, 0x66, 0x66, 0x66, 0x04}}, //BES_11

    {{0xbb, 0x9a, 0x78, 0x56, 0x34, 0x12},{0xaa, 0x9a, 0x78, 0x56, 0x34, 0x12}}, //hejunxiang

    {{0x55, 0xda, 0x61, 0xe9, 0xc6, 0x5c},{0x56, 0xda, 0x61, 0xe9, 0xc6, 0x5c}}, /*test tile*/
    {{0x99, 0x33, 0x33, 0x23, 0x22, 0x11},{0x98, 0x33, 0x33, 0x23, 0x22, 0x11}},    //used for dolby

};

#if !defined(FREE_TWS_PAIRING_ENABLED)
static void app_ibrt_raw_ui_test_load_from_bt_pair_list(void)
{
    const ibrt_pairing_info_t *ibrt_pairing_info_lst = g_ibrt_pairing_info;
    uint32_t lst_size = ARRAY_SIZE(g_ibrt_pairing_info);
    struct nvrecord_env_t *nvrecord_env;
    uint8_t localAddr[BD_ADDR_LEN];

    nv_record_env_get(&nvrecord_env);
    factory_section_original_btaddr_get(localAddr);
    LOG_I("   jay [ %s ] ", __func__);

    bool isRightMasterSidePolicy = true;
#ifdef IBRT_RIGHT_MASTER
    isRightMasterSidePolicy = true;
#else
    isRightMasterSidePolicy = false;
#endif

    for(uint32_t i =0; i<lst_size; i++)
    {
        if (!memcmp(ibrt_pairing_info_lst[i].master_bdaddr.address, localAddr, BD_ADDR_LEN))
        {
            app_tws_ibrt_reconfig_role(IBRT_MASTER, ibrt_pairing_info_lst[i].master_bdaddr.address,
                ibrt_pairing_info_lst[i].slave_bdaddr.address, isRightMasterSidePolicy);
            return;
        }
        else if (!memcmp(ibrt_pairing_info_lst[i].slave_bdaddr.address, localAddr, BD_ADDR_LEN))
        {
            app_tws_ibrt_reconfig_role(IBRT_SLAVE, ibrt_pairing_info_lst[i].master_bdaddr.address,
                            ibrt_pairing_info_lst[i].slave_bdaddr.address, isRightMasterSidePolicy);
            return;
        }
    }
}
#endif

#if defined(A2DP_SBC_PLC_ENABLED)
static uint32_t ch_select;
SRAM_TEXT_LOC int get_ibrt_ch(void){
    return ch_select;
}
#endif

POSSIBLY_UNUSED static int parse_bt_addr(unsigned char *addr_buf, bt_bdaddr_t *bt_addr, unsigned int lenth)
{
    if(lenth < 12)
    {
        LOG_I("invalid recv bt addr");
        return 0;
    }
    char bt_addr_buff[2];
    uint8_t btLen = 0;
    uint8_t bufferStart = 10;
    while(btLen < 6)
    {
        memset(&bt_addr_buff, 0, sizeof(bt_addr_buff));
        memcpy(bt_addr_buff, addr_buf + bufferStart, 2);
        bt_addr->address[btLen] = (unsigned char)strtol((char*)bt_addr_buff, NULL, 16);
        bufferStart -= 2;
        btLen += 1;
    }
    return 1;
}

WEAK void app_ibrt_initialize_nv_role_callback(void *config, void * record_env)
{
    /** customer can replace this with custom nv role configuration */
    /** by default bit 0 of the first byte decides the nv role:
        1: master and right bud
        0: slave and left bud
    */
    ibrt_config_t *ibrt_config = (ibrt_config_t *)config;
    struct nvrecord_env_t *nvrecord_env = (struct nvrecord_env_t *)record_env;

    if (ibrt_config->local_addr.address[0]&1)
    {
        ibrt_config->nv_role = IBRT_MASTER;
    }
    else
    {
        ibrt_config->nv_role = IBRT_SLAVE;
    }

    nvrecord_env->ibrt_mode.mode = ibrt_config->nv_role;
}

int app_ibrt_ui_v2_test_config_load(void *config)
{
#if !defined(FREE_TWS_PAIRING_ENABLED)
    app_ibrt_raw_ui_test_load_from_bt_pair_list();
#endif

    ibrt_config_t *ibrt_config = (ibrt_config_t *)config;
    struct nvrecord_env_t *nvrecord_env;

    nv_record_env_get(&nvrecord_env);
    factory_section_original_btaddr_get(ibrt_config->local_addr.address);
    LOG_I("   jay [ %s ] ", __func__);

#if !defined(FREE_TWS_PAIRING_ENABLED)
    // nv record content has been updated in app_ibrt_raw_ui_test_load_from_bt_pair_list
    ibrt_config->nv_role = nvrecord_env->ibrt_mode.mode;

#else
    app_ibrt_initialize_nv_role_callback(ibrt_config, nvrecord_env);
#endif

#ifdef IBRT_RIGHT_MASTER
    if (IBRT_MASTER == nvrecord_env->ibrt_mode.mode)
    {
        ibrt_config->audio_chnl_sel = A2DP_AUDIO_CHANNEL_SELECT_RCHNL;
        app_ibrt_if_set_side(EAR_SIDE_RIGHT);
        memcpy(ibrt_config->peer_addr.address, nvrecord_env->ibrt_mode.record.bdAddr.address , BD_ADDR_LEN);
    }
    else if (IBRT_SLAVE == nvrecord_env->ibrt_mode.mode)
    {
        ibrt_config->audio_chnl_sel = A2DP_AUDIO_CHANNEL_SELECT_LCHNL;
        app_ibrt_if_set_side(EAR_SIDE_LEFT);
        memcpy(ibrt_config->peer_addr.address, nvrecord_env->ibrt_mode.record.bdAddr.address, BD_ADDR_LEN);
    }
    else
    {
        ibrt_config->nv_role = IBRT_UNKNOW;
        ibrt_config->audio_chnl_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
    }
#else
    if (IBRT_SLAVE == nvrecord_env->ibrt_mode.mode)
    {
        ibrt_config->audio_chnl_sel = A2DP_AUDIO_CHANNEL_SELECT_RCHNL;
        app_ibrt_if_set_side(EAR_SIDE_RIGHT);
        memcpy(ibrt_config->peer_addr.address, nvrecord_env->ibrt_mode.record.bdAddr.address , BD_ADDR_LEN);
    }
    else if (IBRT_MASTER == nvrecord_env->ibrt_mode.mode)
    {
        ibrt_config->audio_chnl_sel = A2DP_AUDIO_CHANNEL_SELECT_LCHNL;
        app_ibrt_if_set_side(EAR_SIDE_LEFT);
        memcpy(ibrt_config->peer_addr.address, nvrecord_env->ibrt_mode.record.bdAddr.address, BD_ADDR_LEN);
    }
    else
    {
        ibrt_config->nv_role = IBRT_UNKNOW;
        ibrt_config->audio_chnl_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
    }
#endif
#if defined(A2DP_SBC_PLC_ENABLED)
    ch_select = ibrt_config->audio_chnl_sel;
#endif
    app_ibrt_conn_set_ui_role(nvrecord_env->ibrt_mode.mode);

    return 0;
}

#if defined( __BT_ANC_KEY__)&&defined(ANC_APP)
extern void app_anc_key(APP_KEY_STATUS *status, void *param);
#endif

#if defined(APP_RX_API_ENABLE)
extern "C" void pmu_reboot(void);
static void app_ibrt_soft_reset_test(void)
{
    LOG_I("soft_reset_test");
    //app_reset();
    pmu_reboot();
}

static void app_ibrt_soft_reset_delete_bt_nv_test(void)
{
    LOG_I("soft_reset_after_delete_nv_test");
    nvrec_btdevicerecord *l_NvRecord;
    uint8_t record_num = 0;
    record_num = nv_record_get_paired_dev_list(&l_NvRecord);
    for (uint8_t i = 0; i < record_num; i++) {
        ddbif_delete_record(&l_NvRecord[i].record.bdAddr);
    }
    nv_record_flash_flush();
    osDelay(500);
    app_reset();
}

void app_ibrt_ui_tws_swtich_test(void)
{
    LOG_I("%s",__func__);
}

void app_ibrt_tws_connect_test(void)
{
    LOG_I("tws_connect_test");
    app_ibrt_conn_tws_connect_request(false, 0);
}

void app_ibrt_tws_disconnect_test(void)
{
    LOG_I("tws_disconnect_test");
    app_ibrt_conn_tws_disconnect();
}

void app_ibrt_mobile_connect_test(unsigned char *bt_addr, unsigned int lenth)
{
    LOG_I("mobile_connect_test");

    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        app_ibrt_conn_remote_dev_connect_request(&remote_addr,OUTGOING_CONNECTION_REQ, true, 0);
    }
}

void app_ibrt_mobile_disconnect_test(unsigned char *bt_addr, unsigned int lenth)
{
    LOG_I("mobile_disconnect_test");
    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        app_ibrt_conn_remote_dev_disconnect_request(&remote_addr, NULL);
    }
}

void app_ibrt_connect_profiles_test(void)
{
    LOG_I("connect_profiles_test");
    app_ibrt_conn_connect_profiles(NULL);
}

void app_ibrt_host_connect_cancel_test(void)
{
    LOG_I("host_connect_cancel_test");
    app_ibrt_conn_remote_dev_connect_request(NULL,OUTGOING_CONNECTION_REQ, true, 0);
    app_ibrt_conn_remote_dev_connect_cancel_request(NULL);
}

void app_ibrt_start_ibrt_test_with_addr(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        LOG_I("start_ibrt_test_with_addr:%02x:%02x:*:*:*:%02x",
            remote_addr.address[0], remote_addr.address[1], remote_addr.address[5]);
        app_ibrt_conn_connect_ibrt(&remote_addr);
    }
}

void app_ibrt_stop_ibrt_test_with_addr(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        LOG_I("stop_ibrt_test_with_addr:%02x:%02x:*:*:*:%02x",
            remote_addr.address[0], remote_addr.address[1], remote_addr.address[5]);
        app_ibrt_conn_disconnect_ibrt(&remote_addr);
    }
}

void app_ibrt_tws_role_get_request_test(void)
{
    tws_role_e role = app_ibrt_conn_tws_role_get_request(NULL);
    LOG_I("%s Role=%d",__func__,role);
}

void app_ibrt_tws_role_switch_test(void)
{
   app_ibrt_if_tws_role_switch_request();
}

void app_ibrt_tws_role_switch_test_with_addr(unsigned char *bt_addr, unsigned int lenth)
{
    ibrt_status_t status;
    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        status = app_ibrt_conn_tws_role_switch(&remote_addr);
        LOG_I("role_switch status=%d, addr:%02x:%02x:*:*:*:%02x", status,
            remote_addr.address[0], remote_addr.address[1], remote_addr.address[5]);
    }
}

void app_ibrt_get_remote_device_count(void)
{
    LOG_I("remote device count = %d",app_ibrt_conn_get_local_connected_mobile_count());
}

void app_ibrt_tws_dump_info_test(void)
{
    LOG_I("%s",__func__);
    app_ibrt_conn_dump_ibrt_info();
}

#ifdef GFPS_ENABLED
extern "C" void app_enter_fastpairing_mode(void);
#endif

void app_ibrt_enable_access_mode_test(void)
{
#ifdef GFPS_ENABLED
    app_enter_fastpairing_mode();
#endif

    LOG_I("%s",__func__);
    app_ibrt_conn_set_discoverable_connectable(true,true);
}

void app_ibrt_disable_access_mode_test(void)
{
    LOG_I("%s",__func__);
    app_ibrt_conn_set_discoverable_connectable(false,false);
}

void app_ibrt_tws_disc_all_mobile_link_test(void)
{
    LOG_I("%s",__func__);
    app_ibrt_conn_disc_all_mobile_link();
}

void app_ibrt_service_fuction_test()
{
    bool result;

    LOG_I("%s",__func__);
    result  = app_ibrt_conn_is_tws_in_pairing_state();
    LOG_I("tws is in pairing mode = %d", result);

    result = app_ibrt_conn_is_ibrt_master(NULL);
    LOG_I("tws current role is master = %d", result);

    result = app_ibrt_conn_is_ibrt_slave(NULL);
    LOG_I("tws current role is slave = %d",result);

    result = app_ibrt_conn_is_freeman_mode();
    LOG_I("tws is freeman mode = %d",result);

    app_tws_buds_info_t buds_info;
    memset((char*)&buds_info,0,sizeof(buds_info));
    app_ibrt_conn_get_tws_buds_info(&buds_info);
    LOG_I("tws local addr:");
    DUMP8("%02x ", buds_info.local_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    LOG_I("tws current role=%d",buds_info.current_ibrt_role);
    LOG_I("peer addr:");
    DUMP8("%02x ", buds_info.peer_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    LOG_I("tws peer role=%d",buds_info.peer_ibrt_role);
}

void app_ibrt_tws_w4_mobile_connect_test(void)
{
    LOG_I("%s",__func__);

    if(app_ibrt_conn_is_nv_master())
    {
         btif_me_set_accessible_mode(BTIF_BAM_GENERAL_ACCESSIBLE,NULL);
    }
    else
    {
         btif_me_set_accessible_mode(BTIF_BAM_NOT_ACCESSIBLE,NULL);
    }
}

void app_ibrt_tws_mobile_connection_test()
{
    LOG_I("%s",__func__);
    LOG_I("mobile connection state = %d",app_ibrt_conn_get_mobile_conn_state(NULL));
}

void app_ibrt_print_log_with_bt_address(const char *tag, uint8_t *bt_addr)
{
    LOG_I("%s addr: %02x:%02x:%02x:%02x:%02x:%02x", tag ? tag : "",
        bt_addr[0], bt_addr[1], bt_addr[2], bt_addr[3], bt_addr[4], bt_addr[5]);
}


void app_ibrt_get_a2dp_state_test(unsigned char *bt_addr,unsigned int lenth)
{
    const char* a2dp_state_strings[] = {
        "IDLE",
        "CODEC_CONFIGURED",
        "OPEN",
        "STREAMING",
        "CLOSED",
        "ABORTING"
    };
    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        AppIbrtA2dpState a2dp_state;
        AppIbrtStatus  status = app_ibrt_if_get_a2dp_state(&remote_addr, &a2dp_state);
        if(APP_IBRT_IF_STATUS_SUCCESS == status)
        {
            LOG_I("ibrt_ui_log:a2dp_state=%s",a2dp_state_strings[a2dp_state]);
        }
        else
        {
            LOG_I("ibrt_ui_log:get a2dp state error");
        }
    }
}

void app_ibrt_get_avrcp_state_test(unsigned char *bt_addr, unsigned int lenth)
{
    const char* avrcp_state_strings[] = {
        "DISCONNECTED",
        "CONNECTED",
        "PLAYING",
        "PAUSED",
        "VOLUME_UPDATED"
    };
    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        AppIbrtAvrcpState avrcp_state;
        AppIbrtStatus status = app_ibrt_if_get_avrcp_state(&remote_addr, &avrcp_state);
        if(APP_IBRT_IF_STATUS_SUCCESS == status)
        {
            LOG_I("ibrt_ui_log:avrcp_state=%s",avrcp_state_strings[avrcp_state]);
        }
        else
        {
            LOG_I("ibrt_ui_log:get avrcp state error");
        }
    }
}

void app_ibrt_get_hfp_state_test(unsigned char *bt_addr, unsigned int lenth)
{
    const char* hfp_state_strings[] = {
        "SLC_DISCONNECTED",
        "CLOSED",
        "SCO_CLOSED",
        "PENDING",
        "SLC_OPEN",
        "NEGOTIATE",
        "CODEC_CONFIGURED",
        "SCO_OPEN",
        "INCOMING_CALL",
        "OUTGOING_CALL",
        "RING_INDICATION"
    };
    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        AppIbrtHfpState hfp_state;
        AppIbrtStatus status = app_ibrt_if_get_hfp_state(&remote_addr, &hfp_state);
        if(APP_IBRT_IF_STATUS_SUCCESS == status)
        {
            LOG_I("ibrt_ui_log:hfp_state=%s",hfp_state_strings[hfp_state]);
        }
        else
        {
            LOG_I("ibrt_ui_log:get hfp state error");
        }
    }

}

void app_ibrt_get_call_status_test(unsigned char *bt_addr, unsigned int lenth)
{
    const char* call_status_strings[] = {
        "NO_CALL",
        "CALL_ACTIVE",
        "HOLD",
        "INCOMMING",
        "OUTGOING",
        "ALERT"
    };
    bt_bdaddr_t remote_addr;

    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        AppIbrtCallStatus call_status;
        AppIbrtStatus status = app_ibrt_if_get_hfp_call_status(&remote_addr, &call_status);
        if(APP_IBRT_IF_STATUS_SUCCESS == status)
        {
            LOG_I("ibrt_ui_log:call_status=%s",call_status_strings[call_status]);
        }
        else
        {
            LOG_I("ibrt_ui_log:get call status error");
        }
    }
}

void app_ibrt_tws_role_get_request_test_with_parameter(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t remote_addr;
    if(parse_bt_addr(bt_addr, &remote_addr, lenth) == 1)
    {
        tws_role_e role = app_ibrt_conn_tws_role_get_request(&remote_addr);
        LOG_I("Role=%d",role);
    }
}

void app_ibrt_get_tws_state_test(void)
{
    app_ibrt_if_get_tws_conn_state_test();
}

void app_ibrt_tws_avrcp_vol_up_test(void)
{
    app_audio_control_streaming_volume_up();
}

void app_ibrt_tws_avrcp_vol_down_test(void)
{
    app_audio_control_streaming_volume_down();
}

void app_ibrt_tws_avrcp_fast_forward_start_test(void)
{
    app_audio_control_media_fast_forward_start();
}

void app_ibrt_tws_avrcp_fast_forward_stop_test(void)
{
    app_audio_control_media_fast_forward_stop();
}

void app_ibrt_tws_avrcp_rewind_start_test(void)
{
    app_audio_control_media_rewind_start();
}

void app_ibrt_tws_avrcp_rewind_stop_test(void)
{
    app_audio_control_media_rewind_stop();
}

void app_ibrt_tws_avrcp_next_track_test(void)
{
    app_audio_control_media_forward();
}

void app_ibrt_tws_avrcp_prev_track_test(void)
{
    app_audio_control_media_backward();
}

void app_ibrt_ui_get_a2dp_active_phone(void)
{
    struct BT_DEVICE_T* curr_device = NULL;

    uint8_t device_id = app_audio_adm_get_bt_active_device();
    curr_device = app_bt_get_device(device_id);
    if (curr_device != NULL)
    {
        LOG_I("a2dp active phone addr: %02x:%02x:%02x:%02x:%02x:%02x", curr_device->remote.address[0], curr_device->remote.address[1],
        curr_device->remote.address[2],curr_device->remote.address[3], curr_device->remote.address[4], curr_device->remote.address[5]);
    }
    else
    {
        LOG_I("a2dp active phone = None");
    }
}

void app_ibrt_call_redial_test(void)
{
    app_audio_control_redial_last_call();
}

void app_ibrt_tws_dump_audio_state_test(void)
{
    app_bt_audio_state_checker();
}

#if defined(IBRT_UI_V2)
void app_ibrt_mgr_open_box_event_test(void)
{
    app_ibrt_if_event_entry(APP_UI_EV_CASE_OPEN);
}

void app_ibrt_mgr_fetch_out_box_event_test(void)
{
    app_ibrt_if_event_entry(APP_UI_EV_UNDOCK);
}

void app_ibrt_mgr_put_in_box_event_test(void)
{
    app_ibrt_if_event_entry(APP_UI_EV_DOCK);
}

void app_ibrt_mgr_close_box_event_test(void)
{
    app_ibrt_if_event_entry(APP_UI_EV_CASE_CLOSE);
}

void app_ibrt_mgr_exit_earbuds_mode_test(void)
{
    app_ui_exit_earbud_mode();
}

void app_ibrt_mgr_wear_up_event_test(void)
{
    app_ibrt_if_event_entry(APP_UI_EV_WEAR_UP);
}

void app_ibrt_mgr_wear_down_event_test(void)
{
    app_ibrt_if_event_entry(APP_UI_EV_WEAR_DOWN);
}

void app_ibrt_mgr_free_man_test(void)
{
    app_ibrt_if_test_enter_freeman();
}

void app_ibrt_mgr_free_man_test_1(void)
{
    app_ibrt_if_event_entry(APP_UI_EV_FREE_MAN_MODE);
}

void app_ibrt_mgr_dump_ui_status_test(void)
{
    app_ibrt_if_dump_ui_status();
}

void app_ibrt_mgr_switch_bt_single_mode_test(void)
{
    if (!app_ui_change_mode_ext(false, false, NULL))
    {
        LOG_I("change mode failed!");
    }
}

void app_ibrt_mgr_switch_bt_multi_mode_test(void)
{
    if (!app_ui_change_mode_ext(false, true, NULL))
    {
        LOG_I("change mode failed!");
    }
}

void app_ibrt_mgr_switch_coex_single_mode_test(void)
{
    if (!app_ui_change_mode_ext(true, false, NULL))
    {
        LOG_I("change mode failed!");
    }
}

void app_ibrt_mgr_switch_coex_multi_mode_test(void)
{
    if (!app_ui_change_mode_ext(true, true, NULL))
    {
        LOG_I("change mode failed!");
    }
}

void app_ibrt_mgr_monitor_dump_test(void)
{
    app_ui_monitor_dump();
}

void app_ibrt_ui_dump_adm_active_info(void)
{
    PLAYBACK_INFO_T* audioContrlInfo = app_bt_get_music_playback_info();
    CALL_STATE_INFO_T* callInfo = app_bt_get_call_state_info();
    uint8_t deviceType = audioContrlInfo->device_type;
    uint8_t deviceId = audioContrlInfo->device_id;
    uint8_t playState = (uint8_t)audioContrlInfo->playback_status;
    uint8_t call_id = callInfo->device_id;
    uint8_t callType = callInfo->device_type;
    uint8_t callState = (uint8_t)callInfo->state;
    char ptrAudioCtr[128];
    uint8_t btAddr[6] = {0};
    switch (deviceType)
    {
        case AUDIO_TYPE_BT:
        {
            struct BT_DEVICE_T *btDevice = app_bt_get_device(deviceId);
            if(NULL != btDevice)
            {
                sprintf(ptrAudioCtr, "audio_control: device: %d, type: %d, music: %d", deviceId, deviceType, playState);
                memcpy(btAddr, btDevice->remote.address, 6);
                app_ibrt_print_log_with_bt_address((const char *)ptrAudioCtr, btAddr);
            }
            break;
        }
        case AUDIO_TYPE_LE_AUDIO:
        {
#if BLE_AUDIO_ENABLED
            AOB_MOBILE_INFO_T *pLEMobileInfo = ble_audio_earphone_info_get_mobile_info(deviceId);
            if(NULL != pLEMobileInfo)
            {
                sprintf(ptrAudioCtr, "audio_control: device: %d, type: %d, music: %d", deviceId, deviceType, playState);
                memcpy(btAddr, pLEMobileInfo->peer_ble_addr.addr, 6);
                app_ibrt_print_log_with_bt_address((const char *)ptrAudioCtr, btAddr);
            }
            break;
#endif
        }
        default:
            sprintf(ptrAudioCtr, "audio_control: device: %d, type: %d, music: %d", deviceId, deviceType, playState);
            app_ibrt_print_log_with_bt_address((const char *)ptrAudioCtr, btAddr);
            break;

    }
    memset(btAddr, 0, 6);
    memset(ptrAudioCtr, 0, 128);
    switch (callType)
    {
        case AUDIO_TYPE_BT:
        {
            struct BT_DEVICE_T *btDevice = app_bt_get_device(call_id);
            if(NULL != btDevice)
            {
                sprintf(ptrAudioCtr, "audio_control: device: %d, type: %d, call: %d", call_id, callType, callState);
                memcpy(btAddr, btDevice->remote.address, 6);
                app_ibrt_print_log_with_bt_address((const char *)ptrAudioCtr, btAddr);
            }
            break;
        }
        case AUDIO_TYPE_LE_AUDIO:
        {
#if BLE_AUDIO_ENABLED
            AOB_MOBILE_INFO_T *pLEMobileInfo = ble_audio_earphone_info_get_mobile_info(deviceId);
            if(NULL != pLEMobileInfo)
            {
                sprintf(ptrAudioCtr, "audio_control: device: %d, type: %d, call: %d", call_id, callType, callState);
                memcpy(btAddr, pLEMobileInfo->peer_ble_addr.addr, 6);
                app_ibrt_print_log_with_bt_address((const char *)ptrAudioCtr, btAddr);
            }
            break;
#endif
        }
        default:
            sprintf(ptrAudioCtr, "audio_control: device: %d, type: %d, call: %d", call_id, callType, callState);
            app_ibrt_print_log_with_bt_address((const char *)ptrAudioCtr, btAddr);
            break;

    }

}

void app_ibrt_dump_media_info_test(void)
{
    //uint16_t activeMedia = bt_media_get_current_media();
    uint8_t support_max_remote_link = app_ibrt_conn_support_max_mobile_dev();
    for (int i = 0; i < support_max_remote_link; i++)
    {
        struct BT_DEVICE_T* bt_device = NULL;
        bt_device = app_bt_get_device(i);
        if(bt_device && bt_device ->acl_is_connected)
        {
            uint8_t btAddr[6] = {0};
            memcpy(btAddr, bt_device->remote.address, 6);
            uint8_t abs_vol = bt_device->a2dp_current_abs_volume;
            uint8_t a2dp_vol = a2dp_volume_get((enum BT_DEVICE_ID_T)i);
            uint8_t stream_vol = app_bt_stream_local_volume_get();
            uint8_t audio_codec = app_audio_manager_get_a2dp_codec_type(i);
            char ptrMediaInfo[128];
            sprintf(ptrMediaInfo, "[d%d]vol abs:%d, local:%d, stream:%d, codec:%d, rate:%d, audio:%d,",
                    i, abs_vol, a2dp_vol,stream_vol,
                    bt_device->codec_type, bt_device->sample_rate, audio_codec);
            app_ibrt_print_log_with_bt_address((const char *)ptrMediaInfo, btAddr);
        }
    }

}

void app_ibrt_mgr_user_role_switch_test(void)
{
    app_ui_user_role_switch(true);
}

void app_ibrt_mgr_user_role_switch_test1(void)
{
    app_ui_user_role_switch(false);
}

void app_ibrt_mgr_enable_page_test(void)
{
    app_ui_set_page_enabled(true);
}

void app_ibrt_mgr_disable_page_test(void)
{
    app_ui_set_page_enabled(false);
}

void app_ibrt_mgr_shutdown_test(void)
{
    app_ui_shutdown();
}

void app_ibrt_mgr_switch2walkie_talkie_test(void)
{
    app_switch_mode(NV_APP_WALKIE_TALKIE, true);
}

#if BLE_AUDIO_ENABLED
void app_ibrt_mgr_user_start_lea_adv(void)
{
    app_ui_user_start_lea_adv(60000, NULL, true);
}

void app_ibrt_mgr_user_stop_lea_adv(void)
{
    app_ui_user_stop_lea_adv(true);
}
#endif

#endif

static void app_prompt_PlayAudio(void)
{
    media_PlayAudio(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

static void app_prompt_locally_PlayAudio(void)
{
    media_PlayAudio_locally(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

static void app_prompt_remotely_PlayAudio(void)
{
    media_PlayAudio_remotely(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

static void app_prompt_standalone_PlayAudio(void)
{
    media_PlayAudio_standalone(AUD_ID_BT_CALL_INCOMING_CALL, 0);
}

static void app_prompt_test_stop_all(void)
{
    app_prompt_stop_all();
}

extern void bt_change_to_iic(APP_KEY_STATUS *status, void *param);
void app_ibrt_ui_iic_uart_switch_test(void)
{
    bt_change_to_iic(NULL,NULL);
}

static bool trace_disabled;

static void app_trace_test_enable(void)
{
    if (trace_disabled) {
        trace_disabled = false;
        hal_trace_continue();
}
}
extern void bt_drv_reg_op_trigger_controller_assert(void);

static void app_trace_test_disable(void)
{
    if (!trace_disabled) {
        trace_disabled = true;
        hal_trace_pause();
}
}

static void app_trace_test_get_history_trace(void)
{
    const uint8_t* preBuf;
    const uint8_t* postBuf;
    uint32_t preBufLen, postBufLen;
    hal_trace_get_history_buffer(&preBuf, &preBufLen, &postBuf, &postBufLen);
    LOG_I("prelen %d postlen %d", preBufLen, postBufLen);
    uint8_t tmpTest[16];
    memset(tmpTest, 0, sizeof(tmpTest));
    memcpy(tmpTest, preBuf,
        (preBufLen > (sizeof(tmpTest)-1)?(sizeof(tmpTest)-1):preBufLen));
    LOG_I("preBuf:");
    LOG_I("%s", tmpTest);
    memset(tmpTest, 0, sizeof(tmpTest));
    memcpy(tmpTest, postBuf,
        (postBufLen > (sizeof(tmpTest)-1)?(sizeof(tmpTest)-1):postBufLen));
    LOG_I("postBuf:");
    LOG_I("%s", tmpTest);
}

void app_trigger_rx_close_test(void)
{
    hal_trace_rx_sleep();
}

#if defined( __BT_ANC_KEY__)&&defined(ANC_APP)
extern void app_anc_key(APP_KEY_STATUS *status, void *param);
static void app_test_anc_handler(void)
{
    app_anc_key(NULL, NULL);
}
#endif

#ifdef __AI_VOICE__
extern void app_test_toggle_hotword_supervising(void);
static void app_test_thirdparty_hotword(void)
{
    app_test_toggle_hotword_supervising();
}
#endif
#ifdef SENSOR_HUB
#ifdef CORE_BRIDGE_DEMO_MSG
#include "mcu_sensor_hub_app.h"

static void app_test_mcu_sensorhub_demo_req_no_rsp(void)
{
    app_mcu_sensor_hub_send_demo_req_no_rsp();
}

static void app_test_mcu_sensorhub_demo_req_with_rsp(void)
{
    app_mcu_sensor_hub_send_demo_req_with_rsp();
}

static void app_test_mcu_sensorhub_demo_instant_req(void)
{
    app_mcu_sensor_hub_send_demo_instant_req();
}
#endif
#endif

// free tws test steps:
/*
Preparation:
1. Assure following macros are proper configured in target.mk
   POWER_ON_ENTER_TWS_PAIRING_ENABLED ?= 0
   FREE_TWS_PAIRING_ENABLED ?= 1
2. Program device A with bt addr testMasterBdAddr, select erase
   the whole flash when programming
3. Program device B with bt addr testSlaveBdAddr, select erase
   the whole flash when programming
Case 1: Do pairing info nv record update and TWS pairing
- Do preparation
- Call uart cmd "ibrt_test:test_master_tws_pairing" on device A
- Call uart cmd "ibrt_test:test_slave_tws_pairing" on device B
The device A and B will be successfully paired, after that,
the master bud A will enter discoverable mode for mobile to discover and connect

Case 2: Do pairing info nv record update only firslty, then trigger reboot and
        let TWS paired
- Do preparation
- Call uart cmd "ibrt_test:master_update_tws_pair_info_test_func" on device A
- Call uart cmd "ibrt_test:slave_update_tws_pair_info_test_func" on device B
- Call uart cmd "ibrt_test:soft_reset" on device A and B
- After A and B reboot
- Call uart cmd "ibrt_test:open_box_event_test" on device A and B
The device A and B will be successfully paired
- Call uart cmd "ibrt_test:enable_access_mode_test" on device A,
the master bud A will enter discoverable mode for mobile to discover and connect

*/

static void test_master_tws_pairing(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t twsSlaveBtAddr;

    if(parse_bt_addr(bt_addr, &twsSlaveBtAddr, lenth) == 1)
    {
        app_ibrt_if_init_open_box_state_for_evb();
        app_ibrt_if_start_tws_pairing(IBRT_MASTER, (uint8_t *)&twsSlaveBtAddr);
    }
}

static void test_slave_tws_pairing(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t twsMasterBtAddr;

    if(parse_bt_addr(bt_addr, &twsMasterBtAddr, lenth) == 1)
    {
        app_ibrt_if_init_open_box_state_for_evb();
        app_ibrt_if_start_tws_pairing(IBRT_SLAVE, (uint8_t *)&twsMasterBtAddr);
    }
}

static void master_update_tws_pair_info_test_func(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t twsSlaveBtAddr;

    if(parse_bt_addr(bt_addr, &twsSlaveBtAddr, lenth) == 1)
    {
        app_ibrt_if_update_tws_pairing_info(IBRT_MASTER, (uint8_t *)&twsSlaveBtAddr);
    }
}

static void slave_update_tws_pair_info_test_func(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t twsMasterBtAddr;

    if(parse_bt_addr(bt_addr, &twsMasterBtAddr, lenth) == 1)
    {
        app_ibrt_if_update_tws_pairing_info(IBRT_SLAVE, (uint8_t *)&twsMasterBtAddr);
    }
}

static void m_write_bt_local_addr_test(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t twsMasterBtAddr;

    if(parse_bt_addr(bt_addr, &twsMasterBtAddr, lenth) == 1)
    {
        app_ibrt_if_write_bt_local_address((uint8_t *)&twsMasterBtAddr);
    }
}

static void s_write_bt_local_addr_test(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t twsSlaveBtAddr;

    if(parse_bt_addr(bt_addr, &twsSlaveBtAddr, lenth) == 1)
    {
        app_ibrt_if_write_bt_local_address((uint8_t *)&twsSlaveBtAddr);
    }
}

static void m_write_ble_local_addr_test(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t twsMasterBLEAddr;

    if(parse_bt_addr(bt_addr, &twsMasterBLEAddr, lenth) == 1)
    {
        app_ibrt_if_write_ble_local_address((uint8_t *)&twsMasterBLEAddr);
    }
}

static void s_write_ble_local_addr_test(unsigned char *bt_addr, unsigned int lenth)
{
    bt_bdaddr_t twsSlaveBleAddr;

    if(parse_bt_addr(bt_addr, &twsSlaveBleAddr, lenth) == 1)
    {
        app_ibrt_if_write_ble_local_address((uint8_t *)&twsSlaveBleAddr);
    }
}

static void turn_on_jlink_test(void)
{
    hal_iomux_set_jtag();
    hal_cmu_jtag_clock_enable();
}

#ifdef AUDIO_MANAGER_TEST_ENABLE
static void app_audio_adm_api_ut_test()
{
    app_audio_adm_api_ut();
}

static void app_audio_focus_stack_test()
{
    app_audio_focus_stack_unit_test();
}

static void app_audio_focus_media_ctrl_test()
{
    app_audio_focus_ctrl_stub_test();
}
#endif

void increase_dst_mtu(void);
void decrease_dst_mtu(void);

void dip_test(void)
{
#if defined(BT_DIP_SUPPORT)
    ibrt_if_pnp_info* pnp_info = NULL;
    struct BT_DEVICE_T *curr_device = NULL;

    curr_device = app_bt_get_device(BT_DEVICE_ID_1);

    if (curr_device)
    {
        pnp_info = app_ibrt_if_get_pnp_info(&curr_device->remote);
    }

    if (pnp_info)
    {
        LOG_I("%s vendor_id %04x product_id %04x product_version %04x",
                __func__, pnp_info->vend_id, pnp_info->prod_id, pnp_info->prod_ver);
    }
    else
    {
        LOG_I("%s N/A", __func__);
    }
#endif
}

#if BLE_AUDIO_ENABLED
void app_ibrt_tws_dump_ble_conn_state(void)
{
	BLE_MODE_ENV_T *ble_mode_env_p = app_ble_get_mode_env();
    //judge ble whether in adv mode
    if(app_ble_is_in_advertising_state())
	{
        uint8_t bleAdvType;
        uint32_t bleAdvIntervalMs;
        app_ble_get_runtime_adv_param(&bleAdvType, &bleAdvIntervalMs);
        LOG_I("BLE adv state: %d, adv type: %d, interval: %d, busy state: %d", APP_ADV_STATE_STARTED,
            ble_mode_env_p->advParamInfo.advType, ble_mode_env_p->advParamInfo.advInterval, ble_mode_env_p->ble_is_busy);
    }
	else
	{
        LOG_I("BLE adv state: %d, adv type: interval: busy state: %d", APP_ACTV_STATE_IDLE, ble_mode_env_p->ble_is_busy);
    }
    //judge ble whether have connect handle
    if(app_ble_is_any_connection_exist())
	{
        LOG_I("BLE cnn state: %d", BLE_CONNECTED);
    }
	else
	{
        LOG_I("BLE cnn state: %d", BLE_DISCONNECTED);
    }
    for (uint8_t index = 0; index < MOBILE_CONNECTION_MAX; index++)
    {
        // dump mobile state
        AOB_MOBILE_INFO_T *p_mobileInfo = ble_audio_earphone_info_get_mobile_info(index);
        if(NULL != p_mobileInfo && true == p_mobileInfo->connected)
        {
            BLE_ADDR_INFO_T bleAddr = p_mobileInfo->peer_ble_addr;
            LOG_I("[BLE Audio]: mobile: %d mute: %d, volume: %d, address: %02x:%02x:%02x:%02x:%02x:%02x",
                p_mobileInfo->conidx, p_mobileInfo->muted, p_mobileInfo->volume,
                bleAddr.addr[0], bleAddr.addr[1], bleAddr.addr[2],bleAddr.addr[3], bleAddr.addr[4], bleAddr.addr[5]);
            //dump ble cis and ase state
            app_bap_ascs_ase_t *p_ase = app_bap_uc_srv_get_ase_info(p_mobileInfo->conidx);
            if(NULL != p_ase)
            {
                LOG_I("[BLE Audio]: index: %d, cis state: %d, handle: %d", p_mobileInfo->conidx, p_ase->ase_state, p_ase->cis_hdl);
            }
        }
    }
}

void app_test_bleaud_adv_test(void)
{
    app_ui_start_ble_connecteable_adv(BLE_AUDIO_ADV_DURATION,NULL);
}
#endif

#ifdef CFG_LE_PWR_CTRL
void app_ibrt_path_loss_test(void)
{
    LOG_I("%s", __func__);
    appm_set_path_loss_rep_param_cmd(0, 1, 1, 1, 0, 0, 1);
}
#endif

#ifdef ANC_ASSIST_ENABLED
#include "bluetooth_bt_api.h"

extern "C" void media_PlayAudio(AUD_ID_ENUM id,uint8_t device_id);

void app_test_adaptive_eq(void) {
    media_PlayAudio(AUD_ID_ANC_PROMPT,0);
}
#endif

#ifdef HEARING_AID_ENABLED
extern "C" int hearing_aid_stream_start(bool on);

void app_hearing_aid_start(void)
{
    hearing_aid_stream_start(true);
}

void app_hearing_aid_stop(void)
{
    hearing_aid_stream_start(false);
}
#endif


#if APP_TRACE_RX_ENABLE
void app_ibrt_dump_a2dp_info_test(void)
{
    uint8_t support_max_remote_link = app_ibrt_conn_support_max_mobile_dev();
    for (int i = 0; i < support_max_remote_link; i++)
    {
        struct BT_DEVICE_T* bt_device = NULL;
        bt_device = app_bt_get_device(i);
        if(bt_device ->acl_is_connected)
        {
            uint8_t abs_vol = a2dp_abs_volume_get(i);
            uint8_t a2dp_vol = a2dp_volume_get(i);
            uint8_t stream_vol = app_bt_stream_local_volume_get();
            uint8_t audio_codec = app_audio_manager_get_a2dp_codec_type(i);
            LOG_I("device:%d, vol abs:%d, local:%d, stream:%d, codec:%d, rate:%d, audio:%d, addr:%02x%02x%02x%02x%02x%02x",
                    i, abs_vol, a2dp_vol, stream_vol, bt_device->codec_type, bt_device->sample_rate, audio_codec,
                    bt_device->remote.address[5], bt_device->remote.address[4], bt_device->remote.address[3],
                    bt_device->remote.address[2], bt_device->remote.address[1], bt_device->remote.address[0]);
        }
    }

}
#endif

/***************************************************************************
Function    :  app_ibrt_tws_active_device_test
Parameters:
Return:
Description:
    test the correct of active device:
    1、if current is a2dp start,the action is send avrcp pause key to stop a2dp play.
    2、if current is a2dp suspend,the action is send avrcp play key to start a2dp play
    3、if current is incoming call,the action is answer call.
    4、if current is call active，the action is hungup call.
****************************************************************************/

void app_ibrt_tws_active_device_test(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    uint8_t state_action = IBRT_ACTION_PLAY;
    BT_AUDIO_DEVICE_T *active_device = app_audio_adm_get_active_device();
    ASSERT(active_device,"no active device");
    uint8_t device_id = active_device->device_id;
    curr_device = app_bt_get_device(device_id);
    if(curr_device->hfchan_call == BTIF_HF_CALL_ACTIVE)
    {
        state_action = IBRT_ACTION_HANGUP;
    }
    else if(curr_device->hfchan_callSetup == BTIF_HF_CALL_SETUP_IN)
    {
        state_action = IBRT_ACTION_ANSWER;
    }
    else if(curr_device->a2dp_streamming)
    {
        state_action = IBRT_ACTION_PAUSE;
    }
    else
    {
        state_action = IBRT_ACTION_PLAY;
    }
    LOG_I("active_device is:d%x,action:%d",device_id, state_action);
    app_ibrt_if_start_user_action_v2(device_id, state_action , 0, 0);
}

void app_ibrt_tws_force_set_bt_as_act_dev_test(void)
{
    BT_AUDIO_DEVICE_T bt_device = {
          .device_type = AUDIO_TYPE_BT,
          .device_id = 0 };

    app_adm_force_set_active_device(&bt_device);
}

void app_ibrt_tws_force_set_ble_as_act_dev_test(void)
{
     BT_AUDIO_DEVICE_T ble_device = {
          .device_type = AUDIO_TYPE_LE_AUDIO,
          .device_id = 0 };

    app_adm_force_set_active_device(&ble_device);
}

void app_ibrt_tws_switching_a2dp_test(void)
{
    app_ibrt_if_switch_streaming_a2dp();
}

void app_ibrt_tws_switch_sco_test(void)
{
    app_ibrt_if_hold_background_switch();
}

void app_ibrt_tws_audio_ctrl_test(void)
{
    bt_key_handle_func_click();
}

void app_terminate_call_test(void)
{
    app_audio_control_call_terminate();
}

void app_hold_call_test(void)
{
    app_audio_control_handle_three_way_incoming_call();
}

void app_reject_3way_incoming(void)
{
    app_audio_control_release_hold_call_and_3way_incoming();
}

void app_reject_3way_active(void)
{
    app_audio_control_release_active_call();
}

void app_ibrt_mgr_pairing_mode_test(void);

void app_answer_call_test(void)
{
    app_audio_control_call_answer();
}

void app_play_muisc_test(void)
{
    app_audio_control_media_play();
}

void app_pause_music_test(void)
{
    app_audio_control_media_pause();
}

void app_dump_hfp_machine_test()
{
    app_audio_get_current_hfp_machine();
}

static void app_ibrt_slt_test_mcu_freq_clkout_enable(void)
{
    LOG_I("%s", __func__);
    hal_iomux_set_mcu_clock_out();
}

static void app_ibrt_slt_test_mcu_freq_clkout_disable(void)
{
    LOG_I("%s", __func__);
    hal_iomux_clr_clock_out();
}

static void app_ibrt_slt_test_mcu_req_208m(void)
{
    LOG_I("%s", __func__);
    app_sysfreq_req(APP_SYSFREQ_SLT_TEST, APP_SYSFREQ_208M);
}

static void app_ibrt_slt_test_mcu_req_104m(void)
{
    LOG_I("%s", __func__);
    app_sysfreq_req(APP_SYSFREQ_SLT_TEST, APP_SYSFREQ_104M);
}

static void app_ibrt_slt_test_mcu_req_52m(void)
{
    LOG_I("%s", __func__);
    app_sysfreq_req(APP_SYSFREQ_SLT_TEST, APP_SYSFREQ_52M);
}

static void app_ibrt_slt_test_mcu_req_26m(void)
{
    LOG_I("%s", __func__);
    app_sysfreq_req(APP_SYSFREQ_SLT_TEST, APP_SYSFREQ_26M);
}

// Need call this function before enter deep sleep!
static void app_ibrt_slt_test_mcu_req_32k(void)
{
    LOG_I("%s", __func__);
    app_sysfreq_req(APP_SYSFREQ_SLT_TEST, APP_SYSFREQ_32K);
}

/*
* Connection step: tws_slave_pairing_test->tws_master_pairing_test->
* w4_host_connect -> start_ibrt_test
*/
const app_bt_cmd_handle_t app_ibrt_v2_uart_test_handle[]=
{
    {"soft_reset", app_ibrt_soft_reset_test},
    {"reboot_with_delete_nv",  app_ibrt_soft_reset_delete_bt_nv_test},
    {"delete_all_ble_nv_info", nv_record_ble_delete_all_entry},

#if defined(IBRT_UI_V2)
    {"open_box_event_test",         app_ibrt_mgr_open_box_event_test},
    {"fetch_out_box_event_test",    app_ibrt_mgr_fetch_out_box_event_test},
    {"put_in_box_event_test",       app_ibrt_mgr_put_in_box_event_test},
    {"close_box_event_test",        app_ibrt_mgr_close_box_event_test},
    {"wear_up_event_test",          app_ibrt_mgr_wear_up_event_test},
    {"wear_down_event_test",        app_ibrt_mgr_wear_down_event_test},
    {"pairing_mode_test",           app_ibrt_mgr_pairing_mode_test},
    {"free_man_test",               app_ibrt_mgr_free_man_test_1},
    {"dump_ui_status_test",         app_ibrt_mgr_dump_ui_status_test},
    {"switch_bt_single_mode_test",  app_ibrt_mgr_switch_bt_single_mode_test},
    {"switch_bt_multi_mode_test",   app_ibrt_mgr_switch_bt_multi_mode_test},
    {"switch_coex_single_mode_test",app_ibrt_mgr_switch_coex_single_mode_test},
    {"switch_coex_multi_mode_test", app_ibrt_mgr_switch_coex_multi_mode_test},
    {"monitor_dump_test",           app_ibrt_mgr_monitor_dump_test},
    {"user_rs_to_master",           app_ibrt_mgr_user_role_switch_test},
    {"user_rs_to_slave",            app_ibrt_mgr_user_role_switch_test1},
    {"enable_page",                 app_ibrt_mgr_enable_page_test},
    {"disable_page",                app_ibrt_mgr_disable_page_test},
    {"shutdown",                    app_ibrt_mgr_shutdown_test},
    {"switch2walkie_talkie",        app_ibrt_mgr_switch2walkie_talkie_test},
    {"exit_earbuds_mode",           app_ibrt_mgr_exit_earbuds_mode_test},
#if BLE_AUDIO_ENABLED
    {"user_start_lea_adv",          app_ibrt_mgr_user_start_lea_adv},
    {"user_stop_lea_adv",           app_ibrt_mgr_user_stop_lea_adv},
#endif
#endif
    {"iic_switch", app_ibrt_ui_iic_uart_switch_test},
    {"enable_access_mode_test",app_ibrt_enable_access_mode_test},
    {"disable_access_mode_test", app_ibrt_disable_access_mode_test},
    {"tws_connect_test",app_ibrt_tws_connect_test},
    {"tws_disconnect_test",app_ibrt_tws_disconnect_test},
    {"connect_profiles_test",app_ibrt_connect_profiles_test},
    {"host_cancel_connect_test",app_ibrt_host_connect_cancel_test},
    {"tws_get_role_test",app_ibrt_tws_role_get_request_test},
    {"role_switch",app_ibrt_tws_role_switch_test},
    {"disc_all_mobile_test",app_ibrt_tws_disc_all_mobile_link_test},
    {"w4_mobile_connect_test",app_ibrt_tws_w4_mobile_connect_test},
    {"get_curr_link_count_test",app_ibrt_get_remote_device_count},
    {"dump_ibrt_conn_info",app_ibrt_tws_dump_info_test},
    {"ibrt_function_test",app_ibrt_service_fuction_test},
    {"get_mobile_conn_status_test",app_ibrt_tws_mobile_connection_test},
    {"get_tws_state",app_ibrt_get_tws_state_test},
    {"avrcp_vol_up_test",app_ibrt_tws_avrcp_vol_up_test},
    {"avrcp_vol_down_test",app_ibrt_tws_avrcp_vol_down_test},
    {"avrcp_next_track_test",app_ibrt_tws_avrcp_next_track_test},
    {"avrcp_prev_track_test",app_ibrt_tws_avrcp_prev_track_test},
    {"avrcp_fast_forward_start_test", app_ibrt_tws_avrcp_fast_forward_start_test},
    {"avrcp_fast_forward_stop_test", app_ibrt_tws_avrcp_fast_forward_stop_test},
    {"avrcp_rewind_start_test", app_ibrt_tws_avrcp_rewind_start_test},
    {"avrcp_rewind_stop_test", app_ibrt_tws_avrcp_rewind_stop_test},
    {"a2dp_active_phone",app_ibrt_ui_get_a2dp_active_phone},
    {"call_redial",app_ibrt_call_redial_test},
    {"app_prompt_PlayAudio",app_prompt_PlayAudio},
    {"app_prompt_locally_PlayAudio",app_prompt_locally_PlayAudio},
    {"app_prompt_remotely_PlayAudio",app_prompt_remotely_PlayAudio},
    {"app_prompt_standalone_PlayAudio",app_prompt_standalone_PlayAudio},
    {"app_prompt_test_stop_all",app_prompt_test_stop_all},
    {"trace_test_enable",app_trace_test_enable},
    {"trace_test_disable",app_trace_test_disable},
    {"test_get_history_trace",app_trace_test_get_history_trace},
    {"rx_close",app_trigger_rx_close_test},
    {"dump_audio_state_info", app_ibrt_tws_dump_audio_state_test},
    {"active device test",app_ibrt_tws_active_device_test},
    {"force_set_bt_act_dev",app_ibrt_tws_force_set_bt_as_act_dev_test},
    {"force_set_ble_act_dev",app_ibrt_tws_force_set_ble_as_act_dev_test},
    {"adm_print",app_ibrt_ui_dump_adm_active_info},
    {"dump_a2dp_info", app_ibrt_dump_media_info_test},
    {"a2dp_switch_test", app_ibrt_tws_switching_a2dp_test},
    {"switch_sco_test",app_ibrt_tws_switch_sco_test},
    {"audio_ctrl_test",app_ibrt_tws_audio_ctrl_test},
    {"terminate_call_test",app_terminate_call_test},
    {"answer_call_test",app_answer_call_test},
    {"play_music_test",app_play_muisc_test},
    {"pause_music_test",app_pause_music_test},
    {"hold_call_test",app_hold_call_test},
    {"reject_3way_incoming_test",app_reject_3way_incoming},
    {"reject_3way_active_test",app_reject_3way_active},
    {"dump_hfp_machine",app_dump_hfp_machine_test},
#if defined( __BT_ANC_KEY__)&&defined(ANC_APP)
    {"toggle_anc_test",app_test_anc_handler},
#endif
#ifdef __AI_VOICE__
    {"toggle_hw_thirdparty",app_test_thirdparty_hotword},
#endif
#ifdef SENSOR_HUB
#ifdef CORE_BRIDGE_DEMO_MSG
    {"no_rsp_sensorhub_mcu_demo_cmd",app_test_mcu_sensorhub_demo_req_no_rsp},
    {"with_rsp_sensorhub_mcu_demo_cmd",app_test_mcu_sensorhub_demo_req_with_rsp},
    {"instant_sensorhub_mcu_demo_cmd",app_test_mcu_sensorhub_demo_instant_req},
#endif
#endif

    {"trigger_controller_assert",bt_drv_reg_op_trigger_controller_assert},
    {"turn_on_jlink_test",turn_on_jlink_test},
    {"increase_dst_mtu",increase_dst_mtu},
    {"decrease_dst_mtu",decrease_dst_mtu},
    {"dip_test",dip_test},
#if BLE_AUDIO_ENABLED
    {"dump_ble_cnn_state",app_ibrt_tws_dump_ble_conn_state},
    {"bleaud_adv_test", app_test_bleaud_adv_test},
#endif

#ifdef AUDIO_MANAGER_TEST_ENABLE
    {"adm_api_ut",app_audio_adm_api_ut_test},
    {"audio_focus_stack_test",app_audio_focus_stack_test},
    {"audio_focus_ctrl_test",app_audio_focus_media_ctrl_test},
#endif

#ifdef CFG_LE_PWR_CTRL
    {"path_loss_test", app_ibrt_path_loss_test},
#endif

#ifdef ANC_ASSIST_ENABLED
    {"adaptive_eq", app_test_adaptive_eq},
#endif

#ifdef HEARING_AID_ENABLED
    {"hearing_aid_start", app_hearing_aid_start},
    {"hearing_aid_stop", app_hearing_aid_stop},
#endif

    //A2DP/SNK/AVP/BI-01-C
    {"AVDTP_reject_INVALID_OBJECT_TYPE", btif_pts_reject_INVALID_OBJECT_TYPE},
    //A2DP/SNK/AVP/BI-02-C
    {"AVDTP_reject_INVALID_CHANNELS", btif_pts_reject_INVALID_CHANNELS},
    //A2DP/SNK/AVP/BI-03-C
    {"AVDTP_reject_INVALID_SAMPLING_FREQUENCY", btif_pts_reject_INVALID_SAMPLING_FREQUENCY},
    //A2DP/SNK/AVP/BI-04-C
    {"AVDTP_reject_INVALID_DRC", btif_pts_reject_INVALID_DRC},
    //A2DP/SNK/AVP/BI-06-C
    {"AVDTP_reject_NOT_SUPPORTED_OBJECT_TYPE", btif_pts_reject_NOT_SUPPORTED_OBJECT_TYPE},
    //A2DP/SNK/AVP/BI-07-C
    {"AVDTP_reject_NOT_SUPPORTED_CHANNELS", btif_pts_reject_NOT_SUPPORTED_CHANNELS},
    //A2DP/SNK/AVP/BI-08-C
    {"AVDTP_reject_NOT_SUPPORTED_SAMPLING_FREQUENCY", btif_pts_reject_NOT_SUPPORTED_SAMPLING_FREQUENCY},
    //A2DP/SNK/AVP/BI-09-C
    {"AVDTP_reject_NOT_SUPPORTED_DRC", btif_pts_reject_NOT_SUPPORTED_DRC},
    //A2DP/SNK/AVP/BI-10-C
    {"AVDTP_reject_INVALID_CODEC_TYPE", btif_pts_reject_INVALID_CODEC_TYPE},
    //A2DP/SNK/AVP/BI-11-C
    {"AVDTP_reject_INVALID_CHANNEL_MODE", btif_pts_reject_INVALID_CHANNEL_MODE},
    //A2DP/SNK/AVP/BI-12-C
    {"AVDTP_reject_INVALID_SUBBANDS", btif_pts_reject_INVALID_SUBBANDS},
    //A2DP/SNK/AVP/BI-13-C
    {"AVDTP_reject_INVALID_ALLOCATION_METHOD", btif_pts_reject_INVALID_ALLOCATION_METHOD},
    //A2DP/SNK/AVP/BI-14-C
    {"AVDTP_reject_INVALID_MINIMUM_BITPOOL_VALUE", btif_pts_reject_INVALID_MINIMUM_BITPOOL_VALUE},
    //A2DP/SNK/AVP/BI-15-C
    {"AVDTP_reject_INVALID_MAXIMUM_BITPOOL_VALUE", btif_pts_reject_INVALID_MAXIMUM_BITPOOL_VALUE},
    //A2DP/SNK/AVP/BI-16-C
    {"AVDTP_reject_INVALID_BLOCK_LENGTH", btif_pts_reject_INVALID_BLOCK_LENGTH},
    //A2DP/SNK/AVP/BI-17-C
    {"AVDTP_reject_INVALID_CP_TYPE", btif_pts_reject_INVALID_CP_TYPE},
    //A2DP/SNK/AVP/BI-18-C
    {"AVDTP_reject_INVALID_CP_FORMAT", btif_pts_reject_INVALID_CP_FORMAT},
    //A2DP/SNK/AVP/BI-20-C
    {"AVDTP_reject_NOT_SUPPORTED_CODEC_TYPE", btif_pts_reject_NOT_SUPPORTED_CODEC_TYPE},

    // SLT TEST
    // CPU freq
    {"slt_test_cpu_req_clkout_enable",  app_ibrt_slt_test_mcu_freq_clkout_enable},
    {"slt_test_cpu_req_clkout_disable", app_ibrt_slt_test_mcu_freq_clkout_disable},
    {"slt_test_cpu_req_208m",           app_ibrt_slt_test_mcu_req_208m},
    {"slt_test_cpu_req_104m",           app_ibrt_slt_test_mcu_req_104m},
    {"slt_test_cpu_req_52m",            app_ibrt_slt_test_mcu_req_52m},
    {"slt_test_cpu_req_26m",            app_ibrt_slt_test_mcu_req_26m},
    {"slt_test_cpu_req_32k",            app_ibrt_slt_test_mcu_req_32k},
};

#ifdef SPA_AUDIO_SEC
#include "tz_audio_process.h"
#include "ree_audio_process.h"

void app_tz_audio_process_demo_func_enable_disable(const char* cmd, uint32_t cmd_len)
{
    int enable_disable = 0;

    if (1 != sscanf(cmd, "%d", &enable_disable))
    {
        LOG_I("%s invalid param %s", __func__, cmd);
        return;
    }

    LOG_I("%s len %d param[0]: 0x%02x 0x%02x",__func__,cmd_len,cmd[0],enable_disable);
    tz_audio_process_demo_func_enable_disable(enable_disable);
}

void app_ree_audio_process_demo_func_enable_disable(const char* cmd, uint32_t cmd_len)
{
    int enable_disable = 0;

    if (1 != sscanf(cmd, "%d", &enable_disable))
    {
        LOG_I("%s invalid param %s", __func__, cmd);
        return;
    }

    LOG_I("%s len %d param[0]: 0x%02x 0x%02x",__func__,cmd_len,cmd[0],enable_disable);
    ree_audio_test_demo_enable_disable(enable_disable);
}

const app_bt_cmd_handle_with_parm_t app_tz_test_cmd_handle_p[]=
{
    {"tz_demo_func_enable_disable",app_tz_audio_process_demo_func_enable_disable},
    {"ree_demo_func_enable_disable",app_ree_audio_process_demo_func_enable_disable},
};

void app_tz_add_test_cmd_table(void)
{
    app_bt_cmd_add_test_table_with_param(app_tz_test_cmd_handle_p,
            ARRAY_SIZE(app_tz_test_cmd_handle_p));
}
#endif

/*
 * example:best_test:mobile1_disconnect_test=08fa79a9931a
 * cmd_title: best_test
 * cmd_name: mobile1_disconnect_test
 * cmd_para: 08fa79a9931a
 */
const app_uart_handle_t_p app_ibrt_uart_test_handle_p[]=
{
    {"get_a2dp_state_test",         app_ibrt_get_a2dp_state_test},
    {"get_avrcp_state_test",        app_ibrt_get_avrcp_state_test},
    {"get_hfp_state_test",          app_ibrt_get_hfp_state_test},
    {"get_call_status",             app_ibrt_get_call_status_test},
    {"tws_get_role_test",           app_ibrt_tws_role_get_request_test_with_parameter},
    {"test_master_tws_pairing",     test_master_tws_pairing},
    {"test_slave_tws_pairing",      test_slave_tws_pairing},
    {"master_update_tws_pair_info_test_func",master_update_tws_pair_info_test_func},
    {"slave_update_tws_pair_info_test_func",slave_update_tws_pair_info_test_func},
    {"m_write_bt_local_addr_test",  m_write_bt_local_addr_test},
    {"s_write_bt_local_addr_test",  s_write_bt_local_addr_test},
    {"m_write_ble_local_addr_test", m_write_ble_local_addr_test},
    {"s_write_ble_local_addr_test", s_write_ble_local_addr_test},
    {"start_ibrt1_test",            app_ibrt_start_ibrt_test_with_addr},
    {"stop_ibrt1_test",             app_ibrt_stop_ibrt_test_with_addr},
    {"mobile1_connect_test",        app_ibrt_mobile_connect_test},
    {"mobile1_disconnect_test",     app_ibrt_mobile_disconnect_test},
    {"tws_role_swap_test",          app_ibrt_tws_role_switch_test_with_addr},
};

void app_ibrt_ui_v2_add_test_cmd_table(void)
{
    app_bt_cmd_add_test_table(app_ibrt_v2_uart_test_handle, ARRAY_SIZE(app_ibrt_v2_uart_test_handle));
}

app_uart_test_function_handle_with_param app_ibrt_ui_find_uart_handle_with_param(unsigned char* buf)
{
    app_uart_test_function_handle_with_param p = NULL;
    for(uint8_t i = 0; i < ARRAY_SIZE(app_ibrt_uart_test_handle_p); i++)
    {
        if(strncmp((char*)buf, app_ibrt_uart_test_handle_p[i].string, strlen(app_ibrt_uart_test_handle_p[i].string))==0 ||
           strstr(app_ibrt_uart_test_handle_p[i].string, (char*)buf))
        {
            p = app_ibrt_uart_test_handle_p[i].function;
            break;
        }
    }
    return p;
}

int app_ibrt_raw_ui_test_cmd_handler_with_param(unsigned char *buf, unsigned char *param, unsigned int length)
{
    //LOG_I("%s execute command: %s\n",__func__, buf);
    //LOG_I("parameter: %s, len: %d \n", param, strlen((char *)param));
    //LOG_I("len: %d \n", length);
    int ret = 0;
    if (buf[length-2] == 0x0d ||
        buf[length-2] == 0x0a)
    {
        buf[length-2] = 0;
    }
    app_uart_test_function_handle_with_param handl_function = app_ibrt_ui_find_uart_handle_with_param(buf);
    if(handl_function)
    {
        handl_function(param, length);
    }
    else
    {
        ret = -1;
        LOG_I("can not find handle function");
    }
    return ret;
}
#endif

#ifdef IBRT_UI_V2
void app_ibrt_mgr_pairing_mode_test(void)
{
#if !defined(FREE_TWS_PAIRING_ENABLED)
    app_ibrt_if_init_open_box_state_for_evb();
    app_ibrt_if_event_entry(APP_UI_EV_TWS_PAIRING);
    LOG_I("   jay [ %s ] ", __func__);
#else
    app_ibrt_if_init_open_box_state_for_evb();
    uint8_t btAddr[6];
    factory_section_original_btaddr_get(btAddr);
    LOG_I("   jay [ %s ] ", __func__);
    btAddr[0] ^= 0x01;
    // the bit 0 of the bt address's first byte clarifies the tws addresses
    // master if it's 1, slave if it's 0
    app_ibrt_if_start_tws_pairing(app_tws_ibrt_get_bt_ctrl_ctx()->nv_role,
        btAddr);
#endif
}

#define POWER_ON_TWS_PAIRING_DELAY_MS   2000

osTimerId   power_on_pairing_delay_timer_id = NULL;
static void power_on_pairing_delay_timer_handler(void const *para)
{
#if BLE_AUDIO_ENABLED
        if (ble_audio_is_ux_mobile()) {
            return;
        }
#endif
    app_ibrt_mgr_pairing_mode_test();
}

void app_ibrt_start_power_on_freeman_pairing(void)
{
    LOG_I("%s [Enter pairing jay] ", __func__);
    app_ibrt_if_enter_freeman_pairing();
}

osTimerDef (power_on_pairing_delay_timer, power_on_pairing_delay_timer_handler);
void app_ibrt_start_power_on_tws_pairing(void)
{
    if (NULL == power_on_pairing_delay_timer_id)
    {
        power_on_pairing_delay_timer_id =
            osTimerCreate(osTimer(power_on_pairing_delay_timer), osTimerOnce, NULL);
    }

    osTimerStart(power_on_pairing_delay_timer_id, POWER_ON_TWS_PAIRING_DELAY_MS);
    LOG_I("%s",__func__);
}
#endif

void app_tws_ibrt_test_key_io_event(APP_KEY_STATUS *status, void *param)
{
    LOG_I("app_tws_ibrt_test_key_io_event");
    LOG_I("%s 000%d,%d",__func__, status->code, status->event);
#ifdef CUSTOMER_APP_BOAT
    LOG_I("app_ibrt_get_raw_ui is %d",app_ibrt_if_is_right_side());
    app_tota_general_button_event_handler(status, app_ibrt_if_is_right_side());
#endif
#if 0//defined(IBRT_UI_V2)
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
            if (status->code== APP_KEY_CODE_FN1)
            {
                app_ibrt_if_event_entry(APP_UI_EV_CASE_OPEN);
            }
            else if (status->code== APP_KEY_CODE_FN2)
            {
                app_ibrt_if_event_entry(APP_UI_EV_UNDOCK);
            }
            else
            {
                app_ibrt_if_event_entry(APP_UI_EV_WEAR_UP);
            }
            break;

        case APP_KEY_EVENT_DOUBLECLICK:
            if (status->code== APP_KEY_CODE_FN1)
            {
                app_ibrt_if_event_entry(APP_UI_EV_CASE_CLOSE);
            }
            else if (status->code== APP_KEY_CODE_FN2)
            {
                app_ibrt_if_event_entry(APP_UI_EV_DOCK);
            }
            else
            {
                app_ibrt_if_event_entry(APP_UI_EV_WEAR_DOWN);
            }
            break;

        case APP_KEY_EVENT_LONGPRESS:
            if (status->code== APP_KEY_CODE_FN1)
            {
#ifdef APP_BT_SPEAKER
                app_speaker_setup_bt_stereo();
#endif
            }
            else if (status->code== APP_KEY_CODE_FN2)
            {
#ifdef APP_BT_SPEAKER
                app_speaker_exit_bt_stereo();
#endif
            }
            else
            {
            }
            break;

        case APP_KEY_EVENT_TRIPLECLICK:
            break;

        case HAL_KEY_EVENT_LONGLONGPRESS:
            break;

        case APP_KEY_EVENT_ULTRACLICK:
            break;

        case APP_KEY_EVENT_RAMPAGECLICK:
            break;
    }
#endif
}

void app_ibrt_raw_ui_test_key(APP_KEY_STATUS *status, void *param)
{
    uint8_t shutdown_key = HAL_KEY_EVENT_LONGLONGPRESS;
    uint8_t device_id = app_bt_audio_get_device_for_user_action();
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    LOG_I("%s %d,%d curr_device %x", __func__, status->code, status->event, device_id);

    if (IBRT_SLAVE == app_tws_get_ibrt_role(&curr_device->remote) && status->event != shutdown_key)
    {
        app_ibrt_if_keyboard_notify_v2(&curr_device->remote, status, param);
    }
    else
    {
#ifdef IBRT_SEARCH_UI
        app_ibrt_search_ui_handle_key_v2(&curr_device->remote, status, param);
#else
        app_ibrt_normal_ui_handle_key_v2(&curr_device->remote, status, param); //jay
#endif
    }
}

#ifdef CMT_008_UI
extern void app_anc_key(APP_KEY_STATUS *status, void *param);
//extern void app_voice_assistant_key(APP_KEY_STATUS *status, void *param);
//extern void bt_key_handle_siri_key(enum APP_KEY_EVENT_T event);
extern void app_ibrt_ui_factory_reset_test(void);
extern void app_factory_reset(void);
extern void bt_key_handle_mute_key(void);

static void app_mute_key_handle(APP_KEY_STATUS *status, void *param)
{
    TRACE(2,"%s event:%d", __func__, status->event);
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
            //bt_key_handle_siri_key(APP_KEY_EVENT_NONE);
            bt_key_handle_mute_key();
            user_custom_reset_standby_time();
            break;

        default:
            break;
    }
}

static void app_factory_reset_handle(APP_KEY_STATUS *status, void *param)
{
    TRACE(2,"%s event:%d", __func__, status->event);

    user_custom_reset_standby_time();

    switch(status->event)
    {
        case APP_KEY_EVENT_LONGPRESS:
            app_factory_reset();
            //app_ibrt_ui_factory_reset_test();
            break;

        default:
            break;
    }
}

#endif/*CMT_008_UI*/

#if 0 /*add by jay, to test MC*/
extern void app_dfu_key_handler(APP_KEY_STATUS *status, void *param);
#endif /*add by jay, to test MC*/


//jay
const APP_KEY_HANDLE  app_ibrt_ui_v2_test_key_cfg[] =
{
#ifdef CMT_008_UI /*add by jay*/
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGPRESS},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
    {{APP_KEY_CODE_ANC,APP_KEY_EVENT_CLICK},"bt anc key", app_anc_key, NULL},
    {{APP_KEY_CODE_VOICE_ASSISTANT,APP_KEY_EVENT_CLICK}, "google assistant key", app_mute_key_handle, NULL},
    {{APP_KEY_CODE_VOICE_ASSISTANT|APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS}, "factory reset key", app_factory_reset_handle, NULL},
    //{{APP_KEY_CODE_VOICE_ASSISTANT|APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_dfu_key_handler, NULL}, //to test MC
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},//to test LineIn
#else /* CMT_008_UI */
#if defined(__AI_VOICE__) || defined(BISTO_ENABLED)
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_FIRST_DOWN}, "google assistant key", app_ai_manager_key_event_handle, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_UP}, "google assistant key", app_ai_manager_key_event_handle, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_LONGPRESS}, "google assistant key", app_ai_manager_key_event_handle, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_CLICK}, "google assistant key", app_ai_manager_key_event_handle, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_DOUBLECLICK}, "google assistant key", app_ai_manager_key_event_handle, NULL},
#endif
#if defined(__BT_ANC_KEY__)&&defined(ANC_APP)
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt anc key",app_anc_key, NULL},
#else
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
#endif
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGPRESS},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_ULTRACLICK},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"app_ibrt_ui_test_key", app_ibrt_raw_ui_test_key, NULL},

    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_CLICK},"app_ibrt_service_test_key", app_tws_ibrt_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_service_test_key", app_tws_ibrt_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"app_ibrt_service_test_key", app_tws_ibrt_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_CLICK},"app_ibrt_service_test_key", app_tws_ibrt_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_service_test_key", app_tws_ibrt_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_LONGPRESS},"app_ibrt_service_test_key", app_tws_ibrt_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_CLICK},"app_ibrt_service_test_key", app_tws_ibrt_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_service_test_key", app_tws_ibrt_test_key_io_event, NULL},
#endif /* CMT_008_UI */
};

void app_tws_ibrt_raw_ui_test_key_init(void)
{
#ifdef APP_KEY_ENABLE
    LOG_I("app_tws_ibrt_raw_ui_test_key_init");
    app_key_handle_clear();
    for (uint8_t i=0; i<ARRAY_SIZE(app_ibrt_ui_v2_test_key_cfg); i++)
    {
        app_key_handle_registration(&app_ibrt_ui_v2_test_key_cfg[i]);
    }
#endif

#ifdef CUSTOMER_APP_BOAT
    app_tota_general_button_event_init();
#endif
}

#endif
