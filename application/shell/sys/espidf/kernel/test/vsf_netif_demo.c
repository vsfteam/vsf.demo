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
 * esp_netif IP event consumer demo.
 *
 * The environment layer (vsf_main.c) is responsible for creating and
 * starting the default netif (wpcap/lwIP on the Windows host). This
 * demo only shows how application code subscribes to IP_EVENT and
 * reacts to DHCP GOT_IP / LOST_IP, using strictly ESP-IDF public API.
 */

#include "vsf_usr_cfg.h"

#if     VSF_USE_ESPIDF == ENABLED                                              \
    &&  VSF_ESPIDF_CFG_USE_NETIF == ENABLED

#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_netif.h"
#include "esp_netif_defaults.h"
#include "esp_netif_types.h"
#include "esp_log.h"

/*============================ LOCAL VARIABLES ===============================*/

static const char *__netif_tag = "netif-demo";

static esp_netif_t *__demo_netif = NULL;
static esp_netif_iodriver_handle __demo_driver_handle = NULL;

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
        ESP_LOGI(__netif_tag, "GOT_IP %u.%u.%u.%u / %u.%u.%u.%u gw %u.%u.%u.%u",
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
        ESP_LOGI(__netif_tag, "LOST_IP");
    }
}

void vsf_netif_demo_set_handle(esp_netif_iodriver_handle handle)
{
    __demo_driver_handle = handle;
}

void vsf_netif_demo_start(void)
{
    ESP_LOGI(__netif_tag, "start() entered");
    if ((NULL == __demo_driver_handle) || (__demo_netif != NULL)) {
        return;
    }

    // The default event loop is created by vsf_espidf_init(); a second
    // call short-circuits to ESP_ERR_INVALID_STATE and is therefore safe
    // to drop in defensively.
    (void)esp_event_loop_create_default();

    esp_err_t er = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                              __netif_demo_ip_handler, NULL);
    if (er != ESP_OK) {
        ESP_LOGE(__netif_tag, "handler_register failed: %d", (int)er);
        return;
    }

    esp_netif_inherent_config_t eth_base = ESP_NETIF_INHERENT_DEFAULT_ETH();
    esp_netif_config_t cfg = {
        .base           = &eth_base,
        .driver_config  = NULL,
        .stack_config   = NULL,
    };
    __demo_netif = esp_netif_new(&cfg);
    if (__demo_netif == NULL) {
        ESP_LOGE(__netif_tag, "esp_netif_new failed");
        return;
    }

    er = esp_netif_attach(__demo_netif, __demo_driver_handle);
    if (er != ESP_OK) {
        ESP_LOGE(__netif_tag, "esp_netif_attach failed with %d", er);
        esp_netif_destroy(__demo_netif);
        __demo_netif = NULL;
        return;
    }

    er = esp_netif_action_start(__demo_netif, NULL, 0, NULL);
    if (er != ESP_OK) {
        ESP_LOGE(__netif_tag, "esp_netif_action_start failed with %d", er);
        esp_netif_destroy(__demo_netif);
        __demo_netif = NULL;
        return;
    }

    ESP_LOGI(__netif_tag, "handler registered, awaiting IP events");
}

#endif      /* VSF_USE_ESPIDF && VSF_ESPIDF_CFG_USE_NETIF */
/* EOF */
