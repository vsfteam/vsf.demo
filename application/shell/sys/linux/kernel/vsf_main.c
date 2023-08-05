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
 * linux demo
 * Based on linux sub-system
 *
 * Dependency:
 *
 * Board:
 *
 * Include Directories necessary for linux:
 *   vsf/source/shell/sys/linux/include
 *   vsf/source/shell/sys/linux/include/simple_libc if VSF_LINUX_USE_SIMPLE_LIBC is enabled
 *
 * Sources necessary for linux:
 *   vsf/source/shell/sys/linux/lib/3rd-party/fnmatch
 *   vsf/source/shell/sys/linux/lib/3rd-party/glob
 *   vsf/source/shell/sys/linux/lib/3rd-party/regex
 *
 * Linker:
 *   If bootloader is used, set image base to the APP address
 *
 * Compiler:
 *   Avoid to use static-base reigster and veneer register, which will be used by applet.
 *   For ARM, should be R9 and R12
 *
 */

/*============================ INCLUDES ======================================*/

#include <unistd.h>
#include <sys/mount.h>
#include <vsf_board.h>

/*============================ MACROS ========================================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED

#if     defined(VSF_USBD_CFG_SPEED_HIGH)
#   define __APP_CFG_MSC_BULK_SIZE                  512
#   define __APP_CFG_HID_INT_SIZE                   1024
#elif   defined(VSF_USBD_CFG_SPEED_FULL)
#   define __APP_CFG_MSC_BULK_SIZE                  64
#   define __APP_CFG_HID_INT_SIZE                   512
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

#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED
dcl_vsf_peda_methods(static, __usr_mscboot_on_romfs_read)
dcl_vsf_peda_methods(static, __usr_mscboot_on_romfs_write)
#endif

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED

static vk_fakefat32_file_t __usr_fakefat32_root[2] = {
    {
        .name               = "mscboot",
        .attr               = (vk_file_attr_t)VSF_FAT_FILE_ATTR_VOLUMID,
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

static vsf_mutex_t __user_flash_mutex;
static vsf_eda_t *__usr_mscboot_eda = NULL;

#endif

/*============================ IMPLEMENTATION ================================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED

// msc update for romfs

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

static const uint8_t __hid_report_desc[] = {
    0x06, 0x00, 0xFF,   // Usage Page (Vendor)
    0x09, 0x01,         // Usage (0x01)
    0xA1, 0x01,         // Collection (Application)
    0x15, 0x00,         //   Logical Minimum (0)
    0x26, 0xFF, 0x00,   //   Logical Maximum (255)
    0x75, 0x08,         //   Report Size (8)
    0x96, (__APP_CFG_HID_INT_SIZE >> 0) & 0xFF, (__APP_CFG_HID_INT_SIZE >> 8) & 0xFF,
                        //   Report Count (__APP_CFG_HID_INT_SIZE)
    0x09, 0x01,         //   Usage (0x01)
    0x81, 0x02,         //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x96, (__APP_CFG_HID_INT_SIZE >> 0) & 0xFF, (__APP_CFG_HID_INT_SIZE >> 8) & 0xFF,
                        //   Report Count (__APP_CFG_HID_INT_SIZE)
    0x09, 0x01,         //   Usage (0x01)
    0x91, 0x02,         //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,               // End Collection
};
static uint8_t __hid_in_buffer[__APP_CFG_HID_INT_SIZE];
static vk_usbd_hid_report_t __hid_reports[] = {
    VSF_USBD_HID_REPORT(USB_HID_REPORT_INPUT, 1, __hid_in_buffer, sizeof(__hid_in_buffer), 0),
    VSF_USBD_HID_REPORT(USB_HID_REPORT_OUTPUT, 1, NULL, __APP_CFG_HID_INT_SIZE, 0),
};
static vsf_eda_t __hid_eda;

describe_usbd(__app_usbd, APP_CFG_USBD_VID, APP_CFG_USBD_PID, VSF_USBD_CFG_SPEED)
    usbd_common_desc_iad(__app_usbd,
                        // str_product, str_vendor, str_serial
                        u"VSFLinux.Romfs", u"VSF", u"1.0.0",
                        // ep0_size
                        64,
                        // total function descriptor size
                        USB_DESC_MSCBOT_IAD_LEN + USB_DESC_HID_IAD_LEN,
                        // total function interface number
                        USB_MSCBOT_IFS_NUM + USB_HID_IFS_NUM,
                        // attribute, max_power
                        USB_CONFIG_ATT_WAKEUP, 100
    )
        usbd_mscbot_desc_iad(__app_usbd,
                        // interface
                        0,
                        // function string index(start from 0)
                        0,
                        // bulk in ep, bulk out ep
                        1, 1,
                        // bulk ep size
                        __APP_CFG_MSC_BULK_SIZE
        )
        usbd_hid_desc_iad(__app_usbd,
                        // interface
                        1,
                        // function string index(start from 0)
                        1,
                        // hid subclass, protocol
                        0, 0,
                        // bcd_version, country_code
                        0x0111, 0,
                        // report_desc_len
                        sizeof(__hid_report_desc),
                        // interrupt ep in, size, interval
                        2, __APP_CFG_HID_INT_SIZE, 1,
                        // interrupt ep out, size, interval
                        2, __APP_CFG_HID_INT_SIZE, 1
        )
    usbd_func_desc(__app_usbd)
        usbd_func_str_desc(__app_usbd, 0, u"Romfs.MSC")
        usbd_func_str_desc(__app_usbd, 1, u"Romfs.HID")
    usbd_std_desc_table(__app_usbd)
        usbd_func_str_desc_table(__app_usbd, 0)
        usbd_func_str_desc_table(__app_usbd, 1)
    usbd_func(__app_usbd)
        usbd_mscbot_func(__app_usbd,
                        // function index
                        0,
                        // bulk in ep, bulk out ep
                        1, 1,
                        // max lun(logic unit number)
                        0,
                        // scsi_dev
                        &__app_mal_scsi.use_as__vk_scsi_t,
                        // stream
                        &__app_usbd_msc_stream.use_as__vsf_stream_t
        )
        usbd_hid_func(__app_usbd,
                        // function index
                        1,
                        // interrupt in ep, interrupt out ep, out ep size
                        2, 2, __APP_CFG_HID_INT_SIZE,
                        // report_num, reports, has_report_id
                        dimof(__hid_reports), __hid_reports, false,
                        // report_desc, report_desc_size
                        __hid_report_desc, sizeof(__hid_report_desc),
                        // notify_eda, notifier
                        true, &__hid_eda
        )
    usbd_ifs(__app_usbd)
        usbd_mscbot_ifs(__app_usbd, 0)
        usbd_hid_ifs(__app_usbd, 1)
end_describe_usbd(__app_usbd, VSF_USB_DC0)

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

    vsf_eda_mutex_init(&__user_flash_mutex);
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

vsf_component_peda_ifs_entry(__usr_mscboot_on_romfs_read, vk_memfs_callback_read)
{
    vsf_peda_begin();

    uint32_t cur_size;

    switch (evt) {
    case VSF_EVT_INIT:
        if (VSF_ERR_NONE != vsf_eda_mutex_enter(&__user_flash_mutex)) {
            break;
        }
        // fall through
    case VSF_EVT_SYNC:
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
        if (VSF_ERR_NONE != vsf_eda_mutex_enter(&__user_flash_mutex)) {
            break;
        }
        // fall through
    case VSF_EVT_SYNC:
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

#endif

// linux

WEAK(vsf_board_init_linux)
void vsf_board_init_linux(void)
{
}

int vsf_linux_create_fhs(void)
{
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();

    // 1. hardware driver

    // 2. fs
#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED
    bool install_embedded_busybox = false;
    static vk_romfs_info_t __usr_romfs_info = {
        .root.header = (vk_romfs_header_t *)(APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_ROMFS_ADDR),
    };
    mkdir("/usr", 0);
    if (mount(NULL, "usr", &vk_romfs_op, 0, (const void *)&__usr_romfs_info) != 0) {
        printf("Fail to mount /usr from romfs, install embedded busybox instead.\n");
        install_embedded_busybox = true;
    } else {
        if (access("/usr/bin/busybox", X_OK) != 0) {
            printf("Can not find valid busybox in /usr/bin/, install embedded busybox instead.\n");
            install_embedded_busybox = true;
        }
    }
    if (install_embedded_busybox) {
        busybox_install();
    } else {
        setenv("PATH", VSF_LINUX_CFG_PATH, true);

        mkdir("/bin", 0);
        if (symlink("/usr/bin/init", "/bin/init") < 0) {
            printf("busybox found in /usr/bin, but init not found\n");
        }
        if (symlink("/usr/bin/sh", "/bin/sh") < 0) {
            printf("busybox found in /usr/bin, but sh not found\n");
        }

        mkdir("/etc", 0);
        if (symlink("/usr/etc/inittab", "/etc/inittab") < 0) {
            printf("fail to symlink inittab to /etc\n");
        }
    }
#else
    busybox_install();
#endif

    // 3. install executables
    vsf_board_init_linux();

    return 0;
}

#ifdef __CPU_WEBASSEMBLY__
int VSF_USER_ENTRY(void)
#else
// TODO: SDL require that main need argc and argv
int VSF_USER_ENTRY(int argc, char *argv[])
#endif
{
    vsf_board_init();
    vsf_start_trace();

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED
    if (APP_BOOT1_KEY_IS_DOWN) {
        __usr_flash_init();

        vk_usbd_init(&__app_usbd);
        vk_usbd_connect(&__app_usbd);
    } else
#endif
    {
        vsf_trace_info("start linux..." VSF_TRACE_CFG_LINEEND);
        vsf_linux_stdio_stream_t stream = {
            .in     = (vsf_stream_t *)&VSF_DEBUG_STREAM_RX,
            .out    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
            .err    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
        };
        vsf_linux_init(&stream);
    }
    return 0;
}

/* EOF */