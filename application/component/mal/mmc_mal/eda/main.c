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

#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

#if VSF_USE_TRACE != ENABLED
#   error this demo depends on VSF_USE_TRACE, please enable it!
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static vk_mmc_mal_t __mmc_mal = {
    .drv                    = &vk_mmc_mal_drv,
    .hw_priority            = vsf_arch_prio_0,
    .working_clock_hz       = 50UL * 1000 * 1000,
};
static vsf_eda_t __eda_task;
static uint8_t __buffer[512];

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static void __eda_task_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    enum {
        STATE_MAL_INIT = 0,
        STATE_MAL_READ,
    };

    switch (evt) {
    case VSF_EVT_INIT:
        vsf_trace_info("initialize mmc_mal\n");
        vk_mal_init(&__mmc_mal.use_as__vk_mal_t);
        vsf_eda_set_user_value(STATE_MAL_INIT);
        break;
    case VSF_EVT_RETURN:
        if ((int)vsf_eda_get_return_value() < 0) {
            vsf_trace_error("error...\n");
            return;
        }

        switch (vsf_eda_get_user_value()) {
        case STATE_MAL_INIT:
            vsf_trace_info("read mmc_mal\n");
            vk_mal_read(&__mmc_mal.use_as__vk_mal_t, 0, 512, __buffer);
            vsf_eda_set_user_value(STATE_MAL_READ);
            break;
        case STATE_MAL_READ:
            vsf_trace_buffer(VSF_TRACE_INFO, __buffer, sizeof(__buffer));
            break;
        }
        break;
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    __mmc_mal.mmc = vsf_board.mmc;
    __mmc_mal.voltage = vsf_board.mmc_voltage;
    __mmc_mal.bus_width = vsf_board.mmc_bus_width;

    vsf_eda_start(&__eda_task, &(vsf_eda_cfg_t){
        .fn.evthandler      = __eda_task_evthandler,
        .priority           = vsf_prio_0,
    });
    return 0;
}
