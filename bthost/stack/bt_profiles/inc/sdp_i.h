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
#ifndef __SDP_I_H__
#define __SDP_I_H__
#include "bluetooth.h"
#include "l2cap_i.h"
#include "bt_sdp_service.h"
#ifdef __cplusplus
extern "C" {
#endif

/*
    @brief SDP data element type_size_table defination
    bit0-5: len
    bit6: 1 -- type_length is value
    bit7: 0 -- len represent the size of data; 1 -- len represent the data size is contained
            in the additional len bytes
*/
#define SDP_DATA_LENGTH_MASK        0x3F
#define SDP_TYPE_SIZE_VALUE         0x40
#define SDP_DATA_CONTAINED_IN_BYTE  0x80

/*
    @brief PDU defination
*/
#define SDP_PDU_ID_OFFSET        0
#define SDP_TRANS_ID_OFFSET      1
#define SDP_PARAM_LEN_OFFSET     3
#define SDP_PDU_LEN              5

/*
    @brief sdp_service_search_request defination
        ServiceSearchPattern        -- varies bytes
        MaximumServiceRecordCount   -- 2 bytes
        ContinuationState           -- 1-17 bytes
*/
#define SDP_SERVICE_SEARCH_MIN_SIZE  8
#define SDP_MAX_SER_RECORD_COUNT_LEN 2

/*
    @brief sdp_service_attribute_request defination
        ServiceRecordHandle         -- 4 bytes
        MaximumAttributeByteCount   -- 2 bytes
        AttributeIDList             -- varies bytes
        ContinuationState           -- 1-17 bytes
*/
#define SDP_SERVICE_ATTR_MIN_SIZE        12
#define SDP_SER_RECORD_HANDLE_OFFSET     0
#define SDP_SER_RECORD_HANDLE_LEN        4
#define SDP_MAX_ATTR_BYTE_COUNT_OFFSET   4
#define SDP_MAX_ATTR_BYTE_COUNT_LEN      2
#define SDP_SER_ATTR_REQ_ID_LIST_OFFSET  6

/*
    @brief sdp_service_search_attribute_request defination
        ServiceSearchPattern        -- varies bytes
        MaximumAttributeByteCount   -- 2 bytes
        AttributeIDList             -- varies bytes
        ContinuationState           -- 1-17 bytes
*/
#define SDP_SERVICE_SEARCH_ATTR_MIN_SIZE    13
//#define SDP_MAX_ATTR_BYTE_COUNT_LEN  2

/*
    @brief Data element type defination
    Refer to Table3.1 in Core_v5.0.pdf
*/
#define DE_TYPE_MASK 0x1F
#define DE_TYPE_OFFSET 3
enum data_element_type {
    DE_TYPE_NIL = 0,        // 0x00
    DE_TYPE_UINT = 1,       // 0x08 0x09 0x0a 0x0b 0x0c uint8~128
    DE_TYPE_S2CMPLINT = 2,  // 0x10 0x11 0x12 0x13 0x14 sint8~128
    DE_TYPE_UUID = 3,       // 0x19 0x1a 0x1c 2-byte 4-byte 16-byte uuid
    DE_TYPE_TEXTSTR = 4,    // 0x25 0x26 0x27
    DE_TYPE_BOOL = 5,       // 0x28
    DE_TYPE_DESEQ = 6,      // 0x35 0x36 0x37
    DE_TYPE_DEALT = 7,      // 0x3d 0x3e 0x3f
    DE_TYPE_URL = 8,        // 0x45 0x46 0x47
};

/*
    @brief Data element size index defination
    Refer to Table3.2 in Core_v5.0.pdf
*/
#define DE_SIZE_MASK    0x07
enum data_element_size {
    DE_SIZE_0 = 0,
    DE_SIZE_1 = 1,
    DE_SIZE_2 = 2,
    DE_SIZE_3 = 3,
    DE_SIZE_4 = 4,
    DE_SIZE_5 = 5,
    DE_SIZE_6 = 6,
    DE_SIZE_7 = 7,
};

/*
    @brief Service Attribute ID
*/
#define SERV_ATTRID_SERVICE_RECORD_HANDLE               0x0000
#define SERV_ATTRID_SERVICE_CLASS_ID_LIST               0x0001
#define SERV_ATTRID_SERVICE_RECORD_STATE                0x0002
#define SERV_ATTRID_SERVICE_ID                          0x0003
#define SERV_ATTRID_PROTOCOL_DESC_LIST                  0x0004
#define SERV_ATTRID_BROWSE_GROUP_LIST                   0x0005
#define SERV_ATTRID_LANG_BASE_ID_LIST                   0x0006
#define SERV_ATTRID_SERVICE_INFO_TIME_TO_LIVE           0x0007
#define SERV_ATTRID_SERVICE_AVAILABILITY                0x0008
#define SERV_ATTRID_BT_PROFILE_DESC_LIST                0x0009
#define SERV_ATTRID_DOC_URL                             0x000a
#define SERV_ATTRID_CLIENT_EXEC_URL                     0x000b
#define SERV_ATTRID_ICON_URL                            0x000c
#define SERV_ATTRID_ADDITIONAL_PROT_DESC_LISTS          0x000d
#define SERV_ATTRID_SERVICE_NAME                        0X0100
#define SERV_ATTRID_SERVICE_DESCRIPTION                 0x0101
#define SERV_ATTRID_PROVIDER_NAME                       0X0102
#define SERV_ATTRID_SDP_DIP_SPECIFICATION_ID            0x0200
#define SERV_ATTRID_SDP_DIP_VENDOR_ID                   0x0201
#define SERV_ATTRID_SDP_DIP_PRODUCT_ID                  0x0202
#define SERV_ATTRID_SDP_DIP_PRODUCT_VERSION             0x0203
#define SERV_ATTRID_SDP_DIP_PRIMARY_RECORD              0x0204
#define SERV_ATTRID_SDP_DIP_VENDOR_ID_SOURCE            0x0205
#define SERV_ATTRID_HID_PARSERVERSION                   0x0201
#define SERV_ATTRID_HID_DEVICESUBCLASS                  0x0202
#define SERV_ATTRID_HID_CONTRYCODE                      0x0203
#define SERV_ATTRID_HID_VIRTUALCABLE                    0x0204
#define SERV_ATTRID_HID_RECONNECTINITIATE               0x0205
#define SERV_ATTRID_HID_DESCRIPTORLIST                  0x0206
#define SERV_ATTRID_HID_LANGIDBASELIST                  0x0207
#define SERV_ATTRID_HID_BATTERYPOWER                    0x0209
#define SERV_ATTRID_HID_REMOTEWAKE                      0x020A
#define SERV_ATTRID_HID_SUPERVISIONTIMEOUT              0x020C
#define SERV_ATTRID_HID_NORMALLYCONNECTABLE             0x020D
#define SERV_ATTRID_HID_BOOTDEVICE                      0x020E
#define SERV_ATTRID_HID_SSRHOSTMAXLATENCY               0x020F
#define SERV_ATTRID_HID_SSRHOSTMINTIMEOUT               0x0210
#define SERV_ATTRID_GOEP_L2CAP_PSM                      0x0200
#define SERV_ATTRID_SDP_VERSION_NUMBER_LIST             0x0200
#define SERV_ATTRID_SDP_HFP_AG_NETWORK                  0x0301
#define SERV_ATTRID_SDP_HSP_REMOTE_VOLUME_CONTROL       0x0302
#define SERV_ATTRID_SUPPORTED_FEATURES                  0x0311
#define SERV_ATTRID_PBAP_SUPPORTED_REPOSITORIES         0x0314
#define SERV_ATTRID_PBAP_SUPPORTED_FEATURES             0x0317
#define SERV_ATTRID_MAP_INSTANCE_ID                     0x0315
#define SERV_ATTRID_MAP_SUPPORTED_MESSAGE_TYPES         0x0316
#define SERV_ATTRID_MAP_SUPPORTED_FEATURES              0x0317
#define SERV_ATTRID_PAN_IP_SUBNET                       0x0200
#define SERV_ATTRID_PAN_SECURITY_DESCRIPTION            0x030A
#define SERV_ATTRID_PAN_NET_ACCESS_TYPE                 0x030B
#define SERV_ATTRID_PAN_MAX_NET_ACCESS_RATE             0x030C
#define SERV_ATTRID_PAN_IPV4_SUBNET                     0x030D
#define SERV_ATTRID_PAN_IPV6_SUBNET                     0x030E
#define SERV_ATTRID_OPP_SUPPORTED_FEATURES              0x0303

#define HFP_UUID_Headset        0x1108
#define HFP_UUID_HandsFree      0x111E
#define HFP_UUID_AudioGateway   0x111F

/*
    @brief UUID
*/
#define SERV_UUID_GENERIC_AUDIO            0x12,0x03
#define SERV_UUID_Headset                  0x11,0x08
#define SERV_UUID_HeadsetAudioGateway      0x11,0x12
#define SERV_UUID_HandsFree                0x11,0x1E
#define SERV_UUID_HandsFreeAudioGateway    0x11,0x1F
#define SERV_UUID_SDP                      0x00,0x01
#define SERV_UUID_UDP                      0x00,0x02
#define SERV_UUID_RFCOMM                   0x00,0x03
#define SERV_UUID_TCP                      0x00,0x04
#define SERV_UUID_TCS_BIN                  0x00,0x05
#define SERV_UUID_TCS_AT                   0x00,0x06
#define SERV_UUID_ATT                      0x00,0x07
#define SERV_UUID_OBEX                     0x00,0x08
#define SERV_UUID_IP                       0x00,0x09
#define SERV_UUID_FTP                      0x00,0x0A
#define SERV_UUID_HTTP                     0x00,0x0C
#define SERV_UUID_WSP                      0x00,0x0E
#define SERV_UUID_BNEP                     0x00,0x0F
#define SERV_UUID_UPNP                     0x12,0x00
#define SERV_UUID_HID_PROTOCOL             0x00,0x11
#define SERV_UUID_HID_CTRL                 0x00,0x11
#define SERV_UUID_HID_INTR                 0x00,0x13
#define SERV_UUID_HardcopyControlChannel   0x00,0x12
#define SERV_UUID_HardcopyDataChannel      0x00,0x14
#define SERV_UUID_HardcopyNotification     0x00,0x16
#define SERV_UUID_AVCTP                    0x00,0x17
#define SERV_UUID_AVDTP                    0x00,0x19
#define SERV_UUID_CMTP                     0x00,0x1B
#define SERV_UUID_MCAPControlChannel       0x00,0x1E
#define SERV_UUID_MCAPDataChannel          0x00,0x1F
#define SERV_UUID_L2CAP                    0x01,0x00
#define SERV_UUID_SPP                      0x11,0x01
#define SERV_UUID_AUDIOSOURCE              0x11,0x0A
#define SERV_UUID_AUDIOSINK                0x11,0x0B
#define SERV_UUID_HID                      0x11,0x24
#define SERV_UUID_MAP                      0x11,0x34
#define SERV_UUID_AdvancedAudioDistribution                0x11,0x0D
#define SERV_UUID_AV_REMOTE_CONTROL        0x11,0x0E
#define SERV_UUID_AV_REMOTE_CONTROL_TARGET 0x11,0x0C
#define SERV_UUID_AV_REMOTE_CONTROL_CONTROLLER 0x11,0x0F
#define SERV_UUID_PBAP_CLIENT              0x11,0x2E
#define SERV_UUID_PBAP_SERVER              0x11,0x2F
#define SERV_UUID_PBAP                     0x11,0x30
#define SERV_UUID_MAP_ACCESS_SERVER        0x11,0x32
#define SERV_UUID_MAP_NOTIFY_SERVER        0x11,0x33
#define SERV_UUID_MAP                      0x11,0x34
#define SERV_UUID_PAN_ROLE_PANU            0x11,0x15
#define SERV_UUID_PAN_ROLE_NAP             0x11,0x16
#define SERV_UUID_PAN_ROLE_GN              0x11,0x17
#define SERV_UUID_OPP_SERVER               0x11,0x05

/*
    @brief Service Record defination helper macros
*/
#define SDP_U8_VALUE(v) ((v)&0xFF)

#define SDP_SPLIT_16BITS_BE(v) \
    SDP_U8_VALUE((v)>>8),SDP_U8_VALUE(v)

#define SDP_SPLIT_32BITS_BE(v) \
    SDP_U8_VALUE((v)>>24),SDP_U8_VALUE((v)>>16),SDP_U8_VALUE((v)>>8),SDP_U8_VALUE(v)

#define X_SDP_COMBINE_16BITS_BE(a,b) \
    (SDP_U8_VALUE(a)<<8) | SDP_U8_VALUE(b)

#define SDP_COMBINE_16BITS_BE(...) \
    X_SDP_COMBINE_16BITS_BE(__VA_ARGS__)

/*
    @brief NIL
*/
#define SDP_DE_NIL_H1_D0 \
    DE_TYPE_NIL<<3

/*
    @brief Unsigned Integer
*/
#define SDP_DE_UINT(size_index) \
    DE_TYPE_UINT<<3|size_index

#define SDP_DE_UINT_H1_D1 \
    SDP_DE_UINT(DE_SIZE_0)

#define SDP_DE_UINT_H1_D2 \
    SDP_DE_UINT(DE_SIZE_1)

#define SDP_DE_UINT_H1_D4 \
    SDP_DE_UINT(DE_SIZE_2)

#define SDP_DE_UINT_H1_D8 \
    SDP_DE_UINT(DE_SIZE_3)

#define SDP_DE_UINT_H1_D16 \
    SDP_DE_UINT(DE_SIZE_4)

/*
    @brief Signed twos-complement integer
*/
#define SDP_DE_S2CMPLINT(size_index) \
    DE_TYPE_S2CMPLINT<<3|size_index

#define SDP_DE_S2CMPLINT_H1_D1 \
    SDP_DE_S2CMPLINT(DE_SIZE_0)

#define SDP_DE_S2CMPLINT_H1_D2 \
    SDP_DE_S2CMPLINT(DE_SIZE_1)

#define SDP_DE_S2CMPLINT_H1_D4 \
    SDP_DE_S2CMPLINT(DE_SIZE_2)

#define SDP_DE_S2CMPLINT_H1_D8 \
    SDP_DE_S2CMPLINT(DE_SIZE_3)

#define SDP_DE_S2CMPLINT_H1_D16 \
    SDP_DE_S2CMPLINT(DE_SIZE_4)

/*
    @brief UUID
*/
#define SDP_DE_UUID(size_index) \
    DE_TYPE_UUID<<3|size_index

#define SDP_DE_UUID_H1_D2 \
    SDP_DE_UUID(DE_SIZE_1)

#define SDP_DE_UUID_H1_D4 \
    SDP_DE_UUID(DE_SIZE_2)

#define SDP_DE_UUID_H1_D16 \
    SDP_DE_UUID(DE_SIZE_4)

/*
    @brief Text string
*/
#define SDP_DE_TEXTSTR_8BITSIZE_H2_D(size) \
    DE_TYPE_TEXTSTR<<3|DE_SIZE_5,SDP_U8_VALUE(size)

#define SDP_DE_TEXTSTR_16BITSIZE_H3_D(size) \
    DE_TYPE_TEXTSTR<<3|DE_SIZE_6,SDP_U8_VALUE(size),SDP_U8_VALUE(size>>8)

#define SDP_DE_TEXTSTR_32BITSIZE_H5_D(size) \
    DE_TYPE_TEXTSTR<<3|DE_SIZE_7,SDP_U8_VALUE(size),SDP_U8_VALUE(size>>8),SDP_U8_VALUE(size>>16),SDP_U8_VALUE(size>>24)

/*
    @brief Boolean
*/
#define SDP_DE_BOOL_H1_D1 \
    DE_TYPE_BOOL<<3|DE_SIZE_0

/*
    @brief Data element sequence
*/
#define SDP_DE_DESEQ_8BITSIZE_H2_D(size) \
    DE_TYPE_DESEQ<<3|DE_SIZE_5,SDP_U8_VALUE(size)

#define SDP_DE_DESEQ_16BITSIZE_H3_D(size) \
    DE_TYPE_DESEQ<<3|DE_SIZE_6,SDP_U8_VALUE(size),SDP_U8_VALUE(size>>8)

#define SDP_DE_DESEQ_32BITSIZE_H5_D(size) \
    DE_TYPE_DESEQ<<3|DE_SIZE_7,SDP_U8_VALUE(size),SDP_U8_VALUE(size>>8),SDP_U8_VALUE(size>>16),SDP_U8_VALUE(size>>24)

/*
    @brief Data element alternative
*/
#define SDP_DE_DEALT_8BITSIZE_H2_D(size) \
    DE_TYPE_DEALT<<3|DE_SIZE_5,SDP_U8_VALUE(size)

#define SDP_DE_DEALT_16BITSIZE_H3_D(size) \
    DE_TYPE_DEALT<<3|DE_SIZE_6,SDP_U8_VALUE(size),SDP_U8_VALUE(size>>8)

#define SDP_DE_DEALT_32BITSIZE_H5_D(size) \
    DE_TYPE_DEALT<<3|DE_SIZE_7,SDP_U8_VALUE(size),SDP_U8_VALUE(size>>8),SDP_U8_VALUE(size>>16),SDP_U8_VALUE(size>>24)

/*
    @brief URL
*/
#define SDP_DE_URL_8BITSIZE_H2_D(size) \
    DE_TYPE_URL<<3|DE_SIZE_5,SDP_U8_VALUE(size)

#define SDP_DE_URL_16BITSIZE_H3_D(size) \
    DE_TYPE_URL<<3|DE_SIZE_6,SDP_U8_VALUE(size),SDP_U8_VALUE(size>>8)

#define SDP_DE_URL_32BITSIZE_H5_D(size) \
    DE_TYPE_URL<<3|DE_SIZE_7,SDP_U8_VALUE(size),SDP_U8_VALUE(size>>8),SDP_U8_VALUE(size>>16),SDP_U8_VALUE(size>>24)

#define SDP_DEF_ATTRIBUTE(attr_id,attrs) \
    { \
        attr_id, sizeof(attrs), attrs, \
    }

#define SDP_REQUEST_DATA_MAX_LEN (512)

enum sdp_error_code {
    SDP_Reserved_for_future_use = 0x0000,
    SDP_Invalid_SDP_version = 0x0001,
    SDP_Invalid_Service_Record_Handle = 0x0002,
    SDP_Invalid_request_syntax = 0x0003,
    SDP_Invalid_PDU_Size = 0x0004,
    SDP_Invalid_Continuation_State = 0x0005,
    SDP_Insufficient_Resources_to_satisfy_Request = 0x0006,
};

enum sdp_pdu_id {
    SDP_PDU_ErrorResponse = 0x01,
    SDP_PDU_ServiceSearchRequest = 0x02,
    SDP_PDU_ServiceSearchResponse = 0x03,
    SDP_PDU_ServiceAttributeRequest = 0x04,
    SDP_PDU_ServiceAttributeResponse = 0x05,
    SDP_PDU_ServiceSearchAttributeRequest = 0x06,
    SDP_PDU_ServiceSearchAttributeResponse = 0x07,
};

struct sdp_control_t;
void sdp_global_init(void);
bt_status_t sdp_close(struct sdp_control_t *sdp_chan);
bt_status_t sdp_client_service_search(const bt_bdaddr_t *remote, const bt_sdp_service_search_t *param);
bt_status_t sdp_client_request(const bt_bdaddr_t *remote, bt_sdp_uuid_search_t *param);
bt_status_t sdp_client_queue_request(const bt_bdaddr_t *remote, bt_sdp_service_uuid_t uuid);
bt_status_t sdp_create_record(const bt_sdp_record_attr_t *attr_list, int attr_count);
bt_status_t sdp_remove_record(const bt_sdp_record_attr_t *attr_list);

typedef struct {
    const uint8_t *attr_value_ptr; // attribute_value element start
    enum data_element_type elem_type; // attribute_value element type
    uint16_t elem_header_len; // attribute_value element header len
    uint16_t elem_data_len; // attribute_value element data len
} bt_sdp_attr_value_t;

typedef struct {
    const uint8_t *uuid_matched_end;
    const uint8_t *attr_value_ptr; // attribute_value element start
    enum data_element_type elem_type; // attribute_value element type
    uint16_t elem_header_len; // attribute_value element header len
    uint16_t elem_data_len; // attribute_value element data len
    uint16_t remain_data_len; // remain len including uuid attached value and left value in curr deseq
} bt_sdp_uuid_attached_value_t;

bool sdp_get_next_attribute_list(bt_sdp_attr_list_t *attr_list);
bool sdp_search_attribute_value(const bt_sdp_attr_list_t *attr_list, uint16_t attribute_id, bt_sdp_attr_value_t *out);
bool sdp_search_bool_attribute_value(bt_sdp_attr_list_t *attr_list, uint16_t attr_id, bool *out);
bool sdp_search_uint_attribute_value(bt_sdp_attr_list_t *attr_list, uint16_t attr_id, uint8_t uint_len, uint32_t *out);
bool sdp_search_text_attribute_value(bt_sdp_attr_list_t *attr_list, uint16_t attr_id, char *out_buf, uint16_t buf_max_len);
bool sdp_search_uuid_in_deseq_attribute_value(bt_sdp_attr_list_t *attr_list, uint16_t attr_id, const uint8_t *uuid, uint16_t uuid_len, uint8_t uint_len, uint32_t *out);
bool sdp_search_uuid_extra_in_deseq_attribute_value(bt_sdp_attr_list_t *attr_list, uint16_t attr_id, const uint8_t *uuid, uint16_t uuid_len, uint8_t uint_len, uint32_t *out, bt_sdp_attr_value_t *extra_value);
uint8_t sdp_search_rfcomm_server_channel_from_attr_list(bt_sdp_attr_list_t *attr_list);
bool sdp_search_uuid_from_value(const bt_sdp_attr_value_t *attr_value, const uint8_t *uuid, uint16_t uuid_len, bt_sdp_uuid_attached_value_t *out);
uint16_t sdp_get_uint_list_from_value(const bt_sdp_attr_value_t *value, uint16_t uint_len, uint32_t *out_uint_list, uint16_t max_list_len);

struct sdp_record_entry_t;
struct sdp_record_entry_t *sdp_is_record_exist(const bt_sdp_record_attr_t *attr_list);
int32 sdp_gather_global_service_uuids(uint8 in_uuid_size, uint8 *out_buff, uint32 out_buff_len, uint32 *out_len, uint32 *out_real_len);
void sdp_register_ibrt_tws_switch_protect_handle(void (*cb)(uint8_t, bt_bdaddr_t *, bool));
const char *bt_sdp_event_to_string(bt_sdp_event_t event);

#ifdef __cplusplus
}
#endif
#endif /* __SDP_I_H__ */
