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

#define __VSF_SIMPLE_STREAM_CLASS_INHERIT__
#include "./vsf_board.h"

#include "hal/driver/vendor_driver.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/

#if VSF_USE_USB_HOST == ENABLED
static const vk_dwcotg_hcd_param_t __dwcotg_hcd_param = {
    .op                         = &VSF_USB_HC0_IP,
    .priority                   = APP_CFG_USBH_ARCH_PRIO,
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/

#if VSF_USE_USB_DEVICE == ENABLED
static const vk_dwcotg_dcd_param_t __dwcotg_dcd_param = {
    .op                         = &VSF_USB_DC1_IP,
    .speed                      = VSF_USBD_CFG_SPEED,
        .dma_en                 = false,
};
#endif

#if VSF_USE_BOARD == ENABLED
vsf_board_t vsf_board = {
#if VSF_HAL_USE_I2C == ENABLED
    .i2c                        = (vsf_i2c_t *)&vsf_hw_i2c2,
#endif
#if VSF_HAL_USE_SPI == ENABLED
    .spi                        = (vsf_spi_t *)&vsf_hw_spi1,
#endif
#if VSF_USE_USB_HOST == ENABLED
    .usbh_dev                   = {
        .drv                    = &vk_dwcotg_hcd_drv,
        .param                  = (void *)&__dwcotg_hcd_param,
    },
#endif
#if VSF_USE_USB_DEVICE == ENABLED
    .dwcotg_dcd                 = {
        .param                  = &__dwcotg_dcd_param,
    },
#endif
#if VSF_USE_AUDIO == ENABLED
#   if VSF_AUDIO_USE_DUMMY == ENABLED
    .audio_dev                  = &vsf_board.audio_dummy.use_as__vk_audio_dev_t,
    .audio_dummy                = {
        .drv                    = &vk_audio_dummy_drv,
    },
#   endif
#endif
};
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

#if VSF_USE_USB_DEVICE == ENABLED
vsf_usb_dc_from_dwcotg_ip(0, vsf_board.dwcotg_dcd, VSF_USB_DC0)
#endif

#if     (!defined(VSF_HAL_USE_DEBUG_STREAM) || VSF_HAL_USE_DEBUG_STREAM == DISABLED)\
    &&  VSF_HAL_USE_USART == ENABLED
#   define __VSF_BOARD_USE_UART_AS_DEBUG_STREAM
#   ifndef VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE
#       define VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE         32
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
            vsf_trace_error("debug stream overflow, please increate VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE\n");
            break;
        } else {
            vsf_stream_write(stream, NULL, vsf_usart_rxfifo_read(uart, buffer, buflen));
        }
    }
}

static void __VSF_DEBUG_STREAM_TX_INIT(void)
{
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart2;
    vsf_err_t err;

    vsf_stream_connect_tx(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t);
    err = vsf_usart_init(debug_usart, &(vsf_usart_cfg_t){
        .mode               = VSF_USART_8_BIT_LENGTH | VSF_USART_1_STOPBIT | VSF_USART_NO_PARITY
                            | VSF_USART_TX_ENABLE | VSF_USART_RX_ENABLE | VSF_USART_RX_FIFO_THRESHOLD_NOT_EMPTY,
        .baudrate           = 921600,
        .isr                = {
            .handler_fn     = __vsf_debug_stream_isrhandler,
            .target_ptr     = &VSF_DEBUG_STREAM_RX,
            .prio           = vsf_arch_prio_0,
        },
    });
    if (err != VSF_ERR_NONE) {
        VSF_ASSERT(false);
        return;
    }

    while (fsm_rt_cpl != vsf_usart_enable(debug_usart));
    vsf_usart_irq_enable(debug_usart, VSF_USART_IRQ_MASK_RX);
}

static void __VSF_DEBUG_STREAM_TX_WRITE_BLOCKED(uint8_t *buf, uint_fast32_t size)
{
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart2;
    uint_fast16_t cur_size;

    while (size > 0) {
        cur_size = vsf_usart_txfifo_write(debug_usart, buf, size);
        if (cur_size > 0) {
            size -= cur_size;
            buf += cur_size;
        }
    }
}

// Because debug stream for N32H7 is not used,
//  VSF_HAL_USE_DEBUG_STREAM is not defined in header files.
// But debug_stream_tx_blocked.inc will need VSF_HAL_USE_DEBUG_STREAM,
//  so define VSF_HAL_USE_DEBUG_STREAM here.
#undef VSF_HAL_USE_DEBUG_STREAM
#define VSF_HAL_USE_DEBUG_STREAM        ENABLED
#include "hal/driver/common/debug_stream/debug_stream_tx_blocked.inc"
#endif

// implement strong vsf_app_driver_init to overwrite weak one in hal
bool vsf_app_driver_init(void)
{
    vsf_hw_peripheral_enable(VSF_HW_EN_PWC);

    /* set power ldo output voltage to 1.3v */
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);

    vsf_hw_clk_enable(&VSF_HW_CLK_HEXT);

    VSF_STATIC_ASSERT(HEXT_VALUE == 12000000);
    // PLL: 12M * 72 / 1 = 864M
    vsf_hw_pll_config(&VSF_HW_CLK_PLL, &VSF_HW_CLK_HEXT, 72, 1, 0);
    // PLLP: PLL / 4 = 216M
    vsf_hw_clk_config(&VSF_HW_CLK_PLLP, NULL, 4, 0);
    vsf_hw_clk_enable(&VSF_HW_CLK_PLL);

    // update flash latency before update system clock
    vsf_hw_update_flash_latency(216 * 1000 * 1000);
    vsf_hw_clk_config(&VSF_HW_CLK_SYS, &VSF_HW_CLK_PLLP, 0, 0);
    // AHB: 216M
    vsf_hw_clk_config(&VSF_HW_CLK_AHB, NULL, 1, 0);
    // APB1: 216M / 2 = 108M
    vsf_hw_clk_config(&VSF_HW_CLK_APB1, NULL, 2, 0);
    // APB2: 216M
    vsf_hw_clk_config(&VSF_HW_CLK_APB2, NULL, 1, 0);
    vsf_hw_clk_config(&VSF_HW_CLK_SCLK, &VSF_HW_CLK_PLLP, 0, 0);
    // PLLU: PLL / 18 = 48M
    vsf_hw_clk_config(&VSF_HW_CLK_PLLU, NULL, 18, 0);
    vsf_hw_clk_enable(&VSF_HW_CLK_PLLU);
    vsf_hw_clk_config(&VSF_HW_CLK_OTGFS1, &VSF_HW_CLK_PLLU, 0, 0);

    system_core_clock_update();
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOA);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOB);
    return true;
}

void vsf_board_prepare_hw_for_linux(void)
{
    return;
}

void vsf_board_init(void)
{
    static const vsf_gpio_port_cfg_pins_t __cfgs[] = {
        // usart
        VSF_PORTA, 1 << 2, 0, VSF_HW_AF_USART2_TX_P0_2,
        VSF_PORTA, 1 << 3, 0, VSF_HW_AF_USART2_RX_P0_3,
        // i2c
        VSF_PORTA, 1 << 0, 0, VSF_HW_AF_I2C2_SCL_P0_0,
        VSF_PORTA, 1 << 1, 0, VSF_HW_AF_I2C2_SDA_P0_1,
        // spi
        VSF_PORTA, 1 << 4, 0, VSF_HW_AF_SPI1_CS_P0_4,
        VSF_PORTA, 1 << 5, 0, VSF_HW_AF_SPI1_SCK_P0_5,
        VSF_PORTA, 1 << 6, 0, VSF_HW_AF_SPI1_MISO_P0_6,
        VSF_PORTA, 1 << 7, 0, VSF_HW_AF_SPI1_MOSI_P0_7,
    };
    vsf_hw_gpio_ports_config_pins((vsf_gpio_port_cfg_pins_t *)__cfgs, dimof(__cfgs));

#ifdef __VSF_BOARD_USE_UART_AS_DEBUG_STREAM
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_RX);
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);
#endif
}
