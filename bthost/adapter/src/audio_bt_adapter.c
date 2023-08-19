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
#ifdef BT_BUILD_WITH_CUSTOMER_HOST

uint8_t bt_sbc_player_get_codec_type(void)
{
    return 0; // sbc
}

uint8_t bt_sbc_player_get_sample_bit(void)
{
    return 16;
}

uint8_t app_bt_get_curr_a2dp_sample_rate(uint8_t curr_a2dp_device_id)
{
    return 0x20; // A2D_SBC_IE_SAMP_FREQ_44;
}

uint16_t a2dp_Get_curr_a2dp_conhdl(void)
{
    return 0xFFFF;
}

bool app_bt_is_curr_a2dp_streaming(uint8_t curr_a2dp_device_id)
{
    return false;
}

uint8_t bt_sco_player_get_codec_type(uint8_t curr_sco_device_id)
{
    return BT_HFP_SCO_CODEC_CVSD;
}

uint16_t app_bt_get_curr_sco_hci_handle(uint8_t curr_sco_device_id)
{
    return 0xFFFF;
}

uint8_t btapp_hfp_incoming_calls(void)
{
    return 0;
}

bool btapp_hfp_is_call_active(void)
{
    return false;
}

uint16_t app_bt_get_conhandle_by_device_id(uint8_t device_id)
{
    return 0xFFFF;
}

uint8_t app_bt_audio_count_straming_mobile_links(void)
{
    return 0;
}

uint8_t app_bt_audio_count_streaming_a2dp(void)
{
    return 0;
}

uint8_t btif_me_get_activeCons(void)
{
    return 0;
}

uint8_t app_bt_audio_get_curr_a2dp_device(void)
{
    return 0; // device 0
}

#endif /* BT_BUILD_WITH_CUSTOMER_HOST */
