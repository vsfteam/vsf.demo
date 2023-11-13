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
 * hal mmc probe demo
 *
 * Dependency:
 * Board:
 *   vsf_board.mmc
 *
 */

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

#if VSF_USE_TRACE != ENABLED
#   error this demo depends on VSF_USE_TRACE, please enable it!
#endif

#if VSF_HAL_USE_MMC != ENABLED
#   error MMC is not supported on current environment
#endif

#if VSF_KERNEL_CFG_EDA_SUPPORT_TIMER != ENABLED || VSF_KERNEL_CFG_SUPPORT_CALLBACK_TIMER != ENABLED
#   error timer and callback_timer is used in this demo
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static void __mmc_on_timer(vsf_callback_timer_t *timer);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static vsf_mmc_probe_t __mmc_probe = {
    .working_clock_hz       = 50 * 1000 * 1000,
    .uhs_en                 = false,
};
static vsf_callback_timer_t __mmc_timer = {
    .on_timer               = __mmc_on_timer,
};
static bool __mmc_is_probed = false;

/*============================ IMPLEMENTATION ================================*/

static void __mmc_probe_irqhandler(vsf_mmc_t *mmc_ptr, vsf_mmc_probe_t *probe, vsf_mmc_irq_mask_t irq_mask, vsf_mmc_transact_status_t status, uint32_t resp[4])
{
    vsf_err_t err = vsf_mmc_probe_irqhandler(mmc_ptr, probe, irq_mask, status, resp);
    switch (err) {
    case VSF_ERR_NONE:
        vsf_trace_debug("mmc_probe done\n");
        vsf_trace_debug("mmc.high_capacity : %d\n", probe->high_capacity);
        vsf_trace_debug("mmc.version : %08X\n", probe->version);
        vsf_trace_debug("mmc.capacity : %lld\n", probe->capacity);
        __mmc_is_probed = true;
        break;
    case VSF_ERR_NOT_READY:
        if (probe->delay_ms > 0) {
            vsf_callback_timer_add_ms(&__mmc_timer, probe->delay_ms);
        }
        break;
    default:
        vsf_trace_debug("mmc_probe failed\n");
        break;
    }
}

static void __mmc_on_timer(vsf_callback_timer_t *timer)
{
    __mmc_probe_irqhandler(vsf_board.mmc, &__mmc_probe, 0, 0, NULL);
}

static void __mmc_irqhandler(void *target_ptr, vsf_mmc_t *mmc_ptr, vsf_mmc_irq_mask_t irq_mask, vsf_mmc_transact_status_t status, uint32_t resp[4])
{
    if (!__mmc_is_probed) {
        __mmc_probe_irqhandler(mmc_ptr, (vsf_mmc_probe_t *)target_ptr, irq_mask, status, resp);
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    vsf_mmc_t *mmc = (vsf_mmc_t *)&vsf_hw_mmc0;
    vsf_mmc_init(mmc, &(vsf_mmc_cfg_t){
        .mode               = MMC_MODE_HOST,
        .isr                = {
            .handler_fn     = __mmc_irqhandler,
            .target_ptr     = &__mmc_probe,
            .prio           = vsf_arch_prio_0,
        },
    });

    __mmc_probe.voltage     = vsf_board.mmc_voltage;
    __mmc_probe.bus_width   = vsf_board.mmc_bus_width;
    __mmc_is_probed         = false;
    vsf_mmc_probe_start(mmc, &__mmc_probe);

    return 0;
}
