/***************************************************************************
 *
 * Copyright 2023 Add by Jay
 * Handle custom application
 * All rights reserved. All unpublished rights reserved.
 *
 ****************************************************************************/

#ifdef CMT_008_BLE_ENABLE

#include "tota_ble_custom.h"
#include "rwip_config.h"
#include "gap.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "co_utils.h"
#include "bluetooth_bt_api.h"




static bool custom_tota_ble_send_ind_ntf_generic(bool isNotification, uint8_t conidx, uint16_t handle, const uint8_t* ptrData, uint32_t length)
{
    TRACE(1, "[%s]  conidx:[%d]", __func__, conidx);
    enum gatt_evt_type evtType = isNotification?GATT_NOTIFY:GATT_INDICATE;

    PRF_ENV_T(tota) *tota_env = PRF_ENV_GET(TOTA, tota);

    if ((tota_env->ntfIndEnableFlag[conidx])&(1 << (uint8_t)evtType)) 
    {

        co_buf_t* p_buf = NULL;
        prf_buf_alloc(&p_buf, length);

        uint8_t* p_data = co_buf_data(p_buf);
        memcpy(p_data, ptrData, length);

        // Dummy parameter provided to GATT
        uint16_t dummy = 0;

        // Inform the GATT that notification must be sent
        uint16_t ret = gatt_srv_event_send(conidx, tota_env->srv_user_lid, dummy, evtType,
                            tota_env->shdl + handle, p_buf);

        // Release the buffer
        co_buf_release(p_buf);

        return (GAP_ERR_NO_ERROR == ret);
    }
    else
    {
        return false;
    }
}

bool custom_tota_ble_send_notification(uint16_t handle, uint8_t* ptrData, uint32_t length)
{
    TRACE(1,"[%s] handle:[%d]", __func__, handle);
    return custom_tota_ble_send_ind_ntf_generic(true, false, handle, ptrData, length);
}

// TODO: Jay
void custom_tota_ble_data_handle(uint8_t* ptrData, uint32_t length)
{
    custom_tota_ble_send_notification(TOTA_IDX_VAL, ptrData, length);
    custom_tota_ble_send_notification(TOTA_IDX1_VAL, ptrData, length);
}

#endif /* CMT_008_BLE_ENABLE */
