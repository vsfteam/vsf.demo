#include "vsf_cfg.h"

#if VSF_USE_LWIP == ENABLED
#   include "lwip/tcpip.h"

static bool __app_wifi_connected = false;

#   if LWIP_MDNS_RESPONDER
#       include "lwip/apps/mdns.h"

typedef struct app_mdns_service_record_t {
    vsf_dlist_node_t node;

    char *name;
    char *service;
    uint16_t port;
    bool is_tcp;
    uint8_t txt_num;
    uint8_t slot;
    char ** txt;
} app_mdns_service_record_t;
static vsf_dlist_t __app_mdns_service_record_list = { 0 };

static void __app_mdns_srv_txt(struct mdns_service *service, void *txt_usrdata)
{
    app_mdns_service_record_t *record = txt_usrdata;
    for (uint8_t i = 0; i < record->txt_num; i++) {
        mdns_resp_add_service_txtitem(service, record->txt[i], strlen(record->txt[i]));
    }
}

static void __app_mdns_add_service_from_record(app_mdns_service_record_t *record)
{
    extern struct netif * app_wifi_get_netif(void);
    struct netif *netif = app_wifi_get_netif();
    record->slot = mdns_resp_add_service(netif, record->name, record->service,
        record->is_tcp ? DNSSD_PROTO_TCP : DNSSD_PROTO_UDP, record->port,
        __app_mdns_srv_txt, record);
}

static void __app_mdns_cleanup_service(app_mdns_service_record_t *service)
{
    if (service->name != NULL) {
        vsf_heap_free(service->name);
        service->name = NULL;
    }
    if (service->service != NULL) {
        vsf_heap_free(service->service);
        service->service = NULL;
    }
    if (service->txt != NULL) {
        for (uint8_t i = 0; i < service->txt_num; i++) {
            if (service->txt[i] != NULL) {
                vsf_heap_free(service->txt[i]);
                service->txt[i] = NULL;
            }
        }
        vsf_heap_free(service->txt);
        service->txt = NULL;
        service->txt_num = 0;
    }
}
#   endif
#endif

void app_wifi_sta_on_connected(void)
{
#if VSF_USE_LWIP == ENABLED
    LOCK_TCPIP_CORE();
    if (__app_wifi_connected) {
        UNLOCK_TCPIP_CORE();
        return;
    } else {
        __app_wifi_connected = true;
    }

#   if LWIP_MDNS_RESPONDER
    __vsf_dlist_foreach_unsafe(app_mdns_service_record_t, node, &__app_mdns_service_record_list) {
        __app_mdns_add_service_from_record(_);
    }
#   endif
    UNLOCK_TCPIP_CORE();
#endif
}

void app_mdns_rename(const char *hostname)
{
#if VSF_USE_LWIP == ENABLED && LWIP_MDNS_RESPONDER
    LOCK_TCPIP_CORE();
        extern struct netif * app_wifi_get_netif(void);
        struct netif *netif = app_wifi_get_netif();
        // update lwip_mdns to the latest version if mdns_resp_rename_netif not exists
        mdns_resp_rename_netif(netif, hostname);
    UNLOCK_TCPIP_CORE();
#endif
}

void app_mdns_remove_service(void *service)
{
#if VSF_USE_LWIP == ENABLED && LWIP_MDNS_RESPONDER
    app_mdns_service_record_t *record = service;
    LOCK_TCPIP_CORE();
        extern struct netif * app_wifi_get_netif(void);
        struct netif *netif = app_wifi_get_netif();
        // update lwip_mdns to the latest version if mdns_resp_del_service not exists
        mdns_resp_del_service(netif, record->slot);
        vsf_dlist_remove(app_mdns_service_record_t, node, &__app_mdns_service_record_list, record);
    UNLOCK_TCPIP_CORE();

    __app_mdns_cleanup_service(record);
    vsf_heap_free(record);
#endif
}

int app_mdns_update_txt(void *service, const char **txt, uint8_t txt_num)
{
#if VSF_USE_LWIP == ENABLED && LWIP_MDNS_RESPONDER
    app_mdns_service_record_t *record = service;
    LOCK_TCPIP_CORE();
    if (record->txt != NULL) {
        for (uint8_t i = 0; i < record->txt_num; i++) {
            if (record->txt[i] != NULL) {
                vsf_heap_free(record->txt[i]);
                record->txt[i] = NULL;
            }
        }
        vsf_heap_free(record->txt);
        record->txt = NULL;
        record->txt_num = 0;
    }

    record->txt = vsf_heap_malloc(sizeof(char *) * txt_num);
    if (NULL == record->txt) {
        return -1;
    }
    memset(record->txt, 0, sizeof(char *) * txt_num);
    for (uint8_t i = 0; i < txt_num; i++) {
        record->txt[i] = vsf_heap_strdup(txt[i]);
        if (NULL == record->txt[i]) {
            return -1;
        }
    }
    UNLOCK_TCPIP_CORE();
#endif
    return 0;
}

void * app_mdns_update_service(void *record_orig, const char *name, const char *service, unsigned short port,
        int is_tcp, const char **txt, uint8_t txt_num)
{
#if VSF_USE_LWIP == ENABLED && LWIP_MDNS_RESPONDER
    app_mdns_service_record_t *record;

    LOCK_TCPIP_CORE();
    if (record_orig != NULL) {
        record = record_orig;
        __app_mdns_cleanup_service(record);
    } else {
        record = vsf_heap_malloc(sizeof(*record));
        if (NULL == name) {
            return NULL;
        }
    }

    vsf_dlist_init_node(app_mdns_service_record_t, node, record);
    record->name = vsf_heap_strdup(name);
    record->service = vsf_heap_strdup(service);
    if ((NULL == record->name) || (NULL == record->service)) {
        goto __cleanup_record_and_fail;
    }
    record->port = port;
    record->is_tcp = !!is_tcp;
    record->txt_num = txt_num;
    record->txt = vsf_heap_malloc(sizeof(char *) * txt_num);
    if ((NULL == record->name) || (NULL == record->service) || (NULL == record->txt)) {
        goto __cleanup_record_and_fail;
    }
    memset(record->txt, 0, sizeof(char *) * txt_num);
    for (uint8_t i = 0; i < txt_num; i++) {
        record->txt[i] = vsf_heap_strdup(txt[i]);
        if (NULL == record->txt[i]) {
            goto __cleanup_record_and_fail;
        }
    }

    vsf_dlist_queue_enqueue(app_mdns_service_record_t, node,
                    &__app_mdns_service_record_list, record);
    if (__app_wifi_connected) {
        __app_mdns_add_service_from_record(record);
    }
    UNLOCK_TCPIP_CORE();
    return record;

__cleanup_record_and_fail:
    if (record_orig != NULL) {
        app_mdns_remove_service(record);
    }
    __app_mdns_cleanup_service(record);
    vsf_heap_free(record);
    UNLOCK_TCPIP_CORE();
#endif
    return NULL;
}
