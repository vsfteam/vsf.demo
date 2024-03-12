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
 * usbd_msc demo
 *
 * Dependency:
 * Board:
 *   VSF_USB_DC0
 *   vsf_board.audio_dev
 *
 * For AIC8800M:
 *   exclude hal/driver/AIC/AIC8800/vendor/audio
 *   exclude hal/driver/AIC/AIC8800/vendor/lwip
 *   exclude hal/driver/AIC/AIC8800/vendor/wifi
 *
 * Linker:
 *   Set image base to the start address of flash, so the mscboot will be executed on power up
 *
 * Optimization:
 *   define empty VSF_ASSERT(...)
 *   disable VSF_USE_TRACE
 *
 */

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

#if     VSF_USBD_CFG_SPEED == USB_SPEED_HIGH
#   define __APP_CFG_MSC_BULK_SIZE                  64
#elif   VSF_USBD_CFG_SPEED == USB_SPEED_FULL
#   define __APP_CFG_MSC_BULK_SIZE                  512
#else
#   error TODO: add support to current USB speed
#endif

#ifndef APP_CFG_FAKEFAT32_SECTOR_SIZE
#   define APP_CFG_FAKEFAT32_SECTOR_SIZE            512
#endif
#ifndef APP_CFG_FAKEFAT32_SIZE
//  0x1000 + reserved sector size(64)
#   define APP_CFG_FAKEFAT32_SIZE                   (APP_CFG_FAKEFAT32_SECTOR_SIZE * (0x10000 + 64))
#endif
#ifndef APP_CFG_FAKEFAT32_SECTORS_PER_CLUSTER
#   define APP_CFG_FAKEFAT32_SECTORS_PER_CLUSTER    1
#endif

// Check FAT32 limits, refer to: https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#Size_limits
#if (((APP_CFG_FAKEFAT32_SIZE / APP_CFG_FAKEFAT32_SECTOR_SIZE) - 64) / APP_CFG_FAKEFAT32_SECTORS_PER_CLUSTER) < 65525
#   warning invalid FAT32 minimum size limits, increase APP_CFG_FAKEFAT32_SIZE
#endif

#if VSF_USBD_CFG_AUTOSETUP != ENABLED
#   error VSF_USBD_CFG_AUTOSETUP is needed for this demo
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

dcl_vsf_peda_methods(static, __usr_mscboot_on_firmware_read)
dcl_vsf_peda_methods(static, __usr_mscboot_on_firmware_write)

dcl_vsf_peda_methods(static, __usr_mscboot_on_romfs_read)
dcl_vsf_peda_methods(static, __usr_mscboot_on_romfs_write)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static vk_fakefat32_file_t __usr_fakefat32_root[3] = {
    {
        .name               = "mscboot",
        .attr               = (vk_file_attr_t)VSF_FAT_FILE_ATTR_VOLUMID,
    },
    {
        .name               = "firmware.bin",
        .size               = APP_MSCBOOT_CFG_FW_SIZE,
        .attr               = VSF_FILE_ATTR_READ | VSF_FILE_ATTR_WRITE,
        .callback.fn_read   = (vsf_peda_evthandler_t)vsf_peda_func(__usr_mscboot_on_firmware_read),
        .callback.fn_write  = (vsf_peda_evthandler_t)vsf_peda_func(__usr_mscboot_on_firmware_write),
    },
    {
        .name               = "usr.romfs",
        .size               = APP_MSCBOOT_CFG_ROMFS_SIZE,
        .attr               = VSF_FILE_ATTR_READ | VSF_FILE_ATTR_WRITE,
        .callback.fn_read   = (vsf_peda_evthandler_t)vsf_peda_func(__usr_mscboot_on_romfs_read),
        .callback.fn_write  = (vsf_peda_evthandler_t)vsf_peda_func(__usr_mscboot_on_romfs_write),
    }
};

static vk_fakefat32_mal_t __app_fakefat32_mal = {
    .drv                    = &vk_fakefat32_mal_drv,
    .sector_size            = APP_CFG_FAKEFAT32_SECTOR_SIZE,
    .sector_number          = APP_CFG_FAKEFAT32_SIZE / APP_CFG_FAKEFAT32_SECTOR_SIZE,
    .sectors_per_cluster    = APP_CFG_FAKEFAT32_SECTORS_PER_CLUSTER,
    .volume_id              = 0x12345678,
    .disk_id                = 0x9ABCEF01,
    .root                   = {
        .name               = "ROOT",
        .d.child            = (vk_memfs_file_t *)__usr_fakefat32_root,
        .d.child_num        = dimof(__usr_fakefat32_root),
    },
};

static bool __usr_flash_is_inited = false;
static vsf_eda_t *__usr_mscboot_eda = NULL;

/*============================ IMPLEMENTATION ================================*/

describe_mem_stream(__app_usbd_msc_stream, 1024)
static const vk_virtual_scsi_param_t __app_scsi_param = {
    .block_size             = APP_CFG_FAKEFAT32_SECTOR_SIZE,
    .block_num              = APP_CFG_FAKEFAT32_SIZE / APP_CFG_FAKEFAT32_SECTOR_SIZE,
    .vendor                 = "Simon   ",
    .product                = "VSFDriver       ",
    .revision               = "1.00",
    .type                   = SCSI_PDT_DIRECT_ACCESS_BLOCK,
};
static vk_mal_scsi_t __app_mal_scsi = {
    .drv                = &vk_virtual_scsi_drv,
    .param              = (void *)&__app_scsi_param,
    .virtual_scsi_drv   = &vk_mal_virtual_scsi_drv,
    .mal                = &__app_fakefat32_mal.use_as__vk_mal_t,
};

describe_usbd(__app_usbd_msc, APP_CFG_USBD_VID, APP_CFG_USBD_PID, VSF_USBD_CFG_SPEED)
    usbd_func(__app_usbd_msc,
        usbd_mscbot_func(__app_usbd_msc,
                        // function index
                        0,
                        // function string
                        u"VSF-MSC0",
                        // function string index(start from 0)
                        0,
                        // interface
                        0,
                        // bulk in ep, bulk out ep
                        1, 1,
                        // bulk ep size
                        __APP_CFG_MSC_BULK_SIZE,
                        // stream
                        &__app_usbd_msc_stream.use_as__vsf_stream_t,
                        // scsi_dev(s)
                        &__app_mal_scsi.use_as__vk_scsi_t
        )
    )

    usbd_common_desc_iad(__app_usbd_msc,
                        // str_product, str_vendor, str_serial
                        u"VSF-USBD-Simplest", u"SimonQian", u"1.0.0",
                        // ep0_size
                        64,
                        // total function descriptor size
                        USB_DESC_MSCBOT_IAD_LEN,
                        // total function interface number
                        USB_MSCBOT_IFS_NUM,
                        // attribute, max_power
                        USB_CONFIG_ATT_WAKEUP, 100,
        usbd_mscbot_desc_iad(__app_usbd_msc, 0)
    )

    usbd_std_desc_table(__app_usbd_msc,
        usbd_func_str_desc_table(__app_usbd_msc, 0)
    )
    usbd_ifs(__app_usbd_msc,
        usbd_mscbot_ifs(__app_usbd_msc, 0)
    )
end_describe_usbd(__app_usbd_msc, VSF_USB_DC0)

static void __usr_flash_isrhandler( void *target_ptr,
                                    vsf_flash_t *flash_ptr,
                                    vsf_flash_irq_mask_t mask)
{
    VSF_ASSERT(__usr_mscboot_eda != NULL);
    vsf_eda_post_evt(__usr_mscboot_eda, VSF_EVT_USER);
}

static void __usr_flash_init(void)
{
    vsf_flash_cfg_t cfg     = {
        .isr                = {
            .handler_fn     = __usr_flash_isrhandler,
        },
    };
    vsf_flash_init(&APP_MSCBOOT_CFG_FLASH, &cfg);
    vsf_flash_enable(&APP_MSCBOOT_CFG_FLASH);
}

static uint32_t __usr_flash_read(uint64_t offset, uint32_t size, uint8_t *buff)
{
    uint32_t cur_size;

#if APP_MSCBOT_CFG_READ_ALIGN != 0
    VSF_ASSERT(!(offset & (APP_MSCBOT_CFG_READ_ALIGN - 1)));
#endif

    cur_size = 0 == APP_MSCBOOT_CFG_READ_BLOCK_SIZE ? size : APP_MSCBOOT_CFG_READ_BLOCK_SIZE;
    VSF_ASSERT(cur_size <= size);
    vsf_flash_read(&APP_MSCBOOT_CFG_FLASH, offset, buff, cur_size);
    return cur_size;
}

// return 1 to indicate erasing
static uint32_t __usr_flash_write(uint64_t offset, uint32_t size, uint8_t *buff)
{
    uint32_t cur_size;

    if (size & 1) {
        // erase done
        size &= ~1;
    } else {
        // write done, need erase next block?
        VSF_ASSERT(APP_MSCBOOT_CFG_ERASE_ALIGN != 0);
        if (!(offset & (APP_MSCBOOT_CFG_ERASE_ALIGN - 1))) {
            cur_size = 0 == APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE ? size : APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE;
            vsf_flash_erase(&APP_MSCBOOT_CFG_FLASH, offset, cur_size);
            return 1;
        }
    }

#if APP_MSCBOT_CFG_WRITE_ALIGN != 0
    VSF_ASSERT(!(offset & (APP_MSCBOT_CFG_WRITE_ALIGN - 1)));
#endif

    cur_size = 0 == APP_MSCBOOT_CFG_WRITE_BLOCK_SIZE ? size : APP_MSCBOOT_CFG_WRITE_BLOCK_SIZE;
    VSF_ASSERT(cur_size <= size);
    vsf_flash_write(&APP_MSCBOOT_CFG_FLASH, offset, buff, cur_size);
    return cur_size;
}

vsf_component_peda_ifs_entry(__usr_mscboot_on_firmware_read, vk_memfs_callback_read)
{
    vsf_peda_begin();

    uint32_t cur_size;

    switch (evt) {
    case VSF_EVT_INIT:
        if (!__usr_flash_is_inited) {
            __usr_flash_is_inited = true;
            __usr_flash_init();
        }

        vsf_local.offset += APP_MSCBOOT_CFG_FW_ADDR;
        vsf_local.rsize = 0;
        __usr_mscboot_eda = vsf_eda_get_cur();
        VSF_ASSERT(__usr_mscboot_eda != NULL);
        // fall through
    case VSF_EVT_USER:
        if (!vsf_local.size) {
            __usr_mscboot_eda = NULL;
            vsf_eda_return(vsf_local.rsize);
            break;
        }

        cur_size = 0 == APP_MSCBOOT_CFG_READ_BLOCK_SIZE ? vsf_local.size : APP_MSCBOOT_CFG_READ_BLOCK_SIZE;
        VSF_ASSERT(cur_size <= vsf_local.size);
        cur_size = __usr_flash_read(vsf_local.offset, cur_size, vsf_local.buff);
        vsf_local.offset += cur_size;
        vsf_local.buff += cur_size;
        vsf_local.rsize += cur_size;
        vsf_local.size -= cur_size;
        break;
    }

    vsf_peda_end();
}

vsf_component_peda_ifs_entry(__usr_mscboot_on_firmware_write, vk_memfs_callback_write)
{
    vsf_peda_begin();

    uint32_t cur_size;

    switch (evt) {
    case VSF_EVT_INIT:
        if (!__usr_flash_is_inited) {
            __usr_flash_is_inited = true;
            __usr_flash_init();
        }

        vsf_local.offset += APP_MSCBOOT_CFG_FW_ADDR;
        vsf_local.wsize = 0;
        __usr_mscboot_eda = vsf_eda_get_cur();
        VSF_ASSERT(__usr_mscboot_eda != NULL);
        // fall through
    case VSF_EVT_USER:
        if (!vsf_local.size) {
            __usr_mscboot_eda = NULL;
            vsf_eda_return(vsf_local.wsize);
            break;
        }

        cur_size = 0 == APP_MSCBOOT_CFG_WRITE_BLOCK_SIZE ? vsf_local.size : APP_MSCBOOT_CFG_WRITE_BLOCK_SIZE;
        VSF_ASSERT(cur_size <= vsf_local.size);
        cur_size = __usr_flash_write(vsf_local.offset, cur_size, vsf_local.buff);
        if (1 == cur_size) {
            vsf_local.size |= 1;
            break;
        } else {
            vsf_local.size &= ~1;
            vsf_local.offset += cur_size;
            vsf_local.buff += cur_size;
            vsf_local.wsize += cur_size;
            vsf_local.size -= cur_size;
        }
        break;
    }

    vsf_peda_end();
}

vsf_component_peda_ifs_entry(__usr_mscboot_on_romfs_read, vk_memfs_callback_read)
{
    vsf_peda_begin();

    uint32_t cur_size;

    switch (evt) {
    case VSF_EVT_INIT:
        if (!__usr_flash_is_inited) {
            __usr_flash_is_inited = true;
            __usr_flash_init();
        }

        vsf_local.offset += APP_MSCBOOT_CFG_ROMFS_ADDR;
        vsf_local.rsize = 0;
        __usr_mscboot_eda = vsf_eda_get_cur();
        VSF_ASSERT(__usr_mscboot_eda != NULL);
        // fall through
    case VSF_EVT_USER:
        if (!vsf_local.size) {
            __usr_mscboot_eda = NULL;
            vsf_eda_return(vsf_local.rsize);
            break;
        }

        cur_size = 0 == APP_MSCBOOT_CFG_READ_BLOCK_SIZE ? vsf_local.size : APP_MSCBOOT_CFG_READ_BLOCK_SIZE;
        VSF_ASSERT(cur_size <= vsf_local.size);
        cur_size = __usr_flash_read(vsf_local.offset, cur_size, vsf_local.buff);
        vsf_local.offset += cur_size;
        vsf_local.buff += cur_size;
        vsf_local.rsize += cur_size;
        vsf_local.size -= cur_size;
        break;
    }

    vsf_peda_end();
}

vsf_component_peda_ifs_entry(__usr_mscboot_on_romfs_write, vk_memfs_callback_write)
{
    vsf_peda_begin();

    uint32_t cur_size;

    switch (evt) {
    case VSF_EVT_INIT:
        if (!__usr_flash_is_inited) {
            __usr_flash_is_inited = true;
            __usr_flash_init();
        }

        vsf_local.offset += APP_MSCBOOT_CFG_ROMFS_ADDR;
        vsf_local.wsize = 0;
        __usr_mscboot_eda = vsf_eda_get_cur();
        VSF_ASSERT(__usr_mscboot_eda != NULL);
        // fall through
    case VSF_EVT_USER:
        if (!vsf_local.size) {
            __usr_mscboot_eda = NULL;
            vsf_eda_return(vsf_local.wsize);
            break;
        }

        cur_size = 0 == APP_MSCBOOT_CFG_WRITE_BLOCK_SIZE ? vsf_local.size : APP_MSCBOOT_CFG_WRITE_BLOCK_SIZE;
        VSF_ASSERT(cur_size <= vsf_local.size);
        cur_size = __usr_flash_write(vsf_local.offset, cur_size, vsf_local.buff);
        if (1 == cur_size) {
            vsf_local.size |= 1;
            break;
        } else {
            vsf_local.size &= ~1;
            vsf_local.offset += cur_size;
            vsf_local.buff += cur_size;
            vsf_local.wsize += cur_size;
            vsf_local.size -= cur_size;
        }
        break;
    }

    vsf_peda_end();
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
#if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#endif

    app_mscboot_init();
    if (app_mscboot_check()) {
        vk_usbd_init(&__app_usbd_msc);
        vk_usbd_connect(&__app_usbd_msc);
    } else {
        app_mscboot_fini();
        app_mscboot_boot();
    }
    return 0;
}
