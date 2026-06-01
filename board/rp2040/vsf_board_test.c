/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *   You may obtain a copy of the License at                                 *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/

#include "./vsf_board.h"

// for timer_hw (used to override vsf_test_busy_wait_ms)
#include "hardware/structs/timer.h"
#include "hardware/structs/watchdog.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

#if VSF_USE_TEST == ENABLED

void vsf_test_hw_config(vsf_peripheral_type_t peripheral_type, const vsf_test_inst_t *inst, bool init)
{
    VSF_TEST_TRACE_DEBUG("hw_config: type=%d inst=%p init=%d" VSF_TRACE_CFG_LINEEND,
                         (int)peripheral_type, (const void *)inst, (int)init);
    switch (peripheral_type) {
    case VSF_PERIPHERAL_TYPE_USART:
        if (init) {
            VSF_TEST_TRACE_DEBUG("hw_config: USART GP8/GP9" VSF_TRACE_CFG_LINEEND);
            vsf_hw_gpio_ports_config_pin(
                (vsf_gpio_port_cfg_pin_t []){ {VSF_PA8, VSF_GPIO_AF, GPIO_FUNC_UART},
                                              {VSF_PA9, VSF_GPIO_AF, GPIO_FUNC_UART} }, 2);
            /* RP2040 IO_BANK0 pinmux takes effect asynchronously. Without a
             * settle delay the first UART1 RX after boot can latch a
             * spurious start bit from the GPIO→AF transition on GP9. */
            vsf_test_busy_wait_ms(1);
        }
        break;

    case VSF_PERIPHERAL_TYPE_I2C:
        if (init) {
            if (inst->fixture.i2c == VSF_BOARD_I2C0_INSTANCE) {
                VSF_TEST_TRACE_DEBUG("hw_config: I2C0 GP20=SDA GP21=SCL" VSF_TRACE_CFG_LINEEND);
                vsf_hw_gpio_ports_config_pin(
                    (vsf_gpio_port_cfg_pin_t []){ {VSF_PA20, VSF_GPIO_AF, GPIO_FUNC_I2C},
                                                  {VSF_PA21, VSF_GPIO_AF, GPIO_FUNC_I2C} }, 2);
            } else if (inst->fixture.i2c == VSF_BOARD_I2C1_INSTANCE) {
                VSF_TEST_TRACE_DEBUG("hw_config: I2C1 GP18=SCL GP19=SDA" VSF_TRACE_CFG_LINEEND);
                vsf_hw_gpio_ports_config_pin(
                    (vsf_gpio_port_cfg_pin_t []){ {VSF_PA18, VSF_GPIO_AF, GPIO_FUNC_I2C},
                                                  {VSF_PA19, VSF_GPIO_AF, GPIO_FUNC_I2C} }, 2);
            } else if (inst->fixture.i2c == VSF_BOARD_GPIO_I2C0_INSTANCE) {
                VSF_TEST_TRACE_DEBUG("hw_config: GPIO_I2C0 GP20=SDA GP21=SCL (bit-bang)" VSF_TRACE_CFG_LINEEND);
                // GPIO I2C configures pins internally in vsf_gpio_i2c_init();
            } else if (inst->fixture.i2c == VSF_BOARD_GPIO_I2C1_INSTANCE) {
                VSF_TEST_TRACE_DEBUG("hw_config: GPIO_I2C1 GP19=SCL GP18=SDA (bit-bang)" VSF_TRACE_CFG_LINEEND);
                // GPIO I2C configures pins internally in vsf_gpio_i2c_init();
            }
        }
        break;

    case VSF_PERIPHERAL_TYPE_I2C_SLAVE:
        if (init) {
            VSF_TEST_TRACE_DEBUG("hw_config: I2C_SLAVE GP18-21" VSF_TRACE_CFG_LINEEND);
            vsf_hw_gpio_ports_config_pin(
                (vsf_gpio_port_cfg_pin_t []){ {VSF_PA18, VSF_GPIO_AF, GPIO_FUNC_I2C},
                                              {VSF_PA19, VSF_GPIO_AF, GPIO_FUNC_I2C},
                                              {VSF_PA20, VSF_GPIO_AF, GPIO_FUNC_I2C},
                                              {VSF_PA21, VSF_GPIO_AF, GPIO_FUNC_I2C} }, 4);
        }
        break;

    case VSF_PERIPHERAL_TYPE_SPI:
        if (init) {
            VSF_TEST_TRACE_DEBUG("hw_config: SPI GP2-5" VSF_TRACE_CFG_LINEEND);
            /* Flash ROM ops leave GP2-5 in QSPI mode. Reset and re-assert SPI. */
            resets_hw->reset |= RESETS_RESET_SPI0_BITS;
            resets_hw->reset &= ~RESETS_RESET_SPI0_BITS;
            while (!(resets_hw->reset_done & RESETS_RESET_SPI0_BITS));
            vsf_hw_gpio_ports_config_pin(
                (vsf_gpio_port_cfg_pin_t []){ {VSF_PA2, VSF_GPIO_AF, GPIO_FUNC_SPI},
                                              {VSF_PA3, VSF_GPIO_AF, GPIO_FUNC_SPI},
                                              {VSF_PA4, VSF_GPIO_AF, GPIO_FUNC_SPI},
                                              {VSF_PA5, VSF_GPIO_AF, GPIO_FUNC_SPI} }, 4);
        }
        break;

    case VSF_PERIPHERAL_TYPE_DMA:
        if (!init) {
            vsf_dma_init(vsf_board.dma, &(vsf_dma_cfg_t){0});
        }
        break;

    default:
        break;
    }
}

static const vsf_test_i2c_slave_ctx_t vsf_board_i2c_slave_ctx = {
    .master_i2c = VSF_BOARD_I2C0_INSTANCE,
    .slave_i2c  = VSF_BOARD_GPIO_I2C0_INSTANCE,
};

static const vsf_test_gpio_pinmux_ctx_t vsf_board_gpio_pinmux_ctx = {
    .gpio  = VSF_BOARD_GPIO_INSTANCE,
    .usart = VSF_BOARD_PINMUX_USART_INSTANCE,
};

const vsf_test_inst_t vsf_board_test_instances[] = {
    { .peripheral_type = VSF_PERIPHERAL_TYPE_I2C,        .fixture.i2c        = VSF_BOARD_I2C0_INSTANCE,      .name = "I2C0 (HW)" },
//  { .peripheral_type = VSF_PERIPHERAL_TYPE_I2C,        .fixture.i2c        = VSF_BOARD_I2C1_INSTANCE,      .name = "I2C1 (HW)" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_I2C,        .fixture.i2c        = VSF_BOARD_GPIO_I2C0_INSTANCE, .name = "GPIO_I2C0 (bit-bang)" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_I2C,        .fixture.i2c        = VSF_BOARD_GPIO_I2C1_INSTANCE, .name = "GPIO_I2C1 (bit-bang)" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_I2C_SLAVE,  .fixture.i2c_slave  = &vsf_board_i2c_slave_ctx,     .name = "I2C0_master + GPIO_I2C0_slave" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_USART,      .fixture.usart      = VSF_BOARD_USART_INSTANCE,     .name = "USART1" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_SPI,        .fixture.spi        = VSF_BOARD_SPI_INSTANCE,       .name = "SPI0" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_ADC,        .fixture.adc        = VSF_BOARD_ADC_INSTANCE,       .name = "ADC" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_PWM,        .fixture.pwm        = VSF_BOARD_PWM_INSTANCE,       .name = "PWM" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_DMA,        .fixture.dma        = VSF_BOARD_DMA_INSTANCE,       .name = "DMA" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_TIMER,      .fixture.timer      = VSF_BOARD_TIMER_INSTANCE,     .name = "Timer" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_RTC,        .fixture.rtc        = VSF_BOARD_RTC_INSTANCE,       .name = "RTC" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_FLASH,      .fixture.flash      = VSF_BOARD_FLASH_INSTANCE,     .name = "Flash" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_WDT,        .fixture.wdt        = VSF_BOARD_WDT_INSTANCE,       .name = "WDT" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_RNG,        .fixture.rng        = VSF_BOARD_RNG_INSTANCE,       .name = "RNG" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_GPIO,       .fixture.gpio       = VSF_BOARD_GPIO_INSTANCE,      .name = "GPIO" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_GPIO_PINMUX, .fixture.gpio_pinmux = &vsf_board_gpio_pinmux_ctx, .name = "GPIO + USART_pinmux" },
    { .peripheral_type = VSF_PERIPHERAL_TYPE_ARCH,       .fixture.raw        = NULL,                              .name = "Arch" },
};
uint8_t vsf_board_test_instance_count = dimof(vsf_board_test_instances);

/* ------------------------------------------------------------------------ */
/* Test-framework watchdog — safety net for hung test cases.                 */
/* ------------------------------------------------------------------------ */

static void __vsf_board_wdt_init(vsf_test_wdt_t *wdt, uint32_t timeout_ms)
{
    (void)wdt;
    vsf_wdt_init(vsf_board.wdt, &(vsf_wdt_cfg_t){
        .mode   = VSF_WDT_MODE_RESET_SOC,
        .max_ms = (timeout_ms > 0) ? timeout_ms : VSF_TEST_CFG_INTERNAL_TIMEOUT_MS,
        .min_ms = 0,
    });
    while (fsm_rt_cpl != vsf_wdt_enable(vsf_board.wdt));
}

static void __vsf_board_wdt_feed(vsf_test_wdt_t *wdt)
{
    (void)wdt;
    vsf_wdt_feed(vsf_board.wdt);
}

static vsf_test_wdt_t __vsf_board_test_wdt = {
    .init       = __vsf_board_wdt_init,
    .feed       = __vsf_board_wdt_feed,
    .timeout_ms = 0,
};

void vsf_test_hw_setup(vsf_test_t *test)
{
    test->wdt.entries      = &__vsf_board_test_wdt;
    test->wdt.count        = 1;
    test->reboot.entries   = (vsf_test_reboot_t *[]){ vsf_arch_reset };
    test->reboot.count     = 1;
    test->instances        = vsf_board_test_instances;
    test->instance_count   = dimof(vsf_board_test_instances);

    /* Log boot reset reason — helps triage WDT vs force vs power-on. */
    {
        uint32_t reason = watchdog_hw->reason;
        if (reason & WATCHDOG_REASON_FORCE_BITS) {
            VSF_TEST_TRACE_INFO("boot: last reset was WDT force trigger"
                                VSF_TRACE_CFG_LINEEND);
        } else if (reason & WATCHDOG_REASON_TIMER_BITS) {
            VSF_TEST_TRACE_INFO("boot: last reset was WDT timeout"
                                VSF_TRACE_CFG_LINEEND);
        }
        /* Reason register is auto-cleared on system reset.
         * Read-only via the hardware struct; no explicit clear needed. */
    }

    /* Check for WDT timeout-accuracy measurement from previous reset.
     * scratch[2] holds the magic marker set by vsf_test_wdt_timeout_accuracy. */
    {
        uint32_t marker = watchdog_hw->scratch[2];
        if (marker == 0x575F4143) {  /* "W_AC" */
            uint32_t start_us  = watchdog_hw->scratch[0];
            uint32_t expect_ms = watchdog_hw->scratch[1];
            uint32_t elapsed_us = timer_hw->timerawl - start_us;
            uint32_t elapsed_ms = elapsed_us / 1000;
            VSF_TEST_TRACE_INFO("WDT:ACCURACY:RESULT expected=%lu actual=%lu us"
                                VSF_TRACE_CFG_LINEEND,
                                (unsigned long)(expect_ms * 1000),
                                (unsigned long)elapsed_us);
            VSF_TEST_TRACE_INFO("WDT:ACCURACY:RESULT expected=%lu actual=%lu ms"
                                VSF_TRACE_CFG_LINEEND,
                                (unsigned long)expect_ms,
                                (unsigned long)elapsed_ms);
            watchdog_hw->scratch[2] = 0;  // clear marker
        }
    }
}

#endif /* VSF_USE_TEST == ENABLED */

// Override vsf_test_busy_wait_ms with vsf_systimer for accurate wall-clock
// timing. The default CPU-cycle loop calibration varies with compiler flags
// and CPU frequency, causing IRQ-driven scenario tests (e.g. timer alarms
// 500ms in the future) to give up before the IRQ fires.
void vsf_test_busy_wait_ms(uint32_t ms)
{
    vsf_systimer_tick_t start = vsf_systimer_get();
    vsf_systimer_tick_t delay_ticks = vsf_systimer_us_to_tick((uint_fast32_t)ms * 1000);
    while ((vsf_systimer_get() - start) < delay_ticks);
}

/* EOF */
