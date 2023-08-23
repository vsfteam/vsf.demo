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

/*============================ INCLUDES ======================================*/

#define __VSF_HEAP_CLASS_INHERIT__
#include "./vsf_board.h"

#if VSF_USE_LINUX == ENABLED
#   include <unistd.h>
#endif

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
#   include "rtos_al.h"
#endif
#include "reg_sysctrl.h"

#if PLF_WIFI_STACK && VSF_USE_LWIP == ENABLED
#include "net_al.h"

#include "fhost_config.h"
#include "fhost.h"

#include "rwnx_defs.h"
#include "rwnx_msg_tx.h"
#include "wlan_if.h"
#endif

/*============================ MACROS ========================================*/

#if __VSF_HAL_SWI_NUM > 0
#   define MFUNC_IN_U8_DEC_VALUE            (__VSF_HAL_SWI_NUM)
#   include "utilities/preprocessor/mf_u8_dec2str.h"
#   define __FHOST_IPC_IRQ_PRIO             MFUNC_OUT_DEC_STR
#else
#   define __FHOST_IPC_IRQ_PRIO             0
#endif

#if VSF_KERNEL_CFG_SUPPORT_DYNAMIC_PRIOTIRY != ENABLED
#   error "current demo need VSF_KERNEL_CFG_SUPPORT_DYNAMIC_PRIOTIRY, if it\'s not supported\
    please make sure wlan_start_sta is called in task with priority higher than(>) vsf_prio_0"
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/

#if VSF_USE_USB_HOST == ENABLED
static const vk_dwcotg_hcd_param_t __dwcotg_hcd_param = {
    .op                         = &VSF_USB_HC0_IP,
    .priority                   = APP_CFG_USBH_ARCH_PRIO,
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/

#if PLF_WIFI_STACK && VSF_USE_LWIP == ENABLED
struct rwnx_hw hw_env;
#endif

#if VSF_USE_USB_DEVICE == ENABLED
static const vk_dwcotg_dcd_param_t __dwcotg_dcd_param = {
    .op                         = &VSF_USB_DC0_IP,
    .speed                      = VSF_USBD_CFG_SPEED,
        .ulpi_en                = true,
        .dma_en                 = false,
};
#endif

#if VSF_USE_USB_HOST == ENABLED || VSF_USE_USB_DEVICE == ENABLED
vsf_board_t vsf_board = {
#if VSF_USE_USB_HOST == ENABLED
    .usbh_dev                   = {
        .drv                    = &vk_dwcotg_hcd_drv,
        .param                  = (void *)&__dwcotg_hcd_param,
    },
#endif
#if VSF_USE_USB_DEVICE == ENABLED
    .dwcotg_dcd                 = {
        .param                  = &__dwcotg_dcd_param,
    },
#endif
};
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

WEAK(app_config_read)
int app_config_read(const char *cfgname, char *cfgvalue, size_t valuelen)
{
    return -1;
}

WEAK(app_config_write)
int app_config_write(const char *cfgname, char *cfgvalue)
{
    return -1;
}

#if VSF_USE_USB_DEVICE == ENABLED
vsf_usb_dc_from_dwcotg_ip(0, vsf_board.dwcotg_dcd, VSF_USB_DC0)
#endif

#if VSF_USE_USB_HOST == ENABLED
// redefine usbh memory allocation, memory MUST be in 0x001A0000 - 0x001C7FFF
struct __usbh_heap_t {
    implement(vsf_heap_t)
    uint8_t memory[32 * 1024];
    // one more as terminator
    vsf_dlist_t freelist[2];
} static __usbh_heap;

static vsf_dlist_t * __usbh_heap_get_freelist(vsf_heap_t *heap, uint_fast32_t size)
{
    return &__usbh_heap.freelist[0];
}

static void __usbh_heap_init(void)
{
    memset(&__usbh_heap.use_as__vsf_heap_t, 0, sizeof(__usbh_heap.use_as__vsf_heap_t));
    for (uint_fast8_t i = 0; i < dimof(__usbh_heap.freelist); i++) {
        vsf_dlist_init(&__usbh_heap.freelist[i]);
    }
    __usbh_heap.get_freelist = __usbh_heap_get_freelist;
    __vsf_heap_add_buffer(&__usbh_heap.use_as__vsf_heap_t, __usbh_heap.memory, sizeof(__usbh_heap.memory));
}

void * __vsf_usbh_malloc_aligned(uint_fast32_t size, uint_fast32_t alignment)
{
    return __vsf_heap_malloc_aligned(&__usbh_heap.use_as__vsf_heap_t, size, alignment);
}

void * __vsf_usbh_malloc(uint_fast32_t size)
{
    return __vsf_heap_malloc_aligned(&__usbh_heap.use_as__vsf_heap_t, size, 0);
}

void __vsf_usbh_free(void *buffer)
{
    __vsf_heap_free(&__usbh_heap.use_as__vsf_heap_t, buffer);
}
#endif

#if PLF_WIFI_STACK && VSF_USE_LWIP == ENABLED
#   if VSF_USE_LINUX == ENABLED
static int __wifi_ap_main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("format: %s SSID PASSWD [CHANNEL]\n", argv[0]);
        return -1;
    }

    char *ssid = argv[1], *pass = argv[2];
    uint8_t channel = 0;
    if (argc > 3) {
        channel = strtoul(argv[3], NULL, 10);
        VSF_ASSERT(channel <= 14);

        if (14 == channel) {
            printf("warning: wifi 2.4G channel 14 is illegal in some countries.\n");
        }
    }
    set_ap_channel_num(channel);
    set_mac_address(NULL);
    int ret = wlan_start_ap(0, (uint8_t *)ssid, (uint8_t *)pass);
    if (!ret) {
        printf("wifi ap started.\n");
    } else {
        printf("fail to start wifi ap.\n");
    }
    return 0;
}

static int __wifi_scan_main(int argc, char *argv[])
{
    int fhost_vif_idx = 0;
    ipc_host_cntrl_start();

    struct fhost_cntrl_link *cntrl_link = fhost_cntrl_cfgrwnx_link_open();
    if (cntrl_link == NULL) {
        printf("fail to open link\n");
        return -1;
    }
    if (fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_UNKNOWN, false) ||
        fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_STA, false)) {
        fhost_cntrl_cfgrwnx_link_close(cntrl_link);
        printf("fail to set link type to sta\n");
        return -1;
    }

    int nb_res = fhost_scan(cntrl_link, fhost_vif_idx, NULL);
    printf("%d scan results:\n", nb_res);

    nb_res = 0;
    struct mac_scan_result result;
    while (fhost_get_scan_results(cntrl_link, nb_res++, 1, &result)) {
        result.ssid.array[result.ssid.length] = '\0'; // set ssid string ending
        printf("(%3d dBm) CH=%3d BSSID=%02x:%02x:%02x:%02x:%02x:%02x SSID=%s\n",
            (int8_t)result.rssi, phy_freq_to_channel(result.chan->band, result.chan->freq),
            ((uint8_t *)result.bssid.array)[0], ((uint8_t *)result.bssid.array)[1],
            ((uint8_t *)result.bssid.array)[2], ((uint8_t *)result.bssid.array)[3],
            ((uint8_t *)result.bssid.array)[4], ((uint8_t *)result.bssid.array)[5],
            (char *)result.ssid.array);
    }
    fhost_cntrl_cfgrwnx_link_close(cntrl_link);

    return 0;
}

static int __wifi_connect_main(int argc, char *argv[])
{
    char __ssid[33], __pass[33];
    char *ssid, *pass;
    if (1 == argc) {
        if (    app_config_read("wifi_ssid", __ssid, sizeof(__ssid))
            ||  app_config_read("wifi_pass", __pass, sizeof(__pass))) {
            printf("ssid/pass not found in app_config\n", argv[0]);
            printf("format: %s [SSID [PASSWD]]\n", argv[0]);
            return -1;
        }
        ssid = __ssid;
        pass = __pass;
    } else {
        ssid = argv[1];
        pass = argc >= 3 ? argv[2] : "";
    }

    if (wlan_get_connect_status()) {
#if APP_USE_LINUX_DEMO == ENABLED && APP_USE_LINUX_HTTPD_DEMO == ENABLED
        net_if_t *netif = fhost_to_net_if(0);
        LOCK_TCPIP_CORE();
            mdns_resp_remove_netif(netif);
        UNLOCK_TCPIP_CORE();
#endif
    }

    set_mac_address(NULL);
    // wlan_start_sta MUST be called with higher priority than internal wpa(AIC8800_OSAL_CFG_PRIORITY_BASE).
    vsf_prio_t prio = vsf_thread_set_priority(AIC8800_OSAL_CFG_PRIORITY_BASE + 1);
        wlan_start_sta((uint8_t *)ssid, (uint8_t *)pass, 0);
    vsf_thread_set_priority(prio);

    if (wlan_get_connect_status()) {
        printf("wifi connected.\n");

#if APP_USE_LINUX_DEMO == ENABLED && APP_USE_LINUX_HTTPD_DEMO == ENABLED
        // start mdns, not ready
        net_if_t *netif = fhost_to_net_if(0);
        LOCK_TCPIP_CORE();
            mdns_resp_init();
            if (ERR_OK == mdns_resp_add_netif(netif, "vsf", 60 * 10)) {
#if APP_USE_LINUX_DEMO == ENABLED && APP_USE_LINUX_HTTPD_DEMO == ENABLED
                mdns_resp_add_service(netif, "vsfweb", "_http",
                    DNSSD_PROTO_TCP, 80, 3600, __mdns_httpd_srv_txt, NULL);
#endif
            }
        UNLOCK_TCPIP_CORE();
#endif

        app_config_write("wifi_ssid", ssid);
        app_config_write("wifi_pass", pass);
        return 0;
    } else {
        printf("fail to connect %s.\n", argv[1]);
        return -1;
    }
}
#   endif       // VSF_USE_LINUX

int fhost_application_init(void)
{
#if VSF_USE_LINUX == ENABLED
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/wifi_ap", __wifi_ap_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/wifi_scan", __wifi_scan_main);
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/wifi_connect", __wifi_connect_main);
#endif

    char config[16];
    if (!app_config_read("wifi_autostart", config, sizeof(config)) && !strcmp(config, "on")) {
        char *argv[2] = { "wifi_connect", NULL };
        __wifi_connect_main(dimof(argv) - 1, argv);
    }
    return 0;
}
#endif

void vsf_board_init_linux(void)
{
#if PLF_WIFI_STACK && VSF_USE_LWIP == ENABLED
#   ifdef CONFIG_RWNX_LWIP
#       ifndef CFG_WIFI_RAM_VER
    wifi_patch_prepare();
#       endif
    rwnx_ipc_init(&hw_env, &ipc_shared_env);
#   endif

    net_init();
    fhost_init(&(fhost_user_cfg_t){
        .ipc_irq_prio       = VSF_MCONNECT(vsf_arch_prio_, __FHOST_IPC_IRQ_PRIO),
    });
#endif
}

void vsf_board_init(void)
{
    // do not change order below
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_RX);
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);

#if VSF_USE_USB_HOST == ENABLED
    __usbh_heap_init();
#endif

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    // currently known dependency on rtos_al: lwip from vendor and audio in SDK
    if (rtos_init()) {
        VSF_HAL_ASSERT(false);
    }
#endif
}
