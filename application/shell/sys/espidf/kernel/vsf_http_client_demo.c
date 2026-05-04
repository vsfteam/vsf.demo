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
 * esp_http_client smoke demo for the vc.espidf host build.
 *
 * Flow:
 *   netif_demo obtains DHCP IP via wpcap + lwIP
 *   -> vsf_http_client_demo_start() spawns a pthread
 *   -> pthread issues a plain HTTP GET against a public endpoint
 *   -> body and status are traced; connection is torn down
 *
 * The demo is intentionally HTTP-only (no TLS); HTTPS requires a CA store
 * which is out of scope for the first smoke run.
 */

#include "./vsf_usr_cfg.h"

#if     VSF_USE_ESPIDF == ENABLED                                              \
    &&  VSF_ESPIDF_CFG_USE_HTTP_CLIENT == ENABLED

#include "vsf.h"

#include <pthread.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_http_client.h"

/*============================ MACROS ========================================*/

#ifndef VSF_HTTP_CLIENT_DEMO_URL
#   define VSF_HTTP_CLIENT_DEMO_URL     "http://example.com/"
#endif

#ifndef VSF_HTTP_CLIENT_DEMO_TIMEOUT_MS
#   define VSF_HTTP_CLIENT_DEMO_TIMEOUT_MS  10000
#endif

#ifndef VSF_HTTP_CLIENT_DEMO_WAIT_DHCP_S
#   define VSF_HTTP_CLIENT_DEMO_WAIT_DHCP_S 5
#endif

/*============================ LOCAL VARIABLES ===============================*/

static volatile int __http_demo_running = 0;

/*============================ IMPLEMENTATION ================================*/

static esp_err_t __http_demo_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ON_CONNECTED:
        vsf_trace_info("http-demo: CONNECTED" VSF_TRACE_CFG_LINEEND);
        break;
    case HTTP_EVENT_HEADERS_SENT:
        vsf_trace_info("http-demo: HEADERS_SENT" VSF_TRACE_CFG_LINEEND);
        break;
    case HTTP_EVENT_ON_HEADER:
        if (evt->header_key && evt->header_value) {
            vsf_trace_info("http-demo: HDR %s: %s" VSF_TRACE_CFG_LINEEND,
                           evt->header_key, evt->header_value);
        }
        break;
    case HTTP_EVENT_ON_STATUS_CODE:
        vsf_trace_info("http-demo: STATUS %d" VSF_TRACE_CFG_LINEEND,
                       evt->data_len);
        break;
    case HTTP_EVENT_ON_DATA:
        vsf_trace_info("http-demo: DATA len=%d" VSF_TRACE_CFG_LINEEND,
                       evt->data_len);
        if (evt->data && evt->data_len > 0) {
            int n = evt->data_len > 128 ? 128 : evt->data_len;
            vsf_trace_buffer(VSF_TRACE_INFO, evt->data, n);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        vsf_trace_info("http-demo: FINISH" VSF_TRACE_CFG_LINEEND);
        break;
    case HTTP_EVENT_DISCONNECTED:
        vsf_trace_info("http-demo: DISCONNECTED" VSF_TRACE_CFG_LINEEND);
        break;
    case HTTP_EVENT_REDIRECT:
        vsf_trace_info("http-demo: REDIRECT" VSF_TRACE_CFG_LINEEND);
        break;
    case HTTP_EVENT_ERROR:
        vsf_trace_error("http-demo: ERROR" VSF_TRACE_CFG_LINEEND);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void __http_demo_run(void)
{
    esp_http_client_config_t cfg = { 0 };
    cfg.url             = VSF_HTTP_CLIENT_DEMO_URL;
    cfg.method          = HTTP_METHOD_GET;
    cfg.event_handler   = __http_demo_event_handler;
    cfg.timeout_ms      = VSF_HTTP_CLIENT_DEMO_TIMEOUT_MS;
    cfg.transport_type  = HTTP_TRANSPORT_OVER_TCP;

    vsf_trace_info("http-demo: GET %s" VSF_TRACE_CFG_LINEEND, cfg.url);

    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (client == NULL) {
        vsf_trace_error("http-demo: esp_http_client_init failed"
                        VSF_TRACE_CFG_LINEEND);
        return;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int     status = esp_http_client_get_status_code(client);
        int64_t clen   = esp_http_client_get_content_length(client);
        vsf_trace_info("http-demo: perform ok status=%d content_length=%lld"
                       VSF_TRACE_CFG_LINEEND,
                       status, (long long)clen);
    } else {
        vsf_trace_error("http-demo: perform failed err=%d"
                        VSF_TRACE_CFG_LINEEND, (int)err);
    }

    esp_http_client_cleanup(client);
}

static void *__http_demo_thread(void *arg)
{
    (void)arg;
    /* Give netif_demo a window to finish DHCP before we try to resolve
     * and connect. A production consumer would subscribe to IP_EVENT
     * got_ip and dispatch into its own worker; here we just sleep to
     * keep the smoke demo tiny. */
    sleep(VSF_HTTP_CLIENT_DEMO_WAIT_DHCP_S);
    __http_demo_run();
    __http_demo_running = 0;
    return NULL;
}

void vsf_http_client_demo_start(void)
{
    if (__http_demo_running) {
        return;                         /* already in-flight */
    }
    __http_demo_running = 1;

    pthread_t       tid;
    pthread_attr_t  attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 32768);

    int r = pthread_create(&tid, &attr, __http_demo_thread, NULL);
    pthread_attr_destroy(&attr);

    if (r != 0) {
        vsf_trace_error("http-demo: pthread_create failed: %d"
                        VSF_TRACE_CFG_LINEEND, r);
        __http_demo_running = 0;
        return;
    }
    pthread_detach(tid);
}

#endif  /* VSF_USE_ESPIDF && VSF_ESPIDF_CFG_USE_HTTP_CLIENT */
/* EOF */
