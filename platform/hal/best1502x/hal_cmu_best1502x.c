/***************************************************************************
 *
 * Copyright 2015-2021 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(reg_cmu)
#include CHIP_SPECIFIC_HDR(reg_aoncmu)
#include CHIP_SPECIFIC_HDR(reg_aonsec)
#include CHIP_SPECIFIC_HDR(reg_btcmu)
#include CHIP_SPECIFIC_HDR(reg_senscmu)
#include "hal_cmu.h"
#include "hal_aud.h"
#include "hal_bootmode.h"
#include "hal_chipid.h"
#include "hal_codec.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_sleep_core_pd.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "cmsis_nvic.h"
#include "pmu.h"
#include "system_cp.h"

#ifdef USB_HIGH_SPEED
#ifndef USB_CLK_SRC_PLL
#define USB_CLK_SRC_PLL
#endif
#endif

//#define NO_RAM_RETENTION_FSM

#ifndef BT_LOG_POWEROFF
#define NO_BT_RAM_BLK_RETENTION
#endif

#ifdef AUDIO_USE_BBPLL
#define AUDIO_PLL_SEL                   HAL_CMU_PLL_BB
#elif defined(AUDIO_USE_USBPLL)
#define AUDIO_PLL_SEL                   HAL_CMU_PLL_USB
#else
#define AUDIO_PLL_SEL                   HAL_CMU_PLL_AUD
#endif

#define HAL_CMU_PLL_LOCKED_TIMEOUT_US   200
#define HAL_CMU_LPU_EXTRA_TIMEOUT_US    50      // 1000
#ifdef BT_RCOSC_CAL
#if defined(BT_LOG_POWEROFF) || defined(CORE_SLEEP_POWER_DOWN) || defined(SENS_CORE_SLEEP_POWER_DOWN)
#define HAL_CMU_26M_READY_TIMEOUT_US    1000
#else
#if defined(NO_BT_RAM_BLK_RETENTION) || defined(NO_RAM_RETENTION_FSM)
#define HAL_CMU_26M_READY_TIMEOUT_US    400     // OSC stable time: 200 us (175 us in real chip as LPO is about 40K)
#else
#define HAL_CMU_26M_READY_TIMEOUT_US    1000
#endif
#endif
#else // !BT_RCOSC_CAL
#define HAL_CMU_26M_READY_TIMEOUT_US    4000
#endif

#define BT_PD_WAKEUP_CYCLE_CNT          17      // (14) [PowerUp] + (2 + timer1) [RAM Wakeup]
#define BT_NORMAL_WAKEUP_CYCLE_CNT      10      // (1 + 2 + timer2 + timer3 + 1 + 1) [RAM RET] + (2 + timer1) [RAM Wakeup]

#define MCU_PD_WAKEUP_CYCLE_CNT         17      // (14) [PowerUp] + (2 + timer1) [RAM Wakeup]
#define MCU_NORMAL_WAKEUP_CYCLE_CNT     4       // (1) + (2 + timer1)

#ifdef BT_LOG_POWEROFF
#define BT_WAKEUP_CYCLE_CNT             BT_PD_WAKEUP_CYCLE_CNT
#elif defined(NO_BT_RAM_BLK_RETENTION) || defined(NO_RAM_RETENTION_FSM)
#define BT_WAKEUP_CYCLE_CNT             0
#else
#define BT_WAKEUP_CYCLE_CNT             BT_NORMAL_WAKEUP_CYCLE_CNT
#endif

#ifdef CORE_SLEEP_POWER_DOWN
#define MCU_WAKEUP_CYCLE_CNT            MCU_PD_WAKEUP_CYCLE_CNT
#else
#define MCU_WAKEUP_CYCLE_CNT            MCU_NORMAL_WAKEUP_CYCLE_CNT
#endif

#ifdef SENS_CORE_SLEEP_POWER_DOWN
#define SENS_WAKEUP_CYCLE_CNT           MCU_PD_WAKEUP_CYCLE_CNT
#else
#define SENS_WAKEUP_CYCLE_CNT           MCU_NORMAL_WAKEUP_CYCLE_CNT
#endif

#if (LPU_TIMER_US(HAL_CMU_26M_READY_TIMEOUT_US) < BT_WAKEUP_CYCLE_CNT)
#error "HAL_CMU_26M_READY_TIMEOUT_US should >= BT_WAKEUP_CYCLE_CNT"
#endif
#if (LPU_TIMER_US(HAL_CMU_26M_READY_TIMEOUT_US) < MCU_WAKEUP_CYCLE_CNT)
#error "HAL_CMU_26M_READY_TIMEOUT_US should >= MCU_WAKEUP_CYCLE_CNT"
#endif
#if (LPU_TIMER_US(HAL_CMU_26M_READY_TIMEOUT_US) < SENS_WAKEUP_CYCLE_CNT)
#error "HAL_CMU_26M_READY_TIMEOUT_US should >= SENS_WAKEUP_CYCLE_CNT"
#endif

#define HAL_CMU_PLL_LOCKED_TIMEOUT      US_TO_TICKS(HAL_CMU_PLL_LOCKED_TIMEOUT_US)
#define HAL_CMU_26M_READY_TIMEOUT       US_TO_TICKS(HAL_CMU_26M_READY_TIMEOUT_US)
#define HAL_CMU_LPU_EXTRA_TIMEOUT       US_TO_TICKS(HAL_CMU_LPU_EXTRA_TIMEOUT_US)

#ifdef CORE_SLEEP_POWER_DOWN
#define TIMER1_SEL_LOC                  BOOT_TEXT_SRAM_LOC
#else
#define TIMER1_SEL_LOC                  BOOT_TEXT_FLASH_LOC
#endif
#define AON_CMU_BT_RAMRUN_RAM_PG_AUTO       (0xF << 9)
#define AON_CMU_SENS_RAM_PG_ENABLE          (1 << 16)
#define AON_CMU_BT_RAM_SLEEP_ALLOW          (1 << 18)
#define AON_CMU_BT_RAM_BLK_PG_AUTO_SHIFT    24
#define AON_CMU_BT_RAM_BLK_PG_AUTO_MASK     (0xFF << AON_CMU_BT_RAM_BLK_PG_AUTO_SHIFT)
#define AON_CMU_BT_RAM_BLK_PG_AUTO(n)       BITFIELD_VAL(AON_CMU_BT_RAM_BLK_PG_AUTO, n)
#define AON_CMU_BT_RAM_LAST_8K_PG_AUTO      AON_CMU_BT_RAM_BLK_PG_AUTO(0x20)

enum CMU_USB_CLK_SRC_T {
    CMU_USB_CLK_SRC_OSC_24M_X2      = 0,
    CMU_USB_CLK_SRC_PLL             = 1,
};

enum CMU_DEBUG_REG_SEL_T {
    CMU_DEBUG_REG_SEL_MCU_PC        = 0,
    CMU_DEBUG_REG_SEL_MCU_LR        = 1,
    CMU_DEBUG_REG_SEL_MCU_SP        = 2,
    CMU_DEBUG_REG_SEL_CP_PC         = 3,
    CMU_DEBUG_REG_SEL_CP_LR         = 4,
    CMU_DEBUG_REG_SEL_CP_SP         = 5,
    CMU_DEBUG_REG_SEL_DEBUG         = 7,
};

enum CMU_I2S_MCLK_SEL_T {
    CMU_I2S_MCLK_SEL_MCU            = 0,
    CMU_I2S_MCLK_SEL_SENS           = 1,
    CMU_I2S_MCLK_SEL_CODEC          = 2,
};

enum CMU_DMA_REQ_T {
    CMU_DMA_REQ_CODEC_RX            = 0,
    CMU_DMA_REQ_CODEC_TX            = 1,
    CMU_DMA_REQ_PCM_RX              = 2,
    CMU_DMA_REQ_PCM_TX              = 3,
    CMU_DMA_REQ_I2S0_RX             = 4,
    CMU_DMA_REQ_I2S0_TX             = 5,
    CMU_DMA_REQ_FIR_RX              = 6,
    CMU_DMA_REQ_FIR_TX              = 7,
    CMU_DMA_REQ_SPDIF0_RX           = 8,
    CMU_DMA_REQ_SPDIF0_TX           = 9,
    CMU_DMA_REQ_RESERVED10          = 10,
    CMU_DMA_REQ_CODEC_TX2           = 11,
    CMU_DMA_REQ_BTDUMP              = 12,
    CMU_DMA_REQ_CODEC_MC            = 13,
    CMU_DMA_REQ_RESERVED14          = 14,
    CMU_DMA_REQ_RESERVED15          = 15,
    CMU_DMA_REQ_DSD_RX              = 16,
    CMU_DMA_REQ_DSD_TX              = 17,
    CMU_DMA_REQ_I2S1_RX             = 18,
    CMU_DMA_REQ_I2S1_TX             = 19,
    CMU_DMA_REQ_FLS0                = 20,
    CMU_DMA_REQ_SDMMC0              = 21,
    CMU_DMA_REQ_I2C0_RX             = 22,
    CMU_DMA_REQ_I2C0_TX             = 23,
    CMU_DMA_REQ_SPILCD0_RX          = 24,
    CMU_DMA_REQ_SPILCD0_TX          = 25,
    CMU_DMA_REQ_RESERVED26          = 26,
    CMU_DMA_REQ_RESERVED27          = 27,
    CMU_DMA_REQ_UART0_RX            = 28,
    CMU_DMA_REQ_UART0_TX            = 29,
    CMU_DMA_REQ_UART1_RX            = 30,
    CMU_DMA_REQ_UART1_TX            = 31,
    CMU_DMA_REQ_I2C1_RX             = 32,
    CMU_DMA_REQ_I2C1_TX             = 33,
    CMU_DMA_REQ_UART2_RX            = 34,
    CMU_DMA_REQ_UART2_TX            = 35,
    CMU_DMA_REQ_SPI_ITN_RX          = 36,
    CMU_DMA_REQ_SPI_ITN_TX          = 37,
    CMU_DMA_REQ_SDMMC1              = 38,
    CMU_DMA_REQ_FLS1                = 39,

    CMU_DMA_REQ_QTY,
    CMU_DMA_REQ_NULL                = CMU_DMA_REQ_QTY,
};

static struct AONCMU_T * const aoncmu = (struct AONCMU_T *)AON_CMU_BASE;

static inline void aocmu_reg_update_wait(void)
{
    // Make sure AOCMU (26M clock domain) write opertions finish before return
    aoncmu->CHIP_ID;
}

#ifndef CHIP_SUBSYS_SENS

static uint32_t cp_entry;

static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;

static struct AONSEC_T * const aonsec = (struct AONSEC_T *)AON_SEC_CTRL_BASE;

static struct SENSCMU_T * const senscmu = (struct SENSCMU_T *)SENS_CMU_BASE;

static struct BTCMU_T * const POSSIBLY_UNUSED btcmu = (struct BTCMU_T *)BT_CMU_BASE;

static uint16_t BOOT_BSS_LOC pll_user_map[HAL_CMU_PLL_QTY];
STATIC_ASSERT(HAL_CMU_PLL_USER_QTY <= sizeof(pll_user_map[0]) * 8, "Too many PLL users");

#ifdef DCDC_CLOCK_CONTROL
static uint8_t dcdc_user_map;
STATIC_ASSERT(HAL_CMU_DCDC_CLOCK_USER_QTY <= sizeof(dcdc_user_map) * 8, "Too many DCDC clock users");
#endif

#ifndef ROM_BUILD
static enum HAL_CMU_PLL_T BOOT_DATA_LOC sys_pll_sel = HAL_CMU_PLL_QTY;
#endif

#ifdef LOW_SYS_FREQ
static enum HAL_CMU_FREQ_T BOOT_BSS_LOC cmu_sys_freq;
#endif
static uint8_t BOOT_BSS_LOC force_sys_div;

#ifdef CHIP_ROLE_CP
static HAL_CMU_CP_SYS_SET_FREQ_CB_T cp_sys_set_freq_cb = NULL;
#endif

static bool anc_enabled;

static HAL_CMU_BT_TRIGGER_HANDLER_T bt_trig_hdlr[HAL_CMU_BT_TRIGGER_SRC_QTY];

#ifdef __AUDIO_RESAMPLE__
static bool aud_resample_en = true;
#endif

void hal_cmu_audio_resample_enable(void)
{
#ifdef __AUDIO_RESAMPLE__
    aud_resample_en = true;
#endif
}

void hal_cmu_audio_resample_disable(void)
{
#ifdef __AUDIO_RESAMPLE__
    aud_resample_en = false;
#endif
}

int hal_cmu_get_audio_resample_status(void)
{
#ifdef __AUDIO_RESAMPLE__
    return aud_resample_en;
#else
    return false;
#endif
}

void hal_cmu_anc_enable(enum HAL_CMU_ANC_CLK_USER_T user)
{
    anc_enabled = true;
}

void hal_cmu_anc_disable(enum HAL_CMU_ANC_CLK_USER_T user)
{
    anc_enabled = false;
}

int hal_cmu_anc_get_status(enum HAL_CMU_ANC_CLK_USER_T user)
{
    return anc_enabled;
}

int hal_cmu_clock_enable(enum HAL_CMU_MOD_ID_T id)
{
    if (id >= HAL_CMU_AON_MCU) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HCLK_ENABLE = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PCLK_ENABLE = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        cmu->OCLK_ENABLE = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->XCLK_ENABLE = (1 << (id - HAL_CMU_MOD_X_PSRAM));
    } else if (id < HAL_CMU_AON_O_WDT) {
        aoncmu->MOD_CLK_ENABLE = (1 << (id - HAL_CMU_AON_A_CMU));
        aocmu_reg_update_wait();
    } else {
        aoncmu->MOD_OCLK_ENABLE = (1 << (id - HAL_CMU_AON_O_WDT));
        aocmu_reg_update_wait();
    }

    return 0;
}

int hal_cmu_clock_disable(enum HAL_CMU_MOD_ID_T id)
{
    if (id >= HAL_CMU_AON_MCU) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HCLK_DISABLE = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PCLK_DISABLE = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        cmu->OCLK_DISABLE = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->XCLK_DISABLE = (1 << (id - HAL_CMU_MOD_X_PSRAM));
    } else if (id < HAL_CMU_AON_O_WDT) {
        aoncmu->MOD_CLK_DISABLE = (1 << (id - HAL_CMU_AON_A_CMU));
    } else {
        aoncmu->MOD_OCLK_DISABLE = (1 << (id - HAL_CMU_AON_O_WDT));
    }

    return 0;
}

enum HAL_CMU_CLK_STATUS_T hal_cmu_clock_get_status(enum HAL_CMU_MOD_ID_T id)
{
    uint32_t status;

    if (id >= HAL_CMU_AON_MCU) {
        return HAL_CMU_CLK_DISABLED;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        status = cmu->HCLK_ENABLE & (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        status = cmu->PCLK_ENABLE & (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        status = cmu->OCLK_ENABLE & (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        status = cmu->XCLK_DISABLE & (1 << (id - HAL_CMU_MOD_X_PSRAM));
    } else if (id < HAL_CMU_AON_O_WDT) {
        status = aoncmu->MOD_CLK_ENABLE & (1 << (id - HAL_CMU_AON_A_CMU));
    } else {
        status = aoncmu->MOD_OCLK_ENABLE & (1 << (id - HAL_CMU_AON_O_WDT));
    }

    return status ? HAL_CMU_CLK_ENABLED : HAL_CMU_CLK_DISABLED;
}

int hal_cmu_clock_set_mode(enum HAL_CMU_MOD_ID_T id, enum HAL_CMU_CLK_MODE_T mode)
{
    __IO uint32_t *reg;
    uint32_t val;
    uint32_t lock;

    if (id >= HAL_CMU_AON_MCU) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        reg = &cmu->HCLK_MODE;
        val = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        reg = &cmu->PCLK_MODE;
        val = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        reg = &cmu->OCLK_MODE;
        val = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        reg = &cmu->XCLK_MODE;
        val = (1 << (id - HAL_CMU_MOD_X_PSRAM));
    } else if (id < HAL_CMU_AON_O_WDT) {
        reg = &aoncmu->MOD_CLK_MODE;
        val = (1 << (id - HAL_CMU_AON_A_CMU));
    } else {
        reg = &aoncmu->MOD_OCLK_MODE;
        val = (1 << (id - HAL_CMU_AON_O_WDT));
    }

    lock = int_lock();
    if (mode == HAL_CMU_CLK_MANUAL) {
        *reg |= val;
    } else {
        *reg &= ~val;
    }
    int_unlock(lock);

    return 0;
}

enum HAL_CMU_CLK_MODE_T hal_cmu_clock_get_mode(enum HAL_CMU_MOD_ID_T id)
{
    uint32_t mode;

    if (id >= HAL_CMU_AON_MCU) {
        return HAL_CMU_CLK_AUTO;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        mode = cmu->HCLK_MODE & (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        mode = cmu->PCLK_MODE & (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        mode = cmu->OCLK_MODE & (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        mode = cmu->XCLK_MODE & (1 << (id - HAL_CMU_MOD_X_PSRAM));
    } else if (id < HAL_CMU_AON_O_WDT) {
        mode = aoncmu->MOD_CLK_MODE & (1 << (id - HAL_CMU_AON_A_CMU));
    } else {
        mode = aoncmu->MOD_OCLK_MODE & (1 << (id - HAL_CMU_AON_O_WDT));
    }

    return mode ? HAL_CMU_CLK_MANUAL : HAL_CMU_CLK_AUTO;
}

int hal_cmu_reset_set(enum HAL_CMU_MOD_ID_T id)
{
    if (id >= HAL_CMU_MOD_QTY) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HRESET_SET = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PRESET_SET = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        cmu->ORESET_SET = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->XRESET_SET = (1 << (id - HAL_CMU_MOD_X_PSRAM));
    } else if (id < HAL_CMU_AON_O_WDT) {
        aoncmu->RESET_SET = (1 << (id - HAL_CMU_AON_A_CMU));
    } else if (id < HAL_CMU_AON_MCU) {
        aoncmu->ORESET_SET = (1 << (id - HAL_CMU_AON_O_WDT));
    } else if (id < HAL_CMU_SENS_H_FLASH2) {
        aoncmu->GBL_RESET_SET = (1 << (id - HAL_CMU_AON_MCU));
    } else {
        if (id == HAL_CMU_SENS_H_FLASH2) {
            senscmu->HRESET_SET = SENS_HCLK_FLASH;
        } else {
            senscmu->ORESET_SET = SENS_OCLK_FLASH;
        }
    }

    return 0;
}

int hal_cmu_reset_clear(enum HAL_CMU_MOD_ID_T id)
{
    if (id >= HAL_CMU_MOD_QTY) {
        return 1;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HRESET_CLR = (1 << id);
        asm volatile("nop; nop;");
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PRESET_CLR = (1 << (id - HAL_CMU_MOD_P_CMU));
        asm volatile("nop; nop; nop; nop;");
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        cmu->ORESET_CLR = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->XRESET_CLR = (1 << (id - HAL_CMU_MOD_X_PSRAM));
        cmu->XRESET_CLR;
    } else if (id < HAL_CMU_AON_O_WDT) {
        aoncmu->RESET_CLR = (1 << (id - HAL_CMU_AON_A_CMU));
        aocmu_reg_update_wait();
    } else if (id < HAL_CMU_AON_MCU) {
        aoncmu->ORESET_CLR = (1 << (id - HAL_CMU_AON_O_WDT));
        aocmu_reg_update_wait();
    } else if (id < HAL_CMU_SENS_H_FLASH2) {
        aoncmu->GBL_RESET_CLR = (1 << (id - HAL_CMU_AON_MCU));
        aocmu_reg_update_wait();
    } else {
        if (id == HAL_CMU_SENS_H_FLASH2) {
            senscmu->HRESET_CLR = SENS_HCLK_FLASH;
        } else {
            senscmu->ORESET_CLR = SENS_OCLK_FLASH;
        }
        senscmu->ORESET_CLR;
    }

    return 0;
}

enum HAL_CMU_RST_STATUS_T hal_cmu_reset_get_status(enum HAL_CMU_MOD_ID_T id)
{
    uint32_t status;

    if (id >= HAL_CMU_MOD_QTY) {
        return HAL_CMU_RST_SET;
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        status = cmu->HRESET_SET & (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        status = cmu->PRESET_SET & (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        status = cmu->ORESET_SET & (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        status = cmu->XRESET_SET & (1 << (id - HAL_CMU_MOD_X_PSRAM));
    } else if (id < HAL_CMU_AON_O_WDT) {
        status = aoncmu->RESET_SET & (1 << (id - HAL_CMU_AON_A_CMU));
    } else if (id < HAL_CMU_AON_MCU) {
        status = aoncmu->ORESET_SET & (1 << (id - HAL_CMU_AON_O_WDT));
    } else if (id < HAL_CMU_SENS_H_FLASH2) {
        status = aoncmu->GBL_RESET_SET & (1 << (id - HAL_CMU_AON_MCU));
    } else {
        if (id == HAL_CMU_SENS_H_FLASH2) {
            status = senscmu->HRESET_SET & SENS_HCLK_FLASH;
        } else {
            status = senscmu->ORESET_SET & SENS_OCLK_FLASH;
        }
    }

    return status ? HAL_CMU_RST_CLR : HAL_CMU_RST_SET;
}

int hal_cmu_reset_pulse(enum HAL_CMU_MOD_ID_T id)
{
    volatile int i;

    if (id >= HAL_CMU_MOD_QTY) {
        return 1;
    }

    if (hal_cmu_reset_get_status(id) == HAL_CMU_RST_SET) {
        return hal_cmu_reset_clear(id);
    }

    if (id < HAL_CMU_MOD_P_CMU) {
        cmu->HRESET_PULSE = (1 << id);
    } else if (id < HAL_CMU_MOD_O_SLEEP) {
        cmu->PRESET_PULSE = (1 << (id - HAL_CMU_MOD_P_CMU));
    } else if (id < HAL_CMU_MOD_X_PSRAM) {
        cmu->ORESET_PULSE = (1 << (id - HAL_CMU_MOD_O_SLEEP));
    } else if (id < HAL_CMU_AON_A_CMU) {
        cmu->XRESET_PULSE = (1 << (id - HAL_CMU_MOD_X_PSRAM));
    } else if (id < HAL_CMU_AON_O_WDT) {
        aoncmu->RESET_PULSE = (1 << (id - HAL_CMU_AON_A_CMU));
        aoncmu->CHIP_ID;
        aoncmu->CHIP_ID;
        aoncmu->CHIP_ID;
    } else if (id < HAL_CMU_AON_MCU) {
        aoncmu->ORESET_PULSE = (1 << (id - HAL_CMU_AON_O_WDT));
        // Total 3 CLK-26M cycles needed
        // AOCMU runs in 26M clock domain and its read operations consume at least 1 26M-clock cycle.
        // (Whereas its write operations will finish at 1 HCLK cycle -- finish once in async bridge fifo)
        aoncmu->CHIP_ID;
        aoncmu->CHIP_ID;
        aoncmu->CHIP_ID;
    } else if (id < HAL_CMU_SENS_H_FLASH2) {
        aoncmu->GBL_RESET_PULSE = (1 << (id - HAL_CMU_AON_MCU));
        // Total 3 CLK-26M cycles needed
        // AOCMU runs in 26M clock domain and its read operations consume at least 1 26M-clock cycle.
        // (Whereas its write operations will finish at 1 HCLK cycle -- finish once in async bridge fifo)
        aoncmu->CHIP_ID;
        aoncmu->CHIP_ID;
        aoncmu->CHIP_ID;
    } else {
        if (id == HAL_CMU_SENS_H_FLASH2) {
            senscmu->HRESET_PULSE = SENS_HCLK_FLASH;
        } else {
            senscmu->ORESET_PULSE = SENS_OCLK_FLASH;
        }
        senscmu->ORESET_CLR;
        senscmu->ORESET_CLR;
        senscmu->ORESET_CLR;
    }
    // Delay 5+ PCLK cycles (10+ HCLK cycles)
    for (i = 0; i < 3; i++);

    return 0;
}

int hal_cmu_timer_set_div(enum HAL_CMU_TIMER_ID_T id, uint32_t div)
{
    uint32_t lock;

    if (div < 1) {
        return 1;
    }

    div -= 1;
    if ((div & (CMU_CFG_DIV_TIMER00_MASK >> CMU_CFG_DIV_TIMER00_SHIFT)) != div) {
        return 1;
    }

    lock = int_lock();
    if (id == HAL_CMU_TIMER_ID_00) {
        cmu->TIMER0_CLK = SET_BITFIELD(cmu->TIMER0_CLK, CMU_CFG_DIV_TIMER00, div);
        aoncmu->TIMER0_CLK = SET_BITFIELD(aoncmu->TIMER0_CLK, AON_CMU_CFG_DIV_TIMER00, div);
    } else if (id == HAL_CMU_TIMER_ID_01) {
        cmu->TIMER0_CLK = SET_BITFIELD(cmu->TIMER0_CLK, CMU_CFG_DIV_TIMER01, div);
        aoncmu->TIMER0_CLK = SET_BITFIELD(aoncmu->TIMER0_CLK, AON_CMU_CFG_DIV_TIMER01, div);
    } else if (id == HAL_CMU_TIMER_ID_10) {
        cmu->TIMER1_CLK = SET_BITFIELD(cmu->TIMER1_CLK, CMU_CFG_DIV_TIMER10, div);
    } else if (id == HAL_CMU_TIMER_ID_11) {
        cmu->TIMER1_CLK = SET_BITFIELD(cmu->TIMER1_CLK, CMU_CFG_DIV_TIMER11, div);
    } else if (id == HAL_CMU_TIMER_ID_20) {
        cmu->TIMER2_CLK = SET_BITFIELD(cmu->TIMER2_CLK, CMU_CFG_DIV_TIMER20, div);
    } else if (id == HAL_CMU_TIMER_ID_21) {
        cmu->TIMER2_CLK = SET_BITFIELD(cmu->TIMER2_CLK, CMU_CFG_DIV_TIMER21, div);
    }
    int_unlock(lock);

    return 0;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_timer0_select_fast(void)
{
    uint32_t lock;

    lock = int_lock();
    // 6.5M
#if (TIMER0_BASE == MCU_TIMER0_BASE)
    cmu->PERIPH_CLK |= (1 << CMU_SEL_TIMER_FAST_SHIFT);
#else
    // AON Timer
    aoncmu->CLK_OUT |= (1 << AON_CMU_SEL_TIMER_FAST_SHIFT);
#endif
    int_unlock(lock);
}

void BOOT_TEXT_FLASH_LOC hal_cmu_timer0_select_slow(void)
{
    uint32_t lock;

    lock = int_lock();
    // 16K
#if (TIMER0_BASE == MCU_TIMER0_BASE)
    cmu->PERIPH_CLK &= ~(1 << CMU_SEL_TIMER_FAST_SHIFT);
#else
    // AON Timer
    aoncmu->CLK_OUT &= ~(1 << AON_CMU_SEL_TIMER_FAST_SHIFT);
#endif
    int_unlock(lock);
}

void TIMER1_SEL_LOC hal_cmu_timer1_select_fast(void)
{
    uint32_t lock;

    lock = int_lock();
    // 6.5M
    cmu->PERIPH_CLK |= (1 << (CMU_SEL_TIMER_FAST_SHIFT + 1));
    int_unlock(lock);
}

void TIMER1_SEL_LOC hal_cmu_timer1_select_slow(void)
{
    uint32_t lock;

    lock = int_lock();
    // 16K
    cmu->PERIPH_CLK &= ~(1 << (CMU_SEL_TIMER_FAST_SHIFT + 1));
    int_unlock(lock);
}

void TIMER1_SEL_LOC hal_cmu_timer2_select_fast(void)
{
    uint32_t lock;

    lock = int_lock();
    // 6.5M
#if (TIMER2_BASE == MCU_TIMER0_BASE)
    cmu->PERIPH_CLK |= (1 << CMU_SEL_TIMER_FAST_SHIFT);
#elif (TIMER2_BASE == AON_TIMER0_BASE)
    // AON Timer
    aoncmu->CLK_OUT |= (1 << AON_CMU_SEL_TIMER_FAST_SHIFT);
#else
    cmu->PERIPH_CLK |= (1 << (CMU_SEL_TIMER_FAST_SHIFT + 2));
#endif
    int_unlock(lock);
}

void TIMER1_SEL_LOC hal_cmu_timer2_select_slow(void)
{
    uint32_t lock;

    lock = int_lock();
    // 16K
#if (TIMER2_BASE == MCU_TIMER0_BASE)
    cmu->PERIPH_CLK &= ~(1 << CMU_SEL_TIMER_FAST_SHIFT);
#elif (TIMER2_BASE == AON_TIMER0_BASE)
    // AON Timer
    aoncmu->CLK_OUT &= ~(1 << AON_CMU_SEL_TIMER_FAST_SHIFT);
#else
    cmu->PERIPH_CLK &= ~(1 << (CMU_SEL_TIMER_FAST_SHIFT + 2));
#endif
    int_unlock(lock);
}

int hal_cmu_sdmmc0_set_pll_div(uint32_t div)
{
    uint32_t lock;
    if (div < 2) {
        return 1;
    }
    div -= 2;
    if ((div & (CMU_CFG_DIV_SDMMC_MASK >> CMU_CFG_DIV_SDMMC_SHIFT)) != div) {
        return 1;
    }
    lock = int_lock();
    cmu->ADMA_CH15_REQ = SET_BITFIELD(cmu->ADMA_CH15_REQ, CMU_CFG_DIV_SDMMC, div) |
        CMU_SEL_OSCX2_SDMMC | CMU_SEL_PLL_SDMMC | CMU_EN_PLL_SDMMC;
    int_unlock(lock);
    return 0;
}

#if defined(OSC_26M_X4_AUD2BB) && !defined(FREQ_78M_USE_PLL)
#define SYS_78M_CFG(F) \
    { \
        enable = CMU_SEL_OSCX4_ ##F## _ENABLE; \
        disable = CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    }
#else
#define SYS_78M_CFG(F) \
    { \
        enable = CMU_SEL_PLL_ ##F## _ENABLE | CMU_RSTN_DIV_ ##F## _ENABLE; \
        disable = CMU_BYPASS_DIV_ ##F## _DISABLE; \
        div = 1; \
    }
#endif

#ifdef OSC_26M_X4_AUD2BB
#define SYS_104M_CFG(F) \
    { \
        enable = CMU_SEL_OSCX4_ ##F## _ENABLE; \
        disable = CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    }
#else
#define SYS_104M_CFG(F) \
    { \
        enable = CMU_SEL_PLL_ ##F## _ENABLE | CMU_RSTN_DIV_ ##F## _ENABLE; \
        disable = CMU_BYPASS_DIV_ ##F## _DISABLE; \
        div = 0; \
    }
#endif

#define SYS_SET_FREQ_FUNC(f, F, C) \
int hal_cmu_ ##f## _set_freq(enum HAL_CMU_FREQ_T freq) \
{ \
    uint32_t lock; \
    uint32_t enable; \
    uint32_t disable; \
    bool clk_en; \
    int div = -1; \
    if (freq >= HAL_CMU_FREQ_QTY) { \
        return 1; \
    } \
    if (freq == HAL_CMU_FREQ_32K || freq == HAL_CMU_FREQ_26M) { \
        enable = 0; \
        disable = CMU_SEL_OSCX2_ ##F## _DISABLE | CMU_SEL_OSCX4_ ##F## _DISABLE | \
            CMU_SEL_PLL_ ##F## _DISABLE | CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    } else if (freq == HAL_CMU_FREQ_52M) { \
        enable = CMU_SEL_OSCX2_ ##F## _ENABLE; \
        disable = CMU_SEL_OSCX4_ ##F## _DISABLE | CMU_SEL_PLL_ ##F## _DISABLE | \
            CMU_RSTN_DIV_ ##F## _DISABLE | CMU_BYPASS_DIV_ ##F## _DISABLE; \
    } else if (freq == HAL_CMU_FREQ_78M) { \
        SYS_78M_CFG(F); \
    } else if (freq == HAL_CMU_FREQ_104M) { \
        SYS_104M_CFG(F); \
    } else { \
        enable = CMU_SEL_PLL_ ##F## _ENABLE | CMU_BYPASS_DIV_ ##F## _ENABLE; \
        disable = CMU_RSTN_DIV_ ##F## _DISABLE; \
    } \
    clk_en = !!(cmu->OCLK_DISABLE & SYS_OCLK_ ## C); \
    if (clk_en) { \
        cmu->OCLK_DISABLE = SYS_OCLK_ ## C; \
        cmu->HCLK_DISABLE = SYS_HCLK_ ## C; \
        aocmu_reg_update_wait(); \
        aocmu_reg_update_wait(); \
    } \
    if (div >= 0) { \
        lock = int_lock(); \
        cmu->SYS_DIV = SET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_ ##F, div); \
        int_unlock(lock); \
    } \
    cmu->SYS_CLK_ENABLE = enable; \
    cmu->SYS_CLK_DISABLE = disable; \
    if (clk_en) { \
        cmu->HCLK_ENABLE = SYS_HCLK_ ## C; \
        cmu->OCLK_ENABLE = SYS_OCLK_ ## C; \
    } \
    return 0; \
}

#ifdef ALT_BOOT_FLASH
BOOT_TEXT_SRAM_LOC SYS_SET_FREQ_FUNC(flash, FLS1, FLASH1);
SYS_SET_FREQ_FUNC(flash1, FLS0, FLASH);
#else
BOOT_TEXT_SRAM_LOC SYS_SET_FREQ_FUNC(flash, FLS0, FLASH);
SYS_SET_FREQ_FUNC(flash1, FLS1, FLASH1);
#endif

int hal_cmu_flash2_set_freq(enum HAL_CMU_FREQ_T freq)
{
    uint32_t lock;
    uint32_t mask;
    uint32_t val;
    bool clk_en;
    int div = 0;

    if (freq >= HAL_CMU_FREQ_QTY) {
        return 1;
    }

    mask = SENS_CMU_RSTN_DIV_FLS | SENS_CMU_CFG_DIV_FLS_MASK |
        SENS_CMU_BYPASS_DIV_FLS | SENS_CMU_SEL_FLS_SRC |
        SENS_CMU_SEL_FLS_OSCX2 | SENS_CMU_SEL_FLS_OSCX4;

    if (freq == HAL_CMU_FREQ_32K || freq == HAL_CMU_FREQ_26M) {
        val = 0;
    } else if (freq == HAL_CMU_FREQ_52M) {
        val = SENS_CMU_SEL_FLS_OSCX2;
    } else if (freq == HAL_CMU_FREQ_78M) {
#if defined(OSC_26M_X4_AUD2BB) && !defined(FREQ_78M_USE_PLL)
        val = SENS_CMU_SEL_FLS_OSCX4;
#else
        val = SENS_CMU_RSTN_DIV_FLS | SENS_CMU_SEL_FLS_SRC;
        div = 1;
#endif
    } else if (freq == HAL_CMU_FREQ_104M) {
#ifdef OSC_26M_X4_AUD2BB
        val = SENS_CMU_SEL_FLS_OSCX4;
#else
        val = SENS_CMU_RSTN_DIV_FLS | SENS_CMU_SEL_FLS_SRC;
        div = 0;
#endif
    } else {
        val = SENS_CMU_BYPASS_DIV_FLS | SENS_CMU_SEL_FLS_SRC;
    }

    val |= SENS_CMU_CFG_DIV_FLS(div);

    lock = int_lock();
    clk_en = !!(senscmu->OCLK_DISABLE & SENS_OCLK_FLASH);
    if (clk_en) {
        senscmu->OCLK_DISABLE = SENS_OCLK_FLASH;
        senscmu->HCLK_DISABLE = SENS_HCLK_FLASH;
        aocmu_reg_update_wait();
        aocmu_reg_update_wait();
    }
    senscmu->ADMA_CH15_REQ = (senscmu->ADMA_CH15_REQ & ~mask) | val;
    if (clk_en) {
        senscmu->HCLK_ENABLE = SENS_HCLK_FLASH;
        senscmu->OCLK_ENABLE = SENS_OCLK_FLASH;
    }
    int_unlock(lock);

    return 0;
}

#ifdef CHIP_ROLE_CP
void hal_cmu_cp_sys_set_freq_register(HAL_CMU_CP_SYS_SET_FREQ_CB_T cb)
{
    cp_sys_set_freq_cb = cb;
}
#endif

#ifdef LOW_SYS_FREQ
#ifdef LOW_SYS_FREQ_6P5M
int hal_cmu_fast_timer_offline(void)
{
    return (cmu_sys_freq == HAL_CMU_FREQ_6P5M);
}
#endif
#endif

void hal_cmu_force_sys_pll_div(uint32_t div)
{
    force_sys_div = div;
}

int hal_cmu_sys_set_freq(enum HAL_CMU_FREQ_T freq)
{
#ifdef CHIP_ROLE_CP
    if (cp_sys_set_freq_cb) {
        cp_sys_set_freq_cb(freq);
    }

    return 0;
#else
    uint32_t enable;
    uint32_t disable;
    int div;
    uint32_t lock;

    if (freq >= HAL_CMU_FREQ_QTY) {
        return 1;
    }

#ifdef LOW_SYS_FREQ
    cmu_sys_freq = freq;
#endif

    div = -1;

    switch (freq) {
    case HAL_CMU_FREQ_32K:
        enable = CMU_SEL_OSCX2_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE | CMU_SEL_OSC_2_SYS_DISABLE | CMU_SEL_OSC_4_SYS_DISABLE |
            CMU_SEL_SLOW_SYS_DISABLE | CMU_SEL_FAST_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE;
        break;
    case HAL_CMU_FREQ_6P5M:
#if defined(LOW_SYS_FREQ) && defined(LOW_SYS_FREQ_6P5M)
        enable = CMU_SEL_OSC_4_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE | CMU_SEL_SLOW_SYS_DISABLE |
            CMU_SEL_FAST_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE;
        break;
#endif
    case HAL_CMU_FREQ_13M:
#ifdef LOW_SYS_FREQ
        enable = CMU_SEL_OSC_2_SYS_ENABLE | CMU_SEL_SLOW_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE |
            CMU_SEL_FAST_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE;
        break;
#endif
    case HAL_CMU_FREQ_26M:
        enable = CMU_SEL_SLOW_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE | CMU_SEL_OSC_2_SYS_DISABLE |
            CMU_SEL_FAST_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE;
        break;
    case HAL_CMU_FREQ_52M:
        enable = CMU_SEL_SLOW_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE | CMU_SEL_FAST_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE;
        break;
    case HAL_CMU_FREQ_78M:
#if !defined(OSC_26M_X4_AUD2BB) || defined(FREQ_78M_USE_PLL)
        enable = CMU_SEL_SLOW_SYS_ENABLE | CMU_RSTN_DIV_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE | CMU_SEL_PLL_SYS_ENABLE;
        disable = CMU_BYPASS_DIV_SYS_DISABLE;
        div = 1;
        break;
#endif
    case HAL_CMU_FREQ_104M:
#ifdef OSC_26M_X4_AUD2BB
        enable = CMU_SEL_SLOW_SYS_ENABLE | CMU_SEL_FAST_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE | CMU_SEL_OSCX2_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE;
        break;
#else
        enable = CMU_SEL_SLOW_SYS_ENABLE | CMU_RSTN_DIV_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE | CMU_SEL_PLL_SYS_ENABLE;
        disable = CMU_BYPASS_DIV_SYS_DISABLE;
        div = 0;
        break;
#endif
    case HAL_CMU_FREQ_208M:
    default:
        enable = CMU_SEL_SLOW_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE | CMU_BYPASS_DIV_SYS_ENABLE | CMU_SEL_PLL_SYS_ENABLE;
        disable = CMU_RSTN_DIV_SYS_DISABLE;
        break;
    };

    uint8_t cur_sys_div = force_sys_div;
    if (cur_sys_div) {
        if (cur_sys_div == 1) {
            enable = (enable & ~CMU_RSTN_DIV_SYS_ENABLE) | CMU_BYPASS_DIV_SYS_ENABLE;
            disable = (disable & ~CMU_BYPASS_DIV_SYS_DISABLE) | CMU_RSTN_DIV_SYS_DISABLE;
        } else {
            div = cur_sys_div - 2;
            enable = (enable & ~CMU_BYPASS_DIV_SYS_ENABLE) | CMU_RSTN_DIV_SYS_ENABLE;
            disable = (disable & ~CMU_RSTN_DIV_SYS_DISABLE) | CMU_BYPASS_DIV_SYS_DISABLE;
        }
    }

    if (div >= 0) {
        lock = int_lock();
        cmu->SYS_DIV = SET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_SYS, div);
        int_unlock(lock);
    }

    if (enable & CMU_SEL_PLL_SYS_ENABLE) {
        cmu->SYS_CLK_ENABLE = CMU_RSTN_DIV_SYS_ENABLE;
        if (enable & CMU_BYPASS_DIV_SYS_ENABLE) {
            cmu->SYS_CLK_ENABLE = CMU_BYPASS_DIV_SYS_ENABLE;
        } else {
            cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_SYS_DISABLE;
        }
    }
    cmu->SYS_CLK_ENABLE = enable;
    if (enable & CMU_SEL_PLL_SYS_ENABLE) {
        cmu->SYS_CLK_DISABLE = disable;
    } else {
        cmu->SYS_CLK_DISABLE = disable & ~(CMU_RSTN_DIV_SYS_DISABLE | CMU_BYPASS_DIV_SYS_DISABLE);
        cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_SYS_DISABLE;
        cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_SYS_DISABLE;
    }

    return 0;
#endif
}

int hal_cmu_mem_set_freq(enum HAL_CMU_FREQ_T freq)
{
    uint32_t enable;
    uint32_t disable;
    bool clk_en;
    uint32_t prev_en;
    int div;
    uint32_t lock;

    if (freq >= HAL_CMU_FREQ_QTY) {
        return 1;
    }
    div = -1;
    if (freq <= HAL_CMU_FREQ_26M) {
        enable = 0;
        disable = CMU_SEL_OSCX2_PSRAM_DISABLE | CMU_SEL_OSCX4_PSRAM_DISABLE |
            CMU_SEL_PLL_PSRAM_DISABLE;
    } else if (freq <= HAL_CMU_FREQ_52M) {
        enable = CMU_SEL_OSCX2_PSRAM_ENABLE;
        disable = CMU_SEL_OSCX4_PSRAM_DISABLE | CMU_SEL_PLL_PSRAM_DISABLE;
    } else if (freq <= HAL_CMU_FREQ_104M) {
#ifdef OSC_26M_X4_AUD2BB
        enable = CMU_SEL_OSCX4_PSRAM_ENABLE;
        disable = CMU_SEL_PLL_PSRAM_DISABLE;
#else
        enable = CMU_SEL_PLL_PSRAM_ENABLE | CMU_RSTN_DIV_PSRAM_ENABLE;
        disable = CMU_BYPASS_DIV_PSRAM_DISABLE;
        if (freq <= HAL_CMU_FREQ_78M) {
            div = 1;
        } else {
            div = 0;
        }
#endif
    } else {
        enable = CMU_SEL_PLL_PSRAM_ENABLE | CMU_BYPASS_DIV_PSRAM_ENABLE;
        disable = CMU_RSTN_DIV_PSRAM_ENABLE;
    }
    clk_en = !!(cmu->OCLK_DISABLE & SYS_OCLK_PSRAM);
    if (clk_en) {
        cmu->OCLK_DISABLE = SYS_OCLK_PSRAM;
        cmu->XCLK_DISABLE = SYS_XCLK_PSRAM;
    }
    prev_en = cmu->SYS_CLK_ENABLE;
    if ((enable & CMU_SEL_PLL_PSRAM_ENABLE) && (prev_en & CMU_SEL_PLL_PSRAM_ENABLE) == 0) {
        hal_cmu_pll_enable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_PSRAM);
    }
    if (div >= 0) {
        lock = int_lock();
        cmu->SYS_DIV = SET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_PSRAM, div);
        int_unlock(lock);
    }
    if (enable & CMU_SEL_PLL_PSRAM_ENABLE) {
        cmu->SYS_CLK_ENABLE = CMU_RSTN_DIV_PSRAM_ENABLE;
        if (enable & CMU_BYPASS_DIV_PSRAM_ENABLE) {
            cmu->SYS_CLK_ENABLE = CMU_BYPASS_DIV_PSRAM_ENABLE;
        } else {
            cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_PSRAM_DISABLE;
        }
    }
    cmu->SYS_CLK_ENABLE = enable;
    if (enable & CMU_SEL_PLL_PSRAM_ENABLE) {
        cmu->SYS_CLK_DISABLE = disable;
    } else {
        cmu->SYS_CLK_DISABLE = disable & ~(CMU_RSTN_DIV_PSRAM_DISABLE | CMU_BYPASS_DIV_PSRAM_DISABLE);
        cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_PSRAM_DISABLE;
        cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_PSRAM_DISABLE;
    }
    if (clk_en) {
        cmu->XCLK_ENABLE = SYS_XCLK_PSRAM;
        cmu->OCLK_ENABLE = SYS_OCLK_PSRAM;
    }
    if ((enable & CMU_SEL_PLL_PSRAM_ENABLE) == 0 && (prev_en & CMU_SEL_PLL_PSRAM_ENABLE)) {
        hal_cmu_pll_disable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_PSRAM);
    }
    return 0;
}

enum HAL_CMU_FREQ_T BOOT_TEXT_SRAM_LOC hal_cmu_sys_get_freq(void)
{
    uint32_t sys_clk;
    uint32_t div;

    sys_clk = cmu->SYS_CLK_ENABLE;

    if (sys_clk & CMU_SEL_PLL_SYS_ENABLE) {
        if (sys_clk & CMU_BYPASS_DIV_SYS_ENABLE) {
            return HAL_CMU_FREQ_208M;
        } else {
            div = GET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_SYS);
            if (div == 0) {
                return HAL_CMU_FREQ_104M;
            } else if (div == 1) {
                // (div == 1): 69M
                return HAL_CMU_FREQ_78M;
            } else {
                // (div == 2): 52M
                // (div == 3): 42M
                return HAL_CMU_FREQ_52M;
            }
        }
    } else if ((sys_clk & (CMU_SEL_FAST_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE)) ==
            (CMU_SEL_FAST_SYS_ENABLE)) {
        return HAL_CMU_FREQ_104M;
    } else if ((sys_clk & (CMU_SEL_FAST_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE)) ==
            (CMU_SEL_FAST_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE)) {
        return HAL_CMU_FREQ_52M;
    } else if (sys_clk & CMU_SEL_SLOW_SYS_ENABLE) {
        return HAL_CMU_FREQ_26M;
    } else {
        return HAL_CMU_FREQ_32K;
    }
}

int BOOT_TEXT_FLASH_LOC hal_cmu_flash_select_pll(enum HAL_CMU_PLL_T pll)
{
    if (pll == HAL_CMU_PLL_BB) {
        cmu->SYS_CLK_ENABLE = CMU_SEL_PLL_FLS0_FAST_ENABLE;
    } else {
        cmu->SYS_CLK_DISABLE = CMU_SEL_PLL_FLS0_FAST_DISABLE;
    }
    return 0;
}

int BOOT_TEXT_FLASH_LOC hal_cmu_flash1_select_pll(enum HAL_CMU_PLL_T pll)
{
    if (pll == HAL_CMU_PLL_BB) {
        cmu->SYS_CLK_ENABLE = CMU_SEL_PLL_FLS1_FAST_ENABLE;
    } else {
        cmu->SYS_CLK_DISABLE = CMU_SEL_PLL_FLS1_FAST_DISABLE;
    }
    return 0;
}

int BOOT_TEXT_FLASH_LOC hal_cmu_flash2_select_pll(enum HAL_CMU_PLL_T pll)
{
    if (pll == HAL_CMU_PLL_BB) {
        senscmu->ADMA_CH15_REQ &= ~SENS_CMU_SEL_PLL_FLS;
    } else {
        senscmu->ADMA_CH15_REQ |= SENS_CMU_SEL_PLL_FLS;
    }
    return 0;
}

int hal_cmu_mem_select_pll(enum HAL_CMU_PLL_T pll)
{
    return 0;
}

int BOOT_TEXT_FLASH_LOC hal_cmu_sys_select_pll(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint8_t sel;

    if (pll >= HAL_CMU_PLL_QTY) {
        return 1;
    }

    // 0:bbpll, 1:dsipll, 2:audpll, 3:usbpll
    if (pll == HAL_CMU_PLL_BB) {
        sel = 0;
    } else if (pll == HAL_CMU_PLL_DSI) {
        sel = 1;
    } else if (pll == HAL_CMU_PLL_USB) {
        sel = 3;
    } else {
        return 1;
    }

    lock = int_lock();
#ifndef ROM_BUILD
    sys_pll_sel = pll;
#endif
    // For PLL clock selection
    cmu->UART_CLK = SET_BITFIELD(cmu->UART_CLK, CMU_SEL_PLL_SOURCE, sel);
    int_unlock(lock);

    return 0;
}

int BOOT_TEXT_FLASH_LOC hal_cmu_audio_select_pll(enum HAL_CMU_PLL_T pll)
{
    uint32_t lock;
    uint8_t sel;

    if (pll == HAL_CMU_PLL_AUD) {
        sel = 0;
    } else if (pll == HAL_CMU_PLL_BB) {
        sel = 2;
    } else if (pll == HAL_CMU_PLL_USB) {
        sel = 3;
    } else {
        return 1;
    }

    lock = int_lock();
    cmu->I2S0_CLK = SET_BITFIELD(cmu->I2S0_CLK, CMU_SEL_PLL_AUD, sel);
    int_unlock(lock);

    return 0;
}

int hal_cmu_get_pll_status(enum HAL_CMU_PLL_T pll)
{
    if (0) {
#ifndef ROM_BUILD
    } else if (pll == sys_pll_sel) {
        return !!(cmu->SYS_CLK_ENABLE & CMU_EN_PLL_ENABLE);
#endif
    } else if (pll == HAL_CMU_PLL_BB) {
        return !!(aoncmu->TOP_CLK_ENABLE & AON_CMU_EN_CLK_TOP_PLLBB_ENABLE);
    } else if (pll == HAL_CMU_PLL_DSI) {
        return !!(aoncmu->TOP_CLK_ENABLE & AON_CMU_EN_CLK_TOP_PLLDSI_ENABLE);
    } else if (pll == HAL_CMU_PLL_AUD) {
        return !!(aoncmu->TOP_CLK_ENABLE & AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE);
    } else if (pll == HAL_CMU_PLL_USB) {
        return !!(aoncmu->TOP_CLK_ENABLE & AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE);
    }

    return 0;
}

int hal_cmu_pll_enable(enum HAL_CMU_PLL_T pll, enum HAL_CMU_PLL_USER_T user)
{
    uint32_t pu_val;
    uint32_t en_val;
    uint32_t en_val1;
    uint32_t check;
    uint32_t lock;
    uint32_t start;
    uint32_t timeout;

    if (pll >= HAL_CMU_PLL_QTY) {
        return 1;
    }
    if (user >= HAL_CMU_PLL_USER_QTY && user != HAL_CMU_PLL_USER_ALL) {
        return 2;
    }

    if (pll == HAL_CMU_PLL_BB) {
        pu_val = AON_CMU_PU_PLLBB_ENABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLBB_ENABLE;
        en_val1 = AON_CMU_EN_CLK_PLLBB_MCU_ENABLE;
        check = AON_CMU_LOCK_PLLBB;
    } else if (pll == HAL_CMU_PLL_DSI) {
        pu_val = AON_CMU_PU_PLLDSI_ENABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLDSI_ENABLE;
        en_val1 = AON_CMU_EN_CLK_PLLDSI_MCU_ENABLE;
        check = AON_CMU_LOCK_PLLDSI;
    } else if (pll == HAL_CMU_PLL_AUD) {
        pu_val = AON_CMU_PU_PLLAUD_ENABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE;
        en_val1 = AON_CMU_EN_CLK_PLLAUD_MCU_ENABLE;
        check = AON_CMU_LOCK_PLLAUD;
    } else {
        pu_val = AON_CMU_PU_PLLUSB_ENABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE;
        if (user == HAL_CMU_PLL_USER_PSRAM) {
            en_val1 = AON_CMU_EN_CLK_PLLUSB_PSRAM_ENABLE;
        } else {
            en_val1 = AON_CMU_EN_CLK_PLLUSB_MCU_ENABLE;
        }
        check = AON_CMU_LOCK_PLLUSB;
    }

    lock = int_lock();
    if (pll_user_map[pll] == 0 || user == HAL_CMU_PLL_USER_ALL) {
#ifndef ROM_BUILD
        if (pll == sys_pll_sel) {
            cmu->SYS_CLK_ENABLE = CMU_PU_PLL_ENABLE;
        } else
#endif
        {
            aoncmu->TOP_CLK_ENABLE = pu_val;
        }
        // Wait at least 10us for clock ready
        hal_sys_timer_delay_us(20);
    } else {
        check = 0;
    }
    if (user < HAL_CMU_PLL_USER_QTY) {
        pll_user_map[pll] |= (1 << user);
    }
    int_unlock(lock);

    start = hal_sys_timer_get();
    timeout = HAL_CMU_PLL_LOCKED_TIMEOUT;
    do {
        if (check) {
            if (aoncmu->AON_CLK & check) {
                //break;
            }
        } else {
#ifndef ROM_BUILD
            if (pll == sys_pll_sel) {
                if (cmu->SYS_CLK_ENABLE & CMU_EN_PLL_ENABLE) {
                    break;
                }
            } else
#endif
            {
                if (aoncmu->TOP_CLK_ENABLE & en_val) {
                    break;
                }
            }
        }
    } while ((hal_sys_timer_get() - start) < timeout);

#ifndef ROM_BUILD
    if (pll == sys_pll_sel) {
        cmu->SYS_CLK_ENABLE = CMU_EN_PLL_ENABLE;
    } else
#endif
    {
        aoncmu->TOP_CLK_ENABLE = en_val;
    }
    aoncmu->TOP_CLK1_ENABLE = en_val1;

    return (aoncmu->AON_CLK & check) ? 0 : 2;
}

int hal_cmu_pll_disable(enum HAL_CMU_PLL_T pll, enum HAL_CMU_PLL_USER_T user)
{
    uint32_t pu_val;
    uint32_t en_val;
    uint32_t en_val1;
    uint32_t lock;

    if (pll >= HAL_CMU_PLL_QTY) {
        return 1;
    }
    if (user >= HAL_CMU_PLL_USER_QTY && user != HAL_CMU_PLL_USER_ALL) {
        return 2;
    }

    if (pll == HAL_CMU_PLL_BB) {
        pu_val = AON_CMU_PU_PLLBB_DISABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLBB_DISABLE;
        en_val1 = AON_CMU_EN_CLK_PLLBB_MCU_DISABLE;
    } else if (pll == HAL_CMU_PLL_DSI) {
        pu_val = AON_CMU_PU_PLLDSI_DISABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLDSI_DISABLE;
        en_val1 = AON_CMU_EN_CLK_PLLDSI_MCU_DISABLE;
    } else if (pll == HAL_CMU_PLL_AUD) {
        pu_val = AON_CMU_PU_PLLAUD_DISABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLAUD_DISABLE;
        en_val1 = AON_CMU_EN_CLK_PLLAUD_MCU_DISABLE;
    } else {
        pu_val = AON_CMU_PU_PLLUSB_DISABLE;
        en_val = AON_CMU_EN_CLK_TOP_PLLUSB_DISABLE;
        if (user == HAL_CMU_PLL_USER_PSRAM) {
            en_val1 = AON_CMU_EN_CLK_PLLUSB_PSRAM_DISABLE;
        } else {
            en_val1 = AON_CMU_EN_CLK_PLLUSB_MCU_DISABLE;
        }
    }

    lock = int_lock();
    if (user < HAL_CMU_PLL_USER_ALL) {
        pll_user_map[pll] &= ~(1 << user);
    }
    if (pll_user_map[pll] == 0 || user == HAL_CMU_PLL_USER_ALL) {
        aoncmu->TOP_CLK1_DISABLE = en_val1;
#ifndef ROM_BUILD
        if (pll == sys_pll_sel) {
            cmu->SYS_CLK_DISABLE = CMU_EN_PLL_DISABLE;
            cmu->SYS_CLK_DISABLE = CMU_PU_PLL_DISABLE;
        } else
#endif
        {
            aoncmu->TOP_CLK_DISABLE = en_val;
            aoncmu->TOP_CLK_DISABLE = pu_val;
        }
    }
    int_unlock(lock);

    return 0;
}

int hal_cmu_sys_is_using_audpll(void)
{
#ifndef ROM_BUILD
    if (sys_pll_sel == HAL_CMU_PLL_AUD) {
        if (cmu->SYS_CLK_ENABLE & CMU_SEL_PLL_SYS_ENABLE) {
            return true;
        }
    }
#endif
    return false;
}

int hal_cmu_sys_is_using_bbpll(void)
{
#ifndef ROM_BUILD
    if (sys_pll_sel == HAL_CMU_PLL_BB) {
        if (cmu->SYS_CLK_ENABLE & CMU_SEL_PLL_SYS_ENABLE) {
            return true;
        }
    }
#endif
    return false;
}

int hal_cmu_flash_is_using_audpll(void)
{
#ifndef ROM_BUILD
    if ((cmu->SYS_CLK_ENABLE & (CMU_SEL_PLL_FLS0_FAST_ENABLE | CMU_SEL_PLL_FLS0_ENABLE)) == CMU_SEL_PLL_FLS0_ENABLE) {
        return true;
    }
    if ((cmu->SYS_CLK_ENABLE & (CMU_SEL_PLL_FLS1_FAST_ENABLE | CMU_SEL_PLL_FLS1_ENABLE)) == CMU_SEL_PLL_FLS1_ENABLE) {
        return true;
    }
#endif
    return false;
}

int hal_cmu_flash_is_using_bbpll(void)
{
#ifndef ROM_BUILD
    if ((cmu->SYS_CLK_ENABLE & (CMU_SEL_PLL_FLS0_FAST_ENABLE | CMU_SEL_PLL_FLS0_ENABLE)) ==
            (CMU_SEL_PLL_FLS0_FAST_ENABLE | CMU_SEL_PLL_FLS0_ENABLE)) {
        return true;
    }
    if ((cmu->SYS_CLK_ENABLE & (CMU_SEL_PLL_FLS1_FAST_ENABLE | CMU_SEL_PLL_FLS1_ENABLE)) ==
            (CMU_SEL_PLL_FLS1_FAST_ENABLE | CMU_SEL_PLL_FLS1_ENABLE)) {
        return true;
    }
#endif
    return false;
}

void hal_cmu_low_freq_mode_enable(enum HAL_CMU_FREQ_T old_freq, enum HAL_CMU_FREQ_T new_freq)
{
#if !(defined(ROM_BUILD) || defined(CHIP_ROLE_CP))
    // TODO: Need to lock irq?
    enum HAL_CMU_PLL_T POSSIBLY_UNUSED pll;

    pll = sys_pll_sel;

#ifdef OSC_26M_X4_AUD2BB
    if (old_freq > HAL_CMU_FREQ_104M && new_freq <= HAL_CMU_FREQ_104M) {
        hal_cmu_pll_disable(pll, HAL_CMU_PLL_USER_SYS);
    }
#else
#ifdef FLASH_LOW_SPEED
    if (old_freq > HAL_CMU_FREQ_52M && new_freq <= HAL_CMU_FREQ_52M) {
        hal_cmu_pll_disable(pll, HAL_CMU_PLL_USER_SYS);
    }
#endif
#endif
#endif
}

void hal_cmu_low_freq_mode_disable(enum HAL_CMU_FREQ_T old_freq, enum HAL_CMU_FREQ_T new_freq)
{
#if !(defined(ROM_BUILD) || defined(CHIP_ROLE_CP))
    // TODO: Need to lock irq?
    enum HAL_CMU_PLL_T POSSIBLY_UNUSED pll;

    pll = sys_pll_sel;

#ifdef OSC_26M_X4_AUD2BB
    if (old_freq <= HAL_CMU_FREQ_104M && new_freq > HAL_CMU_FREQ_104M) {
        hal_cmu_pll_enable(pll, HAL_CMU_PLL_USER_SYS);
    }
#else
#ifdef FLASH_LOW_SPEED
    if (old_freq <= HAL_CMU_FREQ_52M && new_freq > HAL_CMU_FREQ_52M) {
        hal_cmu_pll_enable(pll, HAL_CMU_PLL_USER_SYS);
    }
#endif
#endif
#endif
}

void hal_cmu_rom_enable_pll(void)
{
    hal_cmu_flash_all_select_pll(HAL_CMU_PLL_BB);
    hal_cmu_sys_select_pll(HAL_CMU_PLL_BB);
    hal_cmu_pll_enable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_SYS);
}

void hal_cmu_programmer_enable_pll(void)
{
    hal_cmu_flash_all_select_pll(HAL_CMU_PLL_BB);
    hal_cmu_sys_select_pll(HAL_CMU_PLL_BB);
    hal_cmu_pll_enable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_SYS);

    // Enable flash2 PLL source (sens PLL)
    aoncmu->TOP_CLK1_ENABLE = AON_CMU_EN_CLK_PLLBB_SENS_ENABLE | AON_CMU_EN_CLK_PLLAUD_SENS_ENABLE;
    // Enable flash2 X2/X4 source (sens X2/X4)
    senscmu->SYS_CLK_ENABLE = SENS_CMU_PU_OSCX2_ENABLE | SENS_CMU_PU_OSCX4_ENABLE;
    hal_sys_timer_delay(US_TO_TICKS(60));
    senscmu->SYS_CLK_ENABLE = SENS_CMU_EN_OSCX2_ENABLE | SENS_CMU_EN_OSCX4_ENABLE;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_init_pll_selection(void)
{
    enum HAL_CMU_PLL_T sys;

    // Disable the PLL which might be enabled in ROM
    hal_cmu_pll_disable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_ALL);

#ifdef SYS_USE_USBPLL
    sys = HAL_CMU_PLL_USB;
#elif defined(SYS_USE_BBPLL)
    sys = HAL_CMU_PLL_BB;
#elif defined(SYS_USE_DSIPLL)
    sys = HAL_CMU_PLL_DSI;
#else
    sys = HAL_CMU_PLL_AUD;
#endif
#ifdef MCU_HIGH_PERFORMANCE_MODE
#ifndef __AUDIO_RESAMPLE__
#error "AUDIO_RESAMPLE should be used with MCU_HIGH_PERFORMANCE_MODE"
#endif
    if (sys != HAL_CMU_PLL_BB) {
        hal_cmu_simu_tag(30);
        do { volatile int i = 0; i++; } while (1);
    }
#endif
    hal_cmu_sys_select_pll(sys);

    hal_cmu_audio_select_pll(AUDIO_PLL_SEL);

#if !(defined(ULTRA_LOW_POWER) || defined(OSC_26M_X4_AUD2BB))
    hal_cmu_pll_enable(sys, HAL_CMU_PLL_USER_SYS);
#endif

    // Always select one flash pll -- CMU_PU_PLL_ENABLE will check flash pll sel as well
    enum HAL_CMU_PLL_T flash;
#ifdef ROM_BUILD
    flash = HAL_CMU_PLL_BB;
#elif 0 //defined(MCU_HIGH_PERFORMANCE_MODE))
    // System is using AUDPLL (alias of BBPLL).
    // CAUTION: If sys is changed to use BBPLL as well, flash needs a large divider
    //          and high performance mode controlling codes should be located in RAM
    flash = (sys == HAL_CMU_PLL_AUD) ? HAL_CMU_PLL_DSI : HAL_CMU_PLL_AUD;
#else
    flash = sys;
#endif
    hal_cmu_flash_all_select_pll(flash);

#if defined(FLASH_HIGH_SPEED) || \
        (defined(FLASH_SRC_FREQ_MHZ) && (FLASH_SRC_FREQ_MHZ > 104)) || \
        !(defined(FLASH_LOW_SPEED) || defined(OSC_26M_X4_AUD2BB))
    hal_cmu_pll_enable(flash, HAL_CMU_PLL_USER_FLASH);
#endif
}

void hal_cmu_i2s_clock_out_enable(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;

    if (id == HAL_I2S_ID_0) {
        val = CMU_EN_CLK_I2S0_OUT;
    } else {
        val = CMU_EN_CLK_I2S1_OUT;
    }

    lock = int_lock();
    cmu->I2C_CLK |= val;
    int_unlock(lock);
}

enum HAL_CMU_PLL_T hal_cmu_get_audio_pll(void)
{
    return AUDIO_PLL_SEL;
}

void hal_cmu_codec_clock_enable(void)
{
    uint32_t clk;

    clk = AON_CMU_EN_CLK_OSC_CODEC_ENABLE | AON_CMU_EN_CLK_OSCX2_CODEC_ENABLE;
#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
    if (hal_cmu_get_audio_resample_status()) {
        clk |= AON_CMU_EN_CLK_OSCX4_CODEC_ENABLE;
    }
    else
#endif
    {
        if (AUDIO_PLL_SEL == HAL_CMU_PLL_BB) {
            clk |= AON_CMU_EN_CLK_PLLBB_CODEC_ENABLE;
        } else if (AUDIO_PLL_SEL == HAL_CMU_PLL_USB) {
            clk |= AON_CMU_EN_CLK_PLLUSB_CODEC_ENABLE;
        } else {
            clk |= AON_CMU_EN_CLK_PLLAUD_CODEC_ENABLE;
        }
    }
    aoncmu->TOP_CLK1_ENABLE = clk;

    hal_cmu_clock_enable(HAL_CMU_MOD_H_CODEC);
}

void hal_cmu_codec_clock_disable(void)
{
    uint32_t clk;

    hal_cmu_clock_disable(HAL_CMU_MOD_H_CODEC);

    clk = AON_CMU_EN_CLK_OSC_CODEC_DISABLE | AON_CMU_EN_CLK_OSCX2_CODEC_DISABLE;
#if defined(__AUDIO_RESAMPLE__) && defined(ANA_26M_X4_ENABLE)
    if (hal_cmu_get_audio_resample_status()) {
        clk |= AON_CMU_EN_CLK_OSCX4_CODEC_DISABLE;
    }
    else
#endif
    {
        if (AUDIO_PLL_SEL == HAL_CMU_PLL_BB) {
            clk |= AON_CMU_EN_CLK_PLLBB_CODEC_DISABLE;
        } else if (AUDIO_PLL_SEL == HAL_CMU_PLL_USB) {
            clk |= AON_CMU_EN_CLK_PLLUSB_CODEC_DISABLE;
        } else {
            clk |= AON_CMU_EN_CLK_PLLAUD_CODEC_DISABLE;
        }
    }
    aoncmu->TOP_CLK1_DISABLE = clk;
}

void hal_cmu_codec_reset_set(void)
{
    aoncmu->GBL_RESET_SET = AON_CMU_SOFT_RSTN_CODEC_SET;
}

void hal_cmu_codec_reset_clear(void)
{
    aoncmu->GBL_RESET_CLR = AON_CMU_SOFT_RSTN_CODEC_CLR;
    aocmu_reg_update_wait();
}

void hal_cmu_codec_set_fault_mask(uint32_t msk)
{
    uint32_t lock;

    lock = int_lock();
    // If bit set 1, DAC will be muted when some faults occur
    cmu->PERIPH_CLK = SET_BITFIELD(cmu->PERIPH_CLK, CMU_MASK_OBS, msk);
    int_unlock(lock);
}

void hal_cmu_i2s_clock_out_disable(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;

    if (id == HAL_I2S_ID_0) {
        val = CMU_EN_CLK_I2S0_OUT;
    } else {
        val = CMU_EN_CLK_I2S1_OUT;
    }

    lock = int_lock();
    cmu->I2C_CLK &= ~val;
    int_unlock(lock);
}

void hal_cmu_i2s_set_slave_mode(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;

    if (id == HAL_I2S_ID_0) {
        val = CMU_SEL_I2S0_CLKIN;
    } else {
        val = CMU_SEL_I2S1_CLKIN;
    }

    lock = int_lock();
    cmu->I2C_CLK |= val;
    int_unlock(lock);
}

void hal_cmu_i2s_set_master_mode(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;

    if (id == HAL_I2S_ID_0) {
        val = CMU_SEL_I2S0_CLKIN;
    } else {
        val = CMU_SEL_I2S1_CLKIN;
    }

    lock = int_lock();
    cmu->I2C_CLK &= ~val;
    int_unlock(lock);
}

void hal_cmu_i2s_clock_enable(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;
    volatile uint32_t *reg;

    if (id == HAL_I2S_ID_0) {
        val = CMU_EN_CLK_PLL_I2S0;
        reg = &cmu->I2S0_CLK;
    } else {
        val = CMU_EN_CLK_PLL_I2S1;
        reg = &cmu->I2S1_CLK;
    }

    lock = int_lock();
    *reg |= val;
    int_unlock(lock);
}

void hal_cmu_i2s_clock_disable(enum HAL_I2S_ID_T id)
{
    uint32_t lock;
    uint32_t val;
    volatile uint32_t *reg;

    if (id == HAL_I2S_ID_0) {
        val = CMU_EN_CLK_PLL_I2S0;
        reg = &cmu->I2S0_CLK;
    } else {
        val = CMU_EN_CLK_PLL_I2S1;
        reg = &cmu->I2S1_CLK;
    }

    lock = int_lock();
    *reg &= ~val;
    int_unlock(lock);
}

int hal_cmu_i2s_set_div(enum HAL_I2S_ID_T id, uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    if ((div & (CMU_CFG_DIV_I2S0_MASK >> CMU_CFG_DIV_I2S0_SHIFT)) != div) {
        return 1;
    }

    lock = int_lock();
    if (id == HAL_I2S_ID_0) {
        cmu->I2S0_CLK = SET_BITFIELD(cmu->I2S0_CLK, CMU_CFG_DIV_I2S0, div);
    } else {
        cmu->I2S1_CLK = SET_BITFIELD(cmu->I2S1_CLK, CMU_CFG_DIV_I2S1, div);
    }
    int_unlock(lock);

    return 0;
}

void hal_cmu_pcm_clock_out_enable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2C_CLK |= CMU_EN_CLK_PCM_OUT;
    int_unlock(lock);
}

void hal_cmu_pcm_clock_out_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2C_CLK &= ~CMU_EN_CLK_PCM_OUT;
    int_unlock(lock);
}

void hal_cmu_pcm_set_slave_mode(int clk_pol)
{
    uint32_t lock;
    uint32_t mask;
    uint32_t cfg;

    mask = CMU_SEL_PCM_CLKIN | CMU_POL_CLK_PCM_IN;

    if (clk_pol) {
        cfg = CMU_SEL_PCM_CLKIN | CMU_POL_CLK_PCM_IN;
    } else {
        cfg = CMU_SEL_PCM_CLKIN;
    }

    lock = int_lock();
    cmu->I2C_CLK = (cmu->I2C_CLK & ~mask) | cfg;
    int_unlock(lock);
}

void hal_cmu_pcm_set_master_mode(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2C_CLK &= ~CMU_SEL_PCM_CLKIN;
    int_unlock(lock);
}

void hal_cmu_pcm_clock_enable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2S0_CLK |= CMU_EN_CLK_PLL_PCM;
    int_unlock(lock);
}

void hal_cmu_pcm_clock_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2S0_CLK &= ~CMU_EN_CLK_PLL_PCM;
    int_unlock(lock);
}

int hal_cmu_pcm_set_div(uint32_t div)
{
    uint32_t lock;

    if (div < 2) {
        return 1;
    }

    div -= 2;
    if ((div & (CMU_CFG_DIV_PCM_MASK >> CMU_CFG_DIV_PCM_SHIFT)) != div) {
        return 1;
    }

    lock = int_lock();
    cmu->I2S0_CLK = SET_BITFIELD(cmu->I2S0_CLK, CMU_CFG_DIV_PCM, div);
    int_unlock(lock);
    return 0;
}

int hal_cmu_spdif_clock_enable(enum HAL_SPDIF_ID_T id)
{
    uint32_t lock;
    uint32_t mask;

    if (id >= HAL_SPDIF_ID_QTY) {
        return 1;
    }

    mask = CMU_EN_CLK_PLL_SPDIF0;

    lock = int_lock();
    cmu->I2S1_CLK |= mask;
    int_unlock(lock);
    return 0;
}

int hal_cmu_spdif_clock_disable(enum HAL_SPDIF_ID_T id)
{
    uint32_t lock;
    uint32_t mask;

    if (id >= HAL_SPDIF_ID_QTY) {
        return 1;
    }

    mask = CMU_EN_CLK_PLL_SPDIF0;

    lock = int_lock();
    cmu->I2S1_CLK &= ~mask;
    int_unlock(lock);

    return 0;
}

int hal_cmu_spdif_set_div(enum HAL_SPDIF_ID_T id, uint32_t div)
{
    uint32_t lock;

    if (id >= HAL_SPDIF_ID_QTY) {
        return 1;
    }

    if (div < 2) {
        return 2;
    }

    div -= 2;
    if ((div & (CMU_CFG_DIV_SPDIF0_MASK >> CMU_CFG_DIV_SPDIF0_SHIFT)) != div) {
        return 2;
    }

    lock = int_lock();
    cmu->I2S1_CLK = SET_BITFIELD(cmu->I2S1_CLK, CMU_CFG_DIV_SPDIF0, div);
    int_unlock(lock);
    return 0;
}

#ifdef CHIP_HAS_USB
void hal_cmu_usb_set_device_mode(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->SYS_DIV |= CMU_USB_ID;
    aoncmu->SLEEP_TIMER_OSC &= ~AON_CMU_USB_RX_PD;
    int_unlock(lock);
}

void hal_cmu_usb_set_host_mode(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->SYS_DIV &= ~CMU_USB_ID;
    int_unlock(lock);
}

#ifdef ROM_BUILD
enum HAL_CMU_USB_CLOCK_SEL_T hal_cmu_usb_rom_select_clock_source(int pll_en, unsigned int crystal)
{
    enum HAL_CMU_USB_CLOCK_SEL_T sel;

    sel = HAL_CMU_USB_CLOCK_SEL_24M_X2;

    hal_cmu_usb_rom_set_clock_source(sel);

    return sel;
}

void hal_cmu_usb_rom_set_clock_source(enum HAL_CMU_USB_CLOCK_SEL_T sel)
{
}
#endif

static uint32_t hal_cmu_usb_get_clock_source(void)
{
    uint32_t src;

#ifdef USB_CLK_SRC_PLL
    src = CMU_USB_CLK_SRC_PLL;
#else
    src = CMU_USB_CLK_SRC_OSC_24M_X2;
#endif

    return src;
}

void hal_cmu_usb_clock_enable(void)
{
    enum HAL_CMU_PLL_T pll;
    uint32_t lock;
    uint32_t src;

    pll = HAL_CMU_PLL_USB;
    src = hal_cmu_usb_get_clock_source();

    if (src == CMU_USB_CLK_SRC_PLL) {
        hal_cmu_pll_enable(pll, HAL_CMU_PLL_USER_USB);
    }

    lock = int_lock();
    if (src == CMU_USB_CLK_SRC_PLL) {
        cmu->SYS_DIV &= ~CMU_SEL_USB_SRC;
    } else {
        cmu->SYS_DIV |= CMU_SEL_USB_SRC;
    }
    int_unlock(lock);
    hal_cmu_clock_enable(HAL_CMU_MOD_H_USBC);
#ifdef USB_HIGH_SPEED
    hal_cmu_clock_enable(HAL_CMU_MOD_H_USBH);
#endif
    hal_cmu_clock_enable(HAL_CMU_MOD_O_USB32K);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_USB);
    hal_cmu_reset_set(HAL_CMU_MOD_O_USB);
    hal_cmu_reset_set(HAL_CMU_MOD_O_USB32K);
#ifdef USB_HIGH_SPEED
    hal_cmu_reset_set(HAL_CMU_MOD_H_USBH);
#endif
    hal_cmu_reset_set(HAL_CMU_MOD_H_USBC);
    hal_sys_timer_delay(US_TO_TICKS(60));
    hal_cmu_reset_clear(HAL_CMU_MOD_H_USBC);
#ifdef USB_HIGH_SPEED
    hal_cmu_reset_clear(HAL_CMU_MOD_H_USBH);
#endif
    hal_cmu_reset_clear(HAL_CMU_MOD_O_USB32K);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_USB);
}

void hal_cmu_usb_clock_disable(void)
{
    enum HAL_CMU_PLL_T pll;
    uint32_t src;

    pll = HAL_CMU_PLL_USB;
    src = hal_cmu_usb_get_clock_source();

    hal_cmu_reset_set(HAL_CMU_MOD_O_USB);
    hal_cmu_reset_set(HAL_CMU_MOD_O_USB32K);
#ifdef USB_HIGH_SPEED
    hal_cmu_reset_set(HAL_CMU_MOD_H_USBH);
#endif
    hal_cmu_reset_set(HAL_CMU_MOD_H_USBC);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_USB);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_USB32K);
#ifdef USB_HIGH_SPEED
    hal_cmu_clock_disable(HAL_CMU_MOD_H_USBH);
#endif
    hal_cmu_clock_disable(HAL_CMU_MOD_H_USBC);

    if (src == CMU_USB_CLK_SRC_PLL) {
        hal_cmu_pll_disable(pll, HAL_CMU_PLL_USER_USB);
    }
}
#endif

void hal_cmu_sdmmc0_clock_enable(void)
{
    hal_cmu_clock_enable(HAL_CMU_MOD_O_SDMMC);
    hal_cmu_clock_enable(HAL_CMU_MOD_H_SDMMC);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_SDMMC);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_SDMMC);
}

void hal_cmu_sdmmc0_clock_disable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_SDMMC);
    hal_cmu_reset_set(HAL_CMU_MOD_O_SDMMC);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_SDMMC);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_SDMMC);
}

void hal_cmu_sdmmc1_clock_enable(void)
{
    hal_cmu_clock_enable(HAL_CMU_MOD_O_EMMC);
    hal_cmu_clock_enable(HAL_CMU_MOD_H_EMMC);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_EMMC);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_EMMC);
}

void hal_cmu_sdmmc1_clock_disable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_EMMC);
    hal_cmu_reset_set(HAL_CMU_MOD_O_EMMC);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_EMMC);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_EMMC);
}

void BOOT_TEXT_FLASH_LOC hal_cmu_apb_init_div(void)
{
    // Divider defaults to 2 (reg_val = div - 2)
    //cmu->SYS_DIV = SET_BITFIELD(cmu->SYS_DIV, CMU_CFG_DIV_PCLK, 0);
}

int hal_cmu_periph_set_div(uint32_t div)
{
    uint32_t lock;
    int ret = 0;

    if (div == 0 || div > ((CMU_CFG_DIV_PER_MASK >> CMU_CFG_DIV_PER_SHIFT) + 2)) {
        cmu->DSI_CLK_DISABLE = CMU_RSTN_DIV_PER_DISABLE;
        if (div > ((CMU_CFG_DIV_PER_MASK >> CMU_CFG_DIV_PER_SHIFT) + 2)) {
            ret = 1;
        }
    } else {
        lock = int_lock();
        if (div == 1) {
            cmu->DSI_CLK_ENABLE = CMU_BYPASS_DIV_PER_ENABLE;
        } else {
            cmu->DSI_CLK_DISABLE = CMU_BYPASS_DIV_PER_DISABLE;
            div -= 2;
            cmu->I2S0_CLK = SET_BITFIELD(cmu->I2S0_CLK, CMU_CFG_DIV_PER, div);
        }
        int_unlock(lock);
        cmu->DSI_CLK_ENABLE = CMU_RSTN_DIV_PER_ENABLE;
    }

    return ret;
}

#define PERPH_SET_DIV_FUNC(f, F, r) \
int hal_cmu_ ##f## _set_div(uint32_t div) \
{ \
    uint32_t lock; \
    int ret = 0; \
    lock = int_lock(); \
    if (div < 2 || div > ((CMU_CFG_DIV_ ##F## _MASK >> CMU_CFG_DIV_ ##F## _SHIFT) + 2)) { \
        cmu->r &= ~(CMU_SEL_OSCX2_ ##F | CMU_SEL_PLL_ ##F | CMU_EN_PLL_ ##F); \
        ret = 1; \
    } else { \
        div -= 2; \
        cmu->r = (cmu->r & ~(CMU_CFG_DIV_ ##F## _MASK)) | CMU_SEL_OSCX2_ ##F | CMU_SEL_PLL_ ##F | \
            CMU_CFG_DIV_ ##F(div); \
        cmu->r |= CMU_EN_PLL_ ##F; \
    } \
    int_unlock(lock); \
    return ret; \
}

PERPH_SET_DIV_FUNC(uart0, UART0, UART_CLK);
PERPH_SET_DIV_FUNC(uart1, UART1, UART_CLK);
PERPH_SET_DIV_FUNC(uart2, UART2, UART_CLK);
PERPH_SET_DIV_FUNC(spi, SPI1, SYS_DIV);
PERPH_SET_DIV_FUNC(sdmmc1, EMMC, PERIPH_CLK);
PERPH_SET_DIV_FUNC(sdmmc0, SDMMC, ADMA_CH15_REQ);
PERPH_SET_DIV_FUNC(i2c, I2C, I2C_CLK);

#define PERPH_SET_FREQ_FUNC(f, F, r) \
int hal_cmu_ ##f## _set_freq(enum HAL_CMU_PERIPH_FREQ_T freq) \
{ \
    uint32_t lock; \
    int ret = 0; \
    lock = int_lock(); \
    if (freq == HAL_CMU_PERIPH_FREQ_26M) { \
        cmu->r &= ~(CMU_SEL_OSCX2_ ##F | CMU_SEL_PLL_ ##F | CMU_EN_PLL_ ##F); \
    } else if (freq == HAL_CMU_PERIPH_FREQ_52M) { \
        cmu->r = (cmu->r & ~(CMU_SEL_PLL_ ##F | CMU_EN_PLL_ ##F)) | CMU_SEL_OSCX2_ ##F; \
    } else { \
        ret = 1; \
    } \
    int_unlock(lock); \
    return ret; \
}

PERPH_SET_FREQ_FUNC(uart0, UART0, UART_CLK);
PERPH_SET_FREQ_FUNC(uart1, UART1, UART_CLK);
PERPH_SET_FREQ_FUNC(uart2, UART2, UART_CLK);
PERPH_SET_FREQ_FUNC(spi, SPI1, SYS_DIV);
PERPH_SET_FREQ_FUNC(sdmmc1, EMMC, PERIPH_CLK);
PERPH_SET_FREQ_FUNC(sdmmc0, SDMMC, ADMA_CH15_REQ);
PERPH_SET_FREQ_FUNC(i2c, I2C, I2C_CLK);

int hal_cmu_ispi_set_freq(enum HAL_CMU_PERIPH_FREQ_T freq)
{
    uint32_t lock;
    int ret = 0;

    lock = int_lock();
    if (freq == HAL_CMU_PERIPH_FREQ_26M) {
        cmu->SYS_DIV &= ~CMU_SEL_OSCX2_SPI2;
    } else if (freq == HAL_CMU_PERIPH_FREQ_52M) {
        cmu->SYS_DIV |= CMU_SEL_OSCX2_SPI2;
    } else {
        ret = 1;
    }
    int_unlock(lock);

    return ret;
}

void hal_cmu_sec_eng_clock_enable(void)
{
    hal_cmu_clock_enable(HAL_CMU_MOD_H_SEC_ENG);
    hal_cmu_clock_enable(HAL_CMU_MOD_P_SEC_ENG);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_SEC_ENG);
    hal_cmu_reset_clear(HAL_CMU_MOD_P_SEC_ENG);
}

void hal_cmu_sec_eng_clock_disable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_SEC_ENG);
    hal_cmu_reset_set(HAL_CMU_MOD_P_SEC_ENG);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_SEC_ENG);
    hal_cmu_clock_disable(HAL_CMU_MOD_P_SEC_ENG);
}

int hal_cmu_clock_out_enable(enum HAL_CMU_CLOCK_OUT_ID_T id)
{
    uint32_t lock;
    uint32_t sel;
    uint32_t cfg;

    enum CMU_CLK_OUT_SEL_T {
        CMU_CLK_OUT_SEL_NONE    = 0,
        CMU_CLK_OUT_SEL_CODEC   = 1,
        CMU_CLK_OUT_SEL_BT      = 2,
        CMU_CLK_OUT_SEL_MCU     = 3,
        CMU_CLK_OUT_SEL_SENS    = 4,
        CMU_CLK_OUT_SEL_AON     = 5,

        CMU_CLK_OUT_SEL_QTY
    };

    sel = CMU_CLK_OUT_SEL_QTY;
    cfg = 0;

    if (HAL_CMU_CLOCK_OUT_AON_32K <= id && id <= HAL_CMU_CLOCK_OUT_AON_DCDC) {
        sel = CMU_CLK_OUT_SEL_AON;
        cfg = id - HAL_CMU_CLOCK_OUT_AON_32K;
    } else if (HAL_CMU_CLOCK_OUT_MCU_32K <= id && id <= HAL_CMU_CLOCK_OUT_MCU_DSI) {
        sel = CMU_CLK_OUT_SEL_MCU;
        lock = int_lock();
        cmu->PERIPH_CLK = SET_BITFIELD(cmu->PERIPH_CLK, CMU_CFG_CLK_OUT, id - HAL_CMU_CLOCK_OUT_MCU_32K);
        int_unlock(lock);
    } else if (HAL_CMU_CLOCK_OUT_CODEC_ADC_ANA <= id && id <= HAL_CMU_CLOCK_OUT_CODEC_HCLK) {
        sel = CMU_CLK_OUT_SEL_CODEC;
        hal_codec_select_clock_out(id - HAL_CMU_CLOCK_OUT_CODEC_ADC_ANA);
    } else if (HAL_CMU_CLOCK_OUT_BT_NONE <= id && id <= HAL_CMU_CLOCK_OUT_BT_DACD8) {
        sel = CMU_CLK_OUT_SEL_BT;
        btcmu->CLK_OUT = SET_BITFIELD(btcmu->CLK_OUT, BT_CMU_CFG_CLK_OUT, id - HAL_CMU_CLOCK_OUT_BT_NONE);
    } else if (HAL_CMU_CLOCK_OUT_SENS_32K <= id && id <= HAL_CMU_CLOCK_OUT_SENS_I2S) {
        sel = CMU_CLK_OUT_SEL_SENS;
        senscmu->CLK_OUT = SET_BITFIELD(senscmu->CLK_OUT, SENS_CMU_CFG_CLK_OUT, id - HAL_CMU_CLOCK_OUT_SENS_32K);
    }

    if (sel < CMU_CLK_OUT_SEL_QTY) {
        lock = int_lock();
        aoncmu->CLK_OUT = (aoncmu->CLK_OUT & ~(AON_CMU_SEL_CLK_OUT_MASK | AON_CMU_CFG_CLK_OUT_MASK)) |
            AON_CMU_SEL_CLK_OUT(sel) | AON_CMU_CFG_CLK_OUT(cfg) | AON_CMU_EN_CLK_OUT;
        int_unlock(lock);

        return 0;
    }

    return 1;
}

void hal_cmu_clock_out_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->CLK_OUT &= ~AON_CMU_EN_CLK_OUT;
    int_unlock(lock);
}

int hal_cmu_i2s_mclk_enable(enum HAL_CMU_I2S_MCLK_ID_T id)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2C_CLK = SET_BITFIELD(cmu->I2C_CLK, CMU_SEL_I2S_MCLK, id) | CMU_EN_I2S_MCLK;
    aoncmu->AON_CLK = (aoncmu->AON_CLK & ~AON_CMU_SEL_I2S_MCLK_MASK) |
        AON_CMU_SEL_I2S_MCLK(CMU_I2S_MCLK_SEL_MCU) | AON_CMU_EN_I2S_MCLK;
    int_unlock(lock);

    return 0;
}

void hal_cmu_i2s_mclk_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    cmu->I2C_CLK &= ~CMU_EN_I2S_MCLK;
    aoncmu->AON_CLK &= ~AON_CMU_EN_I2S_MCLK;
    int_unlock(lock);
}

int hal_cmu_pwm_set_freq(enum HAL_PWM_ID_T id, uint32_t freq)
{
    uint32_t lock;
    int clk_32k;
    uint32_t div;

    if (id >= HAL_PWM_ID_QTY) {
        return 1;
    }

    if (freq == 0) {
        clk_32k = 1;
        div = 0;
    } else {
        clk_32k = 0;
        div = hal_cmu_get_crystal_freq() / freq;
        if (div < 2) {
            return 1;
        }

        div -= 2;
        if ((div & (AON_CMU_CFG_DIV_PWM0_MASK >> AON_CMU_CFG_DIV_PWM0_SHIFT)) != div) {
            return 1;
        }
    }

    lock = int_lock();
    if (id == HAL_PWM_ID_0) {
        aoncmu->PWM01_CLK = (aoncmu->PWM01_CLK & ~(AON_CMU_CFG_DIV_PWM0_MASK | AON_CMU_SEL_OSC_PWM0 | AON_CMU_EN_OSC_PWM0)) |
            AON_CMU_CFG_DIV_PWM0(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM0 | AON_CMU_EN_OSC_PWM0));
    } else if (id == HAL_PWM_ID_1) {
        aoncmu->PWM01_CLK = (aoncmu->PWM01_CLK & ~(AON_CMU_CFG_DIV_PWM1_MASK | AON_CMU_SEL_OSC_PWM1 | AON_CMU_EN_OSC_PWM1)) |
            AON_CMU_CFG_DIV_PWM1(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM1 | AON_CMU_EN_OSC_PWM1));
    } else if (id == HAL_PWM_ID_2) {
        aoncmu->PWM23_CLK = (aoncmu->PWM23_CLK & ~(AON_CMU_CFG_DIV_PWM2_MASK | AON_CMU_SEL_OSC_PWM2 | AON_CMU_EN_OSC_PWM2)) |
            AON_CMU_CFG_DIV_PWM2(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM2 | AON_CMU_EN_OSC_PWM2));
    } else if (id == HAL_PWM_ID_3) {
        aoncmu->PWM23_CLK = (aoncmu->PWM23_CLK & ~(AON_CMU_CFG_DIV_PWM3_MASK | AON_CMU_SEL_OSC_PWM3 | AON_CMU_EN_OSC_PWM3)) |
            AON_CMU_CFG_DIV_PWM3(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM3 | AON_CMU_EN_OSC_PWM3));
    } else if (id == HAL_PWM1_ID_0) {
        aoncmu->PWM45_CLK = (aoncmu->PWM45_CLK & ~(AON_CMU_CFG_DIV_PWM4_MASK | AON_CMU_SEL_OSC_PWM4 | AON_CMU_EN_OSC_PWM4)) |
            AON_CMU_CFG_DIV_PWM4(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM4 | AON_CMU_EN_OSC_PWM4));
    } else if (id == HAL_PWM1_ID_1) {
        aoncmu->PWM45_CLK = (aoncmu->PWM45_CLK & ~(AON_CMU_CFG_DIV_PWM5_MASK | AON_CMU_SEL_OSC_PWM5 | AON_CMU_EN_OSC_PWM5)) |
            AON_CMU_CFG_DIV_PWM5(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM5 | AON_CMU_EN_OSC_PWM5));
    } else if (id == HAL_PWM1_ID_2) {
        aoncmu->PWM67_CLK = (aoncmu->PWM67_CLK & ~(AON_CMU_CFG_DIV_PWM6_MASK | AON_CMU_SEL_OSC_PWM6 | AON_CMU_EN_OSC_PWM6)) |
            AON_CMU_CFG_DIV_PWM6(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM6 | AON_CMU_EN_OSC_PWM6));
    } else {
        aoncmu->PWM67_CLK = (aoncmu->PWM67_CLK & ~(AON_CMU_CFG_DIV_PWM7_MASK | AON_CMU_SEL_OSC_PWM7 | AON_CMU_EN_OSC_PWM7)) |
            AON_CMU_CFG_DIV_PWM7(div) | (clk_32k ? 0 : (AON_CMU_SEL_OSC_PWM7 | AON_CMU_EN_OSC_PWM7));
    }
    int_unlock(lock);
    return 0;
}

void hal_cmu_jtag_enable(void)
{
    uint32_t lock;

    lock = int_lock();
    aonsec->SEC_BOOT_ACC &= ~(AON_SEC_SECURE_BOOT_JTAG | AON_SEC_SECURE_BOOT_I2C);
    int_unlock(lock);
}

void hal_cmu_jtag_disable(void)
{
    uint32_t lock;

    lock = int_lock();
    aonsec->SEC_BOOT_ACC |= (AON_SEC_SECURE_BOOT_JTAG | AON_SEC_SECURE_BOOT_I2C);
    int_unlock(lock);
}

void hal_cmu_jtag_clock_enable(void)
{
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_JTAG_ENABLE;
}

void hal_cmu_jtag_clock_disable(void)
{
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_JTAG_DISABLE;
}

void hal_cmu_mcu_pdm_clock_out(uint32_t clk_map)
{
    uint32_t lock;
    uint32_t val = 0;

    if (clk_map & (1 << 0)) {
        val |= AON_CMU_SEL_PDM_CLKOUT0_SENS;
    }
    if (clk_map & (1 << 1)) {
        val |= AON_CMU_SEL_PDM_CLKOUT1_SENS;
    }

    lock = int_lock();
    aoncmu->AON_CLK |= val;
    int_unlock(lock);
}

void hal_cmu_sens_pdm_clock_out(uint32_t clk_map)
{
    uint32_t lock;
    uint32_t val = 0;

    if (clk_map & (1 << 0)) {
        val |= AON_CMU_SEL_PDM_CLKOUT0_SENS;
    }
    if (clk_map & (1 << 1)) {
        val |= AON_CMU_SEL_PDM_CLKOUT1_SENS;
    }

    lock = int_lock();
    aoncmu->AON_CLK &= ~val;
    int_unlock(lock);
}

void hal_cmu_rom_clock_init(void)
{
    aoncmu->AON_CLK |= AON_CMU_BYPASS_LOCK_PLLUSB | AON_CMU_BYPASS_LOCK_PLLBB | AON_CMU_BYPASS_LOCK_PLLAUD |
        AON_CMU_BYPASS_LOCK_PLLDSI | AON_CMU_SEL_CLK_OSC;
    // Enable PMU fast clock
    aoncmu->CLK_OUT |= AON_CMU_BYPASS_DIV_DCDC;
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_DCDC0_ENABLE;
    // Ignore subsystem 26M ready timer
    aoncmu->CLK_SELECT |= AON_CMU_OSC_READY_MODE;

    // Debug Select CMU REG F4
    cmu->MCU_TIMER = SET_BITFIELD(cmu->MCU_TIMER, CMU_DEBUG_REG_SEL, CMU_DEBUG_REG_SEL_DEBUG);
}

void hal_cmu_init_chip_feature(uint16_t feature)
{
    aoncmu->CHIP_FEATURE = feature | AON_CMU_EFUSE_LOCK;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_osc_x2_enable(void)
{
    cmu->SYS_CLK_ENABLE = CMU_PU_OSCX2_ENABLE;
    hal_sys_timer_delay(US_TO_TICKS(60));
    cmu->SYS_CLK_ENABLE = CMU_EN_OSCX2_ENABLE;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_osc_x4_enable(void)
{
#ifdef ANA_26M_X4_ENABLE
    cmu->SYS_CLK_ENABLE = CMU_PU_OSCX4_ENABLE;
    hal_sys_timer_delay(US_TO_TICKS(60));
    cmu->SYS_CLK_ENABLE = CMU_EN_OSCX4_ENABLE;
#endif
#ifdef OSC_26M_X4_AUD2BB
    aoncmu->CLK_SELECT &= ~AON_CMU_SEL_X4_DIG;
#endif
}

void hal_cmu_set_wakeup_vector(uint32_t vector)
{
    aoncmu->MCU_VTOR = (aoncmu->MCU_VTOR & ~AON_CMU_VTOR_MCU_MASK) | (vector & AON_CMU_VTOR_MCU_MASK);
}

void BOOT_TEXT_FLASH_LOC hal_cmu_module_init_state(void)
{
#ifdef SLOW_SYS_BYPASS
    aoncmu->CLK_SELECT |= AON_CMU_SEL_SLOW_SYS_BYPASS;
#endif
    aoncmu->AON_CLK |= AON_CMU_BYPASS_LOCK_PLLUSB | AON_CMU_BYPASS_LOCK_PLLBB | AON_CMU_BYPASS_LOCK_PLLAUD |
        AON_CMU_BYPASS_LOCK_PLLDSI | AON_CMU_SEL_CLK_OSC;
    // Disable AON OSC always on
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_OSC_DISABLE;
    // Slow down PMU fast clock
    aoncmu->CLK_OUT = (aoncmu->CLK_OUT & ~(AON_CMU_BYPASS_DIV_DCDC | AON_CMU_CFG_DIV_DCDC_MASK)) | AON_CMU_CFG_DIV_DCDC(2);
#ifdef DCDC_CLOCK_CONTROL
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_DCDC0_DISABLE;
#endif
    // Let BT to control BT RAM retention states
    // Set all RAM retention timers to 1 clock cycle
    aoncmu->RAM_LP_TIMER = (aoncmu->RAM_LP_TIMER & ~(AON_CMU_TIMER1_MCU_REG_MASK | AON_CMU_TIMER2_MCU_REG_MASK |
        AON_CMU_TIMER3_MCU_REG_MASK | AON_CMU_POWER_MODE_BT_DR)) | AON_CMU_TIMER1_MCU_REG(0) |
        AON_CMU_TIMER2_MCU_REG(0) | AON_CMU_TIMER3_MCU_REG(0);
#ifdef NO_RAM_RETENTION_FSM
    aoncmu->RAM_LP_TIMER &= ~AON_CMU_PG_AUTO_EN_MCU_REG;
#else
    aoncmu->RAM_LP_TIMER |= AON_CMU_PG_AUTO_EN_MCU_REG;
    aoncmu->RESERVED_03C |= AON_CMU_SRAM0_BLK_RET_AUTO(3) | AON_CMU_SRAM1_BLK_RET_AUTO(3) |
        AON_CMU_SRAM2_BLK_RET_AUTO(3) | AON_CMU_SRAM3_BLK_RET_AUTO(3) | AON_CMU_SRAM4_BLK_RET_AUTO(3) |
        AON_CMU_SRAM5_BLK_RET_AUTO(3) | AON_CMU_SRAM6_BLK_RET_AUTO(3) | AON_CMU_SRAM7_BLK_RET_AUTO_1 |
        AON_CMU_SRAM7_BLK_RET_AUTO_2 | AON_CMU_SRAM8_BLK_RET_AUTO(3) | AON_CMU_SENS_SRAM_BLK_RET_AUTO;
#endif

    // DMA channel config
    cmu->ADMA_CH0_4_REQ =
        // codec
        CMU_ADMA_CH0_REQ_IDX(CMU_DMA_REQ_CODEC_RX) | CMU_ADMA_CH1_REQ_IDX(CMU_DMA_REQ_CODEC_TX) |
#ifdef CODEC_DSD
        // codec_dsd
        CMU_ADMA_CH2_REQ_IDX(CMU_DMA_REQ_DSD_RX) | CMU_ADMA_CH3_REQ_IDX(CMU_DMA_REQ_DSD_TX) |
#else
        // btpcm
        CMU_ADMA_CH2_REQ_IDX(CMU_DMA_REQ_PCM_RX) | CMU_ADMA_CH3_REQ_IDX(CMU_DMA_REQ_PCM_TX) |
#endif
        // i2s0
        CMU_ADMA_CH4_REQ_IDX(CMU_DMA_REQ_I2S0_RX);
    cmu->ADMA_CH5_9_REQ =
        // i2s0
        CMU_ADMA_CH5_REQ_IDX(CMU_DMA_REQ_I2S0_TX) |
        // FIR
        CMU_ADMA_CH6_REQ_IDX(CMU_DMA_REQ_FIR_RX) | CMU_ADMA_CH7_REQ_IDX(CMU_DMA_REQ_FIR_TX) |
        // spdif
        CMU_ADMA_CH8_REQ_IDX(CMU_DMA_REQ_SPDIF0_RX) | CMU_ADMA_CH9_REQ_IDX(CMU_DMA_REQ_SPDIF0_TX);
    cmu->ADMA_CH10_14_REQ =
        // codec2
        CMU_ADMA_CH10_REQ_IDX(CMU_DMA_REQ_NULL) | CMU_ADMA_CH11_REQ_IDX(CMU_DMA_REQ_CODEC_TX2) |
        // btdump
        CMU_ADMA_CH12_REQ_IDX(CMU_DMA_REQ_BTDUMP) |
        // mc
        CMU_ADMA_CH13_REQ_IDX(CMU_DMA_REQ_CODEC_MC) |
        // i2s1
        CMU_ADMA_CH14_REQ_IDX(CMU_DMA_REQ_I2S1_RX);
    cmu->ADMA_CH15_REQ =
        // i2s1
        SET_BITFIELD(cmu->ADMA_CH15_REQ, CMU_ADMA_CH15_REQ_IDX, CMU_DMA_REQ_I2S1_TX);
    cmu->GDMA_CH0_4_REQ =
        // sdmmc0
        CMU_GDMA_CH0_REQ_IDX(CMU_DMA_REQ_SDMMC0) |
        // sdmmc1
        CMU_GDMA_CH1_REQ_IDX(CMU_DMA_REQ_SDMMC1) |
        // i2c0
        CMU_GDMA_CH2_REQ_IDX(CMU_DMA_REQ_I2C0_RX) | CMU_GDMA_CH3_REQ_IDX(CMU_DMA_REQ_I2C0_TX) |
        // spi
        CMU_GDMA_CH4_REQ_IDX(CMU_DMA_REQ_SPILCD0_RX);
    cmu->GDMA_CH5_9_REQ =
        // spi
        CMU_GDMA_CH5_REQ_IDX(CMU_DMA_REQ_SPILCD0_TX) |
        CMU_GDMA_CH6_REQ_IDX(CMU_DMA_REQ_NULL) | CMU_GDMA_CH7_REQ_IDX(CMU_DMA_REQ_NULL) |
        // uart0
        CMU_GDMA_CH8_REQ_IDX(CMU_DMA_REQ_UART0_RX) | CMU_GDMA_CH9_REQ_IDX(CMU_DMA_REQ_UART0_TX);
    cmu->GDMA_CH10_14_REQ =
        // uart1
        CMU_GDMA_CH10_REQ_IDX(CMU_DMA_REQ_UART1_RX) | CMU_GDMA_CH11_REQ_IDX(CMU_DMA_REQ_UART1_TX) |
        // i2c1
        CMU_GDMA_CH12_REQ_IDX(CMU_DMA_REQ_I2C1_RX) | CMU_GDMA_CH13_REQ_IDX(CMU_DMA_REQ_I2C1_TX) |
        // uart2
        CMU_GDMA_CH14_REQ_IDX(CMU_DMA_REQ_UART2_RX);
    cmu->GDMA_CH15_REQ =
        // uart2
        SET_BITFIELD(cmu->GDMA_CH15_REQ, CMU_GDMA_CH15_REQ_IDX, CMU_DMA_REQ_UART2_TX);

#if 1 //ndef SIMU
    cmu->XRESET_SET = SYS_XRST_PSRAM | SYS_XRST_SDMMC | SYS_XRST_GPU |  SYS_XRST_L2CC |
        SYS_XRST_VMMU | SYS_XRST_GA2D | SYS_XRST_MISC | SYS_XRST_LCDC;
    cmu->ORESET_SET = SYS_ORST_USB | SYS_ORST_SDMMC | SYS_ORST_WDT | SYS_ORST_TIMER2 |
        SYS_ORST_I2C0 | SYS_ORST_I2C1 | SYS_ORST_SPI | SYS_ORST_EMMC |
        SYS_ORST_UART0 | SYS_ORST_UART1 | SYS_ORST_UART2 | SYS_ORST_I2S0 | SYS_ORST_SPDIF0 | SYS_ORST_PCM |
        SYS_ORST_USB32K | SYS_ORST_I2S1 |
        SYS_ORST_DISPIX | SYS_ORST_DISDSI;
    cmu->PRESET_SET = SYS_PRST_WDT | SYS_PRST_TIMER2 | SYS_PRST_I2C0 | SYS_PRST_I2C1 |
        SYS_PRST_SPI | SYS_PRST_TRNG |
        SYS_PRST_UART0 | SYS_PRST_UART1 | SYS_PRST_UART2 |
        SYS_PRST_PCM | SYS_PRST_I2S0 | SYS_PRST_SPDIF0 | SYS_PRST_I2S1 | SYS_PRST_BCM |
#ifndef ARM_CMNS
        SYS_PRST_TZC |
#endif
        SYS_PRST_DIS | SYS_PRST_PSPY;
    cmu->HRESET_SET = SYS_HRST_PSPY |
        SYS_HRST_EMMC | SYS_HRST_USBC | SYS_HRST_CODEC | SYS_HRST_CP0 | SYS_HRST_BT_TPORT |
        SYS_HRST_USBH | SYS_HRST_LCDC | SYS_HRST_BT_DUMP | SYS_HRST_CORE1 | SYS_HRST_BCM;
    cmu->OCLK_DISABLE = SYS_OCLK_USB | SYS_OCLK_SDMMC | SYS_OCLK_WDT | SYS_OCLK_TIMER2 |
        SYS_OCLK_I2C0 | SYS_OCLK_I2C1 | SYS_OCLK_SPI | SYS_OCLK_EMMC |
        SYS_OCLK_UART0 | SYS_OCLK_UART1 | SYS_OCLK_UART2 | SYS_OCLK_I2S0 | SYS_OCLK_SPDIF0 | SYS_OCLK_PCM |
        SYS_OCLK_USB32K | SYS_OCLK_I2S1 |
        SYS_OCLK_DISPIX | SYS_OCLK_DISDSI;
    cmu->PCLK_DISABLE = SYS_PCLK_WDT | SYS_PCLK_TIMER2 | SYS_PCLK_I2C0 | SYS_PCLK_I2C1 |
        SYS_PCLK_SPI | SYS_PCLK_TRNG |
        SYS_PCLK_UART0 | SYS_PCLK_UART1 | SYS_PCLK_UART2 |
        SYS_PCLK_PCM | SYS_PCLK_I2S0 | SYS_PCLK_SPDIF0 | SYS_PCLK_I2S1 | SYS_PCLK_BCM |
#ifndef ARM_CMNS
        SYS_PCLK_TZC |
#endif
        SYS_PCLK_DIS | SYS_PCLK_PSPY;
    cmu->HCLK_DISABLE = SYS_HCLK_PSPY |
        SYS_HCLK_EMMC | SYS_HCLK_USBC | SYS_HCLK_CODEC | SYS_HRST_CP0 | SYS_HCLK_BT_TPORT |
        SYS_HCLK_USBH | SYS_HCLK_LCDC | SYS_HCLK_BT_DUMP | SYS_HCLK_CORE1 | SYS_HCLK_BCM;
    cmu->XCLK_DISABLE = SYS_XCLK_PSRAM | SYS_XCLK_SDMMC | SYS_XCLK_GPU | SYS_XCLK_L2CC |
		SYS_XCLK_VMMU | SYS_XCLK_GA2D | SYS_XCLK_MISC | SYS_XCLK_LCDC;
#ifdef ALT_BOOT_FLASH
    cmu->ORESET_SET = SYS_ORST_FLASH;
    cmu->HRESET_SET = SYS_HRST_FLASH;
    cmu->OCLK_DISABLE = SYS_OCLK_FLASH;
    cmu->HCLK_DISABLE = SYS_HCLK_FLASH;
#else
    cmu->ORESET_SET = SYS_ORST_FLASH1;
    cmu->HRESET_SET = SYS_HRST_FLASH1;
    cmu->OCLK_DISABLE = SYS_OCLK_FLASH1;
    cmu->HCLK_DISABLE = SYS_HCLK_FLASH1;
#endif

#if !defined(MCU_SPI_SLAVE) && !defined(CODEC_APP)
    cmu->HRESET_SET = SYS_HRST_SPI_AHB;
    cmu->HCLK_DISABLE = SYS_HCLK_SPI_AHB;
#endif

    aoncmu->RESET_SET = AON_CMU_ARESETN_SET(AON_ARST_PWM) |
        AON_CMU_ORESETN_SET(AON_ORST_PWM0 | AON_ORST_PWM1 | AON_ORST_PWM2 | AON_ORST_PWM3 | AON_ORST_BTAON);

    aoncmu->MOD_CLK_DISABLE = AON_CMU_MANUAL_ACLK_DISABLE(AON_ACLK_PWM) |
        AON_CMU_MANUAL_OCLK_DISABLE(AON_OCLK_PWM0 | AON_OCLK_PWM1 | AON_OCLK_PWM2 | AON_OCLK_PWM3 | AON_OCLK_BTAON);

#ifndef MCU_I2C_SLAVE
    aoncmu->GBL_RESET_SET = AON_CMU_SOFT_RSTN_SENS_SET | AON_CMU_SOFT_RSTN_SENSCPU_SET;
    aoncmu->TOP_CLK1_DISABLE = AON_CMU_EN_CLK_32K_SENS_DISABLE;
#endif

    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_OSCX2_DISABLE | AON_CMU_EN_CLK_TOP_OSC_DISABLE |
        AON_CMU_EN_CLK_32K_BT_DISABLE | AON_CMU_EN_X2_DIG_DISABLE | AON_CMU_EN_X4_DIG_DISABLE | AON_CMU_EN_CLK_TOP_OSCX4_DISABLE |
        AON_CMU_EN_BT_CLK_SYS_DISABLE | AON_CMU_EN_CLK_32K_CODEC_DISABLE;

    aoncmu->MOD_CLK_MODE = ~AON_CMU_MODE_ACLK(AON_ACLK_CMU | AON_ACLK_WDT |
        AON_ACLK_PWM | AON_ACLK_TIMER0 | AON_ACLK_PSC | AON_ACLK_IOMUX |
        AON_ACLK_TIMER1);
    cmu->PCLK_MODE &= ~(SYS_PCLK_CMU | SYS_PCLK_WDT | SYS_PCLK_TIMER0 | SYS_PCLK_TIMER1 | SYS_PCLK_TIMER2);

    //cmu->HCLK_MODE = 0;
    //cmu->PCLK_MODE = SYS_PCLK_UART0 | SYS_PCLK_UART1 | SYS_PCLK_UART2;
    //cmu->OCLK_MODE = 0;
#endif

#ifdef CORE_SLEEP_POWER_DOWN
    hal_cmu_set_wakeup_vector(SCB->VTOR);
    NVIC_SetResetHandler(hal_sleep_core_power_up);
#endif
    hal_psc_init();
}

void BOOT_TEXT_FLASH_LOC hal_cmu_save_subsys_metal_id(uint32_t id)
{
    aoncmu->SW_INFO = (aoncmu->SW_INFO & ~0xFF) | (id & 0xFF);
}
void save_hw_metal_id(uint32_t id) __attribute__((alias("hal_cmu_save_subsys_metal_id")));

void hal_cmu_lpu_wait_26m_ready(void)
{
    while ((cmu->WAKEUP_CLK_CFG & (CMU_LPU_AUTO_SWITCH26 | CMU_LPU_STATUS_26M)) ==
            CMU_LPU_AUTO_SWITCH26);
}

int hal_cmu_lpu_busy(void)
{
    if ((cmu->WAKEUP_CLK_CFG & (CMU_LPU_AUTO_SWITCH26 | CMU_LPU_STATUS_26M)) ==
            CMU_LPU_AUTO_SWITCH26) {
        return 1;
    }
    if ((cmu->WAKEUP_CLK_CFG & (CMU_LPU_AUTO_SWITCHPLL | CMU_LPU_STATUS_PLL)) ==
            CMU_LPU_AUTO_SWITCHPLL) {
        return 1;
    }
    return 0;
}

int BOOT_TEXT_FLASH_LOC hal_cmu_lpu_init(enum HAL_CMU_LPU_CLK_CFG_T cfg)
{
    uint32_t lpu_clk;
    uint32_t timer_26m;
    uint32_t timer_pll;
    uint32_t overhead_26m;

#ifdef NO_SLEEP_POWER_DOWN
    // None of MCU/BT/SENS enables sleep power down
    overhead_26m = hal_cmu_get_osc_switch_overhead();
#else
    // Any SUBSYS enables sleep power down
    overhead_26m = 0;
#endif

    timer_26m = LPU_TIMER_US(HAL_CMU_26M_READY_TIMEOUT_US);
    timer_pll = LPU_TIMER_US(HAL_CMU_PLL_LOCKED_TIMEOUT_US);

    if (timer_26m && timer_26m > overhead_26m) {
        timer_26m -= overhead_26m;
    } else {
        timer_26m = 1;
    }
    if (timer_pll == 0) {
        timer_pll = 1;
    }

    if (cfg >= HAL_CMU_LPU_CLK_QTY) {
        return 1;
    }
    if ((timer_26m & (CMU_TIMER_WT26_MASK >> CMU_TIMER_WT26_SHIFT)) != timer_26m) {
        return 2;
    }
    if ((timer_pll & (CMU_TIMER_WTPLL_MASK >> CMU_TIMER_WTPLL_SHIFT)) != timer_pll) {
        return 3;
    }
    if (hal_cmu_lpu_busy()) {
        return -1;
    }

    if (cfg == HAL_CMU_LPU_CLK_26M) {
        lpu_clk = CMU_LPU_AUTO_SWITCH26;
    } else if (cfg == HAL_CMU_LPU_CLK_PLL) {
        lpu_clk = CMU_LPU_AUTO_SWITCHPLL | CMU_LPU_AUTO_SWITCH26;
    } else {
        lpu_clk = 0;
    }

    if (lpu_clk & CMU_LPU_AUTO_SWITCH26) {
        // Disable RAM wakeup early
        cmu->MCU_TIMER &= ~CMU_RAM_RETN_UP_EARLY;
        // MCU/ROM/RAM auto clock gating (which depends on RAM gating signal)
        cmu->HCLK_MODE &= ~(SYS_HCLK_CORE0 | SYS_HCLK_ROM0 | SYS_HCLK_RAM8 |
            SYS_HCLK_RAM0 | SYS_HCLK_RAM1 | SYS_HCLK_RAM2 | SYS_HCLK_RAM7 | SYS_HCLK_RAM3 |
            SYS_HCLK_RAM4 | SYS_HCLK_RAM5 | SYS_HCLK_RAM6);
        // AON_CMU enable auto switch 26M (AON_CMU must have selected 26M and disabled 52M/32K already)
        aoncmu->CLK_SELECT |= AON_CMU_LPU_AUTO_SWITCH26;
    } else {
        // AON_CMU disable auto switch 26M
        aoncmu->CLK_SELECT &= ~AON_CMU_LPU_AUTO_SWITCH26;
    }

    aoncmu->CLK_SELECT |= AON_CMU_TIMER_WT24_EN;
    aoncmu->CLK_SELECT |= AON_CMU_OSC_READY_MODE;
    aoncmu->CLK_SELECT |= AON_CMU_OSC_READY_BYPASS_SYNC;
    aoncmu->CLK_SELECT = SET_BITFIELD(aoncmu->CLK_SELECT, AON_CMU_TIMER_WT24, timer_26m);
    cmu->WAKEUP_CLK_CFG = CMU_TIMER_WT26(timer_26m) | CMU_TIMER_WTPLL(0) | lpu_clk;
    if (timer_pll) {
        hal_sys_timer_delay(US_TO_TICKS(60));
        cmu->WAKEUP_CLK_CFG = CMU_TIMER_WT26(timer_26m) | CMU_TIMER_WTPLL(timer_pll) | lpu_clk;
    }
    return 0;
}

void hal_cmu_select_rc_clock(void)
{
    //aoncmu->AON_CLK |= AON_CMU_SEL_CLK_RC;
}

void hal_cmu_select_osc_clock(void)
{
    //aoncmu->AON_CLK &= ~AON_CMU_SEL_CLK_RC;
}

#ifdef CHIP_ROLE_CP

static int SRAM_TEXT_LOC hal_cmu_lpu_sleep_cp(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    // CP does NOT support deep sleep
    __DSB();
    __WFI();

    return 0;
}

#else // !CHIP_ROLE_CP

#ifdef CORE_SLEEP_POWER_DOWN
static void SRAM_TEXT_LOC save_cmu_regs(struct SAVED_CMU_REGS_T *sav)
{
    sav->HCLK_ENABLE        = cmu->HCLK_ENABLE;
    sav->PCLK_ENABLE        = cmu->PCLK_ENABLE;
    sav->OCLK_ENABLE        = cmu->OCLK_ENABLE;
    sav->HCLK_MODE          = cmu->HCLK_MODE;
    sav->PCLK_MODE          = cmu->PCLK_MODE;
    sav->OCLK_MODE          = cmu->OCLK_MODE;
    sav->REG_RAM_CFG        = cmu->REG_RAM_CFG;
    sav->HRESET_CLR         = cmu->HRESET_CLR;
    sav->PRESET_CLR         = cmu->PRESET_CLR;
    sav->ORESET_CLR         = cmu->ORESET_CLR;
    sav->TIMER0_CLK         = cmu->TIMER0_CLK;
    sav->MCU_TIMER          = cmu->MCU_TIMER;
    sav->SLEEP              = cmu->SLEEP;
    sav->PERIPH_CLK         = cmu->PERIPH_CLK;
    sav->SYS_CLK_ENABLE     = cmu->SYS_CLK_ENABLE;
    sav->ADMA_CH15_REQ      = cmu->ADMA_CH15_REQ;
    sav->REG_RAM_CFG1       = cmu->REG_RAM_CFG1;
    sav->UART_CLK           = cmu->UART_CLK;
    sav->I2C_CLK            = cmu->I2C_CLK;
    sav->MCU2SENS_MASK0     = cmu->MCU2SENS_MASK0;
    sav->MCU2SENS_MASK1     = cmu->MCU2SENS_MASK1;
    sav->WAKEUP_CLK_CFG     = cmu->WAKEUP_CLK_CFG;
    sav->TIMER1_CLK         = cmu->TIMER1_CLK;
    sav->TIMER2_CLK         = cmu->TIMER2_CLK;
    sav->SYS_DIV            = cmu->SYS_DIV;
    sav->RESERVED_0AC       = cmu->RESERVED_0AC;
    sav->MCU2BT_INTMASK0    = cmu->MCU2BT_INTMASK0;
    sav->MCU2BT_INTMASK1    = cmu->MCU2BT_INTMASK1;
    sav->ADMA_CH0_4_REQ     = cmu->ADMA_CH0_4_REQ;
    sav->ADMA_CH5_9_REQ     = cmu->ADMA_CH5_9_REQ;
    sav->ADMA_CH10_14_REQ   = cmu->ADMA_CH10_14_REQ;
    sav->GDMA_CH0_4_REQ     = cmu->GDMA_CH0_4_REQ;
    sav->GDMA_CH5_9_REQ     = cmu->GDMA_CH5_9_REQ;
    sav->GDMA_CH10_14_REQ   = cmu->GDMA_CH10_14_REQ;
    sav->GDMA_CH15_REQ      = cmu->GDMA_CH15_REQ;
    sav->MISC               = cmu->MISC;
    sav->SIMU_RES           = cmu->SIMU_RES;
    sav->MISC_0F8           = cmu->MISC_0F8;
    sav->RESERVED_0FC       = cmu->RESERVED_0FC;
    sav->DSI_CLK_ENABLE     = cmu->DSI_CLK_ENABLE;
    sav->CP_VTOR            = cmu->CP_VTOR;
    sav->I2S0_CLK           = cmu->I2S0_CLK;
    sav->I2S1_CLK           = cmu->I2S1_CLK;
    sav->REG_RAM_CFG2       = cmu->REG_RAM_CFG2;
    sav->TPORT_IRQ_LEN      = cmu->TPORT_IRQ_LEN;
    sav->TPORT_CUR_ADDR     = cmu->TPORT_CUR_ADDR;
    sav->TPORT_START        = cmu->TPORT_START;
    sav->TPORT_END          = cmu->TPORT_END;
    sav->TPORT_CTRL         = cmu->TPORT_CTRL;
    sav->XCLK_ENABLE        = cmu->XCLK_ENABLE;
    sav->XCLK_MODE          = cmu->XCLK_MODE;
    sav->XRESET_CLR         = cmu->XRESET_CLR;
}

static void SRAM_TEXT_LOC restore_cmu_regs(const struct SAVED_CMU_REGS_T *sav)
{
    cmu->HRESET_SET         = ~sav->HRESET_CLR;
    cmu->PRESET_SET         = ~sav->PRESET_CLR;
    cmu->ORESET_SET         = ~sav->ORESET_CLR;
    cmu->XRESET_SET         = ~sav->XRESET_CLR;
    cmu->HCLK_DISABLE       = ~sav->HCLK_ENABLE;
    cmu->PCLK_DISABLE       = ~sav->PCLK_ENABLE;
    cmu->OCLK_DISABLE       = ~sav->OCLK_ENABLE;
    cmu->XCLK_DISABLE       = ~sav->XCLK_ENABLE;

    cmu->HCLK_ENABLE        = sav->HCLK_ENABLE;
    cmu->PCLK_ENABLE        = sav->PCLK_ENABLE;
    cmu->OCLK_ENABLE        = sav->OCLK_ENABLE;
    cmu->HCLK_MODE          = sav->HCLK_MODE;
    cmu->PCLK_MODE          = sav->PCLK_MODE;
    cmu->OCLK_MODE          = sav->OCLK_MODE;
    cmu->REG_RAM_CFG        = sav->REG_RAM_CFG;
    cmu->HRESET_CLR         = sav->HRESET_CLR;
    cmu->PRESET_CLR         = sav->PRESET_CLR;
    cmu->ORESET_CLR         = sav->ORESET_CLR;
    cmu->TIMER0_CLK         = sav->TIMER0_CLK;
    cmu->MCU_TIMER          = sav->MCU_TIMER;
    cmu->SLEEP              = sav->SLEEP;
    cmu->PERIPH_CLK         = sav->PERIPH_CLK;
    //cmu->SYS_CLK_ENABLE     = sav->SYS_CLK_ENABLE;
    cmu->ADMA_CH15_REQ      = sav->ADMA_CH15_REQ;
    cmu->REG_RAM_CFG1       = sav->REG_RAM_CFG1;
    cmu->UART_CLK           = sav->UART_CLK;
    cmu->I2C_CLK            = sav->I2C_CLK;
    cmu->MCU2SENS_MASK0     = sav->MCU2SENS_MASK0;
    cmu->MCU2SENS_MASK1     = sav->MCU2SENS_MASK1;
    cmu->WAKEUP_CLK_CFG     = sav->WAKEUP_CLK_CFG;
    cmu->TIMER1_CLK         = sav->TIMER1_CLK;
    cmu->TIMER2_CLK         = sav->TIMER2_CLK;
    cmu->SYS_DIV            = sav->SYS_DIV;
    cmu->RESERVED_0AC       = sav->RESERVED_0AC;
    cmu->MCU2BT_INTMASK0    = sav->MCU2BT_INTMASK0;
    cmu->MCU2BT_INTMASK1    = sav->MCU2BT_INTMASK1;
    cmu->ADMA_CH0_4_REQ     = sav->ADMA_CH0_4_REQ;
    cmu->ADMA_CH5_9_REQ     = sav->ADMA_CH5_9_REQ;
    cmu->ADMA_CH10_14_REQ   = sav->ADMA_CH10_14_REQ;
    cmu->GDMA_CH0_4_REQ     = sav->GDMA_CH0_4_REQ;
    cmu->GDMA_CH5_9_REQ     = sav->GDMA_CH5_9_REQ;
    cmu->GDMA_CH10_14_REQ   = sav->GDMA_CH10_14_REQ;
    cmu->GDMA_CH15_REQ      = sav->GDMA_CH15_REQ;
    cmu->MISC               = sav->MISC;
    cmu->SIMU_RES           = sav->SIMU_RES;
    cmu->MISC_0F8           = sav->MISC_0F8;
    cmu->RESERVED_0FC       = sav->RESERVED_0FC;
    cmu->DSI_CLK_ENABLE     = sav->DSI_CLK_ENABLE;
    cmu->CP_VTOR            = sav->CP_VTOR;
    cmu->I2S0_CLK           = sav->I2S0_CLK;
    cmu->I2S1_CLK           = sav->I2S1_CLK;
    cmu->REG_RAM_CFG2       = sav->REG_RAM_CFG2;
    cmu->TPORT_IRQ_LEN      = sav->TPORT_IRQ_LEN;
    cmu->TPORT_CUR_ADDR     = sav->TPORT_CUR_ADDR;
    cmu->TPORT_START        = sav->TPORT_START;
    cmu->TPORT_END          = sav->TPORT_END;
    cmu->TPORT_CTRL         = sav->TPORT_CTRL;
    cmu->XCLK_ENABLE        = sav->XCLK_ENABLE;
    cmu->XCLK_MODE          = sav->XCLK_MODE;
    cmu->XRESET_CLR         = sav->XRESET_CLR;
}

static int SRAM_TEXT_LOC hal_cmu_lpu_sleep_pd(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    uint32_t start;
    uint32_t timeout;
    uint32_t saved_hclk;
    uint32_t saved_oclk;
    uint32_t saved_top_clk;
    uint32_t saved_clk_cfg;
    bool wait_pll_locked;
    uint32_t cpu_regs[50];
    struct SAVED_CMU_REGS_T cmu_regs;

    NVIC_PowerDownSleep(cpu_regs, ARRAY_SIZE(cpu_regs));
    save_cmu_regs(&cmu_regs);

    saved_hclk = cmu_regs.HCLK_ENABLE;
    saved_oclk = cmu_regs.OCLK_ENABLE;

    saved_top_clk = aoncmu->TOP_CLK_ENABLE;
    saved_clk_cfg = cmu_regs.SYS_CLK_ENABLE;

    // Disable memory/flash clock
    cmu->OCLK_DISABLE = SYS_OCLK_FLASH | SYS_OCLK_FLASH1 | SYS_OCLK_DISPIX;
    cmu->HCLK_DISABLE = SYS_HCLK_FLASH | SYS_HCLK_FLASH1 | SYS_HCLK_LCDC;

    // Shutdown PLLs
    if (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLAUD_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLAUD_DISABLE;
    }
    if (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLBB_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLBB_DISABLE;
    }
    if (saved_top_clk & AON_CMU_PU_PLLUSB_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLUSB_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLUSB_DISABLE;
    }
    if (saved_top_clk & AON_CMU_PU_PLLDSI_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLDSI_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLDSI_DISABLE;
    }

    // Switch system freq to 26M
    cmu->SYS_CLK_ENABLE = CMU_SEL_SLOW_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE;
    cmu->SYS_CLK_DISABLE = CMU_SEL_OSC_2_SYS_DISABLE | CMU_SEL_FAST_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE;
    cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_SYS_DISABLE;
    cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_SYS_DISABLE;
    // Shutdown system PLL
    if (saved_clk_cfg & CMU_PU_PLL_ENABLE) {
        cmu->SYS_CLK_DISABLE = CMU_EN_PLL_DISABLE;
        cmu->SYS_CLK_DISABLE = CMU_PU_PLL_DISABLE;
    }

    // Set power down wakeup bootmode
    aoncmu->BOOTMODE = (aoncmu->BOOTMODE | HAL_SW_BOOTMODE_POWER_DOWN_WAKEUP) & HAL_SW_BOOTMODE_MASK;

    hal_sleep_core_power_down(false);

    while ((cmu->WAKEUP_CLK_CFG & CMU_LPU_STATUS_26M) == 0);

    // Switch system freq to 26M
    cmu->SYS_CLK_ENABLE = CMU_SEL_SLOW_SYS_ENABLE |
        (saved_clk_cfg & (CMU_PU_OSCX2_ENABLE | CMU_PU_OSCX4_ENABLE));
    cmu->SYS_CLK_DISABLE = CMU_SEL_OSC_2_SYS_DISABLE;
    // System freq is 26M now and will be restored later

    hal_sys_timer_wakeup();

    if (saved_clk_cfg & CMU_PU_PLL_ENABLE) {
        cmu->SYS_CLK_ENABLE = CMU_PU_PLL_ENABLE;
        start = hal_sys_timer_get();
        timeout = HAL_CMU_PLL_LOCKED_TIMEOUT;
        while ((hal_sys_timer_get() - start) < timeout);
        cmu->SYS_CLK_ENABLE = CMU_EN_PLL_ENABLE;
    }

    // Clear power down wakeup bootmode
    aoncmu->BOOTMODE = (aoncmu->BOOTMODE & ~HAL_SW_BOOTMODE_POWER_DOWN_WAKEUP) & HAL_SW_BOOTMODE_MASK;

    // Disable memory/flash clock
    cmu->OCLK_DISABLE = SYS_OCLK_FLASH | SYS_OCLK_FLASH1 | SYS_OCLK_DISPIX;
    cmu->HCLK_DISABLE = SYS_HCLK_FLASH | SYS_HCLK_FLASH1 | SYS_HCLK_LCDC;

    // Restore PLLs
    if (saved_top_clk & (AON_CMU_PU_PLLAUD_ENABLE | AON_CMU_PU_PLLUSB_ENABLE | AON_CMU_PU_PLLBB_ENABLE | AON_CMU_PU_PLLDSI_ENABLE)) {
        wait_pll_locked = false;
        if (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLAUD_ENABLE;
            wait_pll_locked = true;
        }
        if (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLBB_ENABLE;
            wait_pll_locked = true;
        }
        if (saved_top_clk & AON_CMU_PU_PLLUSB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLUSB_ENABLE;
            wait_pll_locked = true;
        }
        if (saved_top_clk & AON_CMU_PU_PLLDSI_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLDSI_ENABLE;
            wait_pll_locked = true;
        }
        if (wait_pll_locked) {
            start = hal_sys_timer_get();
            timeout = HAL_CMU_PLL_LOCKED_TIMEOUT;
            while ((hal_sys_timer_get() - start) < timeout);
        }
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE;
        }
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLBB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLBB_ENABLE;
        }
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE;
        }
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLDSI_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLDSI_ENABLE;
        }
    }

    // Restore system freq
    cmu->SYS_CLK_ENABLE = saved_clk_cfg &
        (CMU_RSTN_DIV_FLS0_ENABLE | CMU_RSTN_DIV_FLS1_ENABLE | CMU_RSTN_DIV_PSRAM_ENABLE |
        CMU_RSTN_DIV_DSI_ENABLE | CMU_RSTN_DIV_SYS_ENABLE);
    cmu->SYS_CLK_ENABLE = saved_clk_cfg &
        (CMU_BYPASS_DIV_FLS0_ENABLE | CMU_BYPASS_DIV_FLS1_ENABLE | CMU_BYPASS_DIV_PSRAM_ENABLE |
        CMU_BYPASS_DIV_DSI_ENABLE | CMU_BYPASS_DIV_SYS_ENABLE);
    cmu->SYS_CLK_ENABLE = saved_clk_cfg;
    cmu->SYS_CLK_DISABLE = ~saved_clk_cfg;

    if (saved_oclk & (SYS_OCLK_FLASH | SYS_OCLK_FLASH1 | SYS_OCLK_DISPIX)) {
        // Enable memory/flash clock
        cmu->HCLK_ENABLE = saved_hclk;
        cmu->OCLK_ENABLE = saved_oclk;
        // Wait until memory/flash clock ready
        hal_sys_timer_delay_us(2);
    }

    restore_cmu_regs(&cmu_regs);
    NVIC_PowerDownWakeup(cpu_regs, ARRAY_SIZE(cpu_regs));

    // TODO:
    // 1) Register pm notif handler for all hardware modules, e.g., sdmmc
    // 2) Recover system timer in rt_suspend() and rt_resume()
    // 3) Dynamically select 32K sleep or power down sleep

    return 0;
}
#endif

static int SRAM_TEXT_LOC hal_cmu_lpu_sleep_normal(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
    uint32_t start;
    uint32_t timeout;
    uint32_t saved_hclk;
    uint32_t saved_oclk;
    uint32_t saved_top_clk;
    uint32_t saved_clk_cfg;
    uint32_t saved_aon_clk;
    uint32_t wakeup_cfg;
    bool pd_aud_pll;
    bool pd_bb_pll;
    bool wait_pll_locked;

#if defined(MCU_USE_SENS_PERIPH)
    hal_cmu_sens_force_sleep();
#endif

    pd_aud_pll = true;
    pd_bb_pll = true;

    saved_hclk = cmu->HCLK_ENABLE;
    saved_oclk = cmu->OCLK_ENABLE;
    saved_aon_clk = aoncmu->AON_CLK;
    saved_top_clk = aoncmu->TOP_CLK_ENABLE;
    saved_clk_cfg = cmu->SYS_CLK_ENABLE;

    if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
        wakeup_cfg = cmu->WAKEUP_CLK_CFG;
    } else {
        wakeup_cfg = 0;
        if (pll_user_map[HAL_CMU_PLL_AUD] & (1 << HAL_CMU_PLL_USER_AUD)) {
            pd_aud_pll = false;
#ifndef ROM_BUILD
            if (sys_pll_sel == HAL_CMU_PLL_AUD) {
                aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLAUD_ENABLE | AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE;
            }
#endif
        }
        if (pll_user_map[HAL_CMU_PLL_BB] & (1 << HAL_CMU_PLL_USER_AUD)) {
            pd_bb_pll = false;
#ifndef ROM_BUILD
            if (sys_pll_sel == HAL_CMU_PLL_BB) {
                aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLBB_ENABLE | AON_CMU_EN_CLK_TOP_PLLBB_ENABLE;
            }
#endif
        }
        // Avoid auto-gating OSC and OSCX2
        aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_OSCX2_ENABLE | AON_CMU_EN_CLK_TOP_OSC_ENABLE;
    }

    // Disable memory/flash clock
    cmu->OCLK_DISABLE = SYS_OCLK_FLASH | SYS_OCLK_FLASH1 | SYS_OCLK_DISPIX;
    cmu->HCLK_DISABLE = SYS_HCLK_FLASH | SYS_HCLK_FLASH1 | SYS_HCLK_LCDC;

    // Setup wakeup mask
    cmu->WAKEUP_MASK0 = NVIC->ISER[0];
    cmu->WAKEUP_MASK1 = NVIC->ISER[1];

    if (wakeup_cfg & CMU_LPU_AUTO_SWITCH26) {
        // Enable auto memory retention
        cmu->SLEEP = (cmu->SLEEP & ~CMU_MANUAL_RAM_RETN) |
            CMU_DEEPSLEEP_EN | CMU_DEEPSLEEP_ROMRAM_EN | CMU_DEEPSLEEP_START;
    } else {
        // Disable auto memory retention
        cmu->SLEEP = (cmu->SLEEP & ~CMU_DEEPSLEEP_ROMRAM_EN) |
            CMU_DEEPSLEEP_EN | CMU_MANUAL_RAM_RETN | CMU_DEEPSLEEP_START;
    }

    // Shutdown PLLs
    if (pd_aud_pll && (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE)) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLAUD_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLAUD_DISABLE;
    }
    if (pd_bb_pll && (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE)) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLBB_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLBB_DISABLE;
    }
    if (saved_top_clk & AON_CMU_PU_PLLUSB_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLUSB_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLUSB_DISABLE;
    }
    if (saved_top_clk & AON_CMU_PU_PLLDSI_ENABLE) {
        aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_TOP_PLLDSI_DISABLE;
        aoncmu->TOP_CLK_DISABLE = AON_CMU_PU_PLLDSI_DISABLE;
    }

    if (wakeup_cfg & CMU_LPU_AUTO_SWITCHPLL) {
        // Do nothing
        // Hardware will switch system freq to 32K and shutdown PLLs automatically
    } else {
        // Switch system freq to 26M
        cmu->SYS_CLK_ENABLE = CMU_SEL_SLOW_SYS_ENABLE | CMU_SEL_OSCX2_SYS_ENABLE;
        cmu->SYS_CLK_DISABLE = CMU_SEL_OSC_2_SYS_DISABLE | CMU_SEL_FAST_SYS_DISABLE | CMU_SEL_PLL_SYS_DISABLE;
        cmu->SYS_CLK_DISABLE = CMU_BYPASS_DIV_SYS_DISABLE;
        cmu->SYS_CLK_DISABLE = CMU_RSTN_DIV_SYS_DISABLE;
        // Shutdown system PLL
        if (saved_clk_cfg & CMU_PU_PLL_ENABLE) {
            cmu->SYS_CLK_DISABLE = CMU_EN_PLL_DISABLE;
            cmu->SYS_CLK_DISABLE = CMU_PU_PLL_DISABLE;
        }
        if (wakeup_cfg & CMU_LPU_AUTO_SWITCH26) {
            // Do nothing
            // Hardware will switch system freq to 32K automatically
        } else {
            if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
#ifdef ANA_26M_X4_ENABLE
                cmu->SYS_CLK_DISABLE = CMU_EN_OSCX4_DISABLE;
                cmu->SYS_CLK_DISABLE = CMU_PU_OSCX4_DISABLE;
#endif
                cmu->SYS_CLK_DISABLE = CMU_EN_OSCX2_DISABLE;
                cmu->SYS_CLK_DISABLE = CMU_PU_OSCX2_DISABLE;
            }
            // Manually switch AON_CMU clock to 32K
            aoncmu->AON_CLK = saved_aon_clk & ~AON_CMU_SEL_CLK_OSC;
            // Switch system freq to 32K
            cmu->SYS_CLK_DISABLE = CMU_SEL_OSC_4_SYS_DISABLE | CMU_SEL_SLOW_SYS_DISABLE;
        }
    }

    if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
        SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;
    } else {
        SCB->SCR = 0;
    }
    __DSB();
    __WFI();

    if (wakeup_cfg & CMU_LPU_AUTO_SWITCHPLL) {
        start = hal_sys_timer_get();
        timeout = HAL_CMU_26M_READY_TIMEOUT + HAL_CMU_PLL_LOCKED_TIMEOUT + HAL_CMU_LPU_EXTRA_TIMEOUT;
        while ((cmu->WAKEUP_CLK_CFG & CMU_LPU_STATUS_PLL) == 0 &&
            (hal_sys_timer_get() - start) < timeout);
        // !!! CAUTION !!!
        // Hardware will switch system freq to PLL divider and enable PLLs automatically
    } else {
        // Wait for 26M ready
        if (wakeup_cfg & CMU_LPU_AUTO_SWITCH26) {
            start = hal_sys_timer_get();
            timeout = HAL_CMU_26M_READY_TIMEOUT + HAL_CMU_LPU_EXTRA_TIMEOUT;
            while ((cmu->WAKEUP_CLK_CFG & CMU_LPU_STATUS_26M) == 0 &&
                (hal_sys_timer_get() - start) < timeout);
            // Hardware will switch system freq to 26M automatically
        } else {
            if (mode == HAL_CMU_LPU_SLEEP_MODE_CHIP) {
                timeout = HAL_CMU_26M_READY_TIMEOUT;
                hal_sys_timer_delay(timeout);

                cmu->SYS_CLK_ENABLE = CMU_PU_OSCX2_ENABLE;
                hal_sys_timer_delay(US_TO_TICKS(60));
                cmu->SYS_CLK_ENABLE = CMU_EN_OSCX2_ENABLE;
#ifdef ANA_26M_X4_ENABLE
                cmu->SYS_CLK_ENABLE = CMU_PU_OSCX4_ENABLE;
                hal_sys_timer_delay(US_TO_TICKS(60));
                cmu->SYS_CLK_ENABLE = CMU_EN_OSCX4_ENABLE;
#endif
            }
            // Switch system freq to 26M
            cmu->SYS_CLK_ENABLE = CMU_SEL_SLOW_SYS_ENABLE;
            cmu->SYS_CLK_DISABLE = CMU_SEL_OSC_2_SYS_DISABLE;
            // Restore AON_CMU clock
            aoncmu->AON_CLK = saved_aon_clk;
        }
        // System freq is 26M now and will be restored later
        if (saved_clk_cfg & CMU_PU_PLL_ENABLE) {
            cmu->SYS_CLK_ENABLE = CMU_PU_PLL_ENABLE;
            start = hal_sys_timer_get();
            timeout = HAL_CMU_PLL_LOCKED_TIMEOUT;
            while ((hal_sys_timer_get() - start) < timeout);
            cmu->SYS_CLK_ENABLE = CMU_EN_PLL_ENABLE;
        }
    }

    // Restore PLLs
    if (saved_top_clk & (AON_CMU_PU_PLLAUD_ENABLE | AON_CMU_PU_PLLUSB_ENABLE | AON_CMU_PU_PLLBB_ENABLE | AON_CMU_PU_PLLDSI_ENABLE)) {
        wait_pll_locked = false;
        if (pd_aud_pll && (saved_top_clk & AON_CMU_PU_PLLAUD_ENABLE)) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLAUD_ENABLE;
            wait_pll_locked = true;
        }
        if (pd_bb_pll && (saved_top_clk & AON_CMU_PU_PLLBB_ENABLE)) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLBB_ENABLE;
            wait_pll_locked = true;
        }
        if (saved_top_clk & AON_CMU_PU_PLLUSB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLUSB_ENABLE;
            wait_pll_locked = true;
        }
        if (saved_top_clk & AON_CMU_PU_PLLDSI_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_PU_PLLDSI_ENABLE;
            wait_pll_locked = true;
        }
        if (wait_pll_locked) {
            start = hal_sys_timer_get();
            timeout = HAL_CMU_PLL_LOCKED_TIMEOUT;
            while ((hal_sys_timer_get() - start) < timeout);
        }
        if (pd_aud_pll && (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE)) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLAUD_ENABLE;
        }
        if (pd_bb_pll && (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLBB_ENABLE)) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLBB_ENABLE;
        }
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLUSB_ENABLE;
        }
        if (saved_top_clk & AON_CMU_EN_CLK_TOP_PLLDSI_ENABLE) {
            aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_TOP_PLLDSI_ENABLE;
        }
    }

#if defined(MCU_USE_SENS_PERIPH)
    hal_cmu_sens_force_wakeup();
#endif

    // Restore system freq
    cmu->SYS_CLK_ENABLE = saved_clk_cfg & CMU_RSTN_DIV_SYS_ENABLE;
    cmu->SYS_CLK_ENABLE = saved_clk_cfg & CMU_BYPASS_DIV_SYS_ENABLE;
    cmu->SYS_CLK_ENABLE = saved_clk_cfg;
    cmu->SYS_CLK_DISABLE = ~saved_clk_cfg;

    if (mode != HAL_CMU_LPU_SLEEP_MODE_CHIP) {
        // Restore the original top clock settings
        aoncmu->TOP_CLK_DISABLE = ~saved_top_clk;
    }

    if (saved_oclk & (SYS_OCLK_FLASH | SYS_OCLK_FLASH1 | SYS_OCLK_DISPIX)) {
        // Enable memory/flash clock
        cmu->HCLK_ENABLE = saved_hclk;
        cmu->OCLK_ENABLE = saved_oclk;
        // Wait until memory/flash clock ready
        hal_sys_timer_delay_us(2);
    }

    return 0;
}

#endif // !CHIP_ROLE_CP

int SRAM_TEXT_LOC hal_cmu_lpu_sleep(enum HAL_CMU_LPU_SLEEP_MODE_T mode)
{
#ifdef CHIP_ROLE_CP
    return hal_cmu_lpu_sleep_cp(mode);
#else
#ifdef CORE_SLEEP_POWER_DOWN
    if (mode == HAL_CMU_LPU_SLEEP_MODE_POWER_DOWN) {
        return hal_cmu_lpu_sleep_pd(mode);
    }
#endif
    return hal_cmu_lpu_sleep_normal(mode);
#endif
}

uint32_t hal_cmu_get_aon_chip_id(void)
{
    return aoncmu->CHIP_ID;
}

uint32_t BOOT_TEXT_FLASH_LOC hal_cmu_get_aon_revision_id(void)
{
    return GET_BITFIELD(aoncmu->CHIP_ID, AON_CMU_REVISION_ID);
}

void hal_cmu_cp_subsys_enable(uint32_t sp, uint32_t entry)
{
    struct CORE_STARTUP_CFG_T * cp_cfg;
    uint32_t cfg_addr;

    // Use (sp - 128) as the default vector. The Address must aligned to 128-byte boundary.
    cfg_addr = (sp - (1 << 7)) & ~((1 << 7) - 1);

    cmu->CP_VTOR = (cmu->CP_VTOR & ~CMU_VTOR_CORE1_MASK) | (cfg_addr & CMU_VTOR_CORE1_MASK);
    cp_cfg = (struct CORE_STARTUP_CFG_T *)cfg_addr;

    cp_cfg->stack = sp;
    cp_cfg->reset_hdlr = entry;

#if (defined(CP_SUBSYS_TIMER0_BASE) && CP_SUBSYS_TIMER0_BASE == MCU_TIMER2_BASE) || \
        (defined(CP_SUBSYS_TIMER1_BASE) && CP_SUBSYS_TIMER1_BASE == MCU_TIMER2_BASE)
    hal_cmu_clock_enable(HAL_CMU_MOD_O_TIMER2);
    hal_cmu_clock_enable(HAL_CMU_MOD_P_TIMER2);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_TIMER2);
    hal_cmu_reset_clear(HAL_CMU_MOD_P_TIMER2);
#endif

    hal_cmu_clock_enable(HAL_CMU_MOD_H_CP);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_CP);
}

void hal_cmu_cp_subsys_disable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_CP);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_CP);

#if (defined(CP_SUBSYS_TIMER0_BASE) && CP_SUBSYS_TIMER0_BASE == MCU_TIMER2_BASE) || \
        (defined(CP_SUBSYS_TIMER1_BASE) && CP_SUBSYS_TIMER1_BASE == MCU_TIMER2_BASE)
    hal_cmu_reset_set(HAL_CMU_MOD_P_TIMER2);
    hal_cmu_reset_set(HAL_CMU_MOD_O_TIMER2);
    hal_cmu_clock_disable(HAL_CMU_MOD_P_TIMER2);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_TIMER2);
#endif
}

void hal_cmu_cp_enable(uint32_t sp, uint32_t entry)
{
    cp_entry = entry;
    hal_cmu_cp_subsys_enable(sp, (uint32_t)system_cp_reset_handler);
}

void hal_cmu_cp_disable(void)
{
    hal_cmu_cp_subsys_disable();
}

uint32_t hal_cmu_cp_get_entry_addr(void)
{
    return cp_entry;
}

uint32_t hal_cmu_get_ram_boot_addr(void)
{
    return aoncmu->WAKEUP_PC;
}

void hal_cmu_sens_clock_enable(void)
{
    aoncmu->TOP_CLK1_ENABLE = AON_CMU_EN_CLK_32K_SENS_ENABLE;
    senscmu->HCLK_ENABLE = SENS_HCLK_MCU;
    aocmu_reg_update_wait();
}

void hal_cmu_sens_clock_disable(void)
{
    aoncmu->TOP_CLK1_DISABLE = AON_CMU_EN_CLK_32K_SENS_DISABLE;
}

void hal_cmu_sens_reset_set(void)
{
#ifdef NO_RAM_RETENTION_FSM
    uint32_t lock;

    lock = int_lock();
    aoncmu->RESERVED_03C |= AON_CMU_SENS_RAM_PG_ENABLE;
    int_unlock(lock);
#endif

    aoncmu->GBL_RESET_SET = AON_CMU_SOFT_RSTN_SENS_SET | AON_CMU_SOFT_RSTN_SENSCPU_SET;
}

void hal_cmu_sens_reset_clear(void)
{
#ifdef NO_RAM_RETENTION_FSM
    uint32_t lock;

    lock = int_lock();
    aoncmu->RESERVED_03C &= ~AON_CMU_SENS_RAM_PG_ENABLE;
    int_unlock(lock);
#endif

    aoncmu->GBL_RESET_CLR = AON_CMU_SOFT_RSTN_SENS_CLR;
    aocmu_reg_update_wait();
}

void hal_cmu_sens_start_cpu(uint32_t sp, uint32_t entry)
{
    volatile struct CORE_STARTUP_CFG_T *core_cfg;
    uint32_t cfg_addr;

    // Use (sp - 128) as the default vector. The Address must aligned to 128-byte boundary.
    cfg_addr = (sp - (1 << 7)) & ~((1 << 7) - 1);

    aoncmu->SENS_VTOR = (aoncmu->SENS_VTOR & ~AON_CMU_VTOR_CORE_SENS_MASK) | (cfg_addr & AON_CMU_VTOR_CORE_SENS_MASK);
    core_cfg = (volatile struct CORE_STARTUP_CFG_T *)cfg_addr;

    core_cfg->stack = sp;
    core_cfg->reset_hdlr = entry;
    // Flush previous write operation
    core_cfg->reset_hdlr;

    aoncmu->GBL_RESET_CLR = AON_CMU_SOFT_RSTN_SENSCPU_CLR;
}

void hal_cmu_sens_ram_cfg(void)
{
#if (SENS_RAMRET_BASE < (RAM8_BASE + RAM8_SIZE)) || (SENS_RAM_BASE < (RAM8_BASE + RAM8_SIZE))
    const uint32_t ram_base_list[] = { RAM4_BASE, RAM5_BASE, RAM6_BASE, RAM7_BASE, RAM8_BASE, RAMSENS_BASE, };
    const uint32_t sens_ram_start = SENS_RAMRET_BASE;
    const uint32_t sens_ram_end = sens_ram_start + SENS_RAMRET_SIZE + SENS_RAM_SIZE;
    const uint8_t ram_first_id = 4;
    uint32_t ram_map = 0;
    bool ram_inside = false;

    for (int i = 0; i < ARRAY_SIZE(ram_base_list) - 1; i++) {
        ram_inside = false;
        if (ram_base_list[i] <= sens_ram_start && sens_ram_start < ram_base_list[i + 1]) {
            ram_inside = true;
        } else if (ram_base_list[i] < sens_ram_end && sens_ram_end <= ram_base_list[i + 1]) {
            ram_inside = true;
        } else if (sens_ram_start <= ram_base_list[i] && ram_base_list[i + 1] < sens_ram_end) {
            ram_inside = true;
        }
        if (ram_inside) {
            ram_map |= (1 << (ram_first_id + i));
        }
    }
    hal_cmu_ram_cfg_sel_update(ram_map, HAL_CMU_RAM_CFG_SEL_SENS);
    TR_INFO(0, "%s: Set sensor ram map: 0x%X", __FUNCTION__, ram_map);
    ram_map = gen_sens_shr_ram_mask(SENS_RAMRET_BASE, SENS_RAMRET_SIZE + SENS_MAILBOX_SIZE);
    hal_psc_shr_ram_config(ram_map, HAL_PSC_SHR_RAM_SLEEP_PU);
#endif
}

void hal_cmu_ram_cfg_sel_update(uint32_t map, enum HAL_CMU_RAM_CFG_SEL_T sel)
{
    uint32_t val;
    int i;

    val = aoncmu->RAM_CFG_SEL;
    for (i = 0; i < 9; i++) {
        if (map & (1 << i)) {
            val = (val & ~(AON_CMU_SRAM0_CFG_SEL_MASK << (2 * i))) |
                (AON_CMU_SRAM0_CFG_SEL(sel) << (2 * i));
        }
    }
    aoncmu->RAM_CFG_SEL = val;
    aocmu_reg_update_wait();
    aocmu_reg_update_wait();
}

void hal_cmu_beco_enable(void)
{
    cmu->HCLK_ENABLE = SYS_HRST_CP0;
    cmu->HRESET_CLR  = SYS_HRST_CP0;
    aocmu_reg_update_wait();
}

void hal_cmu_beco_disable(void)
{
    cmu->HRESET_SET   = SYS_HRST_CP0;
    cmu->HCLK_DISABLE = SYS_HRST_CP0;
}

void hal_cmu_cp_beco_enable(void)
{
    cmu->XCLK_ENABLE = SYS_XRST_CP1;
    cmu->XRESET_CLR  = SYS_XRST_CP1;
    aocmu_reg_update_wait();
}

void hal_cmu_cp_beco_disable(void)
{
    cmu->XRESET_SET   = SYS_XRST_CP1;
    cmu->XCLK_DISABLE = SYS_XRST_CP1;
}

#ifdef DCDC_CLOCK_CONTROL
void hal_cmu_dcdc_clock_enable(enum HAL_CMU_DCDC_CLOCK_USER_T user)
{
    if (user >= HAL_CMU_DCDC_CLOCK_USER_QTY) {
        return;
    }

    if (user == HAL_CMU_DCDC_CLOCK_USER_GPADC) {
        pmu_big_bandgap_enable(PMU_BIG_BANDGAP_USER_GPADC, true);
    }

    if (dcdc_user_map == 0) {
        aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_DCDC0_ENABLE;
    }
    dcdc_user_map |= (1 << user);
}

void hal_cmu_dcdc_clock_disable(enum HAL_CMU_DCDC_CLOCK_USER_T user)
{
    if (user >= HAL_CMU_DCDC_CLOCK_USER_QTY) {
        return;
    }

    if (dcdc_user_map) {
        dcdc_user_map &= ~(1 << user);
        if (dcdc_user_map == 0) {
            aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_DCDC0_DISABLE;
        }
    }

    if (user == HAL_CMU_DCDC_CLOCK_USER_GPADC) {
        pmu_big_bandgap_enable(PMU_BIG_BANDGAP_USER_GPADC, false);
    }
}

void BOOT_TEXT_FLASH_LOC hal_cmu_boot_dcdc_clock_enable(void)
{
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_DCDC0_ENABLE;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_boot_dcdc_clock_disable(void)
{
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_DCDC0_DISABLE;
}
#endif

int hal_cmu_bt_trigger_set_handler(enum HAL_CMU_BT_TRIGGER_SRC_T src, HAL_CMU_BT_TRIGGER_HANDLER_T hdlr)
{
    if (src >= HAL_CMU_BT_TRIGGER_SRC_QTY) {
        return 1;
    }

    bt_trig_hdlr[src] = hdlr;
    return 0;
}

static void hal_cmu_bt_trigger_irq_handler(void)
{
    for (uint32_t i = 0; i < HAL_CMU_BT_TRIGGER_SRC_QTY; i++) {
        if (cmu->ISIRQ_SET & (CMU_BT_PLAYTIME_STAMP_INTR_MSK << i)) {
            cmu->ISIRQ_CLR = (CMU_BT_PLAYTIME_STAMP_INTR_CLR << i);
            if (bt_trig_hdlr[i]) {
                bt_trig_hdlr[i]((enum HAL_CMU_BT_TRIGGER_SRC_T)i);
            }
        }
    }
}

int hal_cmu_bt_trigger_enable(enum HAL_CMU_BT_TRIGGER_SRC_T src)
{
    uint32_t lock;
    uint32_t val;
    uint32_t i;

    if (src >= HAL_CMU_BT_TRIGGER_SRC_QTY) {
        return 1;
    }

    lock = int_lock();
    cmu->ISIRQ_CLR = (CMU_BT_PLAYTIME_STAMP_INTR_CLR << src);
    val = cmu->PERIPH_CLK;
    cmu->PERIPH_CLK = val | (CMU_BT_PLAYTIME_STAMP_MASK << src);
    for (i = 0; i < HAL_CMU_BT_TRIGGER_SRC_QTY; i++) {
        if (val & (CMU_BT_PLAYTIME_STAMP_MASK << i)) {
            break;
        }
    }
    if (i >= HAL_CMU_BT_TRIGGER_SRC_QTY) {
        NVIC_SetVector(BT_STAMP_IRQn, (uint32_t)hal_cmu_bt_trigger_irq_handler);
        NVIC_SetPriority(BT_STAMP_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_ClearPendingIRQ(BT_STAMP_IRQn);
        NVIC_EnableIRQ(BT_STAMP_IRQn);
    }
    int_unlock(lock);

    return 0;
}

int hal_cmu_bt_trigger_disable(enum HAL_CMU_BT_TRIGGER_SRC_T src)
{
    uint32_t lock;
    uint32_t val;
    uint32_t i;

    if (src >= HAL_CMU_BT_TRIGGER_SRC_QTY) {
        return 1;
    }

    lock = int_lock();
    val = cmu->PERIPH_CLK;
    val &= ~(CMU_BT_PLAYTIME_STAMP_MASK << src);
    cmu->PERIPH_CLK = val;
    for (i = 0; i < HAL_CMU_BT_TRIGGER_SRC_QTY; i++) {
        if (val & (CMU_BT_PLAYTIME_STAMP_MASK << i)) {
            break;
        }
    }
    if (i >= HAL_CMU_BT_TRIGGER_SRC_QTY) {
        NVIC_DisableIRQ(BT_STAMP_IRQn);
    }
    int_unlock(lock);

    return 0;
}

void BOOT_TEXT_FLASH_LOC hal_cmu_ispi_access_init(void)
{
    aoncmu->AON_CLK |= AON_CMU_EN_CLK_SPI_ARB;
}


#define GPU_USE_PLL

void hal_cmu_gpu_clock_enable(void)
{
    #ifdef GPU_USE_PLL
    {
    // CMU_CFG_DIV_GPU(value), valid value range is  0x0 ~ 0x3, gpu_clk = pll_clk/(value + 2)
    hal_cmu_pll_enable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_GPU);
    //cmu->LCDC_GPU_CLK |= CMU_SEL_PLL_GPU|CMU_CFG_DIV_GPU(0x2);  //100M
    cmu->LCDC_GPU_CLK |= CMU_SEL_PLL_GPU|CMU_CFG_DIV_GPU(0x3);    //80M
    }

    cmu->LCDC_GPU_CLK &=~(CMU_BYPASS_DIV_GPU);
    cmu->LCDC_GPU_CLK |=(CMU_RSTN_DIV_GPU);
    #else
    //cmu->LCDC_GPU_CLK |= CMU_SEL_SLOW_GPU; //24M
    //cmu->LCDC_GPU_CLK |= CMU_SEL_SLOW_GPU | CMU_SEL_OSC_2_GPU ; //12M
    cmu->LCDC_GPU_CLK |= CMU_SEL_FAST_GPU | CMU_SEL_OSCX2_GPU ; //48M
    #endif

    hal_sys_timer_delay_us(100);

    hal_cmu_clock_enable(HAL_CMU_MOD_H_GPU);
    hal_cmu_clock_enable(HAL_CMU_MOD_X_GPU);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_GPU);

    hal_cmu_reset_set(HAL_CMU_MOD_H_GPU);
    hal_sys_timer_delay_us(100);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_GPU);
    hal_cmu_reset_set(HAL_CMU_MOD_X_GPU);
    hal_sys_timer_delay_us(100);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_GPU);

    hal_cmu_reset_set(HAL_CMU_MOD_O_GPU);
    hal_sys_timer_delay_us(200);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_GPU);
}

void hal_cmu_gpu_clock_disable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_H_GPU);
    hal_cmu_reset_set(HAL_CMU_MOD_X_GPU);
    hal_cmu_reset_set(HAL_CMU_MOD_O_GPU);

    hal_cmu_clock_disable(HAL_CMU_MOD_H_GPU);
    hal_cmu_clock_disable(HAL_CMU_MOD_X_GPU);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_GPU);
    hal_cmu_pll_disable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_GPU);
}


void hal_cmu_dsi_sleep(void)
{
    hal_cmu_clock_disable(HAL_CMU_MOD_O_DISDSI);
    hal_cmu_pll_disable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_DSI);
}

void hal_cmu_dsi_wakeup(void)
{
    hal_cmu_pll_enable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_DSI);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_DISDSI);
}

void hal_cmu_dsi_phy_reset_set(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISPIX);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISDSI);
}

void hal_cmu_dsi_phy_reset_clear(void)
{
    hal_cmu_reset_clear(HAL_CMU_MOD_O_DISPIX);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_DISDSI);
}

void hal_cmu_dsi_clock_enable(void)
{
    hal_cmu_pll_enable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_DSI);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_DISDSI);

    hal_cmu_dsi_phy_reset_clear();
}

void hal_cmu_dsi_clock_disable(void)
{
    hal_cmu_dsi_phy_reset_set();

    hal_cmu_clock_disable(HAL_CMU_MOD_O_DISDSI);
    hal_cmu_pll_disable(HAL_CMU_PLL_USB, HAL_CMU_PLL_USER_DSI);
}

#define LCDC2D_USE_PLL_BB
#ifndef LCDC2D_USE_PLL_BB
#define LCDC2D_USE_OSCX1
//#define LCDC2D_USE_OSCX2
//#define LCDC2D_USE_OSCX4
#endif

enum HAL_CMU_LCDC2D_USER_T {
    HAL_CMU_LCDC2D_USER_LCDC = 0,
    HAL_CMU_LCDC2D_USER_DMA2D,
};

static uint8_t lcdc2d_clk_map;

static void hal_cmu_lcdc2d_mod_clock_enable(enum HAL_CMU_LCDC2D_USER_T user)
{
    uint8_t map = (1 << user);
    bool update = true;

    if (lcdc2d_clk_map) {
        update = false;
    }
    lcdc2d_clk_map |= map;

    if (!update) {
        return;
    }

#ifdef LCDC2D_USE_PLL_BB
    uint32_t div;

    hal_cmu_pll_enable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_LCDC);
    uint32_t bbmhz = 392;
    uint32_t dsimhz = 400;
    uint32_t pixbit = 24;
    uint32_t lane = 1;
#if defined(BBPLL_MHZ)
#if (BBPLL_MHZ == 400)
    bbmhz = 392;
#else
    bbmhz = BBPLL_MHZ;
#endif
#endif
#if defined(DSIPLL_MHZ)
    dsimhz = DSIPLL_MHZ;
#endif
#if defined(CONFIG_DSI_2LANE_MODE)
    lane = 2;
#endif
#if defined(CONFIG_DSIPHY_OUTFMT_RGB565)
    pixbit = 16;
#endif

    // DSIPLL_MHZ / pixbit <= bbmhz/2/(div+2)
    // l frame time = ((WIDTH+56)*HEIGHT) / (DSIPLL_MHZ*1000000/pixbit/1000)
    div = ((bbmhz/2)/((dsimhz*lane+pixbit)/pixbit))-2;
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    div *= 2;
#endif
    cmu->I2S1_CLK = SET_BITFIELD(cmu->I2S1_CLK, CMU_CFG_DIV_PIX, div);

    cmu->DSI_CLK_ENABLE = CMU_SEL_PLL_PIX_ENABLE | CMU_RSTN_DIV_PIX_ENABLE;
    cmu->DSI_CLK_DISABLE = CMU_BYPASS_DIV_PIX_DISABLE;
    cmu->LCDC_GPU_CLK |= CMU_SEL_OSCX4_LCDCP;
    cmu->LCDC_GPU_CLK &= ~(CMU_SEL_PLL_LCDCP | CMU_SEL_OSCX2_LCDCP);
#else

#endif
}

static void hal_cmu_lcdc2d_mod_clock_disable(enum HAL_CMU_LCDC2D_USER_T user)
{
    uint8_t map = (1 << user);

    lcdc2d_clk_map &= ~map;

    if (lcdc2d_clk_map) {
        return;
    }

    cmu->DSI_CLK_DISABLE = CMU_SEL_PLL_PIX_DISABLE | CMU_RSTN_DIV_PIX_DISABLE | CMU_BYPASS_DIV_PIX_DISABLE |
        CMU_SEL_OSCX4_PIX_DISABLE | CMU_SEL_OSCX2_PIX_DISABLE;

#ifdef LCDC2D_USE_PLL_BB
    hal_cmu_pll_disable(HAL_CMU_PLL_BB, HAL_CMU_PLL_USER_LCDC);
#endif
}


void hal_cmu_lcdc_sleep(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_O_2DPNTV);
    //hal_cmu_reset_set(HAL_CMU_MOD_O_DISDSI);
    //hal_cmu_reset_set(HAL_CMU_MOD_O_DISPIX);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISPN);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISTV);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_LCDC_P);
    hal_cmu_clock_disable(HAL_CMU_MOD_X_LCDC);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_LCDC);
    hal_cmu_clock_disable(HAL_CMU_MOD_P_DIS);
    hal_cmu_lcdc2d_mod_clock_disable(HAL_CMU_LCDC2D_USER_LCDC);
}

void hal_cmu_lcdc_wakeup(void)
{
    hal_cmu_lcdc2d_mod_clock_enable(HAL_CMU_LCDC2D_USER_LCDC);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_LCDC_P);
    hal_cmu_clock_enable(HAL_CMU_MOD_X_LCDC);
    hal_cmu_clock_enable(HAL_CMU_MOD_H_LCDC);
    hal_cmu_clock_enable(HAL_CMU_MOD_X_MISC);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_2DPNTV);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_DISPN);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_DISTV);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_DISPIX);
    hal_cmu_clock_enable(HAL_CMU_MOD_P_DIS);

    hal_cmu_reset_clear(HAL_CMU_MOD_O_2DPNTV);
    //hal_cmu_reset_clear(HAL_CMU_MOD_O_DISDSI);
    //hal_cmu_reset_clear(HAL_CMU_MOD_O_DISPIX);

    hal_cmu_reset_clear(HAL_CMU_MOD_O_DISPN);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_DISTV);
}

void hal_cmu_lcdc_clock_enable(void)
{
    hal_cmu_lcdc2d_mod_clock_enable(HAL_CMU_LCDC2D_USER_LCDC);
    hal_psc_display_enable(HAL_PSC_DISPLAY_MOD_LCDC);

    hal_cmu_clock_enable(HAL_CMU_MOD_O_LCDC_P);
    hal_cmu_clock_enable(HAL_CMU_MOD_X_LCDC);
    hal_cmu_clock_enable(HAL_CMU_MOD_H_LCDC);
    hal_cmu_clock_enable(HAL_CMU_MOD_X_MISC);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_2DPNTV);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_DISPN);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_DISTV);
    hal_cmu_clock_enable(HAL_CMU_MOD_O_DISPIX);
    hal_cmu_clock_enable(HAL_CMU_MOD_P_DIS);

    hal_cmu_reset_set(HAL_CMU_MOD_O_2DPNTV);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_2DPNTV);

    hal_cmu_reset_set(HAL_CMU_MOD_X_MISC);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_MISC);

    hal_cmu_reset_set(HAL_CMU_MOD_O_LCDC_P);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_LCDC_P);

    hal_cmu_reset_set(HAL_CMU_MOD_H_LCDC);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_LCDC);

    hal_cmu_reset_set(HAL_CMU_MOD_X_LCDC);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_LCDC);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISDSI);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_DISDSI);

    hal_cmu_reset_set(HAL_CMU_MOD_P_DIS);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_P_DIS);

    hal_cmu_reset_set(HAL_CMU_MOD_O_DISPIX);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_DISPIX);

    hal_cmu_reset_set(HAL_CMU_MOD_O_DISPN);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISTV);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_DISPN);
    hal_cmu_reset_clear(HAL_CMU_MOD_O_DISTV);
}

void hal_cmu_lcdc_clock_disable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_O_2DPNTV);
    hal_cmu_reset_set(HAL_CMU_MOD_X_MISC);
    hal_cmu_reset_set(HAL_CMU_MOD_O_LCDC_P);
    hal_cmu_reset_set(HAL_CMU_MOD_H_LCDC);
    hal_cmu_reset_set(HAL_CMU_MOD_X_LCDC);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISDSI);
    hal_cmu_reset_set(HAL_CMU_MOD_P_DIS);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISPIX);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISPN);
    hal_cmu_reset_set(HAL_CMU_MOD_O_DISTV);

    hal_cmu_clock_disable(HAL_CMU_MOD_O_LCDC_P);
    hal_cmu_clock_disable(HAL_CMU_MOD_X_LCDC);
    hal_cmu_clock_disable(HAL_CMU_MOD_H_LCDC);
    hal_cmu_clock_disable(HAL_CMU_MOD_X_MISC);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_2DPNTV);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_DISPN);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_DISTV);
    hal_cmu_clock_disable(HAL_CMU_MOD_O_DISPIX);
    hal_cmu_clock_disable(HAL_CMU_MOD_P_DIS);
    hal_cmu_lcdc2d_mod_clock_disable(HAL_CMU_LCDC2D_USER_LCDC);
    hal_psc_display_disable(HAL_PSC_DISPLAY_MOD_LCDC);
}

void hal_cmu_gamu_clock_enable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_X_VMMU);
    hal_sys_timer_delay_us(100);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_VMMU);
    hal_cmu_clock_enable(HAL_CMU_MOD_X_VMMU);
}

void hal_cmu_gamu_clock_disable(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_X_VMMU);
    hal_sys_timer_delay_us(100);
    hal_cmu_clock_disable(HAL_CMU_MOD_X_VMMU);
}

#endif

uint32_t hal_cmu_get_osc_ready_cycle_cnt(void)
{
    uint32_t cnt;

    // Only return HAL_CMU_26M_READY_TIMEOUT_US time,RC is not avaliable here!
    cnt = GET_BITFIELD(aoncmu->CLK_SELECT, AON_CMU_TIMER_WT24);

    return cnt;
}

uint32_t hal_cmu_get_osc_switch_overhead(void)
{
    if (aoncmu->AON_CLK & AON_CMU_EN_CLK_RC) {
        return 0;
    } else {
        return 6;
    }
}

void hal_cmu_auto_switch_rc_enable(void)
{
    aoncmu->AON_CLK |= AON_CMU_EN_CLK_RC;
}

void hal_cmu_auto_switch_rc_disable(void)
{
    aoncmu->AON_CLK &= ~AON_CMU_EN_CLK_RC;
}

void hal_cmu_bt_sys_set_freq(enum HAL_CMU_FREQ_T freq)
{
}

void hal_cmu_bt_sys_clock_force_on(void)
{
    uint32_t lock;

    lock = int_lock();
#ifndef NO_BT_RAM_BLK_RETENTION
    aoncmu->RESERVED_03C &= ~AON_CMU_BT_RAM_SLEEP_ALLOW;
#endif
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_BT_CLK_SYS_ENABLE;
    aoncmu->RAM3_CFG0 = AON_CMU_RAM3_RET1N0(0x3FF) | AON_CMU_RAM3_RET1N1(0x3FF);
    aoncmu->RESERVED_03C &= ~AON_CMU_BT_RAM_BLK_PG_AUTO(0xFF);
    aoncmu->REG_RAM_CFG &= ~(AON_CMU_SRAM_AUTO_EN_MODES_BT_EXT(0x3));
    int_unlock(lock);
    aocmu_reg_update_wait();
}

void hal_cmu_bt_sys_clock_auto(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_BT_CLK_SYS_DISABLE;
#ifndef NO_BT_RAM_BLK_RETENTION
    aoncmu->RESERVED_03C |= AON_CMU_BT_RAM_SLEEP_ALLOW;
#endif
    aoncmu->RAM3_CFG0 = AON_CMU_RAM3_RET1N0(0x3FF) | AON_CMU_RAM3_RET2N0(0x3FF);
    aoncmu->RESERVED_03C |= AON_CMU_BT_RAM_BLK_PG_AUTO(0xFF);
    aoncmu->REG_RAM_CFG |= (AON_CMU_SRAM_AUTO_EN_MODES_BT_EXT(0x3));
    int_unlock(lock);
}

void hal_cmu_bt_ram_cfg(void)
{
    uint32_t lock;

    // Power down last 8K BT ram
    lock = int_lock();
    aoncmu->RESERVED_03C &= ~AON_CMU_BT_RAM_LAST_8K_PG_AUTO;
    aoncmu->RAM4_CFG |= AON_CMU_RAM4_RET1N0(1 << 2) | AON_CMU_RAM4_PGEN1(1 << 2);
    int_unlock(lock);
}

void hal_cmu_bt_clock_enable(void)
{
    aoncmu->MOD_CLK_ENABLE = AON_CMU_MANUAL_OCLK_ENABLE(AON_OCLK_BTAON);
    aocmu_reg_update_wait();
    aoncmu->TOP_CLK_ENABLE = AON_CMU_EN_CLK_32K_BT_ENABLE;
    aocmu_reg_update_wait();
}

void hal_cmu_bt_clock_disable(void)
{
    aoncmu->MOD_CLK_DISABLE = AON_CMU_MANUAL_OCLK_DISABLE(AON_OCLK_BTAON);
    aoncmu->TOP_CLK_DISABLE = AON_CMU_EN_CLK_32K_BT_DISABLE;
}

void hal_cmu_bt_reset_set(void)
{
    uint32_t lock;

    lock = int_lock();
    aoncmu->RESET_SET = AON_CMU_ORESETN_SET(AON_ORST_BTAON);
    aoncmu->GBL_RESET_SET = AON_CMU_SOFT_RSTN_BT_SET | AON_CMU_SOFT_RSTN_BTCPU_SET;
#ifdef __BT_RAMRUN__
    aoncmu->RESERVED_03C |= AON_CMU_BT_RAMRUN_RAM_PG_AUTO;
#endif
    int_unlock(lock);
}

void hal_cmu_bt_reset_clear(void)
{
    uint32_t lock;

    lock = int_lock();
#ifdef NO_BT_RAM_BLK_RETENTION
    // Disable btc auto ram retention
    aoncmu->RAM_LP_TIMER &= ~(AON_CMU_PG_AUTO_EN_BT_REG);
#endif
#ifdef __BT_RAMRUN__
    aoncmu->RESERVED_03C &= ~AON_CMU_BT_RAMRUN_RAM_PG_AUTO;
    aoncmu->RAM2_CFG0 = (aoncmu->RAM2_CFG0 & ~ AON_CMU_RAM2_PGEN0(0xF << 5)) | AON_CMU_RAM2_RET1N0(0xF << 5);
#endif
    int_unlock(lock);

    aoncmu->RESET_CLR = AON_CMU_ORESETN_CLR(AON_ORST_BTAON);
    aocmu_reg_update_wait();
    aoncmu->GBL_RESET_CLR = AON_CMU_SOFT_RSTN_BT_CLR | AON_CMU_SOFT_RSTN_BTCPU_CLR;
    aocmu_reg_update_wait();
}

void hal_cmu_bt_module_init(void)
{
    //btcmu->CLK_MODE = 0;
}

#ifndef CHIP_SUBSYS_SENS
void BOOT_TEXT_SRAM_LOC hal_cmu_sens_force_sleep(void)
{
    uint32_t mask;

    while ((senscmu->WAKEUP_CLK_CFG & (SENS_CMU_LPU_AUTO_SWITCH26 | SENS_CMU_LPU_STATUS_26M)) ==
            SENS_CMU_LPU_AUTO_SWITCH26) {}

    // Disable RAM wakeup early
    senscmu->SENS_TIMER &= ~SENS_CMU_RAM_RETN_UP_EARLY;
    // MCU/ROM/RAM auto clock gating (which depends on RAM gating signal)
    senscmu->HCLK_MODE &= ~(SENS_HCLK_MCU | SENS_HCLK_RAM0 | SENS_HCLK_RAM1 | SENS_HCLK_RAM2 | SENS_HCLK_RAM3 |
            SENS_HCLK_RAM4 | SENS_HCLK_RAM5);
    senscmu->WAKEUP_CLK_CFG = SENS_CMU_TIMER_WT26(0) | SENS_CMU_TIMER_WTPLL(0) | SENS_CMU_LPU_AUTO_SWITCH26;

    senscmu->SENS2MCU_IRQ_SET = SENS_CMU_MCU2SENS_DATA_MSK_SET;
    mask = (1 << 26);

    senscmu->WAKEUP_MASK0 = mask;
    senscmu->WAKEUP_MASK1 = 0;
    // Enable auto memory retention
    senscmu->SLEEP = (senscmu->SLEEP & ~SENS_CMU_MANUAL_RAM_RETN) |
            SENS_CMU_DEEPSLEEP_EN | SENS_CMU_DEEPSLEEP_ROMRAM_EN | SENS_CMU_DEEPSLEEP_START;

#ifdef SENS_FORCE_SLEEP_DISABLE_CLOCK
    while ((senscmu->SLEEP & SENS_CMU_DEEPSLEEP_MODE) == 0) {}
    hal_cmu_sens_clock_disable();
#endif
}

void BOOT_TEXT_SRAM_LOC hal_cmu_sens_force_wakeup(void)
{
#ifdef SENS_FORCE_SLEEP_DISABLE_CLOCK
    hal_cmu_sens_clock_enable();
#endif

    cmu->MCU2SENS_IRQ_SET = CMU_MCU2SENS_DATA_IND_SET;

    while ((senscmu->WAKEUP_CLK_CFG & (SENS_CMU_LPU_AUTO_SWITCH26 | SENS_CMU_LPU_STATUS_26M)) ==
            SENS_CMU_LPU_AUTO_SWITCH26) {}

    cmu->MCU2SENS_IRQ_CLR = CMU_MCU2SENS_DATA_IND_CLR;
}

void hal_cmu_cap_sens_clock_enable(void)
{
    senscmu->I2C_CLK |= SENS_CMU_SEL_CAP_CLKIN;
}

void hal_cmu_cap_sens_clock_disable(void)
{
    senscmu->I2C_CLK &= ~SENS_CMU_SEL_CAP_CLKIN;
}

void hal_cmu_mcu_mask_sens_irq(void)
{
    cmu->ISIRQ_SET |= CMU_SENS_ALLIRQ_MASK_SET;
}

void hal_cmu_sens2mcu_irq_selcet(void)
{
    senscmu->SENS2MCU_MASK1 = SET_BITFIELD(senscmu->SENS2MCU_MASK1, SENS_CMU_SENS2MCU_INTISR_MASK1, 0x4000);
}
#endif
