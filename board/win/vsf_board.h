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

#if VSF_USE_DISTBUS == ENABLED && VSF_HAL_USE_DISTBUS == ENABLED && defined(VSF_BOARD_CFG_DISTBUS_USART)
#   define __APP_USE_DISTBUS                    ENABLED
#endif

#if __APP_USE_DISTBUS == ENABLED
#   include "transport/vsf_distbus_transport.h"
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/

typedef struct vsf_board_t {
#if __APP_USE_DISTBUS == ENABLED
    vsf_usart_t *distbus_usart;
    struct {
#define VSF_BOARD_HAL_DISTBUS_DEFINE(__TYPE)                                    \
        struct {                                                                \
            uint8_t dev_num;                                                    \
            VSF_MCONNECT(vsf_, __TYPE, _t) *dev[16];                            \
        } __TYPE;

#define __VSF_HAL_DISTBUS_ENUM  VSF_BOARD_HAL_DISTBUS_DEFINE
#include "hal/driver/vsf/distbus/vsf_hal_distbus_enum.inc"
    } chip;
#endif

    vsf_usart_t *usart;
#if VSF_USE_UI == ENABLED
    vk_disp_t *display_dev;
    vk_disp_wingdi_t disp_wingdi;
#endif
#if VSF_USE_AUDIO == ENABLED
    vk_audio_dev_t *audio_dev;
    vk_winsound_dev_t audio_winsound;
#endif
#if VSF_USE_USB_HOST == ENABLED
    vk_usbh_t usbh_dev;
#endif
#if VSF_USE_FS == ENABLED
    const vk_fs_op_t *fsop;
    void * fsinfo;
    vk_winfs_info_t winfs_info;
#endif
} vsf_board_t;

/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_board_t vsf_board;

#if __APP_USE_DISTBUS == ENABLED

#   define VSF_HAL_HW_DECLARE(__N, __VALUE)                                     \
        extern VSF_MCONNECT(vsf_remapped_, VSF_HAL_CFG_IMP_TYPE, _t) VSF_MCONNECT(vsf_hw_, VSF_HAL_CFG_IMP_TYPE, __N);
#   define VSF_HAL_HW_DECLARE_MULTI()                                           \
        VSF_MREPEAT(VSF_MCONNECT(VSF_HW_, VSF_HAL_CFG_IMP_UPCASE_TYPE, _COUNT), VSF_HAL_HW_DECLARE, NULL)\
        extern VSF_MCONNECT(vsf_remapped_, VSF_HAL_CFG_IMP_TYPE, _t) *VSF_MCONNECT(vsf_hw_, VSF_HAL_CFG_IMP_TYPE)[VSF_MCONNECT(VSF_HW_, VSF_HAL_CFG_IMP_UPCASE_TYPE, _COUNT)];

#   if VSF_HAL_USE_IO == ENABLED
#       ifndef VSF_HW_IO_COUNT
#           define VSF_HW_IO_COUNT                  1
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 io
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          IO
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_GPIO == ENABLED
#       ifndef VSF_HW_GPIO_COUNT
#           define VSF_HW_GPIO_COUNT                32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 gpio
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          GPIO
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_I2C == ENABLED
#       ifndef VSF_HW_I2C_COUNT
#           define VSF_HW_I2C_COUNT                 32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 i2c
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          I2C
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_SPI == ENABLED
#       ifndef VSF_HW_SPI_COUNT
#           define VSF_HW_SPI_COUNT                 32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 spi
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          SPI
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_MMC == ENABLED
#       ifndef VSF_HW_MMC_COUNT
#           define VSF_HW_MMC_COUNT                 32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 mmc
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          MMC
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_ADC == ENABLED
#       ifndef VSF_HW_ADC_COUNT
#           define VSF_HW_ADC_COUNT                 32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 adc
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          ADC
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_DAC == ENABLED
#       ifndef VSF_HW_DAC_COUNT
#           define VSF_HW_DAC_COUNT                 32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 dac
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          DAC
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_PWM == ENABLED
#       ifndef VSF_HW_PWM_COUNT
#           define VSF_HW_PWM_COUNT                 32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 pwm
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          PWM
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_I2S == ENABLED
#       ifndef VSF_HW_I2S_COUNT
#           define VSF_HW_I2S_COUNT                 32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 i2s
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          I2S
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#   if VSF_HAL_USE_USART == ENABLED && VSF_WIN_USART_CFG_USE_AS_HW_USART != ENABLED
#       ifndef VSF_HW_USART_COUNT
#           define VSF_HW_USART_COUNT               32
#       endif

#       undef VSF_HAL_CFG_IMP_TYPE
#       undef VSF_HAL_CFG_IMP_UPCASE_TYPE
#       define VSF_HAL_CFG_IMP_TYPE                 usart
#       define VSF_HAL_CFG_IMP_UPCASE_TYPE          USART
VSF_HAL_HW_DECLARE_MULTI()
#   endif

#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_board_init(void);

#endif      // __VSF_BOARD_CFG_WIN_H__
