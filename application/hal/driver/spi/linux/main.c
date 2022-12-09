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
 * hal spi demo
 * Based on linux sub-system
 *
 * Dependency:
 * Board:
 *   vsf_board.spi
 *
 * Include Directories necessary for linux:
 *   vsf/source/shell/sys/linux/include
 *   vsf/source/shell/sys/linux/include/simple_libc
 */

/*============================ INCLUDES ======================================*/

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
 
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

int spi_main(int argc, char *argv[])
{
    int fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open spi device\n");
        return -1;
    }
    
    uint8_t mode = SPI_MODE_3 | SPI_LSB_FIRST;
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        fprintf(stderr, "fail to set spi address\n");
        close(fd);
        return -1;
    }
    
    uint8_t bits = 8;
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        fprintf(stderr, "fail to set spi address\n");
        close(fd);
        return -1;
    }
    
    uint32_t speed = 1 * 1000 * 1000;
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        fprintf(stderr, "fail to set spi address\n");
        close(fd);
        return -1;
    }



    struct spi_ioc_transfer xfer[2];
    uint8_t txcmd = 0xAA;        
    uint8_t txbuffer[16];
    uint8_t rxbuffer[16];

    for (int i = 0; i < sizeof(txbuffer); i++) {
        txbuffer[i] = i;
    }
    
    xfer[0].tx_buf = (unsigned long)&txcmd;
    xfer[0].len = 1;
    xfer[1].tx_buf = (unsigned long)txbuffer;
    xfer[1].rx_buf = (unsigned long)rxbuffer;
    xfer[1].len = 16;
    
    if (ioctl(fd, SPI_IOC_MESSAGE(dimof(xfer)), xfer) < 0) {
        fprintf(stderr, "fail to set spi address\n");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int vsf_linux_create_fhs(void)
{
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();
    busybox_install();

    // 1. driver
    vsf_linux_fs_bind_spi("/dev/spidev0.0", vsf_board.spi);
    // 2. fs
    // 3. app
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/spi_test", spi_main);
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
