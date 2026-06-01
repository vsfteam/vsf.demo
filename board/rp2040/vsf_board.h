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

//! \note User Level Board Configuration

#ifndef __VSF_BOARD_H__
#define __VSF_BOARD_H__

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "vsf_test_board_pins.h"

#if VSF_USE_TEST == ENABLED
#   include "component/test/vsf_test/vsf_test.h"
#endif

/*============================ MACROS ========================================*/

#define VSF_BOARD_USART_MAX     2
#define VSF_BOARD_I2C_MAX       4
#define VSF_BOARD_SPI_MAX       2

// Compile-time instance bindings for STATIC suite initializers.
// These are addresses of global variables → valid in static initializers.
// Override the NULL defaults in vsf_test.h.
#define VSF_BOARD_GPIO_INSTANCE     ((vsf_gpio_t *)&vsf_hw_gpio0)
#define VSF_BOARD_USART_INSTANCE    ((vsf_usart_t *)&vsf_hw_usart1)
// USART instance used by gpio_pinmux test (must be different from debug UART)
#define VSF_BOARD_PINMUX_USART_INSTANCE  ((vsf_usart_t *)&vsf_hw_usart1)
#define VSF_BOARD_SPI_INSTANCE      ((vsf_spi_t *)&vsf_hw_spi0)
#define VSF_BOARD_ADC_INSTANCE      ((vsf_adc_t *)&vsf_hw_adc0)
#define VSF_BOARD_PWM_INSTANCE      ((vsf_pwm_t *)&vsf_hw_pwm0)
#define VSF_BOARD_TIMER_INSTANCE    ((vsf_timer_t *)&vsf_hw_timer0)
#define VSF_BOARD_RTC_INSTANCE      ((vsf_rtc_t *)&vsf_hw_rtc0)
#define VSF_BOARD_WDT_INSTANCE      ((vsf_wdt_t *)&vsf_hw_wdt0)
#define VSF_BOARD_RNG_INSTANCE      ((vsf_rng_t *)&vsf_hw_rng0)
#define VSF_BOARD_DMA_INSTANCE      ((vsf_dma_t *)&vsf_hw_dma0)
#define VSF_BOARD_FLASH_INSTANCE    ((vsf_flash_t *)&vsf_hw_flash0)

// Multi-instance I2C (I2C0=HW on GP20/21, I2C1=HW on GP18/19,
// I2C2=GPIO bit-bang on GP20/21, I2C3=GPIO bit-bang on GP18/19)
#define VSF_BOARD_I2C0_INSTANCE     ((vsf_i2c_t *)&vsf_hw_i2c0)
#define VSF_BOARD_I2C1_INSTANCE     ((vsf_i2c_t *)&vsf_hw_i2c1)
#define VSF_BOARD_GPIO_I2C0_INSTANCE ((vsf_i2c_t *)&vsf_gpio_i2c0)
#define VSF_BOARD_GPIO_I2C1_INSTANCE ((vsf_i2c_t *)&vsf_gpio_i2c1)

/*============================ GPIO I2C ======================================*/

#include "hal/driver/vsf/i2c/gpio_i2c/vsf_gpio_i2c.h"
extern vsf_gpio_i2c_t vsf_gpio_i2c0;
extern vsf_gpio_i2c_t vsf_gpio_i2c1;

/*============================ TYPES =========================================*/

typedef struct vsf_board_t {
    // multi-instance peripherals
    vsf_usart_t    *usarts[VSF_BOARD_USART_MAX];
    vsf_i2c_t      *i2cs[VSF_BOARD_I2C_MAX];
    vsf_spi_t      *spis[VSF_BOARD_SPI_MAX];

    // single-instance peripherals (NULL = absent)
    vsf_gpio_t  *gpio;
    vsf_flash_t *flash;
    vsf_adc_t   *adc;
    vsf_pwm_t   *pwm0;
    vsf_timer_t *timer0;
    vsf_rtc_t   *rtc;
    vsf_wdt_t   *wdt;
    vsf_rng_t   *rng;
    vsf_dma_t   *dma;
} vsf_board_t;

/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_board_t vsf_board;

#if VSF_USE_TEST == ENABLED
extern const vsf_test_inst_t vsf_board_test_instances[];
extern uint8_t vsf_board_test_instance_count;
#endif

/*============================ PROTOTYPES ====================================*/

extern void vsf_board_init(void);

uint8_t vsf_board_get_uart_funcsel(void);

#endif      // __VSF_BOARD_H__
