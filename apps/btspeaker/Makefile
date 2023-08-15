cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))


obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -I$(BLE_STACK_DIR_PATH)/modules/common/api/ \
    -Iservices/bt_app \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/audioflinger \
    -Iservices/nvrecord \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/log_section \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_process \
    -Iapps/apptester \
    -Iapps/factory \
    -Iutils/crc32 \
    -Iutils/cqueue \
    -Iservices/audio_manager \
    -Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/multimedia/audio/codec/sbc/src/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/cp_accel \
    -Iservices/ble_app \
    -Iservices/ble_app/app_main \
    -Iservices/ble_stack/ble_ip \
    -Iservices/ble_stack/hl/api \
    -Iservices/ble_stack/app/api/ \
    -Iservices/ble_stack/common/api/ \
    -Iservices/ble_stack/hl/inc/ \
    -Iservices/ibrt_middleware/inc \
    -Iservices/ble_stack/ke/api

ifeq ($(IBRT),1)
ccflags-y += -Iservices/ibrt_core_v2/inc
ccflags-y += -Iservices/ibrt_ui_v2/inc
ccflags-y += -Iservices/app_ibrt_v2/inc
endif
