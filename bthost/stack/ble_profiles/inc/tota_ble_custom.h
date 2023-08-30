/***************************************************************************
 *
 * Copyright 2023 Add by Jay
 * All rights reserved. All unpublished rights reserved.
 *
 ****************************************************************************/

#ifdef CMT_008_BLE_ENABLE

#include "tota_ble.h"


typedef enum
{
    SUCCESS_STATUS = 0x00,
    NOT_SUPPORT_STATUS,
    DISALLOW_STATUS,
    NO_RESOURCE_STATUS,
    FORMAT_ERROR_STATUS,
    PARAMETER_ERROR_STATUS,
    FAIL_STATUS = 0xFF,
} TOTA_BLE_STATUS_E;




bool custom_tota_ble_send_notification(uint16_t handle, uint8_t* ptrData, uint32_t length);
void custom_tota_ble_data_handle(uint8_t* ptrData, uint32_t length);


#endif /* CMT_008_BLE_ENABLE */
