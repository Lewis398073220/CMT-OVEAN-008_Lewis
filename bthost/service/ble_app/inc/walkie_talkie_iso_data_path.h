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
#ifndef __WALKIE_TALKIE_ISO_DATA_PATH_H__
#define __WALKIE_TALKIE_ISO_DATA_PATH_H__

#ifdef BLE_WALKIE_TALKIE
#ifdef BLE_ISO_ENABLED

typedef struct
{
    /// Packet Sequence Number
    uint16_t        pkt_seq_nb;
    /// length of the ISO SDU (in bytes)
    uint16_t        sdu_length;
    /// Reception status (@see enum hci_iso_pkt_stat_flag)
    uint8_t         status;
    /// SDU
    uint8_t         *sdu;
} wt_dp_iso_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

bool walkie_iso_src_start_streaming(uint16_t con_hdl,uint32_t sdu_interval, uint32_t trans_latency, uint16_t max_sdu);
void walkie_iso_src_stop_streaming(uint16_t con_hdl);
void walkie_iso_src_send_data(uint16_t con_hdl,uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t ref_time);

uint8_t walkie_iso_sink_rx_stream_start(uint16_t bis_hdl, uint16_t frame_octet);
void walkie_iso_sink_rx_stream_stop(uint16_t bis_hdl);
void *walkie_iso_dp_itf_get_rx_data(uint16_t conhdl, wt_dp_iso_buffer_t *iso_buffer);
void walkie_iso_dp_itf_rx_data_done(uint16_t conhdl, uint16_t sdu_len, uint32_t ref_time, uint8_t* p_buf);


#ifdef __cplusplus
    }
#endif

#endif  //BLE_ISO_ENABLED
#endif  //BLE_WALKIE_TALKIE

#endif /* __WALKIE_TALKIE_ISO_DATA_PATH_H__ */

