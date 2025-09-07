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

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
#   ifndef VSF_BOARD_RGBLCD_LAYER0_WIDTH
#       define VSF_BOARD_RGBLCD_LAYER0_WIDTH        VSF_BOARD_RGBLCD_WIDTH
#   endif
#   ifndef VSF_BOARD_RGBLCD_LAYER0_HEIGHT
#       define VSF_BOARD_RGBLCD_LAYER0_HEIGHT       VSF_BOARD_RGBLCD_HEIGHT
#   endif
#   ifndef VSF_BOARD_RGBLCD_LAYER0_COLOR
#       define VSF_BOARD_RGBLCD_LAYER0_COLOR        VSF_BOARD_RGBLCD_COLOR
#   endif
#   ifndef VSF_BOARD_RGBLCD_LAYER0_FBNUM
#       define VSF_BOARD_RGBLCD_LAYER0_FBNUM        2
#   endif
#endif

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

#ifdef VSF_BOARD_RGBLCD_LAYER0_SRAM_BUFFER_T
static VSF_BOARD_RGBLCD_LAYER0_SRAM_BUFFER_T __vsf_hw_rgblcd_layer0_fb
    [VSF_BOARD_RGBLCD_LAYER0_FBNUM]                                             \
    [VSF_BOARD_RGBLCD_LAYER0_WIDTH * VSF_BOARD_RGBLCD_LAYER0_HEIGHT] VSF_CAL_ALIGN(8);
#endif

#   if      defined(VSF_BOARD_RGBLCD_LAYER1_WIDTH)                              \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_HEIGHT)                             \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_COLOR)                              \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_SRAM_BUFFER_T)
#   ifndef VSF_BOARD_RGBLCD_LAYER1_FBNUM
#       define VSF_BOARD_RGBLCD_LAYER1_FBNUM        2
#   endif
static VSF_BOARD_RGBLCD_LAYER1_SRAM_BUFFER_T __vsf_hw_rgblcd_layer1_fb
    [VSF_BOARD_RGBLCD_LAYER1_FBNUM]
    [VSF_BOARD_RGBLCD_LAYER1_WIDTH * VSF_BOARD_RGBLCD_LAYER1_HEIGHT] VSF_CAL_ALIGN(8);
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
    .hw_fb                      = {
        .color_type             = VSF_HW_FB_RGB,
        .width                  = VSF_BOARD_RGBLCD_WIDTH,
        .height                 = VSF_BOARD_RGBLCD_HEIGHT,
        .pixel_format           = VSF_DISP_COLOR_ARGB8888,
        .timing.rgb             = {
            .fps                = VSF_BOARD_RGBLCD_FPS,
            .hsw                = VSF_BOARD_RGBLCD_HSW,
            .hbp                = VSF_BOARD_RGBLCD_HBP,
            .hfp                = VSF_BOARD_RGBLCD_HFP,
            .vsw                = VSF_BOARD_RGBLCD_VSW,
            .vbp                = VSF_BOARD_RGBLCD_VBP,
            .vfp                = VSF_BOARD_RGBLCD_VFP,
            .use_de             = true,
            .use_pixel_clk      = true,
            .polarity           = VSF_HW_FB_HSYNC_POL_LOW
                                | VSF_HW_FB_VSYNC_POL_LOW
                                | VSF_HW_FB_DE_POL_LOW
                                | VSF_HW_FB_PIXELCLK_NORMAL,
        },
    },
    .display_fb_layer0          = {
        .param                  = {
            .height             = VSF_BOARD_RGBLCD_LAYER0_HEIGHT,
            .width              = VSF_BOARD_RGBLCD_LAYER0_WIDTH,
            .drv                = &vk_disp_drv_fb,
            .color              = VSF_BOARD_RGBLCD_LAYER0_COLOR,
        },
#   ifdef VSF_BOARD_RGBLCD_LAYER0_SRAM_BUFFER_T
        .buffer                 = (void *)__vsf_hw_rgblcd_layer0_fb,
#   else
        .buffer                 = (void *)0xC0000000,
#   endif
        .drv                    = &vsf_disp_hw_fb_drv,
        .drv_param              = (void *)&vsf_board.hw_fb,
        .fb_size                = vsf_disp_get_pixel_format_bytesize(VSF_BOARD_RGBLCD_LAYER0_COLOR)
                                * VSF_BOARD_RGBLCD_LAYER0_WIDTH * VSF_BOARD_RGBLCD_LAYER0_HEIGHT,
        .fb_num                 = VSF_BOARD_RGBLCD_LAYER0_FBNUM,        // front/bancend frame buffer
        .layer_idx              = 0,
        .layer_color_type       = VSF_BOARD_RGBLCD_LAYER0_COLOR,
        .layer_default_color    = 0,
        .layer_alpha            = 0xFF,
        .layer_area             = {
            .pos.x              = (VSF_BOARD_RGBLCD_WIDTH - VSF_BOARD_RGBLCD_LAYER0_WIDTH) / 2,
            .pos.y              = (VSF_BOARD_RGBLCD_HEIGHT - VSF_BOARD_RGBLCD_LAYER0_HEIGHT) / 2,
            .size.x             = VSF_BOARD_RGBLCD_LAYER0_WIDTH,
            .size.y             = VSF_BOARD_RGBLCD_LAYER0_HEIGHT,
        },
    },
#   if defined(APP_MSCBOOT_CFG_FLASH) && VSF_USE_LINUX == ENABLED
    .display_dev                = NULL,
#   else
    .display_dev                = &vsf_board.display_fb_layer0.use_as__vk_disp_t,
#   endif
    .bl_port                    = (vsf_gpio_t *)&vsf_hw_gpio9,
    .rst_port                   = (vsf_gpio_t *)&vsf_hw_gpio9,
    .bl_pin                     = 10,
    .rst_pin                    = 11,
#   if      defined(VSF_BOARD_RGBLCD_LAYER1_WIDTH)                              \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_HEIGHT)                             \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_COLOR)                              \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_SRAM_BUFFER_T)
    .display_fb_layer1          = {
        .param                  = {
            .height             = VSF_BOARD_RGBLCD_LAYER1_HEIGHT,
            .width              = VSF_BOARD_RGBLCD_LAYER1_WIDTH,
            .drv                = &vk_disp_drv_fb,
            .color              = VSF_BOARD_RGBLCD_LAYER1_COLOR,
        },
        .buffer                 = (void *)__vsf_hw_rgblcd_layer1_fb,
        .drv                    = &vsf_disp_hw_fb_drv,
        .drv_param              = (void *)&vsf_board.hw_fb,
        .fb_size                = vsf_disp_get_pixel_format_bytesize(VSF_BOARD_RGBLCD_LAYER1_COLOR)
                                * VSF_BOARD_RGBLCD_LAYER1_WIDTH * VSF_BOARD_RGBLCD_LAYER1_HEIGHT,
        .fb_num                 = VSF_BOARD_RGBLCD_LAYER1_FBNUM,
        .layer_idx              = 1,
        .layer_color_type       = VSF_BOARD_RGBLCD_LAYER1_COLOR,
        .layer_alpha            = 0xFF,
        .layer_area             = {
            .pos.x              = (VSF_BOARD_RGBLCD_WIDTH - VSF_BOARD_RGBLCD_LAYER1_WIDTH) / 2,
            .pos.y              = (VSF_BOARD_RGBLCD_HEIGHT - VSF_BOARD_RGBLCD_LAYER1_HEIGHT) / 2,
            .size.x             = VSF_BOARD_RGBLCD_LAYER1_WIDTH,
            .size.y             = VSF_BOARD_RGBLCD_LAYER1_HEIGHT,
        },
    },
#   endif
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
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart1;
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

// Because debug stream for GD32H7 is not used,
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

void vsf_board_prepare_hw_for_linux(void)
{
#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
#   ifdef APP_MSCBOOT_CFG_FLASH
    char config[16];
    if (!app_config_read("scr.valid", config, sizeof(config)) && (config[0] == '1')) {
        if (app_config_read("scr.width", config, sizeof(config))) {
            vsf_trace_error("scr.width not configured" VSF_TRACE_CFG_LINEEND);
            goto scr_error;
        }
        *(uint16_t *)&vsf_board.display_fb_layer0.param.width = vsf_board.hw_fb.width = atoi(config);

        if (app_config_read("scr.height", config, sizeof(config))) {
            vsf_trace_error("scr.height not configured" VSF_TRACE_CFG_LINEEND);
            goto scr_error;
        }
        *(uint16_t *)&vsf_board.display_fb_layer0.param.height = vsf_board.hw_fb.height = atoi(config);

        if (app_config_read("scr.fps", config, sizeof(config))) {
            vsf_trace_warning("scr.fps not configured, use 60 by default" VSF_TRACE_CFG_LINEEND);
            vsf_board.hw_fb.timing.rgb.fps = 60;
        } else {
            vsf_board.hw_fb.timing.rgb.fps = atoi(config);
        }

        if (app_config_read("scr.hsw", config, sizeof(config))) {
            vsf_trace_error("scr.hsw not configured" VSF_TRACE_CFG_LINEEND);
            goto scr_error;
        }
        vsf_board.hw_fb.timing.rgb.hsw = atoi(config);
        if (app_config_read("scr.hbp", config, sizeof(config))) {
            vsf_trace_error("scr.hbp not configured" VSF_TRACE_CFG_LINEEND);
            goto scr_error;
        }
        vsf_board.hw_fb.timing.rgb.hbp = atoi(config);
        if (app_config_read("scr.hfp", config, sizeof(config))) {
            vsf_trace_error("scr.hfp not configured" VSF_TRACE_CFG_LINEEND);
            goto scr_error;
        }
        vsf_board.hw_fb.timing.rgb.hfp = atoi(config);
        if (app_config_read("scr.vsw", config, sizeof(config))) {
            vsf_trace_error("scr.vsw not configured" VSF_TRACE_CFG_LINEEND);
            goto scr_error;
        }
        vsf_board.hw_fb.timing.rgb.vsw = atoi(config);
        if (app_config_read("scr.vbp", config, sizeof(config))) {
            vsf_trace_error("scr.vbp not configured" VSF_TRACE_CFG_LINEEND);
            goto scr_error;
        }
        vsf_board.hw_fb.timing.rgb.vbp = atoi(config);
        if (app_config_read("scr.vfp", config, sizeof(config))) {
            vsf_trace_error("scr.vfp not configured" VSF_TRACE_CFG_LINEEND);
            goto scr_error;
        }
        vsf_board.hw_fb.timing.rgb.vfp = atoi(config);
        vsf_board.display_dev = &vsf_board.display_fb_layer0.use_as__vk_disp_t;
    }

    return;

scr_error:
    vsf_board.display_dev = NULL;
#   else
    vsf_board.display_dev = &vsf_board.display_fb_layer0.use_as__vk_disp_t;
#   endif
#endif
    return;
}

VSF_CAL_WEAK(delay_us)
void delay_us(uint32_t us)
{
    vsf_systimer_tick_t cur_us = vsf_systimer_tick_to_us(vsf_systimer_get()), due_us = cur_us + us;
    while (vsf_systimer_tick_to_us(vsf_systimer_get()) < due_us);
}

VSF_CAL_WEAK(delay_ms)
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
    // code below depends on 300MHz EXMC clock
    VSF_ASSERT(vsf_hw_clk_get_freq_hz(&VSF_HW_CLK_EXMC) == 300 * 1000 * 1000);

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

void vsf_board_init(void)
{
    static const vsf_gpio_port_cfg_pins_t __cfgs[] = {
        // USART: PA2/PA3
        {VSF_PORTA, (1 << 2) | (1 << 3), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP, 7},

        // SDRAM: PC0/PC4..PC5, PD0..PD1/PD8..PD10/PD14..PD15, PE0..PE1/PE7..PE15, PF0..PF5/PF11.PF15, PG0..PG2/PG4..PG5/PG8/PG15
        {VSF_PORTC, (1 << 0) | (3 << 4), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_85MHZ, 12},
        {VSF_PORTD, (3 << 0) | (7 << 8) | (3 << 14), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_85MHZ, 12},
        {VSF_PORTE, (3 << 0) | (0x1FF << 7), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_85MHZ, 12},
        {VSF_PORTF, (0x3F << 0) | (0x01F << 11), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_85MHZ, 12},
        {VSF_PORTG, (7 << 0) | (3 << 4) | (1 << 8) | (1 << 15), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_85MHZ, 12},
        // SDIO: PB13, PD2, PC9..PC12
#if VSF_HAL_USE_SDIO == ENABLED
        {VSF_PORTB, (1 << 13), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_100MHZ_220MHZ, 12},
        {VSF_PORTD, (1 << 2), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_100MHZ_220MHZ, 12},
        {VSF_PORTC, (1 << 12), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_FLOATING | VSF_GPIO_SPEED_100MHZ_220MHZ, 12},
        {VSF_PORTC, (7 << 9), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_100MHZ_220MHZ, 12},
#endif
#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
        // back light: PJ10, reset: PJ11
        {VSF_PORTJ, (3 << 10), VSF_GPIO_OUTPUT_PUSH_PULL | VSF_GPIO_PULL_UP | VSF_GPIO_SPEED_60MHZ},
        // PA1/PA5..PA7/PA9..PA10/PA15
        // Note PA2 is shared with USART1_TX, we lost R1
        {VSF_PORTA, (1 << 1) | (7 << 5) | (3 << 9) | (1 << 15), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 14},
        // PA8
        {VSF_PORTA, (1 << 8), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 13},
        // PB0..PB1
        {VSF_PORTB, (3 << 0), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 9},
        // PB5
        {VSF_PORTB, (1 << 5), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 3},
        // PB9..PB11
        {VSF_PORTB, (7 << 9), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 14},
        // PC6..PC7
        {VSF_PORTC, (3 << 6), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 14},
        // PD6
        {VSF_PORTD, (1 << 6), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 14},
        // PE4..PE6
        {VSF_PORTE, (7 << 4), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 14},
        // PF10
        {VSF_PORTF, (1 << 10), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 14},
        // PG6..PG7/PG13
        {VSF_PORTG, (3 << 6) | (1 << 13), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 14},
        // PG10/PG12
        {VSF_PORTG, (1 << 10) | (1 << 12), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 9},
        // PK2
        {VSF_PORTK, (1 << 2), VSF_GPIO_AF_PUSH_PULL | VSF_GPIO_NO_PULL_UP_DOWN | VSF_GPIO_SPEED_100MHZ_220MHZ, 14},
#endif
    };
    vsf_hw_gpio_ports_config_pins((vsf_gpio_port_cfg_pins_t *)__cfgs, dimof(__cfgs));

#ifdef __VSF_BOARD_USE_UART_AS_DEBUG_STREAM
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_RX);
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);
#endif

    __sdram_init(EXMC_SDRAM_DEVICE0);

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
#   ifdef VSF_BOARD_RGBLCD_LAYER0_SRAM_BUFFER_T
    vsf_arch_mpu_add_region(0xC0000000, 32 * 1024 * 1024,
                            VSF_ARCH_MPU_NON_SHARABLE           |
                            VSF_ARCH_MPU_EXECUTABLE             |
                            VSF_ARCH_MPU_ACCESS_FULL            |
                            VSF_ARCH_MPU_CACHABLE_WRITE_BACK_ALLOC);
#   else
    // All SDRAM MUST be write-through because for write-back,data will be in cache
    //  before a block transfer to SDRAM, which will take the bus for a long time and
    //  maybe affect normal TLI operation.
    vsf_arch_mpu_add_region(0xC0000000, 32 * 1024 * 1024,
                            VSF_ARCH_MPU_NON_SHARABLE           |
                            VSF_ARCH_MPU_EXECUTABLE             |
                            VSF_ARCH_MPU_ACCESS_FULL            |
                            VSF_ARCH_MPU_CACHABLE_WRITE_THROUGH_NOALLOC);

    // it's a MUST to set frame buffer to write-through, no wirte allocate
    vsf_arch_mpu_add_region(0xC0000000, 4 * 1024 * 1024,
                            VSF_ARCH_MPU_NON_SHARABLE           |
                            VSF_ARCH_MPU_NON_EXECUTABLE         |
                            VSF_ARCH_MPU_ACCESS_FULL            |
                            VSF_ARCH_MPU_CACHABLE_WRITE_THROUGH_NOALLOC);
#   endif
#else
    vsf_arch_mpu_add_region(0xC0000000, 32 * 1024 * 1024,
                            VSF_ARCH_MPU_NON_SHARABLE           |
                            VSF_ARCH_MPU_EXECUTABLE             |
                            VSF_ARCH_MPU_ACCESS_FULL            |
                            VSF_ARCH_MPU_CACHABLE_WRITE_BACK_ALLOC);
#endif

#ifndef VSF_LINUX_CFG_HEAP_SIZE
    vsf_heap_add_memory((vsf_mem_t){
#   ifdef VSF_BOARD_RGBLCD_LAYER0_SRAM_BUFFER_T
        .buffer     = (void *)(0xC0000000),
        .size       = 32 * 1024 * 1024,
#   else
        .buffer     = (void *)(0xC0000000 + 4 * 1024 * 1024),
        .size       = 28 * 1024 * 1024,
#   endif
    });
#endif
    vsf_gpio_set(vsf_board.bl_port, 1 << vsf_board.bl_pin);
    vsf_gpio_set_output(vsf_board.bl_port, 1 << vsf_board.bl_pin);
}

// WORKAROUNDS

#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
// if TLI is enabled, and sdram is used as frame buffer, original memcpy will use
//  performance optimization to read/write multiple words in one instructions,
//  which will cause delay in accessing frame buffer by TLI.
// Implement memcpy without these instructions
void *memcpy(void *dst, const void *src, size_t n)
{
    size_t aligned_n = n & ~3, round;

    if (!((uintptr_t)dst & 3) && !((uintptr_t)src & 3) && (aligned_n > 0)) {
        uint32_t *src32 = (uint32_t *)src;
        uint32_t *dst32 = (uint32_t *)dst;
        src = (const void *)((uintptr_t)src + aligned_n);
        dst = (void *)((uintptr_t)dst + aligned_n);
        n -= aligned_n;
        aligned_n >>= 2;

        round = (aligned_n + 7) >> 3;
        switch (aligned_n & 7) {
            case 0: do {    *dst32++ = *src32++;
            case 7:         *dst32++ = *src32++;
            case 6:         *dst32++ = *src32++;
            case 5:         *dst32++ = *src32++;
            case 4:         *dst32++ = *src32++;
            case 3:         *dst32++ = *src32++;
            case 2:         *dst32++ = *src32++;
            case 1:         *dst32++ = *src32++;
            } while (--round);
        }
    }

    if (n > 0) {
        uint8_t *src8 = (uint8_t *)src;
        uint8_t *dst8 = (uint8_t *)dst;
        round = (n + 7) >> 3;
        switch (n & 7) {
            case 0: do {    *dst8++ = *src8++;
            case 7:         *dst8++ = *src8++;
            case 6:         *dst8++ = *src8++;
            case 5:         *dst8++ = *src8++;
            case 4:         *dst8++ = *src8++;
            case 3:         *dst8++ = *src8++;
            case 2:         *dst8++ = *src8++;
            case 1:         *dst8++ = *src8++;
            } while (--round);
        }
    }
    return dst;
}
#endif

// Enable RGB_TEST and disable other codes in application
// Do not enable VSF_BOARD_RGBLCD_LAYER0_SRAM_BUFFER_T, so that the frame buffer is in SDRAM.
#define RGB_TEST 0
#if RGB_TEST

#ifdef VSF_BOARD_RGBLCD_LAYER0_SRAM_BUFFER_T
#   warning VSF_BOARD_RGBLCD_LAYER0_SRAM_BUFFER_T is defined, frame buffer will not be in SDRAM, test below is meaningless.
#endif

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
#if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#endif

    if (vsf_board.display_dev != NULL) {
        vk_disp_init(vsf_board.display_dev);

        if (vsf_board.display_fb_layer0.param.color == VSF_DISP_COLOR_RGB565) {
            uint16_t *ptr = (uint16_t *)vsf_board.display_fb_layer0.buffer;
            for (int i = 0; i < vsf_board.display_fb_layer0.param.height; i++) {
                for (int j = 0; j < vsf_board.display_fb_layer0.param.width; j++) {
                    if (j < (vsf_board.display_fb_layer0.param.width / 2)) {
                        *ptr++ = 0xF800;
                    } else {
                        *ptr++ = 0x001F;
                    }
                }
            }
        } else if (vsf_board.display_fb_layer0.param.color == VSF_DISP_COLOR_ARGB8888) {
            uint32_t *ptr = (uint32_t *)vsf_board.display_fb_layer0.buffer;
            for (int i = 0; i < vsf_board.display_fb_layer0.param.height; i++) {
                for (int j = 0; j < vsf_board.display_fb_layer0.param.width; j++) {
                    if (j < (vsf_board.display_fb_layer0.param.width / 2)) {
                        *ptr++ = 0xFFFF0000;
                    } else {
                        *ptr++ = 0xFF0000FF;
                    }
                }
            }
        } else {
            vsf_trace_error("not supported color" VSF_TRACE_CFG_LINEEND);
            VSF_ASSERT(false);
        }

        uint32_t *src = (uint32_t *)0xC0000000 + 1 * 1024 * 1024;
        uint32_t *dst = src + 1 * 1024 * 1024;
#if 0
        // write data to sdram will be OK even if sdram is configured to write_back_alloc
        while (1) {
            for (int i = 0; i < 64 * 1024; i++) {
                src[i] = i;
            }
        }
#elif 0
        // if sdram is configured to write_back_alloc, test below will fail
        // if sdram is configured to write_through_noalloc, test below will be ok
        while (1) {
            for (int i = 0; i < 64 * 1024; i++) {
                src[i] = i;
            }
            for (int i = 0; i < 64 * 1024; i++) {
                if (src[i] != i) {
                    vsf_trace_error("fail to verify 0x%p\r\n", &src[i]);
                }
            }
        }
#elif 1
        // test below will fail without memcpy reimplementation above
        while (1) {
            memcpy(dst, src, 4 * 1024 * 1024);
        }
#endif
    }
}
#endif
