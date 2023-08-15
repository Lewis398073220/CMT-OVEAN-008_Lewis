cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += gaf_codec_cc_common.o
ifeq ($(GAF_ENCODER_CROSS_CORE_USE_M55),1)
obj-y += gaf_encoder_cc_m55_off_mcu.o
obj-y += gaf_encoder_cc_m55_lc3.o
endif

ifeq ($(GAF_DECODER_CROSS_CORE_USE_M55),1)
obj-y += gaf_decoder_cc_m55_off_mcu.o
obj-y += gaf_decoder_cc_m55_lc3.o
endif

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_PROFILES_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -I$(BLE_AUDIO_CORE_DIR_PATH)/inc \
    -I$(BLE_AUDIO_TEST_DIR_PATH) \
    -Iservices/bt_app \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/speech_algo_dsp \
    -Imultimedia/inc/speech/inc \
    -Imultimedia/inc/audio/process/resample/include \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/fpga_section \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/cp_accel \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/audioflinger \
    -Iservices/audio_dump/include \
    -Iutils/lockcqueue \
    -Iutils/kfifo \
    -Iutils/intersyshci \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/anc/inc \
    -Iapps/factory \
    -Iservices/interconnection/green \
    -Iutils/hwtimer_list \
    -Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
    -Iservices/nv_section/log_section \
    -Iservices/ai_voice/manager \
    -Iservices/app_ai/inc \
    -Iapps/battery/ \
    -Iutils/crc16 \
    -Iutils/crc32 \
    -Iutils/rom_utils \
    -Imultimedia/inc/audio/codec/sbc/inc \
    -Imultimedia/inc/audio/codec/lc3/inc \
    -Imultimedia/inc/audio/smf/core/inc \
    -Imultimedia/inc/audio/smf/core/include \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/ibrt_middleware/inc \
    -Iservices/ota \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iutils/list \
    -Iapps/bt_sync \
    -Iservices/audio_manager \
    -Ibthost/service/ble_audio/aob_app/gaf_audio \
    -Ibthost/service/ble_audio/aob_app/gaf_codec_off_bth \
    -Imultimedia/inc/audio/process/sbcplc/include \
    -Ibthost/service/ble_audio/gaf_core \
    -Ibthost/service/ble_audio/aob_app/gaf_audio

ccflags-y += -Iapps/dsp_m55