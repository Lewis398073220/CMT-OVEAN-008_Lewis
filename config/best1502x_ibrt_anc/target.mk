#### Add by Jay #####
export CMT_008_LDO_ENABLE ?= 1
ifeq ($(CMT_008_LDO_ENABLE),1)
KBUILD_CPPFLAGS += -DCMT_008_LDO_ENABLE
endif

export CMT_008_UI ?= 1
ifeq ($(CMT_008_UI),1)
KBUILD_CPPFLAGS += -DCMT_008_UI
endif

export CMT_008_UI_LED_INDICATION ?= 1
ifeq ($(CMT_008_UI_LED_INDICATION),1)
KBUILD_CPPFLAGS += -DCMT_008_UI_LED_INDICATION
endif

export CMT_008_MIC_CONFIG ?= 1
ifeq ($(CMT_008_MIC_CONFIG),1)
KBUILD_CPPFLAGS += -DCMT_008_MIC_CONFIG
endif

export CMT_008_NTC_DETECT ?= 1
ifeq ($(CMT_008_NTC_DETECT),1)
KBUILD_CPPFLAGS += -DCMT_008_NTC_DETECT
endif

export CMT_008_BATTERY_LOW ?= 1
ifeq ($(CMT_008_BATTERY_LOW),1)
KBUILD_CPPFLAGS += -DCMT_008_BATTERY_LOW
endif

export CMT_008_CST812T_TOUCH ?= 0
ifeq ($(CMT_008_CST812T_TOUCH),1)
KBUILD_CPPFLAGS += -DCMT_008_CST812T_TOUCH
endif

export __CST816S_TOUCH__ ?= 1
ifeq ($(__CST816S_TOUCH__),1)
KBUILD_CPPFLAGS += -D__CST816S_TOUCH__
endif

export i2c_mode_task ?= 1
ifeq ($(i2c_mode_task),1)
KBUILD_CPPFLAGS += -Di2c_mode_task
endif

export __USE_3_5JACK_CTR__ ?= 1
ifeq ($(__USE_3_5JACK_CTR__),1)
KBUILD_CPPFLAGS += -D__USE_3_5JACK_CTR__
endif

export __AUDIO_FADEIN__ ?= 0
ifeq ($(__AUDIO_FADEIN__),1)
KBUILD_CPPFLAGS += -D__AUDIO_FADEIN__
endif

export __AC107_ADC__ ?= 1
ifeq ($(__AC107_ADC__),1)
KBUILD_CPPFLAGS += -D__AC107_ADC__
endif

export PMU_VIO_3V3_ENABLE ?= 0
ifeq ($(PMU_VIO_3V3_ENABLE),1)
KBUILD_CPPFLAGS += -DPMU_VIO_3V3_ENABLE
endif

#add by pang for I2S linein
#AF_DEVICE_I2S ?= 1
#KBUILD_CPPFLAGS += -DAUDIO_LINEIN
#export I2S_MCLK_PIN ?= 1
#export I2S_MCLK_FROM_SPDIF ?= 1

export AUDIO_LINEIN ?= 1
ifeq ($(AUDIO_LINEIN),1)
KBUILD_CPPFLAGS += -DAUDIO_LINEIN
endif

export AF_DEVICE_I2S ?= 1
ifeq ($(AF_DEVICE_I2S),1)
KBUILD_CPPFLAGS += -DAF_DEVICE_I2S
endif

export I2S_MCLK_PIN ?= 1
ifeq ($(I2S_MCLK_PIN),1)
KBUILD_CPPFLAGS += -DI2S_MCLK_PIN
endif

export I2S_MCLK_FROM_SPDIF ?= 1
ifeq ($(I2S_MCLK_FROM_SPDIF),1)
KBUILD_CPPFLAGS += -DI2S_MCLK_FROM_SPDIF
endif

# If set PLAYBACK_USE_I2S ?= 1, will no BT sound stream output, add by jay. 
export PLAYBACK_USE_I2S ?= 0
ifeq ($(PLAYBACK_USE_I2S),1)
KBUILD_CPPFLAGS += -DPLAYBACK_USE_I2S
endif

export CMT_008_SPP_TOTA_V2 ?= 1
ifeq ($(CMT_008_SPP_TOTA_V2),1)
KBUILD_CPPFLAGS += -DCMT_008_SPP_TOTA_V2
endif


#### Add by Jay, end. #####

#### ANC DEFINE START ######
export ANC_APP ?= 1

#### ANC CONFIG ######
export ANC_FB_CHECK         ?= 0
export ANC_FF_CHECK         ?= 0
export ANC_TT_CHECK         ?= 0
export ANC_FF_ENABLED	    ?= 1
export ANC_FB_ENABLED	    ?= 1
export ANC_ASSIST_ENABLED   ?= 0
export AUDIO_ANC_FB_MC      ?= 0
export AUDIO_ANC_FB_MC_HW   ?= 1
export AUDIO_ANC_FB_ADJ_MC  ?= 0
export AUDIO_SECTION_SUPPT  ?= 1
export AUDIO_ANC_SPKCALIB_HW ?= 0
export AUDIO_ANC_FIR_HW     ?= 1
export AUDIO_ANC_TT_HW      ?= 0
##### ANC DEFINE END ######

export PSAP_APP  ?= 0

AUDIO_HW_LIMITER ?= 0
ifeq ($(AUDIO_HEARING_COMPSATN),1)
AUDIO_HW_LIMITER := 1
export PSAP_APP_ONLY_MUSIC := 1
endif
ifeq ($(AUDIO_HW_LIMITER),1)
export PSAP_APP := 1
endif

ifeq ($(AUDIO_ANC_TT_HW),1)
export AUDIO_ANC_FB_MC_HW := 1
endif

export AUDIO_OUTPUT_DAC2 ?= 0

APP_ANC_TEST ?= 1
ifeq ($(APP_ANC_TEST),1)
export TOTA_v2 := 1
endif

ifeq ($(ANC_APP),1)
KBUILD_CPPFLAGS += \
    -DANC_APP \
    -D__BT_ANC_KEY__
endif

ifeq ($(USE_CYBERON),1)

export THIRDPARTY_LIB ?= cyberon
KBUILD_CPPFLAGS += -D__CYBERON

export KWS_IN_RAM := 0
ifeq ($(KWS_IN_RAM),1)
LDS_CPPFLAGS += -DKWS_IN_RAM
endif #KWS_IN_RAM

endif #USE_CYBERON

export HAS_BT_SYNC = 1
include config/best1502x_ibrt/target.mk
