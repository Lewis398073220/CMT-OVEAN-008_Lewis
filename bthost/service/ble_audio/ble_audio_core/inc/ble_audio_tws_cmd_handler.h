#ifndef __BLE_AUDIO_TWS_CMD_HANDLER_H__
#define __BLE_AUDIO_TWS_CMD_HANDLER_H__

#include "bluetooth_bt_api.h"
#include "app_ibrt_custom_cmd.h"

typedef enum
{
    IBRT_AOB_CMD_EXCH_BLE_AUDIO_INFO           = APP_IBRT_AOB_CMD_PREFIX | 0x01,
    IBRT_AOB_CMD_REQ_TRIGGER_SYNC_CAPTURE      = APP_IBRT_AOB_CMD_PREFIX | 0x02,
    IBRT_AOB_CMD_EXCH_VOLUME_STATUS            = APP_IBRT_AOB_CMD_PREFIX | 0x03,
    IBRT_AOB_CMD_EXCH_VOLUME_OFFSET_STATUS     = APP_IBRT_AOB_CMD_PREFIX | 0x04,
    IBRT_AOB_CMD_SYNC_DEV_INFO                 = APP_IBRT_AOB_CMD_PREFIX | 0x05,
    AOB_CMD_SHARE_SERVICE_INFO                 = APP_IBRT_AOB_CMD_PREFIX | 0x06,
    IBRT_AOB_CMD_SYNC_CAPTURE_US_SINCE_LATEST_ANCHOR_POINT      = APP_IBRT_AOB_CMD_PREFIX | 0x07,
    
    //new customer cmd add here
} app_ibrt_aob_cmd_code_e;

typedef struct
{
    uint8_t  streamContext;
    uint32_t master_clk_cnt;
    uint16_t master_bit_cnt;	
    int32_t  usSinceLatestAnchorPoint;
    uint32_t triggertimeUs;
    uint8_t  reserve[6];
} AOB_TWS_SYNC_TRIGGER_CAPTURE_INFO_T;

typedef struct
{
    int32_t  usSinceLatestAnchorPoint;
    uint8_t  streamContext;	
    uint8_t  reserve[7];
} AOB_TWS_SYNC_US_SINCE_LATEST_ANCHOR_POINT_T;

#ifdef __cplusplus
extern "C"{
#endif

void aob_tws_handshake_handler(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);
void aob_tws_handshake_rsp_handler(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);

void aob_tws_exchange_info_handler(uint16_t rsp_seq, uint8_t* data, uint16_t len);
void aob_tws_exchange_info_rsp_handler(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);

void app_ibrt_aob_tws_exch_ble_audio_info_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_aob_tws_exch_ble_audio_info_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_aob_tws_exch_ble_audio_info(uint8_t *p_buff, uint16_t length);
void app_ibrt_aob_tws_exch_ble_audio_info_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void aob_tws_send_sync_capture_trigger_cmd(uint8_t *p_buff, uint16_t length);
void aob_tws_sync_capture_trigger_handler(uint16_t rsp_seq, uint8_t* data, uint16_t len);
void aob_tws_sync_capture_trigger_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void aob_tws_sync_capture_trigger_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void aob_tws_send_sync_us_since_latest_anchor_point_cmd(uint8_t *p_buff, uint16_t length);
void aob_tws_sync_us_since_latest_anchor_point_handler(uint16_t rsp_seq, uint8_t* data, uint16_t len);

int app_ibrt_aob_cmd_table_get(void **cmd_tbl, uint16_t *cmd_size);

#ifdef __cplusplus
}
#endif

#endif /*__BLE_AUDIO_TWS_CMD_HANDLER_H__*/

