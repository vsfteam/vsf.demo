/*****************************************************************************
 *   Copyright(C)2009-2026 by VSF Team                                       *
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

#ifndef __VSF_USR_CFG_H__
#define __VSF_USR_CFG_H__

#if    !defined(VSF_TEST_KERNEL_PROFILE_EDA) && !defined(VSF_TEST_KERNEL_PROFILE_THREAD) \
    && !defined(VSF_TEST_KERNEL_PROFILE_LINUX)
#   define VSF_USE_HEAP                                 DISABLED
#endif

#include "vsf_board_cfg.h"

#if defined(VSF_TEST_KERNEL_PROFILE_BAREMETAL)
#   include "./vsf_usr_cfg/vsf_usr_cfg_baremetal.h"
#elif defined(VSF_TEST_KERNEL_PROFILE_EDA)
#   include "./vsf_usr_cfg/vsf_usr_cfg_eda.h"
#elif defined(VSF_TEST_KERNEL_PROFILE_THREAD)
#   include "./vsf_usr_cfg/vsf_usr_cfg_thread.h"
#elif defined(VSF_TEST_KERNEL_PROFILE_LINUX)
#   include "./vsf_usr_cfg/vsf_usr_cfg_linux.h"
#else
#   include "./vsf_usr_cfg/vsf_usr_cfg_baremetal.h"
#endif

#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_TRACE                                   ENABLED
#define VSF_USE_TEST                                    ENABLED
#define VSF_TEST_CFG_EMIT_MARKERS                       ENABLED
#define VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE                128

#ifndef VSF_TEST_I2C_BUS_SCAN_ENABLE
#   define VSF_TEST_I2C_BUS_SCAN_ENABLE                DISABLED
#endif
#ifndef VSF_TEST_USART_HW_FLOW_CONTROL_ENABLE
#   define VSF_TEST_USART_HW_FLOW_CONTROL_ENABLE       DISABLED
#endif
// wdt_reboot / wdt_timeout_accuracy disabled — WDT reset not verified on HW.
// See TODO in vsf/source/hal/driver/RaspberryPi/RP2040/wdt/wdt.c.
#ifndef VSF_TEST_WDT_REBOOT_ENABLE
#   define VSF_TEST_WDT_REBOOT_ENABLE                  DISABLED
#endif
#ifndef VSF_TEST_WDT_TIMEOUT_ACCURACY_ENABLE
#   define VSF_TEST_WDT_TIMEOUT_ACCURACY_ENABLE        DISABLED
#endif
#ifndef VSF_TEST_I2C_SLAVE_ENABLE
#   define VSF_TEST_I2C_SLAVE_ENABLE                   DISABLED
#endif
#ifndef VSF_TEST_I2C_SLAVE_FIFO_ENABLE
#   define VSF_TEST_I2C_SLAVE_FIFO_ENABLE              DISABLED
#endif
#ifndef VSF_TEST_RTC_ALARM_ENABLE
#   define VSF_TEST_RTC_ALARM_ENABLE                   DISABLED
#endif
#ifndef VSF_TEST_RTC_EPOCH_ENABLE
#   define VSF_TEST_RTC_EPOCH_ENABLE                   DISABLED
#endif
#ifndef VSF_TEST_ADC_STREAM_ENABLE
#   define VSF_TEST_ADC_STREAM_ENABLE                  DISABLED
#endif

#endif
