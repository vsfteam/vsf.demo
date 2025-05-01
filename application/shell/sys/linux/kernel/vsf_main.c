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
 * Submodule:
 *   vsf
 *     source/component/3rd-party/btstack/raw if VSF_USE_BTSTACK is enabled
 *     source/component/3rd-party/mbedtls/raw if VSF_USE_MBEDTLS is enabled
 *     source/component/3rd-party/PLOOC/raw
 *     source/component/3rd-party/qrcode/raw if VSF_USE_UI is enabled
 *   optional:
 *    for root directory in embedded hardware, littlefs is used
 *     source/component/3rd-party/littlefs/raw
 *    for AIC8800M(not for AIC8800M40B/AIC8800M80)
 *     source/hal/driver/AIC/AIC8800/vendor
 *
 * Board:
 *
 * Include Directories necessary for linux:
 *   vsf/source/shell/sys/linux/include
 *   vsf/source/shell/sys/linux/include/simple_libc if VSF_LINUX_USE_SIMPLE_LIBC is enabled
 *   vsf/source/shell/sys/linux/include/libusb if VSF_USBH_USE_LIBUSB is enabled
 *   optional:
 *    for root directory in embedded hardware, littlefs is used
 *     vsf/source/component/3rd-party/littlefs/port
 *     vsf/source/component/3rd-party/littlefs/raw
 *    for package manager, need VSF_USE_TCPIP from vsf_board
 *     vsf/source/component/3rd-party/mbedtls/raw/include if VSF_USE_MBEDTLS is enabled
 *
 * Pre-defined:
 *   __unix__ for net_sockets/timing/entropy_poll in mbedtls if VSF_USE_MBEDTLS is enabled
 *   LFS_CONFIG to lfs_util_vsf.h
 *
 * Sources necessary for linux:
 *   vsf/source/shell/sys/linux/lib/3rd-party/fnmatch
 *   vsf/source/shell/sys/linux/lib/3rd-party/glob
 *   vsf/source/shell/sys/linux/lib/3rd-party/regex excluding engine.c
 *   ./*.c
 *   optional:
 *    for root directory in embedded hardware, littlefs is used
 *     vsf/source/component/3rd-party/littlefs/port/*
 *     vsf/source/component/3rd-party/littlefs/raw/*
 *    for package manager, need VSF_USE_TCPIP from vsf_board
 *     ./vsf_linux_package_manager.c
 *     vsf/source/component/3rd-party/mbedtls/raw/library/* if VSF_USE_MBEDTLS is enabled
 *     vsf/source/component/3rd-party/mbedtls/port/* if VSF_USE_MBEDTLS is enabled
 *     vsf/source/component/3rd-party/mbedtls/extension/tls_session_client/* if VSF_USE_MBEDTLS is enabled
 *
 * Linker:
 *   If bootloader is used, set image base to the APP address
 *
 * Compiler:
 *   Avoid to use static-base reigster, which will be used by applet.
 *   For ARM, should be R9
 *
 */

/*============================ INCLUDES ======================================*/

// for disp->ui_on_ready
#define __VSF_DISP_CLASS_INHERIT__
// for vsf_linux_fs_bind_xxx
#define __VSF_LINUX_FS_CLASS_INHERIT__
#if VSF_LINUX_USE_X11 == ENABLED
// for pls
#   define __VSF_LINUX_CLASS_INHERIT__
#endif

#include <vsf.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#if VSF_USE_UI == ENABLED
#   include <sys/ioctl.h>
#   include <linux/fb.h>
#endif
#if VSF_USE_USB_HOST == ENABLED && VSF_USBH_USE_LIBUSB == ENABLED && VSF_LINUX_USE_LIBUSB == ENABLED
#   include <libusb.h>
#endif

#include <vsf_board.h>
#if VSF_BOARD_USE_EXT_GAMEPAD == ENABLED
#include <ext/gamepad/vsf_board_ext_gamepad.h>
#endif

#if VSF_USE_APPLET == ENABLED || VSF_LINUX_USE_APPLET == ENABLED
#   if VSF_USE_MBEDTLS == ENABLED
#       include "component/3rd-party/mbedtls/extension/vplt/mbedtls_vplt.h"
#   endif
#   if VSF_USE_FREETYPE == ENABLED
#       include "component/3rd-party/freetype/extension/vplt/freetype_vplt.h"
#   endif
#endif

#if VSF_USE_BTSTACK == ENABLED
#   include "btstack.h"
#   if VSF_USE_USB_HOST == ENABLED && VSF_USBH_USE_BTHCI == ENABLED
#       include "csr/btstack_chipset_csr.h"
#       include "bcm/btstack_chipset_bcm.h"
#   endif
#   include "component/3rd-party/btstack/port/btstack_run_loop_vsf.h"
#endif

#if     defined(APP_MSCBOOT_CFG_FLASH) && defined(APP_MSCBOOT_CFG_ROOT_SIZE)    \
    && (APP_MSCBOOT_CFG_ROOT_SIZE > 0) &&  defined(APP_MSCBOOT_CFG_ROOT_ADDR) && (VSF_FS_USE_LITTLEFS == ENABLED)
#   include "component/3rd-party/littlefs/port/lfs_port.h"
#endif

#if VSF_LINUX_USE_SDL2 == ENABLED
#   include "SDL_config_vsf.h"
#endif

/*============================ MACROS ========================================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED

#if     defined(VSF_USBD_CFG_SPEED_HIGH)
#   define __APP_CFG_MSC_BULK_SIZE                  512
#elif   defined(VSF_USBD_CFG_SPEED_FULL)
#   define __APP_CFG_MSC_BULK_SIZE                  64
#endif

#ifndef APP_CFG_FAKEFAT32_SECTOR_SIZE
#   define APP_CFG_FAKEFAT32_SECTOR_SIZE            512ULL
#endif
#ifndef APP_CFG_FAKEFAT32_SIZE
//  0x1000 + reserved sector size(64)
// for iOS/MacOS support, need 0x10000 + 0x457, no idea why
#   define APP_CFG_FAKEFAT32_SIZE                   (APP_CFG_FAKEFAT32_SECTOR_SIZE * (0x10000 + 0x457))
#endif
#ifndef APP_CFG_FAKEFAT32_SECTORS_PER_CLUSTER
#   define APP_CFG_FAKEFAT32_SECTORS_PER_CLUSTER    1
#endif

// Check FAT32 limits, refer to: https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#Size_limits
#if (((APP_CFG_FAKEFAT32_SIZE / APP_CFG_FAKEFAT32_SECTOR_SIZE) - 64) / APP_CFG_FAKEFAT32_SECTORS_PER_CLUSTER) < 65525
#   warning invalid FAT32 minimum size limits, increase APP_CFG_FAKEFAT32_SIZE
#endif
// max sector_size is 4K, max volume size is 8T
#if     (APP_CFG_FAKEFAT32_SECTOR_SIZE > 4096)
    ||  (APP_CFG_FAKEFAT32_SIZE > 0x80000000000ULL)
    ||  ((APP_CFG_FAKEFAT32_SIZE / APP_CFG_FAKEFAT32_SECTOR_SIZE) >= 0x100000000ULL)
#   error not supported
#endif

#if VSF_USBD_CFG_AUTOSETUP != ENABLED
#   error VSF_USBD_CFG_AUTOSETUP is needed for this demo
#endif

#endif

#define APP_CONFIG_FILE                             "~/appcfg"

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
vk_flash_mal_t flash_mal = {
    .drv                    = &vk_flash_mal_drv,
    .flash                  = (vsf_flash_t *)&APP_MSCBOOT_CFG_FLASH,
};
#   if defined(APP_MSCBOOT_CFG_ROOT_SIZE) && (APP_MSCBOOT_CFG_ROOT_SIZE > 0) && defined(APP_MSCBOOT_CFG_ROOT_ADDR)
vk_mim_mal_t root_mal = {
    .drv                    = &vk_mim_mal_drv,
    .host_mal               = &flash_mal.use_as__vk_mal_t,
    .size                   = APP_MSCBOOT_CFG_ROOT_SIZE,
    .offset                 = APP_MSCBOOT_CFG_ROOT_ADDR,
};
#   endif
#   if defined(APP_MSCBOOT_CFG_ROMFS_SIZE) && defined(APP_MSCBOOT_CFG_ROMFS_ADDR)
static vk_mim_mal_t __romfs_mal = {
    .drv                    = &vk_mim_mal_drv,
    .host_mal               = &flash_mal.use_as__vk_mal_t,
    .size                   = APP_MSCBOOT_CFG_ROMFS_SIZE,
    .offset                 = APP_MSCBOOT_CFG_ROMFS_ADDR,
};
vk_cached_mal_t romfs_mal = {
    .drv                    = &vk_cached_mal_drv,
    .host_mal               = &__romfs_mal.use_as__vk_mal_t,
};
#   endif
#endif

#if VSF_HAL_USE_SDIO == ENABLED
static vk_sdmmc_mal_t __sdmmc_mal = {
    .drv                    = &vk_sdmmc_mal_drv,
    .hw_priority            = vsf_arch_prio_0,
    .working_clock_hz       = 50 * 1000 * 1000,
    .uhs_en                 = false,
};
enum __sdmmc_state_t {
    SDMMC_STATE_FAIL = 0,
    SDMMC_STATE_WAIT_DET,
    SDMMC_STATE_WAIT_DET_STABLE,
    SDMMC_STATE_MAL,
    SDMMC_STATE_FS_OPEN,
    SDMMC_STATE_FS_MOUNT,
    SDMMC_STATE_DONE,
} static __sdmmc_state = SDMMC_STATE_WAIT_DET;
static vsf_teda_t __sdmmc_task;
static vk_malfs_mounter_t *__sdmmc_mounter;
static vsf_mutex_t *__sdmmc_fs_mutex;
#endif

/*============================ IMPLEMENTATION ================================*/

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED && VSF_USE_USB_DEVICE == ENABLED

// msc update for romfs

describe_mem_stream(__app_usbd_msc_stream, 2 * APP_CFG_FAKEFAT32_SECTOR_SIZE)
static vk_virtual_scsi_param_t __app_mscbot_romfs_scsi_param = {
    .block_size         = APP_CFG_FAKEFAT32_SECTOR_SIZE,
    .block_num          = APP_CFG_FAKEFAT32_SIZE / APP_CFG_FAKEFAT32_SECTOR_SIZE,
    .vendor             = "VSFTeam ",
    .product            = "VSF.Romfs MSCBOT",
    .revision           = "1.00",
    .serial             = "12345678",
    .type               = SCSI_PDT_DIRECT_ACCESS_BLOCK,
};
static vk_mal_scsi_t __app_mscbot_romfs_mal_scsi = {
    .drv                = &vk_virtual_scsi_drv,
    .param              = (void *)&__app_mscbot_romfs_scsi_param,
    .virtual_scsi_drv   = &vk_mal_virtual_scsi_drv,
    .mal                = &__app_fakefat32_mal.use_as__vk_mal_t,
};

#   if VSF_HAL_USE_SDIO == ENABLED
static vk_virtual_scsi_param_t __app_mscbot_tf_scsi_param = {
    .block_size         = 0,
    .block_num          = 0,
    .vendor             = "VSFTeam ",
    .product            = "VSF.TF MSCBOT   ",
    .revision           = "1.00",
    .serial             = "87654321",
    .type               = SCSI_PDT_DIRECT_ACCESS_BLOCK,
};
static vk_mal_scsi_t __app_mscbot_tf_mal_scsi = {
    .drv                = &vk_virtual_scsi_drv,
    .param              = (void *)&__app_mscbot_tf_scsi_param,
    .virtual_scsi_drv   = &vk_mal_virtual_scsi_drv,
    .mal                = &__sdmmc_mal.use_as__vk_mal_t,
};
#   endif

describe_usbd(__app_usbd, APP_CFG_USBD_VID, APP_CFG_USBD_PID, VSF_USBD_CFG_SPEED)
    usbd_func(__app_usbd,
        usbd_mscbot_func(__app_usbd,
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
                        &__app_mscbot_romfs_mal_scsi.use_as__vk_scsi_t
#   if VSF_HAL_USE_SDIO == ENABLED
                        ,&__app_mscbot_tf_mal_scsi.use_as__vk_scsi_t
#   endif
        )
#if VSF_USE_TCPIP == ENABLED && VSF_USE_LWIP == ENABLED && VSF_USBD_USE_CDCNCM == ENABLED
        usbd_cdcncm_func(__app_usbd,
                        // function index
                        1,
                        // function string
                        u"VSF-CDCNCM0",
                        // interface_start
                        1 * USB_MSC_IFS_NUM,
                        // function string index(start from 0)
                        1,
                        // interrupt in ep, bulk in ep, bulk out ep
                        2, 3, 3,
                        // bulk ep size
                        512,
                        // interrupt ep interval
                        16,
                        // i_mac
                        8,
                        // str_mac
                        u"AEDF09386412"
        )
#endif
    )

    usbd_common_desc_iad(__app_usbd,
                        // str_product, str_vendor, str_serial
                        u"VSF-USBD-Simplest", u"SimonQian", u"1.0.0",
                        // ep0_size
                        64,
#if VSF_USE_TCPIP == ENABLED && VSF_USE_LWIP == ENABLED && VSF_USBD_USE_CDCNCM == ENABLED
                        // total function descriptor size
                        USB_DESC_MSCBOT_IAD_LEN + USB_DESC_CDC_NCM_IAD_LEN,
                        // total function interface number
                        USB_MSCBOT_IFS_NUM + USB_CDC_NCM_IFS_NUM,
#else
                        // total function descriptor size
                        USB_DESC_MSCBOT_IAD_LEN,
                        // total function interface number
                        USB_MSCBOT_IFS_NUM,
#endif
                        // attribute, max_power
                        USB_CONFIG_ATT_WAKEUP, 100,
        usbd_mscbot_desc_iad(__app_usbd, 0)
#if VSF_USE_TCPIP == ENABLED && VSF_USE_LWIP == ENABLED && VSF_USBD_USE_CDCNCM == ENABLED
        usbd_cdcncm_desc_iad(__app_usbd, 1)
#endif
    )

    usbd_std_desc_table(__app_usbd,
        usbd_mscbot_desc_table(__app_usbd, 0)
#if VSF_USE_TCPIP == ENABLED && VSF_USE_LWIP == ENABLED && VSF_USBD_USE_CDCNCM == ENABLED
        usbd_cdcncm_desc_table(__app_usbd, 1)
#endif
    )
    usbd_ifs(__app_usbd,
        usbd_mscbot_ifs(__app_usbd, 0)
#if VSF_USE_TCPIP == ENABLED && VSF_USE_LWIP == ENABLED && VSF_USBD_USE_CDCNCM == ENABLED
        usbd_cdcncm_ifs(__app_usbd, 1)
#endif
    )
end_describe_usbd(__app_usbd, VSF_USB_DC0)

#endif

// netdrv pnp

#if VSF_USE_TCPIP == ENABLED && VSF_USE_LWIP == ENABLED

#   include "lwip/opt.h"
#   include "lwip/dhcp.h"
#   include "lwip/netif.h"
#   include "lwip/tcpip.h"

#   include "component/3rd-party/lwip/port/lwip_netdrv_adapter.h"

typedef struct lwip_netdrv_ctx_t {
    struct netif netif;
    struct dhcp netif_dhcp;
} lwip_netdrv_ctx_t;

static lwip_netdrv_ctx_t *__first_netif = NULL;

VSF_CAL_WEAK(vsf_board_get_netif)
struct netif * vsf_board_get_netif(void)
{
    if (__first_netif != NULL) {
        return &__first_netif->netif;
    }
    return NULL;
}

void vsf_pnp_on_netdrv_prepare(vk_netdrv_t *netdrv)
{
    lwip_netdrv_ctx_t *netdrv_ctx = vsf_heap_malloc(sizeof(lwip_netdrv_ctx_t));
    if (netdrv_ctx != NULL) {
        memset(netdrv_ctx, 0, sizeof(lwip_netdrv_ctx_t));
        lwip_netif_set_netdrv(&netdrv_ctx->netif, netdrv);
        if (NULL == __first_netif) {
            __first_netif = netdrv_ctx;
        }
    }
}

void vsf_pnp_on_netdrv_connected(vk_netdrv_t *netdrv)
{
    struct netif *netif = vk_netdrv_get_netif(netdrv);
    lwip_netdrv_ctx_t *netdrv_ctx = vsf_container_of(netif, lwip_netdrv_ctx_t, netif);

    LOCK_TCPIP_CORE();
    dhcp_set_struct(netif, &netdrv_ctx->netif_dhcp);
    netif_set_up(netif);
    netif_set_default(netif);

    dhcp_start(netif);
    UNLOCK_TCPIP_CORE();
    vsf_trace(VSF_TRACE_INFO, "dhcpc: start" VSF_TRACE_CFG_LINEEND);
}

void vsf_pnp_on_netdrv_del(vk_netdrv_t *netdrv)
{
    struct netif *netif = vk_netdrv_get_netif(netdrv);
    lwip_netdrv_ctx_t *netdrv_ctx = vsf_container_of(netif, lwip_netdrv_ctx_t, netif);

    vsf_heap_free(netdrv_ctx);
}

VSF_CAL_WEAK(app_mdns_connect)
void app_mdns_connect(void)
{
}

VSF_CAL_WEAK(app_mdns_start)
void app_mdns_start(uint8_t *mac)
{
}

VSF_CAL_WEAK(app_mdns_stop)
void app_mdns_stop(void)
{
}

void app_wifi_sta_on_reconnect(void)
{
    app_mdns_stop();
}

void app_wifi_sta_on_connected(void)
{
    struct netif *netif = vsf_board_get_netif();
    app_mdns_start(netif->hwaddr);
    app_mdns_connect();
}

void app_wifi_ap_on_started(char *ssid, char *pass)
{
#if VSF_USE_QRCODE == ENABLED
    // cmdline: qrcode "Scan to connect AP" "WIFI:S:ssid;P:pass;T:WPA/WPA2;H:vsf;"
    const char *format = "qrcode \"S:%s P:%s\" \"WIFI:S:%s;P:%s;T:WPA/WPA2;H:vsf;\"";
    char cmdline[strlen(format) + 4 * 32];  // format_size + max_size of ssid(32) and pass(32)
    sprintf(cmdline, format, ssid, pass, ssid, pass);
    system(cmdline);
#endif

    struct netif *netif = vsf_board_get_netif();
    app_mdns_start(netif->hwaddr);
    app_mdns_connect();
}

#endif

// linux

VSF_CAL_WEAK(vsf_board_init_linux)
void vsf_board_init_linux(void)
{
}

static int __vsf_board_init_linux_main(int argc, char **argv)
{
    vsf_board_init_linux();
    return 0;
}

VSF_CAL_WEAK(vsf_board_prepare_hw_for_linux)
void vsf_board_prepare_hw_for_linux(void)
{
}

VSF_CAL_WEAK(vsf_linux_install_package_manager)
void vsf_linux_install_package_manager(
#ifdef __WIN__
    const char *package_path,
#else
    vk_romfs_info_t *fsinfo,
#endif
    bool can_uninstall, bool can_install)
{
}

static char * __app_config_read(FILE *f)
{
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size > 0) {
        char *buffer = malloc(size + 1);
        if (size != fread(buffer, 1, size, f)) {
            free(buffer);
            return NULL;
        }
        buffer[size] = '\0';
        return buffer;
    }
    return NULL;
}

int app_config_read(const char *cfgname, char *cfgvalue, int valuelen)
{
    int result = -1;
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
    result = 0;

not_found:
    free(data);
free_token_and_fail:
    free(token);
close_and_fail:
    fclose(f);
    return result;
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
    if ((cfgvalue != NULL) && (value == strstr(value, cfgvalue)) && (value[strlen(cfgvalue)] == '\n')) {
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
    if (cfgvalue != NULL) {
        fprintf(f, "%s%s:%s\n", NULL == data ? "\n" : "", cfgname, cfgvalue);
    }
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

static int __appcfg_main(int argc, char *argv[])
{
    int ret = 0;
    if (1 == argc) {
        int fd = open(APP_CONFIG_FILE, 0);
        if (fd < 0) {
            printf("appcfg: not exists at " APP_CONFIG_FILE "\n");
            return -1;
        }
        sendfile(STDOUT_FILENO, fd, NULL, -1);
        close(fd);
    } else if (2 == argc) {
        char buffer[64];
        ret = app_config_read(argv[1], buffer, sizeof(buffer));
        if (ret != 0) {
            printf("appcfg: fail to get %s\n", argv[1]);
        } else {
            printf("appcfg: %s=%s\n", argv[1], buffer);
        }
    } else if (3 == argc) {
        if (!strcmp(argv[1], "-d")) {
            printf("appcfg: remove %s ... ", argv[2]);
            argv[1] = argv[2];
            argv[2] = NULL;
        } else {
            printf("appcfg: %s=%s ... ", argv[1], argv[2]);
        }
        ret = app_config_write(argv[1], argv[2]);
        if (ret != 0) {
            printf("failed\n");
        } else {
            printf("succeed\n");
        }
    } else {
        printf("format: %s [CONFIG_NAME [CONFIG_VALUE]]\n", argv[0]);
        ret = -1;
    }
    return ret;
}

#if VSF_USE_BTSTACK == ENABLED

VSF_CAL_WEAK(btstack_main)
int btstack_main(int argc, char **argv)
{
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);
    hci_power_control(HCI_POWER_ON);
    return 0;
}

static vsf_eda_t *__btstack_scan_eda;
static void __btstack_scan_pkthandler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    uint8_t event = hci_event_packet_get_type(packet);
    bd_addr_t addr;

    switch (event) {
    case HCI_EVENT_EXTENDED_INQUIRY_RESPONSE: {
            uint8_t *eir_data;
            ad_context_t context;
            int event_type = hci_event_packet_get_type(packet);
            int num_reserved_fields = (event_type == HCI_EVENT_INQUIRY_RESULT) ? 2 : 1;
            if (event_type == HCI_EVENT_EXTENDED_INQUIRY_RESPONSE) {
                // EIR packets only contain a single inquiry response
                eir_data = &packet[3 + (6 + 1 + num_reserved_fields + 3 + 2 + 1)];
                for (ad_iterator_init(&context, EXTENDED_INQUIRY_RESPONSE_DATA_LEN, eir_data) ; ad_iterator_has_more(&context) ; ad_iterator_next(&context)){
                    uint8_t data_type    = ad_iterator_get_data_type(&context);
                    uint8_t data_size    = ad_iterator_get_data_len(&context);
                    const uint8_t * data = ad_iterator_get_data(&context);
                    vsf_trace_debug("eir: type %d size %d:\n", data_type, data_size);
                    vsf_trace_buffer(VSF_TRACE_DEBUG, (void *)data, data_size);
                }
            }
        }
        break;
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
            vsf_trace_info("btstack started...", VSF_TRACE_CFG_LINEEND);
        }
        break;
    case GAP_EVENT_INQUIRY_RESULT:
        gap_event_inquiry_result_get_bd_addr(packet, addr);

        vsf_trace_info("Device found: %s ",  bd_addr_to_str(addr));
        vsf_trace_info("with COD: 0x%06x, ", (unsigned int)gap_event_inquiry_result_get_class_of_device(packet));
        vsf_trace_info("pageScan %d, ",      gap_event_inquiry_result_get_page_scan_repetition_mode(packet));
        vsf_trace_info("clock offset 0x%04x",gap_event_inquiry_result_get_clock_offset(packet));

        if (gap_event_inquiry_result_get_rssi_available(packet)) {
            vsf_trace_info(", rssi %d dBm", (int8_t) gap_event_inquiry_result_get_rssi(packet));
        }
        if (gap_event_inquiry_result_get_name_available(packet)) {
            char name_buffer[240];
            int name_len = gap_event_inquiry_result_get_name_len(packet);
            memcpy(name_buffer, gap_event_inquiry_result_get_name(packet), name_len);
            name_buffer[name_len] = 0;
            vsf_trace_info(", name '%s'", name_buffer);
        }
        vsf_trace_info(VSF_TRACE_CFG_LINEEND);
        break;
    case GAP_EVENT_INQUIRY_COMPLETE:
        VSF_ASSERT(__btstack_scan_eda != NULL);
        vsf_eda_post_evt(__btstack_scan_eda, VSF_EVT_USER);
        break;
    default:
        break;
    }
}

int __btstack_scan_main(int argc, char **argv)
{
    static btstack_packet_callback_registration_t __hci_event_callback_registration;
    if (NULL == __hci_event_callback_registration.callback) {
        __hci_event_callback_registration.callback = &__btstack_scan_pkthandler;
        hci_add_event_handler(&__hci_event_callback_registration);
    }

    if (HCI_STATE_WORKING == hci_get_state()) {
        printf("Starting inquiry scan.." VSF_TRACE_CFG_LINEEND);
        gap_inquiry_start(5);

        __btstack_scan_eda = vsf_eda_get_cur();
        VSF_ASSERT(__btstack_scan_eda != NULL);
        vsf_thread_wfe(VSF_EVT_USER);
        __btstack_scan_eda = NULL;
    } else {
        printf("bluetooth is not available.." VSF_TRACE_CFG_LINEEND);
    }
    return 0;
}
#endif


#if VSF_USE_USB_HOST == ENABLED

#   if VSF_USE_SCSI == ENABLED && VSF_USE_MAL == ENABLED && VSF_MAL_USE_SCSI_MAL == ENABLED
typedef struct vsf_scsi_mounter_t {
    implement(vk_scsi_mal_t)
    implement(vk_malfs_mounter_t)
    vsf_mutex_t __mutex;
    vsf_eda_t eda;
    enum {
        STATE_INIT,
        STATE_OPEN_DIR,
        STATE_MBR_MOUNT,
    } state;
    bool is_mounted;
} vsf_scsi_mounter_t;
static vsf_scsi_mounter_t __usr_scsi_mounter;

void __user_scsi_mounter(vsf_eda_t *eda, vsf_evt_t evt)
{
    vsf_scsi_mounter_t *mounter = vsf_container_of(eda, vsf_scsi_mounter_t, eda);

    switch (evt) {
    case VSF_EVT_INIT:
        mounter->mutex = &mounter->__mutex;
        mounter->state = STATE_INIT;
        vk_mal_init(&mounter->use_as__vk_mal_t);
        break;
    case VSF_EVT_RETURN:
        if (vsf_eda_get_return_value() != VSF_ERR_NONE) {
            vsf_trace_error("fail to mount scsi drive" VSF_TRACE_CFG_LINEEND);
            break;
        }
        switch (mounter->state) {
        case STATE_INIT:
            mounter->state = STATE_OPEN_DIR;
            vk_file_open(NULL, "/mnt/scsi", &mounter->dir);
            break;
        case STATE_OPEN_DIR:
            mounter->mal = &mounter->use_as__vk_mal_t;
            mounter->state = STATE_MBR_MOUNT;
            vk_malfs_mount(&mounter->use_as__vk_malfs_mounter_t);
            break;
        case STATE_MBR_MOUNT:
            if (mounter->partition_mounted > 0) {
                vsf_trace_debug("scsi mounted at /mnt/scsi" VSF_TRACE_CFG_LINEEND);
            } else {
                vsf_trace_debug("no supported partition" VSF_TRACE_CFG_LINEEND);
            }
            break;
        }
        break;
    }
}

void vsf_scsi_on_new(vk_scsi_t *scsi)
{
    if (!__usr_scsi_mounter.is_mounted) {
        __usr_scsi_mounter.is_mounted = true;
        __usr_scsi_mounter.scsi = scsi;
        __usr_scsi_mounter.drv = &vk_scsi_mal_drv;
        vsf_eda_start(&__usr_scsi_mounter.eda, &(vsf_eda_cfg_t){
            .fn.evthandler      = __user_scsi_mounter,
            .priority           = vsf_prio_0,
        });
    }
}

void vsf_scsi_on_delete(vk_scsi_t *scsi)
{
    if (scsi == __usr_scsi_mounter.scsi) {
        __usr_scsi_mounter.is_mounted = false;
    }
}
#   endif

#   if VSF_USBH_USE_UAC == ENABLED
void vsf_usbh_uac_on_new(void *uac, usb_uac_ac_interface_header_desc_t *ac_header)
{
    vk_usbh_uac_stream_t *stream;

    vsf_trace(VSF_TRACE_INFO, "usbh_uac: new dev" VSF_TRACE_CFG_LINEEND);
    for (int i = 0; i < ac_header->bInCollection; i++) {
        stream = vsf_usbh_uac_get_stream_info(uac, i);
        vsf_trace(VSF_TRACE_INFO, "  stream%d:" VSF_TRACE_CFG_LINEEND, i);
        vsf_trace(VSF_TRACE_INFO, "    direction: %s" VSF_TRACE_CFG_LINEEND, stream->is_in ? "IN" : "OUT");
        vsf_trace(VSF_TRACE_INFO, "    format: 0x%04X" VSF_TRACE_CFG_LINEEND, stream->format);
        vsf_trace(VSF_TRACE_INFO, "    channel_num: %d" VSF_TRACE_CFG_LINEEND, stream->channel_num);
        vsf_trace(VSF_TRACE_INFO, "    sample_size: %d" VSF_TRACE_CFG_LINEEND, stream->sample_size);
        vsf_trace(VSF_TRACE_INFO, "    sample_rate: %d" VSF_TRACE_CFG_LINEEND, stream->sample_rate);
    }
}
#   endif

#   if VSF_USBH_USE_UVC == ENABLED
void vsf_usbh_uvc_on_new(void *uvc, usb_uvc_vc_interface_header_desc_t *vc_header,
        usb_uvc_vs_interface_header_desc_t *vs_header)
{
    usb_uvc_format_desc_t *format;
    usb_uvc_frame_desc_t *frame, *tmp;
    uint16_t size;

    vsf_trace(VSF_TRACE_INFO, "usbh_uvc: new dev" VSF_TRACE_CFG_LINEEND);
    for (int i = 0; i < vs_header->bNumFormats; i++) {
        format = vsf_usbh_uvc_get_format(uvc, i);
        vsf_trace(VSF_TRACE_INFO, "  format%d:" VSF_TRACE_CFG_LINEEND, i);
        vsf_trace(VSF_TRACE_INFO, "    type: %d" VSF_TRACE_CFG_LINEEND, format->bDescriptorSubtype);

        frame = (usb_uvc_frame_desc_t *)format;
        size = vs_header->wTotalLength - ((uint8_t *)frame - (uint8_t *)vs_header);
        for (int j = 0; j < format->bNumFrameDescriptors; j++) {
            vsf_usbh_uvc_get_desc((uint8_t *)frame + frame->bLength, size, format->bDescriptorSubtype + 1, (void **)&tmp);
            size -= (uint8_t *)tmp - (uint8_t *)frame;
            frame = tmp;

            vsf_trace(VSF_TRACE_INFO, "    frame%d:" VSF_TRACE_CFG_LINEEND, j);
            vsf_trace(VSF_TRACE_INFO, "      resolution: %d x %d" VSF_TRACE_CFG_LINEEND, frame->wWidth, frame->wHeight);

            for (int k = 0; k < frame->bFrameIntervalType; k++) {
                vsf_trace(VSF_TRACE_INFO, "      fps: %dHz" VSF_TRACE_CFG_LINEEND, 10000000 / frame->dwFrameInterval[k]);
            }
        }
    }
}
#   endif

#   if VSF_USBH_USE_BTHCI == ENABLED && VSF_USE_BTSTACK == ENABLED
vsf_err_t vsf_bluetooth_h2_on_new(void *dev, vk_usbh_dev_id_t *id)
{
    btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_vsf_get_instance());

    extern const hci_transport_t * hci_transport_usb_instance(void);
    hci_init(hci_transport_usb_instance(), dev);
    hci_set_link_key_db(btstack_link_key_db_memory_instance());

    if ((id->idVendor == 0x0A12) && (id->idProduct == 0x0001)) {
        hci_set_chipset(btstack_chipset_csr_instance());
    } else if ((id->idVendor == 0x0A5C) && (id->idProduct == 0x21E8)) {
        hci_set_chipset(btstack_chipset_bcm_instance());
    } else {
        return VSF_ERR_FAIL;
    }

    btstack_main(0, NULL);
    return VSF_ERR_NONE;
}
#   endif

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
#   if VSF_USBH_USE_ECM == ENABLED && VSF_USE_TCPIP == ENABLED
        static vk_usbh_class_t __usbh_ecm = { .drv = &vk_usbh_ecm_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_ecm);
#       if VSF_USBH_USE_LIBUSB == ENABLED
        static vk_usbh_class_t __usbh_ecm_block_libusb = { .drv = &vk_usbh_ecm_block_libusb_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_ecm_block_libusb);
#       endif
#   endif
#   if VSF_USBH_USE_NCM == ENABLED && VSF_USE_TCPIP == ENABLED
        static vk_usbh_class_t __usbh_ncm = { .drv = &vk_usbh_ncm_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_ncm);
#       if VSF_USBH_USE_LIBUSB == ENABLED
        static vk_usbh_class_t __usbh_ncm_block_libusb = { .drv = &vk_usbh_ncm_block_libusb_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_ncm_block_libusb);
#       endif
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
#       if VSF_USE_SCSI == ENABLED && VSF_USE_MAL == ENABLED && VSF_MAL_USE_SCSI_MAL == ENABLED
        mkdirs("/mnt/scsi", 0);
#       endif
#   endif
#   if VSF_USBH_USE_UAC == ENABLED
        static vk_usbh_class_t __usbh_uac = { .drv = &vk_usbh_uac_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_uac);
#   endif
#   if VSF_USBH_USE_UVC == ENABLED
        static vk_usbh_class_t __usbh_uvc = { .drv = &vk_usbh_uvc_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_uvc);
#   endif
#   if VSF_USBH_USE_BTHCI == ENABLED && VSF_USE_BTSTACK == ENABLED
        static vk_usbh_class_t __usbh_bthci = { .drv = &vk_usbh_bthci_drv };
        vk_usbh_register_class(&vsf_board.usbh_dev, &__usbh_bthci);
#   endif
    }
    return 0;
}
#endif

#if VSF_HAL_USE_SDIO == ENABLED
static void __sdmmc_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    switch (evt) {
    case VSF_EVT_TIMER:
    case VSF_EVT_INIT:
        if (!VSF_BOARD_SDMMC_DETECTED()) {
            // TODO: unmount fs if mounted, mal_fini if initialized
            __sdmmc_state = SDMMC_STATE_WAIT_DET;
        set_timer:
            vsf_teda_set_timer_ms(100);
            break;
        }
        if (SDMMC_STATE_WAIT_DET == __sdmmc_state) {
            __sdmmc_state = SDMMC_STATE_WAIT_DET_STABLE;
            goto set_timer;
        }
        __sdmmc_state = SDMMC_STATE_MAL;

        vk_mal_init(&__sdmmc_mal.use_as__vk_mal_t);
        break;
    case VSF_EVT_RETURN:
        if (SDMMC_STATE_MAL == __sdmmc_state) {
            if (VSF_ERR_NONE == vsf_eda_get_return_value()) {
                vsf_trace_debug("sdmmc_probe done" VSF_TRACE_CFG_LINEEND);
                vsf_trace_debug("sdmmc.high_capacity : %d" VSF_TRACE_CFG_LINEEND, __sdmmc_mal.high_capacity);
                vsf_trace_debug("sdmmc.version : %08X" VSF_TRACE_CFG_LINEEND, __sdmmc_mal.version);
                vsf_trace_debug("sdmmc.capacity : %lld MB" VSF_TRACE_CFG_LINEEND, __sdmmc_mal.capacity / 2000);

                __sdmmc_state = SDMMC_STATE_FS_OPEN;
                __sdmmc_mounter = vsf_heap_malloc(sizeof(*__sdmmc_mounter));
                VSF_ASSERT(__sdmmc_mounter != NULL);
                vk_file_open(NULL, "/mnt/sdmmc", &__sdmmc_mounter->dir);
            } else {
                vsf_trace_debug("sdmmc_probe failed" VSF_TRACE_CFG_LINEEND);
            sdmmc_mal_close:
                __sdmmc_state = SDMMC_STATE_FAIL;
                goto set_timer;
            }
        } else if (SDMMC_STATE_FS_OPEN == __sdmmc_state) {
            if (VSF_ERR_NONE == vsf_eda_get_return_value()) {
                __sdmmc_fs_mutex = vsf_heap_malloc(sizeof(*__sdmmc_fs_mutex));
                VSF_ASSERT(__sdmmc_fs_mutex != NULL);

                __sdmmc_mounter->mal = &__sdmmc_mal.use_as__vk_mal_t;
                __sdmmc_mounter->mutex = (vsf_mutex_t *)__sdmmc_fs_mutex;
                __sdmmc_state = SDMMC_STATE_FS_MOUNT;
                vk_malfs_mount(__sdmmc_mounter);
            } else {
                vsf_trace_debug("fail to open /mnt/sdmmc" VSF_TRACE_CFG_LINEEND);
                goto sdmmc_mal_close;
            }
        } else if (SDMMC_STATE_FS_MOUNT == __sdmmc_state) {
            vsf_err_t mount_err = __sdmmc_mounter->err;
            uint8_t partition_mounted = __sdmmc_mounter->partition_mounted;
            vsf_heap_free(__sdmmc_mounter);
            __sdmmc_mounter = NULL;

            if (0 == partition_mounted) {
                vsf_heap_free(__sdmmc_fs_mutex);
                __sdmmc_fs_mutex = NULL;

                if (mount_err != VSF_ERR_NONE) {
                    vsf_trace_debug("fail to mount sdmmc" VSF_TRACE_CFG_LINEEND);
                } else {
                    vsf_trace_debug("no supported partition" VSF_TRACE_CFG_LINEEND);
                }
                goto sdmmc_mal_close;
            } else {
                vsf_trace_debug("sdmmc mounted at /mnt/sdmmc" VSF_TRACE_CFG_LINEEND);
            }
        }
        break;
    }
}
#endif

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED
static bool __romfs_installed = false;
static vk_romfs_info_t __romfs_info = {
    .image      = (vk_romfs_header_t *)(APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_ROMFS_ADDR),
    .image_size = APP_MSCBOOT_CFG_ROMFS_SIZE,
    .is_chained = true,
    .alignment  = APP_MSCBOOT_CFG_ERASE_ALIGN,
};
void vpm_on_installed(void)
{
    if (!__romfs_installed && !__usr_linux_boot) {
        if (!mount(NULL, "usr", &vk_romfs_op, 0, (const void *)&__romfs_info)) {
            __romfs_installed = true;
        }
    }
}
#endif

VSF_CAL_WEAK(hwtest_main)
int hwtest_main(int argc, char **argv)
{
    return 0;
}

VSF_CAL_WEAK(webterminal_main)
int webterminal_main(int argc, char **argv)
{
    return 0;
}

static int __reset_main(int argc, char **argv)
{
    vsf_arch_reset();
    return 0;
}

// application vpls
// add application func/var declare in vsf_usr_cfg.h
#if VSF_USE_APPLET == ENABLED || (VSF_USE_LINUX == ENABLED && VSF_LINUX_USE_APPLET == ENABLED)
typedef struct vsf_app_vplt_t {
    vsf_vplt_info_t info;
    VSF_APPLET_VPLT_ENTRY_FUNC_DEF(app_mdns_rename);
    VSF_APPLET_VPLT_ENTRY_FUNC_DEF(app_mdns_remove_service);
    VSF_APPLET_VPLT_ENTRY_FUNC_DEF(app_mdns_update_service);
    VSF_APPLET_VPLT_ENTRY_FUNC_DEF(app_mdns_update_txt);
    VSF_APPLET_VPLT_ENTRY_FUNC_DEF(app_config_read);
    VSF_APPLET_VPLT_ENTRY_FUNC_DEF(app_config_write);
} vsf_app_vplt_t;

static __VSF_VPLT_DECORATOR__ vsf_app_vplt_t __vsf_app_vplt = {
    VSF_APPLET_VPLT_INFO(vsf_app_vplt_t, 0, 0, true),
    VSF_APPLET_VPLT_ENTRY_FUNC(app_mdns_rename),
    VSF_APPLET_VPLT_ENTRY_FUNC(app_mdns_remove_service),
    VSF_APPLET_VPLT_ENTRY_FUNC(app_mdns_update_service),
    VSF_APPLET_VPLT_ENTRY_FUNC(app_mdns_update_txt),
    VSF_APPLET_VPLT_ENTRY_FUNC(app_config_read),
    VSF_APPLET_VPLT_ENTRY_FUNC(app_config_write),
};
#endif

#if VSF_USE_UI == ENABLED
static void __vk_disp_on_inited(vk_disp_t *disp)
{
    vsf_eda_post_evt((vsf_eda_t *)disp->ui_data, VSF_EVT_USER);
}

VSF_CAL_WEAK(ui_main)
int ui_main(int argc, char **argv)
{
    return 0;
}

#   if VSF_LINUX_USE_DEVFS == ENABLED
static int __fill_screen_main(int argc, char **argv)
{
    if (argc > 2) {
        printf("%s [COLOR]", argv[0]);
        return -1;
    }

    int fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open /dev/fb0\n");
        return -1;
    }

    struct fb_var_screeninfo fb_var;
    ioctl(fd, FBIOGET_VSCREENINFO, &fb_var);

    uint8_t pixel_size = (fb_var.bits_per_pixel + 7) >> 3;
    uint16_t width = fb_var.xres;
    uint16_t height = fb_var.yres;
    uint32_t disp_color = (2 == argc) ? strtoul(argv[1], NULL, 0) : 0;
    uint32_t pitch = pixel_size * width;
    vk_disp_area_t area = {
        .size.x = width,
        .size.y = 1,
        .pos.x = 0,
    };
    uint8_t *linebuf = vsf_heap_malloc(pitch);
    if (NULL == linebuf) {
        fprintf(stderr, "fail to allocate line buffer for screen\n");
        goto fail_close_fd;
    }
    switch (pixel_size) {
    case 1: for (int i = 0; i < width; i++) {((uint8_t *)linebuf)[i] = disp_color;}    break;
    case 2: for (int i = 0; i < width; i++) {((uint16_t *)linebuf)[i] = disp_color;}   break;
    case 4: for (int i = 0; i < width; i++) {((uint32_t *)linebuf)[i] = disp_color;}   break;
    }

    for (uint16_t i = 0; i < height; i++) {
        area.pos.y = i;
        ioctl(fd, FBIOSET_AREA, &area);
        write(fd, linebuf, pitch);
    }

    vsf_heap_free(linebuf);
    close(fd);
    return 0;

fail_close_fd:
    close(fd);
    return -1;
}

static int __write_screen_main(int argc, char **argv)
{
    if (argc != 4) {
        printf("%s X Y COLOR", argv[0]);
        return -1;
    }

    int fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open /dev/fb0\n");
        return -1;
    }

    struct fb_var_screeninfo fb_var;
    ioctl(fd, FBIOGET_VSCREENINFO, &fb_var);

    uint8_t pixel_size = (fb_var.bits_per_pixel + 7) >> 3;
    uint32_t disp_color = strtoul(argv[3], NULL, 0);
    vk_disp_area_t area = {
        .size.x = 1,
        .size.y = 1,
        .pos.x = strtoul(argv[1], NULL, 0),
        .pos.y = strtoul(argv[2], NULL, 0),
    };

    ioctl(fd, FBIOSET_AREA, &area);
    write(fd, &disp_color, pixel_size);
    close(fd);
    return 0;
}
#   endif
#endif

static bool __install_embedded_busybox = true;
bool app_shell_is_orig_busybox(void)
{
    return !__install_embedded_busybox;
}

#if VSF_LINUX_USE_X11 == ENABLED

#include <nano-X.h>
#include "nxlib.h"

#if VSF_LINUX_USE_FLTK == ENABLED
extern int fltk_forms_main(int argc, char *argv[]);
static int __fltk_forms_main(int argc, char *argv[])
{
#if !NONETWORK
    INIT_PER_THREAD_DATA();
#endif
    NANOX_NX11_VSF_INIT_PROCESS_DATA();
    return fltk_forms_main(argc, argv);
}
#endif

extern int tinygl_gears_main(int argc, char *argv[]);
static int __tinygl_gears_main(int argc, char *argv[])
{
#if !NONETWORK
    INIT_PER_THREAD_DATA();
#endif
    NANOX_NX11_VSF_INIT_PROCESS_DATA();
    return tinygl_gears_main(argc, argv);
}

extern int nxterm_main(int argc, char **argv);
extern int nxkbd_main(int argc, char **argv);
extern int nxclock_main(int argc, char **argv);
extern int nxeyes_main(int argc, char **argv);
extern int world_main(int argc, char **argv);
extern int nanox_srv_main(int argc, char **argv);

#if !NONETWORK
static int __nxterm_main(int argc, char **argv)
{
    INIT_PER_THREAD_DATA();
    return nxterm_main(argc, argv);
}

static int __nxkbd_main(int argc, char **argv)
{
    INIT_PER_THREAD_DATA();
    return nxkbd_main(argc, argv);
}

static int __nxclock_main(int argc, char **argv)
{
    INIT_PER_THREAD_DATA();
    return nxclock_main(argc, argv);
}

static int __nxeyes_main(int argc, char **argv)
{
    INIT_PER_THREAD_DATA();
    return nxeyes_main(argc, argv);
}

static int __world_main(int argc, char **argv)
{
    INIT_PER_THREAD_DATA();
    return world_main(argc, argv);
}

static int __nanox_srv_main(int argc, char **argv)
{
    INIT_PER_THREAD_DATA();
    return nanox_srv_main(argc, argv);
}
#endif

static int __startx_main(int argc, char **argv)
{
    return execl("/bin/sh", "sh", "~/.xinitrc", NULL);
}
#endif

#if APP_USE_SDLPAL == ENABLED
#   include "../font.h"

static int __sdlpal_load_file(const char *filename, void **buffer, uint32_t size)
{
    if (NULL == *buffer) {
        FILE *fp = fopen(filename, "r");
        if (NULL == fp) {
            printf("fail to open %s\n", filename);
            return -1;
        }
        *buffer = malloc(size);
        if (NULL == *buffer) {
            printf("fail to allocate buffer for %s\n", filename);
            fclose(fp);
            return -1;
        }
        if (fread(*buffer, 1, size, fp) != size) {
            printf("fail to read %s\n", filename);
            free(*buffer);
            *buffer = NULL;
            fclose(fp);
            return -1;
        }
        fclose(fp);
    }
    return 0;
}

static int __sdlpal_main(int argc, char *argv[])
{
    #define unicode_font_size       65536
    #define fontglyph_cn_size       8836
    #define fontglyph_jp_size       6879
    #define fontglyph_tw_size       13826
    extern unsigned char (*unicode_font)[32];
    extern unsigned char *font_width;
    extern font_t *fontglyph_tw;
    extern font_t *fontglyph_jp;
    extern font_t *fontglyph_cn;

#if 0
    // save font resources to files
    extern const unsigned char __unicode_font[unicode_font_size][32];
    extern const unsigned char __font_width[unicode_font_size];
    extern font_t __fontglyph_tw[fontglyph_tw_size];
    extern font_t __fontglyph_jp[fontglyph_jp_size];
    extern font_t __fontglyph_cn[fontglyph_cn_size];

    FILE *fp;

    // code to save font data in file
    fp = fopen("/usr/sdlpal/fontflyph_tw.bin", "w+");
    fwrite(__fontglyph_tw, sizeof(__fontglyph_tw), 1, fp);
    fclose(fp);
    fp = fopen("/usr/sdlpal/fontflyph_cn.bin", "w+");
    fwrite(__fontglyph_cn, sizeof(__fontglyph_cn), 1, fp);
    fclose(fp);
    fp = fopen("/usr/sdlpal/fontflyph_jp.bin", "w+");
    fwrite(__fontglyph_jp, sizeof(__fontglyph_jp), 1, fp);
    fclose(fp);

    fp = fopen("/usr/sdlpal/unicode_font.bin", "w+");
    fwrite(__unicode_font, sizeof(__unicode_font), 1, fp);
    fclose(fp);
    fp = fopen("/usr/sdlpal/unicode_font_width.bin", "w+");
    fwrite(__font_width, sizeof(__font_width), 1, fp);
    fclose(fp);
#endif

#if 1
    // load font resources from files
    if (__sdlpal_load_file("fontflyph_tw.bin", (void **)&fontglyph_tw, fontglyph_tw_size * sizeof(font_t))) {
        return -1;
    }
    if (__sdlpal_load_file("fontflyph_jp.bin", (void **)&fontglyph_jp, fontglyph_jp_size * sizeof(font_t))) {
        return -1;
    }
    if (__sdlpal_load_file("fontflyph_cn.bin", (void **)&fontglyph_cn, fontglyph_cn_size * sizeof(font_t))) {
        return -1;
    }

    if (__sdlpal_load_file("unicode_font.bin", (void **)&unicode_font, unicode_font_size * 32)) {
        return -1;
    }
    if (__sdlpal_load_file("unicode_font_width.bin", (void **)&font_width, unicode_font_size)) {
        return -1;
    }
#elif 1
    // font is in .rodata
    fontglyph_tw = &__fontglyph_tw[0];
    fontglyph_jp = &__fontglyph_jp[0];
    fontglyph_cn = &__fontglyph_cn[0];
    unicode_font = &__unicode_font[0];
    font_width = &__font_width[0];
#else
#   error how to load font resources?
#endif

    extern int sdlpal_main(int argc, char *argv[]);
    return sdlpal_main(argc, argv);
}
#endif

int vsf_linux_create_fhs(void)
{
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();

    // 0.5. root fs, some other initialization may depend on ${HOME}, so initialize /root first
#if VSF_HAL_USE_FLASH == ENABLED && defined(APP_MSCBOOT_CFG_FLASH)
    // vsf_flash_init will be called in vk_mal_init
    vk_mal_init(&flash_mal.use_as__vk_mal_t);
#endif
#if defined(APP_MSCBOOT_CFG_FLASH) && defined(APP_MSCBOOT_CFG_ROOT_SIZE) && (APP_MSCBOOT_CFG_ROOT_SIZE > 0) && defined(APP_MSCBOOT_CFG_ROOT_ADDR)
    vk_mal_init(&root_mal.use_as__vk_mal_t);

    static vk_lfs_info_t __root_fs;
    vsf_lfs_bind_mal(&__root_fs.config, &root_mal.use_as__vk_mal_t);

    mkdir("/root", 0);
    if (mount(NULL, "root", &vk_lfs_op, 0, (const void *)&__root_fs) != 0) {
        printf("Fail to mount /root.\n");
    }
    putenv("HOME=/root");
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/appcfg", __appcfg_main);
#elif defined(__WIN__)
    static vk_winfs_info_t __root_fs = {
        .root.name = "./root",
    };
    mkdir("/root", 0);
    if (mount(NULL, "root", &vk_winfs_op, 0, (const void *)&__root_fs) != 0) {
        printf("Fail to mount /root.\n");
    }
    putenv("HOME=/root");
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/appcfg", __appcfg_main);
#else
#   warning where should be ${HOME}?
#endif

    // 1. hardware driver
    vsf_board_prepare_hw_for_linux();
#if VSF_HAL_USE_GPIO == ENABLED && VSF_HW_GPIO_COUNT > 0
    vsf_linux_fs_bind_gpio_hw("/sys/class/gpio");
#endif
#if VSF_HAL_USE_I2C == ENABLED
    vsf_linux_fs_bind_i2c("/dev/i2c-0", vsf_board.i2c);
#endif

    vsf_linux_create_pty(1);
#if VSF_USE_UI == ENABLED
    if (vsf_board.display_dev != NULL) {
#   if VSF_LINUX_USE_DEVFS == ENABLED
        vsf_linux_fs_bind_disp("/dev/fb0", vsf_board.display_dev);
#       if VSF_LINUX_USE_FB_ARGB8888 == ENABLED
        if (vsf_board.display_dev->param.color == VSF_DISP_COLOR_ARGB8888) {
            symlink("/dev/fb0", "/dev/fb_argb8888");
        } else {
            static vk_disp_cvrt_t __disp_argb8888 = {
                .param          = {
                    .width      = VSF_BOARD_DISP_WIDTH,
                    .height     = VSF_BOARD_DISP_HEIGHT,
                    .color      = VSF_DISP_COLOR_ARGB8888,
                    .drv        = &vk_disp_cvrt_drv,
                },
            };
            __disp_argb8888.disp_real = vsf_board.display_dev;
            __disp_argb8888.pixel_buffer_size = vsf_disp_get_frame_size(__disp_argb8888.disp_real);
            __disp_argb8888.pixel_buffer = malloc(__disp_argb8888.pixel_buffer_size);
            VSF_ASSERT(__disp_argb8888.pixel_buffer != NULL);
            vsf_linux_fs_bind_disp("/dev/fb_argb8888", &__disp_argb8888.use_as__vk_disp_t);
        }
#       endif
#   endif

        vsf_board.display_dev->ui_data = vsf_eda_get_cur();
        vsf_board.display_dev->ui_on_ready = __vk_disp_on_inited;
        vk_disp_init(vsf_board.display_dev);
        vsf_thread_wfe(VSF_EVT_USER);

        vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/ui", ui_main);
#   if VSF_LINUX_USE_DEVFS == ENABLED
        vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/fill_screen", __fill_screen_main);
        vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/write_screen", __write_screen_main);
#   endif
    }
#endif

#if VSF_USE_QRCODE == ENABLED
    extern int display_qrcode_main(int argc, char **argv);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/qrcode", display_qrcode_main);
#endif

#if VSF_USE_INPUT == ENABLED && VSF_INPUT_CFG_REGISTRATION_MECHANISM == ENABLED && VSF_LINUX_USE_DEVFS == ENABLED
    static vk_input_notifier_t __input_notifier = {
        .mask =     (1 << VSF_INPUT_TYPE_GAMEPAD)
                |   (1 << VSF_INPUT_TYPE_KEYBOARD)
                |   (1 << VSF_INPUT_TYPE_TOUCHSCREEN)
                |   (1 << VSF_INPUT_TYPE_MOUSE),
    };
    vsf_linux_fs_bind_input("/dev/input/event0", &__input_notifier);
    static vsf_linux_mouse_t __mouse = {
        // TODO: fix this sensitivity if necessary for different mouse devices
        .default_sensitivity = 1.0,
    };
    vsf_linux_fs_bind_mouse("/dev/input/mice", &__mouse);
#   if VSF_LINUX_USE_TERMINAL_KEYBOARD == ENABLED
    static vsf_linux_terminal_keyboard_t __terminal_keyboard;
    vsf_linux_fs_bind_terminal_keyboard("/dev/tty", &__terminal_keyboard);
#   endif
#endif
#if VSF_USE_AUDIO == ENABLED
    vk_audio_init(vsf_board.audio_dev);
#   if VSF_LINUX_USE_DEVFS == ENABLED && VSF_LINUX_DEVFS_USE_ALSA == ENABLED && VSF_LINUX_USE_DEVFS == ENABLED
    vsf_linux_fs_bind_audio("/dev/snd", 0, vsf_board.audio_dev);
    vsf_linux_fs_bind_audio_timer("/dev/snd/timer");
#   endif
#endif
#if VSF_HAL_USE_SDIO == ENABLED
    __sdmmc_mal.sdio                = vsf_board.sdio;
    __sdmmc_mal.voltage             = vsf_board.sdio_voltage;
    __sdmmc_mal.bus_width           = vsf_board.sdio_bus_width;
#endif

    // 2. non-root fs

    // /usr
#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED
    __install_embedded_busybox = __usr_linux_boot;
    if (!__usr_linux_boot) {
        mkdir("/usr", 0);
        if (mount(NULL, "usr", &vk_romfs_op, 0, (const void *)&__romfs_info) != 0) {
            printf("Fail to mount /usr from romfs, install embedded busybox instead.\n");
            __install_embedded_busybox = true;
        } else {
            __romfs_installed = true;
            if (access("/usr/bin/busybox", X_OK) != 0) {
                printf("Can not find valid busybox in /usr/bin/, install embedded busybox instead.\n");
                __install_embedded_busybox = true;
            }
        }
    }

    if (__usr_linux_boot) {
        vsf_linux_install_package_manager(&__romfs_info, true, true);
    } else {
        vsf_linux_install_package_manager(&__romfs_info, false, true);
    }
    if (__install_embedded_busybox) {
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
#elif defined(__WIN__)
    static vk_winfs_info_t __usr_fs = {
        .root.name = "./usr",
    };
    mkdir("/usr", 0);
    if (mount(NULL, "usr", &vk_winfs_op, 0, (const void *)&__usr_fs) != 0) {
        printf("Fail to mount /usr.\n");
    }
    vsf_linux_install_package_manager("/usr", true, true);
    busybox_install();
#else
    busybox_install();
#endif

    // 3. install executables and built-in libraries
#if VSF_USE_APPLET == ENABLED || (VSF_USE_LINUX == ENABLED && VSF_LINUX_USE_APPLET == ENABLED)
    vsf_vplt_load_dyn((vsf_vplt_info_t *)&__vsf_app_vplt.info);
#   if VSF_USE_MBEDTLS == ENABLED
    vsf_vplt_load_dyn((vsf_vplt_info_t *)&vsf_mbedtls_vplt.info);
#   endif
#   if VSF_USE_FREETYPE == ENABLED
    vsf_vplt_load_dyn((vsf_vplt_info_t *)&vsf_freetype_vplt.info);
#   endif
#endif
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/board_init", __vsf_board_init_linux_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/reset", __reset_main);
#if VSF_USE_TCPIP == ENABLED
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/webterminal", webterminal_main);
#endif

#if VSF_LINUX_USE_X11 == ENABLED
    extern int nanox_main(int argc, char** argv);
#if !NONETWORK
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nanox_srv", __nanox_srv_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/world", __world_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nxterm", __nxterm_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nxkbd", __nxkbd_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nxclock", __nxclock_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nxeyes", __nxeyes_main);
#else
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/world", world_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nxterm", nxterm_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nxkbd", nxkbd_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nxclock", nxclock_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/nxeyes", nxeyes_main);
#endif
#   if VSF_LINUX_USE_FLTK == ENABLED
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/fltk", __fltk_forms_main);
#   endif
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/gears", __tinygl_gears_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/startx", __startx_main);
#endif
#if VSF_LINUX_USE_SDL2 == ENABLED
    vsf_sdl2_init(&(vsf_sdl2_cfg_t){
#   ifdef VSF_BOARD_DISPLAY_SDL
        .disp_dev = VSF_BOARD_DISPLAY_SDL,
#   else
        .disp_dev = vsf_board.display_dev,
#   endif
#   if VSF_USE_AUDIO == ENABLED
        .audio_dev = vsf_board.audio_dev,
#   endif
    });
#   if APP_USE_SDLPAL == ENABLED
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/sdlpal", __sdlpal_main);
#   endif
#   if APP_USE_MGBA == ENABLED
    extern int mgba_main(int argc, char **argv);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/mgba", mgba_main);
#   endif
#endif
#if VSF_USE_MICROPYTHON == ENABLED
    extern int mpy_main(int argc, char **argv);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/mpy", mpy_main);
#endif

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED
    // if usb host and device share the same usb hardware, in boot mode, usb device is started
#   ifndef VSF_BOARD_SEPERATE_USB_HOST_DEVICE
    if (__usr_linux_boot) {
#   endif
#   if VSF_USE_USB_DEVICE == ENABLED
#       if VSF_HAL_USE_SDIO == ENABLED
        usbd_mscbot_scsi_config(__app_usbd, 0, 1, !VSF_BOARD_SDMMC_DETECTED());
#       endif
        usbd_mscbot_scsi_config(__app_usbd, 0, 0, false);

        vk_usbd_init(&__app_usbd);
        vk_usbd_connect(&__app_usbd);
#   endif
#   ifndef VSF_BOARD_SEPERATE_USB_HOST_DEVICE
    } else
#   endif
#endif      // APP_MSCBOOT_CFG_ROMFS_ADDR && VSF_FS_USE_ROMFS
    {
#if VSF_HAL_USE_SDIO == ENABLED
        vsf_teda_start(&__sdmmc_task, &(vsf_eda_cfg_t){
            .fn.evthandler  = __sdmmc_evthandler,
            .priority       = vsf_prio_0,
        });
        mkdirs("/mnt/sdmmc", 0);
#endif

        // some hw are maybe not available in boot mode, add hwtest in non-boot mode
        vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/hwtest", hwtest_main);

#if VSF_USE_USB_HOST == ENABLED
        vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/usbhost", __usbh_main);
#   if VSF_USBH_USE_LIBUSB == ENABLED && VSF_LINUX_USE_LIBUSB == ENABLED
        vsf_linux_libusb_startup();
#   endif
#endif
#if VSF_USE_BTSTACK == ENABLED
        vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/btscan", __btstack_scan_main);
#endif
    }

#ifdef __VSF_CPP__
    // call vsf_arch_cpp_startup only after dependency of cpp initializer is ready
    vsf_arch_cpp_startup();
#endif
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
#if VSF_BOARD_USE_EXT_GAMEPAD == ENABLED
    vsf_board_ext_gamepad_init();
    gamepad_io_start(8);
#endif

#if defined(APP_MSCBOOT_CFG_ROMFS_ADDR) && VSF_FS_USE_ROMFS == ENABLED
    if (APP_BOOT1_KEY_IS_DOWN) {
        __usr_linux_boot = true;
    }
    vsf_trace_info("start linux %s..." VSF_TRACE_CFG_LINEEND, __usr_linux_boot ? "boot" : "");
#else
    vsf_trace_info("start linux ..." VSF_TRACE_CFG_LINEEND);
#endif

    vsf_linux_stdio_stream_t stream = {
        .in     = (vsf_stream_t *)&VSF_DEBUG_STREAM_RX,
        .out    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
        .err    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
    };
    vsf_linux_init(&stream);
    return 0;
}

/* EOF */