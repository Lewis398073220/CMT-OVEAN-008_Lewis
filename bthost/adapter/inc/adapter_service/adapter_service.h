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
#ifndef __BT_ADAPTER_LAYER_H__
#define __BT_ADAPTER_LAYER_H__
#include "bt_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef int bt_status_t;
#define BT_STS_SUCCESS                      0   /* Host error code: Successful and complete */
#define BT_STS_FAILED                       1   /* Operation failed */
#define BT_STS_PENDING                      2   /* Successfully started but pending */
#define BT_STS_DISCONNECT                   3   /* Link disconnected */
#define BT_STS_NO_LINK                      4   /* No Link layer Connection exists */
#define BT_STS_IN_USE                       5   /* Operation failed - already in use. */
#define BT_STS_MEDIA_BUSY                   6   /* IrDA specific return codes: Media is busy */
#define BT_STS_MEDIA_NOT_BUSY               7   /* IRDA: Media is not busy */
#define BT_STS_NO_PROGRESS                  8   /* IRDA: IrLAP not making progress */
#define BT_STS_LINK_OK                      9   /* IRDA: No progress condition cleared */
#define BT_STS_SDU_OVERRUN                  10  /* IRDA: Sent more data than current SDU size */
#define BT_STS_BUSY                         11  /* Bluetooth specific return codes */
#define BT_STS_NO_RESOURCES                 12
#define BT_STS_NOT_FOUND                    13
#define BT_STS_DEVICE_NOT_FOUND             14
#define BT_STS_CONNECTION_FAILED            15
#define BT_STS_TIMEOUT                      16
#define BT_STS_NO_CONNECTION                17
#define BT_STS_INVALID_PARM                 18
#define BT_STS_IN_PROGRESS                  19
#define BT_STS_RESTRICTED                   20
#define BT_STS_INVALID_TYPE                 21
#define BT_STS_HCI_INIT_ERR                 22
#define BT_STS_NOT_SUPPORTED                23
#define BT_STS_CONTINUE                     24
#define BT_STS_CANCELLED                    25
#define BT_STS_NOT_READY                    200
#define BT_STS_QUERY_FAILED                 201
#define BT_STS_ALREADY_EXIST                202
#define BT_STS_UNKNOWN_HCI_COMMAND          0x01 /* Bluetooth Controller error codes */
#define BT_STS_UNKNOWN_CONNECTION           0x02
#define BT_STS_HARDWARE_FAILURE             0x03
#define BT_STS_PAGE_TIMEOUT                 0x04
#define BT_STS_AUTH_FAILURE                 0x05
#define BT_STS_MISSING_KEY                  0x06
#define BT_STS_MEMORY_FULL                  0x07
#define BT_STS_CONN_TIMEOUT                 0x08
#define BT_STS_MAX_CONNECTIONS              0x09
#define BT_STS_MAX_SCO_CONNECTIONS          0x0a
#define BT_STS_ACL_ALREADY_EXISTS           0x0b
#define BT_STS_COMMAND_DISALLOWED           0x0c
#define BT_STS_LIMITED_RESOURCE             0x0d
#define BT_STS_SECURITY_ERROR               0x0e
#define BT_STS_PERSONAL_DEVICE              0x0f
#define BT_STS_HOST_TIMEOUT                 0x10
#define BT_STS_UNSUPPORTED_FEATURE          0x11
#define BT_STS_INVALID_HCI_PARM             0x12
#define BT_STS_USER_TERMINATED              0x13
#define BT_STS_LOW_RESOURCES                0x14
#define BT_STS_POWER_OFF                    0x15
#define BT_STS_LOCAL_TERMINATED             0x16
#define BT_STS_REPEATED_ATTEMPTS            0x17
#define BT_STS_PAIRING_NOT_ALLOWED          0x18
#define BT_STS_UNKNOWN_LMP_PDU              0x19
#define BT_STS_UNSUPPORTED_REMOTE           0x1a
#define BT_STS_SCO_OFFSET_REJECT            0x1b
#define BT_STS_SCO_INTERVAL_REJECT          0x1c
#define BT_STS_SCO_AIR_MODE_REJECT          0x1d
#define BT_STS_INVALID_LMP_PARM             0x1e
#define BT_STS_UNSPECIFIED_ERR              0x1f
#define BT_STS_UNSUPPORTED_LMP_PARM         0x20
#define BT_STS_ROLE_CHG_NOT_ALLOWED         0x21
#define BT_STS_LMP_RESPONSE_TIMEOUT         0x22
#define BT_STS_LMP_TRANS_COLLISION          0x23
#define BT_STS_LMP_PDU_NOT_ALLOWED          0x24
#define BT_STS_ENCRYP_MODE_NOT_ACC          0x25
#define BT_STS_UNIT_KEY_USED                0x26
#define BT_STS_QOS_NOT_SUPPORTED            0x27
#define BT_STS_INSTANT_PASSED               0x28
#define BT_STS_PAIR_UNITKEY_NO_SUPP         0x29
#define BT_STS_DIFF_TRANS_COLLISION         0x2a
#define BT_STS_QOS_UNACC_PARAMETER          0x2c
#define BT_STS_QOS_REJECTED                 0x2d
#define BT_STS_CHANASSESS_NOT_SUPP          0x2e
#define BT_STS_INSUFFICIENT_SECURITY        0x2f
#define BT_STS_PARAM_OUT_OF_RANGE           0x30
#define BT_STS_ROLE_SWITCH_PENDING          0x32
#define BT_STS_RESERVED_SLOT_VIOLATION      0x34
#define BT_STS_ROLE_SWITCH_FAILED           0x35
#define BT_STS_EXT_INQ_RSP_TOO_LARGE        0x36
#define BT_STS_SSP_NOT_SUPP_BY_HOST         0x37
#define BT_STS_HOST_BUSY_PAIRING            0x38
#define BT_STS_NO_SUITABLE_CHAN_FOUND       0x39
#define BT_STS_CONTROLLER_BUSY              0x3a
#define BT_STS_UNACC_CONN_PARAMETER         0x3b
#define BT_STS_ADVERTISING_TIMEOUT          0x3c
#define BT_STS_MIC_FAILURE                  0x3d
#define BT_STS_CONN_FAILED_TO_ESTABLISH     0x3e
#define BT_STS_COARSE_CLOCK_ADJ_REJECTED    0x40
#define BT_STS_TYPE0_SUBMAP_NOT_DEFINED     0x41
#define BT_STS_UNKNOWN_ADV_IDENTIFIER       0x42
#define BT_STS_LIMIT_REACHED                0x43
#define BT_STS_OPER_CANCELLED_BY_HOST       0x44
#define BT_STS_PACKET_TOO_LONG              0x45
#define BT_STS_REAL_LINK_DISCONNECTED       0xb8
#define BT_STS_BT_CANCEL_PAGE               0xb9
#define BT_STS_BT_DISCONNECT_ITSELF         0xba

typedef enum {
    BT_GROUP_INDEX_LINK = 0,
    BT_GROUP_INDEX_HFP_HF,
    BT_GROUP_INDEX_HFP_AG,
    BT_GROUP_INDEX_A2DP_SNK,
    BT_GROUP_INDEX_A2DP_SRC,
    BT_GROUP_INDEX_AVRCP,
    BT_GROUP_INDEX_HID,
    BT_GROUP_INDEX_MAP,
    BT_GROUP_INDEX_PBAP,
    BT_GROUP_INDEX_DIP,
    BT_GROUP_INDEX_PAN,
    BT_GROUP_INDEX_OPP,
    BT_GROUP_INDEX_SPP,
} bt_event_group_index_t;

#define BT_EVENT_ID_START_BASE 0x1000
#define BT_EVENT_ID_GROUP_DIFF 0x0100
#define BT_EVENT_ID_GROUP_START(idx) (BT_EVENT_ID_START_BASE + (idx) * BT_EVENT_ID_GROUP_DIFF)
#define BT_EVENT_TO_GROUP_MASK(evid) (1 << (((evid&0xff00) - BT_EVENT_ID_START_BASE) / BT_EVENT_ID_GROUP_DIFF))

#define BT_EVENT_MASK_LINK_GROUP        (1<<BT_GROUP_INDEX_LINK)
#define BT_EVENT_MASK_HFP_HF_GROUP      (1<<BT_GROUP_INDEX_HFP_HF)
#define BT_EVENT_MASK_HFP_AG_GROUP      (1<<BT_GROUP_INDEX_HFP_AG)
#define BT_EVENT_MASK_A2DP_SNK_GROUP    (1<<BT_GROUP_INDEX_A2DP_SNK)
#define BT_EVENT_MASK_A2DP_SRC_GROUP    (1<<BT_GROUP_INDEX_A2DP_SRC)
#define BT_EVENT_MASK_AVRCP_GROUP       (1<<BT_GROUP_INDEX_AVRCP)
#define BT_EVENT_MASK_HID_GROUP         (1<<BT_GROUP_INDEX_HID)
#define BT_EVENT_MASK_MAP_GROUP         (1<<BT_GROUP_INDEX_MAP)
#define BT_EVENT_MASK_PBAP_GROUP        (1<<BT_GROUP_INDEX_PBAP)
#define BT_EVENT_MASK_DIP_GROUP         (1<<BT_GROUP_INDEX_DIP)
#define BT_EVENT_MASK_PAN_GROUP         (1<<BT_GROUP_INDEX_PAN)
#define BT_EVENT_MASK_OPP_GROUP         (1<<BT_GROUP_INDEX_OPP)
#define BT_EVENT_MASK_SPP_GROUP         (1<<BT_GROUP_INDEX_SPP)

typedef enum {
    BT_EVENT_INVALID_EVENT = 0,

    BT_EVENT_ACL_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_LINK), // 0x1000
    BT_EVENT_ACL_CLOSED,
    BT_EVENT_SCO_OPENED,
    BT_EVENT_SCO_CLOSED,
    BT_EVENT_ACCESS_CHANGE,
    BT_EVENT_ROLE_DISCOVER,
    BT_EVENT_ROLE_CHANGE,
    BT_EVENT_MODE_CHANGE,
    BT_EVENT_AUTHENTICATED,
    BT_EVENT_ENC_CHANGE,
    BT_EVENT_INQUIRY_RESULT,
    BT_EVENT_INQUIRY_COMPLETE,
    BT_EVENT_BOND_STATE_CHANGE,
    BT_EVENT_PROPERTY_STATE,
    BT_EVENT_LINK_END,

    BT_EVENT_HF_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_HFP_HF), // 0x1100
    BT_EVENT_HF_CLOSED,
    BT_EVENT_HF_AUDIO_OPENED,
    BT_EVENT_HF_AUDIO_CLOSED,
    BT_EVENT_HF_NETWORK_STATE,
    BT_EVENT_HF_NETWORK_ROAMING,
    BT_EVENT_HF_NETWORK_SIGNAL,
    BT_EVENT_HF_BATTERY_LEVEL,
    BT_EVENT_HF_CALL_IND,
    BT_EVENT_HF_CALLSETUP_IND,
    BT_EVENT_HF_CALLHELD_IND,
    BT_EVENT_HF_RING_IND,
    BT_EVENT_HF_CLIP_IND,
    BT_EVENT_HF_CALL_WAITING_IND,
    BT_EVENT_HF_RESP_AND_HOLD,
    BT_EVENT_HF_CURRENT_CALLS,
    BT_EVENT_HF_VOLUME_CHANGE,
    BT_EVENT_HF_AT_CMD_COMPLETE,
    BT_EVENT_HF_CURRENT_OPERATOR,
    BT_EVENT_HF_SUBSCRIBER_INFO,
    BT_EVENT_HF_VOICE_RECOGNITION_STATE,
    BT_EVENT_HF_IN_BAND_RING_STATE,
    BT_EVENT_HF_LAST_VOICE_TAG_NUMBER,
    BT_EVENT_HF_RECEIVE_UNKNOWN,
    BT_EVENT_HF_END,

    BT_EVENT_AG_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_HFP_AG), // 0x1200
    BT_EVENT_AG_CLOSED,
    BT_EVENT_AG_AUDIO_OPENED,
    BT_EVENT_AG_AUDIO_CLOSED,
    BT_EVENT_AG_VOLUME_CONTROL,
    BT_EVENT_AG_WBS_STATE,
    BT_EVENT_AG_BIA_IND,
    BT_EVENT_AG_BIEV_IND,
    BT_EVENT_AG_DIAL_CALL_REQ,
    BT_EVENT_AG_DIAL_MEMORY_REQ,
    BT_EVENT_AG_CALL_ACTION_REQ,
    BT_EVENT_AG_AT_DTMF_REQ,
    BT_EVENT_AG_AT_BVRA_REQ,
    BT_EVENT_AG_AT_NREC_REQ,
    BT_EVENT_AG_AT_CNUM_REQ,
    BT_EVENT_AG_AT_CIND_REQ,
    BT_EVENT_AG_AT_COPS_REQ,
    BT_EVENT_AG_AT_CLCC_REQ,
    BT_EVENT_AG_AT_CKPD_REQ,
    BT_EVENT_AG_AT_BIND_REQ,
    BT_EVENT_AG_UNKNOWN_AT_REQ,
    BT_EVENT_AG_END,

    BT_EVENT_A2DP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_A2DP_SNK), // 0x1300
    BT_EVENT_A2DP_CLOSED,
    BT_EVENT_A2DP_STREAM_START,
    BT_EVENT_A2DP_STREAM_RECONFIG,
    BT_EVENT_A2DP_STREAM_SUSPEND,
    BT_EVENT_A2DP_STREAM_CLOSE,
    BT_EVENT_A2DP_CUSTOM_CMD_REQ,
    BT_EVENT_A2DP_CUSTOM_CMD_RSP,
    BT_EVENT_A2DP_END,

    BT_EVENT_A2DP_SOURCE_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_A2DP_SRC), // 0x1400
    BT_EVENT_A2DP_SOURCE_CLOSED,
    BT_EVENT_A2DP_SOURCE_STREAM_START,
    BT_EVENT_A2DP_SOURCE_STREAM_RECONFIG,
    BT_EVENT_A2DP_SOURCE_STREAM_SUSPEND,
    BT_EVENT_A2DP_SOURCE_STREAM_CLOSE,
    BT_EVENT_A2DP_SOURCE_END,

    BT_EVENT_AVRCP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_AVRCP), // 0x1500
    BT_EVENT_AVRCP_CLOSED,
    BT_EVENT_AVRCP_VOLUME_CHANGE,
    BT_EVENT_AVRCP_PLAY_STATUS_CHANGE,
    BT_EVENT_AVRCP_PLAY_POS_CHANGE,
    BT_EVENT_AVRCP_TRACK_CHANGE,
    BT_EVENT_AVRCP_BATTERY_CHANGE,
    BT_EVENT_AVRCP_RECV_PASSTHROUGH_CMD,
    BT_EVENT_AVRCP_RECV_PLAY_STATUS,
    BT_EVENT_AVRCP_RECV_MEDIA_STATUS,
    BT_EVENT_AVRCP_END,

    BT_EVENT_HID_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_HID),  // 0x1600
    BT_EVENT_HID_CLOSED,
    BT_EVENT_HID_SENSOR_STATE_CHANGED,
    BT_EVENT_HID_END,

    BT_EVENT_MAP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_MAP), // 0x1700
    BT_EVENT_MAP_CLOSED,
    BT_EVENT_MAP_FOLDER_LISTING_RSP,
    BT_EVENT_MAP_MSG_LISTING_ITEM_RSP,
    BT_EVENT_MAP_GET_MSG_RSP,
    BT_EVENT_MAP_PUT_MSG_RSP,
    BT_EVENT_MAP_READ_STATUS_CHANGED,
    BT_EVENT_MAP_END,

    BT_EVENT_SPP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_SPP),
    BT_EVENT_SPP_CLOSED,
    BT_EVENT_SPP_END = BT_EVENT_SPP_OPENED + 5,
} BT_EVENT_T;

#define BT_SOCKET_MAX_TX_PEND_PACKETS (6) // bt controller max support 6 acl data buffer

typedef struct bt_socket_t bt_socket_t;
typedef struct bt_service_port_t bt_service_port_t;
typedef struct bt_sdp_record_attr_t bt_sdp_record_attr_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
    bt_socket_t *socket;
} bt_socket_opened_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
    uint16_t remote_server_channel;
    bt_socket_t *socket;
    bt_service_port_t *port;
    void *sock_priv;
} bt_socket_closed_t;

typedef struct {
    bt_socket_t *socket;
    void *tx_priv;
} bt_socket_tx_done_t;

typedef struct {
    bt_socket_t *socket;
    const uint8_t *data;
    uint16_t len;
} bt_socket_rx_data_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
    uint16_t dlci;
    uint16_t local_server_channel;
    uint32_t channel_handle;
    void *set_sock_priv;
} bt_socket_accept_t;

typedef union {
    void *param_ptr;
    bt_socket_opened_t *opened;
    bt_socket_closed_t *closed;
    bt_socket_tx_done_t *tx_done;
    bt_socket_rx_data_t *rx_data;
    bt_socket_accept_t *accept;
} bt_socket_callback_param_t;

typedef enum {
    BT_SOCKET_EVENT_OPENED = BT_EVENT_SPP_OPENED,
    BT_SOCKET_EVENT_CLOSED,
    BT_SOCKET_EVENT_TX_DONE,
    BT_SOCKET_EVENT_RX_DATA,
    BT_SOCKET_EVENT_ACCEPT,
} bt_socket_event_t;

typedef enum {
    BT_SOCKET_STATE_IDLE = 0,
    BT_SOCKET_STATE_CLOSED,
    BT_SOCKET_STATE_CREATING_SESSION,
    BT_SOCKET_STATE_CONNECTING,
    BT_SOCKET_STATE_OPENED,
    BT_SOCKET_STATE_DISCONNECTING,
} bt_socket_state_t;

typedef int (*bt_socket_callback_t)(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_callback_param_t param);
typedef int (*bt_socket_accept_callback_t)(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_accept_t *accept);

typedef struct bt_service_port_t {
    uint16_t local_server_channel;
    bool server_accept_multi_device_request;
    bool upper_layer_give_credit;
    uint16_t initial_credits;
    uint16_t credit_give_step;
    uint16_t attr_count;
    const bt_sdp_record_attr_t *attr_list;
    bt_socket_callback_t socket_callback;
    bt_socket_accept_callback_t socket_accept_callback;
} bt_service_port_t;

typedef struct bt_socket_t {
    uint8_t device_id;
    bt_socket_state_t state;
    bt_bdaddr_t remote;
    uint32_t channel_handle;
    bool is_initiator;
    uint16_t dlci;
    uint16_t tx_mtu;
    uint16_t rx_credits;
    uint16_t tx_credits;
    uint16_t free_tx_packets; // number of packets currently can be delivered to socket with each packet size <= tx_mtu
    uint16_t remote_server_channel;
    bt_service_port_t *port;
    void *sock_priv;
} bt_socket_t;

bt_status_t bt_socket_create_port(uint16_t local_server_channel, bt_socket_callback_t socket_callback, const bt_sdp_record_attr_t *attr_list, uint16_t attr_count);
bt_status_t bt_socket_set_rx_credits(uint16_t local_server_channel, uint16_t rx_buff_size, bool upper_layer_give_credit, uint16_t initial_credits, uint16_t credit_give_step);
bt_status_t bt_socket_listen(uint16_t local_server_channel, bool support_multi_device, bt_socket_accept_callback_t accept_callback);
bt_status_t bt_socket_remove_listen(uint16_t local_server_channel);
bt_status_t bt_socket_connect(const bt_bdaddr_t *remote, uint16_t local_server_channel, const uint8_t *uuid, uint16_t uuid_len, void *sock_priv);
bt_status_t bt_socket_connect_server_channel(const bt_bdaddr_t *remote, uint16_t local_server_channel, uint16_t remote_server_channel, void *sock_priv);
bt_status_t bt_socket_give_credits(uint32_t channel_handle, uint16_t handled_credits);
bt_status_t bt_socket_disconnect(uint32_t channel_handle, uint8_t reason);
bt_status_t bt_socket_write(uint32_t channel_handle, const uint8_t *data, uint16_t size);
uint16_t bt_socket_count_free_tx_packets(uint32_t channel_handle);
bool bt_socket_is_connected(uint32_t channel_handle);

#include "bt_hci_service.h"
#include "bt_sdp_service.h"
#include "bt_a2dp_service.h"
#include "bt_avrcp_service.h"
#include "bt_hfp_service.h"
#include "bt_spp_service.h"
#include "bt_hid_service.h"
#include "bt_map_service.h"

typedef enum {
    BT_PROPERTY_TYPE_LOCAL_BDNAME = 0x01,   // only set
    BT_PROPERTY_TYPE_LOCAL_BDADDR,          // only set
    BT_PROPERTY_TYPE_ACCESS_MODE,           // only set
    BT_PROPERTY_TYPE_REMOTE_NAME,           // only get
} bt_property_data_type_t;

typedef enum {
    BT_MODE_ACTIVE_MODE = 0x00,
    BT_MODE_HOLD_MODE   = 0x01,
    BT_MODE_SNIFF_MODE  = 0x02,
    BT_MODE_PARK_MODE   = 0x03,
} bt_link_mode_t;

typedef enum {
    BT_ROLE_MASTER  = 0x00,
    BT_ROLE_SLAVE   = 0x01,
} bt_link_role_t;

typedef enum {
    BT_ACCESS_NOT_ACCESSIBLE        = 0x00,
    BT_ACCESS_DISCOVERABLE_ONLY     = 0x01,
    BT_ACCESS_CONNECTABLE_ONLY      = 0x02,
    BT_ACCESS_GENERAL_ACCESSIBLE    = 0x03,
    BT_ACCESS_LIMITED_ACCESSIBLE    = 0x13,
} bt_access_mode_t;

typedef struct {
    uint8_t error_code;
    uint8_t acl_bt_role;
    uint16_t conn_handle;
    uint8_t device_id;
    uint8_t local_is_source;
} bt_adapter_acl_opened_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t disc_reason;
    uint16_t conn_handle;
    uint8_t device_id;
} bt_adapter_acl_closed_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t codec;
    uint16_t sco_handle;
} bt_adapter_sco_opened_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t disc_reason;
    uint16_t sco_handle;
} bt_adapter_sco_closed_param_t;

typedef struct {
    bt_access_mode_t access_mode;
} bt_adapter_access_change_param_t;

typedef struct {
    uint8_t error_code;
    bt_link_role_t acl_bt_role;
} bt_adapter_role_discover_param_t;

typedef struct {
    uint8_t error_code;
    bt_link_role_t acl_bt_role;
} bt_adapter_role_change_param_t;

typedef struct {
    uint8_t error_code;
    bt_link_mode_t acl_link_mode;
    uint16_t sniff_interval;
} bt_adapter_mode_change_param_t;

typedef struct {
    uint8_t error_code;
} bt_adapter_authenticated_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t encrypted;
} bt_adapter_enc_change_param_t;

#define BT_RSSI_INVALID_VALUE 127

typedef struct {
    bt_bdaddr_t remote;
    uint8_t page_scan_repeat_mode;
    int8_t rssi;
    uint16_t clock_offset;
    uint32_t class_of_device;
    uint8_t *eir;
} bt_adapter_inquiry_result_param_t;

typedef struct {
    uint8_t error_code;
} bt_adapter_inquiry_complete_param_t;

typedef struct {
    bool is_bonded;
} bt_adapter_bond_state_change_param_t;

typedef struct {
    bt_property_data_type_t type;
    int len;
    uintptr_t data;
} bt_adapter_property_param_t;

typedef union {
    bt_adapter_acl_opened_param_t *acl_opened;
    bt_adapter_acl_closed_param_t *acl_closed;
    bt_adapter_sco_opened_param_t *sco_opened;
    bt_adapter_sco_closed_param_t *sco_closed;
    bt_adapter_access_change_param_t *access_change;
    bt_adapter_role_discover_param_t *role_discover;
    bt_adapter_role_change_param_t *role_change;
    bt_adapter_mode_change_param_t *mode_change;
    bt_adapter_authenticated_param_t *authenticated;
    bt_adapter_enc_change_param_t *enc_change;
    bt_adapter_inquiry_result_param_t *inq_result;
    bt_adapter_inquiry_complete_param_t *inq_complete;
    bt_adapter_bond_state_change_param_t *bond_change;
    bt_adapter_property_param_t *perperty_state;
} bt_adapter_callback_param_t;

typedef enum {
    BT_ADAPTER_EVENT_ACL_OPENED = BT_EVENT_ACL_OPENED,
    BT_ADAPTER_EVENT_ACL_CLOSED,
    BT_ADAPTER_EVENT_SCO_OPENED,
    BT_ADAPTER_EVENT_SCO_CLOSED,
    BT_ADAPTER_EVENT_ACCESS_CHANGE,
    BT_ADAPTER_EVENT_ROLE_DISCOVER,
    BT_ADAPTER_EVENT_ROLE_CHANGE,
    BT_ADAPTER_EVENT_MODE_CHANGE,
    BT_ADAPTER_EVENT_AUTHENTICATED,
    BT_ADAPTER_EVENT_ENC_CHANGE,
    BT_ADAPTER_EVENT_INQUIRY_RESULT,
    BT_ADAPTER_EVENT_INQUIRY_COMPLETE,
    BT_ADAPTER_EVENT_BOND_STATE_CHANGE,
    BT_ADAPTER_EVENT_END,
} bt_adapter_event_t;

#if BT_ADAPTER_EVENT_END != BT_EVENT_LINK_END
#error "bt_adapter_event_t error define"
#endif

typedef int (*bt_adapter_callback_t)(const bt_bdaddr_t *bd_addr, bt_adapter_event_t event, bt_adapter_callback_param_t param);

bt_status_t bt_adapter_init(bt_adapter_callback_t callback);
bt_status_t bt_adapter_cleanup(void);
bt_status_t bt_adapter_start_inquiry(void);
bt_status_t bt_adapter_cancel_inquiry(void);
bt_status_t bt_adapter_connect_acl(const bt_bdaddr_t *bd_addr);
bt_status_t bt_adapter_disconnect_acl(const bt_bdaddr_t *bd_addr);
bt_status_t bt_adapter_create_bond(const bt_bdaddr_t *bd_addr);
bt_status_t bt_adapter_remove_bond(const bt_bdaddr_t *bd_addr);
bt_status_t bt_adapter_get_property(const bt_bdaddr_t *bd_addr, bt_property_data_type_t type);
bt_status_t bt_adapter_set_property(const bt_adapter_property_param_t *property);
bt_status_t bt_adapter_write_sleep_enable(bool enable);

typedef union {
    void *param_ptr;
    bt_adapter_callback_param_t bt;
#ifdef BT_HFP_SUPPORT
    bt_hf_callback_param_t hf;
#ifdef BT_HFP_AG_ROLE
    bt_ag_callback_param_t ag;
#endif
#endif
#ifdef BT_A2DP_SUPPORT
    bt_a2dp_callback_param_t av;
#endif
#ifdef BT_AVRCP_SUPPORT
    bt_avrcp_callback_param_t ar;
#endif
#ifdef BT_HID_DEVICE
    bt_hid_callback_param_t hid;
#endif
#ifdef BT_MAP_SUPPORT
    bt_map_callback_param_t map;
#endif
} BT_CALLBACK_PARAM_T;

typedef int (*bt_event_callback_t)(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param);

struct BT_ADAPTER_DEVICE_T {
    bt_bdaddr_t remote;
    uint8_t device_id;
    bool acl_is_connected;
    uint8_t acl_bt_role;
    uint8_t acl_link_mode;
    uint16_t acl_conn_hdl;
    uint16_t sniff_interval;

    bool sco_is_connected;
    uint8_t sco_codec_type;
    uint16_t sco_handle;

    bool hfp_is_connected;
    uint8_t hfp_call_state;
    uint8_t hfp_callsetup_state;
    uint8_t hfp_callhold_state;
    uint8_t hfp_speak_vol;

    bool avrcp_is_connected;
    bool a2dp_is_connected;
    bool a2dp_is_streaming;
    uint8_t a2dp_codec_type;
    uint8_t a2dp_sample_rate;

    uint8_t is_use_local_sbm;

    struct list_node spp_list;
};

#define BT_ADAPTER_MAX_DEVICE_NUM (BT_DEVICE_NUM+BT_SOURCE_DEVICE_NUM)

struct BT_ADAPTER_MANAGER_T {
    struct BT_ADAPTER_DEVICE_T bt_sink_device[BT_ADAPTER_MAX_DEVICE_NUM];
    struct BT_ADAPTER_DEVICE_T bt_source_device[BT_ADAPTER_MAX_DEVICE_NUM];
    struct BT_ADAPTER_DEVICE_T bt_tws_device;
    osMutexId adapter_lock;
    uint8_t access_mode;
};

void bt_add_event_callback(bt_event_callback_t cb, uint32_t masks);

typedef struct bes_aud_bt_t {
    void (*aud_set_stay_active_mode)(bool keep_active);
    uint8_t (*aud_get_curr_a2dp_device)(void); // app_bt_audio_get_curr_a2dp_device
    uint8_t (*aud_get_curr_sco_device)(void); // app_bt_audio_get_curr_sco_device
    bool (*aud_is_sco_prompt_play_mode)(void); // app_bt_manager.config.sco_prompt_play_mode
    uint8_t (*aud_get_max_sco_number)(void); // bt_get_max_sco_number
    uint8_t (*aud_get_a2dp_codec_type)(int device_id); // bt_sbc_player_get_codec_type
    uint8_t (*aud_get_a2dp_sample_rate)(int device_id); // curr_device->sample_rate
    int (*aud_switch_sco)(uint16_t sco_handle); // app_bt_Me_switch_sco
    void (*aud_report_hfp_speak_gain)(void); // btapp_hfp_report_speak_gain
    void (*aud_report_a2dp_speak_gain)(void); // btapp_a2dp_report_speak_gain
    bool (*aud_hfp_mic_need_skip_frame)(void); // btapp_hfp_mic_need_skip_frame
    uint8_t (*aud_hfp_need_mute)(void); // btapp_hfp_need_mute
    void (*aud_hfp_set_local_vol)(int id, uint8_t vol); // hfp_volume_local_set
    uint8_t (*aud_adjust_hfp_volume)(uint8_t device_id, bool up, bool adjust_local_vol_level); // app_bt_hfp_adjust_volume
    uint8_t (*aud_adjust_a2dp_volume)(uint8_t device_id, bool up, bool adjust_local_vol_level); // app_bt_a2dp_adjust_volume
    bool (*ignore_ring_and_play_tone_self)(int device_id);
#if defined(A2DP_LHDC_ON)
    bool (*a2dp_lhdc_get_ext_flags)(uint32_t flags); // a2dp_lhdc_get_ext_flags
    uint8_t (*a2dp_lhdc_config_llc_get)(void); // a2dp_lhdc_config_llc_get
#endif
} bes_aud_bt_t;

extern const bes_aud_bt_t * const bes_aud_bt;

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_device(int device_id);
struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_by_id(int device_id);
struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_by_connhdl(uint16_t connhdl);
struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_byaddr(const bt_bdaddr_t *remote);
uint8_t bt_adapter_reset_device_id(struct BT_ADAPTER_DEVICE_T *curr_device, bool reset_to_source);
uint8_t bt_adapter_get_a2dp_codec_type(int device_id);
uint8_t bt_adapter_get_hfp_sco_codec_type(int device_id);
uint8_t bt_adapter_count_mobile_link(void);
uint8_t bt_adapter_count_streaming_a2dp(void);
uint8_t bt_adapter_count_streaming_sco(void);
uint8_t bt_adapter_has_incoming_call(void);
int bt_adapter_get_device_id_by_connhdl(uint16_t connhdl);
int bt_adapter_get_device_id_byaddr(const bt_bdaddr_t *remote);
void bt_adapter_mutex_lock(void);
void bt_adapter_mutex_unlock(void);
void bt_adapter_manager_init(void);
void bt_adapter_local_volume_down(void);
void bt_adapter_local_volume_up(void);
void bt_adapter_local_volume_down_with_callback(void (*cb)(uint8_t device_id));
void bt_adapter_local_volume_up_with_callback(void (*cb)(uint8_t device_id));
void bt_adapter_set_a2dp_codec_info(int device_id, uint8_t codec_type, uint8_t sample_rate, uint8_t sample_bit);
void bt_adapter_set_hfp_sco_codec_type(int device_id, uint8_t sco_codec);

void bt_adapter_report_acl_connected(const bt_bdaddr_t *bd_addr, const bt_adapter_acl_opened_param_t *acl_con);
void bt_adapter_report_acl_disconnected(const bt_bdaddr_t *bd_addr, const bt_adapter_acl_closed_param_t *acl_dis);
void bt_adapter_report_sco_connected(const bt_bdaddr_t *bd_addr, const bt_adapter_sco_opened_param_t *sco_con);
void bt_adapter_report_sco_disconnected(const bt_bdaddr_t *bd_addr, const bt_adapter_sco_closed_param_t *sco_dis);
void bt_adapter_report_access_change(const bt_bdaddr_t *bd_addr, const bt_adapter_access_change_param_t *access_change);
void bt_adapter_report_role_discover(const bt_bdaddr_t *bd_addr, const bt_adapter_role_discover_param_t *role_discover);
void bt_adapter_report_role_change(const bt_bdaddr_t *bd_addr, const bt_adapter_role_change_param_t *role_change);
void bt_adapter_report_mode_change(const bt_bdaddr_t *bd_addr, const bt_adapter_mode_change_param_t *mode_change);
void bt_adapter_report_authenticated(const bt_bdaddr_t *bd_addr, const bt_adapter_authenticated_param_t *auth);
void bt_adapter_report_enc_change(const bt_bdaddr_t *bd_addr, const bt_adapter_enc_change_param_t *enc_change);
void bt_adapter_report_inquiry_result(const bt_bdaddr_t *bd_addr, const bt_adapter_inquiry_result_param_t *inq_result);
void bt_adapter_report_inquiry_complete(const bt_bdaddr_t *bd_addr, const bt_adapter_inquiry_complete_param_t *inq_complete);
void bt_adapter_echo_register(void(*echo_req)(uint8_t device_id, uint16_t conhdl, uint8_t id, uint16_t len, uint8_t *data),
                                     void(*echo_rsp)(uint8_t device_id, uint16_t conhdl, uint8_t *rxdata, uint16_t rxle));
void bt_adapter_echo_req_send(uint8_t device_id, void *conn, uint8_t *data, uint16_t data_len);
void bt_adapter_echo_rsp_send(uint8_t device_id, uint16 conn_handle, uint8 sigid, uint16 len, const uint8* data);
#ifdef BT_HFP_SUPPORT
void bt_adapter_report_hfp_connected(const bt_bdaddr_t *bd_addr, const bt_hf_opened_param_t *hfp_conn);
void bt_adapter_report_hfp_disconnected(const bt_bdaddr_t *bd_addr, const bt_hf_closed_param_t *hfp_disc);
void bt_adapter_report_hfp_ring(const bt_bdaddr_t *bd_addr, const void *param);
void bt_adapter_report_hfp_clip_ind(const bt_bdaddr_t *bd_addr, const bt_hf_clip_ind_param_t *hfp_caller_ind);
void bt_adapter_report_hfp_call_state(const bt_bdaddr_t *bd_addr, const bt_hf_call_ind_param_t *hfp_call);
void bt_adapter_report_hfp_callsetup_state(const bt_bdaddr_t *bd_addr, const bt_hf_callsetup_ind_param_t *hfp_callsetup);
void bt_adapter_report_hfp_callhold_state(const bt_bdaddr_t *bd_addr, const bt_hf_callheld_ind_param_t *hfp_callhold);
void bt_adapter_report_hfp_volume_change(const bt_bdaddr_t *bd_addr, const bt_hf_volume_change_param_t *volume_change);
#endif /* BT_HFP_SUPPORT */
#ifdef BT_A2DP_SUPPORT
void bt_adapter_report_a2dp_connected(const bt_bdaddr_t *bd_addr, const bt_a2dp_opened_param_t *a2dp_conn);
void bt_adapter_report_a2dp_disconnected(const bt_bdaddr_t *bd_addr, const bt_a2dp_closed_param_t *a2dp_disc);
void bt_adapter_report_a2dp_stream_start(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_start_param_t *a2dp_stream_start);
void bt_adapter_report_a2dp_stream_reconfig(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_reconfig_param_t *a2dp_stream_reconfig);
void bt_adapter_report_a2dp_stream_suspend(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_suspend_param_t *a2dp_stream_suspend);
void bt_adapter_report_a2dp_stream_close(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_close_param_t *a2dp_stream_close);
#endif /* BT_A2DP_SUPPORT */
#ifdef BT_AVRCP_SUPPORT
void bt_adapter_report_avrcp_connected(const bt_bdaddr_t *bd_addr, const bt_avrcp_opened_t *avrcp_conn);
void bt_adapter_report_avrcp_disconnected(const bt_bdaddr_t *bd_addr, const bt_avrcp_closed_t *avrcp_disc);
#endif /* BT_AVRCP_SUPPORT */

#ifdef __cplusplus
}
#endif
#endif /* __BT_ADAPTER_LAYER_H__ */
