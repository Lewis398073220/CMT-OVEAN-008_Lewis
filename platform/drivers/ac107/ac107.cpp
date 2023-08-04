#include "plat_types.h"
#include "stdlib.h"
#include "cmsis_os.h"
#include "hal_trace.h" 
#include "hwtimer_list.h"
#include "hal_timer.h" 
#include "hal_iomux.h" 
#include "hal_gpio.h" 
#include "hal_i2c.h"
#include "string.h"
#include "btapp.h"
#include "app_status_ind.h"
#include "hal_codec.h"
#include "apps.h"

#include "ac107.h"

#include "pmu.h"

#if defined(__AC107_ADC__)

#define AC107_DEBUG_EN
 #ifdef AC107_DEBUG_EN
#define AC107_DEBUG(n,str, ...)	 	TRACE(n,str, ##__VA_ARGS__)
#else
#define AC107_DEBUG(n,str, ...)  	TRACE_DUMMY(n,str, ##__VA_ARGS__)
#endif

// AC107 register config data.
const unsigned char AC107_Register_48khz_Config[][2]={
        {0x00, 0x12}, 
        {0x01, 0x80}, 
        {0x02, 0x55},  /* Power */
        // {0x10, 0x4B}, 
        // {0x11, 0x00}, 
        // {0x12, 0x01}, 
        // {0x13, 0xE0}, 
        // {0x14, 0x13}, 
        // {0x16, 0x0F}, 
        // {0x17, 0xD0}, 
        // {0x18, 0x00},  /* PLL */
        {0x20, 0x01}, // PLL Disable, SYSCLK_SRC = MCLK, SYSCLK_EN = 1
        {0x21, 0x07}, 
        {0x22, 0x03},  /* Clock */
        {0x30, 0x15},  //BCLK input, LRCK input, MCLK input, TX EN, GEN
        {0x31, 0x03},  //BCLK div 4
        {0x32, 0x10},  //FS = 32, 0x20-1=0x1f
        {0x33, 0x1F},  //FS = 32, 0x20-1=0x1f
        {0x34, 0x17},  //Encoding disable, left-justified
        {0x35, 0x75},  //SW=32 SR=24
        {0x36, 0x60}, 
        {0x38, 0x01},  //2ch
        {0x39, 0x03},  //enable ch1,ch2
        {0x3A, 0x00}, 
        {0x3C, 0x02},  //ch mapping
        {0x3D, 0x00},  /* I2S */
        {0x60, 0x08},  //ADC sampling rate 48khz
        {0x61, 0x07}, 
        {0x66, 0x03}, 
        {0x70, 0xA0}, 
        {0x71, 0xA0}, 
        {0x76, 0x01}, 
        {0x77, 0x02}, 
        {0x7F, 0x00},  /* ADC */
        {0xA0, 0x00}, 
        {0xA1, 0x00}, 
        {0xA2, 0x01}, //PGA Gain 0dB
        {0xA3, 0x00}, 
        {0xA4, 0x40}, 
        {0xA5, 0x00}, 
        {0xA6, 0x00}, 
        {0xA7, 0x01}, //PGA Gain 0dB
        {0xA8, 0x00}, 
        {0xA9, 0x40}, 
        {0xAA, 0x00}, /* Analog */
};

#define AC107_SLAVER_ADDR     0x36
#define wf_delay 10//2


//#define i2c_gpio              // Enable and Disable, I2C all can work. 
//#define AC107_REG_DEBUG_DATA  // Debug data.

#ifdef i2c_gpio
struct HAL_GPIO_I2C_CONFIG_T cfg_gpio_i2c_cmt1={
	HAL_GPIO_PIN_P0_4,HAL_GPIO_PIN_P0_5,100000
};
#else
static struct HAL_I2C_CONFIG_T _codec_i2c_cfg1;
#endif

void ac107_i2c_init(void)
{
#ifdef i2c_gpio
	hal_gpio_i2c_open(&cfg_gpio_i2c_cmt1);
#else
	hal_iomux_set_i2c0();
	
	_codec_i2c_cfg1.mode = HAL_I2C_API_MODE_SIMPLE;
	_codec_i2c_cfg1.use_dma	= 0;
	_codec_i2c_cfg1.use_sync = 1;
	_codec_i2c_cfg1.speed = 100000;
	_codec_i2c_cfg1.as_master = 1;
	hal_i2c_open(HAL_I2C_ID_0, &_codec_i2c_cfg1);
#endif
}

void ac107_delay_ms(uint16_t Nms)
{
    hal_sys_timer_delay(MS_TO_TICKS(Nms));
}

static uint32_t ac107_write(uint8_t uchDeviceId, const uint8_t uchWriteBytesArr[], uint16_t usWriteLen)
{
#ifdef i2c_gpio	
	return(hal_gpio_i2c_simple_send(&cfg_gpio_i2c_cmt1, uchDeviceId, uchWriteBytesArr, usWriteLen));
#else
	return (hal_i2c_simple_send(HAL_I2C_ID_0, uchDeviceId, uchWriteBytesArr, usWriteLen));
#endif
}

#ifdef AC107_REG_DEBUG_DATA
static uint32_t ac107_read(uint8_t uchDeviceId, const uint8_t uchCmddBytesArr[], uint16_t usCmddLen, uint8_t uchReadBytesArr[], uint16_t usMaxReadLen)
{
#ifdef i2c_gpio	
	return (hal_gpio_i2c_simple_recv(&cfg_gpio_i2c_cmt1, uchDeviceId, uchCmddBytesArr, usCmddLen, uchReadBytesArr, usMaxReadLen));	
#else
	return (hal_i2c_simple_recv(HAL_I2C_ID_0, uchDeviceId, uchCmddBytesArr, usCmddLen, uchReadBytesArr, usMaxReadLen));	
#endif
}
#endif /*AC107_REG_DEBUG_DATA*/

/*
static void ac107_rst_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pin;
	
	pin.pin = AC107_RST_PIN;
	pin.function = HAL_IOMUX_FUNC_GPIO;
	pin.pull_sel = HAL_IOMUX_PIN_PULLUP_ENALBE;
	pin.volt = HAL_IOMUX_PIN_VOLTAGE_VIO;
	hal_iomux_init(&pin, 1);
	hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)AC107_RST_PIN, HAL_GPIO_DIR_OUT, 0);
}

static void hal_set_ac107_rst_high(void)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)AC107_RST_PIN);
}

static void hal_set_ac107_rst_low(void)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)AC107_RST_PIN);
}
*/

void ac107_hw_close(void)
{
    pmu_vio_3v3(false);

#if defined(__USE_LDO_CTL__)
	hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)cfg_hw_ldo_ctl.pin);
#endif
}

void ac107_hw_open(void)
{
    pmu_vio_3v3(true);

    //ac107_i2c_init();
#if defined(__USE_LDO_CTL__)
	hal_gpio_pin_set((enum HAL_GPIO_PIN_T)cfg_hw_ldo_ctl.pin);
#endif
}

void ac107_hw_init(void)
{
    AC107_DEBUG(1,"%s",__func__);
	uint8_t write_cmd[2] = {0};
    int i =0;
    //int h = (int)(sizeof(AC107_Register_48khz_Config))/2;
    for (i = 0; i < (int)(sizeof(AC107_Register_48khz_Config))/2; i++)
    {
        write_cmd[0] = (uint8_t)AC107_Register_48khz_Config[i][0];
        write_cmd[1] = (uint8_t)AC107_Register_48khz_Config[i][1];
        ac107_write(AC107_SLAVER_ADDR, write_cmd, 2);
        ac107_delay_ms(wf_delay);
    }

#ifdef AC107_REG_DEBUG_DATA
    ac107_delay_ms(100);
    uint8_t read_byte=0;
    int j =0;
    for (j = 0; j < (int)(sizeof(AC107_Register_48khz_Config))/2; j++)
    {
        write_cmd[0] = (uint8_t)AC107_Register_48khz_Config[j][0];
        ac107_read(AC107_SLAVER_ADDR, write_cmd, 1, &read_byte, 1);
        AC107_DEBUG(2,"Reg:0x%2x, data: 0x%2x",write_cmd[0],read_byte);
        ac107_delay_ms(wf_delay);
    }
#endif /*AC107_REG_DEBUG_DATA*/
}

#endif
