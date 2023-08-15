
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s   := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.S))
obj_c   := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y :=  $(src_obj)

subdir-ccflags-y += \
    -Iapps/key \
    -Iapps/main \
    -Iplatform/hal \
    -Iplatform/drivers/bt \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/audio_bt \
    -Iservices/ai_voice/manager \
    -Iservices/ibrt_middleware/inc \
    -Iservices/voicepath \
    -Iservices/voicepath/gsound/gsound_target_api_read_only \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/ai_voice/protocol/bixbyvoice \
    -Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
    -Ithirdparty/userapi \
    -Iservices/app_ai/inc \
    -Iutils/cqueue  \
    -Iutils/hsm \
    -Iapps/factory \
    -Iservices/ota \
    -Iservices/audio_manager \
    -Iservices/custom_api

ifeq ($(SBC_REDUCE_SIZE),1)
subdir-ccflags-y += \
	-Iservices/multimedia/audio/codec/sbc_reduce/inc \
	-Iservices/multimedia/audio/codec/sbc_reduce/src/inc
else
subdir-ccflags-y += \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc
endif

ifeq ($(BES_OTA), 1)
subdir-ccflags-y += \
    -Iservices/ota/bes_ota/inc
endif

ifeq ($(AI_OTA), 1)
subdir-ccflags-y += \
    -Iservices/ota/ai_ota
endif

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI_V1),1)
subdir-ccflags-y += \
    -Iservices/app_ibrt/inc \
    -Iservices/ibrt_core/inc \
    -Iservices/ibrt_ui/inc
endif
ifeq ($(IBRT_CORE_V2),1)
subdir-ccflags-y += \
    -Iservices/app_ibrt_v2/inc \
    -Iservices/ibrt_core_v2/inc \
    -Iservices/ibrt_ui_v2/inc
endif
endif

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
   -Iservices/ble_app/app_gfps \
   -Iservices/gfps/inc
endif
