/**
 * @file aob_cis_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2022-04-18
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */

/*****************************header include********************************/
#include "app_gaf_define.h"
#include "aob_cis_api.h"
#include "hal_trace.h"
#include "ble_app_dbg.h"
#include "plat_types.h"
#include "heap_api.h"

#include "app_gaf_custom_api.h"
#include "ble_audio_earphone_info.h"
#include "ble_audio_core_evt.h"
#include "ble_audio_core.h"
#include "aob_bis_api.h"

/****************************for server(earbud)*****************************/

/*********************external function declaration*************************/

/************************private macro defination***************************/
#define INVALID_PDU_SIZE                  (0xFF)
#define APP_BAP_UC_ASE_ID_MIN             (1)
#define INVALID_ASE_LID                   (0xFF)
/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/
static BLE_AUD_CORE_EVT_CB_T *p_cis_cb = NULL;
static aob_cis_pdu_size_info_t cis_pdu_size_info[MOBILE_CONNECTION_MAX] = {0};

static void aob_cis_established_cb(app_gaf_uc_srv_cis_state_ind_t *ascs_cis_established)
{
    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++)
    {
        if (cis_pdu_size_info[i].con_lid == GAF_INVALID_LID)
        {
            cis_pdu_size_info[i].con_lid = ascs_cis_established->con_lid;
            cis_pdu_size_info[i].max_pdu_m2s = ascs_cis_established->cis_config.max_pdu_m2s;
            cis_pdu_size_info[i].max_pdu_s2m = ascs_cis_established->cis_config.max_pdu_s2m;
            break;
        }
    }

    if (NULL != p_cis_cb->ble_cis_established) {
        p_cis_cb->ble_cis_established((AOB_UC_SRV_CIS_INFO_T *)ascs_cis_established);
    }
}

static void aob_cis_rejected_cb(uint16_t con_hdl, uint8_t error)
{
    if (NULL != p_cis_cb->ble_cis_rejected) {
        p_cis_cb->ble_cis_rejected(con_hdl, error);
    }
}

static void aob_cis_terminated_cb(uint8_t cig_id, uint8_t group_lid, uint8_t stream_lid, uint8_t reason)
{
    if (NULL != p_cis_cb->ble_cig_terminated) {
        p_cis_cb->ble_cig_terminated(cig_id, group_lid, stream_lid, reason);
    }
}

static void aob_ase_ntf_value_cb(uint8_t opcode, uint8_t nb_ases, uint8_t ase_lid, uint8_t rsp_code, uint8_t reason)
{
#ifdef CFG_BAP_BC
    app_bap_bc_scan_env_t *aob_bap_scan_env = app_bap_bc_scan_get_scan_env();

    if (opcode == APP_BAP_UC_OPCODE_ENABLE) {
        if (aob_bap_scan_env->scan_state == APP_BAP_BC_SCAN_STATE_STREAMING) {
            TRACE(0, "stop bis before cis start, grp_lid = %d", ble_audio_earphone_info_get_bis_grp_lid());
            aob_bis_sink_disable(ble_audio_earphone_info_get_bis_grp_lid());
            TRACE(0, "stop pa after stop bis, pa_lid = %d", ble_audio_earphone_info_get_bis_pa_lid());
            app_bap_bc_scan_pa_terminate(ble_audio_earphone_info_get_bis_pa_lid());
        }
    }
#endif

    if (NULL != p_cis_cb->ble_ase_ntf_value_cb) {
        p_cis_cb->ble_ase_ntf_value_cb(opcode, nb_ases, ase_lid, rsp_code, reason);
    }
}

static void aob_cis_disconnected_cb(app_gaf_uc_srv_cis_state_ind_t *ascs_cis_disconnected)
{   
    if (ascs_cis_disconnected->con_lid == GAP_INVALID_CONIDX)
    {
        LOG_W("%s ASE [QOS -> CODEC/IDLE] cause CIS state change will not report", __func__);
        return;
    }

    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++)
    {
        if (ascs_cis_disconnected->con_lid == cis_pdu_size_info[i].con_lid)
        {
            LOG_I("%s con_lid:%d ", __func__, ascs_cis_disconnected->con_lid);
            cis_pdu_size_info[i].con_lid = GAF_INVALID_LID;
            cis_pdu_size_info[i].max_pdu_m2s = INVALID_PDU_SIZE;
            cis_pdu_size_info[i].max_pdu_s2m = INVALID_PDU_SIZE;
            break;
        }
    }

    if (ascs_cis_disconnected->ase_lid_sink != GAF_INVALID_LID)
    {
        if (NULL != p_cis_cb->ble_cis_disconnected) {
            p_cis_cb->ble_cis_disconnected(ascs_cis_disconnected->cig_id, ascs_cis_disconnected->cis_id,
                                                ascs_cis_disconnected->status, ascs_cis_disconnected->reason);
        }
    }
}

static cis_conn_evt_handler_t cis_conn_event_cb = {
    .cis_established_cb             = aob_cis_established_cb,
    .cis_rejected_cb                = aob_cis_rejected_cb,
    .cis_terminated_cb              = aob_cis_terminated_cb,
    .ase_ntf_value_cb               = aob_ase_ntf_value_cb,
    .cis_disconnected_cb            = aob_cis_disconnected_cb,
};

aob_cis_pdu_size_info_t* aob_cis_get_pdu_size_info(uint8_t con_lid)
{
    aob_cis_pdu_size_info_t *p_info = &cis_pdu_size_info[0];

    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++)
    {
        if (con_lid == cis_pdu_size_info[i].con_lid)
        {
            LOG_I("%s con_lid:%d  max_pdu_m2s: %d max_pdu_s2m: %d", __func__, cis_pdu_size_info[i].con_lid,
            cis_pdu_size_info[i].max_pdu_m2s, cis_pdu_size_info[i].max_pdu_s2m);
            p_info = &cis_pdu_size_info[i];
            break;
        }
    }

    return p_info;
}

uint8_t aob_get_ase_lid_by_ase_id(uint8_t con_lid, uint8_t ase_id)
{
    AOB_MEDIA_INFO_T *media_info = ble_audio_earphone_info_get_media_info(con_lid);
    uint8_t ase_instance_idx = ase_id - APP_BAP_UC_ASE_ID_MIN;

    if (NULL == media_info)
    {

        return INVALID_ASE_LID;
    }

    for(uint8_t i = 0; i < APP_BAP_DFT_ASCS_NB_ASE_CHAR; i++)
    {
        if ((con_lid == media_info->aob_media_ase_info[i]->con_lid)
            && (ase_instance_idx == media_info->aob_media_ase_info[i]->ase_instance_idx))
        {
            return media_info->aob_media_ase_info[i]->ase_lid;
        }
    }

    return INVALID_ASE_LID;
}

void aob_cis_api_init(void)
{
    p_cis_cb = ble_audio_get_evt_cb();
    aob_mgr_cis_conn_evt_handler_t_register(&cis_conn_event_cb);

    memset(&cis_pdu_size_info, 0, sizeof(aob_cis_pdu_size_info_t) * MOBILE_CONNECTION_MAX);

    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++)
    {
        cis_pdu_size_info[i].con_lid = GAF_INVALID_LID;
        cis_pdu_size_info[i].max_pdu_m2s = INVALID_PDU_SIZE;
        cis_pdu_size_info[i].max_pdu_s2m = INVALID_PDU_SIZE;
    }
}

#ifdef AOB_MOBILE_ENABLED
static aob_cis_pdu_size_info_t mobile_cis_pdu_size_info[MOBILE_CONNECTION_MAX] = {0};

static void aob_mobile_cis_estab_cb(app_gaf_uc_cli_cis_state_ind_t *ascc_cis_established)
{
    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++)
    {
        if (mobile_cis_pdu_size_info[i].con_lid == GAF_INVALID_LID)
        {
            LOG_I("%s con_lid:%d  max_pdu_m2s: %d max_pdu_s2m: %d", __func__, mobile_cis_pdu_size_info[i].con_lid,
            mobile_cis_pdu_size_info[i].max_pdu_m2s, mobile_cis_pdu_size_info[i].max_pdu_s2m);
            mobile_cis_pdu_size_info[i].con_lid = ascc_cis_established->con_lid;
            mobile_cis_pdu_size_info[i].max_pdu_m2s = ascc_cis_established->cis_config.max_pdu_m2s;
            mobile_cis_pdu_size_info[i].max_pdu_s2m = ascc_cis_established->cis_config.max_pdu_s2m;
            break;
        }
    }
}

aob_cis_pdu_size_info_t* aob_cis_mobile_get_pdu_size_info(uint8_t con_lid)
{
    aob_cis_pdu_size_info_t *p_info = &mobile_cis_pdu_size_info[0];

    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++)
    {
        if (con_lid == mobile_cis_pdu_size_info[i].con_lid)
        {
            LOG_I("%s con_lid:%d  max_pdu_m2s: %d max_pdu_s2m: %d", __func__, mobile_cis_pdu_size_info[i].con_lid,
            mobile_cis_pdu_size_info[i].max_pdu_m2s, mobile_cis_pdu_size_info[i].max_pdu_s2m);
            p_info = &mobile_cis_pdu_size_info[i];
            break;
        }
    }

    return p_info;
}

static void aob_mobile_cis_discon_cb(app_gaf_uc_cli_cis_state_ind_t *ascc_cis_disconnected)
{
    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++)
    {
        if (ascc_cis_disconnected->con_lid == mobile_cis_pdu_size_info[i].con_lid)
        {
            LOG_I("%s con_lid:%d ", __func__, ascc_cis_disconnected->con_lid);
            mobile_cis_pdu_size_info[i].con_lid = GAF_INVALID_LID;
            mobile_cis_pdu_size_info[i].max_pdu_m2s = INVALID_PDU_SIZE;
            mobile_cis_pdu_size_info[i].max_pdu_s2m = INVALID_PDU_SIZE;
            break;
        }
    }
}

static cis_mobile_conn_evt_handler_t mobile_cis_conn_event_cb = {
    .mobile_cis_estab_cb             = aob_mobile_cis_estab_cb,
    .mobile_cis_discon_cb            = aob_mobile_cis_discon_cb,
};

void aob_cis_mobile_api_init(void)
{
    aob_mgr_mobile_cis_conn_evt_handler_register(&mobile_cis_conn_event_cb);

    memset(&mobile_cis_pdu_size_info, 0, sizeof(aob_cis_pdu_size_info_t) * MOBILE_CONNECTION_MAX);

    for (uint8_t i = 0; i < MOBILE_CONNECTION_MAX; i++)
    {
        mobile_cis_pdu_size_info[i].con_lid = GAF_INVALID_LID;
        mobile_cis_pdu_size_info[i].max_pdu_m2s = INVALID_PDU_SIZE;
        mobile_cis_pdu_size_info[i].max_pdu_s2m = INVALID_PDU_SIZE;
    }
}
#endif