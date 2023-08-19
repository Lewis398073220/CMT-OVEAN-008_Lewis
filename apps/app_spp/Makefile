cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(CHIP_CAPSENSOR_VER),)
ccflags-y +=  \
    -Iapps/sensorhub/
endif

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iutils/cqueue \
    -Iutils/hsm \
    -Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/ibrt_core_v2/inc \
    -Iplatform/drivers/bt \
    -Iservices/ibrt_middleware/inc
