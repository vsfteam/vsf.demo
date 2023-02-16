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
 *   vsf_hw_gpioX
 *
 * Include Directories necessary for linux:
 *   vsf/source/shell/sys/linux/include
 *   vsf/source/shell/sys/linux/include/simple_libc
 */

/*============================ INCLUDES ======================================*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "vsf_board.h"

/*============================ MACROS ========================================*/

#if VSF_USE_TRACE != ENABLED
#   error this demo depends on VSF_USE_TRACE, please enable it!
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static int __gpio_write(char *path, bool is_write, char * str, size_t len)
{
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open file(%s)\n", path);
        return -1;
    }

    if (is_write) {
        if (write(fd, str, len) != len) {
            fprintf(stderr, "fail to write file(%s)\n", path);
            close(fd);
            return -1;
        }
    } else {
        if (read(fd, str, len) != len) {
            fprintf(stderr, "fail to read file(%s)\n", path);
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

int vsf_linux_create_fhs(void)
{
    char value;
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();
    busybox_install();

    // 1. driver
    vsf_linux_fs_bind_gpio_hw("/sys/class/gpio");

    __gpio_write("/sys/class/gpio/export",           true,  "14",   strlen("14"));
    __gpio_write("/sys/class/gpio/gpio14/direction", true,  "14",   strlen("1"));
    __gpio_write("/sys/class/gpio/gpio14/value",     true,  "1",    strlen("1"));
    __gpio_write("/sys/class/gpio/gpio14/value",     true,  "0",    strlen("0"));

    __gpio_write("/sys/class/gpio/export",           true,  "15",   strlen("15"));
    __gpio_write("/sys/class/gpio/gpio15/direction", true,  "15",   strlen("0"));
    __gpio_write("/sys/class/gpio/gpio15/value",     false, &value, 1);
    __gpio_write("/sys/class/gpio/gpio15/value",     false, &value, 1);

    return 0;
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    vsf_linux_init(&(vsf_linux_stdio_stream_t){
        .in     = (vsf_stream_t *)&VSF_DEBUG_STREAM_RX,
        .out    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
        .err    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
    });

    return 0;
}
