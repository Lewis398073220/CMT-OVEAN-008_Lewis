CHIP		?= best1501

DEBUG		?= 0

NOSTD		?= 1

LIBC_ROM	?= 1

PROGRAMMER	:= 1

FAULT_DUMP	?= 0

WATCHER_DOG ?= 1

ULTRA_LOW_POWER	?= 1

OSC_26M_X4_AUD2BB ?= 0

export SYS_USE_BBPLL ?= 1

export PROGRAMMER_SET_EXT_GPIO ?= 1

init-y		:=
core-y		:= tests/programmer/ platform/cmsis/ platform/hal/
core-y          += tests/programmer_inflash/

ifeq ($(OTA_BIN_COMPRESSED),1)
core-y		+= utils/lzma/
core-y		+= utils/heap/

KBUILD_CPPFLAGS += -Iutils/lzma/
KBUILD_CPPFLAGS += -Iutils/heap/

export OTA_REBOOT_FLASH_REMAP := 0
endif

LDS_FILE	:= programmer_inflash.lds

export CRC32_ROM ?= 1

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal

KBUILD_CPPFLAGS += -DPROGRAMMER_INFLASH

KBUILD_CFLAGS +=

LIB_LDFLAGS +=

CFLAGS_IMAGE +=

LDFLAGS_IMAGE +=

OTA_ENABLE ?= 1

ifneq ($(filter best1501 best1501p best1600,$(CHIP)),)
export OTA_REBOOT_FLASH_REMAP ?= 0
else
export OTA_REBOOT_FLASH_REMAP ?= 0
endif

# change the flash size to 0x1000000 if 1600 uses 16MB single flash die
export SECURE_BOOT ?= 0
ifeq ($(SECURE_BOOT),1)
KBUILD_CPPFLAGS += -DSECURE_BOOT
KBUILD_CPPFLAGS += -DSECURE_BOOT_WORKAROUND_SOLUTION
export OTA_BOOT_APP_BOOT_INFO_OFFSET ?= 0x7e9000
ifneq ($(OTA_BOOT_APP_BOOT_INFO_OFFSET), 0)
KBUILD_CPPFLAGS += -DOTA_BOOT_APP_BOOT_INFO_OFFSET=$(OTA_BOOT_APP_BOOT_INFO_OFFSET)
$(info  ******************************************************************************)
$(info  <<<<<<------        OTA_BOOT_APP_BOOT_INFO_OFFSET $(OTA_BOOT_APP_BOOT_INFO_OFFSET)        ------>>>>>>)
$(info  ******************************************************************************)
endif
KBUILD_CPPFLAGS += -DOTA_BOOT_FROM_APP_FLASH_NC
endif

export FLASH_SIZE ?= 0x400000

export USE_MULTI_FLASH ?= 0
ifeq ($(USE_MULTI_FLASH),1)
export BTH_USE_SYS_FLASH ?= 1
export BTH_USE_SYS_PERIPH ?= 1
export FLASH1_SIZE ?= 0x800000
KBUILD_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
LDS_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
endif


ifneq ($(filter best1501 best1501p best2001 best1600 best1306 best1502x,$(CHIP)),)
ifeq ($(CHIP),best1501)
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=0x2c020000 \
                   -DDOWNLOAD_UART_BANDRATE=921600
else ifeq ($(CHIP),best1501p)
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=0x2c020000 \
                   -DDOWNLOAD_UART_BANDRATE=921600
else ifeq ($(CHIP),best1306)
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=0x2c010000 \
                   -DDOWNLOAD_UART_BANDRATE=921600
else ifeq ($(CHIP),best1502x)
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=0x2c020000 \
                   -DDOWNLOAD_UART_BANDRATE=921600
else ifeq ($(CHIP),best1600)
CHIP_SUBSYS ?= bth
export BTH_AS_MAIN_MCU ?= 1
BTH_USE_SYS_FLASH ?= 1
# For ISPI iomux and DMA config
export PROGRAMMER_HAL_FULL_INIT ?= 1
ifeq ($(OTA_BIN_COMPRESSED),1)
KBUILD_CPPFLAGS += -DBTH_RAM_SIZE=0xE0000
endif
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=0x34020000 \
                   -DDOWNLOAD_UART_BANDRATE=921600
else
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=0x2c180000 \
                   -DDOWNLOAD_UART_BANDRATE=921600
endif
else
ifneq ($(filter best1400 best2300 best2300p,$(CHIP)),)
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=0x3c018000 \
                   -DDOWNLOAD_UART_BANDRATE=921600
else
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=0x3c020000 \
                   -DDOWNLOAD_UART_BANDRATE=921600
endif
endif

export SINGLE_WIRE_DOWNLOAD ?= 1
export UNCONDITIONAL_ENTER_SINGLE_WIRE_DOWNLOAD ?= 1

export PROGRAMMER_WATCHDOG ?= 1
export FLASH_UNIQUE_ID ?= 1
export TRACE_BAUD_RATE := 10*115200

ifeq ($(OTA_REBOOT_FLASH_REMAP),1)
KBUILD_CPPFLAGS += -DOTA_REBOOT_FLASH_REMAP
endif # REMAP

ifeq ($(USER_SECURE_BOOT),1)
## same with next jump entry addr offset##
export USER_SECURE_BOOT_JUMP_ENTRY_ADDR ?= 0x20000
endif

ifneq ($(filter best1501 best1501p best1306 best1502x,$(CHIP)),)
ifeq ($(CHIP),best1501)
export RAM_SIZE := 0x1A0000
else ifeq ($(CHIP),best1501p)
export RAM_SIZE := 0x1A0000
else ifeq ($(CHIP),best1306)
export RAM_SIZE := 0x80000
else ifeq ($(CHIP),best1502x)
export RAM_SIZE := 0x160000
endif
endif
