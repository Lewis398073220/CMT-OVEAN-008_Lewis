
UAUD_CFG_FLAGS :=

ifeq ($(USB_AUDIO_DYN_CFG),1)

UAUD_CFG_FLAGS += -DUSB_AUDIO_DYN_CFG

USB_AUDIO_32BIT ?= 1
USB_AUDIO_24BIT ?= 1
USB_AUDIO_16BIT ?= 1

USB_AUDIO_16K ?= 1
USB_AUDIO_32K ?= 0
USB_AUDIO_44_1K ?= 1
USB_AUDIO_48K ?= 1
USB_AUDIO_96K ?= 1
ifeq ($(USB_HIGH_SPEED),1)
USB_AUDIO_176_4K ?= 1
USB_AUDIO_192K ?= 1
ifeq ($(USB_AUDIO_UAC2),1)
USB_AUDIO_352_8K ?= 1
USB_AUDIO_384K ?= 1
endif
endif

else # USB_AUDIO_DYN_CFG != 1

ifeq ($(filter 1,$(USB_AUDIO_384K) $(USB_AUDIO_352_8K) $(USB_AUDIO_192K) $(USB_AUDIO_176_4K) \
		$(USB_AUDIO_96K) $(USB_AUDIO_44_1K) $(USB_AUDIO_32K) $(USB_AUDIO_16K) $(USB_AUDIO_8K)),)
USB_AUDIO_48K := 1
endif

ifeq ($(filter 1,$(USB_AUDIO_32BIT) $(USB_AUDIO_24BIT)),)
USB_AUDIO_16BIT := 1
endif

endif # USB_AUDIO_DYN_CFG != 1

ifeq ($(filter 1,$(USB_AUDIO_384K) $(USB_AUDIO_352_8K) $(USB_AUDIO_192K) $(USB_AUDIO_176_4K) \
		$(USB_AUDIO_96K) $(USB_AUDIO_48K) $(USB_AUDIO_44_1K) $(USB_AUDIO_32K) $(USB_AUDIO_16K) $(USB_AUDIO_8K)),)
$(error None of usb audio sample rates is enabled)
endif

ifeq ($(USB_AUDIO_384K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_384K
endif
ifeq ($(USB_AUDIO_352_8K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_352_8K
endif
ifeq ($(USB_AUDIO_192K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_192K
endif
ifeq ($(USB_AUDIO_176_4K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_176_4K
endif
ifeq ($(USB_AUDIO_96K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_96K
endif
ifeq ($(USB_AUDIO_48K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_48K
endif
ifeq ($(USB_AUDIO_44_1K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_44_1K
endif
ifeq ($(USB_AUDIO_32K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_32K
endif
ifeq ($(USB_AUDIO_16K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_16K
endif
ifeq ($(USB_AUDIO_8K),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_8K
endif

ifeq ($(filter 1,$(USB_AUDIO_32BIT) $(USB_AUDIO_24BIT) $(USB_AUDIO_16BIT)),)
$(error None of USB_AUDIO_32BIT/USB_AUDIO_24BIT/USB_AUDIO_16BIT is enabled)
endif

ifeq ($(USB_AUDIO_32BIT),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_32BIT
endif
ifeq ($(USB_AUDIO_24BIT),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_24BIT
endif
ifeq ($(USB_AUDIO_16BIT),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_16BIT
endif

ifeq ($(filter 1,$(USB_AUDIO_SEND_32BIT) $(USB_AUDIO_SEND_24BIT)),)
USB_AUDIO_SEND_16BIT := 1
ifneq ($(USB_AUDIO_SEND_16BIT),1)
$(error None of USB_AUDIO_SEND_32BIT/USB_AUDIO_SEND_24BIT/USB_AUDIO_SEND_16BIT is enabled)
endif
endif

ifeq ($(USB_AUDIO_SEND_32BIT),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_SEND_32BIT
endif
ifeq ($(USB_AUDIO_SEND_24BIT),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_SEND_24BIT
endif
ifeq ($(USB_AUDIO_SEND_16BIT),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_SEND_16BIT
endif

ifneq ($(USB_AUDIO_SEND_CHAN),)
UAUD_CFG_FLAGS += -DUSB_AUDIO_SEND_CHAN=$(USB_AUDIO_SEND_CHAN)
endif

ifeq ($(USB_AUDIO_UAC2),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_UAC2
endif

ifeq ($(USB_AUDIO_MULTIFUNC),1)
UAUD_CFG_FLAGS += -DUSB_AUDIO_MULTIFUNC
endif

