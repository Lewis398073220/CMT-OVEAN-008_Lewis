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
#ifndef __HCI_I_H__
#define __HCI_I_H__
#include "string.h"
#include "stdint.h"
#include "bluetooth.h"
#if defined(__cplusplus)
extern "C" {
#endif

#define HCI_STATUS_OK                               0x00
#define HCI_ERR_UNKNOWN_HCI_CMD                     0x01
#define HCI_ERR_NO_CONNECTION                       0x02
#define HCI_ERR_HARDWARE_FAILURE                    0x03
#define HCI_ERR_PAGE_TIMEOUT                        0x04
#define HCI_ERR_AUTH_FAILURE                        0x05
#define HCI_ERR_KEY_MISSING                         0x06
#define HCI_ERR_MEMORY_FULL                         0x07
#define HCI_ERR_CONN_TIMEOUT                        0x08
#define HCI_ERR_MAX_NUM_CONNS                       0x09
#define HCI_ERR_MAX_SCO_CONNS                       0x0A
#define HCI_ERR_ACL_ALREADY_EXISTS                  0x0B
#define HCI_ERR_CMD_DISALLOWED                      0x0C
#define HCI_ERR_HOST_REJ_NO_RESOURCES               0x0D
#define HCI_ERR_HOST_REJ_SECURITY                   0x0E
#define HCI_ERR_HOST_REJ_PERSONAL_DEV               0x0F
#define HCI_ERR_HOST_TIMEOUT                        0x10
#define HCI_ERR_UNSUPP_FEATUR_PARM_VAL              0x11
#define HCI_ERR_INVAL_HCI_PARM_VAL                  0x12
#define HCI_ERR_CONN_TERM_USER_REQ                  0x13
#define HCI_ERR_CONN_TERM_LOW_RESOURCES             0x14
#define HCI_ERR_CONN_TERM_POWER_OFF                 0x15
#define HCI_ERR_CONN_TERM_LOCAL_HOST                0x16
#define HCI_ERR_REPEATED_ATTEMPTS                   0x17
#define HCI_ERR_PAIRING_DISALLOWED                  0x18
#define HCI_ERR_UNKNOWN_LMP_PDU                     0x19
#define HCI_ERR_UNSUPP_REMOTE_FEATURE               0x1A
#define HCI_ERR_SCO_OFFSET_REJECTED                 0x1B
#define HCI_ERR_SCO_INTERVAL_REJECTED               0x1C
#define HCI_ERR_SCO_AIR_MODE_REJECTED               0x1D
#define HCI_ERR_INVALID_LMP_PARM                    0x1E
#define HCI_ERR_UNSPECIFIED_ERROR                   0x1F
#define HCI_ERR_UNSUPP_LMP_PARM                     0x20
#define HCI_ERR_ROLE_CHANGE_DISALLOWED              0x21
#define HCI_ERR_LMP_RESPONSE_TIMEDOUT               0x22
#define HCI_ERR_LMP_ERR_TRANSACT_COLL               0x23
#define HCI_ERR_LMP_PDU_DISALLOWED                  0x24
#define HCI_ERR_ENCRYPTN_MODE_UNACCEPT              0x25
#define HCI_ERR_UNIT_KEY_USED                       0x26
#define HCI_ERR_QOS_NOT_SUPPORTED                   0x27
#define HCI_ERR_INSTANT_PASSED                      0x28
#define HCI_ERR_PAIRING_W_UNIT_KEY_UNSUPP           0x29
#define HCI_ERR_DIFFERENT_TRANSACTION_COLLISION     0x2A
#define HCI_ERR_INSUFF_RESOURCES_FOR_SCATTER_MODE   0x2B
#define HCI_ERR_QOS_UNACCEPTABLE_PARAMETER          0x2C
#define HCI_ERR_QOS_REJECTED                        0x2D
#define HCI_ERR_CHANNEL_CLASSIF_NOT_SUPPORTED       0x2E
#define HCI_ERR_INSUFFICIENT_SECURITY               0x2F
#define HCI_ERR_PARAMETER_OUT_OF_MANDATORY_RANGE    0x30
#define HCI_ERR_SCATTER_MODE_NO_LONGER_REQUIRED     0x31
#define HCI_ERR_ROLE_SWITCH_PENDING                 0x32
#define HCI_ERR_SCATTER_MODE_PARM_CHNG_PENDING      0x33
#define HCI_ERR_RESERVED_SLOT_VIOLATION             0x34
#define HCI_ERR_SWITCH_FAILED                       0x35
#define HCI_ERR_EXTENDED_INQ_RESP_TOO_LARGE         0x36
#define HCI_ERR_SECURE_SIMPLE_PAIR_NOT_SUPPORTED    0x37
#define HCI_ERR_HOST_BUSY_PAIRING                   0x38

/* voice setting*/
#define INPUT_CODING_LINEAR 0x0000
#define INPUT_CODING_ULAW 0x0100
#define INPUT_CODING_ALAW 0x0200
#define INPUT_DATA_FORMAT_1S 0x0000
#define INPUT_DATA_FORMAT_2S 0x0040
#define INPUT_DATA_FORMAT_SIGN 0x0080
#define INPUT_DATA_FORMAT_UNSIGN 0x00C0
#define INPUT_SAMPLE_SIZE_8BITS 0x0000
#define INPUT_SAMPLE_SIZE_16BITS 0x0020
#define AIR_CODING_FORMAT_CVSD 0x0000
#define AIR_CODING_FORMAT_ULAW 0x0001
#define AIR_CODING_FORMAT_ALAW 0x0002
#define AIR_CODING_FORMAT_TRANSPARENT 0x0003
#define AIR_CODING_FORMAT_MSBC 0x0060

/* ACL */
#define HCI_PKT_TYPE_DM1  0x0008
#define HCI_PKT_TYPE_DH1  0x0010
#define HCI_PKT_TYPE_DM3  0x0400
#define HCI_PKT_TYPE_DH3  0x0800
#define HCI_PKT_TYPE_DM5  0x4000
#define HCI_PKT_TYPE_DH5  0x8000
#define HCI_PKT_TYPE_ACL  0xcc18

/* sco esco */
#define HCI_PKT_TYPE_HV1  0x0001
#define HCI_PKT_TYPE_HV2  0x0002
#define HCI_PKT_TYPE_HV3  0x0004
#define HCI_PKT_TYPE_EV3  0x0008
#define HCI_PKT_TYPE_EV4  0x0010
#define HCI_PKT_TYPE_EV5  0x0020
#define HCI_PKT_TYPE_2EV3   0x0040
#define HCI_PKT_TYPE_3EV3   0x0080
#define HCI_PKT_TYPE_2EV5   0x0100
#define HCI_PKT_TYPE_3EV5   0x0200
#define HCI_PKT_TYPE_SCO  0x003f

#define HCI_HANDLE_MASK                        0x0FFF
#define HCI_FLAG_PB_MASK                      0x3000
#define HCI_FLAG_BROADCAST_MASK       0xC000

#define HCI_BLE_CISHDL_MIN                     0X100
#define HCI_BLE_BISHDL_MIN                     0X200

#define BROADCAST_NONE                         0x0000
#define BROADCAST_ACTIVE                      0x4000
#define BROADCAST_PARKED                     0x8000

/*create connection param*/
#define ALLOW_ROLE_SWITCH_YES   0x01
#define ALLOW_ROLE_SWITCH_NO    0x00

/*accept connection req param*/
#define ROLE_BECAME_MASTER 0x00
#define ROLE_REMAIN_SLAVE 0x01

/* link policy */
#define HCI_LP_ENABLE_ROLE_SWITCH_MASK  0x01
#define HCI_LP_ENABLE_HOLD_MODE_MASK        0x02
#define HCI_LP_ENABLE_SNIFF_MODE_MASK       0x04
#define HCI_LP_ENABLE_PARK_MODE_MASK        0x08

/// Enhanced Synchronous Connection HCI:7.1.41 & 7.1.42
#define    CODING_FORMAT_ULAW          0x00
#define    CODING_FORMAT_ALAW          0x01
#define    CODING_FORMAT_CVSD          0x02
#define    CODING_FORMAT_TRANSP        0x03
#define    CODING_FORMAT_LINPCM        0x04
#define    CODING_FORMAT_MSBC          0x05
#define    CODING_FORMAT_VENDSPEC      0xFF

#define    PCM_FORMAT_NA               0x00
#define    PCM_FORMAT_1SCOMP           0x01
#define    PCM_FORMAT_2SCOMP           0x02
#define    PCM_FORMAT_SIGNMAG          0x03
#define    PCM_FORMAT_UNSIGNED         0x04

#define    PCM_SAMPLE_SIZE_8BITS       8
#define    PCM_SAMPLE_SIZE_16BITS      16

#define    AUDIO_DATA_PATH_HCI               0
#define    AUDIO_DATA_PATH_PCM               1

/******************************************************************************************
 * Event Evtcode Definition  (OLD)
 *****************************************************************************************/
#define HCI_MasterLinkKeyCompleteEvt_Code                0x0A
#define HCI_PageScanModeChangeEvt_Code                   0x1F
#define HCI_PageScanRepetitionModeChangeEvt_Code         0x20
#define HCI_InquiryResultEvt_withRSSI                    0x22

#define HCI_EV_INQUIRY_COMPLETE 0x01
 struct hci_ev_inquiry_complete {
    uint8 status;
}__attribute__ ((packed));

#define HCI_EV_INQUIRY_RESULT        0x02
#define HCI_EV_INQUIRY_RESULT_RSSI   0x22
#define HCI_EV_INQUIRY_RESULT_EXTINQ 0x2F

#define PAGE_SCAN_REPETITION_MODE_R0    0x00
#define PAGE_SCAN_REPETITION_MODE_R1    0x01
#define PAGE_SCAN_REPETITION_MODE_R2    0x02
#define PAGE_SCAN_PERIOD_MODE_P0    0x00
#define PAGE_SCAN_PERIOD_MODE_P1    0x01
#define PAGE_SCAN_PERIOD_MODE_P2    0x02
 struct hci_ev_inquiry_result {
    uint8 num_responses;
    struct bdaddr_t bdaddr;
    uint8 page_scan_repetition_mode;
    uint8 reserved1;
    uint8 reserved2;/*must be 0*/
    uint8 class_dev[3];
    uint16 clk_off;
}__attribute__ ((packed));

#define HCI_EV_CONN_COMPLETE    0x03
struct hci_ev_conn_complete {
    uint8     status;
    uint16   handle;
    struct bdaddr_t bdaddr;
    uint8     link_type;
    uint8     encr_mode;
}__attribute__ ((packed));

#define HCI_EV_CONN_REQUEST 0x04

#define HCI_LINK_TYPE_SCO 0x00
#define HCI_LINK_TYPE_ACL 0x01
#define HCI_LINK_TYPE_ESCO 0x02
 struct hci_ev_conn_request {
    struct bdaddr_t bdaddr;
    uint8     class_dev[3];
    uint8     link_type;
}__attribute__ ((packed));

#define HCI_EV_DISCONN_COMPLETE 0x05
struct hci_ev_disconn_complete {
    uint8     status;
    uint16   handle;
    uint8     reason;
}__attribute__ ((packed));

#define HCI_EV_AUTHENTICATION_COMPLETE  0x06
 struct hci_ev_authentication_complete {
    uint8     status;
    uint16   handle;
}__attribute__ ((packed));

#define HCI_EV_REMOTENAMEREQ_COMPLETE   0x07
#define HCI_REMOTENAME_MAX    248
 struct hci_ev_remote_name_req_complete {
    uint8    status;
    struct   bdaddr_t bdaddr;
    uint8    name[HCI_REMOTENAME_MAX];
}__attribute__ ((packed));

#define HCI_EV_ENCRYPTION_CHANGE    0x08
 struct hci_ev_encryption_change {
    uint8    status;
    uint16   conn_handle;
    uint8    encryption_enable;
}__attribute__ ((packed));

#define HCI_EV_READ_REMOTE_FEATURES 0x0B
 struct hci_ev_read_remote_features{
    uint8    status;
    uint16   conn_handle;
    uint8    features[8];
}__attribute__ ((packed));

#define HCI_EV_READ_REMOTE_VERSION  0x0C
 struct hci_ev_read_remote_version{
    uint8    status;
    uint16   conn_handle;
    uint8    lmp_version;
    uint16 manufacturer_name;
    uint16  lmp_subversion;
}__attribute__ ((packed));

#define HCI_EV_QOSSETUP_COMPLETE 0x0D

 struct hci_ev_qossetup_complete{
    uint8    status;
    uint16   conn_handle;
    uint8    flags;
    uint8 service_type;
    uint32 token_rate;
    uint32 peak_bandwith;
    uint32 latency;
    uint32 delay_v;
}__attribute__ ((packed));


#define HCI_EV_CMD_COMPLETE 0x0e
struct hci_ev_cmd_complete {
    uint8   num_hci_cmd_packets;
    uint16  cmd_opcode;
    uint8   param[1];
}__attribute__ ((packed));

#define HCI_EV_CMD_STATUS   0x0f
struct hci_ev_cmd_status {
    uint8   status;
    uint8   num_hci_cmd_packets;
    uint16  cmd_opcode;
}__attribute__ ((packed));

#define HCI_EV_HARDWARE_ERROR   0x10
struct hci_ev_hardware_error {
    uint8 hw_code;
}__attribute__ ((packed));

#define HCI_EV_FLUSH_OCCURRED   0x11

#define HCI_EV_ROLE_CHANGE 0x12
 struct hci_ev_role_change{
    uint8 status;
    struct bdaddr_t bdaddr;
    uint8     new_role;
}__attribute__ ((packed));

 struct hci_ev_cmd_complete_read_stored_linkkey{
    uint8     status;
    uint16   max_num_keys;
    uint8    num_keys_read;
}__attribute__ ((packed));

 struct hci_ev_cmd_complete_le_read_buffer_size{
    uint8     status;
    uint16   le_data_packet_length;
    uint8    total_num_le_data_packets;
}__attribute__ ((packed));

 struct hci_ev_cmd_complete_role_discovery{
    uint8     status;
    uint16   connection_handle;
    uint8    current_role;
}__attribute__ ((packed));


struct hci_ev_cmd_complete_read_local_version{
    uint8 status;
    uint8 hci_version;
    uint16 hci_revision;
    uint8 lmp_version;
    uint16 manu_name;
    uint16 lmp_subversion;
}__attribute__ ((packed));

struct hci_ev_cmd_complete_read_local_sup_features{
    uint8 status;
    uint8 features[8];
}__attribute__ ((packed));

struct hci_ev_cmd_complete_read_local_ext_features{
    uint8 status;
    uint8 page_num;
    uint8 max_page_num;
    uint8 features[8];
}__attribute__ ((packed));

#define HCI_EV_NUM_OF_CMPLT 0x13
 struct hci_ev_num_of_complete{
    uint8  num_handle;
    uint16 handle;
    uint16 num_of_comp;
}__attribute__ ((packed));

 struct hci_rd_bt_rssi_cmd_cmp_evt{
     ///Status for command reception
     uint8_t status;
     ///Connection handle
     uint16_t conhdl;
     ///RSSI value
     int8_t rssi;
 }__attribute__ ((packed));

 struct hci_ev_num_of_complete_item{
    uint16 handle;
    uint16 num_of_comp;
}__attribute__ ((packed));

#define HCI_EV_MODE_CHANGE  0x14

#define HCI_MODE_ACTIVE 0x00
#define HCI_MODE_HOLD 0x01
#define HCI_MODE_SNIFF 0x02
#define HCI_MODE_PARK 0x03

 struct hci_ev_mode_change {
    uint8     status;
    uint16   handle;
    uint8    current_mode;
    uint16  interval;
}__attribute__ ((packed));

#define HCI_EV_RETURN_LINKKEYS  0x15

 struct hci_ev_return_linkkeys{
    uint8     num_keys;
    struct bdaddr_t bdaddr;
    uint8   link_key[16];
}__attribute__ ((packed));

#define HCI_EV_PIN_CODE_REQ 0x16
 struct hci_ev_pin_code_req {
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_LINK_KEY_REQ 0x17
 struct hci_ev_link_key_req {
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_LINK_KEY_NOTIFY  0x18
 struct hci_ev_link_key_notify {
    struct bdaddr_t bdaddr;
    uint8    link_key[16];
    uint8    key_type;
}__attribute__ ((packed));

#define HCI_EV_DATABUF_OVERFLOW 0x1A
struct hci_ev_databuf_overflow{
    uint8 link_type;
}__attribute__ ((packed));


#define HCI_EV_MAX_SLOT_CHANGE  0x1B
 struct hci_ev_max_slot_change {
    uint16 handle;
    uint8   max_slots;
}__attribute__ ((packed));

#define HCI_EV_READ_CLKOFF_Code              0x1C
 struct hci_ev_read_clkoff{
    uint8    status;
    uint16     handle;
    uint16    clkoff;
}__attribute__ ((packed));

#define HCI_EV_CONNPKT_TYPE_CHANGE           0x1D
 struct hci_ev_connpkt_type_change {
    uint8 status;
    uint16 handle;
    uint16 pkt_type;
}__attribute__ ((packed));

#define HCI_EV_QOS_VIOLATION                         0x1E
 struct hci_ev_qos_violation {
    uint16 handle;
}__attribute__ ((packed));

#define HCI_EV_FLOW_SPECIFICATION                    0x21
 struct hci_ev_flow_specification {
    uint8 status;
    uint16 handle;
    uint8 flags;
    uint8 flow_dir;
    uint8 service_type;
    uint32 token_rate;
    uint32 token_bucket;
    uint32 peak_bandwidth;
    uint32 latency;
}__attribute__ ((packed));

#define HCI_EV_READ_REMOTE_EXTFEATURES   0x23
 struct hci_ev_read_remote_extfeatures {
    uint8 status;
    uint16 handle;
    uint8 page_num;
    uint8 max_page_num;
    uint8 ext_features[8];
}__attribute__ ((packed));

#define HCI_EV_SYNC_CONN_COMPLETE   0x2c
struct hci_ev_sync_conn_complete {
    uint8     status;
    uint16   handle;
    struct bdaddr_t bdaddr;
    uint8 link_type;
    uint8 tx_interval;
    uint8 retx_window;
    uint16 rx_pkt_length;
    uint16 tx_pkt_length;
    uint8 air_mode;
}__attribute__ ((packed));

#define HCI_EV_SYNC_CONN_CHANGE 0x2D
 struct hci_ev_sync_conn_change {
    uint8     status;
    uint16   handle;
    uint8 tx_interval;
    uint8 retx_window;
    uint16 rx_pkt_length;
    uint16 tx_pkt_length;
}__attribute__ ((packed));

#define HCI_EV_SNIFF_SUBRATING          0x2E
 struct hci_ev_sniff_subrating {
    uint8 status;
    uint16 handle;
    uint16 maximum_transmit_lantency;
    uint16 maximum_receive_lantency;
    uint16 minimum_remote_timeout;
    uint16 minimum_local_timeout;
}__attribute__ ((packed));

#define HCI_EV_ENCRYPT_KEY_REFRESH_COMPLETE    0x30
struct hci_ev_encryption_refresh {
    uint8    status;
    uint16   conn_handle;
}__attribute__ ((packed));

#define HCI_EV_IO_CAPABILITY_REQUEST    0x31
struct hci_ev_io_capability_request {
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_IO_CAPABILITY_RESPONSE    0x32
struct hci_ev_io_capability_response {
    struct bdaddr_t bdaddr;
    uint8 io_capability;
    uint8 oob_data_present;
    uint8 authentication_requirement;
}__attribute__ ((packed));

#define HCI_EV_USER_CONFIRMATION_REQUEST    0x33
struct hci_ev_user_confirmation_request {
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_USER_PASSKEY_REQUEST      0x34
struct hci_ev_user_passkey_request {
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_EV_SIMPLE_PAIRING_COMPLETE    0x36
struct hci_ev_simple_pairing_complete{
    uint8 status;
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_LINK_SUPERV_TIMEOUT_CHANGED    0x38

#define HCI_EV_ENH_FLUSH_COMPLETE 0x39

#define BT_HCI_EVT_USER_PASSKEY_NOTIFY      0x3B
struct bt_hci_evt_user_passkey_notify {
    struct bdaddr_t bdaddr;
    uint32_t passkey;
} __attribute__ ((packed));

#define BT_HCI_EVT_KEYPRESS_NOTIFY      0x3C
struct bt_hci_evt_keypress_notify {
    struct bdaddr_t bdaddr;
    uint8_t  type;
} __attribute__ ((packed));

#define HCI_EV_NUM_OF_CMPL_BLOCKS   0x48


#define HCI_LE_META_EVT    0x3E

#define HCI_LE_EV_CONN_COMPLETE 0x01
struct hci_ev_le_conn_complete {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t role; // 0x00 central 0x01 peripheral
    uint8_t peer_addr_type; // 0x00 public 0x01 random
    bt_bdaddr_t peer_addr;
    uint16_t conn_interval;
    uint16_t peripheral_latency;
    uint16_t supv_timeout;
    uint8_t central_clock_accuracy;
} __attribute__ ((packed));

#define HCI_LE_EV_ADV_REPORT 0x02

#define HCI_LE_EV_CONN_UPDATE_COMPLETE 0x03
struct hci_ev_le_update_complete {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint16_t conn_interval;
    uint16_t peripheral_latency;
    uint16_t supv_timeout;
} __attribute__ ((packed));

#define HCI_LE_EV_ENHANCE_CONN_COMPLETE 0x0A
struct hci_ev_le_enhance_conn_complete {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t role; // 0x00 central 0x01 peripheral
    uint8_t peer_addr_type; // 0x00 public 0x01 random 0x02 public identity 0x03 random static identity
    bt_bdaddr_t peer_addr;
    bt_bdaddr_t local_rpa;
    bt_bdaddr_t peer_rpa;
    uint16_t conn_interval;
    uint16_t peripheral_latency;
    uint16_t supv_timeout;
    uint8_t central_clock_accuracy;
} __attribute__ ((packed));

#define HCI_LE_EV_PA_SYNC_ESTABLISH 0x0E
struct hci_ev_le_pa_sync_establish {
    uint8_t subcode;
    uint8_t status;
    uint16_t sync_handle;
    uint8_t adv_sid;
    uint8_t adv_addr_type; // 0x00 public 0x01 random 0x02 public identity 0x03 random static identity
    bt_bdaddr_t adv_addr;
    uint8_t adv_phy; // 0x01 le 1m, 0x02 le 2m, 0x03 le coded
    uint16_t pa_interval;
    uint8_t adv_clock_accuracy;
} __attribute__ ((packed));

#define HCI_LE_EV_PA_REPORT 0x0F
struct hci_ev_le_pa_report {
    uint8_t subcode;
    uint16_t sync_handle;
    uint8_t tx_power;
    uint8_t rssi;
    uint8_t cte_type; // 0x00 AoA cte, 0x01 AoD cte with 1us slots, 0x02 AoD cte with 2us slots, 0xff no cte
    uint8_t data_status; // 0x00 data complete, 0x01 data incomplete more data to come, 0x02 data incomplete truncated no more to come
    uint8_t data_length; // 0 to 247
    uint8_t data[1];
} __attribute__ ((packed));

#define HCI_LE_EV_PA_SYNC_LOST 0x10
struct hci_ev_le_pa_sync_lost {
    uint8_t subcode;
    uint16_t sync_handle;
} __attribute__ ((packed));

#define HCI_LE_EV_PA_SYNC_TRANSFER_RECEIVED 0x18
struct hci_ev_le_pa_sync_transfer_received {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint16_t service_data;
    uint16_t sync_handle;
    uint8_t adv_sid;
    uint8_t adv_addr_type; // 0x00 public 0x01 random 0x02 public identity 0x03 random static identity
    bt_bdaddr_t adv_addr;
    uint8_t adv_phy; // 0x01 le 1m, 0x02 le 2m, 0x03 le coded
    uint16_t pa_interval;
    uint8_t adv_clock_accuracy;
} __attribute__ ((packed));

#define HCI_EV_DEBUG                0xFF
 struct hci_ev_debug {
    uint16   debug_evt_code;
    uint8 param[1];
}__attribute__ ((packed));

struct get_buffer
{
    /// length of buffer
    uint8_t length;
    /// data of 128 bytes length
    uint8_t data[128];
};

struct hci_ev_rd_mem_cmp_evt
{
    ///Status
    uint8_t status;
    ///buffer structure to return
    struct get_buffer buf;
};


/* vendor event */
#define HCI_EV_VENDOR_EVENT                (0xFF)

//sub event code
#define HCI_DBG_TRACE_WARNING_EVT_CODE      0x01
#define HCI_ACL_SNIFFER_STATUS_EVT_CODE     0x03
#define HCI_TWS_EXCHANGE_CMP_EVT_CODE       0x04
#define HCI_NOTIFY_CURRENT_ADDR_EVT_CODE    0x05
#define HCI_NOTIFY_DATA_XFER_EVT_CODE       0x06
#define HCI_START_SWITCH_EVT_CODE           0x09
#define HCI_LL_MONITOR_EVT_CODE             0x0A
#define HCI_DBG_LMP_MESSAGE_RECORD_EVT_SUBCODE  0x0B
#define HCI_GET_TWS_SLAVE_MOBILE_RSSI_CODE             0x0C

#define HCI_SCO_SNIFFER_STATUS_EVT_CODE 0x02
#define SNIFFER_SCO_STOP                0
#define SNIFFER_SCO_START               1
#define SNIFFER_SCO_RESTART             2
#define SNIFFER_SCO_RESUME_AFTER_TO     3
#define SNIFFER_ACL_DISCONNECT_STOP_SCO 4
struct hci_ev_ibrt_snoop_sco {
    uint8_t subcode;
    uint16_t connhdl;
    uint8_t sco_status;
    uint8_t airmode;
    uint32_t bandwidth;
} __attribute__ ((packed));

#define HCI_DBG_IBRT_CONNECTED_EVT_SUBCODE 0x0E
struct hci_ev_ibrt_snoop_conn {
    uint8_t subcode;
    uint16_t connhdl;
    uint8_t status;
    bt_bdaddr_t mobile_addr;
    uint8_t role;
} __attribute__ ((packed));

#define HCI_DBG_IBRT_DISCONNECTED_EVT_SUBCODE 0x0f
struct hci_ev_ibrt_snoop_disc {
    uint8_t subcode;
    uint16_t connhdl;
    uint8_t reason;
    bt_bdaddr_t mobile_addr;
    uint8_t role;
} __attribute__ ((packed));

#define BLE_ADV_DATA_LENGTH 31
#define BLE_SCAN_RSP_DATA_LENGTH 31

/******************************************************************************************
   OGF opcodes group define
   hence the values
******************************************************************************************/
#define HCI_OGF_BIT_OFFSET                ((INT8U) 10) /* Number of bit shifts */
#define HCI_OPCODE_MASK                       0x03FF

/******************************************************************************************
   OCF opcode defines
******************************************************************************************/

/******************************************************************************************
   OCF opcode defines - Link Control Commands  (OGF: 0x01)
******************************************************************************************/
//#define HCI_INQUIRY                         0x0401
#define HCI_INQUIRY_CANCEL                    0x0402
#define HCI_PERIODIC_INQUIRY_MODE             0x0403
#define HCI_EXIT_PERIODIC_INQUIRY_MODE        0x0404
//#define HCI_CREATE_CONNECTION                   0x0405
//#define HCI_DISCONNECT                      0x0406
#define HCI_ADDSCO_CONN                       0x0407
//#define HCI_CREATE_CONNECTION_CANCEL          0x0408
//#define HCI_ACCEPT_CONNECTION_REQ           0x0409
//#define HCI_REJECT_CONNECTION_REQ               0x040A
//#define HCI_LINK_KEY_REQ_REPLY                  0x040B
//#define HCI_LINK_KEY_REQ_NEG_REPLY          0x040C
//#define HCI_PIN_CODE_REQ_REPLY                  0x040D
//#define HCI_PIN_CODE_REQ_NEG_REPLY          0x040E
#define HCI_CHANGE_CONN_PKT_TYPE              0x040F
//#define HCI_AUTH_REQ                        0x0411
//#define HCI_SET_CONN_ENCRYPTION                 0x0413
#define HCI_CHANGE_CONN_LINK_KEY              0x0415
#define HCI_MASTER_LINK_KEY                   0x0417
//#define HCI_REMOTE_NAME_REQ                 0x0419
//#define HCI_REMOTE_NAME_CANCEL              0x041A
#define HCI_READ_REMOTE_SUP_FEATURES          0x041B
#define HCI_READ_REMOTE_VER_INFO              0x041D
#define HCI_READ_CLOCK_OFFSET                 0x041F
#define HCI_READREMOTE_EXT_FEATURES           0x041C
#define HCI_READ_LMP_HANDLE                   0x0420
//#define HCI_SETUP_SYNC_CONN                     0x0428
//#define HCI_ACCEPT_SYNC_CONN                0x0429
//#define HCI_REJECT_SYNC_CONN                0x042A

/******************************************************************************************
   OCF opcode defines - Link Policy Commands  (OGF 0x02)
 ******************************************************************************************/
#define HCI_HOLD_MODE                             0x0801
#define HCI_PARK_MODE                             0x0805
#define HCI_EXIT_PARK_MODE                        0x0806
//#define HCI_QOS_SETUP                             0x0807
#define HCI_ROLE_DISCOVERY                        0x0809
//#define HCI_SWITCH_ROLE                           0x080B
#define HCI_READ_LP_SETTINGS                      0x080C           //LP:LINK POLICY
#define HCI_READ_DEFAULT_LP_SETTINGS              0x080E
#define HCI_WRITE_DEFAULT_LP_SETTINGS             0x080F
#define HCI_FLOW_SPECIFICATION                    0x0810

/******************************************************************************************
 OCF opcode defines -Host Controller and Baseband Commands (OGF 0x03)
******************************************************************************************/
#define HCI_SET_EVENT_MASK                    0x0C01
#define HCI_RESET                             0x0C03
#define HCI_SET_EVENT_FILTER                  0x0C05
#define HCI_FLUSH                             0x0C08
#define HCI_READ_PIN_TYPE                     0x0C09
#define HCI_WRITE_PIN_TYPE                    0x0C0A
#define HCI_CREATE_NEW_UNIT_KEY               0x0C0B
//#define HCI_DELETE_STORED_LINK_KEY          0x0C12
//#define HCI_WRITE_LOCAL_NAME                  0x0C13
#define HCI_READ_LOCAL_NAME                   0x0C14
#define HCI_READ_CONN_ACCEPT_TIMEOUT          0x0C15
#define HCI_WRITE_CONN_ACCEPT_TIMEOUT         0x0C16
#define HCI_READ_PAGE_TIMEOUT                 0x0C17
#define HCI_WRITE_PAGE_TIMEOUT                0x0C18
#define HCI_READ_SCAN_ENABLE                  0x0C19
#define HCI_WRITE_SCAN_ENABLE                 0x0C1A
#define HCI_READ_PAGESCAN_ACTIVITY            0x0C1B
#define HCI_WRITE_PAGESCAN_ACTIVITY           0x0C1C
#define HCI_READ_INQUIRYSCAN_ACTIVITY         0x0C1D
#define HCI_WRITE_INQUIRYSCAN_ACTIVITY        0x0C1E
#define HCI_READ_AUTH_ENABLE                  0x0C1F
//#define HCI_WRITE_AUTH_ENABLE                   0x0C20
#define HCI_READ_ENCRY_MODE                   0x0C21
#define HCI_WRITE_ENCRY_MODE                  0x0C22
#define HCI_READ_CLASS_OF_DEVICE              0x0C23
//#define HCI_WRITE_CLASS_OF_DEVICE               0x0C24
#define HCI_READ_VOICE_SETTING                0x0C25
#define HCI_WRITE_VOICE_SETTING               0x0C26
#define HCI_READ_AUTO_FLUSH_TIMEOUT           0x0C27
#define HCI_WRITE_AUTO_FLUSH_TIMEOUT          0x0C28
#define HCI_READ_NUM_BCAST_RETRANSMIT         0x0C29
#define HCI_WRITE_NUM_BCAST_RETRANSMIT        0x0C2A
#define HCI_READ_HOLD_MODE_ACTIVITY           0x0C2B
#define HCI_WRITE_HOLD_MODE_ACTIVITY          0x0C2C
#define HCI_READ_TX_POWER_LEVEL               0x0C2D
#define HCI_READ_SYNC_FLOW_CON_ENABLE         0x0C2E
#define HCI_WRITE_SYNC_FLOW_CON_ENABLE        0x0C2F
#define HCI_SET_HCTOHOST_FLOW_CONTROL         0x0C31
#define HCI_HOST_BUFFER_SIZE                  0x0C33
#define HCI_HOST_NUM_COMPLETED_PACKETS        0x0C35
#define HCI_READ_LINK_SUPERV_TIMEOUT          0x0C36
//#define HCI_WRITE_LINK_SUPERV_TIMEOUT         0x0C37
#define HCI_READ_NUM_SUPPORTED_IAC            0x0C38
#define HCI_READ_CURRENT_IAC_LAP              0x0C39
#define HCI_WRITE_CURRENT_IAC_LAP             0x0C3A
#define HCI_READ_PAGESCAN_PERIOD_MODE         0x0C3B
#define HCI_WRITE_PAGESCAN_PERIOD_MODE        0x0C3C

#define HCI_READ_PAGESCAN_MODE                0x0C3D
#define HCI_WRITE_PAGESCAN_MODE               0x0C3E

//#define SET_AFH_HOST_CHL_CLASSFICAT           0x0C3F
#define HCI_READ_INQUIRYSCAN_TYPE             0x0C42
#define HCI_WRITE_INQUIRYSCAN_TYPE            0x0C43
#define HCI_READ_INQUIRY_MODE                 0x0C44
#define HCI_WRITE_INQUIRY_MODE                0x0C45
#define HCI_READ_PAGESCAN_TYPE                0x0C46
//#define HCI_WRITE_PAGESCAN_TYPE               0x0C47
#define HCI_READ_AFH_CHL_ASSESS_MODE          0x0C48
#define HCI_WRITE_AFH_CHL_ASSESS_MODE         0x0C49
#define HCI_WRITE_EXTENDED_INQ_RESP           0x0C52
//#define HCI_WRITE_SIMPLE_PAIRING_MODE         0x0C56
#define HCI_READ_DEF_ERR_DATA_REPORTING       0x0C5A
#define HCI_WRITE_DEF_ERR_DATA_REPORTING      0x0C5B
#define HCI_READ_FLOW_CONTROL_MODE            0x0C66
#define HCI_WRITE_FLOW_CONTROL_MODE           0x0C67
#define HCI_WR_LE_HOST_SUPPORT                0x0C6D
#define HCI_WR_SEC_CON_HOST_SUPP              0x0C7A

/******************************************************************************************
 OCF opcode defines -Information Parameters (OGF  0x04)
******************************************************************************************/
#define HCI_READ_LOCAL_VER_INFO               0x1001
#define HCI_READ_LOCAL_SUP_COMMANDS           0x1002
#define HCI_READ_LOCAL_SUP_FEATURES           0x1003
#define HCI_READ_LOCAL_EXT_FEATURES           0x1004
#define HCI_READ_BD_ADDR                      0x1009

/******************************************************************************************
 OCF opcode defines -Status Parameters (0GF 0X05)
******************************************************************************************/
#define HCI_READ_FAILED_CONTACT_COUNT              0x1401
#define HCI_RESET_FAILED_CONTACT_COUNT             0x1402
#define HCI_READ_LINK_QUALITY                      0x1403
#define HCI_READ_RSSI                              0x1405
#define HCI_READ_AFH_CHANNEL_MAP                   0x1406
#define HCI_READ_CLOCK                             0x1407

/******************************************************************************************
 *OCF opcode defines -Testing Commands (OGF 0X06)
******************************************************************************************/
#define HCI_READ_LOOPBACK_MODE                0x1801
#define HCI_WRITE_LOOPBACK_MODE               0x1802
#define HCI_ENABLE_DUT_MODE                   0x1803

/******************************************************************************************
 *OCF opcode defines -BLE Commands (OGF 0X08)
******************************************************************************************/
#define HCI_BLE_SET_EVENT_MASK                0x2001


/******************************************************************************************
 *OCF opcode defines -Vendor Commands (OGF 0xff)
******************************************************************************************/

#define HCI_READ_LMP_PARAM               0xFC01
#define HCI_SET_AFH                      0xFC02
//#define HCI_SET_BD_ADDR                    0xFC04
#define HCI_PRJ_VERSION                  0xFC05
#define HCI_GET_PKT_STATICS              0xFC06
//#define HCI_READ_MEMORY                  0xFC07
//#define HCI_WRITE_MEMORY                 0xFC08
#define HCI_READ_HW_REGISTER             0xFC09
#define HCI_WRITE_HW_REGISTER            0xFC0A
#define HCI_TEST_CONTROL                 0xFC0B
#define HCI_SEND_PDU                     0xFC10
#define HCI_SET_SCO_CHANNEL              0xFC11
#define HCI_SET_ESCO_CHANNEL             0xFC12
#define HCI_DBG_OPCODE                   0xFC3f
#define HCI_SET_UART_BAUD_RATE           0xFC13
#define HCI_SET_UART_PORT                0xFC14
#define HCI_SET_CLOCK                    0xFC15
#define HCI_GET_PKTS_ERR                 0xFC16
#define HCI_DEEP_SLEEP                   0xFC19
//#define HCI_SET_SCOOVER_TYPE             0xFC1A
#define HCI_SET_SCOOVER_TYPE             0xFC04
//#define HCI_GET_SCOOVER_TYPE             0xFC1B
#define HCI_GET_SCOOVER_TYPE             0xFC03

#define HCI_CONFIG_WRITE             0xFC1C
#define HCI_CONFIG_READ             0xFC1D
#define HCI_CONFIG_FIXED_FREQ             0xFC1E
#define HCI_CONFIG_HOP_FREQ               0xFC1F
#define HCI_GET_IVT_SECCODE             0xfc20
#define HCI_SET_IVT_SECCODE             0xfc21
#define HCI_SET_CLK_DBGMODE             0xfc22
#define HCI_SET_SLAVE_TEST_MODE         0xfc23

#define HCI_OGF_VENDOR  0x3f
#define HCI_OCF_CONFIG_WRITE     0x1C
#define HCI_OCF_CONFIG_READ      0x1D

#define HCI_EVTCODE_VENDOR  0xff

#define HCI_GET_OGF(val)  ((unsigned int)val>>10)
#define HCI_GET_OCF(val)  ((unsigned int)val & 0x3ff)
#define HCI_GET_OPCODE(ogf,ocf)  ((ogf<<10) + ocf)

#ifdef HCI_AFH
#define HCI_AFHEvt_Code     0xFD
#define HCI_ChannelEvt_Code  0xFE
#endif

/******************************************************************************************
 *OCF opcode defines -Debug Commands (OGF )
******************************************************************************************/

/* LM/HCI Errors                                                                        */
#define HCI_ERROR_NO_ERROR             0x00
#define HCI_ERROR_UNKNOWN_HCI_CMD      0x01
#define HCI_ERROR_NO_CONNECTION        0x02
#define HCI_ERROR_HARDWARE_FAILURE     0x03
#define HCI_ERROR_PAGE_TIMEOUT         0x04
#define HCI_ERROR_AUTHENTICATE_FAILURE 0x05
#define HCI_ERROR_MISSING_KEY          0x06
#define HCI_ERROR_MEMORY_FULL          0x07
#define HCI_ERROR_CONNECTION_TIMEOUT   0x08
#define HCI_ERROR_MAX_CONNECTIONS      0x09
#define HCI_ERROR_MAX_SCO_CONNECTIONS  0x0a
#define HCI_ERROR_ACL_ALREADY_EXISTS   0x0b
#define HCI_ERROR_COMMAND_DISALLOWED   0x0c
#define HCI_ERROR_LIMITED_RESOURCE     0x0d
#define HCI_ERROR_SECURITY_ERROR       0x0e
#define HCI_ERROR_PERSONAL_DEVICE      0x0f
#define HCI_ERROR_HOST_TIMEOUT         0x10
#define HCI_ERROR_UNSUPPORTED_FEATURE  0x11
#define HCI_ERROR_INVALID_HCI_PARM     0x12
#define HCI_ERROR_USER_TERMINATED      0x13
#define HCI_ERROR_LOW_RESOURCES        0x14
#define HCI_ERROR_POWER_OFF            0x15
#define HCI_ERROR_LOCAL_TERMINATED     0x16
#define HCI_ERROR_REPEATED_ATTEMPTS    0x17
#define HCI_ERROR_PAIRING_NOT_ALLOWED  0x18
#define HCI_ERROR_UNKNOWN_LMP_PDU      0x19
#define HCI_ERROR_UNSUPPORTED_REMOTE   0x1a
#define HCI_ERROR_SCO_OFFSET_REJECT    0x1b
#define HCI_ERROR_SCO_INTERVAL_REJECT  0x1c
#define HCI_ERROR_SCO_AIR_MODE_REJECT  0x1d
#define HCI_ERROR_INVALID_LMP_PARM     0x1e
#define HCI_ERROR_UNSPECIFIED_ERR      0x1f
#define HCI_ERROR_UNSUPPORTED_LMP_PARM 0x20
#define HCI_ERROR_ROLE_CHG_NOT_ALLOWED 0x21
#define HCI_ERROR_LMP_RESPONSE_TIMEOUT 0x22
#define HCI_ERROR_LMP_TRANS_COLLISION  0x23
#define HCI_ERROR_LMP_PDU_NOT_ALLOWED  0x24
#define HCI_ERROR_ENCRYP_MODE_NOT_ACC  0x25
#define HCI_ERROR_UNIT_KEY_USED        0x26
#define HCI_ERROR_QOS_NOT_SUPPORTED    0x27
#define HCI_ERROR_INSTANT_PASSED       0x28
#define HCI_ERROR_PAIR_UNITKEY_NO_SUPP 0x29
#define HCI_ERROR_NOT_FOUND            0xf1
#define HCI_ERROR_REQUEST_CANCELLED    0xf2
#define HCI_ERROR_INVALID_SDP_PDU      0xd1
#define HCI_ERROR_SDP_DISCONNECT       0xd2
#define HCI_ERROR_SDP_NO_RESOURCES     0xd3
#define HCI_ERROR_SDP_INTERNAL_ERR     0xd4
#define HCI_ERROR_STORE_LINK_KEY_ERR   0xe0
//BES vendor error code
#define HCI_INVALID_BT_MODE            0xB2


#define HCI_INQUIRY                                   0x0401
struct hci_cp_inquiry
{
    uint8     lap[3];
    uint8     length;
    uint8     num_rsp;
} __attribute__ ((packed));

#define HCI_CREATE_CONNECTION                 0x0405
struct hci_cp_create_conn
{
    struct bdaddr_t bdaddr;
    uint16 pkt_type;
    uint8 page_scan_repetition_mode;
    uint8 reserved;
    uint16 clk_off;
    uint8 allow_role_switch;
} __attribute__ ((packed));

#define HCI_DISCONNECT                    0x0406
struct hci_cp_disconnect
{
    uint16   handle;
    uint8     reason;
} __attribute__ ((packed));

#define HCI_ADDSCO_CONN                       0x0407
struct hci_cp_addsco_conn
{
    uint16 conn_handle;
    uint16 pkt_type;
} __attribute__ ((packed));

#define HCI_CREATE_CONNECTION_CANCEL          0x0408
struct hci_create_connection_cancel
{
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_ACCEPT_CONNECTION_REQ             0x0409
struct hci_cp_accept_conn_req
{
    struct bdaddr_t bdaddr;

    uint8     role;
} __attribute__ ((packed));

#define HCI_REJECT_CONNECTION_REQ         0x040A
struct hci_cp_reject_conn_req
{
    struct bdaddr_t bdaddr;
    uint8     reason;
} __attribute__ ((packed));
#define HCI_LINK_KEY_REQ_REPLY            0x040B
struct hci_cp_link_key_reply
{
    struct bdaddr_t bdaddr;
    uint8     link_key[16];
} __attribute__ ((packed));

#define HCI_LINK_KEY_REQ_NEG_REPLY            0x040C
struct hci_cp_link_key_neg_reply
{
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_PIN_CODE_REQ_REPLY                0x040D
struct hci_cp_pin_code_reply
{
    struct bdaddr_t  bdaddr;
    uint8     pin_len;
    uint8     pin_code[16];
} __attribute__ ((packed));

#define HCI_PIN_CODE_REQ_NEG_REPLY            0x040E
struct hci_cp_pin_code_neg_reply
{
    struct bdaddr_t  bdaddr;
} __attribute__ ((packed));
#define HCI_CHANGE_CONN_PKT_TYPE              0x040F
struct hci_cp_change_conn_pkt_type
{
    uint16 conn_handle;
    uint16 pkt_type;
} __attribute__ ((packed));


#define HCI_AUTH_REQ                      0x0411
struct hci_cp_auth_req
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_SET_CONN_ENCRYPTION                   0x0413
struct hci_cp_set_conn_encryption
{
    uint16 conn_handle;
    uint8 encryption_enable;
} __attribute__ ((packed));


#define HCI_REMOTE_NAME_REQ                   0x0419
struct hci_cp_remote_name_request
{
    struct bdaddr_t bdaddr;
    uint8 page_scan_repetition_mode;
    uint8 reserved;
    uint16 clk_off;
} __attribute__ ((packed));

#define HCI_REMOTE_NAME_CANCEL                0x041A
struct hci_cp_remote_name_cancel
{
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_REMOTE_SUPPORTED_FEATURES   0x041B
struct hci_cp_remote_supported_feat
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_REMOTE_EXTENDED_FEATURES    0x041C
struct hci_cp_remote_extended_feat
{
    uint16 conn_handle;
    uint8 page_num;
} __attribute__ ((packed));

#define HCI_REMOTE_VERSION_INFO         0x041D
struct hci_cp_remote_version_info
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_SETUP_SYNC_CONN           0x0428
struct hci_cp_setup_sync_conn
{
    uint16 conn_handle;
    uint32 tx_bandwidth;
    uint32 rx_bandwidth;
    uint16 max_latency;
    uint16 voice_setting;
    uint8 retx_effort;
    uint16 pkt_type;
} __attribute__ ((packed));

#define HCI_ACCEPT_SYNC_CONN              0x0429
struct hci_cp_accept_sync_conn
{
    struct bdaddr_t bdaddr;
    uint32 tx_bandwidth;
    uint32 rx_bandwidth;
    uint16 max_latency;
    uint16 voice_setting;
    uint8 retx_effort;
    uint16 pkt_type;
} __attribute__ ((packed));


#define HCI_REJECT_SYNC_CONN              0x042A
struct hci_cp_reject_sync_conn
{
    struct bdaddr_t bdaddr;
    uint8     reason;
} __attribute__ ((packed));

#define HCI_IO_CAPABILIRY_RESPONSE           0x042B
struct hci_cp_io_capability_request_reply
{
    struct bdaddr_t bdaddr;
    uint8  io_caps;
    uint8  oob_present;
    uint8  auth_requirements;
}__attribute__ ((packed));

#define HCI_USER_PASSKEY_REQUEST_REPLY   0x042E
struct hci_cp_user_passkey_request_reply {
    struct bdaddr_t bdaddr;
    uint32_t passkey;
} __attribute__ ((packed));

#define HCI_USER_PASSKEY_REQUEST_NEG_REPLY  0x042F
struct hci_cp_user_passkey_request_neg_reply {
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_SEND_KEYPRESS_NOTIFY     0x0C60
struct hci_cp_send_keypress_notify {
    struct bdaddr_t bdaddr;
    uint8_t  type;
} __attribute__ ((packed));

#define HCI_ENHANCED_SETUP_SYNC_CONN           0x043d
struct hci_cp_enhanced_setup_sync_conn
{
    uint16 conn_handle;
    uint32 tx_bandwidth;
    uint32 rx_bandwidth;
    uint8 tx_coding_format_type;
    uint16 tx_coding_format_vendor;
    uint16 tx_coding_format_id;
    uint8 rx_coding_format_type;
    uint16 rx_coding_format_vendor;
    uint16 rx_coding_format_id;
    uint16 tx_codec_frame_size;
    uint16 rx_codec_frame_size;
    uint32 input_bandwidth;
    uint32 output_bandwidth;
    uint8 input_coding_format_type;
    uint16 input_coding_format_vendor;
    uint16 input_coding_format_id;
    uint8 output_coding_format_type;
    uint16 output_coding_format_vendor;
    uint16 output_coding_format_id;
    uint16 intput_coded_data_size;
    uint16 output_coded_data_size;
    uint8 input_pcm_data_format;
    uint8 output_pcm_data_format;
    uint8 input_pcm_sample_payload_msb_position;
    uint8 output_pcm_sample_payload_msb_position;
    uint8 intput_data_path;
    uint8 output_data_path;
    uint8 input_transport_unit_size;
    uint8 output_transport_unit_size;
    uint16 max_latency;
    uint16 packet_type;
    uint8 retx_effort;
} __attribute__ ((packed));

#define HCI_IO_CAPABILIRY_NEGATIVE_REPLY          0x0434
struct hci_cp_io_capability_negative_reply
{
    struct bdaddr_t bdaddr;
    uint8  reason;
}__attribute__ ((packed));

#define HCI_USER_CONFIRMATION_REPLY    0x042C
#define HCI_USER_CONFIRMATION_NEG_REPLY    0x042D
struct hci_cp_usr_confirmation_reply
{
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_SNIFF_MODE                            0x0803
struct hci_cp_sniff_mode
{
    uint16 conn_handle;
    uint16 sniff_max_interval;
    uint16 sniff_min_interval;
    uint16 sniff_attempt;
    uint16 sniff_timeout;
} __attribute__ ((packed));

#define HCI_EXIT_SNIFF_MODE                       0x0804
struct hci_cp_exit_sniff_mode
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_QOS_SETUP                             0x0807
struct hci_qos_setup
{
    uint16 conn_handle;
    uint8  unused;
    uint8  service_type;
    uint32 token_rate;
    uint32 peak_bandwidth;
    uint32 latency;
    uint32 delay_variation;
} __attribute__ ((packed));

#define HCI_ROLE_DISCOVERY                        0x0809
struct hci_write_role_discovery
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_SWITCH_ROLE                       0x080B
struct hci_witch_role
{
    struct bdaddr_t bdaddr;
    uint8           role;
} __attribute__ ((packed));

#define HCI_WRITE_LP_SETTINGS                     0x080D
struct hci_cp_write_link_policy
{
    uint16 conn_handle;
    uint16 link_policy_settings;
} __attribute__ ((packed));

#define HCI_SNIFF_SUBRATING             0x0811
struct hci_cp_sniff_subrating
{
    uint16 conn_handle;
    uint16 maximum_lantency;
    uint16 minimum_remote_timeout;
    uint16 minimum_local_timeout;
} __attribute__ ((packed));

#define HCI_SET_FILTER_TYPE_CLEAR   0x00
#define HCI_SET_FILTER_TYPE_INQUIRY 0x01
#define HCI_SET_FILTER_TYPE_CONNECT 0x02

#define HCI_SET_FILTER_COND_ALL     0x00
#define HCI_SET_FILTER_COND_COD     0x01
#define HCI_SET_FILTER_COND_ADDR    0x02

#define HCI_SET_FILTER_DONT_AUTO_CONNECT 0x00
#define HCI_SET_FILTER_AUTO_ACCEPT_DISABLE_MSS 0x01
#define HCI_SET_FILTER_AUTO_ACCEPT_ENABLE_MSS 0x02

struct hci_cp_set_clear_all_filters
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
} __attribute__ ((packed));

struct hci_cp_set_inquiry_no_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
} __attribute__ ((packed));

struct hci_cp_set_inquiry_cod_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 class_of_device[3]; // zeros, return all; non-zero interested
    uint8 class_of_device_mask[3]; // which bits in 'class_of_device' parameter are 'dont care', zero-bit means 'dont care'
} __attribute__ ((packed));

struct hci_cp_set_inquiry_address_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 bd_addr[6]; // interest address
} __attribute__ ((packed));

struct hci_cp_set_connect_no_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 auto_accept_flag; // 0x00 dont auto accept, 0x01 auto accept and disable role switch, 0x02 auto accept and enable role switch
} __attribute__ ((packed));

struct hci_cp_set_connect_cod_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 class_of_device[3]; // zeros, return all; non-zero interested
    uint8 class_of_device_mask[3]; // use zero to indicate which bits in 'class_of_device' are 'dont care'
    uint8 auto_accept_flag; // 0x00 dont auto accept, 0x01 auto accept and disable role switch, 0x02 auto accept and enable role switch
} __attribute__ ((packed));

struct hci_cp_set_connect_address_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 bd_addr[6]; // address interested
    uint8 auto_accept_flag; // 0x00 dont auto accept, 0x01 auto accept and disable role switch, 0x02 auto accept and enable role switch
} __attribute__ ((packed));

#define HCI_READ_STORED_LINK_KEY              0x0C0D
struct hci_cp_read_stored_linkkey
{
    struct bdaddr_t bdaddr;
    uint8 read_all_flag;
} __attribute__ ((packed));

#define HCI_WRITE_STORED_LINK_KEY             0x0C11
#define HCI_HS_STORED_LINK_KEY                0x201C //
struct hci_cp_write_stored_linkkey
{
    uint8 num_keys_to_write;
    struct bdaddr_t bdaddr;
    uint8    link_key[16];
} __attribute__ ((packed));

#define HCI_DELETE_STORED_LINK_KEY            0x0C12
struct hci_cp_delete_stored_linkkey
{
    struct bdaddr_t bdaddr;
    uint8  delete_all_flag;
} __attribute__ ((packed));

#define HCI_WRITE_LOCAL_NAME                  0x0C13
struct hci_cp_write_local_name
{
    uint8    local_name[BTM_NAME_MAX_LEN];
} __attribute__ ((packed));

#define HCI_WRITE_SCAN_ENABLE                 0x0C1A
struct hci_cp_write_scan_enable
{
    uint8     scan_enable;
} __attribute__ ((packed));

#define HCI_WRITE_AUTH_ENABLE                 0x0C20
struct hci_cp_write_auth_enable
{
    uint8 enable_flag;
} __attribute__ ((packed));

#define HCI_WRITE_CLASS_OF_DEVICE             0x0C24
struct hci_cp_write_class_of_device
{
    uint8    class_dev[3];
} __attribute__ ((packed));

#define HCI_WRITE_LINK_SUPERV_TIMEOUT         0x0C37
struct hci_write_superv_timeout
{
    uint16   connhandle;
    uint16   superv_timeout;
} __attribute__ ((packed));

#define HCI_WRITE_CURRENT_IAC_LAP        0x0C3A
struct hci_write_current_iac_lap
{
    uint8  iac_lap[4];
} __attribute__ ((packed));

#define HCI_WRITE_INQ_MODE              0x0C45
struct hci_write_inqmode
{
    uint8  inqmode;
} __attribute__ ((packed));

#define HCI_WRITE_PAGESCAN_TYPE          0x0C47
struct hci_write_pagescan_type
{
    uint8  pagescan_type;
} __attribute__ ((packed));

#define SET_AFH_HOST_CHL_CLASSFICAT           0x0C3F
struct hci_set_afh_channel_classification
{
    uint8  map[10];
} __attribute__ ((packed));

#define HCI_WRITE_INQSCAN_TYPE           0x0C43
struct hci_write_inqscan_type
{
    uint8  inqscan_type;
} __attribute__ ((packed));

#define HCI_WRITE_SIMPLE_PAIRING_MODE         0x0C56
struct hci_cp_write_simple_paring_mode
{
    uint8     simple_paring_mode;
} __attribute__ ((packed));

// HCI_WRITE_EXTENDED_INQ_RESP
struct hci_write_extended_inquiry_response
{
    uint8  fec;
    uint8  eir[240];
} __attribute__ ((packed));

#define HCI_READ_BUFFER_SIZE                  0x1005
struct hci_read_buffer_size_cmpl
{
    uint8_t status;
    uint16_t acl_data_packet_length;
    uint8_t sco_data_packet_length;
    uint16_t total_num_acl_data_packets;
    uint16_t total_num_sco_data_packets;
} __attribute__ ((packed));

#define HCI_READ_DATA_BLOCK_SIZE              0x100A
struct hci_read_data_block_size_cmpl
{
    uint8_t status;
    uint16_t max_acl_data_packet_length; // max length of the data portion of an HCI ACL data packet the btc can accept
    uint16_t data_block_length; // max length of the data portion of each HCI ACL data packet the btc can hold
    uint16_t total_num_data_blocks; // total number of data block buffers available in the btc
} __attribute__ ((packed));

#define HCI_READ_AVG_RSSI                    0x1405
struct hci_read_rssi_avg_rssi
{
    uint16 conn_handle;
}__attribute__ ((packed));

#define HCI_SET_BD_ADDR                  0xFC72
struct hci_cp_set_bdaddr
{
    uint8 addr_type; /* 0 bt, 1 ble */
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_WRITE_MEMORY                  0xFC02
struct hci_cp_write_memory
{
    uint32 address;
    uint8  type;
    uint8  length;
    uint32 value;
} __attribute__ ((packed));

#define HCI_READ_MEMORY                  0xFC01
struct hci_cp_read_memory
{
    uint32 address;
    uint8  type;
    uint8  length;
} __attribute__ ((packed));

#define HCI_SET_LOCAL_CLOCK                  0xFC0F
struct hci_set_local_clock
{
    uint32 clock;
} __attribute__ ((packed));

#define HCC_DBG_WRITE_SLEEP_EXWAKEUP_EN  0xFC77

//set sco path
#define HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE            0xFC8F
#define HCC_DBG_SET_SCO_SWITCH           0xFC89

#define HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE_OLD_VER            0xFC51
#define HCC_DBG_SET_SCO_SWITCH_OLD_VER           0xFC62

struct hci_set_switch_sco
{
    uint16  sco_handle;
} __attribute__ ((packed));

#define HCI_DBG_SET_SNIFFER_ENV_CMD_OPCODE    0xFC8E
struct hci_set_sniffer_env
{
    uint8 sniffer_active;
    uint8 sniffer_role;
    struct bdaddr_t monitor_bdaddr;
    struct bdaddr_t sniffer_bdaddr;
} __attribute__ ((packed));

#define HCI_DBG_START_TWS_EXCHANGE_CMD_OPCODE 0xFC91
struct hci_start_tws_exchange
{
    uint16   tws_slave_conn_handle;
    uint16   mobile_conn_handle;
} __attribute__ ((packed));

#define HCI_DBG_BTADDR_EXCHANGE_CMD_OPCODE  0xFC92
struct hci_tws_bdaddr_exchange
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_DBG_SCO_TX_SILENCE_CMD_OPCODE     0xFC94
struct hci_dbg_sco_tx_silence_cmd
{
    uint16   connhandle;
    uint8    silence_on;
} __attribute__ ((packed));

#define HCI_DBG_SNIFFER_CMD_OPCODE            0xFC95
struct hci_dbg_sniffer_interface
{
    uint16   connhandle;
    uint8    subcode;
} __attribute__ ((packed));

#define HCI_DBG_SET_LINK_LBRT_CMD_OPCODE          0xFC97
struct hci_set_link_lbrt_enable
{
    uint16 conn_handle;
    uint8  enable;
} __attribute__ ((packed));

#define HCI_LOWLAYER_MONITOR                     0xFC9b
struct hci_cp_lowlayer_monitor
{
    uint16 conn_handle;
    uint8 control_flag;
    uint8 report_format;
    uint32 data_format;
    uint8 report_unit;
} __attribute__ ((packed));

#define HCI_DBG_ENABLE_FASTACK_CMD_OPCODE     0xFCA1
struct hci_enable_fastack
{
    uint16 conn_handle;
    uint8 direction;
    uint8 enable;
} __attribute__ ((packed));

#define HCI_DBG_ENABLE_IBRT_MODE_CMD_OPCODE    0xFCA2
struct hci_ibrt_mode_op
{
    uint8 enable;
    uint8 switchOp;
} __attribute__ ((packed));

#define HCI_DBG_START_IBRT_CMD_OPCODE     0xFCA3
struct hci_start_ibrt
{
    uint16 slaveConnHandle;
    uint16 mobilConnHandle;
} __attribute__ ((packed));

#define HCI_DBG_GET_TWS_SLAVE_OF_MOBILE_RSSI_CMD_OPCODE  0xFCA4
struct hci_get_tws_slave_of_mobile_rssi
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_DBG_STOP_IBRT_CMD_OPCODE     0xFCA8
struct hci_stop_ibrt
{
    uint8 enable;
    uint8 reason;
} __attribute__ ((packed));

#define HCI_DBG_SEND_PREFER_RATE_CMD_OPCODE 0xFCA9
struct hci_dbg_send_prefer_rate
{
    uint16 conn_handle;
    uint8 rate;
} __attribute__ ((packed));

#define HCI_DBG_SET_FIX_TWS_INTERVAL_PARAM_CMD_OPCODE  0xFCAB
struct hci_set_fix_tws_interval_param
{
    uint16_t duration;
    uint16_t interval;
}__attribute__ ((packed));

#define HCI_DBG_RESUME_IBRT_CMD_OPCODE     0xFCAC
struct hci_resume_ibrt
{
    uint8 enable;
} __attribute__ ((packed));

#define HCI_DBG_SET_TXPWR_LINK_THD_OPCODE 0xFCB4
struct hci_dbg_set_txpwr_link_thd
{
    ///array index.if index is 0xff,reset all index data
    uint8_t     index;
    ///index enable or disable
    uint8_t     enable;
    uint8_t     link_id;
    uint16_t    rssi_avg_nb_pkt;
    int8_t      rssi_high_thr;
    int8_t      rssi_low_thr;
    int8_t      rssi_below_low_thr;
    int8_t      rssi_interf_thr;
} __attribute__ ((packed));

#define HCI_DBG_SET_ECC_DATA_TEST_CMD_OPCODE  0xFCBB

#define HCI_DBG_IBRT_SWITCH_CMD_OPCODE          0xFCBE
struct hci_ibrt_switch
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_DBG_STOP_MULTI_POINT_IBRT_CMD_OPCODE         0xFCBF
struct hci_stop_multi_point_ibrt
{
    uint16 mobile_conhdl;
    uint8 reason;
} __attribute__ ((packed));

#define HCI_DBG_SET_ISO_QUALITY_REPORT_THR  0xFCDC
struct hci_dbg_iso_quality_report
{
    uint16 conn_handle;
    uint16 qlty_rep_evt_cnt_thr;
    uint16 tx_unack_pkts_thr;
    uint16 tx_flush_pkts_thr;
    uint16 tx_last_subevent_pkts_thr;
    uint16 retrans_pkts_thr;
    uint16 crc_err_pkts_thr;
    uint16 rx_unreceived_pkts_thr;
    uint16 duplicate_pkts_thr;
} __attribute__ ((packed));

#define HCI_DBG_LE_TX_POWER_REQUEST_CMD_OPCODE  0xfcde
struct hci_dbg_le_tx_power_request
{
    uint16_t conhdl;
    // Enable/Disable LE tx power request (0: disable / 1: enable)
    uint8_t  enable;
    /// Delta in power control required (dBm)
    int8_t   delta;
    /// The rate on which delta required (@see enum btif_le_phy_rate)
    uint8_t  rx_rate;
}__attribute__ ((packed));


#define HCI_LE_READ_BUFFER_SIZE             0x2002
#define HCI_LE_READ_BUFFER_SIZE_V2          0x2060
struct hci_le_read_buffer_size_cmpl
{
    uint8_t status;
    uint16_t le_acl_data_packet_length;
    uint8_t total_num_le_acl_data_packets;
    uint16_t iso_data_packet_length;
    uint8_t total_num_iso_data_packets;
} __attribute__ ((packed));

#define HCC_WRITE_RANDOM_ADDR               0x2005
struct hci_write_random_addr
{
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCC_WRITE_ADV_PARAMETER             0x2006
struct hci_write_adv_param
{
    uint16  interval_min;
    uint16  interval_max;
    uint8   adv_type;
    uint8   own_addr_type;
    uint8   peer_addr_type;
    struct bdaddr_t bdaddr;
    uint8   adv_chanmap;
    uint8   adv_filter_policy;
} __attribute__ ((packed));

#define HCC_WRITE_ADV_DATA                  0x2008
struct hci_write_adv_data
{
    uint8 len;
    uint8 data[BLE_ADV_DATA_LENGTH];
} __attribute__ ((packed));

#define HCC_WRITE_SCAN_RSP_DATA             0x2009
struct hci_write_scan_rsp_data
{
    uint8 len;
    uint8 data[BLE_SCAN_RSP_DATA_LENGTH];
} __attribute__ ((packed));

#define HCC_WRITE_ADV_ENABLE                0x200A
struct hci_write_adv_en
{
    uint8 en;
} __attribute__ ((packed));

#define HCC_WRITE_BLE_SCAN_PARAMETER        0x200B
struct hci_write_ble_scan_param
{
    uint8   scan_type;
    uint16  scan_interval;
    uint16  scan_window;
    uint8   own_addr_type;
    uint8   scan_filter_policy;
} __attribute__ ((packed));

#define HCC_WRITE_BLE_SCAN_ENABLE           0x200C
struct hci_write_ble_can_en
{
    uint8 scan_en;
    uint8 filter_duplicat;
} __attribute__ ((packed));

#define HCC_CLEAR_WHITE_LIST                0x2010
#define HCC_ADD_DEVICE_TO_WHITELIST         0x2011
struct hci_add_device_to_wl
{
    uint8 addr_type;
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_LE_SET_HOST_CHL_CLASSIFICATION  0x2014
struct hci_set_le_channel_classification
{
    uint8  map[5];
} __attribute__ ((packed));

#define HCI_LE_READ_CHANNEL_MAP             0x2015
struct hci_le_read_chnl_map
{
    uint16 conn_handle;
} __attribute__ ((packed));

/**
 * The HCI_LE_Set_CIG_Parameters command is used by a Central's Host to create a CIG and
 * to set the parameters of one or more CISes that are associated with a CIG in the Controller.
 * And the HCI_LE_Set_CIG_Parameters_Test command should only be used for testing purpose.
 *
 * The CIG_ID parameter identifies a CIG, and it is allocated by the Central's Host and
 * passed to Peripheral's Host through LL during the process of creating a CIS. If the
 * CIG_ID does not exist, then the Controller shall first create a new CIG. Once the CIG
 * is created (whether through this command or previously), the Controller shall modify
 * or add CIS configurations in the CIG that is identified by the CIG_ID and update all
 * the parameters that apply to the CIG.
 *
 * The CIS_ID parameter identifies a CIS, and it is set by the Central's Host and paased
 * to Peripheral's Host through LL during the process of establishing a CIS.
 *
 * If the Host issues this command when the CIG is not in the configurable state, the
 * Controller shall return the error code Command Disallowed (0x0C).
 *
 * If the Host attempts to create a CIG or set parameters that exceed the max supported
 * resources in the Controller, the Controller shall return the error code Memory
 * Capacity Exceeded (0x07).
 *
 * If the Host attempts to set CIS parameters that exceed the maximum supported connections
 * in the Controller, the Controller shall return the error code Connection Limit
 * Exceeded (0x09).
 *
 * If the Host sets the PHY a bit that the Controller does not support, including a bit that
 * is reserved for future use, the Controller shall return error code Unsupported Feature
 * ore Parameter Value (0x11).
 *
 * If the Controller does not support asymmetric PHYs and the Host sets phy_c2p to a different
 * value than phy_p2c, the Controller shall return the error code Unsupported Feature or
 * Parameter Value (0x11).
 *
 * If the Host specifies an invalid combination of CIS parameters, the Controller shall
 * return the error code Unsupported Feature or Parameter Value (0x11).
 *
 */
#define HCI_LE_SET_CIG_PARAMETERS           0x2062
#define HCI_LE_SET_CIG_PARAMETERS_TEST      0x2063
struct hci_le_cig_configuration {
    uint8_t cis_id; // 0x00 to 0xEF, used to identify a CIS
    uint16_t max_sdu_c2p; // 0x0000 to 0x0FFFF, max octets of the sdu payload from C host
    uint16_t max_sdu_p2c; // 0x0000 to 0x0FFFF, max octets of the sdu payload from P Host
    uint8_t phy_c2p; // bit 0 - C TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t phy_p2c; // bit 0 - P TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t rtn_c2p; // retransmission number of every CIS Data PDU from C to P before ack or flushed, just recommendation
    uint8_t rtn_p2c; // retransmission number of every CIS Data PDU from P to C before ack or flushed, just recommendation
} __attribute__ ((packed));
struct hci_le_set_cig_parameters
{
    uint8_t cig_id; // 0x00 to 0xEF, used to identify the CIG
    uint8_t sdu_interval_c2p[3]; // us, 0xFF to 0x0F_FFFF, SDU interval from the C Host for all CISes in CIG
    uint8_t sdu_interval_p2c[3]; // us, 0xFF to 0x0F_FFFF, SDU interval from the P host for all CISes in CIG
    uint8_t worst_case_sca; // worst case sleep clock accuracy of all the Peripherals that will participate in the CIG
    uint8_t packing; // 0x00 sequential, 0x01 interleaved, this is a recommendation the controller may ignore
    uint8_t framing; // 0x00 unframed, 0x01 framed
    uint16_t max_transport_latency_c2p; // ms, 0x05 to 0x0FA0, max transport latency from C btc to P btc
    uint16_t max_transport_latency_p2c; // ms
    uint8_t cis_count; // 0x00 to 0x1F, total number of CIS configurations in the CIG being added or modified
    struct hci_le_cig_configuration cig_cfg[1];
} __attribute__ ((packed));
struct hci_le_cig_configuration_test {
    uint8_t cis_id; // 0x00 to 0xEF, used to identify a CIS
    uint8_t nse; // 0x01 to 0x1F, max number of subevents in each CIS event
    uint16_t max_sdu_c2p; // 0x0000 to 0x0FFFF, max octets of the sdu payload from C host
    uint16_t max_sdu_p2c; // 0x0000 to 0x0FFFF, max octets of the sdu payload from P Host
    uint16_t max_pdu_c2p; // 0x00 to 0xFB, max octets of the pdu payload from C LL to P LL
    uint16_t max_pdu_p2c; // 0x00 to 0xFB, max octets of the pdu payload from P LL to C LL
    uint8_t phy_c2p; // bit 0 - C TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t phy_p2c; // bit 0 - P TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t bn_c2p; // 0x00 no ISO data from C to P, 0x01 to 0x0F BN for C to P transmission
    uint8_t bn_p2c; // 0x00 no ISO data from P to C, 0x01 to 0x0F BN for P to C transmission
} __attribute__ ((packed));
struct hci_le_set_cig_parameters_test
{
    uint8_t cig_id; // 0x00 to 0xEF, used to identify the CIG
    uint8_t sdu_interval_c2p[3]; // us, 0xFF to 0x0F_FFFF, SDU interval from the C Host for all CISes in CIG
    uint8_t sdu_interval_p2c[3]; // us, 0xFF to 0x0F_FFFF, SDU interval from the P host for all CISes in CIG
    uint8_t ft_c2p; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from C to P
    uint8_t ft_p2c; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from P to C
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, CIS anchor points interval
    uint8_t worst_case_sca; // worst case sleep clock accuracy of all the Peripherals that will participate in the CIG
    uint8_t packing; // 0x00 sequential, 0x01 interleaved, this is a recommendation the controller may ignore
    uint8_t framing; // 0x00 unframed, 0x01 framed
    uint8_t cis_count; // 0x00 to 0x1F, total number of CIS configurations in the CIG being added or modified
    struct hci_le_cig_configuration_test cig_cfg[1];
} __attribute__ ((packed));
struct hci_le_set_cig_params_cmpl
{
    uint8_t status;
    uint8_t cig_id; // 0x00 to 0xEF
    uint8_t cis_count;  // 0x00 to 0x1F
    uint16_t cis_handle[1];
} __attribute__ ((packed));

/**
 * The HCI_LE_Create_CIS command is used by the Central's Host to create
 * one or more CISes.
 *
 * The CIS_Connection_Handle parameter specifies the connection handle
 * corresponding to the configuration of the CIS to be created and whose
 * configuration is already stored in a CIG.
 *
 * The ACL_Connection_Handle parameter specifies the connection handle
 * of the ACL associated with each CIS to be created.
 *
 * If the Host issue this command before all the HCI_LE_CIS_Established
 * events from the previous use of the command have been generated, the
 * Controller shall return the error code Command Disallowed (0x0C).
 *
 * If the Host issues this command when the CIS (Host Support) feature
 * bit is not set, the Controller shall return the error code Command
 * Disallowed (0x0C).
 *
 * When the Controller receives this command, the Controller sends the
 * HCI_Command_Status event to the Host. An HCI_LE_CIS_Established event
 * will be generated for each CIS when it is established or if it is
 * disconnected or considered lost before being established; until all
 * the events are generated, the command remains pending.
 *
 * The HCI_LE_CIS_Established event indicates that a CIS has been established,
 * was considered lost before being established, or -- on the Central -- was
 * rejected by the Peripheral. It is generated by the Controller in the
 * Central and Peripheral.
 *
 * If HCI_LE_CIS_REQUEST is rejected by Peripheral, only HCI_Command_Complete
 * event is generated, there is no HCI_LE_CIS_Established event in Peripheral
 * side.
 *
 */
#define HCI_LE_CREATE_CIS           0x2064
struct hci_le_create_cis_item {
    uint16_t cis_handle;
    uint16_t acl_handle;
} __attribute__ ((packed));
struct hci_le_create_cis {
    uint8_t cis_count;
    struct hci_le_create_cis_item cis_item[1];
} __attribute__ ((packed));

#define HCI_LE_EV_CIS_REQUEST 0x1A
struct hci_ev_le_cis_request {
    uint8_t subcode;
    uint16_t acl_handle;
    uint16_t cis_handle;
    uint8_t cig_id;
    uint8_t cis_id;
} __attribute__ ((packed));

#define HCI_LE_ACCEPT_CIS_REQUEST   0x2066
#define HCI_LE_REJECT_CIS_REQUEST   0x2067
struct hci_le_accept_cis_request {
    uint16_t cis_handle;
} __attribute__ ((packed));
struct hci_le_reject_cis_request {
    uint16_t cis_handle;
    uint8_t reason;
} __attribute__ ((packed));
struct hci_le_reject_cis_request_cmpl {
    uint8_t status;
    uint16_t cis_handle;
} __attribute__ ((packed));

#define HCI_LE_EV_CIS_ESTABLISH 0x19
struct hci_ev_le_cis_establish {
    uint8_t subcode;
    uint8_t status;
    uint16_t cis_handle;
    uint8_t cig_sync_delay[3]; // 0xEA to 0x7F_FFFF, max us for transmission of PDUs of all CISes in a CIG event
    uint8_t cis_sync_delay[3]; // 0xEA to 0x7F_FFFF, max us for transmission of PDUs of the specified CIS in a CIG event
    uint8_t transport_latency_c2p[3]; // us, 0xEA to 0x7F_FFFF, the actual transport latency us from C btc to P btc
    uint8_t transport_latency_p2c[3]; // us
    uint8_t phy_c2p;
    uint8_t phy_p2c;
    uint8_t nse; // 0x01 to 0x1F, max number of subevents in each ISO event (CIS event)
    uint8_t bn_c2p; // 0x00 no ISO data from C to P, 0x01 to 0x0F BN for C to P transmission
    uint8_t bn_p2c; // 0x00 no ISO data from P to C, 0x01 to 0x0F BN for P to C transmission
    uint8_t ft_c2p; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from C to P
    uint8_t ft_p2c; // 0x01 to 0xFF
    uint16_t max_pdu_c2p; // 0x00 to 0xFB, max octets of the pdu payload from C LL to P LL
    uint16_t max_pdu_p2c; // 0x00 to 0xFB
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, CIS anchor points interval
} __attribute__ ((packed));

/**
 * The HCI_LE_Remove_CIG command is used by the Central's Host to remove the CIG identified
 * by CIG_ID. This command shall delete the CIG_ID and also delete the Connection_Handles of
 * the CIS configurations stored in the CIG.
 *
 * This command shall also remove the ISO data paths that are associated with the Connection_Handles
 * of the CIS configurations, which is equivalent to issuing the HCI_LE_Remove_ISO_Data_Path command.
 *
 * If the Host tries to remove a CIG which is in the active state, then the Controller shall
 * return the error code Command Disablled (0x0C). So before call this command, all CISes in
 * the CIG shall be disconnected using HCI_Disconnect command issued by Central or Peripheral.
 *
 * If the Host issues this command with a CIG_ID that does not exist, the Controller shall return
 * the error code Unknown Connection Identifier (0x02).
 *
 */
#define HCI_LE_REMOVE_CIG           0x2065
struct hci_le_remove_cig {
    uint8_t cig_id;
} __attribute__ ((packed));
struct hci_le_remove_cig_cmpl {
    uint8_t status;
    uint8_t cig_id;
} __attribute__ ((packed));

/**
 * The HCI_LE_Setup_ISO_Data_Path command is used to identify and create the
 * ISO data path between the Host and the Controller for a CIS, CIS configuration,
 * or BIS identified by the Connection_Handle parameter.
 *
 * The input and output directions are defined from the perspective of the
 * Controller, so "input" refers to data flowing from the Host to Controller.
 *
 * When Data_Path_Direction is set to 0x00 (input, host to controller direction),
 * the Controller_Delay parameter specifies the delay at the data source from the
 * reference time of an SDU to the CIG reference point or BIG anchor point.
 * When Data_Path_Direction is set to 0x01 (output, controller to host direction),
 * Controller_dealy specifies the delay from the SDU_Synchronization_Reference to
 * the point in time at which the Controller begins to transfer the corresponding data
 * to the data path interface. The Host should use the HCI_Read_Local_Supported_Controller_Dealy
 * command to obtain a suitable value for Controller_Delay.
 *
 * Note: Controller vendors may provides additional guidance to the Host on how
 * to select a suitable Controller_Delay value from the range of values provided
 * by the HCI_Read_Local_Supported_Controller_Delay command for various configurations
 * of the data path interface.
 *
 * The Min_Controller_Delay and Max_Controller_Delay parameters returned by the Controller
 * provide a range of allowed values to be used by the Host when issuing the
 * HCI_LE_Setup_ISO_Data_Path command. The Min_Controller_Delay shall be greater than or
 * equal to the codec processing delay for the specified direction and codec configuration.
 * The Max_Controller_delay shall be less than or equal to the sum of the codec processing
 * delay and the max time the Controller can buffer the data for the specified directon
 * and codec configuration.
 *
 * If the Host issues this command more than once for the same Connection_Handle
 * and direction before issuing HCI_LE_Remove_ISO_Data_Path command for that
 * Connection_Handle and direction, the Controller shall return the error code
 * Command Disallowed (0x0C).
 *
 * If the Host issues this command for a CIS on a Peripheral before it has issued
 * the HCI_LE_Accept_CIS_Request for that CIS, then the Controller shall return
 * the error code Command Disallowed (0x0C).
 *
 * The Data_Path_ID parameter specifies the data transport path used. When set
 * to 0x00, the data path shall be over the HCI transport. When set to a value in
 * the range 0x01 to 0xFE, the data path shall use a vendor-specific transport
 * interface (e.g., a PCM interface) with logical transport numbers. The meanings
 * of these logical transport numbers are vendor-specific.
 *
 * If the Host issues this command for a vendor-specific data transport path that
 * has not been configured using the HCI_Configure_Data_Path command, the Controller
 * shall return the error code Command Disallowed (0x0C).
 *
 * If the Host attempts to set a data path with a Connection Handle that does not
 * exist or that is not for a CIS, CIS configuration, or BIS, the Controller shall
 * return the error code Unknown Connection Identifier (0x02).
 *
 * If the Host attempts to set an output data path using a connection handle that
 * is for an ISO Broadcaster, for an input data path on a Synchroized Receiver,
 * or for a data path for the direction on a unidirectional CIS where BN is set
 * to 0, the Controller shall return the error code Command Disabllowed (0x0C).
 *
 * If the Host issues this command with Codec_Configuration_Length non-zero and
 * Codec_ID set to transparent air mode, the Controller shall return the error
 * codec Invalid HCI Command Parameters (0x12).
 *
 * If the Host issues this command with codec-related parameters that exceed the
 * bandwidth and latency allowed on the established CIS or BIS identified by
 * the Connection Handle paramerter, the Controller shall return the error code
 * Invalid HCI Command Parameters (0x12).
 *
 */
typedef enum {
    HCI_ISO_CODEC_U_LAW_LOG = 0x00,
    HCI_ISO_CODEC_A_LAW_LOG = 0x01,
    HCI_ISO_CODEC_CVSD = 0x02,
    HCI_ISO_CODEC_TRANSPARENT = 0x03, // indicates that the controller does not do any transcoding or resampling
    HCI_ISO_CODEC_LINEAR_PCM = 0x04,
    HCI_ISO_CODEC_MSBC = 0x05,
    HCI_ISO_CODEC_LC3 = 0x06,
    HCI_ISO_CODEC_G729A = 0x07,
    HCI_ISO_CODEC_VENDOR = 0xFF,
} hci_iso_codec_t;

typedef enum {
    HCI_CODEC_DT_BT_ACL = 0x01,
    HCI_CODEC_DT_BT_SCO = 0x02,
    HCI_CODEC_DT_LE_CIS = 0x04,
    HCI_CODEC_DT_LE_BIS = 0x08,
} hci_codec_data_transport_t;

typedef enum {
    HCI_LOGICAL_TRANSPORT_BT_ACL = 0x00,
    HCI_LOGICAL_TRANSPORT_BT_SCO = 0x01,
    HCI_LOGICAL_TRANSPORT_LE_CIS = 0x02,
    HCI_LOGICAL_TRANSPORT_LE_BIS = 0x03,
} hci_logical_transport_t;

typedef enum {
    HCI_ISO_DATA_DIR_HOST_TX = 0x00,
    HCI_ISO_DATA_DIR_HOST_RX = 0x01,
} hci_iso_data_dir_t;

typedef enum {
    HCI_ISO_DATA_ID_OVER_HCI = 0x00,
} hci_iso_data_id_t;

#define HCI_READ_LOCAL_SUPP_CODECS      0x100B // read local controller supported codecs
#define HCI_READ_LOCAL_SUPP_CODESC_V2   0x100D
struct hci_local_standard_codec_item {
    uint8_t standard_codec_id;
    uint8_t codec_transport; // bit 0 BR/EDR ACL, bit 1 BR/EDR SCO/eSCO, bit 2 LE CIS, bit 3 LE BIS
} __attribute__ ((packed));
struct hci_local_vendor_codec_item {
    uint8_t vendor_codec_id[4]; // octets 0 to 1: company id, octets 2 to 3: vendor-defined codec id
    uint8_t codec_transport; // bit 0 BR/EDR ACL, bit 1 BR/EDR SCO/eSCO, bit 2 LE CIS, bit 3 LE BIS
} __attribute__ ((packed));
struct hci_read_local_supp_codecs_v2_cmpl { // v2 shall return codecs supported on all physical transports
    uint8_t status;
    uint8_t num_supp_standard_codecs;
    struct hci_local_standard_codec_item standard_codec[1];
} __attribute__ ((packed));
struct hci_read_local_supp_codec_v2_tail {
    uint8_t num_supp_vendor_codecs;
    struct hci_local_vendor_codec_item vendor_codec[1];
} __attribute__ ((packed));
struct hci_read_local_supp_codecs_cmpl { // v1 only return codec supported on the BR/EDR physical transport
    uint8_t status;
    uint8_t num_supp_standard_codecs;
    uint8_t standard_codec_id[1];
} __attribute__ ((packed));
struct hci_read_local_supp_codec_tail {
    uint8_t num_supp_vendor_codecs;
    uint8_t vendor_codec_id[4];
} __attribute__ ((packed));

#define HCI_READ_LOCAL_SUPP_CODEC_CAPABILITIES  0x100E
struct hci_read_local_supp_codec_capabilities {
    uint8_t codec_id[5]; // octet 0: coding format, octet 1 to 2: company id, octet 3 to 4: vendor-defined codec id
    uint8_t logical_transport_type; // 0x00 BT ACL, 0x01 BT SCO, 0x02 LE CIS, 0x03 LE BIS
    uint8_t direction; // 0x00 input (host to controller), 0x01 output (controller to host)
} __attribute__ ((packed));
struct hci_codec_capability_t {
    uint8_t codec_capability_length;
    uint8_t codec_capability[1];
} __attribute__ ((packed));
struct hci_read_local_supp_codec_capabilities_cmpl {
    uint8_t status;
    uint8_t num_codec_capabilities;
    struct hci_codec_capability_t codec_capability[1];
} __attribute__ ((packed));

#define HCI_READ_LOCAL_SUPP_CONTROLLER_DELAY    0x100F
struct hci_read_local_controller_dealy {
    uint8_t codec_id[5]; // octet 0: coding format, octet 1 to 2: company id, octet 3 to 4: vendor-defined codec id
    uint8_t logical_transport_type; // 0x00 BT ACL, 0x01 BT SCO, 0x02 LE CIS, 0x03 LE BIS
    uint8_t direction; // 0x00 input (host to controller), 0x01 output (controller to host)
    uint8_t codec_configuration_length;
    uint8_t codec_configuration[1]; // codec-specific configuration data
} __attribute__ ((packed));
struct hci_read_local_controller_dealy_cmpl {
    uint8_t status;
    uint8_t min_controller_delay[3]; // 0x00 to 0x3D0900, 0s to 4s, min controller delay in us
    uint8_t max_controller_delay[3]; // 0x00 to 0x3D0900, 0s to 4s, max controller delay in us
} __attribute__ ((packed));

#define HCI_CONFIGURE_DATA_PATH     0x0C83
struct hci_configure_data_path { // request the Controller to config the vendor transport data path in a give dir
    uint8_t data_path_direction; // 0x00 input (host to controller), 0x01 output (controller to host)
    uint8_t vendor_data_path_id; // 0x01 to 0xFE iso over vendor-specific transport, the meaning is vendor-specific
    uint8_t vendor_config_length;
    uint8_t vendor_specific_config[1]; // additional vendor info that the Host may provided to Controller
} __attribute__ ((packed));
struct hci_configure_data_path_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_SETUP_ISO_DATA_PATH  0x206E
#define HCI_LE_REMOVE_ISO_DATA_PATH 0x206F
struct hci_le_setup_iso_data_path {
    uint16_t iso_handle; // connection handle of the CIS or BIS
    uint8_t data_path_direction; // 0x00 input (host to controller), 0x01 output (controller to host)
    uint8_t data_path_id; // 0x00 iso over hci, 0x01 to 0xFE iso over vendor-specific transport
    uint8_t codec_id[5]; // octet 0: coding format, octet 1 to 2: company id, octet 3 to 4: vendor-defined codec id
    uint8_t controller_dealy[3]; // 0x00 to 0x3D0900, 0s to 4s, controller delay in us
    uint8_t codec_configuration_length;
    uint8_t codec_configuration[1]; // codec-specific configuration data
} __attribute__ ((packed));
struct hci_le_setup_iso_data_path_cmpl {
    uint8_t status;
    uint16_t iso_handle;
} __attribute__ ((packed));
struct hci_le_remove_iso_data_path {
    uint16_t iso_handle;
    uint8_t data_path_direction; // bit 0 input, bit 1 output
} __attribute__ ((packed));
struct hci_le_remove_iso_data_path_cmpl {
    uint8_t status;
    uint16_t iso_handle;
} __attribute__ ((packed));

/**
 * The HCI_LE_Create_BIG command is used to create a BIG with one or more BISes. All BISes
 * in a BIG have the same value for all parameters. The HCI_LE_Create_BIG_Test command
 * should only be used for testing purpose.
 *
 * The BIG_Handle is the identifier of the BIG and it is allocated by the Host and used by
 * the Controller and the Host to identify a BIG. the Advertising_Handle identifies the
 * associated pa train of the BIG.
 *
 * If the Advertising_Handle does not identifiy a PA train or the PA train is associated
 * with another BIG, the Controller shall return the error code Unknown Advertising Identifier (0x42).
 *
 * If the Host issues this command with a BIG_Handle for a BIG that is already created, the
 * Controller shall return the error code Command Disallowed (0x0C).
 *
 * If the Host specifies an invalid combination of BIG parameters, the Controller shall return
 * an error Unsupporte Feature or Parameter Value (0x11).
 *
 * The HCI_LE_Terminate_BIG command is used to terminate a BIG identified by the BIG_Handle.
 * The command also terminates the transmission of all BISes of the BIG, destroys the associated
 * connection handles of the BISes in the BIG and removes the data paths for all BISes in the BIG.
 *
 * If the BIG_Handle does not identify a BIG, the Controller shall return the error code
 * Unknown Advertising Identifier (0x42). If the Controller is not the ISO Broadcaster for the
 * BIG identifified by BIG_Handle, the Controller shall return the error code Command Disallowed (0x0C).
 *
 */
#define HCI_LE_CREATE_BIG               0x2068
#define HCI_LE_CREATE_BIG_TEST          0x2069
struct hci_le_create_big {
    uint8_t big_handle; // 0x00 to 0xEF, used to identify the BIG
    uint8_t adv_handle; // 0x00 to 0xEF, used to identify the pa train
    uint8_t num_bis; // 0x01 to 0x1F, total number of BISes in the BIG
    uint8_t sdu_interval[3]; // 0xFF to 0x0F_FFFF, the interval in us of periodic SDUs
    uint16_t max_sdu; // 0x01 to 0x0FFFF, max octets of an SDU
    uint16_t max_transport_latency; // 0x05 to 0x0FA0, max transport latency in us, this includes pre-transmissions
    uint8_t rtn; // 0x00 to 0x1E, retransmition number of every BIS data PDU, just recommendation Controller may ignore
    uint8_t phy; // bit 0: transmitter phy is LE 1M, bit 1: LE 2M, bit 2: LE Coded, Host shall set at least one bit
    uint8_t packing; // 0x00 sequential, 0x01 interleaved, just recommendation
    uint8_t framing; // 0x00 unframed, 0x01 framed
    uint8_t encryption; // 0x00 unencrypted, 0x01 encrypted
    uint8_t broadcast_code[16]; // used to derive the session key that is used to encrypt and decrpt BIS payloads
} __attribute__ ((packed));
struct hci_le_create_big_test {
    uint8_t big_handle; // 0x00 to 0xEF, used to identify the BIG
    uint8_t adv_handle; // 0x00 to 0xEF, used to identify the pa train
    uint8_t num_bis; // 0x01 to 0x1F, total number of BISes in the BIG
    uint8_t sdu_interval[3]; // 0xFF to 0x0F_FFFF, the interval in us of periodic SDUs
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, interval between BIG anchor points
    uint8_t nse; // 0x01 to 0x1F, total number of subevents in each interval of each BIS in the BIG
    uint16_t max_sdu; // 0x01 to 0x0FFFF, max octets of an SDU
    uint16_t max_pdu; // 0x01 to 0xFB, max octets of PDU payload
    uint8_t phy; // bit 0: transmitter phy is LE 1M, bit 1: LE 2M, bit 2: LE Coded
    uint8_t packing; // 0x00 sequential, 0x01 interleaved
    uint8_t framing; // 0x00 unframed, 0x01 framed
    uint8_t bn; // 0x01 to 0x07, number of new payloads in each interval for each BIS
    uint8_t irc; // 0x01 to 0x0F, number of times the scheduled payloads are transmitted in a given event
    uint8_t pto; // 0x00 to 0x0F, offset used for pre-transmissions
    uint8_t encryption; // 0x00 unencrypted, 0x01 encrypted
    uint8_t broadcast_code[16]; // used to derive the session key that is used to encrypt and decrpt BIS payloads
} __attribute__ ((packed));

#define HCI_LE_EV_CREATE_BIG_COMPLETE 0x1B
struct hci_ev_le_create_big_complete {
    uint8_t subcode;
    uint8_t status;
    uint8_t big_handle; // 0x00 to 0xEF, the identifier of the BIG
    uint8_t big_sync_delay[3]; // 0xEA to 0x7F_FFFF, max time in us for transmission of PDUs of all BISes in a BIG event
    uint8_t transport_latency_big[3]; // 0xEA to 0x7F_FFFF, actual transport latency, in us
    uint8_t phy; // 0x01 the phy used to create the BIG is LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint8_t nse; // 0x01 to 0x1F, num of subevents in each BIS event in the BIG
    uint8_t bn; // 0x01 to 0x07, the number of new payloads in each BIS event
    uint8_t pto; // 0x00 to 0x0F, offset used for pre-transmissions
    uint8_t irc; // 0x01 to 0x0F, num of times a payload is transmitted in a BIS event
    uint16_t max_pdu; // 0x01 to 0xFB, max octets of the PDU payload
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, BIG anchor points interval
    uint8_t num_bis; // 0x01 to 0x1F, total number of BISes in the BIG
    uint16_t bis_handle[1];
} __attribute__ ((packed));

#define HCI_LE_TERMINATE_BIG            0x206A
struct hci_le_terminate_big {
    uint8_t big_handle;
    uint8_t reason;
} __attribute__ ((packed));

#define HCI_LE_EV_TERMINATE_BIG_COMPLETE 0x1C
struct hci_ev_le_terminate_big_complete {
    uint8_t subcode;
    uint8_t big_handle;
    uint8_t reason;
} __attribute__ ((packed));

/**
 * The HCI_LE_BIG_Create_Sync command is used to sync to a BIG described in the pa train
 * specified by the Sync_Handle parameter.
 *
 * The BIG_Handle parameter is assigned by the Host to identify the synchronized BIG.
 * If the Host sends this command with a BIG_Handle that is already allocated, the Controller
 * shall return the error code Command Disallowed (0x0C).
 *
 * If the Encryption paramter set by the Host does not match the encryption status of
 * the BIG, the Controller shall return the error Encryption Mode Not Acceptable (0x25).
 *
 * The MSE (Maximum Subevents) parameter is the max number of subevents that a Controller
 * should use to receive data payloads in each interval for a BIS.
 *
 * The BIG_Sync_Timeout parameter specifies the max permitted time between successful
 * receptions of BIS PDUs. If the time is exceeded, sync is lost. When the Controller
 * establishes sync and if the BIG_Sync_Timeout set by the Host is less than 6 * ISO_Interval,
 * the Controller shall set the timeout to 6 * ISO_Interval.
 *
 * The number of BISes requested may be a subset of the BISes available in the BIG. If
 * the Num_BIS parameter is greater than the total number of BISes in the BIG, the
 * Controller shall return the error code Unsupported Feature or Parameter Value (0x11).
 *
 * The BIS_Index is a list of indices corresponding to BISes in the synchronized BIG.
 * The list of BIS indices shall be in ascending order and shall not contain any duplicates.
 * This list of specified BISes may be all or a subset of BISes available in the BIG.
 *
 * If the Sync_Handle does not exist, the Controller shall return the error code Unknown
 * Advertising Identifier (0x42).
 *
 * If the info describing the BIG does not specify a PHY supported by the Controller or
 * does not specify exactly one PHY, the Controller shall return the error code
 * Unsupported Feature or Parameter Value (0x11).
 *
 * If the Host sends this command when the Controller is in the process of synchronizing to
 * any BIG. i.e., a previous HCI_LE_BIG_Create_Sync is sent but the HCI_LE_BIG_Sync_Established
 * event has not been generated yet, the Controller shall return the error code Command
 * Disallowed (0x0C).
 *
 * If the Controller is unalbe to receive PDUs from the specified number of BISes in
 * the syncronized BIG, it shall return the error code Connection Rejected Due to Limited
 * Resources (0x0D).
 *
 * The HCI_LE_BIG_Terminate_Sync command is used to stop synchronizing or cancel the process
 * of synchronizing to the BIG identified by the BIG Handle parameter. The command also
 * terminates the reception of BISes in the BIG specified in the HCI_LE_BIG_Create_Sync,
 * destroys the associated connection handles of the BISes in the BIG and removes the data
 * paths for all BISes in the BIG.
 *
 * The HCI_LE_BIG_Sync_Lost event indicates that the Controller has not received any PDUs
 * on a BIG within the timeout period BIG_Sync_Timeout or the BIS has been terminated by
 * the remote device.
 *
 * The Reason parameter is used to indicate the reason why the synchronization was lost
 * or terminated. If synchronization was terminated due to the Broadcaster terminating the
 * BIG, the Reason shall be set to Remote User Terminated Connection (0x13). If synchronization
 * was terminated due to a timeout, the Reason shall be set to Connection Timeout (0x08).
 * If the synchronization was terminated due to a MIC failure, the Reason shall be set to
 * Connection Terminated due to MIC Failure (0x3D).
 *
 * When the HCI_LE_BIG_Sync_Lost event occurs, the Controller shall remove the connection
 * handles and data paths of all BISes in the BIG with which the Controller was synchronized.
 *
 */
#define HCI_LE_EV_BIG_INFO_ADV_REPORT 0x22
struct hci_ev_le_big_info_adv_report { // shall be generated even if the Controller already sync to the BIG
    uint8_t subcode;
    uint16_t sync_handle; // 0x0000 to 0x0EFE, sync_handle identifying the pa train
    uint8_t num_bis; // 0x01 to 0x1F, value of the Num_BIS subfield of the BIGInfo field
    uint8_t nse; // 0x01 to 0x1F, value of the NSE subfield of the BIGInfo field
    uint16_t iso_interval; // iso_interval subfield of the BIGInfo field
    uint8_t bn; // 0x01 to 0x07, BN subfield of the BIGInfo field
    uint8_t pto; // 0x00 to 0x0F, pre-transmission offset, PTO subfield of the BIGInfo field
    uint8_t irc; // 0x01 to 0x0F, IRC subfield of the BIGInfo field
    uint16_t max_pdu; // 0x01 to 0xFB, max_pdu subfiled of the BIGInfo
    uint8_t sdu_interval[3]; // 0xFF to 0x0F_FFFF, sdu_interval subfield of the BIGInfo field
    uint16_t max_sdu; // 0x01 to 0x0FFF, max_sdu subfield of the BIGInfo
    uint8_t phy; // 0x01 the BIG is transmitted on the LE 1M PHY, 0x02 LE 2M, 0x03 LE Coded
    uint8_t framing; // 0x00 unframed 0x01 framed
    uint8_t encrypt; // 0x00 BIG carries unencrypted data, 0x01 BIG carries encrypted data
} __attribute__ ((packed));

#define HCI_LE_BIG_CREATE_SYNC          0x206B
struct hci_le_big_create_sync {
    uint8_t big_handle; // 0x00 to 0xEF, used to identify the BIG
    uint16_t sync_handle; // 0x0000 to 0x0EFF, identifier of the pa train
    uint8_t encryption; // 0x00 unencrypted, 0x01 encrypted
    uint8_t broadcast_code[16]; // used for deriving the session key for decrypting payloads of BISes in the BIG
    uint8_t mse; // 0x00 controler can schedule reception of any num of se up to NSE, 0x01 to 0x1F max num of se should be used
    uint16_t big_sync_timeout; // 0x0A to 0x4000, per 10ms, 100ms to 163.84s, sync timeout for the BIG
    uint8_t num_bis; // 0x01 to 0x1F, total number of BISes to synchronize
    uint8_t bis_index[1]; // 0x01 to 0x1F, index of a BIS in the BIG
} __attribute__ ((packed));

#define HCI_LE_EV_BIG_SYNC_ESTABLISHED 0x1D
struct hci_ev_le_big_sync_established {
    uint8_t subcode;
    uint8_t status;
    uint8_t big_handle; // 0x00 to 0xEF, the identifier of the BIG
    uint8_t transport_latency_big[3]; // 0xEA to 0x7F_FFFF, the actual transport latency in us
    uint8_t nse; // 0x01 to 0x1F, num of subevents in each BIS event in the BIG
    uint8_t bn; // 0x01 to 0x07, the number of new payloads in each BIS event
    uint8_t pto; // 0x00 to 0x0F, offset used for pre-transmissions
    uint8_t irc; // 0x01 to 0x0F, num of times a payload is transmitted in a BIS event
    uint16_t max_pdu; // 0x01 to 0xFB, max octets of the PDU payload
    uint16_t iso_interval; // 0x0004 to 0x0C80, per 1.25ms, 5ms to 4s, time between two BIG anchor points
    uint8_t num_bis; // 0x01 to 0x1F, total number of established BISes, shall be same in LE_BIG_Create_Sync command
    uint16_t bis_handle[1];
} __attribute__ ((packed));

#define HCI_LE_BIG_TERMINATE_SYNC       0x206C
struct hci_le_big_terminate_sync {
    uint8_t big_handle;
} __attribute__ ((packed));
struct hci_le_big_terminate_sync_cmpl {
    uint8_t status;
    uint8_t big_handle;
} __attribute__ ((packed));

#define HCI_LE_EV_BIG_SYNC_LOST 0x1E
struct hci_ev_le_big_sync_lost {
    uint8_t subcode;
    uint8_t big_handle;
    uint8_t reason;
} __attribute__ ((packed));

#ifdef IBRT
typedef void (*hci_tx_buf_tss_process_cb_type)(void);
typedef void (*bt_hci_acl_ecc_softbit_handler_func)(uint16_t*,uint16_t*, uint16_t, uint8_t*);
void register_hci_acl_ecc_softbit_handler_callback(bt_hci_acl_ecc_softbit_handler_func func);
#endif

bt_status_t hci_simulate_event(const uint8_t *buff, uint16_t buff_len);
bt_status_t hci_send_cmd_direct(const uint8_t *cmd_packet, uint8_t packet_len);
void hci_stop_rx_bt_acl_data(uint16_t connhdl, bool stop);

#if defined(__cplusplus)
}
#endif
#endif /* __HCI_I_H__ */
