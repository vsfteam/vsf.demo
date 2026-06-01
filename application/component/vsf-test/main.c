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

#include "vsf.h"
#include "vsf_board.h"
#include "test/vsf_test_suite/vsf_test_suites.h"

/*============================ ENTRY =========================================*/

int VSF_USER_ENTRY(void)
{
#if VSF_USE_KERNEL == DISABLED
    vsf_arch_init();
    vsf_hal_init_early();
    vsf_service_init();
    vsf_hal_init();
    vsf_systimer_init();
    vsf_systimer_start();
#endif
    vsf_board_init();
    vsf_start_trace();

    vsf_test_reboot_t *__vsf_test_reboot_entries[] = {
        vsf_arch_reset,
    };

    vsf_test_t test = {
        .wdt = { .entries = NULL, .count = 0 },
        .reboot = { .entries = __vsf_test_reboot_entries,
                    .count   = dimof(__vsf_test_reboot_entries) },
        .suites      = vsf_test_suite_list,
        .suite_count = vsf_test_suite_count,
        .instances   = vsf_board_test_instances,
        .instance_count = vsf_board_test_instance_count,
    };
    vsf_test_run(&test);

    return 0;
}

/* EOF */

