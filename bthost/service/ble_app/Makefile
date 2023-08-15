cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_main/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ble_uart/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_dis/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_gatt/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)test/*.c))

ifneq ($(__BES_OTA_MODE__),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_htp/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_datapath/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_hrps/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_hid/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_sec/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_batt/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ota/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_gfps/*.c))

ifeq ($(ANCS),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ancs/*.c))
endif

ifeq ($(ANCC),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ancc/*.c))
endif

ifeq ($(AMSC),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_amsc/*.c))
endif

ifeq ($(BLE_WALKIE_TALKIE),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_walkie_talkie/*.c))
endif
ifeq ($(SWIFT_ENABLE),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_swift/*.c))
subdir-ccflags-y += \
    -I$(BLE_APP_DIR_PATH)/app_swift/
endif

endif

ifeq ($(AI_VOICE),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ai/*.c))
endif

ifeq ($(TILE_DATAPATH_ENABLED),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_tile/*.c))
endif

ifeq ($(TEST_OVER_THE_AIR),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/app_tota/*.c))
subdir-ccflags-y += \
    -I$(BLE_PROFILES_DIR_PATH)/tota/\
    -I$(BLE_APP_DIR_PATH)/app_tota/
endif

ifeq ($(TEST_OVER_THE_AIR_v2),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/app_tota/*.c))
subdir-ccflags-y += \
    -I$(BLE_PROFILES_DIR_PATH)/tota/\
    -I$(BLE_APP_DIR_PATH)/app_tota/
endif

ifeq ($(BLE_HOST_PTS_TEST_ENABLED),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/app_pts_test/src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/app_pts_test/src/GAP/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/app_pts_test/src/L2CAP/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/app_pts_test/src/GATT/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/app_pts_test/src/SM/*.c))
subdir-ccflags-y +=                         \
    -I$(BLE_APP_DIR_PATH)/app_pts_test/inc/
endif

obj-y := $(obj_c:.c=.o) $(obj_cpp:.cpp=.o)

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_PROFILES_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -I$(BLE_PROFILES_DIR_PATH)/hogp \
    -I$(BLE_PROFILES_DIR_PATH)/hogp/hogpbh/api \
    -I$(BLE_PROFILES_DIR_PATH)/hogp/hogpbh/src \
    -I$(BLE_PROFILES_DIR_PATH)/hogp/hogpd/api \
    -I$(BLE_PROFILES_DIR_PATH)/hogp/hogpd/src \
    -I$(BLE_PROFILES_DIR_PATH)/hogp/hogprh/api \
    -I$(BLE_PROFILES_DIR_PATH)/hogp/hogprh/src \
    -I$(BLE_PROFILES_DIR_PATH)/bas/basc/api \
    -I$(BLE_PROFILES_DIR_PATH)/bas/basc/src \
    -I$(BLE_PROFILES_DIR_PATH)/bas/bass/api \
    -I$(BLE_PROFILES_DIR_PATH)/bas/bass/src \
    -I$(BLE_PROFILES_DIR_PATH)/dis/diss/api  \
    -I$(BLE_PROFILES_DIR_PATH)/dis/diss/src  \
    -I$(BLE_APP_DIR_PATH)/app_dis/ \
    -Iservices/voicepath/$(VOICE_DATAPATH_TYPE) \
    -Iservices/audio_process \
    -Iapps/audioplayers/a2dp_decoder \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/nvrecord \
    -Iservices/resources \
    -Iservices/voicepath/ \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/retention_ram \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/heap \
    -Iutils/hsm \
    -Iutils/crc16 \
    -Iutils/hwtimer_list \
    -Iutils/aes_cmac \
    -Iapps/battery \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/factory \
    -Iservices/audio_dump/include \
    -Iservices/bridge/ \
    -Imultimedia/inc/speech/inc \
    -Iservices/audio_bt \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/voicepath/gsound/gsound_custom/src \
    -Iservices/voicepath/gsound/gsound_target_api_read_only \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/transport \
    -Iservices/app_ai/inc \
    -Imultimedia/inc/audio/codec/sbc/inc \
    -Iservices/multimedia/audio/codec/sbc/inc \
    -Imultimedia/inc/audio/codec/lc3/inc \
    -Iservices/ibrt_middleware/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath/gsound/gsound_target \
    -Ithirdparty/tile \
    -Ithirdparty/tile/tile_target \
    -Ithirdparty/tile/tile_common/tile_assert \
    -Ithirdparty/tile/tile_common/tile_features \
    -Ithirdparty/tile/tile_common/tile_gatt_db \
    -Ithirdparty/tile/tile_common/tile_player \
    -Ithirdparty/tile/tile_common/tile_service \
    -Ithirdparty/tile/tile_common/tile_storage \
    -Iservices/ota\
    -Iservices/aob_app/gaf_app \
    -Iservices/audio_manager \
    -Iutils/encrypt

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_voice_over_ble.o += -D__AUDIO_RESAMPLE__
endif
ifeq ($(SW_PLAYBACK_RESAMPLE),1)
CFLAGS_voice_over_ble.o += -DSW_PLAYBACK_RESAMPLE
endif
ifeq ($(SW_CAPTURE_RESAMPLE),1)
CFLAGS_voice_over_ble.o += -DSW_CAPTURE_RESAMPLE
endif
ifeq ($(BES_OTA_BASIC),1)
ifeq ($(IBRT_OTA),1)
ccflags-y += -Iservices/ibrt_ota/inc
else
ccflags-y += -Iservices/ota
endif
endif

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI_V1),1)
subdir-ccflags-y += \
    -Iservices/app_ibrt/inc \
    -Iservices/ibrt_ui/inc \
    -Iservices/ibrt_core/inc

endif
ifeq ($(IBRT_CORE_V2),1)
subdir-ccflags-y += \
    -Iservices/ibrt_core_v2/inc \
    -Iservices/ibrt_ui_v2/inc \
    -Iservices/app_ibrt_v2/inc

endif
endif

ifeq ($(BES_OTA),1)
ccflags-y += -Iservices/ota/bes_ota/inc
endif

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
    -I$(BLE_APP_DIR_PATH)/app_gfps \
	-Iservices/gfps/inc
endif

ifeq ($(BLE_AUDIO_ENABLED),1)
ccflags-y += \
    -Iservices/audio_manager \
    -I$(BLE_AUDIO_CORE_DIR_PATH)/inc
endif

ifeq ($(LE_ADV_TX_LOWPOWER),1)
ccflags-y += -DLE_ADV_TX_LOWPOWER
endif