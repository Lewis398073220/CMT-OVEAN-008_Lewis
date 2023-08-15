cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

# src_obj := \
	# src/ldacBT.c \
	# src/ldaclib.c
# src_obj := $(src_obj:.c=.o)
APP_LDAC_LIB_NAME := ldac_lib
# $(APP_LDAC_LIB_NAME)-y := $(src_obj)
LIB_BIN_DIR := lib/thirdparty/audio_codec/ldac
obj-y += $(APP_LDAC_LIB_NAME).a

src_bco_decoder_obj := bco_decoder/decoder.c
src_bco_decoder_obj := $(src_bco_decoder_obj:.c=.o)
obj-y += $(src_bco_decoder_obj)

src_bco_obj := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bco_src/*.c))
src_bco_obj := $(src_bco_obj:.c=.o)
APP_LDAC_BCO_LIB_NAME := ldac_bco_lib
$(APP_LDAC_BCO_LIB_NAME)-y := $(src_bco_obj)
LIB_BIN_DIR := lib/thirdparty/audio_codec/ldac

ifeq ($(A2DP_LDAC_BCO),1)
obj-y += $(APP_LDAC_BCO_LIB_NAME).a
endif


subdir-ccflags-y += \
		-Ithirdparty/audio_codec_lib/ldac/inc  \
		-Iservices/multimedia/speech/process/common/include \
		-Iservices/bt_app \
		-Iinclude/cmsis_dsp \
		-Ithirdparty/audio_codec_lib/ldac/bco_decoder


subdir-ccflags-y += -Wall -Wno-long-long -Ofast


		
	