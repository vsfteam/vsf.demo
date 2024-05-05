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

#define __VSF_DISP_CLASS_INHERIT__
#include <vsf.h>
#include <vsf_board.h>

#include <sgl.h>

/*============================ MACROS ========================================*/

#if VSF_USE_TRACE != ENABLED
#   error this demo depends on VSF_USE_TRACE, please enable it!
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_disp_bind_sgl(vk_disp_t *disp);
extern void sgl_application_init(void);

/*============================ IMPLEMENTATION ================================*/

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    vsf_disp_bind_sgl(vsf_board.display_dev);
    sgl_application_init();

    while (1) {
        sgl_tick_inc(5);
        sgl_task_handler();

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED && VSF_OS_CFG_MAIN_MODE == VSF_OS_CFG_MAIN_MODE_THREAD
        vsf_thread_delay_ms(5);
#endif
    }
    return 0;
}
