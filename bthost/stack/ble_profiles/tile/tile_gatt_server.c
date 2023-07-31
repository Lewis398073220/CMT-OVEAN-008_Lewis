#include "tile_gatt_server.h"


#if (BLE_TILE)

#include "ke_mem.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"



#define TILE_SHIPPING_UUID    0xFEEC               /** Advertized by Tiles in Shipping Mode. */
#define TILE_ACTIVATED_UUID   0xFEED               /** Advertized by Tiles in Activated Mode. */
#define TILE_SERVICE_UUID     TILE_ACTIVATED_UUID              /** Used to declare Tile Gatt Service. */
static const uint8_t ATT_SVC_TILE_UUID_SERVICE[GATT_UUID_16_LEN] = {0xED,0xFE};


#define TILE_SVC_BASE_UUID    { 0xC0, 0x91, 0xC4, 0x8D, 0xBD, 0xE7, 0x60, 0xBA, 0xDD, 0xF4, 0xD6, 0x35, 0x00, 0x00, 0x41, 0x9D }
#define TILE_TOA_RSP_UUID     { 0xC0, 0x91, 0xC4, 0x8D, 0xBD, 0xE7, 0x60, 0xBA, 0xDD, 0xF4, 0xD6, 0x35, 0x19, 0x00, 0x41, 0x9D } // tx
#define TILE_TOA_CMD_UUID     { 0xC0, 0x91, 0xC4, 0x8D, 0xBD, 0xE7, 0x60, 0xBA, 0xDD, 0xF4, 0xD6, 0x35, 0x18, 0x00, 0x41, 0x9D } //rx
#define TILE_TILEID_CHAR_UUID { 0xC0, 0x91, 0xC4, 0x8D, 0xBD, 0xE7, 0x60, 0xBA, 0xDD, 0xF4, 0xD6, 0x35, 0x07, 0x00, 0x41, 0x9D }

#define TILE_MAX_LEN  255

#define GATT_DECL_PRIMARY_SERVICE_UUID       { 0x00, 0x28 }
#define GATT_DECL_CHARACTERISTIC_UUID        { 0x03, 0x28 }
#define GATT_DESC_CLIENT_CHAR_CFG_UUID       { 0x02, 0x29 }


/*******************************************************************************
 *******************************************************************************
 * Full tile service Database Description
 *  - Used to add attributes into the database
 *
 *
 *******************************************************************************
 *******************************************************************************/

const struct gatt_att_desc tile_att_db[TILE_IDX_NUM] = {
    /**************************************************/
    // Service Declaration
    [TILE_IDX_SVC]             =    {GATT_DECL_PRIMARY_SERVICE_UUID, PROP(RD), 0},

    [TILE_IDX_ID_CHAR]         =    {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [TILE_IDX_ID_VAL]          =    {TILE_TILEID_CHAR_UUID, PROP(RD)| ATT_UUID(128), TILE_ID_LEN},

    [TILE_IDX_TOA_CMD_CHAR]    =    {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [TILE_IDX_TOA_CMD_VAL]     =    {TILE_TOA_CMD_UUID, PROP(RD)| PROP(WC)| ATT_UUID(128), TILE_MAX_LEN},
    
    [TILE_IDX_TOA_RSP_CHAR]    =    {GATT_DECL_CHARACTERISTIC_UUID, PROP(RD), 0},
    [TILE_IDX_TOA_RSP_VAL]     =    {TILE_TOA_RSP_UUID,PROP(RD)| PROP(N)| ATT_UUID(128), TILE_MAX_LEN},
    [TILE_IDX_TOA_RSP_NTF_CFG] =    {GATT_DESC_CLIENT_CHAR_CFG_UUID,PROP(RD)|PROP(WR), 0},
};

__STATIC void tile_gatt_cb_event_sent(uint8_t conidx, uint8_t user_lid, uint16_t dummy, uint16_t status)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    struct tile_gatt_tx_complete_ind_t *ind = KE_MSG_ALLOC(TILE_TOA_TX_DATA_SENT_DONE_IND,
                                                           PRF_DST_TASK(TILE),PRF_SRC_TASK(TILE),
                                                           tile_gatt_tx_complete_ind_t);

      ind->conidx  = conidx;
      ind->success = GAP_ERR_NO_ERROR;
      ke_msg_send(ind);
}

extern void app_tile_get_device_id(uint8_t* pDevId);

__STATIC void tile_gatt_cb_att_read_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl, uint16_t offset,
                             uint16_t max_length)
{
    co_buf_t* p_data = NULL;
    uint16_t dataLen = 0;
    uint16_t status = GAP_ERR_NO_ERROR;

    TRACE(0, "%s conidx 0x%x", __func__, conidx);
    TRACE(1, "read hdl %d", hdl);

    // Get the address of the environment
     PRF_ENV_T(tile_gatt)* tile_env = PRF_ENV_GET(TILE, tile_gatt);

    if (hdl == (tile_env->start_hdl + TILE_IDX_TOA_RSP_NTF_CFG)) {
        uint16_t notify_ccc = tile_env->isNotificationEnabled[conidx];
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
    }
    else if(hdl == (tile_env->start_hdl + TILE_IDX_ID_VAL))
    {
        uint16_t notify_ccc = tile_env->isNotificationEnabled[conidx];
        dataLen = sizeof(notify_ccc);
        prf_buf_alloc(&p_data, dataLen);
        memcpy(co_buf_data(p_data), (uint8_t *)&notify_ccc, dataLen);
        app_tile_get_device_id((uint8_t *)p_data);      //need to check;
    }
    else{
        dataLen = 0;
        status = ATT_ERR_REQUEST_NOT_SUPPORTED;
    }

    gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, dataLen, p_data);

    // Release the buffer
    co_buf_release(p_data);
}

__STATIC void tile_gatt_cb_att_event_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t dummy, uint16_t hdl,
                     uint16_t max_length)
{
    TRACE(0, "%s conidx 0x%x", __func__, conidx);
}

__STATIC void tile_gatt_cb_att_info_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl)  
{
    uint16_t length = 0;
    uint16_t status = GAP_ERR_NO_ERROR;

    TRACE(1, "%s status %d hdl %d", __func__, status, hdl);

    PRF_ENV_T(tile_gatt)* tile_env = PRF_ENV_GET(TILE, tile_gatt);

    // check if it's a client configuration char
    if (hdl == tile_env->start_hdl + TILE_IDX_TOA_RSP_NTF_CFG)
    {
        // CCC attribute length = 2
        length = 2;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == tile_env->start_hdl + TILE_IDX_TOA_RSP_VAL)
    {
        length = 0;
        status = GAP_ERR_NO_ERROR;
    } else if (hdl == tile_env->start_hdl + TILE_IDX_TOA_CMD_VAL)
    {
        length = 0;
        status = GAP_ERR_NO_ERROR;
    } else
    {
        length = 0;
        status = ATT_ERR_WRITE_NOT_PERMITTED;
    }

    // Send the confirmation
    gatt_srv_att_info_get_cfm(conidx, user_lid, token, status, length);
}

__STATIC void tile_gatt_cb_att_set(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
                                       uint16_t offset, co_buf_t* p_buf)
{
    // Get the address of the environment
     PRF_ENV_T(tile_gatt)* tile_env = PRF_ENV_GET(TILE, tile_gatt);

    uint16_t status = GAP_ERR_NO_ERROR;

    TRACE(4, "%s tile_env 0x%x write handle %d shdl %d,offset is %d ", __func__, (uint32_t)tile_env, hdl, tile_env->start_hdl,offset);
   
    uint8_t* pData = co_buf_data(p_buf);
    uint16_t dataLen = p_buf->data_len;

    DUMP8("%02x ", pData, dataLen);
   
    if (tile_env != NULL)
    {
        if (hdl == (tile_env->start_hdl + TILE_IDX_TOA_RSP_NTF_CFG))
        {
            TRACE(0, "TILE_IDX_TOA_RSP_NTF_CFG dataLen %d", dataLen);
            uint16_t value = 0x0000;

            //Extract value before check
            memcpy(&value, pData, sizeof(uint16_t));
            TRACE(0,"value is %d",value);

            if ((value == PRF_CLI_STOP_NTFIND)||(value == PRF_CLI_START_NTF))
            {
                tile_env->isNotificationEnabled[conidx] = value;
            }
            else
            {
                status = PRF_APP_ERROR;
            }
            // Inform GATT about handling
            gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);

            if(status == GAP_ERR_NO_ERROR)
            {
                struct tile_gatt_connection_event_t* ind = KE_MSG_ALLOC(TILE_CHANNEL_CONNECTED_IND,
                                                                        PRF_DST_TASK(TILE),
                                                                        PRF_SRC_TASK(TILE),
                                                                        tile_gatt_connection_event_t);
                 ind->conidx = conidx;
                 ind->handle = hdl;
                 ke_msg_send(ind);
            }
            else
            {
                struct tile_gatt_connection_event_t* ind = KE_MSG_ALLOC(TILE_CHANNEL_DISCONNECTED_IND,
                                                                        PRF_DST_TASK(TILE),
                                                                        PRF_SRC_TASK(TILE),
                                                                        tile_gatt_connection_event_t);
                 ind->conidx = conidx;
                 ind->handle = hdl;
                 ke_msg_send(ind);
            }

        }
        else if (hdl == (tile_env->start_hdl + TILE_IDX_TOA_CMD_VAL))
        {
             TRACE(0,"TILE_IDX_TOA_CMD_VAL:");
             DUMP8("%02x ", pData, p_buf->data_len);
             //inform APP of data
             struct tile_gatt_rx_data_ind_t * ind = KE_MSG_ALLOC_DYN(TILE_TOA_RECEIVED_IND,
                                                                     PRF_DST_TASK(TILE),
                                                                     PRF_SRC_TASK(TILE),
                                                                     tile_gatt_rx_data_ind_t,
                                                                     p_buf->data_len);
             ind->conidx = conidx;
             ind->length =p_buf->data_len;
             memcpy((uint8_t *)(ind->data), pData, p_buf->data_len);
             ke_msg_send(ind);

             gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);

        }

        else
        {
            // Inform GATT about handling
            gatt_srv_att_val_set_cfm(conidx, user_lid, token, PRF_APP_ERROR);
        }
    }

}


/// Set of callbacks functions for communication with GATT as a GATT User Server
__STATIC const gatt_srv_cb_t tile_gatt_srv_cb = {
   .cb_event_sent      = tile_gatt_cb_event_sent,
   .cb_att_read_get    = tile_gatt_cb_att_read_get,
   .cb_att_event_get   = tile_gatt_cb_att_event_get,
   .cb_att_info_get    = tile_gatt_cb_att_info_get,
   .cb_att_val_set     = tile_gatt_cb_att_set,
};


static int tile_data_send_handler(ke_msg_id_t const msgid,
                                         struct ble_tile_sent_ntf_t *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
     PRF_ENV_T(tile_gatt)* tile_env = PRF_ENV_GET(TILE, tile_gatt);
     enum gatt_evt_type evtType = GATT_NOTIFY;

    if(tile_env->isNotificationEnabled[param->conidx])
    {
       co_buf_t* p_buf = NULL;
        prf_buf_alloc(&p_buf, param->data_len);

        uint8_t* p_data = co_buf_data(p_buf);
        memcpy(p_data, param->data, param->data_len);

        // Dummy parameter provided to GATT
        uint16_t dummy = 0;

        // Inform the GATT that notification must be sent
        uint16_t ret = gatt_srv_event_send(param->conidx, tile_env->srv_user_lid, dummy, evtType,
                            tile_env->start_hdl + TILE_IDX_TOA_RSP_VAL, p_buf);

        // Release the buffer
        co_buf_release(p_buf);

        return (GAP_ERR_NO_ERROR == ret);; 
     }
     else
     {
         return false;
     }
}


/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(tile_gatt) { /// AMA ancc_msg_handler_tab
    /// handlers for command from upper layer
    {TILE_TOA_TX_DATA_SENT_CMD,       (ke_msg_func_t)tile_data_send_handler},
};

static uint16_t tile_init(prf_data_t* env, uint16_t* start_hdl, uint8_t sec_lvl,
                              uint8_t user_prio, const void* params, const void* p_cb)
{
    uint16_t status = GAP_ERR_NO_ERROR;;
    TRACE(0, "tile_init returns %d start handle is %d sec_lvl 0x%x", status, *start_hdl, sec_lvl);

    // Allocate BUDS required environment variable
    PRF_ENV_T(tile_gatt)* tile_env =
            (PRF_ENV_T(tile_gatt)* ) ke_malloc(sizeof(PRF_ENV_T(tile_gatt)), KE_MEM_PROFILE);

    memset((uint8_t *)tile_env, 0, sizeof(PRF_ENV_T(tile_gatt)));
    // Initialize BUDS environment
    env->p_env = (prf_hdr_t*) tile_env;

        // Register as GATT User Client
    status = gatt_user_srv_register(PREFERRED_BLE_MTU, 0, &tile_gatt_srv_cb,
                                    &(tile_env->srv_user_lid));

     //-------------------- allocate memory required for the profile  ---------------------
    if (GAP_ERR_NO_ERROR == status)
    {
        tile_env->start_hdl    = *start_hdl;
        status = gatt_db_svc_add(tile_env->srv_user_lid, SVC_SEC_LVL(NOT_ENC), ATT_SVC_TILE_UUID_SERVICE,
                                   TILE_IDX_NUM, NULL, tile_att_db, TILE_IDX_NUM,
                                   &tile_env->start_hdl);

        TRACE(1, "%s status %d nb_att %d shdl %d", __func__, status, TILE_IDX_NUM, tile_env->start_hdl);

        if(status == GAP_ERR_NO_ERROR)
        {
            env->desc.msg_handler_tab = tile_gatt_msg_handler_tab;
            env->desc.msg_cnt         = ARRAY_LEN(tile_gatt_msg_handler_tab);
            env->desc.state           = &tile_env->state;
            env->desc.idx_max         = BLE_CONNECTION_MAX;
        }
    }

    return (status);
}

static uint16_t tile_destroy(prf_data_t* env)
{
    uint16_t status = GAP_ERR_NO_ERROR;
    PRF_ENV_T(tile_gatt)* tile_env = (PRF_ENV_T(tile_gatt)*) env->p_env;

    if(status == GAP_ERR_NO_ERROR)
    {
        // free profile environment variables
        env->p_env = NULL;
        ke_free(tile_env);
    }
    return status;
}


static void tile_create(prf_data_t* env, uint8_t conidx) {
  /*PRF_ENV_T(tile_gatt) *tile_env = (PRF_ENV_T(tile_gatt) *) env->env;

  struct prf_svc tile_svc = {
      tile_env->start_hdl,
      tile_env->start_hdl + ARRAY_SIZE(tile_att_db)
  };
*/
    TRACE(3,"Tile gatt create: conidx=%d ", conidx);
}

static void tile_cleanup(prf_data_t* env, uint8_t conidx, uint8_t reason) {
  /* Nothing to do */
  TRACE(4,"%s env %p, conidx %d reason %d", __func__, env, conidx, reason);
  /* Nothing to do */
}

static void tile_upd(prf_data_t* p_env, uint8_t conidx, const gap_le_con_param_t* p_con_param)
{
    TRACE(0, "%s", __func__);
}


const struct prf_task_cbs tile_itf = {
    (prf_init_cb) tile_init,
    (prf_destroy_cb)tile_destroy,
    (prf_con_create_cb)tile_create,
    (prf_con_cleanup_cb)tile_cleanup,
    (prf_con_upd_cb)tile_upd,
};

/*
static int gattc_cmp_evt_handler(ke_msg_id_t const msgid, struct gattc_cmp_evt const *param, ke_task_id_t const dest_id, ke_task_id_t const src_id) {
  PRF_ENV_T(tile_gatt) *tile_env = PRF_ENV_GET(TILE, tile_gatt);
  uint8_t conidx = KE_IDX_GET(dest_id);

  if (param->operation == GATTC_NOTIFY) {
    if (param->status != 0) {
     // LOG_DEBUG(1,"GSound GATT Notification Failed: status=%d", param->status);
    }
    struct tile_gatt_tx_complete_ind_t *ind = KE_MSG_ALLOC(TILE_TOA_TX_DATA_SENT,
      prf_dst_task_get(&tile_env->prf_env, conidx),
      prf_src_task_get(&tile_env->prf_env, conidx),
      tile_gatt_tx_complete_ind_t);
    ind->success = (param->status == GAP_ERR_NO_ERROR);
    ke_msg_send(ind);
  }

  return KE_MSG_CONSUMED;
}
*/
const struct prf_task_cbs* tile_prf_itf_get(void)
{
   return &tile_itf;
}

#endif /* (BLE_TILE) */
