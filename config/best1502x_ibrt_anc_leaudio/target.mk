
#### LE-Audio related feature ####
export BLE_AUDIO_ENABLED := 1
export SUPPORT_REMOTE_COD := 1
ifeq ($(BLE_AUDIO_ENABLED),1)
export BLE := 1
export BLE_CONNECTION_MAX := 3
export HOST_GEN_ECDH_KEY := 1
export BLE_IP_VERSION := v11_0_7
export IS_BLE_FLAGS_ADV_DATA_CONFIGURED_BY_APP_LAYER := 1
export BLE_AUDIO_DOLPHIN_COMPATIBLE_SUPPORT := 0
export BLE_AUDIO_STARLORD_COMPATIBLE_SUPPORT := 1
export REPORT_EVENT_TO_CUSTOMIZED_UX := 0
export ALIGNED_WITH_FINAL_AOB_SPEC := 0
export PROMPT_SELF_MANAGEMENT := 1
export CTKD_ENABLE := 1
export IS_CTKD_OVER_BR_EDR_ENABLED := 1
export UES_MIC_AS_THE_LE_AUD_INPUT := 1
export BLE_AUDIO_24BIT := 1
export BT_FA_ECC := 0
export IO_CAPABILITY_NO_INPUT_NO_OUTPUT_MITM_FALSE := 1
export LEA_CALL_FIX_ADC_SAMPLE_RATE := 1
export SPEECH_TX_AEC2FLOAT := 0
export BT_SEC_CON_BASED_ON_COD_LE_AUD := 0 #for starlord phone

#GFPS need RPA
export BLE_ADV_RPA_ENABLED := 1
export GFPS_ENABLE := 1

export IS_USE_NEW_LC3_CODEC := 1
export AOB_LOW_LATENCY_MODE := 0
export AOB_MOBILE_ENABLED := 0

export CHIP_HAS_CP := 1
export AOB_CODEC_CP := 0
export A2DP_CP_ACCEL := 1
export SCO_CP_ACCEL := 1
export BT_RAMRUN := 1
export RAMCP_SIZE := 0x40000
export RAMCPX_SIZE := 0x20000
endif
#### LE-Audio related feature end####
export UNIFY_HEAP_ENABLED ?= 1

export USE_OVERLAY_TXT_GAP ?= 1

include config/best1502x_ibrt_anc/target.mk