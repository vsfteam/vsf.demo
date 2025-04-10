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

#define __VSF_EDA_CLASS_INHERIT__
#define __VSF_DISP_CLASS_INHERIT__
#define __VSF_SIMPLE_STREAM_CLASS_INHERIT__
#include "./vsf_board.h"

#include "bf0_hal_cortex.h"
#include "bf0_hal_gpio.h"
#include "bf0_hal_pinmux.h"
#include "bf0_hal_lcdc.h"

/*============================ MACROS ========================================*/

#if VSF_BOARD_DISP_COLOR == VSF_DISP_COLOR_RGB565
#   define LCDC_PIXEL_FORMAT        LCDC_PIXEL_FORMAT_RGB565
#elif VSF_BOARD_DISP_COLOR == VSF_DISP_COLOR_ARGB8888
#   define LCDC_PIXEL_FORMAT        LCDC_PIXEL_FORMAT_ARGB888
#elif VSF_BOARD_DISP_COLOR == VSF_DISP_COLOR_RGB888
#   define LCDC_PIXEL_FORMAT        LCDC_PIXEL_FORMAT_RGB888
#else
#   error add color support
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

typedef struct sf32_lcdc_info_t {
    LCDC_InitTypeDef Init;

    const uint8_t *init_seq;
    uint16_t init_seq_len;
    uint16_t width;
    uint16_t height;
    uint16_t x_offset;
    uint16_t y_offset;
} sf32_lcdc_info_t;

typedef struct vk_disp_sf32_lcdc_t {
    implement(vk_disp_t)
    const sf32_lcdc_info_t *info;

    LCDC_HandleTypeDef handle;
} vk_disp_sf32_lcdc_t;

/*============================ LOCAL VARIABLES ===============================*/

#if VSF_USE_USB_HOST == ENABLED
#endif

static const uint8_t __lcdc_init_seq_co5300[] = {
    VSF_DISP_MIPI_LCD_INITSEQ(VSF_DISP_MIPI_LCD_CO5300_BASE)
};

static const sf32_lcdc_info_t __sf32_lcdc_co5300_info = {
    .Init                       = {
        .lcd_itf = LCDC_INTF_SPI_DCX_4DATA,
        .freq = 48000000,        //CO5300 RGB565 only support 50000000,  RGB888 support 60000000
        .color_mode = LCDC_PIXEL_FORMAT,

        .cfg = {
            .spi = {
                .dummy_clock = 0,
                .syn_mode = HAL_LCDC_SYNC_DISABLE,
                .vsyn_polarity = 1,
                //default_vbp=2, frame rate=82, delay=115us,
                //TODO: use us to define delay instead of cycle, delay_cycle=115*48
                .vsyn_delay_us = 0,
                .hsyn_num = 0,
            },
        },
    },
    .init_seq                   = __lcdc_init_seq_co5300,
    .init_seq_len               = sizeof(__lcdc_init_seq_co5300),
    .width                      = 390,
    .height                     = 450,
    .x_offset                   = (390 - VSF_BOARD_DISP_WIDTH) / 2,
    .y_offset                   = (450 - VSF_BOARD_DISP_HEIGHT) / 2,
};

static vk_disp_sf32_lcdc_t __vk_disp_sf32_lcdc = {
    .param                      = {
        .width                  = VSF_BOARD_DISP_WIDTH,
        .height                 = VSF_BOARD_DISP_HEIGHT,
        .color                  = VSF_BOARD_DISP_COLOR,
    },
    .info                       = &__sf32_lcdc_co5300_info,
};

/*============================ GLOBAL VARIABLES ==============================*/

#if VSF_USE_USB_DEVICE == ENABLED
#endif

#if VSF_USE_BOARD == ENABLED
vsf_board_t vsf_board = {
#if VSF_HAL_USE_I2C == ENABLED
    .i2c                        = (vsf_i2c_t *)&vsf_hw_i2c0,
#endif
#if VSF_HAL_USE_SDIO == ENABLED
    .sdio                       = (vsf_sdio_t *)&vsf_hw_sdio0,
    .sdio_bus_width             = 4,
    .sdio_voltage               = SD_OCR_VDD_32_33 | SD_OCR_VDD_33_34,
#endif
#if VSF_USE_AUDIO == ENABLED
#   if VSF_AUDIO_USE_DUMMY == ENABLED
    .audio_dev                  = &vsf_board.audio_dummy.use_as__vk_audio_dev_t,
    .audio_dummy                = {
        .drv                    = &vk_audio_dummy_drv,
    },
#   endif
#endif
#if VSF_USE_UI == ENABLED
    .display_dev                = &__vk_disp_sf32_lcdc.use_as__vk_disp_t,
#endif
};
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

#if VSF_USE_USB_DEVICE == ENABLED
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
    vsf_stream_connect_tx(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t);
    __vsf_hw_usart_config_isr((vsf_hw_usart_t *)debug_usart, &(vsf_usart_isr_t){
        .handler_fn     = __vsf_debug_stream_isrhandler,
        .target_ptr     = &VSF_DEBUG_STREAM_RX,
        .prio           = vsf_arch_prio_0,
    });
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
    return true;
}

// flash table for W25Q01JVIM

#if SF32_USE_W25Q01JVIM

#   include "flash_table.h"

static FLASH_RDID_TYPE_T flash_cmd_id_pool_typ2[] =
{
    {0xef, 0x70, 0x21, 1, 0x8000000},   //W25Q01JVIM
    {FLASH_INVALID_ID, 0, 0, 0, 0},      //last one
};

FLASH_RDID_TYPE_T * flash_cmd_id_pool[] =
{
    NULL,
    NULL,
    &flash_cmd_id_pool_typ2[0],
    NULL,
    NULL,
    NULL,
};
#endif

// LCD driver

#define LCD_RESET_PIN           (0)
#define LCD_BACKLIGHT_PIN       (1)
#define TP_RESET                (9)
#define LCD_VADD_EN             (37)

#define LCD_ID                  0x331100

static void LCD_ReadMode(LCDC_HandleTypeDef *hlcdc, bool enable)
{
    if (enable) {
        HAL_LCDC_SetFreq(hlcdc, 2000000);
    } else {
        HAL_LCDC_SetFreq(hlcdc, __sf32_lcdc_co5300_info.Init.freq);
    }
}

static uint32_t LCD_ReadData(LCDC_HandleTypeDef *hlcdc, uint16_t RegValue, uint8_t ReadSize)
{
    uint32_t rd_data = 0;

    LCD_ReadMode(hlcdc, true);
    HAL_LCDC_ReadU32Reg(hlcdc, ((0x03 << 24) | (RegValue << 8)), (uint8_t *)&rd_data, ReadSize);
    LCD_ReadMode(hlcdc, false);
    return rd_data;
}

static void LCD_WriteReg(LCDC_HandleTypeDef *hlcdc, uint16_t LCD_Reg, uint8_t *Parameters, uint32_t NbParameters)
{
    uint32_t cmd;

    if (MIPI_DCS_CMD_HEX_CODE_WRITE_MEMORY_START == LCD_Reg) {
        cmd = (0x32 << 24) | (LCD_Reg << 8);
    } else {
        cmd = (0x02 << 24) | (LCD_Reg << 8);
    }
    HAL_LCDC_WriteU32Reg(hlcdc, cmd, Parameters, NbParameters);
}

void LCDC1_IRQHandler(void)
{
    HAL_LCDC_IRQHandler(&__vk_disp_sf32_lcdc.handle);
}

static void __vk_lcdc_refresh_on_ready(LCDC_HandleTypeDef *lcdc)
{
    vk_disp_on_ready(vsf_board.display_dev);
}

static vsf_err_t __vk_disp_sf32_lcdc_refresh(vk_disp_t *pthis, vk_disp_area_t *area, void *disp_buff)
{
    vk_disp_sf32_lcdc_t *sf32_lcdc = (vk_disp_sf32_lcdc_t *)pthis;
    LCDC_HandleTypeDef *hlcdc = &sf32_lcdc->handle;
    uint8_t parameter[4];
    uint16_t Xpos0 = area->pos.x;
    uint16_t Ypos0 = area->pos.y;
    uint16_t Xpos1 = area->pos.x + area->size.x - 1;
    uint16_t Ypos1 = area->pos.y + area->size.y - 1;

    HAL_LCDC_Exit_LP(hlcdc);
    HAL_LCDC_SetROIArea(hlcdc, Xpos0, Ypos0, Xpos1, Ypos1);

    parameter[0] = (Xpos0 + sf32_lcdc->info->x_offset) >> 8;
    parameter[1] = (Xpos0 + sf32_lcdc->info->x_offset) & 0xFF;
    parameter[2] = (Xpos1 + sf32_lcdc->info->x_offset) >> 8;
    parameter[3] = (Xpos1 + sf32_lcdc->info->x_offset) & 0xFF;
    LCD_WriteReg(hlcdc, MIPI_DCS_CMD_HEX_CODE_SET_COLUMN_ADDRESS, parameter, 4);

    parameter[0] = (Ypos0 + sf32_lcdc->info->y_offset) >> 8;
    parameter[1] = (Ypos0 + sf32_lcdc->info->y_offset) & 0xFF;
    parameter[2] = (Ypos1 + sf32_lcdc->info->y_offset) >> 8;
    parameter[3] = (Ypos1 + sf32_lcdc->info->y_offset) & 0xFF;
    LCD_WriteReg(hlcdc, MIPI_DCS_CMD_HEX_CODE_SET_PAGE_ADDRESS, parameter, 4);

    HAL_LCDC_LayerSetData(hlcdc, HAL_LCDC_LAYER_DEFAULT, (uint8_t *)disp_buff, Xpos0, Ypos0, Xpos1, Ypos1);
    HAL_LCDC_SendLayerData2Reg_IT(hlcdc, ((0x32 << 24) | (MIPI_DCS_CMD_HEX_CODE_WRITE_MEMORY_START << 8)), 4);
    HAL_LCDC_Enter_LP(hlcdc);
    return VSF_ERR_NONE;
}

static vsf_err_t __vk_disp_sf32_lcdc_init(vk_disp_t *pthis)
{
    vk_disp_sf32_lcdc_t *sf32_lcdc = (vk_disp_sf32_lcdc_t *)pthis;
    LCDC_HandleTypeDef *hlcdc = &sf32_lcdc->handle;
    uint8_t *__init_seq_cur = (uint8_t *)sf32_lcdc->info->init_seq;

// already called in vsf_board_prepare_hw_for_linux when detecting screen
//  HAL_LCDC_Enter_LP(hlcdc);
//  HAL_LCDC_Init(hlcdc);

    while ((__init_seq_cur - sf32_lcdc->info->init_seq) < sf32_lcdc->info->init_seq_len) {
        if ((0 == __init_seq_cur[0]) && (__init_seq_cur[1] != 0)) {
            // should be delay_ms(__init_seq_cur[1])
            VSF_ASSERT(false);
        } else {
            LCD_WriteReg(hlcdc, __init_seq_cur[0], &__init_seq_cur[2], __init_seq_cur[1]);
            __init_seq_cur += __init_seq_cur[1] + 2;
        }
    }

    HAL_LCDC_SetBgColor(hlcdc, 0, 0, 0);
    HAL_LCDC_LayerReset(hlcdc, HAL_LCDC_LAYER_DEFAULT);

    uint8_t bright = 255;
    LCD_WriteReg(hlcdc, MIPI_DCS_CMD_HEX_CODE_SET_DISPLAY_BRIGHTNESS, &bright, 1);

    HAL_LCDC_LayerSetFormat(hlcdc, 0, LCDC_PIXEL_FORMAT);

    HAL_NVIC_SetPriority(LCDC1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(LCDC1_IRQn);

    vk_disp_on_ready(vsf_board.display_dev);
    return VSF_ERR_NONE;
}

static const vk_disp_drv_t __vk_disp_sf32_lcdc_drv = {
    .init               = __vk_disp_sf32_lcdc_init,
    .refresh            = __vk_disp_sf32_lcdc_refresh,
};

void vsf_board_prepare_hw_for_linux(void)
{
    // LCDC1 - QSPI
    HAL_PIN_Set(PAD_PA02, LCDC1_SPI_TE, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA03, LCDC1_SPI_CS, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA04, LCDC1_SPI_CLK, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA05, LCDC1_SPI_DIO0, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA06, LCDC1_SPI_DIO1, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA07, LCDC1_SPI_DIO2, PIN_NOPULL, 1);
    HAL_PIN_Set(PAD_PA08, LCDC1_SPI_DIO3, PIN_NOPULL, 1);

    HAL_PIN_Set(PAD_PA09, GPIO_A9,  PIN_NOPULL, 1);    // CTP_RESET
    HAL_PIN_Set(PAD_PA31, GPIO_A31, PIN_NOPULL, 1);    // CTP_INT
    HAL_PIN_Set(PAD_PA30, I2C1_SCL, PIN_PULLUP, 1);
    HAL_PIN_Set(PAD_PA33, I2C1_SDA, PIN_PULLUP, 1);

    HAL_GPIO_Init(hwp_gpio1, &(GPIO_InitTypeDef){
        .Mode = GPIO_MODE_OUTPUT,
        .Pin = LCD_VADD_EN,
        .Pull = GPIO_NOPULL,
    });
    HAL_GPIO_WritePin(hwp_gpio1, LCD_VADD_EN, (GPIO_PinState)1);

    HAL_GPIO_Init(hwp_gpio1, &(GPIO_InitTypeDef){
        .Mode = GPIO_MODE_OUTPUT,
        .Pin = LCD_RESET_PIN,
        .Pull = GPIO_NOPULL,
    });

    HAL_GPIO_Init(hwp_gpio1, &(GPIO_InitTypeDef){
        .Mode = GPIO_MODE_OUTPUT,
        .Pin = LCD_BACKLIGHT_PIN,
        .Pull = GPIO_NOPULL,
    });
    HAL_GPIO_WritePin(hwp_gpio1, LCD_BACKLIGHT_PIN, (GPIO_PinState)1);

    LCDC_HandleTypeDef *hlcdc = &__vk_disp_sf32_lcdc.handle;
    hlcdc->Instance = LCDC1;
    hlcdc->Init = __sf32_lcdc_co5300_info.Init;
    hlcdc->XferCpltCallback = __vk_lcdc_refresh_on_ready;

    HAL_LCDC_Enter_LP(hlcdc);
    HAL_LCDC_Init(hlcdc);

    HAL_GPIO_WritePin(hwp_gpio1, LCD_RESET_PIN, (GPIO_PinState)1);
    vsf_thread_delay_ms(10);
    HAL_GPIO_WritePin(hwp_gpio1, LCD_RESET_PIN, (GPIO_PinState)0);
    vsf_thread_delay_ms(10);
    HAL_GPIO_WritePin(hwp_gpio1, LCD_RESET_PIN, (GPIO_PinState)1);
    vsf_thread_delay_ms(50);

    uint32_t lcd_id = LCD_ReadData(hlcdc, MIPI_DCS_CMD_HEX_CODE_GET_DISPLAY_ID, 3);
    if (lcd_id == LCD_ID) {
        ((vk_disp_param_t *)(&__vk_disp_sf32_lcdc.param))->drv = &__vk_disp_sf32_lcdc_drv;
        vsf_trace_debug("LCD: 0x%08X" VSF_TRACE_CFG_LINEEND, lcd_id);
    } else {
        ((vk_disp_param_t *)(&__vk_disp_sf32_lcdc.param))->drv = &vk_disp_dummy_drv;
        HAL_LCDC_DeInit(hlcdc);
    }

    return;
}

void vsf_board_init(void)
{
#ifdef __VSF_BOARD_USE_UART_AS_DEBUG_STREAM
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_RX);
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);
#endif

    vsf_heap_add_memory((vsf_mem_t){
        .buffer     = (void *)0x60000000,
        .size       = 8 * 1024 * 1024,
    });
}
