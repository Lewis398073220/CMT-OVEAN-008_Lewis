cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

AOB_LIB_NAME := libaob_core

ifeq ($(AOB_MOBILE_ENABLED), 1)
ifeq ($(BLE_USB_AUDIO_SUPPORT), 1)
AOB_LIB_NAME := $(AOB_LIB_NAME)_usb_dongle
endif
endif

$(AOB_LIB_NAME)-y := $(src_obj)

obj-y :=  $(AOB_LIB_NAME).a

CFLAGS_ble_audio_core.o += -DBESLIB_INFO=$(BESLIB_INFO)

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_PROFILES_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -I$(BLE_AUDIO_CORE_DIR_PATH)/inc \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/resources \
    -Imservices/ultimedia/inc/audio/process/resample/include \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/fpga_section \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/audioflinger \
    -Iservices/audio_dump/include \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/anc/inc \
    -Iapps/factory \
    -Iservices/interconnection/green \
    -Iutils/hwtimer_list \
    -Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
    -Iservices/nv_section/log_section \
    -Iservices/ai_voice/manager \
    -Iservices/app_ai/inc \
    -Iapps/battery/ \
    -Iutils/crc16 \
    -Iutils/crc32 \
    -Iutils/rom_utils \
    -Iservices/multimedia/inc/audio/codec/sbc/inc \
    -Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/multimedia/inc/audio/codec/lc3/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/ibrt_middleware/inc \
    -Iservices/ota \
    -Iutils/hsm \
    -Iutils/list \
    -Iservices/audio_manager \
    -Ibthost/service/ble_audio/ble_audio_test \
    -Ibthost/service/ble_audio/aob_app/inc

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI_V1),1)
subdir-ccflags-y += \
    -Iservices/app_ibrt/inc \
    -Iservices/ibrt_ui/inc \
    -Iservices/ibrt_core/inc
endif
ifeq ($(IBRT_CORE_V2),1)
subdir-ccflags-y += \
    -Iservices/app_ibrt_v2/inc \
    -Iservices/ibrt_ui_v2/inc \
    -Iservices/ibrt_core_v2/inc
endif
endif

ifeq ($(BLE_USB_AUDIO_SUPPORT),1)
subdir-ccflags-y += \
    -Ibthost/service/ble_audio/ble_usb_audio
endif
