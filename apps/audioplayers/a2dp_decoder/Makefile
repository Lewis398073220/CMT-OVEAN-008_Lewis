
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := a2dp_decoder.o

ifneq ($(A2DP_SMF),1)
obj-y += a2dp_decoder_sbc.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_sbc.o += -O3
endif

ifeq ($(A2DP_AAC_ON),1)
obj-y += a2dp_decoder_aac_lc.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_aac_lc.o += -O3
endif
ifeq ($(FDKAAC_VERSION),2)
ifeq ($(AAC_REDUCE_SIZE),1)
AAC_INCLUDES =  \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libAACdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libAACenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libFDK/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libMpegTPDec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libMpegTPEnc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libPCMutils/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libSBRdec/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libSBRenc/include \
	-Iservices/multimedia/audio/codec/fdkaac_codec_2_0_1_reduce/libSYS/include
else
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
endif
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

ifeq ($(A2DP_SCALABLE_ON),1)
obj-y += a2dp_decoder_scalable.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_scalable.o += -O3
endif
SCALABLE_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/scalable
else
SCALABLE_INCLUDES =
endif

ifeq ($(A2DP_LHDC_ON),1)
obj-y += a2dp_decoder_lhdc.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_lhdc.o += -O3
endif
LHDC_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc
else
LHDC_INCLUDES =
endif

ifeq ($(A2DP_LHDCV5_ON),1)
obj-y += a2dp_decoder_lhdcv5.o
CFLAGS_a2dp_decoder_lhdcv5.o += -O3
LHDC_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/liblhdcv5-dec/inc
else
LHDC_INCLUDES =
endif

ifeq ($(A2DP_LDAC_ON),1)
obj-y += a2dp_decoder_ldac.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_ldac.o += -O3
endif
LDAC_INCLUDES =  \
	-Ithirdparty/audio_codec_lib/ldac/inc
else
LDAC_INCLUDES =
endif

ifeq ($(A2DP_LC3_ON),1)
obj-y += a2dp_decoder_lc3.o
ifeq ($(OPT_LEVEL),)
#CFLAGS_a2dp_decoder_lc3.o += -O3
endif
LC3_INCLUDES =  \
	-Iservices/multimedia/audio/codec/lc3/inc
else
LC3_INCLUDES =
endif

else
obj-y += a2dp_decoder_smf.o
ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp_decoder_smf.o += -O3
endif
SMF_INCLUDES :=  
SMF_INCLUDES += -Iservices/multimedia/audio/smf/core/inc 
SMF_INCLUDES += -Iservices/multimedia/audio/smf/core/include
SMF_INCLUDES += -DA2DP_SMF=1
endif

ccflags-y := \
	$(AAC_INCLUDES) \
	$(SCALABLE_INCLUDES) \
	$(LHDC_INCLUDES) \
	$(LDAC_INCLUDES) \
	$(LC3_INCLUDES) \
	$(SMF_INCLUDES) \
	-Iservices/audio_bt \
	-Iservices/audio_process \
	-Iservices/app_ai/inc \
	-Iservices/resources \
	-Iservices/bt_app \
	-Iservices/bt_app/a2dp_codecs/include \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iplatform/drivers/uarthci \
	-Iutils/cqueue \
	-Iservices/audio_dump/include \
	-Iservices/multimedia/speech/inc \
	-Iservices/multimedia/rbcodec/inc \
	-Iservices/multimedia/audio/process/eq/include \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/sbcplc/include \
	-Iservices/multimedia/audio/process/virtual_surround_process/include \
	-Iservices/multimedia/fm/inc \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Iservices/overlay \
	-Iservices/norflash_api \
	-Iservices/nv_section/log_section \
	-Iapps/main \
	-Iapps/audioplayers/rbplay/ \
	-Iapps/audioplayers/a2dp_decoder \
	-Iutils/list \
	-Iutils/heap \
	-Iplatform/drivers/ana \
	-Iapps/apptester \
	-Iapps/key  \
	-Iplatform/drivers/bt \
	-Iutils/crc32 \
	-Iservices/ai_voice/audio \
	-Iservices/ai_voice/protocol/bixbyvoice \
	-Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
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

ifeq ($(A2DP_LHDC_ON),1)
ccflags-y += -Iservices/bt_if_enhanced/lhdc_license
endif

ifeq ($(A2DP_LHDCV5_ON),1)
ccflags-y += -Iservices/bt_if_enhanced/lhdc_license
endif

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI_V1),1)
ccflags-y += \
    -Iservices/ibrt_core/inc \
    -Iservices/ibrt_ui/inc \
    -Iservices/app_ibrt/inc
endif

ifeq ($(IBRT_CORE_V2),1)
ccflags-y += \
    -Iservices/app_ibrt_v2/inc \
    -Iservices/ibrt_core_v2/inc \
    -Iservices/custom_api
endif

ifeq ($(IBRT_UI_V2),1)
subdir-ccflags-y += \
    -Iservices/ibrt_ui_v2/inc
endif

subdir-ccflags-y += \
    -Iservices/ibrt_middleware/inc
endif

ifeq ($(A2DP_CP_ACCEL),1)
obj-y += a2dp_decoder_cp.o
ccflags-y += -Iplatform/drivers/cp_accel
endif

ifeq ($(A2DP_TRACE_CP_ACCEL),1)
ccflags-y += -DA2DP_TRACE_CP_ACCEL
endif

ifeq ($(A2DP_TRACE_DEC_TIME),1)
ccflags-y += -DA2DP_TRACE_DEC_TIME
endif

ifeq ($(A2DP_TRACE_CP_DEC_TIME),1)
ccflags-y += -DA2DP_TRACE_CP_DEC_TIME
endif

ifeq ($(A2DP_HEAP_DEBUG),1)
CFLAGS_a2dp_decoder.o += -DA2DP_HEAP_DEBUG
endif
ifeq ($(A2DP_CP_HEAP_DEBUG),1)
CFLAGS_a2dp_decoder_cp.o += -DA2DP_CP_HEAP_DEBUG
endif

ifeq ($(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH),1)
ccflags-y += -DBT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH
endif

ifeq ($(PLAYBACK_FORCE_48K),1)
CFLAGS_a2dp_decoder.o += -DPLAYBACK_FORCE_48K
endif

ifeq ($(A2DP_SBC_PLC_ENABLED),1)
CFLAGS_a2dp_decoder_sbc.o += -DA2DP_SBC_PLC_ENABLED
endif

ifeq ($(AUDIO_REVERB),1)
ccflags-y += -D__AUDIO_REVERB__
endif

ifeq ($(A2DP_LDAC_ON),1)
ifeq ($(A2DP_LDAC_BCO),1)
ccflags-y += -DA2DP_LDAC_BCO
endif
endif

ifeq ($(AUDIO_DYNAMIC_BOOST),1)
ccflags-y += -D__AUDIO_DYNAMIC_BOOST__
endif
