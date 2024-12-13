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
 * bootlaoder for aic8800m_uikit
 * If PB7 is low when startup, enter bootloader, or enter application.
 *
 * Dependency:
 * Board:
 *   VSF_USB_DC0
 *
 * Submodule(except PLOOC):
 *   CMSIS(source/utilities/compiler/arm/3rd-party/CMSIS)
 *   AIC8800M_SDK_vsf(source/hal/driver/AIC/AIC8800/vendor)
 *
 * Linker:
 *   flash starts at 0x8000000
 */

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "vsf_board.h"

// for SystemInit
#include "system.h"

/*============================ MACROS ========================================*/

#ifndef USRAPP_CFG_USBD_SPEED
#   define USRAPP_CFG_USBD_SPEED            USB_DC_SPEED_HIGH
#endif

// __APP_CFG_MSC_BULK_SIZE is for internal usage
#if USRAPP_CFG_USBD_SPEED == USB_DC_SPEED_HIGH
#   define __APP_CFG_MSC_BULK_SIZE          512
#else
#   define __APP_CFG_MSC_BULK_SIZE          64
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

dcl_vsf_peda_methods(static, __mscboot_on_firmware_read)
dcl_vsf_peda_methods(static, __mscboot_on_firmware_write)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static bool __flash_is_inited = false;
static vsf_eda_t *__flash_eda_pending;

static vk_fakefat32_file_t __fakefat32_root[2] = {
    {
        .name               = "mscboot",
        .attr               = (vk_file_attr_t)VSF_FAT_FILE_ATTR_VOLUMID,
    },
    {
        .name               = "firmware.bin",
        .size               = MSCBOOT_CFG_FW_SIZE,
        .attr               = VSF_FILE_ATTR_READ | VSF_FILE_ATTR_WRITE,
        .callback.fn_read   = (vsf_peda_evthandler_t)vsf_peda_func(__mscboot_on_firmware_read),
        .callback.fn_write  = (vsf_peda_evthandler_t)vsf_peda_func(__mscboot_on_firmware_write),
    }
};

static vk_fakefat32_mal_t __fakefat32mal = {
    .drv                    = &vk_fakefat32_mal_drv,
    .sector_size            = 512,
    .sector_number          = 0x80040,  // more than 65536 clusters, to match FAT32
    .sectors_per_cluster    = 8,        // 512 * 8 = 4K is erase sector size of AIC8800M
    .volume_id              = 0x12345678,
    .disk_id                = 0x9ABCEF01,
    .root                   = {
        .name               = "ROOT",
        .d.child            = (vk_memfs_file_t *)__fakefat32_root,
        .d.child_num        = dimof(__fakefat32_root),
    },
};

describe_mem_stream(__user_usbd_msc_stream, 1024)
static const vk_virtual_scsi_param_t __usrapp_scsi_param = {
    .block_size             = 512,
    .block_num              = 0x80040,  // more than 65536 clusters, to match FAT32
    .vendor                 = "VSF     ",
    .product                = "VSF MSCBOOT     ",
    .revision               = "1.00",
    .type                   = SCSI_PDT_DIRECT_ACCESS_BLOCK,
};
static vk_mal_scsi_t __usrapp_mal_scsi = {
    .drv                = &vk_virtual_scsi_drv,
    .param              = (void *)&__usrapp_scsi_param,
    .virtual_scsi_drv   = &vk_mal_virtual_scsi_drv,
    .mal                = &__fakefat32mal.use_as__vk_mal_t,
};

describe_usbd(__user_usbd_msc, APP_CFG_USBD_VID, APP_CFG_USBD_PID, USRAPP_CFG_USBD_SPEED)
    usbd_func(__user_usbd_msc,
        usbd_mscbot_func(__user_usbd_msc,
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
                        &__user_usbd_msc_stream.use_as__vsf_stream_t,
                        // scsi_dev
                        &__usrapp_mal_scsi.use_as__vk_scsi_t
        )
    )

    usbd_common_desc_iad(__user_usbd_msc,
                        // str_product, str_vendor, str_serial
                        u"VSF-MSCBOOT", u"VSF", u"1.0.0",
                        // ep0_size
                        64,
                        // total function descriptor size
                        USB_DESC_MSCBOT_IAD_LEN,
                        // total function interface number
                        USB_MSCBOT_IFS_NUM,
                        // attribute, max_power
                        USB_CONFIG_ATT_WAKEUP, 100,
        usbd_mscbot_desc_iad(__user_usbd_msc, 0)
    )

    usbd_std_desc_table(__user_usbd_msc,
        usbd_func_str_desc_table(__user_usbd_msc, 0)
    )
    usbd_ifs(__user_usbd_msc,
        usbd_mscbot_ifs(__user_usbd_msc, 0)
    )
end_describe_usbd(__user_usbd_msc, VSF_USB_DC0)

/*============================ IMPLEMENTATION ================================*/

static void __mscboot_flash_irqhandler(void *target, vsf_flash_t *flash, vsf_flash_irq_mask_t mask)
{
    vsf_eda_t *eda_pending = __flash_eda_pending;
    if (eda_pending != NULL) {
        __flash_eda_pending = NULL;
        vsf_eda_post_evt(eda_pending, VSF_EVT_RETURN);
    }
}

vsf_component_peda_ifs_entry(__mscboot_on_firmware_read, vk_memfs_callback_read)
{
    vsf_peda_begin();

    enum {
        STATE_FW_READ_INIT,
        STATE_FW_READ_READ,
    };

    switch (evt) {
    case VSF_EVT_INIT:
        vsf_local.rsize = 0;
        vsf_local.offset += MSCBOOT_CFG_FW_ADDR;
        vsf_eda_set_user_value(STATE_FW_READ_INIT);

        if (!__flash_is_inited) {
            __flash_is_inited = true;
            vsf_flash_init(&vsf_hw_flash0, &(vsf_flash_cfg_t){
                .isr            = {
                    .handler_fn = __mscboot_flash_irqhandler,
                    .prio       = vsf_arch_prio_0,
                },
            });
            while (vsf_flash_enable(&vsf_hw_flash0) != fsm_rt_cpl);
        }
        // fall through
    case VSF_EVT_RETURN:
        switch (vsf_eda_get_user_value()) {
        case STATE_FW_READ_INIT:
            break;
        case STATE_FW_READ_READ:
            vsf_eda_return(vsf_local.rsize);
            return;
        }

        __flash_eda_pending = vsf_eda_get_cur();
        vsf_eda_set_user_value(STATE_FW_READ_READ);
        vsf_flash_read(&vsf_hw_flash0, vsf_local.offset, vsf_local.buff, vsf_local.size);
        vsf_local.offset += vsf_local.size;
        vsf_local.buff += vsf_local.size;
        vsf_local.rsize += vsf_local.size;
        vsf_local.size -= vsf_local.size;
        break;
    }

    vsf_peda_end();
}

vsf_component_peda_ifs_entry(__mscboot_on_firmware_write, vk_memfs_callback_write)
{
    vsf_peda_begin();

    enum {
        STATE_FW_WRITE_INIT,
        STATE_FW_WRITE_ERASE,
        STATE_FW_WRITE_WRITE,
    };
    vsf_flash_capability_t cap = vsf_flash_capability(&vsf_hw_flash0);

    switch (evt) {
    case VSF_EVT_INIT:
        vsf_local.wsize = 0;
        vsf_local.offset += MSCBOOT_CFG_FW_ADDR;
        vsf_eda_set_user_value(STATE_FW_WRITE_INIT);

        if (!__flash_is_inited) {
            __flash_is_inited = true;
            vsf_flash_init(&vsf_hw_flash0, &(vsf_flash_cfg_t){
                .isr            = {
                    .handler_fn = __mscboot_flash_irqhandler,
                    .prio       = vsf_arch_prio_0,
                },
            });
            while (vsf_flash_enable(&vsf_hw_flash0) != fsm_rt_cpl);
        }
        // fall through
    case VSF_EVT_RETURN:
        switch (vsf_eda_get_user_value()) {
        case STATE_FW_WRITE_INIT:
            if (!(vsf_local.offset % cap.erase_sector_size)) {
                // erase on erase_sector_size aligned address
                __flash_eda_pending = vsf_eda_get_cur();
                vsf_eda_set_user_value(STATE_FW_WRITE_ERASE);
                vsf_flash_erase_one_sector(&vsf_hw_flash0, vsf_local.offset);
                return;
            }
            break;
        case STATE_FW_WRITE_ERASE:
            break;
        case STATE_FW_WRITE_WRITE:
            vsf_eda_return(vsf_local.wsize);
            return;
        }

        __flash_eda_pending = vsf_eda_get_cur();
        vsf_eda_set_user_value(STATE_FW_WRITE_WRITE);
        vsf_flash_write(&vsf_hw_flash0, vsf_local.offset, vsf_local.buff, vsf_local.size);
        vsf_local.offset += vsf_local.size;
        vsf_local.buff += vsf_local.size;
        vsf_local.wsize += vsf_local.size;
        vsf_local.size -= vsf_local.size;
        break;
    }

    vsf_peda_end();
}

bool vsf_driver_init(void)
{
    SystemInit();
    __vsf_hw_aic8800_gpio_init();
    return true;
}

int VSF_USER_ENTRY(void)
{
    if (!(vsf_gpio_read(&vsf_hw_gpio1) & (1 << 7))) {
        vk_usbd_init(&__user_usbd_msc);
        vk_usbd_connect(&__user_usbd_msc);
    } else {
        vsf_flash_capability_t cap = vsf_flash_capability(&vsf_hw_flash0);
        uint32_t *image = (uint32_t *)(cap.base_address + MSCBOOT_CFG_FW_ADDR);
        vsf_arch_set_stack(image[0]);
        ((void (*)(void))(image[1]))();
    }
    return 0;
}
