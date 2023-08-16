cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
    -Iservices/audio_bt \
    -Iservices/bt_if/inc \
    -Iservices/bt_app \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iservices/nvrecord \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/log_section \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_process \
    -Iapps/apptester \
    -Iapps/factory \
    -Iutils/crc32 \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ac107 \
    -Iplatform/drivers/ana \
	-Iplatform\hal \
    -Iapps/audioplayers/rbplay \
	-Iapps\audioplayers \
    -Itests/anc_usb \
    -Iapps/anc/inc \
    -Iapps/ota \
    -Ithirdparty/userapi \
    -Iservices/voicepath \
    -Iservices/ble_app/app_main \
    -Iservices/voicepath/gsound/gsound_service \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/voicepath/gsound/gsound_target_api_read_only \
    -Iservices/bt_if/inc \
    -Iservices/app_ibrt_v2/inc\
    -Iservices/ibrt_core_v2/inc\
    -Iservices/ibrt_middleware/inc\
    -Iservices/ibrt_ui_v2/inc\
	-Iservices\multimedia\audio\codec\sbc\inc \
    -Iservices/communication \
    -Iutils/cqueue \
	-Iutils/hwtimer_list \
    -Iservices/ai_voice/audio \
    -Iservices/ai_voice/manager \
	-Iservices/ai_voice/transport \
    -Iservices/interconnection \
	-Iservices/ble_app \
	-Iservices/ble_stack/ble_ip \
	-Iservices/ble_stack/hl/api \
	-Iservices/ble_stack/app/api/  \
	-Iservices/ble_stack/common/api/ \
	-Iservices/ble_stack/hl/inc/ \
	-Iservices/ble_stack/ke/api \
	-Iservices/bridge/ \
	-Iservices/bt_profiles/inc/ \
	-Iservices/bt_profiles/inc/sys/ \
	-Iservices/ble_stack/common/api/ \
	-Iservices/ble_stack/hl/inc/ \
	-Iservices/ble_stack/hl/api/ \
	-Iservices/ble_stack/hl/src/gap/ \
	-Iservices/ble_stack/hl/src/gap/gapc/ \
	-Iservices/ble_stack/hl/src/gap/gapm/ \
	-Iservices/ble_stack/hl/src/gap/smpc/ \
	-Iservices/ble_stack/ke/api/ \
	-Iservices/ble_stack/ke/src/ \
	-Iservices/ble_stack/hl/src/gatt/attc/ \
	-Iservices/ble_stack/hl/src/gatt/attm/ \
	-Iservices/ble_stack/hl/src/gatt/atts/ \
	-Iservices/ble_stack/hl/src/gatt/gattc/ \
	-Iservices/ble_stack/hl/src/gatt/gattm/ \
	-Iservices/ble_stack/hl/src/gatt/ \
	-Iservices/ble_stack/hl/src/l2c/l2cc/ \
	-Iservices/ble_stack/hl/src/l2c/l2cm/ \
	-Iservices/ble_stack/hci/api/ \
	-Iservices/ble_stack/hci/src/ \
	-Iservices/ble_stack/ble_ip/  \
	-Iservices/ble_stack/app/api/  \
	-Iservices/ble_stack/app/src/ \
    -Ithirdparty/tile \
    -Ithirdparty/tile/tile_target \
	-Iservices\multimedia\audio\process\filters\include \
	-Iservices\bt_app\a2dp_codecs\include \
	-Iplatform\drivers\ac107 \
 -Iservices/bt_app \
					-Iservices/audio_bt \
                    -Iservices/audio_manager \
                    -Ibthost/service\bt_app \
                    -Iapps/key \
                    -Iservices/bt_if_enhanced/inc \
                    -Iutils/cqueue \
                    -Iutils/hwtimer_list \
                    -Iapps/common \
                    -Iservices/multimedia/audio/codec/sbc/inc \
                    -Iservices/bt_app/a2dp_codecs/include \
                    -Iplatform/hal \
                    -Iapps/anc/inc \
                    -Iapps/main \
                    -Ibthost/service/bt_app/inc \
                    -Ibthost/stack/bt_if/inc \
                    -Ibthost/service/bt_app \
                    -Ibthost/adapter/inc/adapter_service \
                    -Ibthost/adapter/inc/bt/common \
                    -Ibthost/stack/bt_profiles/inc \
                    -Ibthost/adapter/inc/bt \
                    -Ibthost/service/bt_app/inc/a2dp_codecs \
	
ifeq ($(ANC_APP),1)
CFLAGS_app_user.o += -DANC_APP
endif