cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

subdir-ccflags-y += -Iservices/bt_app \
                                -Iapps/key \
		-Iservices/bt_if\inc \
                                -Iapps/common \
		-Iservices/multimedia/audio/codec/sbc/inc \
		-Iservices/bt_app/a2dp_codecs/include \
		-Iplatform/hal \
        -Iplatform/drivers/ana \
		-Iapps/anc/inc \
		-Ibthost/service/bt_app/inc \
		-Ibthost/stack/bt_if/inc \
		-Ibthost/adapter/inc/adapter_service \
		-Ibthost/adapter/inc/bt/common \
		-Ibthost/stack/bt_profiles/inc \
		-Iutils/cqueue \
		-Ibthost/adapter/inc/bt \
		-Ibthost/service/bt_app/inc/a2dp_codecs \
		-Iservices/audio_manager \
		-Iapps/main



