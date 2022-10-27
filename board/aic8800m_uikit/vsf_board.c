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
#define __VSF_SIMPLE_STREAM_CLASS_INHERIT__
#include "./vsf_board.h"

#include "rtos_al.h"
#include "reg_sysctrl.h"

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

#if VSF_USE_UI == ENABLED
static const uint8_t __vsf_board_lcd_initseq[] = {
    VSF_DISP_MIPI_LCD_ST7789V_BASE,
    VSF_DISP_MIPI_LCD_INIT_MODE_AND_FORMAT(
            MIPI_DCS_PAGE_ADDRESS_TOP_TO_BOTTOM
        |   MIPI_DCS_COLUME_ADDRESS_LEFT_TO_RIGHT
        |   MIPI_DCS_PAGE_COLUMN_NORMAL_ORDER,
            MIPI_DCS_PIXEL_FORMAT_DBI_16_BITS
    ),
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/

vsf_board_t vsf_board = {
#if VSF_HAL_USE_USART == ENABLED
    .usart                      = (vsf_usart_t *)&vsf_hw_usart1,
#endif
#if VSF_HAL_USE_SPI == ENABLED
    .spi                        = (vsf_spi_t *)&vsf_hw_spi0,
#endif
#if VSF_HAL_USE_I2C == ENABLED
    .i2c                        = (vsf_i2c_t *)&vsf_hw_i2c0,
#endif
#if VSF_HAL_USE_MMC == ENABLED
    .mmc                        = (vsf_mmc_t *)&vsf_hw_mmc0,
    .mmc_bus_width              = 4,
    .mmc_voltage                = SD_OCR_VDD_32_33 | SD_OCR_VDD_33_34,
#endif
#if VSF_HAL_USE_I2S == ENABLED
    .i2s                        = (vsf_i2s_t *)&vsf_hw_i2s0,
#endif

#if VSF_USE_UI == ENABLED
    .display_dev                = &vsf_board.disp_spi_mipi.use_as__vk_disp_t,
    .disp_spi_mipi              = {
        .param                  = {
            .height             = 320,
            .width              = 240,
            .drv                = &vk_disp_drv_mipi_lcd,
            .color              = VSF_DISP_COLOR_RGB565,
        },
        .spi                    = (vsf_spi_t *)&vsf_hw_spi0,
        .reset                  = {
            .gpio               = (vsf_gpio_t *)&vsf_hw_gpio1,
            .pin_mask           = 1 << 7,
        },
        .dcx                    = {
            .gpio               = (vsf_gpio_t *)&vsf_hw_gpio0,
            .pin_mask           = 1 << 4,
        },
        .clock_hz               = 60ul * 1000ul * 1000ul,
        .init_seq               = __vsf_board_lcd_initseq,
        .init_seq_len           = sizeof(__vsf_board_lcd_initseq),
    },
#endif
#if VSF_USE_AUDIO == ENABLED
    .audio_dev                  = &vsf_board.aic1000a.use_as__vk_audio_dev_t,
    .aic1000a                   = {
        .drv                    = &vk_aic1000a_drv,
        .pwrkey_port            = (vsf_gpio_t *)&vsf_hw_gpio1,
        .psi_port               = (vsf_gpio_t *)&vsf_hw_gpio1,
        .pwrkey_pin             = 5,
        .psi_clk_pin            = 4,
        .psi_dat_pin            = 13,
    },
#endif
#if VSF_USE_USB_HOST == ENABLED
    .usbh_dev                   = {
        .drv                    = &vk_dwcotg_hcd_drv,
        .param                  = (void *)&__dwcotg_hcd_param,
    },
#endif
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

#if VSF_USE_USB_HOST
// redefine usbh memory allocation, memory MUST be in 0x001A0000 - 0x001C7FFF
struct __usbh_heap_t {
    implement(vsf_heap_t)
    uint8_t memory[32 * 1024];
    // one more as terminator
    vsf_dlist_t freelist[2];
} static __usbh_heap;

static vsf_dlist_t * __usbh_heap_get_freelist(uint_fast32_t size)
{
    return &__usbh_heap.freelist[0];
}

static void __usbh_heap_init(void)
{
    memset(&__usbh_heap.use_as__vsf_heap_t, 0, sizeof(__usbh_heap.use_as__vsf_heap_t));
    for (uint_fast8_t i = 0; i < dimof(__usbh_heap.freelist); i++) {
        vsf_dlist_init(&__usbh_heap.freelist[i]);
    }
    __usbh_heap.get_freelist = __usbh_heap_get_freelist;
    __vsf_heap_add_buffer(&__usbh_heap.use_as__vsf_heap_t, __usbh_heap.memory, sizeof(__usbh_heap.memory));
}

void * __vsf_usbh_malloc_aligned(uint_fast32_t size, uint_fast32_t alignment)
{
    return __vsf_heap_malloc_aligned(&__usbh_heap.use_as__vsf_heap_t, size, alignment);
}

void * __vsf_usbh_malloc(uint_fast32_t size)
{
    return __vsf_heap_malloc_aligned(&__usbh_heap.use_as__vsf_heap_t, size, 0);
}

void __vsf_usbh_free(void *buffer)
{
    __vsf_heap_free(&__usbh_heap.use_as__vsf_heap_t, buffer);
}
#endif

#if !defined(VSF_HAL_USE_DEBUG_STREAM) || VSF_HAL_USE_DEBUG_STREAM == DISABLED
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
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart1;
    vsf_err_t err;

    vsf_stream_connect_tx(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t);
    err = vsf_usart_init(debug_usart, &(vsf_usart_cfg_t){
        .mode               = USART_8_BIT_LENGTH | USART_1_STOPBIT | USART_NO_PARITY
                            | USART_TX_ENABLE | USART_RX_ENABLE,
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
    vsf_usart_irq_enable(debug_usart, USART_IRQ_MASK_RX);
}

static void __VSF_DEBUG_STREAM_TX_WRITE_BLOCKED(uint8_t *buf, uint_fast32_t size)
{
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart1;

    for (uint_fast32_t i = 0; i < size; i++) {
        vsf_usart_txfifo_write(debug_usart, buf++, 1);
    }
}

// Because debug stream for AIC8800M is not used,
//  VSF_HAL_USE_DEBUG_STREAM is not defined in header files.
// But debug_stream_tx_blocked.inc will need VSF_HAL_USE_DEBUG_STREAM,
//  so define VSF_HAL_USE_DEBUG_STREAM here.
#undef VSF_HAL_USE_DEBUG_STREAM
#define VSF_HAL_USE_DEBUG_STREAM        ENABLED
#include "hal/driver/common/debug_stream/debug_stream_tx_blocked.inc"
#endif

void vsf_board_init(void)
{
    static const vsf_io_cfg_t cfgs[] = {
#if VSF_USE_UI == ENABLED
        {VSF_PA0,   0x09,   0},
        {VSF_PA3,   0x09,   0},
        {VSF_PA4,   0x00,   0},
#endif
        // PB7 as lcd/touch reset
        {VSF_PB7,   0x00,   0},

#if VSF_USE_AUDIO == ENABLED
        // PA5 as 26M clock output
        {VSF_PA5,  0x06,   0},
#endif

        // PA10 - PA15 as SDIOM
        {VSF_PA10,  0x06,   0},
        {VSF_PA11,  0x06,   0},
        {VSF_PA12,  0x06,   0},
        {VSF_PA13,  0x06,   0},
        {VSF_PA14,  0x06,   0},
        {VSF_PA15,  0x06,   0},

//        {VSF_PA1,   0x06,   0},
        {VSF_PA2,   0x06,   0},
//        {VSF_PA7,   0x08,   0},
//        {VSF_PA13,  0x08,   0},
        {VSF_PA7,   0x08,   0},
        {VSF_PA8,   0x08,   0},
        {VSF_PA9,   0x08,   0},

        {VSF_PB0,   0x01,   0},
        {VSF_PB1,   0x01,   0},
        {VSF_PB5,   0x00,   0},
        {VSF_PB4,   0x00,   0},
        {VSF_PB13,  0x00,   0},

        // PB2 PB3 as UART1
        {VSF_PB2,   0x01,   0},
        {VSF_PB3,   0x01,   0},
    };
    vsf_io_config((vsf_io_cfg_t *)cfgs, dimof(cfgs));

    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);
#if VSF_USE_USB_HOST == ENABLED
    __usbh_heap_init();
#endif

#if VSF_USE_AUDIO == ENABLED
    vsf_board.aic1000a.i2s = vsf_board.i2s;
    // generate 26M debug_clk for audio
    AIC_CPUSYSCTRL->TPSEL = (AIC_CPUSYSCTRL->TPSEL & ~(0xFFUL << 8)) | (0x18UL << 8);
#endif

    // currently known dependency on rtos_al: lwip from vendor and audio in SDK
    if (rtos_init()) {
        VSF_HAL_ASSERT(false);
    }
}
