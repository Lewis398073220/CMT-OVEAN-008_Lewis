cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iservices/ibrt_middleware/inc \
	-Iapps/audioplayers/a2dp_decoder \
	-Iservices/bt_app \
	-Iservices/audio_bt \
	-Iservices/resources \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iutils/list \
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

ifeq ($(IBRT_UI_V2),1)
ccflags-y += \
    -Iservices/ibrt_ui_v2/inc
endif

ifeq ($(A2DP_LDAC_ON),1)
ifeq ($(A2DP_LDAC_BCO),1)
ccflags-y += -DA2DP_LDAC_BCO
endif
endif