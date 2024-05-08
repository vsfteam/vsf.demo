/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/

#include "./vsf_board.h"

/*============================ MACROS ========================================*/

#if     !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT)                            \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_OUT_PIN)                         \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_IN_PIN)                          \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_PL_PIN)                          \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_CLK_PIN)                         \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_OE_PIN)
#   error current board does not support gamepad extension
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ GLOBAL VARIABLES ==============================*/

static void __74hc_load_control(void *param, uint_fast8_t bit);
static void __74hc_clock_control(void *param, uint_fast8_t bit);
static uint_fast8_t __74hc_serial_input(void *param);
#define __74hc_latch_control            __74hc_load_control
static void __74hc_oe_control(void *param, uint_fast8_t bit);
static void __74hc_clock_control(void *param, uint_fast8_t bit);
static void __74hc_serial_output(void *param, uint_fast8_t bit);

#if VSF_HAL_USE_74HC165_GPIO == ENABLED
describe_74hc165_gpio(__gpio_74hc165, 2, NULL,
    NULL,
    __74hc_load_control,
    __74hc_clock_control,
    __74hc_serial_input
)
#endif

#if VSF_HAL_USE_74HC595_GPIO == ENABLED
describe_74hc595_gpio(__gpio_74hc595, 1, NULL,
    __74hc_oe_control,
    __74hc_latch_control,
    __74hc_clock_control,
    __74hc_serial_output
)

#   if VSF_HAL_USE_TIMER_GPIO_PWM == ENABLED
static void __pwm_led0_gpio_control(uint_fast8_t value)
{
    vsf_gpio_write(&__gpio_74hc595, 1 << VSF_BOARD_GPIO_OUT_LED0_PIN, value ? 1 << VSF_BOARD_GPIO_OUT_LED0_PIN : 0);
}
describe_timer_gpio_single_pwm(__pwm_led0, &vsf_hw_timer0, __pwm_led0_gpio_control)
#   endif
#endif

#if VSF_HAL_USE_ADS7830_ADC == ENABLED
describe_ads7830_adc(__adc_joystick, NULL, 0x48)
#endif

#if VSF_HAL_USE_I2C == ENABLED
describe_multiplexer_i2c(__i2c_multiplexer, NULL, __i2c_adc)
#endif

vsf_board_t vsf_board_gamepad = {
#if VSF_HAL_USE_74HC165_GPIO == ENABLED
    .gpio_input                 = (vsf_gpio_t *)&__gpio_74hc165,
#endif
#if VSF_HAL_USE_74HC595_GPIO == ENABLED
    .gpio_output                = (vsf_gpio_t *)&__gpio_74hc595,
#   if VSF_HAL_USE_TIMER_GPIO_PWM == ENABLED
    .pwm_led[0]                 = (vsf_pwm_t *)&__pwm_led0,
#   endif
#endif

#if VSF_HAL_USE_SPI == ENABLED
    .spi                        = (vsf_spi_t *)&vsf_hw_spi0,
#endif
#if VSF_HAL_USE_I2C == ENABLED
#   if      defined(SE_CFG_I2C_GPIO_PORT)                                       \
        &&  defined(SE_CFG_I2C_GPIO_SCL_PIN) && defined(SE_CFG_I2C_GPIO_SDA_PIN)
    .i2c                        = (vsf_i2c_t *)&__gpio_i2c,
#   else
    .i2c                        = (vsf_i2c_t *)&vsf_hw_i2c0,
#   endif
#   if VSF_HAL_USE_ADS7830_ADC == ENABLED
    .i2c_adc                    = (vsf_i2c_t *)&__i2c_adc,
#   endif
#endif
#if VSF_HAL_USE_ADC == ENABLED && VSF_HAL_USE_ADS7830_ADC == ENABLED
    .adc                        = (vsf_adc_t *)&__adc_joystick,
#endif
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

static void __74hc_load_control(void *param, uint_fast8_t bit)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_PL_PIN;
    vsf_gpio_write(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT, mask, bit ? mask : 0);
}

static void __74hc_clock_control(void *param, uint_fast8_t bit)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_CLK_PIN;
    vsf_gpio_write(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT, mask, bit ? mask : 0);
}

static uint_fast8_t __74hc_serial_input(void *param)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_IN_PIN;
    return vsf_gpio_read(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT) & mask ? 1 : 0;
}

static void __74hc_oe_control(void *param, uint_fast8_t bit)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_OE_PIN;
    vsf_gpio_write(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT, mask, bit ? mask : 0);
}

static void __74hc_serial_output(void *param, uint_fast8_t bit)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_OUT_PIN;
    vsf_gpio_write(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT, mask, bit ? mask : 0);
}

void vsf_board_ext_gamepad_init(void)
{

#if VSF_HAL_USE_74HC165_GPIO == ENABLED || VSF_HAL_USE_74HC595_GPIO == ENABLED
    vsf_gpio_set_output(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT,
                            (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_PL_PIN)
                        |   (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_CLK_PIN));
#   if VSF_HAL_USE_74HC165_GPIO == ENABLED
    vsf_gpio_set_input(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT,
                            (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_IN_PIN));
    vsf_74hc165_gpio_init(&__gpio_74hc165);
    vsf_gpio_set_input(vsf_board.gpio_input, 0xFFFF);
#   endif

#   if VSF_HAL_USE_74HC595_GPIO == ENABLED
    vsf_gpio_set_output(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT,
                            (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_OE_PIN)
                        |   (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_OUT_PIN));
    vsf_74hc595_gpio_init(&__gpio_74hc595);
    vsf_gpio_clear(vsf_board.gpio_output, 0xFF);
    vsf_gpio_set_output(vsf_board.gpio_output, 0xFF);
#   endif
#endif

#if VSF_HAL_USE_I2C == ENABLED
    __i2c_multiplexer.i2c_ptr = vsf_board.i2c;
#   if VSF_HAL_USE_ADS7830_ADC == ENABLED
    __adc_joystick.i2c = vsf_board.i2c_adc;
#   endif
#endif
}
