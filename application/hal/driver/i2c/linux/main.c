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
 * Board:
 *   vsf_board.i2c
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

int i2c_main(int argc, char *argv[])
{
    int fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open i2c device\n");
        return -1;
    }

    // chip address
    if (ioctl(fd, I2C_SLAVE, 0x30) < 0) {
        fprintf(stderr, "fail to set i2c address\n");
        close(fd);
        return -1;
    }

    uint8_t buffer[16];
    buffer[0] = 0x84;       // register address
    if (write(fd, buffer, 1) != 1) {
        fprintf(stderr, "fail to write register address\n");
        close(fd);
        return -1;
    }

    if (read(fd, buffer, 16) != 16) {
        fprintf(stderr, "fail to read register data\n");
        close(fd);
        return -1;
    }

    vsf_trace_info("reg84:\n");
    vsf_trace_buffer(VSF_TRACE_INFO, buffer, 16);

    close(fd);
    return 0;
}

int vsf_linux_create_fhs(void)
{
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();
    busybox_install();

    // 1. driver
    vsf_linux_fs_bind_i2c("/dev/i2c-0", vsf_board.i2c);
    // 2. fs
    // 3. app
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/i2c_test", i2c_main);
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
