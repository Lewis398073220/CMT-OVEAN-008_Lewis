obj-y := 

obj-y += ota_ble_adapter.o

ifeq ($(AI_OTA),1)
obj-y += ai_ota/
endif

ifeq ($(BES_OTA),1)
obj-y += bes_ota/
endif

ifeq ($(OTA_ENABLE),1)
obj-y += ota_basic.o
endif

ifeq ($(IBRT),1)
subdir-ccflags-y += \
    -Iservices/ibrt_middleware/inc
	
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
    -Iservices/ibrt_ui_v2/inc \
    -Iservices/bt_app \
    -Iapps/key \
    -Iservices/bt_app/a2dp_codecs/include
endif
ifeq ($(IBRT_UI_V2),1)
ccflags-y += \
    -Iservices/ibrt_ui_v2/inc
endif
endif

ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/audio_bt \
    -Iplatform/drivers/bt \
    -Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/multimedia/audio/codec/sbc/src/inc \
    -Iservices/custom_api \
    -Iservices/ota/bes_ota/inc \
    -Iapps/key \
    -Iutils/cqueue \
    -Iservices/audio_manager

