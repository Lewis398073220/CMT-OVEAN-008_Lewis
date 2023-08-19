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
#ifndef __L2CAP_I_H__
#define __L2CAP_I_H__
#include "co_ppbuff.h"
#include "bluetooth.h"
#include "btm_i.h"
#if defined(__cplusplus)
extern "C" {
#endif

#define L2CAP_PPB_HEAD_RESERVE      4       /*len+cid+control+sdulen  2+2+2+2*/
#define L2CAP_PPB_TAIL_RESERVE      0        /* fcs 2 */
#define L2CAP_PPB_RESERVE  (L2CAP_PPB_HEAD_RESERVE + L2CAP_PPB_TAIL_RESERVE)
#define L2CAP_HCI_HEADER_LEN (HCI_DATA_TYPE_LEN + HCI_ACL_HEADER_LEN)

typedef uint16_t l2cap_psm_target_profile_t;
typedef enum {
    L2CAP_PSM_TARGET_SELF,
    L2CAP_PSM_TARGET_AVDTP_SIGNAL = 1,
    L2CAP_PSM_TARGET_AVDTP_MEDIA,
    L2CAP_PSM_TARGET_OUTGOING_SDP,
    L2CAP_PSM_TARGET_INCOMING_SDP,
    L2CAP_PSM_TARGET_OUTGOING_ATT,
    L2CAP_PSM_TARGET_INCOMING_ATT,
    L2CAP_PSM_TARGET_OUTGOING_EATT,
    L2CAP_PSM_TARGET_INCOMING_EATT,
    L2CAP_DYNAMIC_PSM_TARGET_PBAP,
    L2CAP_DYNAMIC_PSM_TARGET_MAP_ACCESS,
    L2CAP_DYNAMIC_PSM_TARGET_MAP_NOTIFY,
    L2CAP_DYNAMIC_PSM_TARGET_OPP,
} _impl_l2cap_psm_target_profile_t;

// Bluetooth SIG standard
#define PSM_SDP                 0x0001
#define PSM_RFCOMM              0x0003
#define PSM_TCS_BIN             0x0005 // telephony control spec / tcs binary
#define PSM_TCS_BIN_CORDLESS    0x0007
#define PSM_BNEP                0x000F
#define PSM_HID_CTRL            0x0011
#define PSM_HID_INTR            0x0013
#define PSM_UPNP                0x0015
#define PSM_AVCTP               0x0017
#define PSM_AVDTP               0x0019
#define PSM_AVCTP_BROWSING      0x001B
#define PSM_UDI_C_PLANE         0x001D // unrestricted digital information profile UDI
#define PSM_ATT                 0x001F
#define PSM_3DSP                0x0021 // 3D synchronization profile
#define PSM_LE_IPSP             0x0023 // internet protocol support profile IPSP
#define PSM_OTS                 0x0025 // object transfer service OTS
#define PSM_EATT                0x0027

// Local specified defines
#define PSM_BESAUD              0x0033
#define PSM_AVDTP_SIGNAL        PSM_AVDTP
#define PSM_AVDTP_MEDIA_FAKE    0xffff
#define PSM_INCOMING_ATT        0xaa00
#define PSM_INCOMING_EATT       0xbb00
#define PSM_INCOMING_SDP_BASE   0xee00
#define PSM_OUTGOING_SDP_BASE   0xdd00
#define PSM_MAP_ACCESS_BASE     0xcc00
#define PSM_DYNAMIC_PBAP        0x1011
#define PSM_DYNAMIC_MAP_ACCESS  0x1013
#define PSM_DYNAMIC_MAP_NOTIFY  0x1015
#define PSM_DYNAMIC_OPP         0x1003


#define L2CAP_SCID_PREFIX_MASK              0xff00
#define L2CAP_SCID_INDEX_MASK               0x00ff

#define L2CAP_SCID_PREFIX_OUTGOING_SDP      0x1000
#define L2CAP_SCID_PREFIX_INCOMING_SDP      0x2000
#define L2CAP_SCID_PREFIX_RFCOMM            0x3000
#define L2CAP_SCID_PREFIX_BNEP              0x4000
#define L2CAP_SCID_PREFIX_HID_CTRL          0x5000
#define L2CAP_SCID_PREFIX_HID_INTR          0x5100
#define L2CAP_SCID_PREFIX_UPNP              0x6000
#define L2CAP_SCID_PREFIX_AVCTP             0x7000
#define L2CAP_SCID_PREFIX_AVCTP_BROWSING    0x8000
#define L2CAP_SCID_PREFIX_AVDTP_SINGAL      0x9000
#define L2CAP_SCID_PREFIX_AVDTP_MEDIA       0xa000
#define L2CAP_SCID_PREFIX_OUTGOING_ATT      0xaa00
#define L2CAP_SCID_PREFIX_INCOMING_ATT      0xab00
#define L2CAP_SCID_PREFIX_OUTGOING_EATT     0xea00
#define L2CAP_SCID_PREFIX_INCOMING_EATT     0xeb00
#define L2CAP_SCID_PREFIX_PBAP              0xb100
#define L2CAP_SCID_PREFIX_MAP_ACCESS        0xb200
#define L2CAP_SCID_PREFIX_MAP_NOTIFY        0xb300
#define L2CAP_SCID_PREFIX_IDENTIFIER        0xc000
#define L2CAP_SCID_PREFIX_OPP               0xd000

#define L2CAP_BESAUD_EXTRA_CHAN_ID 0xbbee

enum l2cap_event_enum {
    L2CAP_CHANNEL_CONN_REQ,
    L2CAP_CHANNEL_OPENED,
    L2CAP_CHANNEL_NEW_OPENED,
    L2CAP_CHANNEL_TX_HANDLED,
    L2CAP_CHANNEL_CLOSED,
};

struct l2cap_ctx_input {
    struct ctx_content ctx;
    struct bdaddr_t *remote;
    uint32 l2cap_handle;
    uint16 conn_handle;
    int (*l2cap_notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason);
    void (*l2cap_datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb);
};

struct l2cap_ctx_output {
    uint32 l2cap_handle;
};

typedef void (*l2cap_sdp_disconnect_callback)(const void *bdaddr);
typedef uint8 (*btm_get_ibrt_role_callback)(const void *para);
typedef uint8 (*btm_get_ui_role_callback)(void);
typedef uint8_t (*btm_get_tss_state_callback)(const void *para);

#define L2CAP_NULL_CID                      0x0000
#define L2CAP_SIG_CID                       0x0001
#define L2CAP_CONNECTIONLESS_CID            0x0002
#define L2CAP_AMP_MANAGER_CID               0x0003
#define L2CAP_ATT_CID                       0x0004
#define L2CAP_LE_SIGNALING_CID              0x0005
#define L2CAP_SECURITY_MANAGER_CID          0x0006
#define L2CAP_BREDR_SECURITY_MANAGER_CID    0x0007
#define L2CAP_AMP_TEST_MANAGER_CID          0x003F

#define L2CAP_MIN_DYNAMIC_CID               (0x40)
#define l2cap_cid_to_index(cid)        (cid - L2CAP_MIN_DYNAMIC_CID)
#define l2cap_index_to_cid(index)      (index + L2CAP_MIN_DYNAMIC_CID)


struct l2cap_hdr {
    uint16 len;
    uint16 cid;
}__attribute__ ((packed));

#define L2CAP_SIG_REJECT_RSP                0x01    // acl-u le-u both allowed
#define L2CAP_SIG_CONN_REQ                  0x02    // only acl-u
#define L2CAP_SIG_CONN_RSP                  0x03    // only acl-u
#define L2CAP_SIG_CFG_REQ                   0x04    // only acl-u
#define L2CAP_SIG_CFG_RSP                   0x05    // only acl-u
#define L2CAP_SIG_DISCONN_REQ               0x06    // acl-u le-u both allowed
#define L2CAP_SIG_DISCONN_RSP               0x07    // acl-u le-u both allowed
#define L2CAP_SIG_ECHO_REQ                  0x08    // only acl-u
#define L2CAP_SIG_ECHO_RSP                  0x09    // only acl-u
#define L2CAP_SIG_INFO_REQ                  0x0A    // only acl-u
#define L2CAP_SIG_INFO_RSP                  0x0B    // only acl-u
#define L2CAP_SIG_CONN_PRAM_UPDATE_REQ      0x12    // only le-u
#define L2CAP_SIG_CONN_PRAM_UPDATE_RSP      0x13    // only le-u
#define L2CAP_SIG_LE_CREDIT_BASED_CONN_REQ  0x14    // only le-u
#define L2CAP_SIG_LE_CREDIT_BASED_CONN_RSP  0x15    // only le-u
#define L2CAP_SIG_FLOW_CONTROL_CREDIT_IND   0x16    // acl-u le-u both allowed
#define L2CAP_SIG_CREDIT_BASED_CONN_REQ     0x17    // acl-u le-u both allowed
#define L2CAP_SIG_CREDIT_BASED_CONN_RSP     0x18    // acl-u le-u both allowed
#define L2CAP_SIG_CREDIT_BASED_RECONF_REQ   0x19    // acl-u le-u both allowed
#define L2CAP_SIG_CREDIT_BASED_RECONF_RSP   0x1A    // acl-u le-u both allowed

struct l2cap_sig_hdr {
    byte code;
    byte id;
    uint16 len;
}__attribute__ ((packed));


#define L2CAP_SIG_REASON_NOT_UNDERSTOOD 0x0
#define L2CAP_SIG_REASON_MTU_EXCEED 0x1
#define L2CAP_SIG_REASON_INVALID_CID 0x2
struct l2cap_sig_rej {

    uint16 reason;
/*data*/
    uint16 scid;            /*the data len is 0 - 4*/
    uint16 dcid;
}__attribute__ ((packed));

struct l2cap_sig_conn_req {
    uint16 psm;
    uint16 scid;
}__attribute__ ((packed));

#define L2CAP_SIG_RESULT_SUCCESS 0x0
#define L2CAP_SIG_RESULT_PENDING 0x1
#define L2CAP_SIG_RESULT_REFUSE_PSM 0x2
#define L2CAP_SIG_RESULT_REFUSE_SECURITY 0x3
#define L2CAP_SIG_RESULT_REFUSE_RESOURCE 0x4
#define L2CAP_SIG_RESULT_INVALID_SOURCE_ID 0x6
#define L2CAP_SIG_RESULT_SOURCE_ID_ALREADY_ALL0CATED 0x7

#define L2CAP_SIG_RESULT_PENDING_NOINFO 0x00
#define L2CAP_SIG_RESULT_PENDING_AUTHEN 0x01
#define L2CAP_SIG_RESULT_PENDING_AUTHOR 0x02
struct l2cap_sig_conn_rsp {
    uint16 dcid;
    uint16 scid;

    uint16 result;
    uint16 status;   /*only defined when result = pending */
}__attribute__ ((packed));

struct l2cap_sig_cfg_req {
    uint16 dcid;
    uint16 flags;           /* bit0=1:continue  bit0=0:complete  */
}__attribute__ ((packed));

#define L2CAP_CFGRSP_SUCCESS                        0x0000
#define L2CAP_CFGRSP_UNACCEPT_PARAMS       0x0001
#define L2CAP_CFGRSP_REJ                                 0x0002
#define L2CAP_CFGRSP_UNKNOWN                       0x0003
struct l2cap_sig_cfg_rsp {
    uint16 scid;
    uint16 flags;
    uint16 result;
}__attribute__ ((packed));

#define L2CAP_CFG_TYPE_MTU          0x01
#define L2CAP_CFG_TYPEF_FLUSH_TO    0x02
#define L2CAP_CFG_TYPE_QOS          0x03
#define L2CAP_CFG_TYPE_RFC          0x04    //retransmission and flow control
#define L2CAP_CFG_TYPE_FCS          0x05
#define L2CAP_CFG_TYPE_EFS          0x05    //extended flow specification
#define L2CAP_CFG_TYPE_EWS          0x06    //extended window size
struct l2cap_sig_cfg_opt_hdr{
    byte type;
    byte len;
}__attribute__ ((packed));

struct l2cap_sig_cfg_opt_mtu {

    uint16 mtu;
}__attribute__ ((packed));
struct l2cap_sig_cfg_opt_flushto {

    uint16 flushto;
}__attribute__ ((packed));

#define L2CAP_QOS_NO_TRAFFIC		0x00
#define L2CAP_QOS_BEST_EFFORT		0x01
#define L2CAP_QOS_GUARANTEED		0x02
struct l2cap_sig_cfg_opt_qos {

    byte	 flags;
    byte     service_type;
    uint32  token_rate;
    uint32  token_size;
    uint32  bandwidth;
    uint32  latency;
    uint32  delay_variation;
}__attribute__ ((packed));

#define L2CAP_MODE_BASE                         0
#define L2CAP_MODE_RETRANSMISSION               1
#define L2CAP_MODE_FLOWCONTROL                  2
#define L2CAP_MODE_ENHANCED_RETRANSMISSION      3
#define L2CAP_MODE_STREAMING                    4
#define L2CAP_MODE_LE_CREDIT_FLOWCONTROL        5
#define L2CAP_MODE_ENHANCED_CREDIT_FLOWCONTROL  6

struct l2cap_sig_cfg_opt_rfc {

    byte     mode;
    byte      txwindow;
    byte      maxtransmit;
    uint16   retransmission_timeout;
    uint16   monitor_timeout;
    uint16   mps;
}__attribute__ ((packed));

#define L2CAP_FCS_TYPE_NONE 0x00
#define L2CAP_FCS_TYPE_16_BIT   0x01
struct l2cap_sig_cfg_opt_fcs {
    byte type;
}__attribute__ ((packed));

struct l2cap_sig_disconn_req {

    uint16 dcid;
    uint16 scid;
}__attribute__ ((packed));

struct l2cap_sig_disconn_rsp {
    uint16 dcid;
    uint16 scid;
}__attribute__ ((packed));

#define L2CAP_INFOTYPE_CONNLESS_MTU     0x01    // 2-octet
#define L2CAP_INFOTYPE_EXTENED_FEATURE  0x02    // 4-octet
#define L2CAP_INFOTYPE_FIXED_CHANNELS   0x03    // 8-octet

#define L2CAP_FIXCH_SUPPORT_SIGNALING_CHANNEL       0x02
#define L2CAP_FIXCH_SUPPORT_CONNLESS_RECEPTION      0x04
#define L2CAP_FIXCH_SUPPORT_BREDR_SECURITY_MANAGER  0x80

struct l2cap_sig_info_req {
    uint16 infotype;
}__attribute__ ((packed));

#define L2CAP_INFOTYPE_SUCCESS          0x00
#define L2CAP_INFOTYPE_NOT_SUPPORT      0x01

#define L2CAP_EXTFEATURE_SUPPORT_FLOW_CONTROL_MODE       0x0001
#define L2CAP_EXTFEATURE_SUPPORT_RETRANSMISSION_MODE     0x0002
#define L2CAP_EXTFEATURE_SUPPORT_BIDIRECTIONAL_QOS       0x0004
#define L2CAP_EXTFEATURE_SUPPORT_ENHANCED_RETRANS_MODE   0x0008
#define L2CAP_EXTFEATURE_SUPPORT_STREAMING_MODE          0x0010
#define L2CAP_EXTFEATURE_SUPPORT_FCS_OPTION              0x0020
#define L2CAP_EXTFEATURE_SUPPORT_EXTENDED_FLOW_SPEC      0x0040
#define L2CAP_EXTFEATURE_SUPPORT_FIXED_CHANNELS          0x0080
#define L2CAP_EXTFEATURE_SUPPORT_EXTENDED_WINDOW_SIZE    0x0100
#define L2CAP_EXTFEATURE_SUPPORT_UNICAST_CONNECTIONLESS_DATA_RECEPTION   0x0200
#define L2CAP_EXTFEATURE_SUPPORT_ENHANCED_CREDIT_BASED_FLOW_CONTROL_MODE 0x0400
struct l2cap_sig_info_rsp {
    uint16 infotype;
    uint16 result;
    /*if result == success, data: mtu(2 bytes), feature mask(4 bytes) */
    uint32 mask;
}__attribute__ ((packed));

struct l2cap_sig_fcs_rsp {
    uint16 infotype;
    uint16 result;
    uint8 bitmask[8];
}__attribute__ ((packed));

#define L2CAP_SIG_CFG_MTU_MASK                  (1<<0)
#define L2CAP_SIG_CFG_FLUSHTO_MASK          (1<<1)
#define L2CAP_SIG_CFG_QOS_MASK                  (1<<2)
#define L2CAP_SIG_CFG_RFC_MASK                   (1<<3)
#define L2CAP_SIG_CFG_FCS_MASK                   (1<<4)

struct config_in_t {
    uint8  cfgin_flag;
    struct l2cap_sig_cfg_opt_mtu mtu_in;
    struct l2cap_sig_cfg_opt_flushto flushto_in;
    struct l2cap_sig_cfg_opt_qos qos_in;
    struct l2cap_sig_cfg_opt_rfc rfc_in;
    struct l2cap_sig_cfg_opt_fcs fcs_in;
};

struct config_out_t {
    uint8 cfgout_flag;
    struct l2cap_sig_cfg_opt_mtu mtu_out;
    struct l2cap_sig_cfg_opt_flushto flushto_out;
    struct l2cap_sig_cfg_opt_qos qos_out;
    struct l2cap_sig_cfg_opt_rfc rfc_out;
    struct l2cap_sig_cfg_opt_fcs fcs_out;
};

#define L2C_NOTIFY_RESULT_ACCEPT (0)
#define L2C_NOTIFY_RESULT_REJECT (1)
#define L2C_NOTIFY_RESULT_UPPER_LAYER_HANDLED (2)

typedef int (*l2cap_ext_callback_t)(const bt_bdaddr_t *remote, enum l2cap_event_enum event, void *param);

struct l2cap_registered_psm_item_t {
    struct list_node list;
    uint16 psm;
    int8 conn_count;    /*how many conn can be created*/
    int (*l2cap_notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason);
    void (*l2cap_datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb);
};

enum l2cap_channel_state_enum {
    L2CAP_CLOSE,                /*baseband connection closed, wait for hci conn openning, and then can send out conn request signal*/
    L2CAP_WAIT_DISCONNECT,
    L2CAP_WAITING,              /* waitf for the baseband connection to send out conn req signal */
    L2CAP_AUTH_PENDING, /* waiting for baseband authentication or encryption */
    L2CAP_WAIT_CONNECTION_RSP,
    L2CAP_WAIT_CONFIG,
    L2CAP_WAIT_CONFIG_REQ_RSP,
    L2CAP_WAIT_CONFIG_RSP,
    L2CAP_WAIT_CONFIG_REQ,
    L2CAP_OPEN
};


#ifdef BT_L2CAP_ENRE_MODE_SUPPORT

/**
 * The TxWindow specifies the size of the transmission window for
 * Flow Control mode, Retransmission mode, and Enhanced Retransmission
 * mode. The range is 1 to 63 for Enhanced Retransmission mode.
 *
 * In Enhanced Retransmission mode this value indicates the max number
 * of I-frames that the sender of the option can receive without ack
 * some of the received frames. It is not negotiated.
 *
 * It is an informational parameter that each l2cap entity can specify
 * separately. In general, the TxWindow size should be made as large as
 * possible to maximize channel utilization.
 *
 * TxWindow size values in cfg rsp indicate the max number of packets
 * the sender can send before it requires an ack. In other words it
 * represents the number of unacked packets the send can hold.
 * The value sent in an cfg rsp shall be less than or equal to the
 * TxWindow size sent in the cfg req. The receiver of this option in
 * the cfg rsp may use this value as part of its acknowledgment algorithm.
 *
 * ---
 *
 * The MaxTransmit controls the number of transmissions of a single I-frame
 * that l2cap is allowed to try in Retransmission mode and Enhanced Retransmission
 * mode before accepting that a packet and the channel is lost. The minimum
 * value is 1 (one transmission is permitted). When a packet is lost after
 * being transmitted MaxTransmit times the channel shall be disconnected by
 * sending a Disconnect request.
 *
 * In Enhanced Retransmission mode MaxTransmit controls the number of
 * retransmissions for I-frames and S-frames with P-bit set to 1.
 * The sender of the option in an cfg req specifies the value that shall be
 * used by the receiver of the option. MaxTransmit values in an cfg rsp
 * shall be ignored.
 *
 * Lower values might be appropriate for services requiring low latency.
 * Higher values will be suitable for a link requiring robust operation.
 * A value of 1 means tha no retransmission will be made but also means
 * that the channel will be disconnected as soon as a packet is lost.
 *
 * MaxTransmit shall not be set to zero in Retransmission mode, In Enhanced
 * Retransmission mode a value of zero for MaxTransmit means infinite
 * retansmissions.
 *
 * ---
 *
 * The RetransmissionTimeout is the value in ms of the retransmission timeout.
 * It is used to initialize the RetransmissionTimer. The purpose of this
 * timer in Enhanced Retransmission mode is to detect lost I-frames and initiate
 * appropriate error recovery. A value for the RetransmissionTimeout shall be
 * sent in a positive cfg rsp and indicates the value that will be used by
 * the sender of the cfg rsp.
 *
 * ---
 *
 * In Enhanced Retransmission mode the MonitorTimeout is used to detect
 * lost S-frames with P-bit set to 1. If the timeout occurs before a
 * response with the F-bit set to 1 is received the S-frame is resent.
 * A value for the MonitorTimeout shall be sent in a positive cfg rsp
 * and indicates the value that will be used by the sender of the cfg
 * rsp.
 *
 * ---
 *
 * The MaxPduPayloadSize (MPS) is the max size of payload data in octets
 * that the l2cap entity sending the option in an cfg req is capable of
 * accepting.
 *
 * An MPS value sent in a positive cfg rsp is the actual MPS the receiver
 * of the cfg req will use on this channel for traffic flowing into the
 * lcoal device. An MPS value sent in a positive cfg rsp shall be equal
 * to or smaller than the value sent in the cfg req.
 *
 */

#define L2CAP_TX_WINDOW_SIZE    5 // max num of I-frames can rx w/o ack

#define L2CAP_MAX_TRANSMIT      3

#define L2CAP_RETRANS_TIMEOUT   2000

#define L2CAP_MONITOR_TIMEOUT   12000

#define L2CAP_MAX_PDU_PAYLOAD_SIZE 662

struct l2cap_i_frame_node
{
    struct list_node node;
    struct pp_buff *ppb;
    uint8_t i_frame_txseq;
    uint8_t transmit_count; // when a i-frame retry times reach MaxTransmit shall close channel
    bool retransmit_flag;
    bool set_final_bit;
};

struct l2cap_enre_channel_t
{
    // i-frame tx side
    uint8_t expected_ackseq;    // the first seqn that have not received ack, previous are all acked
    uint8_t next_txseq;         // curr seqn need to tx, previous are all sent
    uint8_t tx_window;          // tx_window is start from expected_ackseq
    bool remote_rx_busy;        // remote can receive i-frames or not, set when rx RNR, cleared when rx RR(P=1)
    uint8_t i_frame_retrans_timer; // used to detect lost i-frames
    uint8_t s_frame_monitor_timer; // used to detect lost s-frames
    bool monitor_timer_is_started;
    bool wait_f_tx_state_entered;
    bool set_fbit_in_i_frame;
    uint16_t remote_rx_mps; // remote can receive the max PDU Payload Size
    struct list_node pending_list; // i-frames waiting to send due to remote rx window full, or busy condition, or pending in the queue
    struct list_node unacked_list; // i-frames sent but not receive ack, and may need retransmit some of them
    bool send_pending_frame;

    // i-frame rx side
    uint8_t buffer_start_seq; // the first seqn that not sent ack, previous are all acked
    uint8_t expected_txseq;   // curr seqn that not received, previous are all received
    uint8_t rx_window;        // rx_window is start from buffer_start_seq and beyond expected_txseq
    uint8_t poll_s_frame_tx_count; // s-frame operation retry times, if reach MaxTransmit times shall close channel
    bool local_rx_busy; // local can receive i-frames or not, set when tx RNR, should sent RR(p=1) to clear busy
    bool not_ready_is_sent; // RNR is sent or not after local rx busy
    bool rej_sent_state_entered;
    bool srej_sent_state_entered;
    bool poll_srej_is_sent;
    uint16_t local_rx_mps; // local can receive the Max PDU Payload Size

    bool fcs_option_on;
    uint16_t sdu_total_length;
    uint16_t sdu_curr_length;
};

#endif /* BT_L2CAP_ENRE_MODE_SUPPORT */


struct l2cap_resent_cfg_req_t {
    uint16 resent_count;
    uint16 channel_scid;
    uint16 channel_last_sigid;
};

struct l2cap_channel {
    struct list_node list;
    struct l2cap_conn *conn;
    uint32 l2cap_handle;
    uint16 expected_mtu_size;

    uint16 scid;
    uint16 dcid;
    uint16 psm_remote;
    int16 scid_arridx;

    uint8 used;         /* channel used or not*/
    uint8 initiator;    /* local or peer initate l2cap channel*/
    uint8 sigid_last_send;   /*to save our last request signal id*/
    uint8 sigid_last_recv;  /*to save the last remote's request signal id*/
    uint8_t cfg_req_resent_count;
    uint8_t chan_index;

    //max co timer num is less than 255 in our stack,so just one byte is ok
    uint8 disconnect_req_timeout_timer;/*to avoid disconnect req not response,so we need to give a timeout flag*/
    uint8 wait_conn_req_timer;
    uint8 wait_config_req_timer;
    uint8 wait_channel_open_timer;

    uint8 disconnect_req_reason;
    /* for config req and resp */
    uint8 wait_cfg_req_done;
    uint8 wait_cfg_rsp_done;
    struct config_in_t cfgin;
    struct config_out_t cfgout;
    struct config_in_t cfgin_rsp;

    enum l2cap_channel_state_enum state;
    int (*l2cap_notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason);
    void (*l2cap_datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb);
    struct pp_buff *recv_cfg_req_before_conn_rsp;
    uint8 cfg_req_recv_trans_id;
    uint16 cfg_req_recv_siglen;

#ifdef BT_L2CAP_ENRE_MODE_SUPPORT
    struct l2cap_enre_channel_t *enre_chan;
    bool may_use_enre_mode;
#endif

    uint8 l2cap_mode;
    uint8 remote_cfg_nego_mode;
    uint8 local_cfg_nego_mode;
    bool slave_ignore_wrong_state;
    uint8 l2cap_send_role;
    bool disc_req_pending;
    bool disc_channel_after_conn_rsp;
    bool dont_report_close_event;
};

#define L2CAP_MAX_CHANNELS_PER_LINK (12)
struct l2cap_channel_item_t {
    struct l2cap_channel *channel;
};

typedef enum {
    BLUETOOTH_BT_SDP = 0,
    BLUETOOTH_BT_RFC,
    BLUETOOTH_PROTO_MAX_NUM,
} bluetooth_proto_id_t;

typedef struct { /* placeholder */ } bt_proto_conn_t;
typedef void (*bt_proto_conn_close_callback)(bt_proto_conn_t *proto_conn);

struct l2cap_proto_conn_t {
    struct l2cap_conn *conn;
    bt_proto_conn_t *proto_conn;
    bt_proto_conn_close_callback conn_close_cb;
};

struct l2cap_proto_conn_item_t {
    struct l2cap_proto_conn_t *item;
};

struct l2cap_conn {
    struct list_node list;
    struct bdaddr_t remote;
    uint16_t conn_handle;
    uint8_t sigid_seed;
    uint8_t device_id;
    bool echo_req_sent;
    uint8_t pending_echo_rsp_count;
    uint8_t disconnect_by_acl;
    uint8_t disconnect_reason;
    uint8_t delay_free_conn_timer;
    uint8_t l2cap_channel_count;
    struct l2cap_channel_item_t chans[L2CAP_MAX_CHANNELS_PER_LINK];
    struct l2cap_proto_conn_item_t proto[BLUETOOTH_PROTO_MAX_NUM];
};

struct l2cap_conn_req_param_t {
    uint16 conn_handle;
    uint16 remote_scid;
    uint16 psm;
    uint8 identifier;
    uint8 device_id;
    struct bdaddr_t remote_addr;
    l2cap_psm_target_profile_t target_profile;
    uint16 expected_mtu_size;
};

struct l2cap_channel * l2cap_accept_conn_req(struct l2cap_conn_req_param_t* req);
void l2cap_reject_conn_req(struct l2cap_conn_req_param_t* req, uint16 reason);

bt_proto_conn_t *l2cap_proto_conn_find_or_add(const bt_bdaddr_t *remote, bluetooth_proto_id_t proto_id, uint16_t alloc_size, bt_proto_conn_close_callback cb);
bt_proto_conn_t *l2cap_proto_conn_search(uint32_t l2cap_handle, bluetooth_proto_id_t proto_id);
struct l2cap_conn *l2cap_conn_from_proto_conn(bt_proto_conn_t *proto_conn);
void l2cap_proto_conn_free(bt_proto_conn_t *proto_conn);
bt_bdaddr_t l2cap_proto_conn_get_address(bt_proto_conn_t *proto_conn);
uint16_t l2cap_proto_conn_get_connhdl(bt_proto_conn_t *proto_conn);
uint8_t l2cap_proto_conn_get_device_id(bt_proto_conn_t *proto_conn);
uint8_t l2cap_get_device_id_by_address(const bt_bdaddr_t *remote);
int l2cap_proto_conn_foreach(bluetooth_proto_id_t proto_id, bool (*cb)(bt_proto_conn_t *conn, void *priv), void *priv);

int8 l2cap_send_frame_done(uint8 device_id, uint16 conn_handle, struct pp_buff *ppb);
uint8* l2cap_make_sig_req(struct l2cap_channel *channel,uint8 sig_code,uint16 sig_datalen,struct pp_buff *ppb);
struct l2cap_conn *l2cap_conn_search_conn_handle(uint16 conn_handle);
uint16 l2cap_get_conn_handle(struct bdaddr_t *bdaddr);
struct l2cap_channel *l2cap_channel_search_psm(struct l2cap_conn *conn, uint16 psm);
struct l2cap_channel *l2cap_channel_search_exist_psm(const bt_bdaddr_t *remote, uint16_t psm);

typedef void (*l2cap_fill_in_echo_req_data_callback_func)(uint8 device_id, struct l2cap_conn *conn, uint8 *data, uint16 data_len);
void register_l2cap_fill_in_echo_req_data_callback(l2cap_fill_in_echo_req_data_callback_func func);
void l2cap_fill_in_echo_req_data(uint8 device_id, void *conn, uint8 *data, uint16 data_len);

typedef void (*l2cap_process_echo_req_callback_func)(uint8 device_id, uint16 conhdl, uint8 id, uint16 len, uint8 *data);
void register_l2cap_process_echo_req_callback(l2cap_process_echo_req_callback_func func);
void l2cap_process_echo_req_rewrite_rsp_data(uint8 device_id, uint16 conhdl, uint8 id, uint16 len, uint8 *data);

typedef void (*l2cap_process_echo_res_callback_func)(uint8 device_id, uint16 conhdl, uint8* rxdata, uint16 rxlen);
void register_l2cap_process_echo_res_callback(l2cap_process_echo_res_callback_func func);
void l2cap_process_echo_res_analyze_data(uint8 device_id, uint16 conhdl, uint8* rxdata, uint16 rxlen);

void l2cap_find_and_free_pending_avdtp_channel(struct bdaddr_t* remote);
int8 l2cap_channel_close(uint8 device_id, struct l2cap_channel *channel, uint8 reason);
uint16 l2cap_get_converted_psm_from_handle_by_addr(uint32 l2cap_handle, struct bdaddr_t *remote_addr);

int8 l2cap_init ( void );

int8 l2cap_register  (uint16 psm,
                      int8 l2cap_conn_count_max,
                      int (*l2cap_notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason),
                      void (*l2cap_datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb)
                      );

uint32 l2cap_open (struct bdaddr_t *remote,
                    uint16 psm,
                    int (*l2cap_notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason),
                    void (*l2cap_datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb)
                    );

uint32 l2cap_open_with_target(struct bdaddr_t *remote,
                  uint16 psm, l2cap_psm_target_profile_t target_profile,
                  int (*l2cap_notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason),
                  void (*l2cap_datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb));

uint32 l2cap_open_with_target_with_mtu(struct bdaddr_t *remote,
                  uint16 psm, l2cap_psm_target_profile_t target_profile, uint16 expected_mtu_size,
                  int (*l2cap_notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason),
                  void (*l2cap_datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb));

uint32 l2cap_enre_open(struct bdaddr_t *remote, uint16 psm, l2cap_psm_target_profile_t target,
                  int (*l2cap_notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason),
                  void (*l2cap_datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb));

void l2cap_create_besaud_extra_channel(void* remote,
    int (*notify_callback)(uint8 device_id, enum l2cap_event_enum event, uint32 l2cap_handle, void *pdata, uint8 reason),
    void (*datarecv_callback)(uint8 device_id, uint32 l2cap_handle, struct pp_buff *ppb));

uint16 l2cap_get_converted_psm_from_handle(uint32 l2cap_handle);
int8 l2cap_close(uint32 l2cap_handle);
int8 l2cap_close_v2(uint32 l2cap_handle,uint8 reason);
int8 l2cap_close_after_conn_rsp(uint32 l2cap_handle);
struct pp_buff *l2cap_data_ppb_alloc_with_ca_line(uint32 datalen, void *context, uint32_t ca, uint32_t line);
#define l2cap_data_ppb_alloc(datalen, context) l2cap_data_ppb_alloc_with_ca_line(datalen, context, (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
int8 l2cap_send_data_link(uint32 l2cap_handle, struct data_link *head, void *context);
int8 l2cap_send_data_ppb( uint32 l2cap_handle, struct pp_buff *ppb);
int8 l2cap_send_data_auto_fragment(uint32 l2cap_handle, const uint8* data, uint32 len, void *context);
int8 l2cap_send_data( uint32 l2cap_handle, uint8 *data, uint32 datalen, void *context);
int l2cap_enre_send_data(uint32 l2cap_handle, const uint8 *sdu_data, uint32 datalen, void* context);
int8 l2cap_unregister(uint16 psm);
int8 l2cap_close_v2(uint32 l2cap_handle, uint8 reason);
int8 l2cap_close_delay (uint32 l2cap_handle, int delay_sec);
int32 l2cap_get_tx_mtu(uint32 l2cap_handle);
bool l2cap_is_valid_dynamic_psm(uint16 psm);
const char *l2cap_event2str(enum l2cap_event_enum event);
const char *l2cap_psm2str(uint32 psm);
void l2cap_conn_reset_sigid(struct bdaddr_t *bdaddr);
void l2cap_clean_ibrt_slave_status(const bt_bdaddr_t *remote);

void l2cap_btm_notify_callback(uint8_t device_id, enum btm_l2cap_event_enum event, uint16 conn_handle, void *pdata, uint8 reason);
void l2cap_btm_datarecv_callback (uint8_t device_id, uint16 conn_handle, struct pp_buff *ppb);
int8 l2cap_send_frame(uint16 conn_handle, struct pp_buff *ppb);
struct bdaddr_t *l2cap_get_conn_remote_addr(uint32 l2cap_handle);
struct l2cap_channel *l2cap_channel_search_l2caphandle(uint32 l2cap_handle);

uint32 l2cap_save_ctx(uint32 l2cap_handle, uint8_t *buf, uint32_t buf_len);
uint32 l2cap_restore_ctx(struct l2cap_ctx_input *input, struct l2cap_ctx_output *output, void (*close_old_channel_cb)(uint8_t));

void l2cap_pts_send_disconnect_channel(void);
void l2cap_pts_send_l2cap_data(void);

typedef void (*l2cap_process_bredr_smp_req_callback_func)(uint8 device_id, uint16 conn_handle, uint16 len, uint8 *data);
void register_l2cap_process_bredr_smp_req_callback(l2cap_process_bredr_smp_req_callback_func func);
int8 l2cap_send_bredr_security_manager_rsp(uint8 device_id, uint16 conn_handle, uint16 len, uint8 *data);
int8 l2cap_send_echo_req(uint8 device_id, struct l2cap_conn *conn, uint16 len, const uint8* data);
int8 l2cap_send_echo_rsp(uint8 device_id, uint16 conn_handle, uint8 sigid, uint16 len, const uint8* data);
#if defined(__cplusplus)
}
#endif
#endif /* __L2CAP_I_H__ */
