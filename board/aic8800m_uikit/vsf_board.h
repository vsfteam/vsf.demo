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

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/

typedef struct vsf_board_t {
    vsf_usart_t *usart;
    vsf_spi_t *spi;
    vsf_i2c_t *i2c;
    vsf_mmc_t *mmc;
    uint8_t mmc_bus_width;
    uint32_t mmc_voltage;

#if VSF_USE_UI == ENABLED
    vk_disp_t *display_dev;
    vk_disp_mipi_lcd_t disp_spi_mipi;
#endif
#if VSF_USE_AUDIO == ENABLED
    vk_audio_dev_t *audio_dev;
    // TODO: add audio device instance
#endif
#if VSF_USE_USB_HOST == ENABLED
    vk_usbh_t usbh_dev;
#endif
} vsf_board_t;

/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_board_t vsf_board;

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_board_init(void);

#endif      // __VSF_BOARD_CFG_WIN_H__
