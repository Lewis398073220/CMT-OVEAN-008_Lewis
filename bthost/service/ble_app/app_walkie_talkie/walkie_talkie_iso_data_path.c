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
#include "hal_trace.h"
#include "bt_drv_reg_op.h"
#include "co_hci.h"
#include "hl_hci.h"
#include "data_path.h"
#include "iap.h"
#include "isoohci_int.h"
#include "app_bap_data_path_itf.h"
#include "walkie_talkie_iso_data_path.h"

#ifdef BLE_WALKIE_TALKIE
#ifdef BLE_ISO_ENABLED_X

struct data_path_itf* wt_iso_rx_dp_itf = NULL;
struct data_path_itf* wt_iso_tx_dp_itf = NULL;
uint32_t wt_iso_rx_sdu_cnt = 0;

/****************************function defination****************************/
static void walkie_iso_set_rx_dp_itf(struct data_path_itf *itf)
{
    TRACE(3,"%s old %p new %p", __func__, wt_iso_rx_dp_itf, itf);
    if (wt_iso_rx_dp_itf != itf)
    {
        wt_iso_rx_dp_itf = itf;
    }
    wt_iso_rx_sdu_cnt = 0;
}

struct data_path_itf *walkie_iso_get_rx_dp_itf(void)
{
    return wt_iso_rx_dp_itf;
}

// sink
void *walkie_iso_dp_itf_get_rx_data(uint16_t conhdl, wt_dp_iso_buffer_t *iso_buffer)
{
    gaf_media_data_t  *p_sdu_buf = NULL;
    uint32_t ref_time;
    uint16_t sdu_len;

    if (BLE_ISOHDL_TO_ACTID(conhdl) >= BLE_ACTIVITY_MAX)
    {
        TRACE(1,"error channel %d", BLE_ISOHDL_TO_ACTID(conhdl));
        return NULL;
    }

    if (wt_iso_rx_dp_itf != NULL)
    {
        if (wt_iso_rx_dp_itf->cb_sdu_get != NULL)
        {
            p_sdu_buf = (gaf_media_data_t *)wt_iso_rx_dp_itf->cb_sdu_get(conhdl, 0, &ref_time, &sdu_len);
        }

        return p_sdu_buf;
    }

    return NULL;
}

void walkie_iso_dp_itf_rx_data_done(uint16_t conhdl, uint16_t sdu_len, uint32_t ref_time, uint8_t* p_buf)
{
    if (wt_iso_rx_dp_itf->cb_sdu_done != NULL)
    {
        p_buf = (p_buf - OFFSETOF(isoohci_buffer_t,sdu));
        wt_iso_rx_dp_itf->cb_sdu_done(conhdl, sdu_len, ref_time, p_buf, 0);
        wt_iso_rx_sdu_cnt++;
    }
}

uint8_t walkie_iso_sink_rx_stream_start(uint16_t bis_hdl, uint16_t frame_octet)
{
    uint8_t status = CO_ERROR_NO_ERROR;
    const struct data_path_itf *_rx_dp_itf = NULL;

    TRACE(1,"W-T-ISO:sink start rx stream, bis_hdl = %u", bis_hdl);
    _rx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_RX);
    if (_rx_dp_itf == NULL)
    {
        TRACE(0,"W-T-ISO: sink get rx data path interface failed");
        _rx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_RX);
    }
    walkie_iso_set_rx_dp_itf((struct data_path_itf *)_rx_dp_itf);

    if (NULL != _rx_dp_itf->cb_start)
    {
        status = _rx_dp_itf->cb_start(bis_hdl, WALKIE_BC_SDU_INTERVAL_US, 2 * WALKIE_BC_MAX_SDU_SIZE, (frame_octet * 2));
        if (status != CO_ERROR_NO_ERROR)
        {
            TRACE(1,"W-T-G: bc sink start rx stream failed, status = %d", status);
        }
    }

    return status;
}

void walkie_iso_sink_rx_stream_stop(uint16_t bis_hdl)
{
    TRACE(2,"W-T-G: bc sink stop rx stream, bis_hdl = %d _rx_dp_itf %p", bis_hdl,wt_iso_rx_dp_itf);
    if (wt_iso_rx_dp_itf && NULL != wt_iso_rx_dp_itf->cb_stop)
    {
        wt_iso_rx_dp_itf->cb_stop(bis_hdl, 0);
    }
}

// src
static void walkie_iso_set_tx_dp_itf(struct data_path_itf *itf)
{
    TRACE(2,"%s old %p new %p", __func__, wt_iso_tx_dp_itf, itf);
    if (wt_iso_tx_dp_itf != itf)
    {
        wt_iso_tx_dp_itf = itf;
    }
}

struct data_path_itf *walkie_iso_get_tx_dp_itf(void)
{
    return wt_iso_tx_dp_itf;
}

bool walkie_iso_src_start_streaming(uint16_t con_hdl,uint32_t sdu_interval, uint32_t trans_latency, uint16_t max_sdu)
{
    const struct data_path_itf *iso_tx_dp_itf = NULL;
    uint8_t status = CO_ERROR_NO_ERROR;

    iso_tx_dp_itf = data_path_itf_get(ISO_DP_ISOOHCI, ISO_SEL_TX);
    if (!iso_tx_dp_itf)
    {
       TRACE(0,"W-T-ISO:src get tx data path interface failed");
       iso_tx_dp_itf = data_path_itf_get(ISO_DP_DISABLE, ISO_SEL_TX);
    }
    walkie_iso_set_tx_dp_itf((struct data_path_itf *)iso_tx_dp_itf);

    TRACE(0, "W-T-ISO: bis_send_iso_start,hd = 0x%x", con_hdl);
    if (iso_tx_dp_itf && (NULL != iso_tx_dp_itf->cb_start))
    {
        status = iso_tx_dp_itf->cb_start(con_hdl, sdu_interval, trans_latency, max_sdu);
        if (status == CO_ERROR_NO_ERROR)
        {
            TRACE(0,"W-T-ISO: src start tx stream success");
            return true;
        }
    }

    return false;
}

void walkie_iso_src_stop_streaming(uint16_t con_hdl)
{
    if (wt_iso_tx_dp_itf && (NULL != wt_iso_tx_dp_itf->cb_stop))
    {
        wt_iso_tx_dp_itf->cb_stop(con_hdl, 0);
    }
}

void walkie_iso_src_send_data(uint16_t con_hdl,uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t ref_time)
{
    app_bap_dp_itf_send_data_directly(con_hdl, seq_num, payload, payload_len, ref_time);
}

#endif  //BLE_ISO_ENABLED
#endif  //BLE_WALKIE_TALKIE

