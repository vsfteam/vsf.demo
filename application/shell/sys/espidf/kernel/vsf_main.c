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

/*
 * espidf demo
 * Based on espidf sub-system
 *
 * Dependency:
 *
 * Submodule:
 *   vsf
 *     source/component/3rd-party/btstack/raw if VSF_USE_BTSTACK is enabled
 *     source/component/3rd-party/mbedtls/raw if VSF_USE_MBEDTLS is enabled
 *     source/component/3rd-party/PLOOC/raw
 *   optional:
 *    for root directory in embedded hardware, littlefs is used
 *     source/component/3rd-party/littlefs/raw
 *
 * Board:
 *
 * Include Directories necessary for espidf:
 *   vsf/source/shell/sys/espidf/include
 *   optional:
 *    for root directory in embedded hardware, littlefs is used
 *     vsf/source/component/3rd-party/littlefs/port
 *     vsf/source/component/3rd-party/littlefs/raw
 *
 * Pre-defined:
 *   LFS_CONFIG to lfs_util_vsf.h
 *
 * Sources necessary for espidf:
 *   vsf/source/shell/sys/espidf/*.c
 *   ./*.c
 *   optional:
 *    for root directory in embedded hardware, littlefs is used
 *     vsf/source/component/3rd-party/littlefs/port/.*
 *     vsf/source/component/3rd-party/littlefs/raw/.*
 *     vsf/source/component/3rd-party/mbedtls/raw/library/.* if VSF_USE_MBEDTLS is enabled
 *     vsf/source/component/3rd-party/mbedtls/port/.* if VSF_USE_MBEDTLS is enabled
 *     vsf/source/component/3rd-party/mbedtls/extension/tls_session_client/.* if VSF_USE_MBEDTLS is enabled
 *
 * Compiler:
 *
 */

/*============================ INCLUDES ======================================*/

#include <vsf.h>
#include <vsf_board.h>

#include "shell/sys/espidf/vsf_espidf.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

#ifdef __CPU_WEBASSEMBLY__
int VSF_USER_ENTRY(void)
#else
// TODO: SDL require that main need argc and argv
int VSF_USER_ENTRY(int argc, char *argv[])
#endif
{
    vsf_board_init();
    vsf_start_trace();
    vsf_trace_info("start espidf ..." VSF_TRACE_CFG_LINEEND);

    vsf_espidf_init();
    return 0;
}

/* EOF */