
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_SYMBOL_H__
#define  __BT_CONTROLLER_SYMBOL_H__


#define HCI_FC_ENV_ADDR                                               0XC000729C
#define LD_ACL_ENV_ADDR                                               0XC0005154
#define BT_UTIL_BUF_ENV_ADDR                                          0XC0004608
#define BLE_UTIL_BUF_ENV_ADDR                                         0XC000538C
#define LD_BES_BT_ENV_ADDR                                            0XC00052F4
#define DBG_STATE_ADDR                                                0XC0007390
#define LC_STATE_ADDR                                                 0XC0004CBC
#define LD_SCO_ENV_ADDR                                               0XC0005170
#define RX_MONITOR_ADDR                                               0XC0007BBC
#define LC_ENV_ADDR                                                   0XC0004CA8
#define LM_NB_SYNC_ACTIVE_ADDR                                        0XC0004CA2
#define LM_ENV_ADDR                                                   0XC0004950
#define LM_KEY_ENV_ADDR                                               0XC0004AC0
#define HCI_ENV_ADDR                                                  0XC00071BC
#define LC_SCO_ENV_ADDR                                               0XC0004C84
#define LLM_ENV_ADDR                                                  0XC00058B0
#define LD_ENV_ADDR                                                   0XC0004DC8
#define RWIP_ENV_ADDR                                                 0XC0007D60
#define BLE_RX_MONITOR_ADDR                                           0XC0007B64
#define LLC_ENV_ADDR                                                  0XC0006650
#define RWIP_RF_ADDR                                                  0XC0007CC8
#define LD_ACL_METRICS_ADDR                                           0XC0004E0C
#define RF_RX_HWGAIN_TBL_ADDR                                         0XC00043A6
#define RF_RX_GAIN_FIXED_TBL_ADDR                                     0XC0007B3E
#define HCI_DBG_EBQ_TEST_MODE_ADDR                                    0XC0007484
#define DBG_BT_COMMON_SETTING_ADDR                                    0XC00073EC
#define DBG_BT_SCHE_SETTING_ADDR                                      0XC00074B4
#define DBG_BT_IBRT_SETTING_ADDR                                      0XC0007316
#define DBG_BT_HW_FEAT_SETTING_ADDR                                   0XC00073C0
#define HCI_DBG_SET_SW_RSSI_ADDR                                      0XC00074DC
#define LP_CLK_ADDR                                                   0XC0007D5C
#define RWIP_PROG_DELAY_ADDR                                          0XC0007D54
#define DATA_BACKUP_CNT_ADDR                                          0XC0004080
#define DATA_BACKUP_ADDR_PTR_ADDR                                     0XC0004084
#define DATA_BACKUP_VAL_PTR_ADDR                                      0XC0004088
#define SCH_MULTI_IBRT_ADJUST_ENV_ADDR                                0XC0006FF0
#define RF_RX_GAIN_THS_TBL_LE_ADDR                                    0XC000436A
#define RF_RX_GAIN_THS_TBL_LE_2M_ADDR                                 0XC0004388
#define RF_RPL_TX_PW_CONV_TBL_ADDR                                    0XC00042D4
#define REPLACE_MOBILE_ADDR_ADDR                                      0XC0004D1C
#define REPLACE_ADDR_VALID_ADDR                                       0XC0004586
#define PCM_NEED_START_FLAG_ADDR                                      0XC00045C4
#define RT_SLEEP_FLAG_CLEAR_ADDR                                      0XC00045F8
#define RF_RX_GAIN_THS_TBL_BT_3M_ADDR                                 0XC0007B20
#define NORMAL_IQTAB_ADDR                                             0XC0007B8C
#define NORMAL_IQTAB_EN_ADDR                                          0XC00045DC
#define POWER_ADJUST_EN_ADDR                                          0XC00045E0
#define LD_IBRT_ENV_ADDR                                              0XC0005308
#define LLM_LOCAL_LE_FEATS_ADDR                                       0XC0004290
#define ISOOHCI_ENV_ADDR                                              0XC0007034
#define RF_RX_GAIN_THS_TBL_BT_ADDR                                    0XC0004310
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC000743A
#define LLD_CON_ENV_ADDR                                              0XC0006A94
#define POWER_ADJUST_EN_ADDR                                          0XC00045E0
#define LLD_ISO_ENV_ADDR                                              0XC0006AD4
#define ECC_RX_MONITOR_ADDR                                           0XC0007B18
#define __STACKLIMIT_ADDR                                             0XC0008948
#define PER_MONITOR_PARAMS_ADDR                                       0XC0004282
#define TX_POWER_VAL_BKUP_ADDR                                        0XC0007C78
#define DBG_ENV_ADDR                                                  0XC000754C
#define RF_RX_GAIN_THS_TBL_ECC_ADDR                                   0XC000434C
#define MASTER_CON_SUPPORT_LE_AUDIO_ADDR                              0XC0004584
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC000743A
#define RX_RECORD_ADDR                                                0XC00051B8
#define SCH_PROG_DBG_ENV_ADDR                                         0XC0006D14
#define LLD_PER_ADV_ENV_ADDR                                          0XC00069FC
#define HCI_DBG_BLE_ADDR                                              0XC000737C
#define LLD_ENV_ADDR                                                  0XC00068E8

//commit 2947d47ee3c2fa2ff3ab2332aa35e541dca6fd95
//Author: Leo <junhongliu@bestechnic.com>
//Date:   Sun Apr 23 17:43:31 2023 +0800
//    optimize tws bandwidth in page
//    
//    Change-Id: I39333e1d6c14e94a802822539f348c08a11a2e2e

#ifndef  BT_CONTROLLER_COMMIT_ID
#define  BT_CONTROLLER_COMMIT_ID                            "commit 2947d47ee3c2fa2ff3ab2332aa35e541dca6fd95"
#endif
#ifndef  BT_CONTROLLER_COMMIT_AUTHOR
#define  BT_CONTROLLER_COMMIT_AUTHOR                        "Author: Leo <junhongliu@bestechnic.com>"
#endif
#ifndef  BT_CONTROLLER_COMMIT_DATE
#define  BT_CONTROLLER_COMMIT_DATE                          "Date:   Sun Apr 23 17:43:31 2023 +0800"
#endif
#ifndef  BT_CONTROLLER_COMMIT_MESSAGE
#define  BT_CONTROLLER_COMMIT_MESSAGE                       "optimize tws bandwidth in page  Change-Id: I39333e1d6c14e94a802822539f348c08a11a2e2e "
#endif
#ifndef  BT_CONTROLLER_BUILD_TIME
#define  BT_CONTROLLER_BUILD_TIME                           "2023-04-23 19:41:51"
#endif
#ifndef  BT_CONTROLLER_BUILD_OWNER
#define  BT_CONTROLLER_BUILD_OWNER                          "jiansong@bestechnic.com"
#endif

#endif
