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
 * hal i2c demo
 * Based on linux sub-system
 *
 * Dependency:
 * Submodule:
 *   i2c-tools/raw
 *
 * Board:
 *   vsf_board.i2c
 *
 * Source Code:
 *   i2c-tools/raw/lib/smbus.c
 *   i2c-tools/raw/tools/i2cbusses.c
 *   i2c-tools/raw/tools/i2cdetect.c
 *   i2c-tools/raw/tools/i2cdump.c
 *   i2c-tools/raw/tools/i2cget.c
 *   i2c-tools/raw/tools/i2cset.c
 *   i2c-tools/raw/tools/i2ctransfer.c
 *   i2c-tools/raw/tools/util.c
 *
 * Pre-define:
 *   i2c-tools/raw/tools/i2cdetect.c:   main=i2cdetect_main
 *   i2c-tools/raw/tools/i2cdump.c:     main=i2cdump_main
 *   i2c-tools/raw/tools/i2cget.c:      main=i2cget_main
 *   i2c-tools/raw/tools/i2cset.c:      main=i2cset_main
 *   i2c-tools/raw/tools/i2ctransfer.c: main=i2ctransfer_main
 *
 * Include Directories necessary for linux:
 *   vsf/source/shell/sys/linux/include
 *   vsf/source/shell/sys/linux/include/simple_libc
 *   i2c-tools/raw/include
 */

/*============================ INCLUDES ======================================*/

#include <unistd.h>
#include <fcntl.h>
#include "vsf_board.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static int __vsf_linux_create_string_file(const char *filename, char *data)
{
    int fd = open(filename, O_CREAT);
    if (fd < 0) {
        return -1;
    }

    write(fd, data, strlen(data));
    close(fd);
    return 0;
}

int vsf_linux_create_fhs(void)
{
    // 0. devfs, procfs, busybox, etc
    vsf_linux_vfs_init();
    busybox_install();
    mkdir("/proc", 0);
    __vsf_linux_create_string_file("/proc/mounts", "sysfs /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0");

    // 1. driver
#if VSF_HAL_USE_GPIO == ENABLED && VSF_HW_GPIO_COUNT > 0
    vsf_linux_fs_bind_gpio_hw("/sys/class/gpio");
#endif
    mkdirs("/sys/class/i2c-dev/i2c-0", 0);
    __vsf_linux_create_string_file("/sys/class/i2c-dev/i2c-0/name", "vsf_i2c0");
    vsf_linux_fs_bind_i2c("/dev/i2c/0", vsf_board.i2c);
    // 2. fs
    // 3. app
    extern int i2cdetect_main(int argc, char *argv[]);
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/i2cdetect", i2cdetect_main);
    extern int i2cget_main(int argc, char *argv[]);
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/i2cget", i2cget_main);
    extern int i2cset_main(int argc, char *argv[]);
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/i2cset", i2cset_main);
    extern int i2ctransfer_main(int argc, char *argv[]);
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/i2ctransfer", i2ctransfer_main);
    extern int i2cdump_main(int argc, char *argv[]);
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/i2cdump", i2cdump_main);
    return 0;
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
#if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#endif

    vsf_linux_init(&(vsf_linux_stdio_stream_t){
        .in     = (vsf_stream_t *)&VSF_DEBUG_STREAM_RX,
        .out    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
        .err    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
    });

    return 0;
}
