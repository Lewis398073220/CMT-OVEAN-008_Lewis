
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/a2dp_encoder_cp/*.c))

ifeq ($(A2DP_SOURCE_TEST),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)test/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)test/*.cpp))
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(A2DP_AAC_ON),1)
ifeq ($(FDKAAC_VERSION),2)
AAC_INCLUDES =  \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libAACdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libAACenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libFDK/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libMpegTPDec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libMpegTPEnc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libPCMutils/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libSBRdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libSBRenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1/libSYS/include
else
AAC_INCLUDES =  \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libAACdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libAACenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libFDK/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libMpegTPDec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libMpegTPEnc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libPCMutils/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libSBRdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libSBRenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec/libSYS/include
endif
else
AAC_INCLUDES =
endif

ccflags-y += -DBLUETOOTH_BT_IMPL

subdir-ccflags-y += \
	-Iservices/overlay \
	-Iservices/resources \
	-Iservices/audio_bt \
	-Iservices/audioflinger \
	-Iservices/audio_process \
	-Iservices/audio_manager \
	-Iapps/audioplayers \
	-Iapps/audioplayers/a2dp_decoder \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iutils/heap \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/main \
	-Iapps/key \
	-Iapps/common \
	-Iapps/battery \
	-Iservices/ibrt_middleware/inc \
	-Iservices/app_bt_source/inc \
	-Iservices/app_bt_source/src/a2dp_encoder_cp \
	-Iservices/osif/ \
	-Iservices/tota/ \
	-Ithirdparty/audio_codec_lib/liblhdc-enc/inc \
	$(AAC_INCLUDES) \
	-Iplatform/drivers/cp_accel \
	-Iservices/norflash_api \
	-Itests/anc_usb \
	-Iservices/audio_manager

ifeq ($(BT_FA_ECC),1)
subdir-ccflags-y += -D__FASTACK_ECC_ENABLE__
endif

ifeq ($(A2DP_SOURCE_TEST),1)
subdir-ccflags-y += -DA2DP_SOURCE_TEST
subdir-ccflags-y += \
	$(ESHELL_INCLUDES)
endif

