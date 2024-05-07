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
 * sgl demo
 * based on raw VSF(without VSF.linux)
 *
 * Dependency:
 *
 * Submodule:
 *   vsf
 *     source/component/3rd-party/PLOOC/raw
 *   application/component/ui/sgl/raw
 *
 * Board:
 *   vsf_board.display_dev
 *
 * Include Directories necessary for sgl:
 *   application/component/ui/sgl/application
 *   application/component/ui/sgl/application/test
 *   application/component/ui/sgl/platform/vsf
 *   application/component/ui/sgl/raw/source
 *
 * Pre-defined:
 *
 * Sources necessary for sgl:
 *   application/component/ui/sgl/application/main.c
 *   application/component/ui/sgl/application/test/sgl_application_test.c
 *   application/component/ui/sgl/platform/vsf/sgl_platform_vsf.c
 *   application/component/ui/sgl/raw/source/*.c
 *
 * Linker:
 *
 * Compiler:
 *
 */

/*============================ INCLUDES ======================================*/

#define __VSF_DISP_CLASS_INHERIT__
#include <vsf.h>
#include <vsf_board.h>

#include <sgl.h>
#include <sgl_platform.h>

/*============================ MACROS ========================================*/

#if VSF_USE_TRACE != ENABLED
#   error this demo depends on VSF_USE_TRACE, please enable it!
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

VSF_CAL_WEAK(demo_setup)
void demo_setup(void)
{
    SGL_LOG_INFO("empty demo, please implement strong version of demo_setup");
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    if (sgl_platform_bind_vsf(vsf_board.display_dev, false) != VSF_ERR_NONE) {
        vsf_trace_error("fail to bind vsf to sgl" VSF_TRACE_CFG_LINEEND);
        return -1;
    }

    sgl_init();
    demo_setup();

    while (1) {
        sgl_tick_inc(5);
        sgl_task_handler();

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED && VSF_OS_CFG_MAIN_MODE == VSF_OS_CFG_MAIN_MODE_THREAD
        vsf_thread_delay_ms(5);
#endif
    }
    return 0;
}
