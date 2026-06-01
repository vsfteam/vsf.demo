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

#define __VSF_HEAP_CLASS_INHERIT__
#include "./vsf_board.h"
#include "hardware/structs/watchdog.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ GPIO I2C INSTANCE =============================*/

describe_gpio_i2c(vsf_gpio_i2c0, (vsf_gpio_t *)&vsf_hw_gpio0, NULL, 21, 20);
// SCL=GP21, SDA=GP20 — same pins as hardware I2C0, but using bit-banging.
describe_gpio_i2c(vsf_gpio_i2c1, (vsf_gpio_t *)&vsf_hw_gpio0, NULL, 19, 18);
// SCL=GP19, SDA=GP18 — swapped vs vsf_test_board_pins.h label,
// but matches the physical wiring: GP18↔GP20=SDA, GP19↔GP21=SCL.
// fn_delay is NULL → driver uses default empty delay. At 125 MHz this may
// produce a very fast I2C clock; if EEPROM does not ack, add a real delay.

/*============================ GLOBAL VARIABLES ==============================*/

vsf_board_t vsf_board = {
    .usarts = {
        (vsf_usart_t *)&vsf_hw_usart0,  // UART0: GP0=TX, GP1=RX (debug stream)
        (vsf_usart_t *)&vsf_hw_usart1,  // UART1: GP8=TX, GP9=RX (test usart)
    },
    .i2cs = {
        (vsf_i2c_t *)&vsf_hw_i2c0,      // I2C0 = HW on GP20=SDA, GP21=SCL
        (vsf_i2c_t *)&vsf_hw_i2c1,      // I2C1 = HW on GP18=SCL, GP19=SDA
        (vsf_i2c_t *)&vsf_gpio_i2c0,    // I2C2 = GPIO bit-bang on GP20=SDA, GP21=SCL
        (vsf_i2c_t *)&vsf_gpio_i2c1,    // I2C3 = GPIO bit-bang on GP18=SDA, GP19=SCL
    },
    .spis = {
        (vsf_spi_t *)&vsf_hw_spi0,      // SPI0: GP2=SCK, GP3=MOSI, GP4=MISO, GP5=CS
        (vsf_spi_t *)&vsf_hw_spi1,      // SPI1: GP10=SCK, GP11=MOSI, GP12=MISO, GP13=CS
    },
    .gpio   = (vsf_gpio_t  *)&vsf_hw_gpio0,
    .flash  = (vsf_flash_t *)&vsf_hw_flash0,
    .adc    = (vsf_adc_t   *)&vsf_hw_adc0,
    .pwm0   = (vsf_pwm_t   *)&vsf_hw_pwm0,
    .timer0 = (vsf_timer_t *)&vsf_hw_timer0,
    .rtc    = (vsf_rtc_t   *)&vsf_hw_rtc0,
    .wdt    = (vsf_wdt_t   *)&vsf_hw_wdt0,
    .rng    = (vsf_rng_t   *)&vsf_hw_rng0,
    .dma    = (vsf_dma_t   *)&vsf_hw_dma0,
};

/*============================ IMPLEMENTATION ================================*/

VSF_CAL_WEAK(app_config_read)
int app_config_read(const char *cfgname, char *cfgvalue, int valuelen)
{
    return -1;
}

VSF_CAL_WEAK(app_config_write)
int app_config_write(const char *cfgname, char *cfgvalue)
{
    return -1;
}

#if     (!defined(VSF_HAL_USE_DEBUG_STREAM) || VSF_HAL_USE_DEBUG_STREAM == DISABLED)\
    &&  VSF_HAL_USE_USART == ENABLED
#   define __VSF_BOARD_USE_UART_AS_DEBUG_STREAM
#   ifndef VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE
#       define VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE         256
#   endif
static uint8_t __vsf_debug_stream_rx_buff[VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE];
vsf_mem_stream_t VSF_DEBUG_STREAM_RX = {
    .op         = &vsf_mem_stream_op,
    .buffer     = __vsf_debug_stream_rx_buff,
    .size       = sizeof(__vsf_debug_stream_rx_buff),
};

static void __vsf_debug_stream_isrhandler(void *target, vsf_usart_t *uart,
        vsf_usart_irq_mask_t irq_mask)
{
    vsf_stream_t *stream = target;
    uint8_t *buffer;
    uint_fast32_t buflen;

    while (vsf_usart_rxfifo_get_data_count(uart) > 0) {
        buflen = vsf_stream_get_wbuf(stream, &buffer);
        if (!buflen) {
            // buffer full, no consumer — drain to clear interrupt
            uint8_t dummy;
            while (vsf_usart_rxfifo_get_data_count(uart) > 0)
                vsf_usart_rxfifo_read(uart, &dummy, 1);
            break;
        }
        vsf_stream_write(stream, NULL, vsf_usart_rxfifo_read(uart, buffer, buflen));
    }
}

static void __VSF_DEBUG_STREAM_TX_INIT(void)
{
    // UART0 pinmux for GP0/GP1 is configured in vsf_board_init() before this
    // function runs.
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart0;
    vsf_err_t err;

    vsf_stream_connect_tx(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t);
    err = vsf_usart_init(debug_usart, &(vsf_usart_cfg_t){
        .mode               = VSF_USART_8_BIT_LENGTH | VSF_USART_1_STOPBIT | VSF_USART_NO_PARITY
                            | VSF_USART_TX_ENABLE | VSF_USART_RX_ENABLE
                            | VSF_USART_RX_FIFO_THRESHOLD_NOT_EMPTY,
        .baudrate           = 115200,
        .isr                = {
            .handler_fn     = __vsf_debug_stream_isrhandler,
            .target_ptr     = &VSF_DEBUG_STREAM_RX,
            // vsf_arch_prio_0 maps to the lowest priority (same as PendSV
            // where the VSF kernel runs), so the RX IRQ would never preempt
            // scheduling. Use vsf_arch_prio_highest so debug RX bytes are
            // serviced while threads run.
            .prio           = vsf_arch_prio_highest,
        },
    });
    if (err != VSF_ERR_NONE) {
        VSF_ASSERT(false);
        return;
    }

    while (fsm_rt_cpl != vsf_usart_enable(debug_usart));
    // Enable both FIFO-threshold and RX-timeout IRQs. The threshold alone
    // would leave the last <threshold bytes of a burst stuck in the FIFO
    // because no IRQ fires; RX_TIMEOUT drains them after a brief silence.
    vsf_usart_irq_enable(debug_usart, VSF_USART_IRQ_MASK_RX | VSF_USART_IRQ_MASK_RX_TIMEOUT);
}

static void __VSF_DEBUG_STREAM_TX_WRITE_BLOCKED(uint8_t *buf, uint_fast32_t size)
{
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart0;
    uint_fast16_t cur_size;

    while (size > 0) {
        cur_size = vsf_usart_txfifo_write(debug_usart, buf, size);
        if (cur_size > 0) {
            size -= cur_size;
            buf += cur_size;
        }
    }
}

#undef VSF_HAL_USE_DEBUG_STREAM
#define VSF_HAL_USE_DEBUG_STREAM        ENABLED
#include "hal/driver/common/debug_stream/debug_stream_tx_blocked.inc"

int vsf_stdin_getchar(void)
{
    uint8_t byte;
    while (vsf_stream_read(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t, &byte, 1) == 0);
    return byte;
}
#endif

VSF_CAL_WEAK(vsf_board_get_uart_funcsel)
uint8_t vsf_board_get_uart_funcsel(void)
{
    return GPIO_FUNC_UART;
}

void vsf_board_init(void)
{
    static const vsf_gpio_port_cfg_pin_t __pinmux_cfgs[] = {
        // UART0: GP0=TX, GP1=RX (debug stream) — kept here, boot-time need
        {VSF_PA0,   VSF_GPIO_AF,  GPIO_FUNC_UART},
        {VSF_PA1,   VSF_GPIO_AF,  GPIO_FUNC_UART},
        // SPI0: GP2=SCK, GP3=MOSI, GP4=MISO, GP5=CS
        // SPI test scenarios not yet enabled; keep in boot table for now.
        {VSF_PA2,   VSF_GPIO_AF,  GPIO_FUNC_SPI},
        {VSF_PA3,   VSF_GPIO_AF,  GPIO_FUNC_SPI},
        {VSF_PA4,   VSF_GPIO_AF,  GPIO_FUNC_SPI},
        {VSF_PA5,   VSF_GPIO_AF,  GPIO_FUNC_SPI},
    };
    vsf_hw_gpio_ports_config_pin((vsf_gpio_port_cfg_pin_t *)__pinmux_cfgs, dimof(__pinmux_cfgs));

#ifdef __VSF_BOARD_USE_UART_AS_DEBUG_STREAM
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);
#endif

#if VSF_HAL_USE_DMA == ENABLED
    vsf_dma_init(vsf_board.dma, &(vsf_dma_cfg_t){0});
#endif
}

/* ------------------------------------------------------------------------ */
/* Fault recovery — WDT force-reset on HardFault                              */
/* ------------------------------------------------------------------------ */

#include "hardware/structs/timer.h"

#if VSF_USE_TRACE == ENABLED
#   define __FAULT_TRACE(fmt) \
       vsf_trace_error(fmt VSF_TRACE_CFG_LINEEND)
#else
#   define __FAULT_TRACE(fmt) ((void)0)
#endif

void HardFault_Handler(void)
{
    __FAULT_TRACE("HardFault: WDT force reset");
    watchdog_hw->ctrl = WATCHDOG_CTRL_TRIGGER_BITS;
    while (1);   // insurance — never reached if TRIGGER succeeds
}

// newlib exit()/abort() require _exit on bare-metal targets.
// VSF Linux subsystem provides its own _exit; avoid duplicate definition.
// _getpid and _kill are provided by VSF's __retarget_io_gcc_llvm.c (weak).
#if !defined(VSF_USE_LINUX) || (VSF_USE_LINUX == DISABLED)
VSF_CAL_NO_RETURN void _exit(int status)
{
    (void)status;
    while (1);
}
#endif


