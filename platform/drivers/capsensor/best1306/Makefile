cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
    -Iplatform/drivers/ana/ \
    -Iservices/capsensor/ \
    -Iplatform/hal/$(CHIP)/ \
    -Iplatform/drivers/capsensor/

ifeq ($(CAPSENSOR_SEVEN_CHANNEL),1)
ccflags-y  += -DCAPSENSOR_SEVEN_CHANNEL
endif
