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
 * IAR flash loader — generic algorithm.
 *
 * Implements the three entry points that IAR C-SPY calls to program a
 * target flash (FlashInit / FlashWrite / FlashErase). This file is
 * MCU-agnostic: the only target-specific piece — *which* VSF flash driver
 * instance to program — is injected via the FLASHLOADER_FLASH_INSTANCE
 * macro, which belongs in the board-level configuration
 * (board/<xxx>/vsf_board_cfg.h).
 *
 *   // In board/<xxx>/vsf_board_cfg.h:
 *   #define FLASHLOADER_FLASH_INSTANCE      vsf_hw_xip_flash0
 *
 * The flash itself is driven exclusively through the VSF generic flash
 * HAL (vsf_flash_init/enable/capability/erase_multi_sector/
 * write_multi_sector), so any VSF flash driver whose first class member
 * is a vsf_flash_t (i.e. VSF_FLASH_CFG_MULTI_CLASS=ENABLED, the framework
 * default) can be plugged in without source changes.
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
 * so the return value of erase/write already reflects completion. We do
 * not rely on vsf_flash_irq_clear() polling — some drivers (e.g. BH1098)
 * don't implement it.
 *
 * Clock / target bring-up: flashloader_init_clock() is a weak no-op by
 * default. The assumption is that the target chip's boot ROM / reset
 * state already brings the flash controller up to a usable frequency
 * (true for XIP-style MCUs, whose boot path must itself be able to
 * fetch code from flash). If your target needs an explicit PLL / wait-
 * state / power-domain bring-up before the flash driver can operate,
 * provide a strong override in the board support — e.g. a small .c
 * that calls the board's vsf_driver_init() or a narrower subset.
 */

#include "flash_loader.h"
#include "flash_loader_extra.h"

#include "hal/vsf_hal.h"

#if VSF_HAL_USE_FLASH != ENABLED
#   error "IAR flashloader requires VSF_HAL_USE_FLASH=ENABLED"
#endif

#ifndef FLASHLOADER_FLASH_INSTANCE
#   error "FLASHLOADER_FLASH_INSTANCE is not defined: set it in vsf_board_cfg.h to a vsf_flash driver instance name (e.g. vsf_hw_xip_flash0)."
#endif

// Under MULTI_CLASS=ENABLED the first member of the driver struct is a
// vsf_flash_t, so the instance's address is directly usable as vsf_flash_t*
// without knowing the concrete (driver-private) layout. MULTI_CLASS defaults
// to ENABLED in vsf_template_flash.h and propagates to every
// VSF_${IP}_FLASH_CFG_MULTI_CLASS fallback.
#if VSF_FLASH_CFG_MULTI_CLASS != ENABLED
#   error "IAR flashloader requires VSF_FLASH_CFG_MULTI_CLASS=ENABLED"
#endif

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static vsf_flash_capability_t    g_cap;

/*============================ IMPLEMENTATION ================================*/

// Board-overridable hook. Default: no-op. See the file banner for why
// this is usually sufficient (boot ROM has already configured the flash
// controller on XIP-capable MCUs). Provide a strong symbol in board
// support when explicit clock / wait-state setup is required.
VSF_CAL_WEAK(flashloader_init_clock)
void flashloader_init_clock(void)
{
}

static vsf_err_t flashloader_erase_range(vsf_flash_t      *flash,
                                         vsf_flash_size_t  off,
                                         vsf_flash_size_t  size)
{
    vsf_flash_size_t sec = g_cap.erase_sector_size;

    // XIP-style drivers don't expose an independent erase granule — their
    // write_multi_sector does read-modify-erase-write internally, so
    // there's nothing to do here. Reporting OK lets FlashInit(ERASE_ONLY)
    // and FlashErase simply fall through to the write path.
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
    flashloader_init_clock();

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
    vsf_flash_size_t unit = g_cap.write_sector_size;

    // Split the request into write_sector_size chunks so the driver never
    // sees a size it cannot handle (e.g. XIP requires 32B-aligned writes).
    for (vsf_flash_size_t pos = 0; pos < count; ) {
        vsf_flash_size_t len = count - pos;
        if ((unit != 0) && (len > unit)) {
            len = unit;
        }

        vsf_err_t err = vsf_flash_write_multi_sector(&FLASHLOADER_FLASH_INSTANCE,
            flash_off + pos, (uint8_t *)buffer + pos, len);
        if (err != VSF_ERR_NONE) {
            return RESULT_ERROR;
        }
        pos += len;
    }
    return RESULT_OK;
}

uint32_t FlashErase(void *block_start, uint32_t block_size)
{
    vsf_flash_size_t flash_off = (vsf_flash_size_t)block_start
                               - g_cap.base_address;
    vsf_err_t err = flashloader_erase_range((vsf_flash_t *)&FLASHLOADER_FLASH_INSTANCE,
                                            flash_off, block_size);
    return (err == VSF_ERR_NONE) ? RESULT_OK : RESULT_ERROR;
}

/* EOF */
