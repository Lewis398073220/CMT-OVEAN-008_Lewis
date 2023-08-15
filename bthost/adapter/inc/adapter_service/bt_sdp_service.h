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
#ifndef __BT_SDP_SERVICE_H__
#define __BT_SDP_SERVICE_H__
#include "adapter_service.h"
#ifdef __cplusplus
extern "C" {
#endif

#define DETD_NIL  0x00
#define DETD_UINT 0x08
#define DETD_SINT 0x10
#define DETD_UUID 0x18
#define DETD_TEXT 0x20
#define DETD_BOOL 0x28
#define DETD_SEQ  0x30
#define DETD_ALT  0x38
#define DETD_URL  0x40
#define DETD_MASK 0xf8

#define DESD_1BYTE      0x00
#define DESD_2BYTES     0x01
#define DESD_4BYTES     0x02
#define DESD_8BYTES     0x03
#define DESD_16BYTES    0x04
#define DESD_ADD_8BITS  0x05
#define DESD_ADD_16BITS 0x06
#define DESD_ADD_32BITS 0x07
#define DESD_MASK       0x07

#define SDP_ATTRIBUTE(attribId, attrib) \
          { attribId,           \
            sizeof(attrib),     \
            attrib}

#define SDP_ATTRIB_HEADER_8BIT(size) \
            DETD_SEQ + DESD_ADD_8BITS,      \
            size

#define SDP_ATTRIB_HEADER_16BIT(size) \
            DETD_SEQ + DESD_ADD_16BITS,      \
            (U8)(((size) & 0xff00) >> 8),    \
            (U8)((size) & 0x00ff)

#define SDP_ATTRIB_HEADER_32BIT(size) \
            DETD_SEQ + DESD_ADD_32BITS,         \
            (U8)(((size) & 0xff000000) >> 24),  \
            (U8)(((size) & 0x00ff0000) >> 16),  \
            (U8)(((size) & 0x0000ff00) >> 8),   \
            (U8)((size) & 0x000000ff)

#define SDP_ATTRIB_HEADER_ALT_8BIT(size) \
            DETD_ALT + DESD_ADD_8BITS,      \
            size

#define SDP_ATTRIB_HEADER_ALT_16BIT(size) \
            DETD_ALT + DESD_ADD_16BITS,      \
            (U8)(((size) & 0xff00) >> 8),    \
            (U8)((size) & 0x00ff)

#define SDP_ATTRIB_HEADER_ALT_32BIT(size) \
            DETD_ALT + DESD_ADD_32BITS,         \
            (U8)(((size) & 0xff000000) >> 24),  \
            (U8)(((size) & 0x00ff0000) >> 16),  \
            (U8)(((size) & 0x0000ff00) >> 8),   \
            (U8)((size) & 0x000000ff)

#define SDP_UUID_16BIT(uuid) \
            DETD_UUID + DESD_2BYTES,         \
            (U8)(((uuid) & 0xff00) >> 8),    \
            (U8)((uuid) & 0x00ff)

#define SDP_UUID_32BIT(uuid) \
            DETD_UUID + DESD_4BYTES,            \
            (U8)(((uuid) & 0xff000000) >> 24),  \
            (U8)(((uuid) & 0x00ff0000) >> 16),  \
            (U8)(((uuid) & 0x0000ff00) >> 8),   \
            (U8)((uuid) & 0x000000ff)

#define SDP_UUID_128BIT(uuid)                \
            DETD_UUID + DESD_16BYTES,        \
            (U8)(uuid[15]), \
            (U8)(uuid[14]), \
            (U8)(uuid[13]), \
            (U8)(uuid[12]), \
            (U8)(uuid[11]), \
            (U8)(uuid[10]), \
            (U8)(uuid[9]),  \
            (U8)(uuid[8]),  \
            (U8)(uuid[7]),  \
            (U8)(uuid[6]),  \
            (U8)(uuid[5]),  \
            (U8)(uuid[4]),  \
            (U8)(uuid[3]),  \
            (U8)(uuid[2]),  \
            (U8)(uuid[1]),  \
            (U8)(uuid[0])

#define SDP_UINT_8BIT(uint) \
            DETD_UINT + DESD_1BYTE,          \
            (U8)(uint)

#define SDP_UINT_16BIT(uint) \
            DETD_UINT + DESD_2BYTES,         \
            (U8)(((uint) & 0xff00) >> 8),    \
            (U8)((uint) & 0x00ff)

#define SDP_UINT_32BIT(uint) \
            DETD_UINT + DESD_4BYTES,            \
            (U8)(((uint) & 0xff000000) >> 24),  \
            (U8)(((uint) & 0x00ff0000) >> 16),  \
            (U8)(((uint) & 0x0000ff00) >> 8),   \
            (U8)((uint) & 0x000000ff)

#define SDP_UINT_64BIT(uint)                    \
            DETD_UINT + DESD_8BYTES,            \
            uint

#define SDP_UINT_128BIT(uint)                   \
            DETD_UINT + DESD_16BYTES,           \
            uint

#define SDP_TEXT_8BIT(size) \
            DETD_TEXT + DESD_ADD_8BITS,\
            (U8)(size)

#define SDP_TEXT_16BIT(size) \
            DETD_TEXT + DESD_ADD_16BITS,\
            (U8)(((size) & 0xff00) >> 8),\
            (U8)((size) & 0x00ff)

#define SDP_TEXT_32BIT(size) \
            DETD_TEXT + DESD_ADD_32BITS,        \
            (U8)(((size) & 0xff000000) >> 24),  \
            (U8)(((size) & 0x00ff0000) >> 16),  \
            (U8)(((size) & 0x0000ff00) >> 8),   \
            (U8)((size) & 0x000000ff)

#define SDP_BOOL(value) \
            DETD_BOOL + DESD_1BYTE,          \
            (U8)(value)

#define BT_SDP_U8_VALUE(v) ((uint8_t)((v)&0xFF))

#define BT_SDP_SPLIT_16BITS_BE(v) \
    BT_SDP_U8_VALUE((v)>>8),BT_SDP_U8_VALUE(v)

#define ATT_SERVICE_UUID 0x0118

#define AID_SERVICE_RECORD_HANDLE               0x0000
#define AID_SERVICE_CLASS_ID_LIST               0x0001
#define AID_SERVICE_RECORD_STATE                0x0002
#define AID_SERVICE_ID                          0x0003
#define AID_PROTOCOL_DESC_LIST                  0x0004
#define AID_BROWSE_GROUP_LIST                   0x0005
#define AID_LANG_BASE_ID_LIST                   0x0006
#define AID_SERVICE_INFO_TIME_TO_LIVE           0x0007
#define AID_SERVICE_AVAILABILITY                0x0008
#define AID_BT_PROFILE_DESC_LIST                0x0009
#define AID_DOC_URL                             0x000a
#define AID_CLIENT_EXEC_URL                     0x000b
#define AID_ICON_URL                            0x000c
#define AID_ADDITIONAL_PROT_DESC_LISTS          0x000d
#define AID_SERVICE_NAME                        0x0000
#define AID_SERVICE_DESCRIPTION                 0x0001
#define AID_PROVIDER_NAME                       0x0002
#define AID_IP_SUBNET                           0x0200
#define AID_GROUP_ID                            0x0200
#define AID_VERSION_NUMBER_LIST                 0x0200
#define AID_SERVICE_DATABASE_STATE              0x0201
#define AID_SERVICE_VERSION                     0x0300
#define AID_EXTERNAL_NETWORK                    0x0301
#define AID_SUPPORTED_DATA_STORES_LIST          0x0301
#define AID_FAX_CLASS_1_SUPPORT                 0x0302
#define AID_REMOTE_AUDIO_VOL_CONTROL            0x0302
#define AID_FAX_CLASS_20_SUPPORT                0x0303
#define AID_SUPPORTED_FORMATS_LIST              0x0303
#define AID_FAX_CLASS_2_SUPPORT                 0x0304
#define AID_AUDIO_FEEDBACK_SUPPORT              0x0305
#define AID_NETWORK_ADDRESS                     0x0306
#define AID_WAP_GATEWAY                         0x0307
#define AID_HOME_PAGE_URL                       0x0308
#define AID_WAP_STACK_TYPE                      0x0309
#define AID_SECURITY_DESC                       0x030A
#define AID_NET_ACCESS_TYPE                     0x030B
#define AID_MAX_NET_ACCESS_RATE                 0x030C
#define AID_IPV4_SUBNET                         0x030D
#define AID_IPV6_SUBNET                         0x030E
#define AID_SUPPORTED_CAPABILITIES              0x0310
#define AID_SUPPORTED_FEATURES                  0x0311
#define AID_SUPPORTED_FUNCTIONS                 0x0312
#define AID_TOTAL_IMAGE_DATA_CAPACITY           0x0313
#define AID_SUPPORTED_REPOSITORIES              0x0314
#define AID_MAS_INSTANCE_ID                     0x0315
#define AID_SUPPORTED_MESSAGE_TYPES             0x0316
#define AID_SUPPORTED_DOC_FORMATS               0x0350
#define AID_SUPPORTED_CHAR_REPERTOIRES          0x0352
#define AID_SUPPORTED_XHTML_IMAGE_FORMATS       0x0354
#define AID_COLOR_SUPPORTED                     0x0356
#define AID_PRINTER_1284ID                      0x0358
#define AID_DUPLEX_SUPPORTED                    0x035E
#define AID_SUPPORTED_MEDIA_TYPES               0x0360
#define AID_MAX_MEDIA_WIDTH                     0x0362
#define AID_MAX_MEDIA_LENGTH                    0x0364
#define AID_HID_PARSERVERSION                   0x201
#define AID_HID_DEVICESUBCLASS                  0x202
#define AID_HID_CONTRYCODE                      0x203
#define AID_HID_VIRTUALCABLE                    0x204
#define AID_HID_RECONNECTINITIATE               0x205
#define AID_HID_DESCRIPTORLIST                  0x206
#define AID_HID_LANGIDBASELIST                  0x207
#define AID_HID_BATTERYPOWER                    0x209
#define AID_HID_REMOTEWAKE                      0x20A
#define AID_HID_SUPERVISIONTIMEOUT              0x20C
#define AID_HID_NORMALLYCONNECTABLE             0x20D
#define AID_HID_BOOTDEVICE                      0x20E
#define AID_HID_SSRHOSTMAXLATENCY               0x20F
#define AID_HID_SSRHOSTMINTIMEOUT               0x210

#define SC_SERVICE_DISCOVERY_SERVER             0x1000
#define SC_BROWSE_GROUP_DESC                    0x1001
#define SC_PUBLIC_BROWSE_GROUP                  0x1002
#define SC_SERIAL_PORT                          0x1101
#define SC_LAN_ACCESS_PPP                       0x1102
#define SC_DIALUP_NETWORKING                    0x1103
#define SC_IRMC_SYNC                            0x1104
#define SC_OBEX_OBJECT_PUSH                     0x1105
#define SC_OBEX_FILE_TRANSFER                   0x1106
#define SC_IRMC_SYNC_COMMAND                    0x1107
#define SC_HEADSET                              0x1108
#define SC_CORDLESS_TELEPHONY                   0x1109
#define SC_AUDIO_SOURCE                         0x110A
#define SC_AUDIO_SINK                           0x110B
#define SC_AV_REMOTE_CONTROL_TARGET             0x110C
#define SC_AUDIO_DISTRIBUTION                   0x110D
#define SC_AV_REMOTE_CONTROL                    0x110E
#define SC_VIDEO_CONFERENCING                   0x110F
#define SC_INTERCOM                             0x1110
#define SC_FAX                                  0x1111
#define SC_HEADSET_AUDIO_GATEWAY                0x1112
#define SC_WAP                                  0x1113
#define SC_WAP_CLIENT                           0x1114
#define SC_PANU                                 0x1115
#define SC_NAP                                  0x1116
#define SC_GN                                   0x1117
#define SC_DIRECT_PRINTING                      0x1118
#define SC_REFERENCE_PRINTING                   0x1119
#define SC_IMAGING                              0x111A
#define SC_IMAGING_RESPONDER                    0x111B
#define SC_IMAGING_AUTOMATIC_ARCHIVE            0x111C
#define SC_IMAGING_REFERENCED_OBJECTS           0x111D
#define SC_HANDSFREE                            0x111E
#define SC_HANDSFREE_AUDIO_GATEWAY              0x111F
#define SC_DIRECT_PRINTING_REF_OBJECTS          0x1120
#define SC_REFLECTED_UI                         0x1121
#define SC_BASIC_PRINTING                       0x1122
#define SC_PRINTING_STATUS                      0x1123
#define SC_HUMAN_INTERFACE_DEVICE               0x1124
#define SC_HCR                                  0x1125
#define SC_HCR_PRINT                            0x1126
#define SC_HCR_SCAN                             0x1127
#define SC_ISDN                                 0x1128
#define SC_VIDEO_CONFERENCING_GW                0x1129
#define SC_UDI_MT                               0x112A
#define SC_UDI_TA                               0x112B
#define SC_AUDIO_VIDEO                          0x112C
#define SC_SIM_ACCESS                           0x112D
#define SC_PBAP_CLIENT                          0x112E
#define SC_PBAP_SERVER                          0x112F
#define SC_PBAP_PROFILE                         0x1130
#define SC_MAP_SERVER                           0x1132
#define SC_MAP_NOTIFY_SERVER                    0x1133
#define SC_MAP_PROFILE                          0x1134
#define SC_PNP_INFO                             0x1200
#define SC_GENERIC_NETWORKING                   0x1201
#define SC_GENERIC_FILE_TRANSFER                0x1202
#define SC_GENERIC_AUDIO                        0x1203
#define SC_GENERIC_TELEPHONY                    0x1204
#define SC_UPNP_SERVICE                         0x1205
#define SC_UPNP_IP_SERVICE                      0x1206
#define SC_ESDP_UPNP_IP_PAN                     0x1300
#define SC_ESDP_UPNP_IP_LAP                     0x1301
#define SC_ESDP_UPNP_L2CAP                      0x1302
#define SC_VIDEO_SOURCE                         0x1303
#define SC_VIDEO_SINK                           0x1304
#define SC_VIDEO_DISTRIBUTION                   0x1305

#define PROT_SDP                     0x0001
#define PROT_UDP                     0x0002
#define PROT_RFCOMM                  0x0003
#define PROT_TCP                     0x0004
#define PROT_TCS_BIN                 0x0005
#define PROT_TCS_AT                  0x0006
#define PROT_OBEX                    0x0008
#define PROT_IP                      0x0009
#define PROT_FTP                     0x000A
#define PROT_HTTP                    0x000C
#define PROT_WSP                     0x000E
#define PROT_BNEP                    0x000F
#define PROT_UPNP                    0x0010
#define PROT_HIDP                    0x0011
#define PROT_HCR_CONTROL_CHANNEL     0x0012
#define PROT_HCR_DATA_CHANNEL        0x0014
#define PROT_HCR_NOTIFICATION        0x0016
#define PROT_AVCTP                   0x0017
#define PROT_AVDTP                   0x0019
#define PROT_AVCTP_BROWSING          0x001B
#define PROT_UDI_C                   0x001D
#define PROT_L2CAP                   0x0100

#define BT_SDP_BASE_UUID_BE {0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB}

typedef enum {
    BT_SDP_SERV_UUID_UNKNOWN = 0,
    BT_SDP_SERV_UUID_HFP_HF,    // 0x111E
    BT_SDP_SERV_UUID_HFP_AG,    // 0x111F
    BT_SDP_SERV_UUID_HSP_HS,    // 0x1108
    BT_SDP_SERV_UUID_HSP_AG,    // 0x1112
    BT_SDP_SERV_UUID_A2DP,      // 0x110D
    BT_SDP_SERV_UUID_A2DP_SRC,  // 0x110A
    BT_SDP_SERV_UUID_A2DP_SNK,  // 0x110B
    BT_SDP_SERV_UUID_AVRCP,     // 0x110E
    BT_SDP_SERV_UUID_AVRCP_CT,  // 0x110F
    BT_SDP_SERV_UUID_AVRCP_TG,  // 0x110C
    BT_SDP_SERV_UUID_DIP,       // 0x1200
    BT_SDP_SERV_UUID_OPP,       // 0x1105
    BT_SDP_SERV_UUID_HID,       // 0x1124
    BT_SDP_SERV_UUID_PAN_PANU,  // 0x1115
    BT_SDP_SERV_UUID_PAN_NAP,   // 0x1116
    BT_SDP_SERV_UUID_PAN_GN,    // 0x1117
    BT_SDP_SERV_UUID_PBAP,      // 0x1130
    BT_SDP_SERV_UUID_PBAP_PCE,  // 0x112E
    BT_SDP_SERV_UUID_PBAP_PSE,  // 0x112F
    BT_SDP_SERV_UUID_MAP,       // 0x1134
    BT_SDP_SERV_UUID_MAP_MAS,   // 0x1132
    BT_SDP_SERV_UUID_MAP_MNS,   // 0x1133
    BT_SDP_SERV_UUID_SAP,       // 0x112D
    BT_SDP_SERV_UUID_MAX_NUM,
} bt_sdp_service_uuid_t;

typedef enum {
    BT_SDP_SERVICE_SEARCH_TYPE_UNKNOWN = 0,
    BT_SDP_SERVICE_SEARCH_REQUEST = 2,
    BT_SDP_SERVICE_ATTRIBUTE_REQUEST = 4,
    BT_SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST = 6,
} bt_sdp_service_search_type_t;

typedef enum {
    BT_SDP_RESPONSE_TYPE_HANDLE_LIST,
    BT_SDP_RESPONSE_TYPE_ATTRIBUTE_LIST,
} bt_sdp_response_type_t;

typedef struct bt_sdp_record_attr_t {
    uint16_t attr_id;
    uint16_t data_len;
    const uint8_t *data;
} __attribute__ ((__packed__)) bt_sdp_record_attr_t;

typedef struct {
    bt_sdp_record_attr_t *attrs;
    uint32_t attr_count;
    uint32_t COD;
} bt_sdp_record_param_t;

typedef struct {
    int total_remain_len;
    uint16_t record_count;
    uint16_t curr_list_len;
    const uint8_t *curr_attr_list;
} bt_sdp_attr_list_t;

typedef struct {
    bt_sdp_response_type_t type;
    uint16_t handle_count;
    uint32_t *handle_list;
    bt_sdp_attr_list_t attr_list;
} bt_sdp_response_data_t;

typedef struct bt_sdp_remote_record_t {
    struct bt_sdp_remote_record_t *next;
    bt_sdp_service_uuid_t uuid;
} bt_sdp_remote_record_t;

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t a2dp_version;
    uint16_t avdtp_version;
    uint16_t supported_features;
} bt_sdp_a2dp_record_t;

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t avrcp_version;
    uint16_t avctp_version;
    uint16_t supported_features;
} bt_sdp_avrcp_record_t;

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t hfp_version;   // hfp or hsp version
    uint16_t supported_features; // only hf or hs has RVC feature
    uint8_t server_channel; // remote rfcomm server channel
    uint8_t call_reject_ability; // only ag have this attribute
} bt_sdp_hfp_record_t;

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t opp_version;
    uint16_t supported_features;
    uint16_t l2cap_psm;
    uint8_t server_channel;
} bt_sdp_opp_record_t;

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t hid_version;
    uint16_t hid_control_psm;
    uint16_t hid_interrupt_psm;
    uint16_t parser_version;
    uint8_t device_subclass;
    uint8_t contry_code;
    bool virtual_cable;
    bool reconnect_initiate;
    bool boot_device;
    uint16_t descriptor_list_len;
    uint8_t *descriptor_list_data;
} bt_sdp_hid_record_t;

#define BT_SDP_PAN_MAX_SUPPORT_NETWORK_PROTOCOLS 16

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t pan_version;
    uint16_t bnep_version;
    uint16_t num_network_proto_types;
    uint16_t supported_network_proto_types[BT_SDP_PAN_MAX_SUPPORT_NETWORK_PROTOCOLS];
    uint16_t security_description;
    uint16_t network_access_type;
    uint32_t max_network_access_rate;
} bt_sdp_pan_record_t;

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t pbap_version;
    uint16_t l2cap_psm;
    uint8_t server_channel;
    uint8_t supported_repositories;
    uint32_t pbap_supported_features;
} bt_sdp_pbap_record_t;

#define BT_SDP_MAP_MAX_MAS_INSTANCE 12 // Up to 12 MAS Instances may be supported by an MSE device
#define BT_SDP_MAP_MAX_SERVICE_NAME_LEN (31)

typedef struct {
    bt_sdp_remote_record_t head;
    bool mas_instance_found;
    uint8_t mas_instance_id;
    uint16_t map_version;
    uint16_t l2cap_psm;
    uint8_t server_channel;
    uint8_t supported_message_types;
    uint32_t map_supported_features;
    char service_name[BT_SDP_MAP_MAX_SERVICE_NAME_LEN+1];
} bt_sdp_map_record_t;

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t sap_version;
    uint8_t server_channel;
} bt_sdp_sap_record_t;

typedef struct {
    bt_sdp_remote_record_t head;
    uint16_t dip_version;
    uint16_t vendor_id;
    uint16_t vendor_id_source;
    uint16_t product_id;
    uint16_t product_version;
    bool primary_record;
} bt_sdp_dip_record_t;

typedef struct {
    uint16_t error_code;
    bt_sdp_service_uuid_t request_uuid;
    bt_sdp_response_data_t data;
    bt_sdp_remote_record_t *record;
    void *control;
    void *priv;
} bt_sdp_search_result_t;

typedef union {
    bt_sdp_search_result_t *result;
} bt_sdp_callback_param_t;

typedef enum {
    BT_SDP_EVENT_RESPONSE,
    BT_SDP_EVENT_RESPONSE_ERROR,
    BT_SDP_EVENT_CHANNEL_CLOSED,
} bt_sdp_event_t;

typedef int (*bt_sdp_callback_t)(const bt_bdaddr_t *bd_addr, bt_sdp_event_t event, bt_sdp_callback_param_t param);

typedef struct {
    bt_sdp_service_uuid_t remote_uuid;
    bt_sdp_callback_t sdp_callback;
    void *priv;
} bt_sdp_uuid_search_t;

typedef struct {
    bt_sdp_service_search_type_t type;
    uint16_t request_len;
    const uint8_t *request_data; // can be local data
    bt_sdp_callback_t sdp_callback;
    void *priv;
} bt_sdp_service_search_t;

bt_status_t bt_sdp_uuid_search(const bt_bdaddr_t *bd_addr, const bt_sdp_uuid_search_t *param);
bt_status_t bt_sdp_service_search(const bt_bdaddr_t *bd_addr, const bt_sdp_service_search_t *param);
bt_status_t bt_sdp_create_record(const bt_sdp_record_attr_t *attr_list, int attr_count);
bt_status_t bt_sdp_remove_record(const bt_sdp_record_attr_t *attr_list); // attr_list shall be global defined array

#ifdef __cplusplus
}
#endif
#endif /* __BT_SDP_SERVICE_H__ */

