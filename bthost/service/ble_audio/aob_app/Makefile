cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_tmp_aob := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gaf_app/*.cpp))

obj_tmp_gaf := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gaf_audio/*.cpp))

ifeq ($(GAF_CODEC_CROSS_CORE),1)
obj_tmp_gaf += gaf_codec_off_bth/gaf_codec_cc_common.o
endif

#aob bis
export CFG_BAP_BC ?= 0
ifeq ($(CFG_BAP_BC), 0)
obj_cpp := $(filter-out $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gaf_app/aob_bis_*.*)), $(obj_tmp_aob))
obj_cpp += $(filter-out $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gaf_audio/gaf_bis_*.*)), $(obj_tmp_gaf))
else
obj_cpp := $(obj_tmp_aob)
obj_cpp += $(obj_tmp_gaf)
endif

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y := $(src_obj)

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_PROFILES_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -I$(BLE_AOB_APP_DIR_PATH)/gaf_app \
    -I$(BLE_AOB_APP_DIR_PATH)/gaf_audio \
    -I$(BLE_AUDIO_CORE_DIR_PATH)/inc \
    -I$(BLE_AUDIO_TEST_DIR_PATH) \
    -Iinclude/cmsis_dsp \
    -Iservices/bt_app \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_bt \
    -Iservices/multimedia/inc/audio/process/resample/include \
    -Iservices/multimedia/inc/audio/process/filters/include \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/fpga_section \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/cp_accel \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/bone_sensor \
    -Iservices/audioflinger \
    -Iservices/audio_process \
    -Iservices/audio_dump/include \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/anc/inc \
    -Iapps/anc_assist/inc \
    -Iapps/anc_assist/src/assist \
    -Iapps/factory \
    -Iservices/interconnection/green \
    -Iutils/hwtimer_list \
    -Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
    -Iservices/nv_section/log_section \
    -Iservices/ai_voice/manager \
    -Iservices/app_ai/inc \
    -Iapps/anc/src/assist \
    -Iapps/anc_assist/inc \
    -Iapps/anc_assist/src/assist/ \
    -Iapps/battery/ \
    -Iutils/crc16 \
    -Iutils/crc32 \
    -Iutils/signal_generator \
    -Iutils/rom_utils \
    -Iservices/multimedia/inc/audio/codec/sbc/inc \
    -Iservices/multimedia/audio/codec/sbc/inc  \
    -Iservices/multimedia/inc/audio/codec/lc3/inc \
    -Iservices/multimedia/audio/codec/lc3/inc  \
    -Iservices/multimedia/audio/process/filters/include \
    -Iservices/multimedia/inc/audio/smf/core/inc \
    -Iservices/multimedia/inc/audio/smf/core/include \
    -Iservices/multimedia/inc/speech/inc \
    -Iservices/multimedia/speech/inc     \
    -Iservices/multimedia/audio/smf/core/inc \
    -Iservices/multimedia/audio/smf/core/include \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/ibrt_middleware/inc \
    -Iservices/ota \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iutils/list \
    -Iapps/bt_sync \
    -Iservices/bt_sync \
    -Iservices/audio_manager \
    -Iservices/ble_audio_core/inc \
    -Iservices/aob_app/gaf_audio \
    -Iservices/ble_audio_test \
    -Ibthost/service/ble_audio/aob_app/gaf_codec_off_bth \
    -Iapps/dsp_m55 \
    -Ibthost/service/ble_audio/aob_app/gaf_audio \
    -Ibthost/service/ble_audio/ble_usb_audio/test

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI_V1),1)
subdir-ccflags-y += \
    -Iservices/app_ibrt/inc \
    -Iservices/ibrt_ui/inc \
    -Iservices/ibrt_core/inc
endif
ifeq ($(IBRT_CORE_V2),1)
subdir-ccflags-y += \
    -Iservices/app_ibrt_v2/inc \
    -Iservices/ibrt_ui_v2/inc \
    -Iservices/ibrt_core_v2/inc
endif
endif

ifeq ($(BLE_USB_AUDIO_SUPPORT),1)
subdir-ccflags-y += \
    -Ibthost/service/ble_audio/ble_usb_audio
endif

BLE_SMF ?= 0
ifeq ($(BLE_SMF),1)
subdir-ccflags-y += -DBLE_SMF
endif

BLE_LC3 ?= 0
ifeq ($(BLE_LC3),1)
subdir-ccflags-y += -DBLE_LC3
endif

ifeq ($(BINAURAL_RECORD_PROCESS),1)
subdir-ccflags-y += -Iservices/binaural_record_process
CFLAGS_gaf_media_stream.o += -DBINAURAL_RECORD_PROCESS
CFLAGS_gaf_stream_process.o += -DBINAURAL_RECORD_PROCESS
endif

ifeq ($(CODEC_DAC_MULTI_VOLUME_TABLE),1)
CFLAGS_gaf_media_stream.o += -DCODEC_DAC_MULTI_VOLUME_TABLE
CFLAGS_aob_volume_api.o += -DCODEC_DAC_MULTI_VOLUME_TABLE
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN),1)
CFLAGS_gaf_stream_process.o += -DAUDIO_OUTPUT_SW_GAIN
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC),1)
CFLAGS_gaf_stream_process.o += -DAUDIO_OUTPUT_SW_GAIN_BEFORE_DRC
endif
