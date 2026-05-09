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

/*
 * Single entry for the vc.espidf host build.
 *
 * This file is intentionally minimal. It owns only the VSF boot
 * sequence (linux init + espidf/freertos init) and delegates all
 * application-specific wiring to the vsf_app_* hooks declared in
 * vsf_app.h. The concrete application under test/ (or any future
 * peer directory) is selected at link time by compiling exactly one
 * translation unit that implements vsf_app_setup_espidf_cfg() and
 * vsf_app_main().
 */

/*============================ INCLUDES ======================================*/

#include <vsf.h>
#include <vsf_board.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include "shell/sys/espidf/vsf_espidf.h"
#include "shell/sys/freertos/vsf_freertos.h"

#if     VSF_USE_ESPIDF == ENABLED                                              \
    &&  VSF_ESPIDF_CFG_USE_NETIF == ENABLED
#   include "esp_netif.h"
#   if VSF_NETDRV_USE_WPCAP == ENABLED
#       define __VSF_NETDRV_CLASS_INHERIT_NETLINK__
#       include "component/tcpip/vsf_tcpip.h"
#       include "component/tcpip/netdrv/driver/wpcap/vsf_netdrv_wpcap.h"
#   endif

// lwIP's arch/cc.h sets LWIP_PROVIDE_ERRNO, which asks lwIP to supply its
// own 'int errno' symbol. api/sockets.c is excluded from this build (the
// host uses hostsock for AF_INET), so anchor storage here in the
// environment TU to satisfy the link.
int errno;
#endif

/*============================ IMPLEMENTATION ================================*/

VSF_CAL_WEAK(app_main)
void app_main(void)
{
}

VSF_CAL_WEAK(vsf_netif_demo_set_handle)
void vsf_netif_demo_set_handle(void *handle)
{
    (void)handle;
}

static void * __vsf_espidf_start_main(void *arg)
{
    app_main();
    return NULL;
}

// Overrides the weak vsf_linux_create_fhs() provided by the linux shim.
// Runs on the init-process thread before any other linux process is
// started, so it is safe to do file-system / driver bring-up here.
int vsf_linux_create_fhs(void)
{
    vsf_linux_vfs_init();

#if defined(APP_MSCBOOT_CFG_FLASH) && defined(APP_MSCBOOT_CFG_ROOT_SIZE) && (APP_MSCBOOT_CFG_ROOT_SIZE > 0) && defined(APP_MSCBOOT_CFG_ROOT_ADDR)
    vk_mal_init(&root_mal.use_as__vk_mal_t);

    static vk_lfs_info_t __root_fs;
    vsf_lfs_bind_mal(&__root_fs.config, &root_mal.use_as__vk_mal_t);

    mkdir("/root", 0);
    if (mount(NULL, "root", &vk_lfs_op, 0, (const void *)&__root_fs) != 0) {
        printf("Fail to mount /root.\n");
    }
    putenv("HOME=/root");
#elif defined(__WIN__)
#   define __PART_ROOT_SIZE        (64u * 1024u)
#   define __PART_ROOT_BLKSZ       256u
#   define __PART_ROOT_FILE        "/root/nvs_flash.bin"

    static vk_winfs_info_t __root_fs = {
        .root.name = "./root",
    };
    mkdir("/root", 0);
    if (mount(NULL, "root", &vk_winfs_op, 0, (const void *)&__root_fs) != 0) {
        printf("Failed to mount /root.\n");
    }
    putenv("HOME=/root");

    int root_fd = open(__PART_ROOT_FILE, O_RDWR | O_CREAT, 0666);
    if (root_fd < 0) {
        printf("Failed to open " __PART_ROOT_FILE ".\n");
        return -1;
    }

    struct stat st;
    if ((fstat(root_fd, &st) == 0) && (st.st_size < __PART_ROOT_SIZE)) {
        if (ftruncate(root_fd, __PART_ROOT_SIZE) != 0) {
            printf("Failed to grow " __PART_ROOT_FILE " to %u bytes\n", (unsigned)__PART_ROOT_SIZE);
            return -1;
        }
    }

    extern vk_file_t * __vsf_linux_get_fs_ex(vsf_linux_process_t *process, int fd);
    vk_file_t *vkfile = __vsf_linux_get_fs_ex(NULL, root_fd);
    if (vkfile == NULL) {
        printf("Failed to get fd->vk_file\n");
        return -1;
    }

    static vk_file_mal_t __root_mal = {
        .drv                        = &vk_file_mal_drv,
        .feature                    = VSF_MAL_READABLE | VSF_MAL_WRITABLE | VSF_MAL_ERASABLE,
        .block_size                 = __PART_ROOT_BLKSZ,
    };
    __root_mal.file = vkfile;
    vk_mal_init((vk_mal_t *)&__root_mal);

    static const vsf_espidf_partition_entry_t __root_entries[] = {
        {
            .label      = "nvs",
            .type       = ESP_PARTITION_TYPE_DATA,
            .subtype    = ESP_PARTITION_SUBTYPE_DATA_NVS,
            .offset     = 0x0000,
            .size       = 0x0400,
        },
        {
            .label      = "phy_init",
            .type       = ESP_PARTITION_TYPE_DATA,
            .subtype    = ESP_PARTITION_SUBTYPE_DATA_PHY,
            .offset     = 0x0400,
            .size       = 0x0400,
            .readonly   = 1,
        },
        {
            .label      = "storage",
            .type       = ESP_PARTITION_TYPE_DATA,
            .subtype    = ESP_PARTITION_SUBTYPE_DATA_UNDEFINED,
            .offset     = 0x0800,
            .size       = 0x0800,
        },
        {
            // Dedicated LittleFS partition. Sized at 32 KiB / 128 blocks on
            // the 256 B root erase unit -- comfortably above the lfs driver's
            // minimum footprint (two superblocks + root dir + a few data
            // blocks). Offset is 0x1000 so esp_partition_register_external()
            // still has room above the static table.
            .label      = "lfs",
            .type       = ESP_PARTITION_TYPE_DATA,
            .subtype    = ESP_PARTITION_SUBTYPE_DATA_UNDEFINED,
            .offset     = 0x1000,
            .size       = 0x8000,
        },
    };
    static const uint16_t __root_entries_count = dimof(__root_entries);

    static vk_netdrv_wpcap_t __env_wpcap_netdrv = { 0 };
    vk_netdrv_set_netlink_op((vk_netdrv_t *)&__env_wpcap_netdrv, &vk_netdrv_wpcap_netlink_op, NULL);
    vsf_netif_demo_set_handle(vsf_netdrv_new_netif_glue(&__env_wpcap_netdrv.use_as__vk_netdrv_t));
#else
#   warning where should be ${HOME}?
#endif

    busybox_install();

    vsf_trace_info("start espidf ..." VSF_TRACE_CFG_LINEEND);

    vsf_espidf_cfg_t cfg = {
        .partition.root_mal         = &__root_mal.use_as__vk_mal_t,
        .partition.entries          = __root_entries,
        .partition.entry_count      = __root_entries_count,
#if VSF_USE_USB_HOST == ENABLED
        .usb_host.usbh              = &vsf_board.usbh_dev,
#endif
    };

    vsf_freertos_init();
    vsf_espidf_init(&cfg);

    // espidf application entry is app_main
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 16384);
    pthread_create(&tid, &attr, __vsf_espidf_start_main, NULL);
    pthread_attr_destroy(&attr);
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
    vsf_trace_info("start linux ..." VSF_TRACE_CFG_LINEEND);

    vsf_linux_stdio_stream_t stream = {
        .in     = (vsf_stream_t *)&VSF_DEBUG_STREAM_RX,
        .out    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
        .err    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
    };
    vsf_linux_init(&stream);
    return 0;
}

/* EOF */
