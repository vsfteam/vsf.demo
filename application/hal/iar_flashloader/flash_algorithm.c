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
 * IAR flash loader -- generic algorithm.
 *
 * Implements the three entry points that IAR C-SPY calls to program a
 * target flash (FlashInit / FlashWrite / FlashErase). This file is
 * MCU-agnostic: the only target-specific piece -- *which* VSF flash driver
 * instance to program -- is injected via the FLASHLOADER_FLASH_INSTANCE
 * macro, which belongs in the board-level configuration
 * (board/<xxx>/vsf_board_cfg.h).
 *
 *   // In board/<xxx>/vsf_board_cfg.h:
 *   #define FLASHLOADER_FLASH_INSTANCE      vsf_hw_xip_flash0
 *
 * The flash is driven exclusively through the VSF generic flash HAL
 * (vsf_flash_init / enable / capability / erase_multi_sector /
 * write_multi_sector / read_multi_sector), so any VSF flash driver
 * whose first class member is a vsf_flash_t (i.e.
 * VSF_FLASH_CFG_MULTI_CLASS=ENABLED, the framework default) can be
 * plugged in without source changes.
 *
 * Two driver flavors are supported transparently:
 *   - Conventional flash (capability.erase_sector_size > 0): FlashErase
 *     performs an erase pass; FlashWrite then programs the erased region.
 *   - XIP-style flash (capability.erase_sector_size == 0, e.g.
 *     vsf_hw_xip_flash0 on BH1098): the driver performs read-modify-
 *     erase-write inside write_multi_sector, so FlashErase is treated as
 *     a no-op and all work happens inside FlashWrite.
 *
 * Completion semantics: VSF flash drivers used by flashloaders run
 * synchronously (the flashloader executes from RAM with interrupts off),
 * so the return value of erase/write already reflects completion.
 *
 * Bring-up sequence in FlashInit():
 *   1. vsf_arch_init()  -- arch/NVIC baseline.
 *   2. vsf_hal_init()   -- vsf_driver_init + vsf_app_driver_init (DPLL,
 *                          flash QSPI mode switch, flash clock).
 *   3. vsf_flash_init() on the target driver instance.
 *
 * The VSF kernel scheduler is NOT started: the flashloader is a C-SPY
 * RPC, and FlashInit/FlashWrite/FlashErase are invoked synchronously by
 * the debugger.
 */

#include "flash_loader.h"
#include "flash_loader_extra.h"

#include "vsf.h"

#include <string.h>

#if VSF_HAL_USE_FLASH != ENABLED
#   error "IAR flashloader requires VSF_HAL_USE_FLASH=ENABLED"
#endif

#ifndef FLASHLOADER_FLASH_INSTANCE
#   error "FLASHLOADER_FLASH_INSTANCE is not defined: set it in vsf_board_cfg.h to a vsf_flash driver instance name (e.g. vsf_hw_xip_flash0)."
#endif

// Under MULTI_CLASS=ENABLED the first member of the driver struct is a
// vsf_flash_t, so the instance's address is directly usable as vsf_flash_t*
// without knowing the concrete (driver-private) layout.
#if VSF_FLASH_CFG_MULTI_CLASS != ENABLED
#   error "IAR flashloader requires VSF_FLASH_CFG_MULTI_CLASS=ENABLED"
#endif

/*============================ LOCAL VARIABLES ===============================*/

static vsf_flash_capability_t    g_cap;

// Read-back verify of freshly written/erased data is NOT performed here:
// the IAR C-SPY J-Link driver runs its own verify pass (reads the XIP
// window and compares against the .out sections) after download.

/*============================ IMPLEMENTATION ================================*/

static vsf_err_t flashloader_erase_range(vsf_flash_t      *flash,
                                         vsf_flash_size_t  off,
                                         vsf_flash_size_t  size)
{
    vsf_flash_size_t sec = g_cap.erase_sector_size;

    // XIP-style drivers don't expose an independent erase granule -- their
    // write_multi_sector does read-modify-erase-write internally, so there
    // is nothing to do here.
    if (sec == 0) {
        return VSF_ERR_NONE;
    }

    for (vsf_flash_size_t pos = 0; pos < size; pos += sec) {
        vsf_flash_size_t chunk = size - pos;
        if (chunk > sec) chunk = sec;
        vsf_err_t err = vsf_flash_erase_multi_sector(flash, off + pos, chunk);
        if (err != VSF_ERR_NONE) {
            return err;
        }
    }
    return VSF_ERR_NONE;
}

#if USE_ARGC_ARGV
uint32_t FlashInit(void *base_of_flash, uint32_t image_size,
                   uint32_t link_address, uint32_t flags,
                   int argc, char const *argv[])
#else
uint32_t FlashInit(void *base_of_flash, uint32_t image_size,
                   uint32_t link_address, uint32_t flags)
#endif
{
    // arch: NVIC/systimer baseline. The flashloader is entered through
    // FlashInitEntry (flash_loader_asm.s), which bypasses
    // __iar_program_start; only __iar_init_vfp + __iar_data_init3 are
    // chained from the asm stub, so vsf_arch_init has to be called
    // here explicitly.
    vsf_arch_init();

    // hal: vsf_driver_init (I/D cache + vendor low-level init) and
    // vsf_app_driver_init (DPLL, flash QSPI-mode switch, flash clock).
    vsf_hal_init();

    vsf_flash_t *flash = (vsf_flash_t *)&FLASHLOADER_FLASH_INSTANCE;
    vsf_flash_cfg_t cfg = { .isr = { .handler_fn = NULL } };
    if (vsf_flash_init(flash, &cfg) != VSF_ERR_NONE) {
        return RESULT_ERROR;
    }
    vsf_flash_enable(flash);

    g_cap = vsf_flash_capability(flash);

    if (flags & FLAG_ERASE_ONLY) {
        vsf_flash_size_t off = (vsf_flash_size_t)base_of_flash
                             - g_cap.base_address;
        vsf_err_t err = flashloader_erase_range(flash, off, image_size);
        return (err == VSF_ERR_NONE) ? RESULT_ERASE_DONE : RESULT_ERROR;
    }

    return RESULT_OK;
}

uint32_t FlashWrite(void        *block_start,
                    uint32_t     offset_into_block,
                    uint32_t     count,
                    char const  *buffer)
{
    vsf_flash_size_t flash_off = (vsf_flash_size_t)block_start
                               - g_cap.base_address + offset_into_block;

    // C-SPY hands us a buffer whose offset and size are already aligned
    // to the <page> declared in the .flash file (matched to the driver's
    // write granularity), so the VSF flash HAL contract is satisfied for
    // a single multi-sector call -- no per-chunk slicing is needed here.
    vsf_err_t err = vsf_flash_write_multi_sector(
        (vsf_flash_t *)&FLASHLOADER_FLASH_INSTANCE,
        flash_off, (uint8_t *)buffer, count);
    return (err == VSF_ERR_NONE) ? RESULT_OK : RESULT_ERROR;
}

uint32_t FlashErase(void *block_start, uint32_t block_size)
{
    vsf_flash_size_t flash_off = (vsf_flash_size_t)block_start
                               - g_cap.base_address;

    vsf_err_t err = flashloader_erase_range((vsf_flash_t *)&FLASHLOADER_FLASH_INSTANCE,
                                            flash_off, block_size);
    if (err != VSF_ERR_NONE) {
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

/* EOF */
