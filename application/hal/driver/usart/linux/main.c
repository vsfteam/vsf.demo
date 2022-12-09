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
 * hal usart demo
 * Based on linux sub-system
 *
 * Dependency:
 * Board:
 *   vsf_board.usart
 *
 * Include Directories necessary for linux:
 *   vsf/source/shell/sys/linux/include
 *   vsf/source/shell/sys/linux/include/simple_libc
 */

/*============================ INCLUDES ======================================*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

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

int usart_main(int argc, char *argv[])
{
    int result = 0;
    
    int fd = open("/dev/tty0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open usart device\n");
        result = -1;
        goto _usart_exit;
    }
    
    char *wstr = "hello, world\r\n";
    int wlen = strlen(wstr);
    if (write(fd, wstr, wlen) != wlen) {
        fprintf(stderr, "fail to write to usart\n");
        result = -1;
        goto _usart_close;
    }
    
    char rstr[20] = {0};
    int rlen = sizeof(rstr) - 1;
    rlen = read(fd, rstr, rlen);
    if (rlen) {
        vsf_trace_info("read data:\n");
        vsf_trace_buffer(VSF_TRACE_INFO, rstr, rlen);
        goto _usart_close;
    }

_usart_close:
    close(fd);
_usart_exit:
    return 0;
}

int vsf_linux_create_fhs(void)
{
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();
    busybox_install();

    // 1. driver
    vsf_linux_fs_bind_uart("/dev/tty0", vsf_board.usart);
    // 2. fs
    // 3. app
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/usart_test", usart_main);
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
