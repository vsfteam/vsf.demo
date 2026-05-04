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
 * Minimal esp_netif + wpcap smoke demo for the host (Windows) build.
 *
 * Wiring:
 *   esp_event_loop_create_default (already done by vsf_espidf_init, idempotent)
 *   -> register IP_EVENT handler
 *   -> wpcap netdrv: set_netlink_op(&vk_netdrv_wpcap_netlink_op)
 *   -> esp_netif_new(ETH default) -> esp_netif_attach_netdrv(wpcap)
 *   -> esp_netif_action_start() -> DHCP client kicks in -> IP_EVENT_ETH_GOT_IP
 *
 * The handler prints the acquired IPv4 so the smoke test can be validated
 * purely by inspecting trace output.
 */

#include "./vsf_usr_cfg.h"

#if     VSF_USE_ESPIDF == ENABLED                                              \
    &&  VSF_ESPIDF_CFG_USE_NETIF == ENABLED                                    \
    &&  VSF_NETDRV_USE_WPCAP == ENABLED

#include "vsf.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_netif.h"
#include "esp_netif_defaults.h"
#include "esp_netif_types.h"

#define __VSF_NETDRV_CLASS_INHERIT_NETLINK__
#include "component/tcpip/vsf_tcpip.h"
#include "component/tcpip/netdrv/driver/wpcap/vsf_netdrv_wpcap.h"

/*============================ LOCAL VARIABLES ===============================*/

static vk_netdrv_wpcap_t        __wpcap_netdrv;
static esp_netif_t             *__demo_netif = NULL;

/* lwIP's arch/cc.h in this port sets LWIP_PROVIDE_ERRNO, which asks lwIP to
 * supply its own 'int errno' symbol. Normally api/sockets.c would define it;
 * we exclude sockets.c from the build (host uses hostsock for AF_INET), so
 * anchor the storage here to satisfy the link. */
int errno;


/*============================ IMPLEMENTATION ================================*/

static void __netif_demo_ip_handler(void *arg, esp_event_base_t base,
                                    int32_t event_id, void *event_data)
{
    (void)arg;
    (void)base;

    if (event_id == IP_EVENT_ETH_GOT_IP || event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ev = (ip_event_got_ip_t *)event_data;
        uint32_t ip = ev->ip_info.ip.addr;
        uint32_t nm = ev->ip_info.netmask.addr;
        uint32_t gw = ev->ip_info.gw.addr;
        vsf_trace_info("netif-demo: GOT_IP %u.%u.%u.%u / %u.%u.%u.%u"
                       " gw %u.%u.%u.%u"VSF_TRACE_CFG_LINEEND,
                       (unsigned)((ip >>  0) & 0xff),
                       (unsigned)((ip >>  8) & 0xff),
                       (unsigned)((ip >> 16) & 0xff),
                       (unsigned)((ip >> 24) & 0xff),
                       (unsigned)((nm >>  0) & 0xff),
                       (unsigned)((nm >>  8) & 0xff),
                       (unsigned)((nm >> 16) & 0xff),
                       (unsigned)((nm >> 24) & 0xff),
                       (unsigned)((gw >>  0) & 0xff),
                       (unsigned)((gw >>  8) & 0xff),
                       (unsigned)((gw >> 16) & 0xff),
                       (unsigned)((gw >> 24) & 0xff));
    } else if (event_id == IP_EVENT_ETH_LOST_IP
            || event_id == IP_EVENT_STA_LOST_IP) {
        vsf_trace_info("netif-demo: LOST_IP"VSF_TRACE_CFG_LINEEND);
    }
}

void vsf_netif_demo_start(void)
{
    vsf_trace_info("netif-demo: start() entered"VSF_TRACE_CFG_LINEEND);
    if (__demo_netif != NULL) {
        return;                         /* idempotent */
    }

    /* The default event loop is created by vsf_espidf_init(); this call
     * short-circuits to ESP_ERR_INVALID_STATE on second invocation and is
     * therefore safe to drop in defensively. */
    vsf_trace_info("netif-demo: before event_loop_create_default"VSF_TRACE_CFG_LINEEND);
    (void)esp_event_loop_create_default();
    vsf_trace_info("netif-demo: after event_loop_create_default"VSF_TRACE_CFG_LINEEND);

    esp_err_t er = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                              __netif_demo_ip_handler, NULL);
    if (er != ESP_OK) {
        vsf_trace_error("netif-demo: handler_register failed: %d"
                        VSF_TRACE_CFG_LINEEND, (int)er);
        return;
    }

    /* Bind the wpcap netlink op; NULL name lets the driver's own
     * pcap_findalldevs_ex loop pick the first live interface. */
    __wpcap_netdrv.name = NULL;
    vk_netdrv_set_netlink_op((vk_netdrv_t *)&__wpcap_netdrv,
                             &vk_netdrv_wpcap_netlink_op, NULL);

    esp_netif_inherent_config_t eth_base = ESP_NETIF_INHERENT_DEFAULT_ETH();
    esp_netif_config_t cfg = {
        .base           = &eth_base,
        .driver_config  = NULL,
        .stack_config   = NULL,
    };
    __demo_netif = esp_netif_new(&cfg);
    if (__demo_netif == NULL) {
        vsf_trace_error("netif-demo: esp_netif_new failed"
                        VSF_TRACE_CFG_LINEEND);
        return;
    }

    er = esp_netif_attach_netdrv(__demo_netif, (struct vk_netdrv *)&__wpcap_netdrv);
    if (er != ESP_OK) {
        vsf_trace_error("netif-demo: attach_netdrv failed: %d"
                        VSF_TRACE_CFG_LINEEND, (int)er);
        esp_netif_destroy(__demo_netif);
        __demo_netif = NULL;
        return;
    }

    er = esp_netif_action_start(__demo_netif, NULL, 0, NULL);
    if (er != ESP_OK) {
        vsf_trace_error("netif-demo: action_start failed: %d"
                        VSF_TRACE_CFG_LINEEND, (int)er);
        esp_netif_destroy(__demo_netif);
        __demo_netif = NULL;
        return;
    }

    vsf_trace_info("netif-demo: started, waiting for DHCP..."
                   VSF_TRACE_CFG_LINEEND);
}

#endif      /* VSF_USE_ESPIDF && VSF_ESPIDF_CFG_USE_NETIF && VSF_NETDRV_USE_WPCAP */
/* EOF */
