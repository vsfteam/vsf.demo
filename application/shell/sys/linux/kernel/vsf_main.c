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
 *   vsf/source/component/3rd-party/littlefs/port
 *   vsf/source/component/3rd-party/littlefs/raw
 *   optional:
 *    for package manager, need VSF_USE_LWIP from vsf_board
 *     vsf/source/component/3rd-party/mbedtls/raw/include
 *
 * Pre-defined:
 *   __unix__ for net_sockets/timing/entropy_poll in mbedtls
 *   LFS_CONFIG to lfs_util_vsf.h
 *
 * Sources necessary for linux:
 *   vsf/source/shell/sys/linux/lib/3rd-party/fnmatch
 *   vsf/source/shell/sys/linux/lib/3rd-party/glob
 *   vsf/source/shell/sys/linux/lib/3rd-party/regex
 *   optional:
 *    for package manager, need VSF_USE_LWIP from vsf_board
 *     ./vsf_linux_package_manager.c
 *     vsf/source/component/3rd-party/littlefs/port/*
 *     vsf/source/component/3rd-party/littlefs/raw/*
 *     vsf/source/component/3rd-party/mbedtls/raw/library/*
 *     vsf/source/component/3rd-party/mbedtls/port/*
 *     vsf/source/component/3rd-party/mbedtls/extension/tls_session_client/*
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

#if VSF_USE_MBEDTLS == ENABLED
#   include "component/3rd-party/mbedtls/extension/vplt/mbedtls_vplt.h"
#endif
#if     defined(APP_MSCBOOT_CFG_FLASH) && defined(APP_MSCBOOT_CFG_ROOT_SIZE)    \
    &&  defined(APP_MSCBOOT_CFG_ROOT_ADDR) && (VSF_FS_USE_LITTLEFS == ENABLED)
#   include "component/3rd-party/littlefs/port/lfs_port.h"
#endif

/*============================ MACROS ========================================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED

#if     defined(VSF_USBD_CFG_SPEED_HIGH)
#   define __APP_CFG_MSC_BULK_SIZE                  512
#elif   defined(VSF_USBD_CFG_SPEED_FULL)
#   define __APP_CFG_MSC_BULK_SIZE                  64
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

#define APP_CONFIG_FILE                             "/root/appcfg"

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED

#   if VSF_USE_USB_DEVICE == ENABLED
static vk_fakefat32_file_t __usr_fakefat32_root[2] = {
    {
        .name               = "mscboot",
        .attr               = (vk_file_attr_t)VSF_FAT_FILE_ATTR_VOLUMID,
    },
    {
        .name               = "usr.romfs",
        .size               = APP_MSCBOOT_CFG_ROMFS_SIZE,
        .attr               = VSF_FILE_ATTR_READ,
        .f.buff             = (uint8_t *)(APP_MSCBOOT_CFG_ROMFS_ADDR + APP_MSCBOOT_CFG_FLASH_ADDR),
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
#   endif

static bool __usr_linux_boot = false;
#endif

#if VSF_HAL_USE_FLASH == ENABLED && defined(APP_MSCBOOT_CFG_FLASH)
vk_hw_flash_mal_t flash_mal = {
    .drv                    = &vk_hw_flash_mal_drv,
    .flash                  = &APP_MSCBOOT_CFG_FLASH,
};
#endif

/*============================ IMPLEMENTATION ================================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED

// msc update for romfs

describe_mem_stream(__app_usbd_msc_stream, 1024)
static const vk_virtual_scsi_param_t __app_mscbot_scsi_param = {
    .block_size             = APP_CFG_FAKEFAT32_SECTOR_SIZE,
    .block_num              = APP_CFG_FAKEFAT32_SIZE / APP_CFG_FAKEFAT32_SECTOR_SIZE,
    .vendor                 = "VSFTeam ",
    .product                = "VSF.Romfs MSCBOT",
    .revision               = "1.00",
    .type                   = SCSI_PDT_DIRECT_ACCESS_BLOCK,
};
static vk_mal_scsi_t __app_mscbot_mal_scsi = {
    .drv                = &vk_virtual_scsi_drv,
    .param              = (void *)&__app_mscbot_scsi_param,
    .virtual_scsi_drv   = &vk_mal_virtual_scsi_drv,
    .mal                = &__app_fakefat32_mal.use_as__vk_mal_t,
};

describe_usbd(__app_usbd, APP_CFG_USBD_VID, APP_CFG_USBD_PID, VSF_USBD_CFG_SPEED)
    usbd_common_desc_iad(__app_usbd,
                        // str_product, str_vendor, str_serial
                        u"VSFLinux.Romfs", u"VSF", u"1.0.0",
                        // ep0_size
                        64,
                        // total function descriptor size
                        USB_DESC_MSCBOT_IAD_LEN,
                        // total function interface number
                        USB_MSCBOT_IFS_NUM,
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
    usbd_func_desc(__app_usbd)
        usbd_func_str_desc(__app_usbd, 0, u"Romfs.MSC")
    usbd_std_desc_table(__app_usbd)
        usbd_func_str_desc_table(__app_usbd, 0)
    usbd_func(__app_usbd)
        usbd_mscbot_func(__app_usbd,
                        // function index
                        0,
                        // bulk in ep, bulk out ep
                        1, 1,
                        // max lun(logic unit number)
                        0,
                        // scsi_dev
                        &__app_mscbot_mal_scsi.use_as__vk_scsi_t,
                        // stream
                        &__app_usbd_msc_stream.use_as__vsf_stream_t
        )
    usbd_ifs(__app_usbd)
        usbd_mscbot_ifs(__app_usbd, 0)
end_describe_usbd(__app_usbd, VSF_USB_DC0)

#endif

// linux

WEAK(vsf_board_init_linux)
void vsf_board_init_linux(void)
{
}

WEAK(vsf_linux_install_package_manager)
void vsf_linux_install_package_manager(void)
{
}

#if defined(APP_MSCBOOT_CFG_FLASH) && defined(APP_MSCBOOT_CFG_ROOT_SIZE) && defined(APP_MSCBOOT_CFG_ROOT_ADDR)
static char * __app_config_read(FILE *f)
{
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size > 0) {
        char *buffer = malloc(size);
        if (size != fread(buffer, 1, size, f)) {
            free(buffer);
            return NULL;
        }
        return buffer;
    }
    return NULL;
}

int app_config_read(const char *cfgname, char *cfgvalue, size_t valuelen)
{
    FILE *f = fopen(APP_CONFIG_FILE, "r");
    if (NULL == f) {
        return -1;
    }

    int namelen = strlen(cfgname);
    char *token = malloc(namelen + 3), *data, *start, *value, *end;
    if (NULL == token) {
        goto close_and_fail;
    }
    sprintf(token, "\n%s:", cfgname);

    data = __app_config_read(f);
    if (NULL == data) {
        goto free_token_and_fail;
    }

    start = strstr(data, token);
    if (NULL == start) {
        goto not_found;
    }

    value = start + strlen(token);
    end = strchr(value, '\n');
    if (end != NULL) {
        *end = '\0';
    }
    strncpy(cfgvalue, value, valuelen);
    return 0;

not_found:
    free(data);
free_token_and_fail:
    free(token);
close_and_fail:
    fclose(f);
    return -1;
}

int app_config_write(const char *cfgname, char *cfgvalue)
{
    FILE *f = fopen(APP_CONFIG_FILE, "r+");
    if (NULL == f) {
        f = fopen(APP_CONFIG_FILE, "w+");
        if (NULL == f) {
            return -1;
        }
    }

    int result = -1;
    int namelen = strlen(cfgname);
    char *token = malloc(namelen + 3), *data, *start, *value, *end;
    if (NULL == token) {
        goto close_and_fail;
    }
    sprintf(token, "\n%s:", cfgname);

    data = __app_config_read(f);
    if (NULL == data) {
        goto write_cfg;
    }

    start = strstr(data, token);
    if (NULL == start) {
        goto write_cfg;
    }

    value = start + strlen(token);
    if ((value == strstr(value, cfgvalue)) && (value[strlen(cfgvalue)] == '\n')) {
        result = 0;
        goto done;
    }

    fclose(f);
    truncate(APP_CONFIG_FILE, 0);
    f = fopen(APP_CONFIG_FILE, "r");

    end = strchr(value, '\n');
    if (end != NULL) {
        strcpy(start, end);
    }
    fwrite(data, 1, strlen(data), f);

write_cfg:
    fprintf(f, "%s%s:%s\n", NULL == data ? "\n" : "", cfgname, cfgvalue);
    result = 0;

done:
    if (data != NULL) {
        free(data);
    }
    free(token);
close_and_fail:
    fclose(f);
    return result;
}
#endif

#if VSF_USE_USB_HOST == ENABLED
static int __usbh_main(int argc, char *argv[])
{
    static bool __usbh_inited = false;
    if (!__usbh_inited) {
        __usbh_inited = true;
        vk_usbh_init(&vsf_board.usbh_dev);

#   if VSF_USBH_USE_LIBUSB == ENABLED
        static vk_usbh_class_t __usbh_libusb = { .drv = &vk_usbh_libusb_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_libusb);
#   endif
#   if VSF_USBH_USE_HUB == ENABLED
        static vk_usbh_class_t __usbh_hub = { .drv = &vk_usbh_hub_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_hub);
#   endif
#   if VSF_USBH_USE_ECM == ENABLED
        static vk_usbh_class_t __usbh_ecm = { .drv = &vk_usbh_ecm_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_ecm);
#   endif
#   if VSF_USBH_USE_HID == ENABLED
        static vk_usbh_class_t __usbh_hid = { .drv = &vk_usbh_hid_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_hid);
#   endif
#   if VSF_USBH_USE_DS4 == ENABLED
        static vk_usbh_class_t __usbh_ds4 = { .drv = &vk_usbh_ds4_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_ds4);
#   endif
#   if VSF_USBH_USE_DS5 == ENABLED
//        static vk_usbh_class_t __usbh_ds5 = { .drv = &vk_usbh_ds5_drv };
//        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_ds5);
#   endif
#   if VSF_USBH_USE_NSPRO == ENABLED
        static vk_usbh_class_t __usbh_nspro = { .drv = &vk_usbh_nspro_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_nspro);
#   endif
#   if VSF_USBH_USE_XB360 == ENABLED
        static vk_usbh_class_t __usbh_xb360 = { .drv = &vk_usbh_xb360_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_xb360);
#   endif
#   if VSF_USBH_USE_XB1 == ENABLED
        static vk_usbh_class_t __usbh_xb1 = { .drv = &vk_usbh_xb1_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_xb1);
#   endif
#   if VSF_USBH_USE_MSC == ENABLED
        static vk_usbh_class_t __usbh_msc = { .drv = &vk_usbh_msc_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_msc);
#   endif
    }
    return 0;
}
#endif

int vsf_linux_create_fhs(void)
{
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();

    // 1. hardware driver
#if VSF_HAL_USE_FLASH == ENABLED && defined(APP_MSCBOOT_CFG_FLASH)
    vsf_hw_flash_init(flash_mal.flash, NULL);
    vsf_flash_enable(flash_mal.flash);
    vk_mal_init(&flash_mal.use_as__vk_mal_t);
#endif

    // 2. fs
#if defined(APP_MSCBOOT_CFG_FLASH) && defined(APP_MSCBOOT_CFG_ROOT_SIZE) && defined(APP_MSCBOOT_CFG_ROOT_ADDR)
    static vk_mim_mal_t __root_mal = {
        .drv            = &vk_mim_mal_drv,
        .host_mal       = &flash_mal.use_as__vk_mal_t,
        .size           = APP_MSCBOOT_CFG_ROOT_SIZE,
        .offset         = APP_MSCBOOT_CFG_ROOT_ADDR,
    };
    vk_mal_init(&__root_mal.use_as__vk_mal_t);

    vsf_flash_capability_t cap = vsf_hw_flash_capability(&APP_MSCBOOT_CFG_FLASH);
    static vk_lfs_info_t __root_fs = {
        .config         = {
            .context    = &__root_mal.use_as__vk_mal_t,
            .read       = vsf_lfs_mal_read,
            .prog       = vsf_lfs_mal_prog,
            .erase      = vsf_lfs_mal_erase,
            .sync       = vsf_lfs_mal_sync,
            .lookahead_size = 8,
            .block_cycles   = 500,
        },
    };
    __root_fs.config.read_size = cap.write_sector_size,
    __root_fs.config.prog_size = cap.write_sector_size,
    __root_fs.config.block_size = cap.erase_sector_size,
    __root_fs.config.block_count = APP_MSCBOOT_CFG_ROOT_SIZE / cap.erase_sector_size,
    __root_fs.config.cache_size = cap.erase_sector_size,

    mkdir("/root", 0);
    if (mount(NULL, "root", &vk_lfs_op, 0, (const void *)&__root_fs) != 0) {
        printf("Fail to mount /root.\n");
    }
#endif

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED
    bool install_embedded_busybox = __usr_linux_boot;
    if (!__usr_linux_boot) {
        static vk_romfs_info_t __usr_romfs_info = {
            .image      = (vk_romfs_header_t *)(APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_ROMFS_ADDR),
            .image_size = APP_MSCBOOT_CFG_ROMFS_SIZE,
            .is_chained = true,
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
    }

    if (install_embedded_busybox) {
        vsf_linux_install_package_manager();
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

    // 3. install executables and built-in libraries
#if VSF_USE_MBEDTLS == ENABLED
    vsf_vplt_load_dyn((vsf_vplt_info_t *)&vsf_mbedtls_vplt.info);
#endif
#if VSF_USE_USB_HOST == ENABLED
    if (!__usr_linux_boot) {
        vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/usbhost", __usbh_main);
    }
#endif
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
        vk_usbd_init(&__app_usbd);
        vk_usbd_connect(&__app_usbd);

        __usr_linux_boot = true;
    }
#endif

    vsf_trace_info("start linux %s..." VSF_TRACE_CFG_LINEEND, __usr_linux_boot ? "boot" : "");
    vsf_linux_stdio_stream_t stream = {
        .in     = (vsf_stream_t *)&VSF_DEBUG_STREAM_RX,
        .out    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
        .err    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
    };
    vsf_linux_init(&stream);
    return 0;
}

/* EOF */