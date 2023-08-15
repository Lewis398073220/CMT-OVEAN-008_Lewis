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

#include "ke_task.h"
#include "ke_msg.h"
#include "gapm_le_msg.h"
#include "gapc_msg.h"
#include "bt_drv_reg_op.h"
#include "co_hci.h"
#include "hl_hci.h"
#include "data_path.h"
#include "iap.h"
#include "walkie_talkie_iso_data_path.h"
#include "isoohci_int.h"
#include "walkie_talkie_ble_gapm_cmd.h"

void walkie_stop_activity(uint8_t actv_idx)
{
    struct gapm_activity_stop_cmd *cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_STOP_CMD,
                                                  TASK_GAPM, TASK_BLE_WALKIE,
                                                  gapm_activity_stop_cmd);
    if (!cmd)
    {
        return;
    }

    // Fill the allocated kernel message
    cmd->operation = GAPM_STOP_ACTIVITY;
    cmd->actv_idx = actv_idx;

    // Send the message
    ke_msg_send(cmd);
}

void walkie_delete_activity(uint8_t actv_idx)
{
    // Prepare the GAPM_ACTIVITY_STOP_CMD message
    struct gapm_activity_delete_cmd *cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_DELETE_CMD,
                                                      TASK_GAPM, TASK_BLE_WALKIE,
                                                      gapm_activity_delete_cmd);
    if (!cmd)
    {
        return;
    }

    // Fill the allocated kernel message
    cmd->operation = GAPM_DELETE_ACTIVITY;
    cmd->actv_idx = actv_idx;

    // Send the message
    ke_msg_send(cmd);
}

void walkie_adv_creat(uint8_t *adv_para)
{
    struct gapm_activity_create_adv_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD,
                                                    TASK_GAPM, TASK_BLE_WALKIE,
                                                    gapm_activity_create_adv_cmd);

    if (!p_cmd)
    {
       return;
    }

    memcpy(p_cmd, adv_para, sizeof(struct gapm_activity_create_adv_cmd));
    // Set operation code
    p_cmd->operation = GAPM_CREATE_ADV_ACTIVITY;

    // Send the message
    ke_msg_send(p_cmd);
}

void walkie_adv_enable(uint16_t duration, uint8_t max_adv_evt, uint8_t actv_idx)
{
    // Prepare the GAPM_ACTIVITY_START_CMD message
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_ACTIVITY_START_CMD,
                                                    TASK_GAPM, TASK_BLE_WALKIE,
                                                    gapm_activity_start_cmd,
                                                    sizeof(gapm_adv_param_t));
    if (!p_cmd)
    {
       return;
    }

    gapm_adv_param_t *adv_add_param = (gapm_adv_param_t *)(p_cmd->u_param);
    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = actv_idx;
    adv_add_param->duration = duration;
    adv_add_param->max_adv_evt = max_adv_evt;

    // Send the message
    ke_msg_send(p_cmd);
}


void walkie_set_adv_data_cmd(uint8_t operation, uint8_t actv_idx,
                                      uint8_t *adv_data, uint8_t data_len)
{
    // Prepare the GAPM_SET_ADV_DATA_CMD message
    struct gapm_set_adv_data_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_SET_ADV_DATA_CMD,
                                                TASK_GAPM, TASK_BLE_WALKIE,
                                                gapm_set_adv_data_cmd,
                                                data_len);
    if (!p_cmd)
    {
        return;
    }
    // Fill the allocated kernel message
    p_cmd->operation       = operation;
    p_cmd->actv_idx        = actv_idx;
    p_cmd->length          = data_len;

    memcpy(p_cmd->data, adv_data, data_len);

    // Send the message
    ke_msg_send(p_cmd);
}

void walkie_periodic_sync_create(uint8_t own_addr_type)
{
    struct gapm_activity_create_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD,
                                                         TASK_GAPM, TASK_BLE_WALKIE,
                                                         gapm_activity_create_cmd);
    if (!p_cmd)
    {
        return;
    }

    // Set operation code
    p_cmd->operation = GAPM_CREATE_PERIOD_SYNC_ACTIVITY;
    p_cmd->own_addr_type = own_addr_type;

    // Send the message
    ke_msg_send(p_cmd);
}

void walkie_periodic_sync_enable(uint8_t actv_idx, uint8_t *per_para)
{
    // Prepare the GAPM_ACTIVITY_START_CMD message
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_ACTIVITY_START_CMD,
                                                         TASK_GAPM, TASK_BLE_WALKIE,
                                                         gapm_activity_start_cmd,
                                                         sizeof(gapm_per_sync_param_t));
    if (!p_cmd)
    {
        return;
    }

    gapm_per_sync_param_t *per_sync_param = (gapm_per_sync_param_t *)(p_cmd->u_param);

    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = actv_idx;
    memcpy(per_sync_param, per_para, sizeof(gapm_per_sync_param_t));

    // Send the message
    ke_msg_send(p_cmd);
}

void walkie_scan_creat(uint8_t own_addr_type)
{
    struct gapm_activity_create_cmd *p_cmd =
        KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD,TASK_GAPM, TASK_BLE_WALKIE,gapm_activity_create_cmd);
    if (!p_cmd)
    {
        return;
    }

    // Set operation code
    p_cmd->operation = GAPM_CREATE_SCAN_ACTIVITY;
    p_cmd->own_addr_type = own_addr_type;

    // Send the message
    ke_msg_send(p_cmd);
}

void walkie_scan_enable(uint8_t actv_idx,uint8_t *scan_para)
{
    // Prepare the GAPM_ACTIVITY_START_CMD message
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_ACTIVITY_START_CMD,
                                                         TASK_GAPM, TASK_BLE_WALKIE,
                                                         gapm_activity_start_cmd,
                                                         sizeof(gapm_scan_param_t));
    if (!p_cmd)
    {
        return;
    }

    gapm_scan_param_t *scan_param = (gapm_scan_param_t *)(p_cmd->u_param);

    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = actv_idx;
    memcpy(scan_param, scan_para, sizeof(gapm_scan_param_t));

    // Send the message
    ke_msg_send(p_cmd);
}

void walkie_set_device_list(uint8_t list_type, uint8_t *bdaddr, uint8_t size)
{
    // Prepare the GAPM_ACTIVITY_START_CMD message
    uint8_t para_len = ((list_type == GAPM_SET_WL)? sizeof(gap_bdaddr_t):sizeof(gap_per_adv_bdaddr_t)) * size;
    struct gapm_list_set_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_LIST_SET_CMD,
                                                         TASK_GAPM, TASK_BLE_WALKIE,
                                                         gapm_list_set_cmd,
                                                         para_len);
    if (!p_cmd)
    {
        return;
    }

    p_cmd->operation = list_type;
    p_cmd->size = size;
    if(bdaddr)
    {
        if (list_type == GAPM_SET_WL){
            struct gapm_list_set_wl_cmd *para_data = (struct gapm_list_set_wl_cmd *)p_cmd;
            memcpy(&para_data->wl_info[0], bdaddr, sizeof(gap_bdaddr_t)*size);
        }
        else
        {
            struct gapm_list_set_pal_cmd *para_data = (struct gapm_list_set_pal_cmd *)p_cmd;
            for(int i=0; i<size; i++)
            {
                memcpy(&para_data->pal_info[i], bdaddr + i*sizeof(gap_bdaddr_t), sizeof(gap_bdaddr_t));
                para_data->pal_info[i].adv_sid = 0;
            }
        }
    }
    TRACE(0, "W-T-G:%s type=%x, size=%x", __FUNCTION__, list_type, size);
    // Send the message
    ke_msg_send(p_cmd);
}

walie_gap_cb_func_t* walkie_cb = NULL;

static int walkie_gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                            struct gapm_cmp_evt *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
     if(walkie_cb){
          return(walkie_cb->cmp_evt_handler(param->operation, param->status, param->actv_idx));
     }else{
          return (KE_MSG_CONSUMED);
     }
}

static int walkie_gapm_activity_created_ind_handler(ke_msg_id_t const msgid,
                                      struct gapm_activity_created_ind const *p_param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    TRACE(0, "W-T-G:actv index %d created, type=%d, tx_pwr=%d", p_param->actv_idx,
                            p_param->actv_type, p_param->tx_pwr);

    return (KE_MSG_CONSUMED);
}

static int walkie_gapm_adv_report_evt_handler(ke_msg_id_t const msgid,
                                     struct gapm_ext_adv_report_ind *param,
                                     ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
	 if(walkie_cb){
          return(walkie_cb->adv_handler(param->data , param->actv_idx, param->trans_addr.addr, param->rssi, param->length));
     }else{
          return (KE_MSG_CONSUMED);
     }
}

static int walkie_gapm_activity_stopped_ind_handler(ke_msg_id_t const msgid,
                                      struct gapm_activity_stopped_ind const *p_param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
     TRACE(0, "W-T-G:activity_stopped:actv index %d, type=%d, %d,%d",
        p_param->actv_idx,p_param->actv_type, p_param->reason, p_param->per_adv_stop);
     if(walkie_cb){
          return(walkie_cb->stop_ind(p_param->actv_idx));
     }else{
          return (KE_MSG_CONSUMED);
     }
}
 int walkie_gapm_sync_established_evt_handler(ke_msg_id_t const msgid,
                                  struct gapm_sync_established_ind *param,
                                  ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    TRACE(0, "[%s][%d]W-T-G: sync device, actv_idx=%d", __FUNCTION__, __LINE__, param->actv_idx);
    DUMP8("%02x ", &param->addr.addr, GAP_BD_ADDR_LEN);
     if(walkie_cb){
          return(walkie_cb->per_sync_est_evt(param->actv_idx));
     }else{
          return (KE_MSG_CONSUMED);
     }

}
static int walkie_appm_msg_handler(ke_msg_id_t const msgid,
                          void *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}

KE_MSG_HANDLER_TAB(ble_walkie)
{
    /// please add in order of gapm_msg size
    // GAPM messages
    {GAPM_CMP_EVT,                 (ke_msg_func_t)walkie_gapm_cmp_evt_handler},
    {GAPM_ACTIVITY_CREATED_IND,    (ke_msg_func_t)walkie_gapm_activity_created_ind_handler},
    {GAPM_ACTIVITY_STOPPED_IND,    (ke_msg_func_t)walkie_gapm_activity_stopped_ind_handler},
    {GAPM_EXT_ADV_REPORT_IND,      (ke_msg_func_t)walkie_gapm_adv_report_evt_handler},
    {GAPM_SYNC_ESTABLISHED_IND,    (ke_msg_func_t)walkie_gapm_sync_established_evt_handler},
#ifdef BLE_ISO_ENABLED_X
    {GAPM_BIG_INFO_ADV_REPORT_IND, (ke_msg_func_t)walkie_gapm_big_info_adv_evt_report_handler},
#endif
    {KE_MSG_DEFAULT_HANDLER,       (ke_msg_func_t)walkie_appm_msg_handler},
};

// Application task descriptor
static ke_state_t ble_walkie_state;
const struct ke_task_desc TASK_BLE_WALKIE_APP = {ble_walkie_msg_handler_tab, &ble_walkie_state, 1, ARRAY_LEN(ble_walkie_msg_handler_tab)};

void walkie_ble_gapm_task_init(walie_gap_cb_func_t * cb)
{
    if (ke_task_check(TASK_BLE_WALKIE) == TASK_NONE) {//task type exist
        // Create APP task
        ke_task_create(TASK_BLE_WALKIE, &TASK_BLE_WALKIE_APP);
        // Initialize Task state
        ke_state_set(TASK_BLE_WALKIE, 0);
    }
    walkie_cb = cb;
}

#ifdef BLE_ISO_ENABLED_X
static void walkie_bis_recv_iso_data(uint16_t conhdl, uint8_t pkt_status)
{
    wt_dp_iso_buffer_t p_sdu_buf = {0};
    POSSIBLY_UNUSED walkie_gap_data_t audio_data = {0};

    while (walkie_iso_dp_itf_get_rx_data(conhdl, &p_sdu_buf))
    {
        audio_data.actv_idx = walkie_get_actv_idx_by_con_hdl(conhdl);
        ASSERT(audio_data.actv_idx != 0xFF,"actv idx error.")
        audio_data.pacet_num = p_sdu_buf.pkt_seq_nb;
        audio_data.data_len = p_sdu_buf.sdu_length;
        audio_data.data      = p_sdu_buf.sdu;
        // TRACE(4,"W-T-GAP:iso data con_hdl=0x%x,seq = %u,len = %u,status = %d",
        //        conhdl,p_sdu_buf.pkt_seq_nb, p_sdu_buf.sdu_length,p_sdu_buf.status);
        if (0 == p_sdu_buf.status)
        {
            walkie_event_cb(WALKIE_GAP_AUDIO_DATA, &audio_data);
        }else if(p_sdu_buf.sdu_length){
            TRACE(4,"W-T-GAP:iso data con_hdl=0x%x,seq = %u,len = %u,status = %d",
               conhdl,p_sdu_buf.pkt_seq_nb, p_sdu_buf.sdu_length,p_sdu_buf.status);
        }

        walkie_iso_dp_itf_rx_data_done(conhdl, p_sdu_buf.sdu_length,0, p_sdu_buf.sdu);
    }
}

void walkie_bis_register_recv_iso_data_callback()
{
    if (!isoohci_data_comed_callback_already_flag())
    {
        isoohci_data_come_callback_register((void*)walkie_bis_recv_iso_data);
    }
}

void walkie_bis_unregister_recv_iso_data_callback()
{
    isoohci_data_come_callback_register(NULL);
}

void walkie_send_iso_dp_remove_cmd(uint8_t conhdl,uint8_t direction_bf)
{
    // Allocate HCI command message
    struct hci_le_remove_iso_data_path_cmd* p_cmd =
            HL_HCI_CMD_ALLOC(HCI_LE_REMOVE_ISO_DATA_PATH_CMD_OPCODE, hci_le_remove_iso_data_path_cmd);

    // Fill command parameters
    p_cmd->data_path_direction = direction_bf;
    p_cmd->conhdl = conhdl;

    // Configure environment for reception of command complete event
    HL_HCI_CMD_SEND_TO_CTRL(p_cmd, direction_bf, walkie_talkie_big_hci_cmd_cmp_handler);
}

static void walkie_big_hci_cmd_setup_iso_data(struct hci_le_setup_iso_data_path_cmd *para)
{
    struct hci_le_setup_iso_data_path_cmd* p_cmd =
        HL_HCI_CMD_ALLOC(HCI_LE_SETUP_ISO_DATA_PATH_CMD_OPCODE,hci_le_setup_iso_data_path_cmd);

    if (para == NULL)
    {
        TRACE(0, "%s: setup_iso_data is NULL!", __func__);
        return;
    }
    memcpy(p_cmd, para, sizeof(struct hci_le_setup_iso_data_path_cmd));
    // Send the HCI command
    HL_HCI_CMD_SEND_TO_CTRL(p_cmd, p_cmd->data_path_direction, walkie_talkie_big_hci_cmd_cmp_handler);
}

static void walkie_bc_sink_start_setup_data_path(uint16_t conhdl)
{
    struct hci_le_setup_iso_data_path_cmd big_data;

    big_data.conhdl = conhdl;
    big_data.data_path_direction = WALKIE_IAP_DP_DIRECTION_OUTPUT;
    big_data.data_path_id = 0;
    big_data.ctrl_delay[0] = 6;
    big_data.codec_cfg_len = 0;

    walkie_big_hci_cmd_setup_iso_data(&big_data);
}

static void walkie_talkie_big_hci_cmd_cmp_handler(uint16_t opcode, uint16_t event, void const *p_evt)
{
    switch(opcode)
    {
        case HCI_LE_CREATE_BIG_CMD_OPCODE:
        {
            struct hci_basic_cmd_cmp_evt * bc_hci_cmp_evt = (struct hci_basic_cmd_cmp_evt *)p_evt;

            if(bc_hci_cmp_evt->status == CO_ERROR_NO_ERROR)
            {
                TRACE(0, "%s:create big cmd OK!", WALKIE_LOG_T);
            }
            else
            {
                TRACE(0, "%s:create big cmd fail,error code = %d!", WALKIE_LOG_T,bc_hci_cmp_evt->status);
            }
        }
            break;
        case HCI_LE_SETUP_ISO_DATA_PATH_CMD_OPCODE:
        {
            struct hci_le_setup_iso_data_path_cmd_cmp_evt *iso_dp_cmp_evt =
                            (struct hci_le_setup_iso_data_path_cmd_cmp_evt*)p_evt;
            TRACE(2, "%s:set iso data path status = %d,bis_hdl = 0x%x!,dircetion=%d",
                WALKIE_LOG_T,iso_dp_cmp_evt->status,iso_dp_cmp_evt->conhdl, event);
            if ((iso_dp_cmp_evt->status == CO_ERROR_NO_ERROR)&&(event == WALKIE_IAP_DP_DIRECTION_INPUT))
            {
                walkie_big_info.iso_data_path_enable = walkie_iso_src_start_streaming(iso_dp_cmp_evt->conhdl,
                    WALKIE_BC_SDU_INTERVAL_US, WALKIE_BC_MAX_TRANS_LATENCY_MS, WALKIE_BC_MAX_SDU_SIZE);
            }
         }
            break;
        case HCI_LE_REMOVE_ISO_DATA_PATH_CMD_OPCODE:
            TRACE(1, "%s:remove iso data path", WALKIE_LOG_T);
            break;
        case HCI_LE_TERMINATE_BIG_CMD_OPCODE:
            walkie_big_info.big_creat_cmp.conhdl[0] = 0xFFFF;
            walkie_big_info.iso_data_path_enable = false;
            TRACE(1, "%s:terminate big", WALKIE_LOG_T);
            break;
        case HCI_LE_BIG_TERMINATE_SYNC_CMD_OPCODE:
            TRACE(1, "%s:big terminate sync,big_hdl=%d", WALKIE_LOG_T,event);
            uint8_t act_idx = BIG_HDL_TO_ACTV_IDX(event);
            walkie_event_cb(WALKIE_BG_SYNC_STATUS_TERMINATE, (void*)&act_idx);
            break;
        default:
            break;
    }
}

void walkie_big_sync_estab_evt_handler(uint8_t evt_code, struct hci_le_big_sync_est_evt const* p_evt)
{
    uint8_t actv_idx;
    TRACE(3,"%s:%s big_hdl = 0x%x,con_hdl=0x%x",WALKIE_LOG_T,__func__,p_evt->big_hdl,p_evt->conhdl[0]);

    if (p_evt->status == CO_ERROR_NO_ERROR)
    {
        actv_idx = BIG_HDL_TO_ACTV_IDX(p_evt->big_hdl);
        TRACE(5,"actv_idx = %d,bn = %d,irc = %d,max_pdu =%d,nse=%d",
                actv_idx,p_evt->bn,p_evt->irc,p_evt->max_pdu,p_evt->nse);
        TRACE(3,"pto = %d,trans_latency = %d,iso_itv= %d",
                p_evt->pto,p_evt->big_trans_latency,p_evt->iso_interval);

        walkie_update_bis_receiver_con_hdl(actv_idx,p_evt->conhdl[0]);
        walkie_event_cb(WALKIE_BG_SYNC_STATUS_ESTABLISHED, (void*)&actv_idx);
        walkie_bc_sink_start_setup_data_path(p_evt->conhdl[0]);
    }
    else
    {
        if (p_evt->status == CO_ERROR_OPERATION_CANCELED_BY_HOST)
        {
            walkie_event_cb(WALKIE_BG_SYNC_STATUS_CANCELLED, (void*)&p_evt->big_hdl);
        }
        else
        {
            walkie_event_cb(WALKIE_BG_SYNC_STATUS_FAILED, (void*)&p_evt->big_hdl);
        }
    }
}

void walkie_big_sync_lost_evt_handler(uint8_t evt_code, struct hci_le_big_sync_lost_evt const* p_evt)
{
    uint8_t actv_idx = BIG_HDL_TO_ACTV_IDX(p_evt->big_hdl);
    TRACE(3,"%s:big_sync_lost actv_idx = %d,big_hdl = 0x%x",WALKIE_LOG_T,actv_idx,p_evt->big_hdl);

     walkie_event_cb(WALKIE_BG_SYNC_STATUS_LOST, (void*)&actv_idx);
}

void walkie_talkie_print_big_info(const walkie_gap_big_info_t* p_big_info)
{
    TRACE(0,"W-T-GAP:Big Info:");
    TRACE(2,"big_hdl=0x%x,sync_hdl=0x%x",p_big_info->big_hdl,p_big_info->sync_hdl);
    TRACE(4,"sdu_interval= %u,iso_interval=%u,max_sdu=%u,num_bis=%u",
        p_big_info->sdu_interval,p_big_info->iso_interval,p_big_info->max_sdu,p_big_info->num_bis);
    TRACE(4,"nse= %u,bn=%u,pto=%u,irc=%u",p_big_info->nse,p_big_info->bn,p_big_info->pto,p_big_info->irc);
    TRACE(4,"phy= %u,framing=%u,encrypted=%u",p_big_info->phy,p_big_info->framing,p_big_info->encrypted);
}

void walkie_big_hci_create_cmp_evt_handler(uint8_t evt_code, struct hci_le_create_big_cmp_evt const* p_evt)
{
    if (p_evt->status == CO_ERROR_NO_ERROR)
    {
        memcpy(&walkie_big_info.big_creat_cmp, p_evt, sizeof(struct hci_le_create_big_cmp_evt const));
        TRACE(1,"%s:big_hci_create_cmp bis con_hdl = 0x%x",WALKIE_LOG_T,p_evt->conhdl[0]);
        struct hci_le_setup_iso_data_path_cmd big_data;
        big_data.conhdl = p_evt->conhdl[0];
        big_data.data_path_direction = WALKIE_IAP_DP_DIRECTION_INPUT;
        big_data.data_path_id = 0;
        big_data.ctrl_delay[0] = 6;
        big_data.codec_cfg_len = 0;
        walkie_big_hci_cmd_setup_iso_data(&big_data);
    }
    else
    {
        TRACE(0, "[%s][%d][ERROR]: BIG creat fail!", __FUNCTION__, __LINE__);
    }
}

bool walkie_send_big_create_sync_hci_cmd(const walkie_gap_big_info_t* p_big_info)
{
    TRACE(0, "%s:create big sync_start", WALKIE_LOG_T);

    uint8_t p_broadcast_code[IAP_BROADCAST_CODE_LEN] = {0x11,0x22,};

    // Allocate HCI command message
    struct hci_le_big_create_sync_cmd* p_cmd =
            HL_HCI_CMD_ALLOC(HCI_LE_BIG_CREATE_SYNC_CMD_OPCODE,hci_le_big_create_sync_cmd);

    // Fill command parameters
    p_cmd->big_hdl = p_big_info->big_hdl;
    p_cmd->big_sync_timeout = 100;
    p_cmd->mse = p_big_info->nse;
    p_cmd->sync_hdl = p_big_info->sync_hdl;
    p_cmd->num_bis = p_big_info->num_bis;
    p_cmd->bis_id[0] = 0x01;
    p_cmd->encryption = p_big_info->encrypted;

    if (p_cmd->encryption)
    {
        memcpy(&p_cmd->broadcast_code[0], p_broadcast_code, IAP_BROADCAST_CODE_LEN);
    }

    // Send the HCI command
    HL_HCI_CMD_SEND_TO_CTRL(p_cmd, 0, walkie_talkie_big_hci_cmd_cmp_handler);

    return true;
}

// receiver stop bis sync
static void walkie_send_big_terminate_sync_hci_cmd(uint8_t big_hdl)
{
    struct hci_le_big_terminate_sync_cmd* p_big_cmd = HL_HCI_CMD_ALLOC(HCI_LE_BIG_TERMINATE_SYNC_CMD_OPCODE, hci_le_big_terminate_sync_cmd);
    p_big_cmd->big_hdl = big_hdl;

    // Send the HCI command
    HL_HCI_CMD_SEND_TO_CTRL(p_big_cmd, big_hdl , walkie_talkie_big_hci_cmd_cmp_handler);
}

// src
static void walkie_send_create_big_hci_cmd(struct hci_le_create_big_cmd *para)
{
    struct hci_le_create_big_cmd* p_big_cmd = HL_HCI_CMD_ALLOC(HCI_LE_CREATE_BIG_CMD_OPCODE, hci_le_create_big_cmd);

    if (para)
    {
        memcpy(p_big_cmd, para, sizeof(struct hci_le_create_big_cmd));
    }
    else
    {
        TRACE(0, "[%s][%d]: create big parmater is null!", WALKIE_LOG_T, __LINE__);
        return;
    }

    // Send the HCI command
    HL_HCI_CMD_SEND_TO_CTRL(p_big_cmd, 0, walkie_talkie_big_hci_cmd_cmp_handler);
}

// src stop bis
static void walkie_send_terminate_big_hci_cmd(uint8_t big_hdl)
{
    struct hci_le_terminate_big_cmd* p_big_cmd = HL_HCI_CMD_ALLOC(HCI_LE_TERMINATE_BIG_CMD_OPCODE, hci_le_terminate_big_cmd);
    p_big_cmd->big_hdl = big_hdl;
    p_big_cmd->reason = CO_ERROR_REMOTE_USER_TERM_CON;
    // Send the HCI command
    HL_HCI_CMD_SEND_TO_CTRL(p_big_cmd, 0, walkie_talkie_big_hci_cmd_cmp_handler);
}

#endif  //BLE_ISO_ENABLED


