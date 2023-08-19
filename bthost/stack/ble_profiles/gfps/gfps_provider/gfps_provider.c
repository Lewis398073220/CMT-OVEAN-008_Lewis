/**
 ****************************************************************************************
 * @addtogroup GFPSP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "cmsis_os.h"

#if (BLE_GFPS_PROVIDER)
#include "gfps_provider.h"
#include "gfps_provider_task.h"
#include "gfps_crypto.h"
#include "prf_utils.h"
#include "prf.h"
#include "gapm.h"
#include "gatt_user.h"
#include "ke_mem.h"

#include "nvrecord_fp_account_key.h"
#include "app_gfps.h"

/*
 * MACROS
 ****************************************************************************************
 */

/// Maximal length for Characteristic values - 128 bytes
#define GFPSP_VAL_MAX_LEN                         (128)
///System ID string length
#define GFPSP_SYS_ID_LEN                          (0x08)
///IEEE Certif length (min 6 bytes)
#define GFPSP_IEEE_CERTIF_MIN_LEN                 (0x06)
///PnP ID length
#define GFPSP_PNP_ID_LEN                          (0x07)


#define GATT_DECL_PRIMARY_SERVICE_UUID       { 0x00, 0x28 }
#define GATT_DECL_CHARACTERISTIC_UUID        { 0x03, 0x28 }
#define GATT_DESC_CLIENT_CHAR_CFG_UUID       { 0x02, 0x29 }

#define GFPS_USE_128BIT_UUID

#ifdef GFPS_USE_128BIT_UUID

static const uint8_t ATT_SVC_GOOGLE_FAST_PAIR_PROVIDER[GATT_UUID_16_LEN] = {0x2C,0xFE};

#define ATT_CHAR_MODEL_ID                     {0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x33, 0x12, 0x2C, 0xFE}
#define ATT_CHAR_KEY_BASED_PAIRING            {0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x34, 0x12, 0x2C, 0xFE}
#define ATT_CHAR_PASSKEY                      {0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x35, 0x12, 0x2C, 0xFE}
#define ATT_CHAR_ACCOUNTKEY                   {0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x36, 0x12, 0x2C, 0xFE}
#define ATT_CHAR_NAME                         {0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x37, 0x12, 0x2C, 0xFE}
#define ATT_CHAR_MSG_STRAM                    {0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x39, 0x12, 0x2C, 0xFE}
#ifdef SPOT_ENABLED
#define ATT_CHAR_BEACON_ACTIONS               {0xEA, 0x0B, 0x10, 0x32, 0xDE, 0x01, 0xB0, 0x8E, 0x14, 0x48, 0x66, 0x83, 0x38, 0x12, 0x2C, 0xFE}
#endif

/*
 * DIS ATTRIBUTES
 ****************************************************************************************
 */ 

static const struct gatt_att_desc gfpsp_att_db[GFPSP_IDX_NB] =
{
    // Google fast pair service provider Declaration
    [GFPSP_IDX_SVC]                             =   {GATT_DECL_PRIMARY_SERVICE_UUID, PROP(RD), 0},

    [GFPSP_IDX_MODEL_ID_CHAR]                   =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD)|PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_MODEL_ID_VAL]                    =   {ATT_CHAR_MODEL_ID, PROP(RD)|ATT_UUID(128), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_MODEL_ID_CFG]                    =   {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), GFPSP_VAL_MAX_LEN},

    [GFPSP_IDX_KEY_BASED_PAIRING_CHAR]          =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [GFPSP_IDX_KEY_BASED_PAIRING_VAL]           =   {ATT_CHAR_KEY_BASED_PAIRING, PROP(N) | PROP(WR)|ATT_UUID(128), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_KEY_BASED_PAIRING_NTF_CFG]       =   {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), sizeof(uint16_t)},

    [GFPSP_IDX_PASSKEY_CHAR]                    =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [GFPSP_IDX_PASSKEY_VAL]                     =   {ATT_CHAR_PASSKEY, PROP(N) | PROP(WR)|ATT_UUID(128),  GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_PASSKEY_NTF_CFG]                 =    {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), sizeof(uint16_t)},

    [GFPSP_IDX_ACCOUNT_KEY_CHAR]                =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD)|PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_ACCOUNT_KEY_VAL]                 =   {ATT_CHAR_ACCOUNTKEY, PROP(N)|PROP(WR)|ATT_UUID(128), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_ACCOUNT_KEY_CFG]                 =   {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), GFPSP_VAL_MAX_LEN},

    [GFPSP_IDX_NAME_CHAR]                       =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD)|PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_NAME_VAL]                        =   {ATT_CHAR_NAME, PROP(N) |PROP(WR)|ATT_UUID(128), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_NAME_CFG]                        =   {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), GFPSP_VAL_MAX_LEN},

#ifdef SPOT_ENABLED
    [GFPSP_IDX_BEACON_ACTIONS_CHAR]             =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD)|PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_BEACON_ACTIONS_VAL]              =   {ATT_CHAR_BEACON_ACTIONS,   PROP(N)|PROP(RD)|PROP(WR)|ATT_UUID(128), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_BEACON_ACTIONS_CFG]              =   {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD)|PROP(WR), GFPSP_VAL_MAX_LEN},
#endif

#if BLE_AUDIO_ENABLED
    [GFPSP_IDX_MSG_STREAM_CHAR]                 =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [GFPSP_IDX_MSG_STREAM_VAL]                  =   {ATT_CHAR_MSG_STRAM,   PROP(RD) | ATT_UUID(128), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_MSG_STREAM_CFG]                  =   {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD)|PROP(WR), GFPSP_VAL_MAX_LEN},
#endif
};
#else

/*----------------- SERVICES ---------------------*/


/*----------------- SERVICES ---------------------*/
#define GATT_DECL_PRIMARY_SERVICE_UUID       0x2800
#define GATT_DECL_CHARACTERISTIC_UUID        0x2803
#define GATT_DESC_CLIENT_CHAR_CFG_UUID       0x2902

/// 
#define ATT_SVC_GOOGLE_FAST_PAIR_PROVIDER     0xFE2C


#define ATT_CHAR_KEY_BASED_PAIRING            0x1234
#define ATT_CHAR_PASSKEY                      0x1235
#define ATT_CHAR_ACCOUNTKEY                   0x1236
#define ATT_CHAR_NAME                         0x1237


static const struct gatt_att16_desc gfpsp_att_db[GFPSP_IDX_NB] =
{
    // Google fast pair service provider Declaration
    [GFPSP_IDX_SVC]                             =   {GATT_DECL_PRIMARY_SERVICE_UUID, PROP(RD), 0},

    [GFPSP_IDX_KEY_BASED_PAIRING_CHAR]          =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [GFPSP_IDX_KEY_BASED_PAIRING_VAL]           =   {ATT_CHAR_KEY_BASED_PAIRING, PROP(N) | PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_KEY_BASED_PAIRING_NTF_CFG]       =   {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), sizeof(uint16_t)},
    
    
    [GFPSP_IDX_PASSKEY_CHAR]                    =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [GFPSP_IDX_PASSKEY_VAL]                     =   {ATT_CHAR_PASSKEY, PROP(N) | PROP(WR),  GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_PASSKEY_NTF_CFG]                 =    {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), sizeof(uint16_t)},
    
    [GFPSP_IDX_ACCOUNT_KEY_CHAR]                =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD)|PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_ACCOUNT_KEY_VAL]                 =   {ATT_CHAR_ACCOUNTKEY,   PROP(N) |PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_ACCOUNT_KEY_CFG]                 =    {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), GFPSP_VAL_MAX_LEN},

    [GFPSP_IDX_NAME_CHAR]                       =   {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD)|PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_NAME_VAL]                        =   {ATT_CHAR_NAME,   PROP(N) |PROP(WR), GFPSP_VAL_MAX_LEN},
    [GFPSP_IDX_NAME_CFG]                        =    {GATT_DESC_CLIENT_CHAR_CFG_UUID, PROP(RD) | PROP(WR), GFPSP_VAL_MAX_LEN},

};

#endif

#if BLE_AUDIO_ENABLED
static gfpsp_ble_env_t gfpsBleEnv;
gfpsp_l2cap_env_t *gfps_ble_l2cap_get_env(uint8_t conidx)
{
    gfpsp_l2cap_env_t *env = NULL;
    for (uint8_t i = 0; i < BT_DEVICE_NUM; i++)
    {
        if (gfpsBleEnv.l2capEnv[i].conidx == conidx)
        {
            env = &(gfpsBleEnv.l2capEnv[i]);
            break;
        }
    }
    return env;
}

uint8_t gfps_ble_l2cap_send(uint8_t conidx, uint8_t *ptrData, uint32_t length)
{
    uint8_t status = GAP_ERR_NO_ERROR;

    // check if PDU transmission is supported
    if (ptrData && (length > 0))
    {
        co_buf_t* p_sdu = NULL;
        uint16_t chan_id;
        gfpsp_l2cap_env_t *env = gfps_ble_l2cap_get_env(conidx);
        if (env)
        {
            chan_id = env->chan_id;
            // allocated buffer - ensure that SIG header and reject optional parameters can be added
            if (CO_BUF_ERR_NO_ERROR == \
                co_buf_alloc(&p_sdu, L2CAP_BUFFER_HEADER_LEN, length, L2CAP_BUFFER_TAIL_LEN))
            {
                // Load SDU data
                co_buf_copy_data_from_mem(p_sdu, ptrData, length);

                l2cap_chan_sdu_send(conidx, 0, chan_id, 0, p_sdu);
 
                // release buffer
                co_buf_release(p_sdu);
            }
            else
            {
                status = GAP_ERR_INSUFF_RESOURCES;
            }
        }
        else
        {
            status = GAP_ERR_INSUFF_RESOURCES;
        }
    }

    return (status);
}

__STATIC void gfps_ble_l2cap_sdu_rx_cb(uint8_t conidx, uint8_t chan_lid, uint16_t status, co_buf_t* p_sdu)
{
    // do nothing if a complete SIG header has not been received
    if (status == GAP_ERR_NO_ERROR)
    {
        // extract header information
        uint16_t len = co_buf_data_len(p_sdu);
        //inform APP of data
        struct gfpsp_le_l2cap_recv_ind_t * ind = KE_MSG_ALLOC_DYN(GFPSP_LE_L2CAP_RECEIVED_IND,
                                                   KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                                                   KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                                                   gfpsp_le_l2cap_recv_ind_t,
                                                   len);
        ind->conidx = conidx; 
        ind->length = len;
        memcpy(ind->data, co_buf_data(p_sdu), len);
        ke_msg_send(ind);
    }
}

__STATIC void gfps_ble_l2cap_sdu_sent_cb(uint8_t conidx, uint16_t dummy, uint8_t chan_lid, uint16_t status, co_buf_t* p_sdu)
{
    return;
}

__STATIC void gfps_ble_l2cap_created_cb(uint8_t conidx, uint16_t dummy, uint8_t chan_lid, uint16_t spsm, uint16_t rx_mtu, 
                                       uint16_t tx_mtu, uint16_t rx_mps, uint16_t tx_mps)
{
    for (uint8_t i = 0; i < BT_DEVICE_NUM; i++)
    {
        if (gfpsBleEnv.l2capEnv[i].conidx == 0xFF)
        {
            gap_addr_t* bleAddr = gapm_get_connected_bdaddr(conidx);
            gfpsBleEnv.l2capEnv[i].conidx = conidx;
            gfpsBleEnv.l2capEnv[i].mtu = co_min(tx_mtu, rx_mtu);     
            gfpsBleEnv.l2capEnv[i].chan_id = chan_lid;

            // extract header information
            struct gfpsp_le_l2cap_conn_ind_t * ind = KE_MSG_ALLOC(GFPSP_LE_L2CAP_CONNECTED_IND,
                                                   KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                                                   KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                                                   gfpsp_le_l2cap_conn_ind_t);
            ind->conidx = conidx; 
            memcpy(ind->addr, bleAddr->addr, sizeof(gap_addr_t));
            ke_msg_send(ind);
            break;
        }
    }
}

__STATIC void gfps_ble_l2cap_reconfigure_cmp_cb(uint8_t conidx, uint16_t dummy, uint16_t status)
{
    return;
}

__STATIC void gfps_ble_l2cap_mtu_changed_cb(uint8_t conidx, uint16_t dummy, uint8_t chan_lid, uint16_t local_rx_mtu, uint16_t peer_rx_mtu)
{
    gfpsp_l2cap_env_t * env = gfps_ble_l2cap_get_env(conidx);
    if (env->chan_id == chan_lid)
    {
        env->mtu = co_min(local_rx_mtu, peer_rx_mtu);
    }
}

__STATIC void gfps_ble_l2cap_terminated_cb(uint8_t conidx, uint16_t dummy, uint8_t chan_lid, uint16_t reason)
{
    gfpsp_l2cap_env_t * env = gfps_ble_l2cap_get_env(conidx);
    if (env->chan_id == chan_lid)
    {
        // extract header information
        struct gfpsp_le_l2cap_disconn_ind_t * ind = KE_MSG_ALLOC(GFPSP_LE_L2CAP_DISCONNECTED_IND,
                                               KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                                               KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                                               gfpsp_le_l2cap_disconn_ind_t);
        ind->conidx = conidx;
        ke_msg_send(ind);

        memset((uint8_t *)env, 0xFF, sizeof(gfpsp_l2cap_env_t));
    }
}

__STATIC void gfps_ble_l2cap_terminate_cmp_cb(uint8_t conidx, uint16_t dummy, uint8_t chan_lid, uint16_t status)
{
    return;
}

__STATIC const l2cap_chan_coc_cb_t gfps_ble_l2cap_chan_cb =
{
        .cb_sdu_rx              = gfps_ble_l2cap_sdu_rx_cb,
        .cb_sdu_sent            = gfps_ble_l2cap_sdu_sent_cb,
        .cb_coc_create_cmp      = NULL,
        .cb_coc_created         = gfps_ble_l2cap_created_cb,
        .cb_coc_reconfigure_cmp = gfps_ble_l2cap_reconfigure_cmp_cb,
        .cb_coc_mtu_changed     = gfps_ble_l2cap_mtu_changed_cb,
        .cb_coc_terminated      = gfps_ble_l2cap_terminated_cb,
        .cb_coc_terminate_cmp   = gfps_ble_l2cap_terminate_cmp_cb,
};

__STATIC bool gfps_ble_l2cap_connect_req_cb(uint8_t conidx, uint16_t token, uint8_t nb_chan, uint16_t spsm,
                                            uint16_t peer_rx_mtu)
{
    uint16_t local_rx_mtu    = gatt_user_pref_mtu_get();

    // Accept connection
    // TODO: implemnt the bearer coc rx credit configuration to replace the fixed value 6
    l2cap_coc_connect_cfm(conidx, token, 1, local_rx_mtu, 6, &gfps_ble_l2cap_chan_cb);
    return true;
}

__STATIC const l2cap_coc_spsm_cb_t gfps_ble_l2cap_cb =
{
        .cb_coc_connect_req = gfps_ble_l2cap_connect_req_cb,
};

void gfps_ble_l2cap_disconnect(uint8_t conidx)
{
    gfpsp_l2cap_env_t * env = gfps_ble_l2cap_get_env(conidx);
    if (env)
    {
        l2cap_coc_terminate(conidx, 0, env->chan_id);
    }
}
#endif

__STATIC void gfpsp_gatt_cb_event_sent(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    // Get the address of the environment
    //struct gfpsp_env_tag *gfpsp_env = PRF_ENV_GET(GFPSP, gfpsp);

   // struct ble_gfpsp_tx_sent_ind_t *ind= 

}

__STATIC void gfpsp_gatt_cb_att_read_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl, uint16_t offset,
                             uint16_t max_length)
{
    co_buf_t* p_data = NULL;
    uint16_t dataLen = 0;
    uint16_t status = GAP_ERR_NO_ERROR;
    
    // Get the address of the environment
    PRF_ENV_T(gfpsp)* gfpsp_env = PRF_ENV_GET(GFPSP, gfpsp);
    TRACE(3, "read hdl %d, start_hdl %d,  notification_enable is %d", hdl, gfpsp_env->start_hdl, gfpsp_env->isNotificationEnabled[conidx]);

    if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_MODEL_ID_VAL)) {
#ifndef IS_USE_CUSTOM_FP_INFO
        uint32_t model_id = 0x2B677D;
#else
        uint32_t model_id = app_gfps_get_model_id();
#endif
        dataLen = sizeof(model_id);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&model_id, dataLen);
    }
    else if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_KEY_BASED_PAIRING_NTF_CFG)) {
        uint16_t notify_ccc = gfpsp_env->isNotificationEnabled[conidx];
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
    }
#ifdef SPOT_ENABLED
    else if(hdl == (gfpsp_env->start_hdl + GFPSP_IDX_BEACON_ACTIONS_CFG)) {
        uint16_t notify_ccc = gfpsp_env->isNotificationEnabled[conidx];
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
    }
    else if(hdl == (gfpsp_env->start_hdl + GFPSP_IDX_BEACON_ACTIONS_CFG)) {
        uint16_t notify_ccc = gfpsp_env->isNotificationEnabled[conidx];
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
    }
    else if(hdl == (gfpsp_env->start_hdl + GFPSP_IDX_BEACON_ACTIONS_VAL)) {
        uint8_t nonce[9];
        app_gfps_set_protocol_version();
        nonce[0]= app_gfps_get_protocol_version();    //protocol major version number;
        app_gfps_generate_nonce();
        memcpy(&nonce[1], app_gfps_get_nonce(),8);;
    
        dataLen = sizeof(nonce);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), nonce, dataLen);
    }
#endif
    else if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_MSG_STREAM_VAL))
    {
        uint8_t data[3] = {0};
        data[0] = STATE_READY_CONNECT;
        data[1] = (L2CAP_SPSM_GFPS >> 8);
        data[2] = (L2CAP_SPSM_GFPS & 0xFF);
        dataLen = 3;
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), data, dataLen);
    }
    else {
        dataLen = 0;
        status = ATT_ERR_REQUEST_NOT_SUPPORTED;
    }

    gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, dataLen, p_data);

    // Release the buffer
    co_buf_release(p_data);
}

__STATIC void gfpsp_gatt_cb_att_event_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t dummy, uint16_t hdl,
                        uint16_t max_length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

__STATIC void gfpsp_gatt_cb_att_info_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl)    //similar to gattc_att_info_req_ind_handler
{
    uint16_t length = 0;
    uint16_t status = GAP_ERR_NO_ERROR;
   
    TRACE(1, "%s status %d hdl %d", __func__, status, hdl);
    
    PRF_ENV_T(gfpsp)* gfpsp_env = PRF_ENV_GET(GFPSP, gfpsp);
   
    // check if it's a client configuration char
    if (hdl == gfpsp_env->start_hdl + GFPSP_KEY_BASED_PAIRING_NTF_CFG)
    {
        // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == gfpsp_env->start_hdl + GFPSP_KEY_PASS_KEY_NTF_CFG)
    {
         // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == gfpsp_env->start_hdl + GFPSP_IDX_ACCOUNT_KEY_CFG)
    {
         // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == gfpsp_env->start_hdl + GFPSP_IDX_NAME_CFG)
    {
         // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == gfpsp_env->start_hdl + GFPSP_IDX_KEY_BASED_PAIRING_VAL)
    {
       // force length to zero to reject any write starting from something != 0
        length = 0;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == gfpsp_env->start_hdl + GFPSP_IDX_PASSKEY_VAL)
    {
          // force length to zero to reject any write starting from something != 0
        length = 0;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == gfpsp_env->start_hdl + GFPSP_IDX_ACCOUNT_KEY_VAL)
    {
          // force length to zero to reject any write starting from something != 0
        length = 0;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == gfpsp_env->start_hdl + GFPSP_IDX_NAME_VAL)
    {
          // force length to zero to reject any write starting from something != 0
        length = 0;
        status = GAP_ERR_NO_ERROR;
    }
    // not expected request
    else
    {
        length = 0;
        status = ATT_ERR_WRITE_NOT_PERMITTED;
    }
   
    // Send the confirmation
    gatt_srv_att_info_get_cfm(conidx, user_lid, token, status, length);
}

__STATIC void gfpsp_gatt_cb_att_set(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                       uint16_t offset, co_buf_t* p_buf)
{
    // Get the address of the environment
     PRF_ENV_T(gfpsp)* gfpsp_env = PRF_ENV_GET(GFPSP, gfpsp);
   
    uint16_t status = GAP_ERR_NO_ERROR;
   
    TRACE(4, "%s gfpsp_env 0x%x write handle %d shdl %d,offset is %d ", __func__, (uint32_t)gfpsp_env, hdl, gfpsp_env->start_hdl,offset);
   
    uint8_t* pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;
   
    DUMP8("%02x ", pData, dataLen);
   
    if (gfpsp_env != NULL)
    {
       
        if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_KEY_BASED_PAIRING_VAL))
        {
            TRACE(0, "GFPSP_IDX_KEY_BASED_PAIRING_VAL,datalen is %d,conidx is %d,user_lid is %d,token is %d",
                                                       p_buf->data_len,conidx,user_lid,token);

            //inform APP of data
            struct gfpsp_write_ind_t * ind = KE_MSG_ALLOC_DYN(GFPSP_KEY_BASED_PAIRING_WRITE_IND,
                                                       KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                                                       KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                                                       gfpsp_write_ind_t,
                                                       p_buf->data_len);
   
            ind->length = p_buf->data_len;
            ind->pendingWriteRsp.conidx = conidx;
            ind->pendingWriteRsp.token = token;
            ind->pendingWriteRsp.user = user_lid;
            ind->pendingWriteRsp.status = status;
            memcpy((uint8_t *)(ind->data), pData, dataLen);

            TRACE(3,"length is %d, condix is %d,token is %d,user_lid is %d, status is %d",ind->length,ind->pendingWriteRsp.conidx,
                                ind->pendingWriteRsp.token,ind->pendingWriteRsp.user,ind->pendingWriteRsp.status);
            DUMP8("%02x ",ind->data,ind->length);
            DUMP8("%02x ",ind, ind->length);

   
            ke_msg_send(ind);
              
        }
     
        else if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_KEY_BASED_PAIRING_NTF_CFG))
        {
            TRACE(0, "GFPSP_IDX_KEY_BASED_PAIRING_NTF_CFG dataLen %d", dataLen);
            uint16_t value = 0x0000;
   
            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));
            TRACE(0,"value is %d",value);
   
            if ((value == PRF_CLI_STOP_NTFIND)||(value == PRF_CLI_START_NTF))
            {
                gfpsp_env->isNotificationEnabled[conidx] = value;
            }
            else
            {
                status = PRF_APP_ERROR;
            }
            // Inform GATT about handling
            gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
   
            if(status == GAP_ERR_NO_ERROR)
            {
                struct app_gfps_key_based_notif_config_t * ind = KE_MSG_ALLOC(GFPSP_KEY_BASED_PAIRING_NTF_CFG,
                        KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                        KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                        app_gfps_key_based_notif_config_t);
                if(PRF_CLI_STOP_NTFIND == gfpsp_env->isNotificationEnabled[conidx])
                {
                    ind->isNotificationEnabled  = false;
                }
                else if(PRF_CLI_START_NTF == gfpsp_env->isNotificationEnabled[conidx])
                {
                    ind->isNotificationEnabled  = true;
                }
   
                 ke_msg_send(ind);
            }
   
        }
        else if(hdl == (gfpsp_env->start_hdl + GFPSP_IDX_PASSKEY_NTF_CFG))
        {
            TRACE(0, "GFPSP_IDX_PASSKEY_NTF_CFG  dataLen %d", dataLen);
            uint16_t value = 0x0000;
   
            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));
   
            if ((value == PRF_CLI_STOP_NTFIND)||(value == PRF_CLI_START_NTF))
            {
                gfpsp_env->isNotificationEnabled[conidx] = value;
            }
            else
            {
                status = PRF_APP_ERROR;
            }
            // Inform GATT about handling
            gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
   
            if(status == GAP_ERR_NO_ERROR)
            {
                struct app_gfps_key_based_notif_config_t * ind = KE_MSG_ALLOC(GFPSP_KEY_PASS_KEY_NTF_CFG,
                        KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                        KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                        app_gfps_key_based_notif_config_t);
                if(PRF_CLI_STOP_NTFIND == gfpsp_env->isNotificationEnabled[conidx])
                {
                    ind->isNotificationEnabled  = false;
                }
                else if(PRF_CLI_START_NTF == gfpsp_env->isNotificationEnabled[conidx])
                {
                    ind->isNotificationEnabled  = true;
                }
   
                 ke_msg_send(ind);
            }
        }
        else if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_PASSKEY_VAL))
        {
            TRACE(0, "GFPSP_IDX_PASSKEY_VAL,%d", p_buf->data_len);
   
            struct gfpsp_write_ind_t * ind = KE_MSG_ALLOC_DYN(GFPSP_KEY_PASS_KEY_WRITE_IND,
                        KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                        KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                        gfpsp_write_ind_t,
                        p_buf->data_len);
   
            ind->length = p_buf->data_len;
            memcpy((uint8_t *)(ind->data), pData, dataLen);
   
             ke_msg_send(ind);
             gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
              
        }
        else if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_ACCOUNT_KEY_VAL))
        {
             TRACE(0,"Get account key:");
             DUMP8("%02x ", pData, p_buf->data_len);
            struct gfpsp_write_ind_t * ind = KE_MSG_ALLOC_DYN(GFPSP_KEY_ACCOUNT_KEY_WRITE_IND,
                        KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                        KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                        gfpsp_write_ind_t,
                        p_buf->data_len);
   
            ind->length = p_buf->data_len;
            memcpy((uint8_t *)(ind->data), pData, dataLen);
   
             ke_msg_send(ind);
             gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
              
        }
        else if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_NAME_VAL))
        {
             TRACE(1,"Get conidx %d updated name:",conidx);
             DUMP8("%02x ", pData, p_buf->data_len);
             struct gfpsp_write_ind_t * ind = KE_MSG_ALLOC_DYN(GFPSP_NAME_WRITE_IND,
                        KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                        KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                        gfpsp_write_ind_t,
                        p_buf->data_len);
   
            ind->length = p_buf->data_len;
            ind->pendingWriteRsp.conidx = conidx;
            memcpy((uint8_t *)(ind->data), pData, dataLen);
   
             ke_msg_send(ind);
             gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
              
        }
#ifdef SPOT_ENABLED
        else if(hdl == (gfpsp_env->start_hdl + GFPSP_IDX_BEACON_ACTIONS_VAL))
        {
            TRACE(4, "GFPSP_IDX_BEACON_ACTIONS_VAL,datalen is %d,conidx is %d,user_lid is %d,token is %d",
                                                       p_buf->data_len,conidx,user_lid,token);

            //inform APP of data
            struct gfpsp_write_ind_t * ind = KE_MSG_ALLOC_DYN(GFPSP_BEACON_ACTIONS_WRITE_IND,
                                                       KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                                                       KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                                                       gfpsp_write_ind_t,
                                                       p_buf->data_len);
   
            ind->length = p_buf->data_len;
            ind->pendingWriteRsp.conidx = conidx;
            ind->pendingWriteRsp.token = token;
            ind->pendingWriteRsp.user = user_lid;
            ind->pendingWriteRsp.status = status;
            memcpy((uint8_t *)(ind->data), pData, dataLen);

            TRACE(3,"length is %d, condix is %d,token is %d,user_lid is %d, status is %d",ind->length,ind->pendingWriteRsp.conidx,
                                ind->pendingWriteRsp.token,ind->pendingWriteRsp.user,ind->pendingWriteRsp.status);
            DUMP8("%02x ",ind->data,ind->length);
            DUMP8("%02x ",ind, ind->length);

            ke_msg_send(ind);
        }
        else if (hdl == (gfpsp_env->start_hdl + GFPSP_IDX_BEACON_ACTIONS_CFG))
        {
            TRACE(4, "GFPSP_IDX_BEACON_ACTIONS_CFG,datalen is %d,conidx is %d,user_lid is %d,token is %d",
                                                       p_buf->data_len,conidx,user_lid,token);
            uint16_t value = 0x0000;
            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));
            if((value == PRF_CLI_STOP_NTFIND) || (value == PRF_CLI_START_NTF))
            {
                gfpsp_env->isNotificationEnabled[conidx] = value;
            }
            else
            {
                status = PRF_APP_ERROR;
            }

            // Inform GATT about handling
            gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);

            if(status == GAP_ERR_NO_ERROR)
            {
                struct app_gfps_key_based_notif_config_t * ind = KE_MSG_ALLOC(GFPSP_BEACON_ACTIONS_NTF_CFG,
                        KE_BUILD_ID(PRF_DST_TASK(GFPSP), conidx),
                        KE_BUILD_ID(PRF_SRC_TASK(GFPSP), conidx),
                        app_gfps_key_based_notif_config_t);
                if(PRF_CLI_STOP_NTFIND == gfpsp_env->isNotificationEnabled[conidx])
                {
                    ind->isNotificationEnabled  = false;
                }
                else if(PRF_CLI_START_NTF == gfpsp_env->isNotificationEnabled[conidx])
                {
                    ind->isNotificationEnabled  = true;
                }
   
                 ke_msg_send(ind);
            }
            
        }
#endif
        else
        {
            // Inform GATT about handling
            gatt_srv_att_val_set_cfm(conidx, user_lid, token, GAP_ERR_NO_ERROR);
        }
    }

}


/// Set of callbacks functions for communication with GATT as a GATT User Server
__STATIC const gatt_srv_cb_t gfpsp_gatt_srv_cb = {
    .cb_event_sent      = gfpsp_gatt_cb_event_sent,
    .cb_att_read_get    = gfpsp_gatt_cb_att_read_get,
    .cb_att_event_get   = gfpsp_gatt_cb_att_event_get,
    .cb_att_info_get    = gfpsp_gatt_cb_att_info_get,
    .cb_att_val_set     = gfpsp_gatt_cb_att_set,
};




/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the GFPSP module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    env        Collector or Service allocated environment data.
 * @param[in|out] start_hdl  Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task   Application task number.
 * @param[in]     sec_lvl    Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     param      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint16_t gfpsp_init (prf_data_t* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct gfpsp_db_cfg* params)
{
    //------------------ create the attribute database for the profile -------------------

    // DB Creation Statis
    uint16_t status = GAP_ERR_NO_ERROR;

    nv_record_fp_account_key_init();

    PRF_ENV_T(gfpsp)* gfpsp_env =
            (PRF_ENV_T(gfpsp)* ) ke_malloc(sizeof(PRF_ENV_T(gfpsp)), KE_MEM_PROFILE);

    memset((uint8_t *)gfpsp_env,0,sizeof(PRF_ENV_T(gfpsp)));
    env->p_env = (prf_hdr_t *) gfpsp_env;

  
    // register as GATT User 
    status = gatt_user_srv_register(PREFERRED_BLE_MTU, 0, &gfpsp_gatt_srv_cb, &(gfpsp_env->user_lid));
    
    if (status == GAP_ERR_NO_ERROR) 
    {
        gfpsp_env->start_hdl = *start_hdl;
        
#ifdef GFPS_USE_128BIT_UUID
        status = gatt_db_svc_add(gfpsp_env->user_lid, SVC_SEC_LVL(NOT_ENC), 
                   ATT_SVC_GOOGLE_FAST_PAIR_PROVIDER,
                   GFPSP_IDX_NB, NULL, gfpsp_att_db, GFPSP_IDX_NB,
                   &gfpsp_env->start_hdl);
#else 
       status = gatt_db_svc16_add(gfpsp_env->user_lid, SVC_SEC_LVL(NOT_ENC), 
                ATT_SVC_GOOGLE_FAST_PAIR_PROVIDER,
                GFPSP_IDX_NB, NULL, gfpsp_att_db, GFPSP_IDX_NB,
                &gfpsp_env->start_hdl);
#endif

#if BLE_AUDIO_ENABLED
         uint16_t fp_sec_lvl = 0;
         SETF(fp_sec_lvl, L2CAP_COC_AUTH, GAP_SEC_UNAUTH); // Unauthenticated pairing required
         status = l2cap_coc_spsm_add(L2CAP_SPSM_GFPS, fp_sec_lvl, &gfps_ble_l2cap_cb);      
         memset((uint8_t *)&(gfpsBleEnv.l2capEnv), 0xFF, sizeof(gfpsp_l2cap_env_t) * BT_DEVICE_NUM);
#endif

         TRACE(1, "%s status %d,UUID is %d,nb_att %d shdl %d", __func__, status, gfpsp_env->user_lid,GFPSP_IDX_NB, gfpsp_env->start_hdl);

        if(GAP_ERR_NO_ERROR == status)
        {
            env->p_env = (prf_hdr_t *)gfpsp_env;
            gfpsp_task_init(&(env->desc));

            /* in Idle state */
            // ke_state_set(KE_BUILD_ID(env->prf_task, 0), GFPSP_IDLE);
        }
    }

    return (status);

   
}
/**
 ****************************************************************************************
 * @brief Destruction of the GFPSP module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static uint16_t gfpsp_destroy(prf_data_t* env)
{
    uint16_t status = GAP_ERR_NO_ERROR;
    PRF_ENV_T(gfpsp)* gfpsp_env = (PRF_ENV_T(gfpsp)*) env->p_env;

    if(status == GAP_ERR_NO_ERROR)
    {
        // free profile environment variables
        env->p_env = NULL;
        ke_free(gfpsp_env);
    }

    return status;

}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void gfpsp_create(prf_data_t* env, uint8_t conidx,bool is_le_con)
{
    /* Nothing to do */
    // gfpsp_env_tag *gfpsp_env = (gfpsp_env_tag *) env->p_env;    
    //ASSERT_ERR(conidx < BLE_CONNECTION_MAX);    
    // force notification config to zero when peer device is disconnected    
    // gfpsp_env->ntfIndEnableFlag[conidx] = 0;
    
    TRACE(0, "%s conidx 0x%x", __func__, conidx);

}

/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void gfpsp_cleanup(prf_data_t* env, uint8_t conidx, uint8_t reason)
{
    /* Nothing to do */
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

static void gfpsp_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param)
{
    TRACE(0, "%s", __func__);
}


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// GFPSP Task interface required by profile manager
const struct prf_task_cbs gfpsp_itf =
{
        (prf_init_cb)gfpsp_init,
        (prf_destroy_cb)gfpsp_destroy,
        (prf_con_create_cb)gfpsp_create,
        (prf_con_cleanup_cb)gfpsp_cleanup,
        (prf_con_upd_cb)gfpsp_upd,
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* gfpsp_prf_itf_get(void)
{
   return &gfpsp_itf;
}

uint32_t gfpsp_compute_cfg_flag(uint16_t features)
{
    //Service Declaration
    uint32_t cfg_flag = 1;

    for (uint8_t i = 0; i<GFPSP_CHAR_MAX; i++)
    {
        if (((features >> i) & 1) == 1)
        {
            cfg_flag |= (3 << (i*2 + 1));
        }
    }

    return cfg_flag;
}


uint8_t gfpsp_handle_to_value(PRF_ENV_T(gfpsp)* env, uint16_t handle)
{
    uint8_t value = GFPSP_CHAR_MAX;

    // handle cursor, start from first characteristic of service handle
    uint16_t cur_hdl = env->start_hdl + 1;

    for (uint8_t i = 0; i<GFPSP_CHAR_MAX; i++)
    {
        if (((env->features >> i) & 1) == 1)
        {
            // check if value handle correspond to requested handle
            if((cur_hdl +1) == handle)
            {
                value = i;
                break;
            }
            cur_hdl += 2;
        }
    }

    return value;
}

uint16_t gfpsp_value_to_handle(PRF_ENV_T(gfpsp)* env, uint8_t value)
{
    uint16_t handle = env->start_hdl + 1;
    int8_t i;

    for (i = 0; i<GFPSP_CHAR_MAX; i++)
    {
        if (((env->features >> i) & 1) == 1)
        {
            // requested value
            if(value == i)
            {
                handle += 1;
                break;
            }
            handle += 2;
        }
    }

    // check if handle found
    return ((i == GFPSP_CHAR_MAX) ? GATT_INVALID_HDL : handle);
}

uint8_t gfpsp_check_val_len(uint8_t char_code, uint8_t val_len)
{
    uint8_t status = GAP_ERR_NO_ERROR;

    // Check if length is upper than the general maximal length
    if (val_len > GFPSP_VAL_MAX_LEN)
    {
        status = PRF_ERR_UNEXPECTED_LEN;
    }
    else
    {
        // Check if length matches particular requirements
        switch (char_code)
        {
            case GFPSP_SYSTEM_ID_CHAR:
                if (val_len != GFPSP_SYS_ID_LEN)
                {
                    status = PRF_ERR_UNEXPECTED_LEN;
                }
                break;
            case GFPSP_IEEE_CHAR:
                if (val_len < GFPSP_IEEE_CERTIF_MIN_LEN)
                {
                    status = PRF_ERR_UNEXPECTED_LEN;
                }
                break;
            case GFPSP_PNP_ID_CHAR:
                if (val_len != GFPSP_PNP_ID_LEN)
                {
                    status = PRF_ERR_UNEXPECTED_LEN;
                }
                break;
            default:
                break;
        }
    }

    return (status);
}

uint32_t app_sec_get_P256_key(uint8_t * out_public_key,uint8_t * out_private_key)
{
    return gfps_crypto_make_P256_key(out_public_key,out_private_key);
}

#endif //BLE_GFPSP_SERVER

/// @} GFPSP
