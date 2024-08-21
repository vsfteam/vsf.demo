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

#include "hal/driver/GigaDevice/GD32H7XX/common/vendor/Include/gd32h7xx.h"

/*============================ MACROS ========================================*/

// TODO: remove code below after bug in float support in simple_sprintf beging fixed
#if VSF_USE_SIMPLE_SPRINTF == ENABLED && VSF_SIMPLE_SPRINTF_SUPPORT_FLOAT == ENABLED
#   error VSF_SIMPLE_SPRINTF_SUPPORT_FLOAT is not stable now for CortexM7 targets
#endif

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
#   define TLI_PIXFORMAT_ARGB8888       0x00
#   define TLI_PIXFORMAT_RGB888         0x01
#   define TLI_PIXFORMAT_RGB565         0x02
#   define TLI_PIXFORMAT_ARGB1555       0x03
#   define TLI_PIXFORMAT_ARGB4444       0x04
#   define TLI_PIXFORMAT_L8             0x05
#   define TLI_PIXFORMAT_AL44           0x06
#   define TLI_PIXFORMAT_AL88           0x07
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
typedef struct vsf_hw_rgblcd_param_t {
    vk_disp_coord_t width, height;      // in pixels
    vk_disp_color_type_t pixel_format;
    uint16_t hsw, vsw, hbp, vbp, hfp, vfp;
} vsf_hw_rgblcd_param_t;
#endif

/*============================ PROTOTYPES ====================================*/

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
static vsf_err_t __gd32h7xx_fb_init(vsf_hw_rgblcd_param_t *fb, int color_format, void *initial_pixel_buffer);
static vsf_err_t __gd32h7xx_fb_fini(vsf_hw_rgblcd_param_t *fb);
static vsf_err_t __gd32h7xx_fb_present(vsf_hw_rgblcd_param_t *fb, void *pixel_buffer);
#endif

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
    .op                         = &VSF_USB_DC0_IP,
    .speed                      = VSF_USBD_CFG_SPEED,
        .ulpi_en                = true,
        .dma_en                 = false,
};
#endif

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
static const vsf_hw_rgblcd_param_t __vsf_board_lcd_param = {
    .width                      = VSF_BOARD_RGBLCD_WIDTH,
    .height                     = VSF_BOARD_RGBLCD_HEIGHT,
    .hsw                        = VSF_BOARD_RGBLCD_HSW,
    .hbp                        = VSF_BOARD_RGBLCD_HBP,
    .hfp                        = VSF_BOARD_RGBLCD_HFP,
    .vsw                        = VSF_BOARD_RGBLCD_VSW,
    .vbp                        = VSF_BOARD_RGBLCD_VBP,
    .vfp                        = VSF_BOARD_RGBLCD_VFP,
    .pixel_format               = VSF_DISP_COLOR_ARGB8888,
};

static const vk_disp_fb_drv_t __vk_disp_fb_drv_gd32h7xx = {
    .init       = NULL,
    .fb.init    = (vsf_err_t (*)(void *, int, void *))__gd32h7xx_fb_init,
    .fb.fini    = (vsf_err_t (*)(void *))__gd32h7xx_fb_fini,
    .fb.present = (vsf_err_t (*)(void *, void *))__gd32h7xx_fb_present,
};
#endif

#if VSF_USE_BOARD == ENABLED
vsf_board_t vsf_board = {
#if VSF_HAL_USE_I2C == ENABLED
    .i2c                        = (vsf_i2c_t *)&vsf_hw_i2c0,
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
#if VSF_HAL_USE_SDIO == ENABLED
    .sdio                       = (vsf_sdio_t *)&vsf_hw_sdio0,
    .sdio_bus_width             = 4,
    .sdio_voltage               = SD_OCR_VDD_32_33 | SD_OCR_VDD_33_34,
#endif
#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
    .display_fb                 = {
        .param                  = {
            .height             = VSF_BOARD_RGBLCD_HEIGHT,
            .width              = VSF_BOARD_RGBLCD_WIDTH,
            .drv                = &vk_disp_drv_fb,
            .color              = VSF_BOARD_RGBLCD_COLOR,
        },
        .fb                     = {
            .buffer             = (void *)0xC0000000,
            .drv                = &__vk_disp_fb_drv_gd32h7xx,
            .param              = (void *)&__vsf_board_lcd_param,
            .size               = vsf_disp_get_pixel_format_bytesize(VSF_BOARD_RGBLCD_COLOR)
                                * VSF_BOARD_RGBLCD_WIDTH * VSF_BOARD_RGBLCD_HEIGHT,
            .num                = 2,        // front/bancend frame buffer
            .pixel_byte_size    = vsf_disp_get_pixel_format_bytesize(VSF_BOARD_RGBLCD_COLOR),
        },
    },
    .display_dev                = &vsf_board.display_fb.use_as__vk_disp_t,
    .bl_port                    = (vsf_gpio_t *)&vsf_hw_gpio9,
    .rst_port                   = (vsf_gpio_t *)&vsf_hw_gpio9,
    .bl_pin                     = 10,
    .rst_pin                    = 11,
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
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart1;
    vsf_err_t err;

    vsf_stream_connect_tx(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t);
    err = vsf_usart_init(debug_usart, &(vsf_usart_cfg_t){
        .mode               = VSF_USART_8_BIT_LENGTH | VSF_USART_1_STOPBIT | VSF_USART_NO_PARITY
                            | VSF_USART_TX_ENABLE | VSF_USART_RX_ENABLE,
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
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart1;
    uint_fast16_t cur_size;

    while (size > 0) {
        cur_size = vsf_usart_txfifo_write(debug_usart, buf, size);
        if (cur_size > 0) {
            size -= cur_size;
            buf += cur_size;
        }
    }
}

// Because debug stream for RP2040 is not used,
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
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOA);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOB);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOC);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOD);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOE);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOF);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOG);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOH);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOJ);
    vsf_hw_peripheral_enable(VSF_HW_EN_GPIOK);
    vsf_hw_peripheral_enable(VSF_HW_EN_EXMC);
    return true;
}

void vsf_board_init_linux(void)
{
}

void delay_us(uint32_t us)
{
    vsf_systimer_tick_t cur_us = vsf_systimer_get_us(), due_us = cur_us + us;
    while (vsf_systimer_get_us() < due_us);
}

void delay_ms(uint32_t ms)
{
    delay_us(1000 * ms);
}

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003)

#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)

#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)

#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)

#define SDRAM_TIMEOUT                            ((uint32_t)0x0000FFFF)

static void __sdram_init(uint32_t sdram_device)
{
    exmc_sdram_parameter_struct sdram_init_struct;
    exmc_sdram_timing_parameter_struct sdram_timing_init_struct = {
        .load_mode_register_delay = 2,
        .exit_selfrefresh_delay = 11,
        .row_address_select_delay = 10,
        .auto_refresh_delay = 10,
        .write_recovery_delay = 2,
        .row_precharge_delay = 3,
        .row_to_column_delay = 3,
    };
    exmc_sdram_command_parameter_struct sdram_command_init_struct;

    uint32_t command_content = 0;
    uint32_t timeout = SDRAM_TIMEOUT;
    uint32_t bank_select = (EXMC_SDRAM_DEVICE0 == sdram_device) ? EXMC_SDRAM_DEVICE0_SELECT : EXMC_SDRAM_DEVICE1_SELECT;

    exmc_sdram_struct_para_init(&sdram_init_struct);

    sdram_init_struct.sdram_device = sdram_device;
    sdram_init_struct.column_address_width = EXMC_SDRAM_COW_ADDRESS_9;
    sdram_init_struct.row_address_width = EXMC_SDRAM_ROW_ADDRESS_13;
    sdram_init_struct.data_width = EXMC_SDRAM_DATABUS_WIDTH_16B;
    sdram_init_struct.internal_bank_number = EXMC_SDRAM_4_INTER_BANK;
    sdram_init_struct.cas_latency = EXMC_CAS_LATENCY_3_SDCLK;
    sdram_init_struct.write_protection = DISABLE;
    sdram_init_struct.sdclock_config = EXMC_SDCLK_PERIODS_2_CK_EXMC;
    sdram_init_struct.burst_read_switch = ENABLE;
    sdram_init_struct.pipeline_read_delay = EXMC_PIPELINE_DELAY_2_CK_EXMC;
    sdram_init_struct.timing = &sdram_timing_init_struct;
    exmc_sdram_init(&sdram_init_struct);
    while((exmc_flag_get(sdram_device, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) {
        timeout--;
    }

    sdram_command_init_struct.command = EXMC_SDRAM_CLOCK_ENABLE;
    sdram_command_init_struct.bank_select = bank_select;
    sdram_command_init_struct.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_1_SDCLK;
    sdram_command_init_struct.mode_register_content = 0;
    exmc_sdram_command_config(&sdram_command_init_struct);

    delay_ms(10);

    sdram_command_init_struct.command = EXMC_SDRAM_PRECHARGE_ALL;
    sdram_command_init_struct.bank_select = bank_select;
    sdram_command_init_struct.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_1_SDCLK;
    sdram_command_init_struct.mode_register_content = 0;

    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(sdram_device, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) {
        timeout--;
    }

    exmc_sdram_command_config(&sdram_command_init_struct);

    sdram_command_init_struct.command = EXMC_SDRAM_AUTO_REFRESH;
    sdram_command_init_struct.bank_select = bank_select;
    sdram_command_init_struct.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_9_SDCLK;
    sdram_command_init_struct.mode_register_content = 0;

    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(sdram_device, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) {
        timeout--;
    }

    exmc_sdram_command_config(&sdram_command_init_struct);

    command_content = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1    |
                      SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL       |
                      SDRAM_MODEREG_CAS_LATENCY_3               |
                      SDRAM_MODEREG_OPERATING_MODE_STANDARD     |
                      SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    sdram_command_init_struct.command = EXMC_SDRAM_LOAD_MODE_REGISTER;
    sdram_command_init_struct.bank_select = bank_select;
    sdram_command_init_struct.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_1_SDCLK;
    sdram_command_init_struct.mode_register_content = command_content;

    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(sdram_device, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) {
        timeout--;
    }

    exmc_sdram_command_config(&sdram_command_init_struct);

    exmc_sdram_refresh_count_set(7.81 * 150 - 20);

    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(sdram_device, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) {
        timeout--;
    }
}

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED

static vsf_err_t __gd32h7xx_fb_init(vsf_hw_rgblcd_param_t *fb, int color_format, void *initial_pixel_buffer)
{
    uint8_t pixel_byte_size = vsf_disp_get_pixel_format_bytesize(fb->pixel_format);
    tli_parameter_struct tli_init_struct = {
        .signalpolarity_hs = TLI_HSYN_ACTLIVE_LOW,
        .signalpolarity_vs = TLI_VSYN_ACTLIVE_LOW,
        .signalpolarity_de = TLI_DE_ACTLIVE_LOW,
        .signalpolarity_pixelck = TLI_PIXEL_CLOCK_TLI,

        .synpsz_hpsz = fb->hsw - 1,
        .synpsz_vpsz = fb->vsw - 1,
        .backpsz_hbpsz = fb->hsw + fb->hbp - 1,
        .backpsz_vbpsz = fb->vsw + fb->vbp - 1,
        .activesz_hasz = fb->hsw + fb->hbp + fb->width - 1,
        .activesz_vasz = fb->vsw + fb->vbp + fb->height - 1,
        .totalsz_htsz = fb->hsw + fb->hbp + fb->width + fb->hfp - 1,
        .totalsz_vtsz = fb->vsw + fb->vbp + fb->height + fb->vfp - 1,

        .backcolor_red = 0xFF,
        .backcolor_green = 0xFF,
        .backcolor_blue = 0xFF,
    };
    tli_layer_parameter_struct tli_layer_init_struct = {
        .layer_window_leftpos = fb->hsw + fb->hbp,
        .layer_window_rightpos = (fb->width + fb->hsw + fb->hbp - 1),
        .layer_window_toppos = fb->vsw + fb->vbp,
        .layer_window_bottompos = fb->height + fb->vsw + fb->vbp - 1,
        .layer_sa = 255,
        .layer_acf1 = LAYER_ACF1_PASA,
        .layer_acf2 = LAYER_ACF2_PASA,
        .layer_default_alpha = 0,
        .layer_default_blue = 0,
        .layer_default_green = 0,
        .layer_default_red = 0,
        .layer_frame_bufaddr = (uint32_t)initial_pixel_buffer,
        .layer_frame_buf_stride_offset = (fb->width * pixel_byte_size),
        .layer_frame_line_length = ((fb->width * pixel_byte_size) + 3),
        .layer_frame_total_line_number = fb->height,
    };

    vsf_hw_peripheral_enable(VSF_HW_EN_TLI);
    vsf_hw_clk_disable(&VSF_HW_CLK_PLL2_VCO);
    vsf_hw_pll_vco_config(&VSF_HW_CLK_PLL2_VCO, 25, 396000000);
    vsf_hw_clk_config(&VSF_HW_CLK_PLL2R, NULL, 3, 0);
    vsf_hw_clk_config(&VSF_HW_CLK_TLI, NULL, 4, 0);
    vsf_hw_clk_enable(&VSF_HW_CLK_PLL2R);
    vsf_hw_clk_enable(&VSF_HW_CLK_PLL2_VCO);

    tli_init(&tli_init_struct);

    switch (fb->pixel_format) {
    case VSF_DISP_COLOR_ARGB8888:
        tli_layer_init_struct.layer_ppf = TLI_PIXFORMAT_ARGB8888;
        break;
    case VSF_DISP_COLOR_RGB888_24:
        tli_layer_init_struct.layer_ppf = TLI_PIXFORMAT_RGB888;
        break;
    case VSF_DISP_COLOR_RGB565:
        tli_layer_init_struct.layer_ppf = TLI_PIXFORMAT_RGB565;
        break;
    default:
        VSF_ASSERT(false);
        break;
    }
    tli_layer_init(LAYER0, &tli_layer_init_struct);

    tli_layer_enable(LAYER0);
    tli_reload_config(TLI_REQUEST_RELOAD_EN);
    tli_enable();

    vsf_gpio_set(vsf_board.bl_port, 1 << vsf_board.bl_pin);
    return VSF_ERR_NONE;
}

static vsf_err_t __gd32h7xx_fb_fini(vsf_hw_rgblcd_param_t *fb)
{
    vsf_hw_peripheral_rst_set(VSF_HW_RST_TLI);
    vsf_hw_peripheral_rst_clear(VSF_HW_RST_TLI);
    return VSF_ERR_NONE;
}

static vsf_err_t __gd32h7xx_fb_present(vsf_hw_rgblcd_param_t *fb, void *pixel_buffer)
{
    return VSF_ERR_NONE;
}
#endif

void vsf_board_init(void)
{
    static const vsf_io_cfg_t __cfgs[] = {
        // USART
        {VSF_PA2,   7,  VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP},
        {VSF_PA3,   7,  VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP},

        // SDRAM
        {VSF_PC0,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PC4,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PC5,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PD0,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PD1,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PD8,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PD9,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PD10,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PD14,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PD15,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE0,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE1,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE7,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE8,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE9,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE10,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE11,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE12,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE13,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE14,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PE15,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF0,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF1,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF2,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF3,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF4,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF5,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF11,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF12,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF13,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF14,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PF15,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PG0,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PG1,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PG2,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PG4,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PG5,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PG8,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},
        {VSF_PG15,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_85MHZ},

        // SDIO
#if VSF_HAL_USE_SDIO == ENABLED
        {VSF_PB13,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PC9,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PC10,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PC11,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PC12,  12, VSF_IO_AF_PUSH_PULL | VSF_IO_FLOATING | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PD2,   12, VSF_IO_AF_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_100MHZ_220MHZ},
#endif

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
        // back light
        {VSF_PJ10,  0,  VSF_IO_OUTPUT_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_60MHZ},
        // reset
        {VSF_PJ11,  0,  VSF_IO_OUTPUT_PUSH_PULL | VSF_IO_PULL_UP | VSF_IO_SPEED_60MHZ},
        // DE
        {VSF_PF10,  14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        // VSYNC
        {VSF_PA7,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        // HSYNC
        {VSF_PC6,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        // PCLK
        {VSF_PG7,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        // RGB
        {VSF_PA1,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PA2,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PA5,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PA6,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PA8,   13, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PA9,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PA10,  14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PA15,  14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PB0,   9,  VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PB1,   9,  VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PB5,   3,  VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PB9,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PB10,  14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PB11,  14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PC7,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PD6,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PE4,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PE5,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PE6,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PG6,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PG10,  9,  VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PG12,  9,  VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PG13,  14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
        {VSF_PK2,   14, VSF_IO_AF_PUSH_PULL | VSF_IO_NO_PULL_UP_DOWN | VSF_IO_SPEED_100MHZ_220MHZ},
#endif
    };
    vsf_io_config((vsf_io_cfg_t *)__cfgs, dimof(__cfgs));

#ifdef __VSF_BOARD_USE_UART_AS_DEBUG_STREAM
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_RX);
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);
#endif

    __sdram_init(EXMC_SDRAM_DEVICE0);
}
