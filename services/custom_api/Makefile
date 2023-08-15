cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

CUSTOM_API_LIB_NAME := libcustom_api

ifeq ($(IBRT_UI_V2),1)
CUSTOM_API_LIB_NAME := $(CUSTOM_API_LIB_NAME)_multipoint
endif

ifeq ($(USE_TRACE_ID), 1)
CUSTOM_API_LIB_NAME := $(CUSTOM_API_LIB_NAME)_log_compressed
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
CUSTOM_API_LIB_NAME := $(CUSTOM_API_LIB_NAME)_aob
endif

ifeq ($(GFPS_ENABLE), 1)
CUSTOM_API_LIB_NAME := $(CUSTOM_API_LIB_NAME)_gfps
endif

ifeq ($(SASS_ENABLE), 1)
CUSTOM_API_LIB_NAME := $(CUSTOM_API_LIB_NAME)_sass
endif

ifeq ($(AOB_MOBILE_ENABLED), 1)
ifeq ($(BLE_USB_AUDIO_SUPPORT), 1)
CUSTOM_API_LIB_NAME := $(CUSTOM_API_LIB_NAME)_usb_dongle
endif
endif

$(CUSTOM_API_LIB_NAME)-y := $(obj-y)

obj-y :=  $(CUSTOM_API_LIB_NAME).a

CFLAGS_app_custom_api.o += -DBESLIB_INFO=$(BESLIB_INFO)

ccflags-y += \
    -Iapps \
    -Iapps/main \
    -Iapps/common \
    -Iapps/key \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iplatform/drivers/bt \
    -Iservices/bt_client \
    -Iutils/cqueue \
    -Iutils/heap	\
    -Iutils/hsm     \
    -Iservices/nv_section/factory_section	\
    -Iservices/bt_profiles_enhanced/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/ibrt_ui_v2/inc/ \
    -Iservices/app_ibrt_v2/inc/ \
    -Iutils/intersyshci/ \
    -Iservices/osif/ \
    -Iservices/audio_manager

ifeq ($(SBC_REDUCE_SIZE),1)
ccflags-y += \
	-Iservices/multimedia/audio/codec/sbc_reduce/inc \
	-Iservices/multimedia/audio/codec/sbc_reduce/src/inc
else
ccflags-y += \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc
endif

ifeq ($(IBRT),1)
subdir-ccflags-y += \
    -Iservices/ibrt_middleware/inc

ifeq ($(IBRT_CORE_V2),1)
subdir-ccflags-y += \
    -Iservices/ibrt_core_v2/inc
endif
endif

ifeq ($(BLE_AUDIO_ENABLED),1)
subdir-ccflags-y += \
    -I$(BLE_AUDIO_CORE_DIR_PATH)/inc/ \
    $(BLE_STACK_INCLUDES)	\
    $(BLE_PROFILES_INCLUDES)
endif

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
   -Iservices/ble_app/app_gfps \
   -Iservices/gfps/inc
endif