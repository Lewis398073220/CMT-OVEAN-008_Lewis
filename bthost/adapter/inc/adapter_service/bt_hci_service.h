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
#ifndef __BT_HCI_SERVICE_H__
#define __BT_HCI_SERVICE_H__
#include "adapter_service.h"
#include "hci_transport.h"
#include "co_ppbuff.h"
#ifdef __cplusplus
extern "C" {
#endif

/// HCI ISO_Data_Load - Packet Status Flag
typedef enum
{
    /// Valid data. The complete ISO_SDU was received correctly
    GAF_ISO_PKT_STATUS_VALID   = (0),
    /// Possibly invalid data. The contents of the ISO_SDU may contain errors or part of the ISO_SDU may
    /// be missing. This is reported as "data with possible errors".
    GAF_ISO_PKT_STATUS_INVALID = (1),
    /// Part(s) of the ISO_SDU were not received correctly. This is reported as "lost data".
    GAF_ISO_PKT_STATUS_LOST    = (2),
} GAF_ISO_PKT_STATUS_E;

typedef struct
{
    /// Time_Stamp
    uint32_t        time_stamp;
    /// Packet Sequence Number
    uint16_t        pkt_seq_nb;
    uint16_t        data_len;
    uint8_t         *origin_buffer;
    uint8_t         *sdu_data;
    /// the status of data packet
    GAF_ISO_PKT_STATUS_E pkt_status;
    uint8_t         cisChannel;
    uint16_t        conhdl;
} gaf_media_data_t;

struct hci_evt_packet_t {
    uint8 evt_code;
    uint8 length;
    uint8 data[1];
} __attribute__ ((packed));

struct hci_cmd_evt_param_t {
    const uint8_t *command_data;
    const uint8_t *return_parameters;
    uint8_t cmd_data_len;
    uint8_t return_param_len;
    uint8_t return_status;
    bool command_status_event;
};

struct hci_bt_callback_t {
    void (*bt_acl_tx_done)(uint16_t connhdl, struct pp_buff *ppb);
    void (*rx_bt_acl_data)(uint16_t connhdl, struct pp_buff *ppb);
    void (*rx_bt_evt_data)(struct hci_evt_packet_t *evt_packet);
#if (CFG_SCO_OVER_HCI == 1)
    void (*rx_bt_sco_data)(uint16_t connhdl, hci_sco_ps_flag_t sco_ps_flag, const uint8_t *data, uint8_t len);
#endif
};

#ifdef BLE_HOST_SUPPORT
struct hci_le_callback_t {
    void (*le_acl_tx_done)(uint16_t connhdl);
    void (*rx_le_acl_data)(uint16_t connhdl_flags, const uint8_t *l2cap_hdr_ptr, uint16_t data_len);
    void (*rx_le_evt_data)(struct hci_evt_packet_t *evt_packet);
#ifdef BLE_ISO_ENABLED
    void *(*iso_rx_alloc)(uint32_t);
    void (*iso_rx_free)(void *);
    bool (*rx_iso_ready)(void);
    void (*rx_iso_data)(uint16_t connhdl);
#endif
};
#endif

void hci_global_init(void);
void hci_register_bt_callback(const struct hci_bt_callback_t *bt_cb);
struct hci_rx_desc_t *hci_get_rx_packet(void);
bt_status_t hci_send_bt_acl_packet(uint16 connhdl_flags, struct pp_buff *ppb);
bt_status_t hci_send_cmd_packet(struct pp_buff *ppb);
bt_status_t hci_send_command(uint16_t cmd_opcode, const uint8_t *cmd_data, uint8_t data_len);

typedef void (*hci_cmd_evt_func_t)(uint16_t cmd_opcode, struct hci_cmd_evt_param_t *param); // cmd cmpl status cb func
struct pp_buff *hci_create_cmd_packet(uint16_t cmd_opcode, uint8_t cmd_data_len, hci_cmd_evt_func_t cmpl_status_cb);
bt_status_t hci_send_command_with_callback(uint16_t cmd_opcode, const uint8_t *cmd_data, uint8_t data_len, hci_cmd_evt_func_t cmpl_status_cb);
bt_status_t hci_send_raw_comand(const uint8_t *cmd_packet, uint8_t packet_len, hci_cmd_evt_func_t cmpl_status_cb);

#ifdef BLE_HOST_SUPPORT
void hci_register_le_callback(const struct hci_le_callback_t *le_cb);
bt_status_t hci_send_le_acl_packet(const uint8_t *hci_header, uint8_t header_len, const uint8_t *l2cap_payload, uint16_t payload_len);
bt_status_t hci_send_le_cmd_packet(uint16_t cmd_opcode, const uint8_t *cmd_data, uint8_t data_len);
void hci_free_le_acl_rx_buffer(uint8_t *l2cap_hdr_ptr);
int hci_count_free_le_tx_buff(void);
#endif

#ifdef BLE_ISO_ENABLED
bt_status_t hci_send_iso_packet(struct hci_tx_iso_desc_t *tx_iso_desc);
gaf_media_data_t *hci_get_iso_rx_packet(uint16_t connhdl);
void hci_clean_iso_rx_packet(uint16_t connhdl);
void hci_clean_iso_tx_packet(uint16_t connhdl);
#endif

void hci_enable_cmd_evt_debug(bool enable);
void hci_enable_tx_flow_debug(bool enable);
void hci_enable_tx_0c35_without_alloc(bool enable);
void hci_register_controller_state_check(void (*cb)(void));
void hci_register_pending_too_many_rx_acl_packets(void (*cb)(void));
#ifdef IBRT
void hci_set_start_ibrt_reserve_buff(bool reserve);
void hci_register_acl_tx_buff_tss_process(void (*cb)(void));
#endif

uint8_t *hci_get_curr_pending_cmd(uint16_t cmd_opcode);
uint16_t hci_get_curr_cmd_opcode(void);
void hci_print_statistic(void);
void hci_rxtx_buff_process(void);
int hci_count_free_bt_tx_buff(void);

#ifdef __cplusplus
}
#endif
#endif /* __BT_HCI_SERVICE_H__ */
