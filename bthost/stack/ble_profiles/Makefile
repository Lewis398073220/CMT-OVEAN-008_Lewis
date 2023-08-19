
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)datapath/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hrp/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hogp/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bas/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dis/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)prf/*.c))

ifeq ($(BLE_AUDIO_ENABLED),1)
ble_profile_src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)csip/*.c))
ble_profile_src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)otp/*.c))
ble_profile_src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dtp/*.c))
endif

ifeq ($(AI_VOICE),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ai/*.c))
endif

ifeq ($(BMS),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bms/*.c))
ccflags-y += \
        -I$(BLE_PROFILES_DIR_PATH)/bms/bmsp/api \
        -I$(BLE_PROFILES_DIR_PATH)/bms/bmsp/src
endif

ifeq ($(BMSC),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bms/*.c))
ccflags-y += \
        -I$(BLE_PROFILES_DIR_PATH)/bms/bmsc/api \
        -I$(BLE_PROFILES_DIR_PATH)/bms/bmsc/src
endif

ifeq ($(ANCC), 1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)anc/*.c))
ccflags-y += \
        -I$(BLE_PROFILES_DIR_PATH)/anc/ancc/api \
        -I$(BLE_PROFILES_DIR_PATH)/anc/ancc/src
endif

ifeq ($(ANCS), 1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)anc/*.c))
ccflags-y += \
        -I$(BLE_PROFILES_DIR_PATH)/anc/ancs/api \
        -I$(BLE_PROFILES_DIR_PATH)/anc/ancs/src
endif

ifeq ($(AMS), 1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ams/*.c))
ccflags-y += \
        -I$(BLE_PROFILES_DIR_PATH)/ams/amsp/api \
        -I$(BLE_PROFILES_DIR_PATH)/ams/amsp/src
endif

ifeq ($(AMSC),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ams/*.c))
ccflags-y += \
        -I$(BLE_PROFILES_DIR_PATH)/ams/amsc/api \
        -I$(BLE_PROFILES_DIR_PATH)/ams/amsc/src
endif

ifeq ($(TILE_DATAPATH_ENABLED),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/tile/*.c))
endif

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ota/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gfps/gfps_provider/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)buds/*.c))

ifeq ($(TEST_OVER_THE_AIR),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tota/*.c))
subdir-ccflags-y +=-Iservices/tota
endif

ifeq ($(TEST_OVER_THE_AIR_v2),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tota/*.c))
subdir-ccflags-y +=-Iservices/tota_v2
endif

obj-y := $(obj_c:.c=.o)

ifeq ($(GFPS_ENABLE),1)
obj-y += gfps/
subdir-ccflags-y += -Iservices/gfps/inc
endif

ifeq ($(BLE_AUDIO_ENABLED),1)
BLE_PROFILE_LIB_NAME := libble_profile

ifeq ($(GFPS_ENABLE),1)
BLE_PROFILE_LIB_NAME := $(BLE_PROFILE_LIB_NAME)_gfps
endif

ifeq ($(USE_TRACE_ID), 1)
BLE_PROFILE_LIB_NAME := $(BLE_PROFILE_LIB_NAME)_log_compressed
endif

$(BLE_PROFILE_LIB_NAME)-y := $(ble_profile_src_obj:.c=.o)
obj-y += $(BLE_PROFILE_LIB_NAME).a
endif

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_PROFILES_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -I$(BLE_PROFILES_DIR_PATH)/hogp            \
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
    -I$(BLE_PROFILES_DIR_PATH)/dis/diss/api \
    -I$(BLE_PROFILES_DIR_PATH)/dis/diss/src \
    -Iservices/audio_process \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/nvrecord \
    -Iservices/resources \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/retention_ram \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/factory \
    -Iservices/bridge/ \
    -Iservices/ai_voice/manager \
    -Iservices/app_ai/inc \
    -Ithirdparty/tile/tile_target/ \
    -Ithirdparty/tile/tile_common/tile_assert \
    -Ithirdparty/tile/tile_common/tile_features \
    -Ithirdparty/tile/tile_common/tile_gatt_db \
    -Ithirdparty/tile/tile_common/tile_player \
    -Ithirdparty/tile/tile_common/tile_service \
    -Ithirdparty/tile/tile_common/tile_storage \
    -Iservices/bt_app \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iutils/encrypt \
    -Iservices/ota/bes_ota/inc \
    -Iservices/ota

ifeq ($(BES_OTA),1)
ifeq ($(IBRT_OTA),1)
ccflags-y += -Iservices/ibrt_ota/inc
endif
endif
