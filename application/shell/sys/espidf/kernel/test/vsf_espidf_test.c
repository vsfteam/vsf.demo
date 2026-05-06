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
 * ESP-IDF shim-layer test suite.
 *
 * This translation unit is pure ESP-IDF-only code: it exercises the
 * public ESP-IDF APIs (esp_err / esp_log / esp_system / esp_timer /
 * esp_ringbuf / esp_heap_caps / esp_event / esp_partition / esp_nvs /
 * esp_flash / esp_vfs_littlefs / esp_vfs_fat) and the FreeRTOS shim.
 *
 * It assumes the runtime environment -- linux shim bring-up, root
 * filesystem mount, synthetic flash / partition table, network and
 * DHCP -- has already been prepared by the environment provider
 * (vsf_main.c). The only public entry point exported here is
 * app_main(), which is invoked from a pthread spawned by the
 * environment provider.
 *
 * Dependency:
 *   Submodule:
 *     vsf
 *       source/component/3rd-party/btstack/raw if VSF_USE_BTSTACK is enabled
 *       source/component/3rd-party/mbedtls/raw if VSF_USE_MBEDTLS is enabled
 *       source/component/3rd-party/PLOOC/raw
 */

/*============================ INCLUDES ======================================*/

#include <unistd.h>
#include <fcntl.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_ringbuf.h"
#include "esp_heap_caps.h"
#include "esp_event.h"
#include "esp_partition.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_littlefs.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"
#include "stream_buffer.h"
#include "message_buffer.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/*============================ MACROS ========================================*/

#define TEST_EXPECT(__cond)                                                 \
    do {                                                                    \
        __test_total++;                                                     \
        if (__cond) {                                                       \
            __test_pass++;                                                  \
            printf("  [PASS] " #__cond "\n");     \
        } else {                                                            \
            printf("  [FAIL] " #__cond " @%s:%d"                 \
                            "\n", __FILE__, __LINE__);     \
        }                                                                   \
    } while (0)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

#if VSF_ESPIDF_CFG_USE_USB_HOST == ENABLED
extern void __test_esp_usb_host(void);
#endif

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static int __test_total;
static int __test_pass;

// Test body runs on a FreeRTOS (shim) task spawned from app_main, so a
// vTaskDelay yields cleanly to the scheduler and lets expired timers /
// pending events fire between TEST_EXPECT steps.
#define __wait_ms(__ms)                 vTaskDelay(pdMS_TO_TICKS(__ms))

/*============================ IMPLEMENTATION ================================*/

static void __test_esp_err(void)
{
    printf("[esp_err] begin" "\n");

    // esp_err_to_name() on known codes
    TEST_EXPECT(strcmp(esp_err_to_name(ESP_OK),              "ESP_OK")              == 0);
    TEST_EXPECT(strcmp(esp_err_to_name(ESP_FAIL),            "ESP_FAIL")            == 0);
    TEST_EXPECT(strcmp(esp_err_to_name(ESP_ERR_NO_MEM),      "ESP_ERR_NO_MEM")      == 0);
    TEST_EXPECT(strcmp(esp_err_to_name(ESP_ERR_INVALID_ARG), "ESP_ERR_INVALID_ARG") == 0);
    TEST_EXPECT(strcmp(esp_err_to_name(ESP_ERR_TIMEOUT),     "ESP_ERR_TIMEOUT")     == 0);
    TEST_EXPECT(strcmp(esp_err_to_name(ESP_ERR_NOT_ALLOWED), "ESP_ERR_NOT_ALLOWED") == 0);

    // esp_err_to_name() on unknown code returns the generic "UNKNOWN ERROR"
    TEST_EXPECT(strcmp(esp_err_to_name(0x7fff), "UNKNOWN ERROR") == 0);

    // esp_err_to_name_r() known code: copies full name and NUL-terminates
    char buf[64];
    memset(buf, 0xaa, sizeof(buf));
    TEST_EXPECT(esp_err_to_name_r(ESP_ERR_TIMEOUT, buf, sizeof(buf)) == buf);
    TEST_EXPECT(strcmp(buf, "ESP_ERR_TIMEOUT") == 0);

    // esp_err_to_name_r() unknown code: renders hex
    memset(buf, 0, sizeof(buf));
    esp_err_to_name_r(0x7fff, buf, sizeof(buf));
    TEST_EXPECT(strstr(buf, "UNKNOWN ERROR") != NULL);
    TEST_EXPECT(strstr(buf, "7fff")          != NULL);

    // esp_err_to_name_r() small buffer: truncated but NUL-terminated
    char small[4];
    memset(small, 0xaa, sizeof(small));
    esp_err_to_name_r(ESP_OK, small, sizeof(small));
    TEST_EXPECT(small[sizeof(small) - 1] == '\0');
    TEST_EXPECT(strlen(small) == sizeof(small) - 1);

    // esp_err_to_name_r() invalid args
    TEST_EXPECT(esp_err_to_name_r(ESP_OK, NULL, 16) == NULL);
    TEST_EXPECT(esp_err_to_name_r(ESP_OK, buf,  0)  == NULL);

    // ESP_ERROR_CHECK_WITHOUT_ABORT(): must log and return the value,
    // without aborting (so tests after it still run).
    esp_err_t rc = ESP_ERROR_CHECK_WITHOUT_ABORT(ESP_ERR_TIMEOUT);
    TEST_EXPECT(rc == ESP_ERR_TIMEOUT);
    // Note: ESP_ERROR_CHECK(x) is intentionally NOT exercised here,
    // because on failure its internal trap terminates the whole run.

    printf("[esp_err] end" "\n");
}

// --- esp_log -------------------------------------------------------------

static int __log_hook_calls;
static int __log_hook_last_nchars;
static int __log_hook_vprintf(const char *fmt, va_list ap)
{
    char buf[128];
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    __log_hook_calls++;
    __log_hook_last_nchars = n;
    return n;
}

static void __test_esp_log(void)
{
    printf("[esp_log] begin" "\n");

    // Default level is INFO; register a tag and flip levels around.
    esp_log_level_set("TAG", ESP_LOG_WARN);
    TEST_EXPECT(esp_log_level_get("TAG") == ESP_LOG_WARN);

    esp_log_level_set("TAG", ESP_LOG_DEBUG);
    TEST_EXPECT(esp_log_level_get("TAG") == ESP_LOG_DEBUG);

    // Wildcard "*" shifts the default for unregistered tags.
    esp_log_level_set("*", ESP_LOG_ERROR);
    TEST_EXPECT(esp_log_level_get("NEW_TAG") == ESP_LOG_ERROR);
    // Registered tag keeps its own setting.
    TEST_EXPECT(esp_log_level_get("TAG") == ESP_LOG_DEBUG);

    // Restore a visible default for the remaining tests.
    esp_log_level_set("*", ESP_LOG_INFO);

    // Install a vprintf hook, verify it receives calls, then uninstall.
    vprintf_like_t prev = esp_log_set_vprintf(__log_hook_vprintf);
    TEST_EXPECT(prev == NULL);
    __log_hook_calls = 0;
    ESP_LOGE("HOOK", "value=%d", 42);
    TEST_EXPECT(__log_hook_calls == 1);
    TEST_EXPECT(__log_hook_last_nchars > 0);

    // A level below the effective filter must NOT invoke the hook.
    esp_log_level_set("HOOK", ESP_LOG_ERROR);
    __log_hook_calls = 0;
    ESP_LOGI("HOOK", "should be filtered");
    TEST_EXPECT(__log_hook_calls == 0);

    // Restore default backend.
    (void)esp_log_set_vprintf(NULL);

    // Timestamp is monotonic.
    uint32_t t0 = esp_log_timestamp();
    __wait_ms(2);
    uint32_t t1 = esp_log_timestamp();
    TEST_EXPECT(t1 >= t0);

    // One round of real output via the esp_log backend, visual only.
    ESP_LOGI("demo", "esp_log integration line, t=%lu ms",
             (unsigned long)esp_log_timestamp());

    printf("[esp_log] end" "\n");
}

// --- esp_system ----------------------------------------------------------

static void __test_esp_system(void)
{
    printf("[esp_system] begin" "\n");

    // Reset reason before any esp_restart() call.
    TEST_EXPECT(esp_reset_reason() == ESP_RST_POWERON);

    // IDF version string is provided by the environment.
    const char *ver = esp_get_idf_version();
    TEST_EXPECT(ver != NULL);
    TEST_EXPECT(ver[0] == 'v');
    TEST_EXPECT(strlen(ver) > 1);

    // chip_info must populate a well-formed record.
    esp_chip_info_t info;
    memset(&info, 0xAA, sizeof(info));
    esp_chip_info(&info);
    TEST_EXPECT(info.cores >= 1);

    // PRNG: two successive draws should differ (xorshift32 cannot return 0
    // and cannot stagnate on a non-zero seed).
    uint32_t r0 = esp_random();
    uint32_t r1 = esp_random();
    TEST_EXPECT(r0 != r1);

    // esp_fill_random writes the requested count without overflowing.
    uint8_t buf[17];
    memset(buf, 0, sizeof(buf));
    uint8_t guard = 0xCC;
    (void)guard;
    esp_fill_random(buf, sizeof(buf));
    int nonzero = 0;
    for (size_t i = 0; i < sizeof(buf); i++) {
        if (buf[i] != 0) { nonzero++; }
    }
    TEST_EXPECT(nonzero > 0);

    // Heap size: must be >=0; minimum should not exceed current.
    uint32_t free_now = esp_get_free_heap_size();
    uint32_t free_min = esp_get_minimum_free_heap_size();
    TEST_EXPECT(free_min <= free_now);

    printf("[esp_system] end" "\n");
}

// --- esp_timer -----------------------------------------------------------

static volatile int __timer_once_hits;
static volatile int __timer_periodic_hits;

static void __timer_once_cb(void *arg)
{
    (void)arg;
    __timer_once_hits++;
}

static void __timer_periodic_cb(void *arg)
{
    (void)arg;
    __timer_periodic_hits++;
}

static void __test_esp_timer(void)
{
    printf("[esp_timer] begin" "\n");

    // init is idempotent.
    TEST_EXPECT(esp_timer_init() == ESP_OK);
    TEST_EXPECT(esp_timer_init() == ESP_OK);

    // get_time advances.
    int64_t t0 = esp_timer_get_time();
    __wait_ms(2);
    int64_t t1 = esp_timer_get_time();
    TEST_EXPECT(t1 > t0);

    // Invalid args.
    TEST_EXPECT(esp_timer_create(NULL, NULL) == ESP_ERR_INVALID_ARG);

    // One-shot.
    esp_timer_handle_t h1 = NULL;
    esp_timer_create_args_t a1 = {
        .callback = __timer_once_cb,
        .arg      = NULL,
        .name     = "once",
    };
    TEST_EXPECT(esp_timer_create(&a1, &h1) == ESP_OK);
    TEST_EXPECT(h1 != NULL);
    __timer_once_hits = 0;
    TEST_EXPECT(esp_timer_start_once(h1, 5 * 1000) == ESP_OK);     // 5 ms
    TEST_EXPECT(esp_timer_is_active(h1));
    __wait_ms(100);
    TEST_EXPECT(__timer_once_hits == 1);
    TEST_EXPECT(!esp_timer_is_active(h1));
    TEST_EXPECT(esp_timer_delete(h1) == ESP_OK);

    // Periodic.
    esp_timer_handle_t h2 = NULL;
    esp_timer_create_args_t a2 = {
        .callback = __timer_periodic_cb,
        .arg      = NULL,
        .name     = "periodic",
    };
    TEST_EXPECT(esp_timer_create(&a2, &h2) == ESP_OK);
    __timer_periodic_hits = 0;
    TEST_EXPECT(esp_timer_start_periodic(h2, 10 * 1000) == ESP_OK);    // 10 ms
    __wait_ms(100);
    TEST_EXPECT(esp_timer_stop(h2) == ESP_OK);
    TEST_EXPECT(__timer_periodic_hits >= 3);
    // Double-stop returns INVALID_STATE (not running).
    TEST_EXPECT(esp_timer_stop(h2) == ESP_ERR_INVALID_STATE);
    TEST_EXPECT(esp_timer_delete(h2) == ESP_OK);

    printf("[esp_timer] periodic hits=%d" "\n",
                   __timer_periodic_hits);
    printf("[esp_timer] end" "\n");
}

// --- esp_ringbuf ---------------------------------------------------------

static void __test_esp_ringbuf(void)
{
    printf("[esp_ringbuf] begin" "\n");

    RingbufHandle_t rb = xRingbufferCreate(16, RINGBUF_TYPE_BYTEBUF);
    TEST_EXPECT(rb != NULL);
    TEST_EXPECT(xRingbufferGetMaxItemSize(rb) == 16);
    TEST_EXPECT(xRingbufferGetCurFreeSize(rb) == 16);
    TEST_EXPECT(xRingbufferGetCurFilledSize(rb) == 0);

    const char hello[] = "hello";      // 5 chars, no NUL included below
    TEST_EXPECT(xRingbufferSend(rb, hello, 5, 0) == pdTRUE);
    TEST_EXPECT(xRingbufferGetCurFilledSize(rb) == 5);
    TEST_EXPECT(xRingbufferGetCurFreeSize(rb) == 11);

    size_t got = 0;
    void *p = xRingbufferReceive(rb, &got, 0);
    TEST_EXPECT(p != NULL);
    TEST_EXPECT(got == 5);
    TEST_EXPECT(memcmp(p, hello, 5) == 0);
    vRingbufferReturnItem(rb, p);
    TEST_EXPECT(xRingbufferGetCurFilledSize(rb) == 0);

    // Fill until full; next send must fail.
    uint8_t pat[16];
    for (size_t i = 0; i < sizeof(pat); i++) { pat[i] = (uint8_t)i; }
    TEST_EXPECT(xRingbufferSend(rb, pat, sizeof(pat), 0) == pdTRUE);
    TEST_EXPECT(xRingbufferGetCurFreeSize(rb) == 0);
    TEST_EXPECT(xRingbufferSend(rb, pat, 1, 0) == pdFALSE);

    // Read in chunks with ReceiveUpTo to exercise wrap-around on next write.
    got = 0;
    p = xRingbufferReceiveUpTo(rb, &got, 0, 10);
    TEST_EXPECT(p != NULL);
    TEST_EXPECT(got == 10);
    TEST_EXPECT(memcmp(p, pat, 10) == 0);
    vRingbufferReturnItem(rb, p);

    // Now write 8 more bytes; should wrap the internal head across 0.
    uint8_t more[8] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7 };
    TEST_EXPECT(xRingbufferSend(rb, more, sizeof(more), 0) == pdTRUE);
    TEST_EXPECT(xRingbufferGetCurFilledSize(rb) == 6 + 8);

    // Drain fully; verify order preserves (last 6 of pat, then 8 of more).
    got = 0;
    p = xRingbufferReceive(rb, &got, 0);
    TEST_EXPECT(p != NULL);
    TEST_EXPECT(got == 14);
    TEST_EXPECT(memcmp(p, pat + 10, 6) == 0);
    TEST_EXPECT(memcmp((uint8_t *)p + 6, more, 8) == 0);
    vRingbufferReturnItem(rb, p);

    // Empty receive returns NULL.
    got = 99;
    p = xRingbufferReceive(rb, &got, 0);
    TEST_EXPECT(p == NULL);

    vRingbufferDelete(rb);
    printf("[esp_ringbuf] end" "\n");
}

// --- FreeRTOS shim -------------------------------------------------------

static volatile int __frt_worker_hits;
static void __frt_worker(void *arg)
{
    int n = (int)(intptr_t)arg;
    for (int i = 0; i < n; i++) {
        __frt_worker_hits++;
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    vTaskDelete(NULL);
}

static void __test_esp_heap_caps(void)
{
    printf("[heap_caps] begin" "\n");

    // Basic malloc/free round trip on DEFAULT caps.
    void *p = heap_caps_malloc(128, MALLOC_CAP_DEFAULT);
    TEST_EXPECT(p != NULL);
    heap_caps_free(p);

    // 8BIT and INTERNAL caps succeed on a flat-pool backend.
    p = heap_caps_malloc(64, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    TEST_EXPECT(p != NULL);
    heap_caps_free(p);

    // size == 0 yields NULL.
    TEST_EXPECT(heap_caps_malloc(0, MALLOC_CAP_DEFAULT) == NULL);

    // MALLOC_CAP_INVALID is rejected.
    TEST_EXPECT(heap_caps_malloc(16, MALLOC_CAP_INVALID) == NULL);

    // calloc zero-initialises.
    uint8_t *cp = (uint8_t *)heap_caps_calloc(8, 4, MALLOC_CAP_DEFAULT);
    TEST_EXPECT(cp != NULL);
    if (cp != NULL) {
        bool all_zero = true;
        for (int i = 0; i < 32; i++) {
            if (cp[i] != 0) { all_zero = false; break; }
        }
        TEST_EXPECT(all_zero);
        heap_caps_free(cp);
    }

    // realloc grows and preserves leading bytes.
    uint8_t *rp = (uint8_t *)heap_caps_malloc(16, MALLOC_CAP_DEFAULT);
    TEST_EXPECT(rp != NULL);
    if (rp != NULL) {
        for (int i = 0; i < 16; i++) { rp[i] = (uint8_t)i; }
        uint8_t *rp2 = (uint8_t *)heap_caps_realloc(rp, 64, MALLOC_CAP_DEFAULT);
        TEST_EXPECT(rp2 != NULL);
        if (rp2 != NULL) {
            bool preserved = true;
            for (int i = 0; i < 16; i++) {
                if (rp2[i] != (uint8_t)i) { preserved = false; break; }
            }
            TEST_EXPECT(preserved);
            heap_caps_free(rp2);
        } else {
            heap_caps_free(rp);
        }
    }

    // aligned_alloc returns a pointer aligned to the requested boundary.
    void *ap = heap_caps_aligned_alloc(64, 200, MALLOC_CAP_DEFAULT);
    TEST_EXPECT(ap != NULL);
    TEST_EXPECT(((uintptr_t)ap & 63u) == 0);
    heap_caps_aligned_free(ap);

    // Non-power-of-two alignment is rejected.
    TEST_EXPECT(heap_caps_aligned_alloc(48, 32, MALLOC_CAP_DEFAULT) == NULL);

    // Stats bridge: total = free + used (within our process view).
    size_t total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    size_t free_sz = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    TEST_EXPECT(total >= free_sz);

    // minimum_free_size tracks historical low-water (all - max_used_size).
    // It MUST be <= current free size and >0 if the heap has served any
    // allocation in this run (which it has by the time we get here).
    size_t min_free = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    TEST_EXPECT(min_free <= free_sz);
    TEST_EXPECT(min_free <= total);

    // largest_free_block is exact (freelist-scanned). It MUST be <= total
    // free (a single block can't exceed aggregate free), and strictly > 0
    // whenever free_sz > 0.
    size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    TEST_EXPECT(largest <= free_sz);
    TEST_EXPECT((free_sz == 0) || (largest > 0));

    // heap_caps_get_info populates free/used/largest consistently.
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    TEST_EXPECT(info.total_free_bytes == free_sz);
    TEST_EXPECT(info.minimum_free_bytes == min_free);
    TEST_EXPECT(info.largest_free_block == largest);

    // Integrity helpers return sane defaults.
    TEST_EXPECT(heap_caps_check_integrity_all(false) == true);
    TEST_EXPECT(heap_caps_check_integrity(MALLOC_CAP_INVALID, false) == false);

    printf("[heap_caps] end" "\n");
}

static void __test_freertos(void)
{
    printf("[freertos] begin" "\n");

    // pdMS_TO_TICKS is identity in this shim (1 tick == 1 ms).
    TEST_EXPECT(pdMS_TO_TICKS(0)   == 0);
    TEST_EXPECT(pdMS_TO_TICKS(1)   == 1);
    TEST_EXPECT(pdMS_TO_TICKS(100) == 100);

    // xTaskGetTickCount advances after a delay.
    TickType_t t0 = xTaskGetTickCount();
    vTaskDelay(pdMS_TO_TICKS(5));
    TickType_t t1 = xTaskGetTickCount();
    TEST_EXPECT((TickType_t)(t1 - t0) >= 4);

    // taskYIELD compiles and returns cleanly.
    taskYIELD();

    // Calling task handle must be non-NULL inside a FreeRTOS task.
    TaskHandle_t self = xTaskGetCurrentTaskHandle();
    TEST_EXPECT(self != NULL);

    // xTaskCreate spawns a worker; we wait for it via vTaskDelay.
    __frt_worker_hits = 0;
    TaskHandle_t h = NULL;
    BaseType_t rc = xTaskCreate(__frt_worker, "worker", 4096,
                                (void *)(intptr_t)5, 0, &h);
    TEST_EXPECT(rc == pdPASS);
    TEST_EXPECT(h  != NULL);
    vTaskDelay(pdMS_TO_TICKS(60));
    TEST_EXPECT(__frt_worker_hits == 5);

    // Invalid args return pdFAIL.
    TEST_EXPECT(xTaskCreate(NULL, "bad", 4096, NULL, 0, NULL) == pdFAIL);

    printf("[freertos] end" "\n");
}

static void __test_freertos_queue(void)
{
    printf("[freertos_queue] begin" "\n");

    // Invalid args on create.
    TEST_EXPECT(xQueueCreate(0, sizeof(uint32_t)) == NULL);
    TEST_EXPECT(xQueueCreate(4, 0) == NULL);

    // 4-slot queue of uint32_t items.
    QueueHandle_t q = xQueueCreate(4, sizeof(uint32_t));
    TEST_EXPECT(q != NULL);
    TEST_EXPECT(uxQueueMessagesWaiting(q) == 0);
    TEST_EXPECT(uxQueueSpacesAvailable(q) == 4);

    // Fill the queue.
    for (uint32_t i = 1; i <= 4; i++) {
        TEST_EXPECT(xQueueSend(q, &i, 0) == pdTRUE);
    }
    TEST_EXPECT(uxQueueMessagesWaiting(q) == 4);
    TEST_EXPECT(uxQueueSpacesAvailable(q) == 0);

    // Non-blocking send on a full queue must fail.
    uint32_t drop = 99;
    TEST_EXPECT(xQueueSend(q, &drop, 0) == pdFAIL);

    // FIFO order out.
    for (uint32_t exp = 1; exp <= 4; exp++) {
        uint32_t v = 0;
        TEST_EXPECT(xQueueReceive(q, &v, 0) == pdTRUE);
        TEST_EXPECT(v == exp);
    }
    TEST_EXPECT(uxQueueMessagesWaiting(q) == 0);

    // Non-blocking receive on empty must fail.
    uint32_t tmp = 0;
    TEST_EXPECT(xQueueReceive(q, &tmp, 0) == pdFAIL);

    // Ring wrap-around: push/pop several times beyond capacity.
    for (uint32_t round = 0; round < 3; round++) {
        uint32_t a = 10 + round, b = 20 + round, c = 30 + round;
        TEST_EXPECT(xQueueSend(q, &a, 0) == pdTRUE);
        TEST_EXPECT(xQueueSend(q, &b, 0) == pdTRUE);
        TEST_EXPECT(xQueueSend(q, &c, 0) == pdTRUE);
        uint32_t v;
        TEST_EXPECT(xQueueReceive(q, &v, 0) == pdTRUE); TEST_EXPECT(v == a);
        TEST_EXPECT(xQueueReceive(q, &v, 0) == pdTRUE); TEST_EXPECT(v == b);
        TEST_EXPECT(xQueueReceive(q, &v, 0) == pdTRUE); TEST_EXPECT(v == c);
    }
    TEST_EXPECT(uxQueueMessagesWaiting(q) == 0);

    // xQueueReset clears any pending items and restores capacity.
    uint32_t leftover = 77;
    TEST_EXPECT(xQueueSend(q, &leftover, 0) == pdTRUE);
    TEST_EXPECT(uxQueueMessagesWaiting(q) == 1);
    TEST_EXPECT(xQueueReset(q) == pdPASS);
    TEST_EXPECT(uxQueueMessagesWaiting(q) == 0);
    TEST_EXPECT(uxQueueSpacesAvailable(q) == 4);

    // Struct payload: prove memcpy handles arbitrary item size.
    struct __payload { uint32_t seq; char tag[8]; } p_in = { 0 }, p_out = { 0 };
    QueueHandle_t q2 = xQueueCreate(2, sizeof(p_in));
    TEST_EXPECT(q2 != NULL);
    p_in.seq = 0xBEEF;
    memcpy(p_in.tag, "hello", 6);
    TEST_EXPECT(xQueueSend(q2, &p_in, 0) == pdTRUE);
    TEST_EXPECT(xQueueReceive(q2, &p_out, 0) == pdTRUE);
    TEST_EXPECT(p_out.seq == 0xBEEF);
    TEST_EXPECT(memcmp(p_out.tag, "hello", 6) == 0);

    // ISR API smoke test: only parameter-validation paths are exercised
    // here because xQueueSendFromISR / xQueueReceiveFromISR are required
    // by the FreeRTOS contract to be called from a real ISR context. On
    // this host build those entries defer the transfer to a task-side
    // handler, so calling them on a task is a semantic misuse (not a
    // supported scenario). Actual data-path coverage for the ISR
    // variants belongs in a test that fires a real interrupt source.
    BaseType_t woken = pdFALSE;
    TEST_EXPECT(xQueueSendFromISR(NULL, &p_in, &woken) == pdFAIL);
    TEST_EXPECT(woken == pdFALSE);
    TEST_EXPECT(xQueueReceiveFromISR(NULL, &p_out, &woken) == pdFAIL);
    TEST_EXPECT(woken == pdFALSE);
    TEST_EXPECT(xQueueSendFromISR(q2, NULL, &woken) == pdFAIL);
    TEST_EXPECT(xQueueReceiveFromISR(q2, NULL, &woken) == pdFAIL);

    // Invalid args on the task-side API surface as pdFAIL.
    TEST_EXPECT(xQueueSend(NULL, &p_in, 0) == pdFAIL);
    TEST_EXPECT(xQueueReceive(NULL, &p_out, 0) == pdFAIL);
    TEST_EXPECT(xQueueSend(q2, NULL, 0) == pdFAIL);

    // Cleanup. vQueueDelete(NULL) must be a no-op.
    vQueueDelete(NULL);
    vQueueDelete(q2);
    vQueueDelete(q);

    printf("[freertos_queue] end" "\n");
}

// --- FreeRTOS shim: semaphores / mutex ---------------------------------

static volatile int __frt_sem_producer_hits;
static SemaphoreHandle_t __frt_sem_shared;
static void __frt_sem_producer(void *arg)
{
    int n = (int)(intptr_t)arg;
    for (int i = 0; i < n; i++) {
        __frt_sem_producer_hits++;
        xSemaphoreGive(__frt_sem_shared);
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

static void __test_freertos_semphr(void)
{
    printf("[freertos_semphr] begin" "\n");

    // Binary sem starts empty: non-blocking take times out.
    SemaphoreHandle_t b = xSemaphoreCreateBinary();
    TEST_EXPECT(b != NULL);
    TEST_EXPECT(xSemaphoreTake(b, pdMS_TO_TICKS(5)) == pdFAIL);
    // Give, then Take must succeed.
    TEST_EXPECT(xSemaphoreGive(b) == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(b, pdMS_TO_TICKS(50)) == pdTRUE);
    vSemaphoreDelete(b);

    // Counting sem: initial=2, max=3. Take twice, then block on third.
    SemaphoreHandle_t c = xSemaphoreCreateCounting(3, 2);
    TEST_EXPECT(c != NULL);
    TEST_EXPECT(xSemaphoreTake(c, 0) == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(c, 0) == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(c, pdMS_TO_TICKS(5)) == pdFAIL);
    TEST_EXPECT(xSemaphoreGive(c) == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(c, pdMS_TO_TICKS(50)) == pdTRUE);
    vSemaphoreDelete(c);

    // Invalid-arg on counting create.
    TEST_EXPECT(xSemaphoreCreateCounting(0, 0) == NULL);
    TEST_EXPECT(xSemaphoreCreateCounting(2, 3) == NULL); // initial > max

    // Mutex: take, then give, then re-take.
    SemaphoreHandle_t m = xSemaphoreCreateMutex();
    TEST_EXPECT(m != NULL);
    TEST_EXPECT(xSemaphoreTake(m, pdMS_TO_TICKS(50)) == pdTRUE);
    TEST_EXPECT(xSemaphoreGive(m) == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(m, pdMS_TO_TICKS(50)) == pdTRUE);
    TEST_EXPECT(xSemaphoreGive(m) == pdTRUE);
    vSemaphoreDelete(m);

    // Producer/consumer via a shared binary sem across tasks.
    __frt_sem_producer_hits = 0;
    __frt_sem_shared = xSemaphoreCreateCounting(8, 0);
    TEST_EXPECT(__frt_sem_shared != NULL);
    TaskHandle_t h = NULL;
    TEST_EXPECT(xTaskCreate(__frt_sem_producer, "semprd", 4096,
                            (void *)(intptr_t)4, 0, &h) == pdPASS);
    int got = 0;
    for (int i = 0; i < 4; i++) {
        if (xSemaphoreTake(__frt_sem_shared, pdMS_TO_TICKS(100)) == pdTRUE) {
            got++;
        }
    }
    TEST_EXPECT(got == 4);
    TEST_EXPECT(__frt_sem_producer_hits == 4);
    vSemaphoreDelete(__frt_sem_shared);
    __frt_sem_shared = NULL;

    // Invalid-arg surface.
    TEST_EXPECT(xSemaphoreTake(NULL, 0) == pdFAIL);
    TEST_EXPECT(xSemaphoreGive(NULL) == pdFAIL);
    vSemaphoreDelete(NULL);

    printf("[freertos_semphr] end" "\n");
}

// --- FreeRTOS shim: event groups ---------------------------------------

static EventGroupHandle_t __frt_eg_shared;
static void __frt_eg_setter(void *arg)
{
    EventBits_t bits = (EventBits_t)(uintptr_t)arg;
    vTaskDelay(pdMS_TO_TICKS(10));
    xEventGroupSetBits(__frt_eg_shared, bits);
}

static void __test_freertos_event_groups(void)
{
    printf("[freertos_event_groups] begin" "\n");

    EventGroupHandle_t eg = xEventGroupCreate();
    TEST_EXPECT(eg != NULL);
    TEST_EXPECT(xEventGroupGetBits(eg) == 0);

    // Set / clear / get.
    EventBits_t prev = xEventGroupSetBits(eg, 0x05);
    TEST_EXPECT((prev & 0x05) == 0x05);
    TEST_EXPECT(xEventGroupGetBits(eg) == 0x05);
    TEST_EXPECT(xEventGroupClearBits(eg, 0x01) == 0x05);
    TEST_EXPECT(xEventGroupGetBits(eg) == 0x04);

    // WaitBits: already satisfied -> returns immediately, clearOnExit clears.
    xEventGroupSetBits(eg, 0x03);            // value = 0x07
    EventBits_t got = xEventGroupWaitBits(eg, 0x03, pdTRUE, pdTRUE,
                                          pdMS_TO_TICKS(50));
    TEST_EXPECT((got & 0x03) == 0x03);
    TEST_EXPECT(xEventGroupGetBits(eg) == 0x04);

    // Timeout path: wait for a bit that is not set.
    got = xEventGroupWaitBits(eg, 0x10, pdFALSE, pdTRUE,
                              pdMS_TO_TICKS(10));
    TEST_EXPECT((got & 0x10) == 0);

    // Cross-task: a helper task sets bit 0x08 after 10ms; we block-wait.
    __frt_eg_shared = eg;
    TEST_EXPECT(xEventGroupClearBits(eg, 0xFF) == 0x04);
    TaskHandle_t h = NULL;
    TEST_EXPECT(xTaskCreate(__frt_eg_setter, "egset", 4096,
                            (void *)(uintptr_t)0x08, 0, &h) == pdPASS);
    got = xEventGroupWaitBits(eg, 0x08, pdTRUE, pdFALSE,
                              pdMS_TO_TICKS(200));
    TEST_EXPECT((got & 0x08) == 0x08);
    TEST_EXPECT((xEventGroupGetBits(eg) & 0x08) == 0);  // cleared on exit

    vEventGroupDelete(eg);
    __frt_eg_shared = NULL;

    // Invalid-arg surface.
    TEST_EXPECT(xEventGroupGetBits(NULL) == 0);
    vEventGroupDelete(NULL);

    printf("[freertos_event_groups] end" "\n");
}

// --- FreeRTOS shim: task notifications ---------------------------------

static TaskHandle_t __frt_notify_target;
static volatile uint32_t __frt_notify_take_value;
static volatile int      __frt_notify_take_returned;
static volatile uint32_t __frt_notify_wait_value;
static volatile int      __frt_notify_wait_rc;

static void __frt_notify_take_worker(void *arg)
{
    (void)arg;
    __frt_notify_target = xTaskGetCurrentTaskHandle();
    // Block until notified.
    __frt_notify_take_value    = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500));
    __frt_notify_take_returned = 1;
}

static void __frt_notify_wait_worker(void *arg)
{
    (void)arg;
    __frt_notify_target = xTaskGetCurrentTaskHandle();
    uint32_t v = 0;
    BaseType_t rc = xTaskNotifyWait(0x00, 0xFFFFFFFF, &v,
                                    pdMS_TO_TICKS(500));
    __frt_notify_wait_value = v;
    __frt_notify_wait_rc    = (rc == pdPASS) ? 1 : 0;
}

static void __test_freertos_notify(void)
{
    printf("[freertos_notify] begin" "\n");

    // --- xTaskNotifyGive / ulTaskNotifyTake ---
    __frt_notify_target         = NULL;
    __frt_notify_take_value     = 0;
    __frt_notify_take_returned  = 0;
    TaskHandle_t h = NULL;
    TEST_EXPECT(xTaskCreate(__frt_notify_take_worker, "ntake", 4096,
                            NULL, 0, &h) == pdPASS);
    // Wait for the worker to park itself.
    for (int i = 0; i < 20 && __frt_notify_target == NULL; i++) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    TEST_EXPECT(__frt_notify_target != NULL);
    // Give twice before the worker claims -> count must merge to 2.
    TEST_EXPECT(xTaskNotifyGive(__frt_notify_target) == pdPASS);
    TEST_EXPECT(xTaskNotifyGive(__frt_notify_target) == pdPASS);
    // Wait for the worker to wake and record the value.
    for (int i = 0; i < 40 && !__frt_notify_take_returned; i++) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    TEST_EXPECT(__frt_notify_take_returned == 1);
    TEST_EXPECT(__frt_notify_take_value    == 2);

    // --- xTaskNotify(eSetBits) + xTaskNotifyWait ---
    __frt_notify_target      = NULL;
    __frt_notify_wait_value  = 0;
    __frt_notify_wait_rc     = 0;
    h = NULL;
    TEST_EXPECT(xTaskCreate(__frt_notify_wait_worker, "nwait", 4096,
                            NULL, 0, &h) == pdPASS);
    for (int i = 0; i < 20 && __frt_notify_target == NULL; i++) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    TEST_EXPECT(__frt_notify_target != NULL);
    TEST_EXPECT(xTaskNotify(__frt_notify_target, 0xA5, eSetBits) == pdPASS);
    for (int i = 0; i < 40 && !__frt_notify_wait_rc; i++) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    TEST_EXPECT(__frt_notify_wait_rc    == 1);
    TEST_EXPECT((__frt_notify_wait_value & 0xA5) == 0xA5);

    // --- eSetValueWithoutOverwrite: second notify must be rejected while
    //     a prior value is still pending ---
    __frt_notify_target         = NULL;
    __frt_notify_take_value     = 0;
    __frt_notify_take_returned  = 0;
    h = NULL;
    TEST_EXPECT(xTaskCreate(__frt_notify_take_worker, "novr", 4096,
                            NULL, 0, &h) == pdPASS);
    for (int i = 0; i < 20 && __frt_notify_target == NULL; i++) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    TEST_EXPECT(__frt_notify_target != NULL);
    TEST_EXPECT(xTaskNotify(__frt_notify_target, 42,
                            eSetValueWithoutOverwrite) == pdPASS);
    TEST_EXPECT(xTaskNotify(__frt_notify_target, 99,
                            eSetValueWithoutOverwrite) == pdFAIL);
    for (int i = 0; i < 40 && !__frt_notify_take_returned; i++) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    TEST_EXPECT(__frt_notify_take_returned == 1);
    TEST_EXPECT(__frt_notify_take_value    == 42);

    // Invalid-arg surface (no target).
    TEST_EXPECT(xTaskNotify(NULL, 0, eSetBits) == pdFAIL);
    TEST_EXPECT(xTaskNotifyGive(NULL) == pdFAIL);

    printf("[freertos_notify] end" "\n");
}

// --- FreeRTOS timers ---------------------------------------------------

static volatile int __frt_tmr_one_shot_hits;
static volatile int __frt_tmr_periodic_hits;
static volatile int __frt_tmr_stop_hits;
static volatile int __frt_tmr_period_hits;
static volatile void *__frt_tmr_last_id;

static void __frt_tmr_one_shot_cb(TimerHandle_t xTimer)
{
    __frt_tmr_last_id = pvTimerGetTimerID(xTimer);
    __frt_tmr_one_shot_hits++;
}

static void __frt_tmr_periodic_cb(TimerHandle_t xTimer)
{
    (void)xTimer;
    __frt_tmr_periodic_hits++;
}

static void __frt_tmr_stop_cb(TimerHandle_t xTimer)
{
    // After the first hit the callback self-stops; subsequent ticks must
    // not observe additional hits.
    __frt_tmr_stop_hits++;
    xTimerStop(xTimer, 0);
}

static void __frt_tmr_period_cb(TimerHandle_t xTimer)
{
    (void)xTimer;
    __frt_tmr_period_hits++;
}

static void __test_freertos_timers(void)
{
    printf("[freertos_timers] begin" "\n");

    // --- One-shot timer: fires exactly once ---
    __frt_tmr_one_shot_hits = 0;
    __frt_tmr_last_id       = NULL;
    int sentinel = 0;
    TimerHandle_t t1 = xTimerCreate("one", pdMS_TO_TICKS(20), pdFALSE,
                                    &sentinel, __frt_tmr_one_shot_cb);
    TEST_EXPECT(t1 != NULL);
    TEST_EXPECT(xTimerIsTimerActive(t1) == pdFALSE);
    TEST_EXPECT(xTimerGetPeriod(t1)      == pdMS_TO_TICKS(20));
    TEST_EXPECT(pvTimerGetTimerID(t1)    == &sentinel);
    TEST_EXPECT(xTimerStart(t1, 0)       == pdPASS);
    TEST_EXPECT(xTimerIsTimerActive(t1)  == pdTRUE);
    for (int i = 0; i < 40 && __frt_tmr_one_shot_hits == 0; i++) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    TEST_EXPECT(__frt_tmr_one_shot_hits == 1);
    TEST_EXPECT(__frt_tmr_last_id       == &sentinel);
    // Give it a bit more time to prove it really doesn't re-fire.
    vTaskDelay(pdMS_TO_TICKS(60));
    TEST_EXPECT(__frt_tmr_one_shot_hits == 1);
    TEST_EXPECT(xTimerIsTimerActive(t1) == pdFALSE);
    TEST_EXPECT(xTimerDelete(t1, 0)     == pdPASS);

    // --- Auto-reload timer: fires repeatedly until stopped ---
    __frt_tmr_periodic_hits = 0;
    TimerHandle_t t2 = xTimerCreate("per", pdMS_TO_TICKS(10), pdTRUE,
                                    NULL, __frt_tmr_periodic_cb);
    TEST_EXPECT(t2 != NULL);
    TEST_EXPECT(xTimerStart(t2, 0) == pdPASS);
    vTaskDelay(pdMS_TO_TICKS(60));
    TEST_EXPECT(xTimerStop(t2, 0) == pdPASS);
    int fired = __frt_tmr_periodic_hits;
    // With ~50ms of effective runtime and 10ms period we expect at
    // least 3 hits (allowing generous slack for scheduler jitter).
    TEST_EXPECT(fired >= 3);
    TEST_EXPECT(xTimerIsTimerActive(t2) == pdFALSE);
    // No more hits after stop.
    vTaskDelay(pdMS_TO_TICKS(40));
    TEST_EXPECT(__frt_tmr_periodic_hits == fired);
    TEST_EXPECT(xTimerDelete(t2, 0) == pdPASS);

    // --- Callback-initiated stop on an auto-reload timer ---
    __frt_tmr_stop_hits = 0;
    TimerHandle_t t3 = xTimerCreate("self", pdMS_TO_TICKS(10), pdTRUE,
                                    NULL, __frt_tmr_stop_cb);
    TEST_EXPECT(t3 != NULL);
    TEST_EXPECT(xTimerStart(t3, 0) == pdPASS);
    vTaskDelay(pdMS_TO_TICKS(80));
    TEST_EXPECT(__frt_tmr_stop_hits     == 1);
    TEST_EXPECT(xTimerIsTimerActive(t3) == pdFALSE);
    TEST_EXPECT(xTimerDelete(t3, 0) == pdPASS);

    // --- xTimerChangePeriod re-arms with the new period ---
    __frt_tmr_period_hits = 0;
    TimerHandle_t t4 = xTimerCreate("chp", pdMS_TO_TICKS(500), pdFALSE,
                                    NULL, __frt_tmr_period_cb);
    TEST_EXPECT(t4 != NULL);
    TEST_EXPECT(xTimerStart(t4, 0) == pdPASS);
    // Shrink to 10ms. Fire must happen within ~a few tens of ms, not 500ms.
    TEST_EXPECT(xTimerChangePeriod(t4, pdMS_TO_TICKS(10), 0) == pdPASS);
    TEST_EXPECT(xTimerGetPeriod(t4) == pdMS_TO_TICKS(10));
    for (int i = 0; i < 20 && __frt_tmr_period_hits == 0; i++) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    TEST_EXPECT(__frt_tmr_period_hits == 1);
    TEST_EXPECT(xTimerDelete(t4, 0) == pdPASS);

    // --- vTimerSetTimerID round-trip ---
    int a = 1, b = 2;
    TimerHandle_t t5 = xTimerCreate("id", pdMS_TO_TICKS(1000), pdFALSE,
                                    &a, __frt_tmr_one_shot_cb);
    TEST_EXPECT(t5 != NULL);
    TEST_EXPECT(pvTimerGetTimerID(t5) == &a);
    vTimerSetTimerID(t5, &b);
    TEST_EXPECT(pvTimerGetTimerID(t5) == &b);
    TEST_EXPECT(xTimerDelete(t5, 0) == pdPASS);

    // Invalid-arg surface.
    TEST_EXPECT(xTimerCreate("bad", 0, pdFALSE, NULL,
                             __frt_tmr_one_shot_cb) == NULL);
    TEST_EXPECT(xTimerCreate("bad", pdMS_TO_TICKS(10), pdFALSE, NULL,
                             NULL) == NULL);
    TEST_EXPECT(xTimerStart(NULL, 0) == pdFAIL);
    TEST_EXPECT(xTimerStop(NULL, 0)  == pdFAIL);
    TEST_EXPECT(xTimerDelete(NULL, 0) == pdFAIL);
    TEST_EXPECT(pvTimerGetTimerID(NULL) == NULL);
    TEST_EXPECT(xTimerGetPeriod(NULL)   == 0);

    printf("[freertos_timers] end" "\n");
}

// --- FreeRTOS stream_buffer + message_buffer -------------------------

static StreamBufferHandle_t __frt_sb_producer_handle;
static MessageBufferHandle_t __frt_mb_producer_handle;

static void __frt_sb_producer(void *arg)
{
    (void)arg;
    // Block until the driver is ready, then push "HELLO" in two chunks.
    vTaskDelay(pdMS_TO_TICKS(10));
    const char *p1 = "HEL";
    const char *p2 = "LO";
    (void)xStreamBufferSend(__frt_sb_producer_handle, p1, 3, portMAX_DELAY);
    (void)xStreamBufferSend(__frt_sb_producer_handle, p2, 2, portMAX_DELAY);
    vTaskDelete(NULL);
}

static void __frt_mb_producer(void *arg)
{
    (void)arg;
    vTaskDelay(pdMS_TO_TICKS(10));
    const char *m1 = "abc";
    const char *m2 = "defghi";
    (void)xMessageBufferSend(__frt_mb_producer_handle, m1, 3, portMAX_DELAY);
    (void)xMessageBufferSend(__frt_mb_producer_handle, m2, 6, portMAX_DELAY);
    vTaskDelete(NULL);
}

static void __test_freertos_stream_buffer(void)
{
    printf("[freertos_stream_buffer] begin" "\n");

    // --- Basic single-task non-blocking send/receive ---
    StreamBufferHandle_t sb = xStreamBufferCreate(32, 1);
    TEST_EXPECT(sb != NULL);
    TEST_EXPECT(xStreamBufferIsEmpty(sb) == pdTRUE);
    TEST_EXPECT(xStreamBufferIsFull(sb)  == pdFALSE);
    TEST_EXPECT(xStreamBufferBytesAvailable(sb)  == 0);
    TEST_EXPECT(xStreamBufferSpacesAvailable(sb) == 32);

    const char *msg = "WORLD";
    TEST_EXPECT(xStreamBufferSend(sb, msg, 5, 0) == 5);
    TEST_EXPECT(xStreamBufferBytesAvailable(sb)  == 5);
    TEST_EXPECT(xStreamBufferSpacesAvailable(sb) == 27);

    char rx[16] = {0};
    TEST_EXPECT(xStreamBufferReceive(sb, rx, sizeof(rx), 0) == 5);
    TEST_EXPECT(memcmp(rx, msg, 5) == 0);
    TEST_EXPECT(xStreamBufferIsEmpty(sb) == pdTRUE);

    // --- Trigger level: receive blocks until trigger bytes arrive ---
    TEST_EXPECT(xStreamBufferSetTriggerLevel(sb, 4) == pdPASS);
    // 3 bytes pending should not satisfy a trigger-4 receive with timeout.
    TEST_EXPECT(xStreamBufferSend(sb, "xyz", 3, 0) == 3);
    memset(rx, 0, sizeof(rx));
    TEST_EXPECT(xStreamBufferReceive(sb, rx, sizeof(rx),
                                     pdMS_TO_TICKS(30)) == 0);
    TEST_EXPECT(xStreamBufferBytesAvailable(sb) == 3);
    // Send the 4th byte -- receive now completes.
    TEST_EXPECT(xStreamBufferSend(sb, "w", 1, 0) == 1);
    memset(rx, 0, sizeof(rx));
    TEST_EXPECT(xStreamBufferReceive(sb, rx, sizeof(rx),
                                     pdMS_TO_TICKS(50)) == 4);
    TEST_EXPECT(memcmp(rx, "xyzw", 4) == 0);

    // --- Ring wrap-around correctness ---
    TEST_EXPECT(xStreamBufferReset(sb) == pdPASS);
    TEST_EXPECT(xStreamBufferSetTriggerLevel(sb, 1) == pdPASS);
    for (int i = 0; i < 3; i++) {
        // Push 10, pop 10 -- forces head/tail around the 32-byte ring.
        const char *p = "0123456789";
        TEST_EXPECT(xStreamBufferSend(sb, p, 10, 0) == 10);
        memset(rx, 0, sizeof(rx));
        TEST_EXPECT(xStreamBufferReceive(sb, rx, sizeof(rx), 0) == 10);
        TEST_EXPECT(memcmp(rx, p, 10) == 0);
    }

    // --- Cross-task producer -> consumer ---
    __frt_sb_producer_handle = sb;
    TEST_EXPECT(xStreamBufferReset(sb) == pdPASS);
    TEST_EXPECT(xStreamBufferSetTriggerLevel(sb, 5) == pdPASS);
    TaskHandle_t h = NULL;
    TEST_EXPECT(xTaskCreate(__frt_sb_producer, "sbp", 4096, NULL, 0, &h)
                == pdPASS);
    memset(rx, 0, sizeof(rx));
    // With trigger=5 the consumer only wakes once both chunks have
    // been pushed, so a single receive is enough.
    TEST_EXPECT(xStreamBufferReceive(sb, rx, sizeof(rx),
                                     pdMS_TO_TICKS(300)) == 5);
    TEST_EXPECT(memcmp(rx, "HELLO", 5) == 0);

    vStreamBufferDelete(sb);

    // Invalid-arg surface.
    TEST_EXPECT(xStreamBufferCreate(0, 1) == NULL);
    TEST_EXPECT(xStreamBufferSend(NULL, "x", 1, 0) == 0);
    TEST_EXPECT(xStreamBufferReceive(NULL, rx, 1, 0) == 0);
    TEST_EXPECT(xStreamBufferIsEmpty(NULL) == pdTRUE);

    printf("[freertos_stream_buffer] end" "\n");
}

static void __test_freertos_message_buffer(void)
{
    printf("[freertos_message_buffer] begin" "\n");

    // --- Message framing ---
    MessageBufferHandle_t mb = xMessageBufferCreate(64);
    TEST_EXPECT(mb != NULL);
    TEST_EXPECT(xMessageBufferIsEmpty(mb) == pdTRUE);
    TEST_EXPECT(xMessageBufferSend(mb, "abc",      3, 0) == 3);
    TEST_EXPECT(xMessageBufferSend(mb, "defghi",   6, 0) == 6);
    TEST_EXPECT(xMessageBufferNextLengthBytes(mb) == 3);

    char rx[16] = {0};
    // Buffer too small -> no consume.
    TEST_EXPECT(xMessageBufferReceive(mb, rx, 2, 0) == 0);
    TEST_EXPECT(xMessageBufferNextLengthBytes(mb) == 3);

    memset(rx, 0, sizeof(rx));
    TEST_EXPECT(xMessageBufferReceive(mb, rx, sizeof(rx), 0) == 3);
    TEST_EXPECT(memcmp(rx, "abc", 3) == 0);
    TEST_EXPECT(xMessageBufferNextLengthBytes(mb) == 6);

    memset(rx, 0, sizeof(rx));
    TEST_EXPECT(xMessageBufferReceive(mb, rx, sizeof(rx), 0) == 6);
    TEST_EXPECT(memcmp(rx, "defghi", 6) == 0);
    TEST_EXPECT(xMessageBufferIsEmpty(mb) == pdTRUE);
    TEST_EXPECT(xMessageBufferNextLengthBytes(mb) == 0);

    // Empty receive with non-zero timeout returns 0.
    TEST_EXPECT(xMessageBufferReceive(mb, rx, sizeof(rx),
                                      pdMS_TO_TICKS(20)) == 0);

    // --- Cross-task producer ---
    __frt_mb_producer_handle = mb;
    TaskHandle_t h = NULL;
    TEST_EXPECT(xTaskCreate(__frt_mb_producer, "mbp", 4096, NULL, 0, &h)
                == pdPASS);
    memset(rx, 0, sizeof(rx));
    TEST_EXPECT(xMessageBufferReceive(mb, rx, sizeof(rx),
                                      pdMS_TO_TICKS(200)) == 3);
    TEST_EXPECT(memcmp(rx, "abc", 3) == 0);
    memset(rx, 0, sizeof(rx));
    TEST_EXPECT(xMessageBufferReceive(mb, rx, sizeof(rx),
                                      pdMS_TO_TICKS(200)) == 6);
    TEST_EXPECT(memcmp(rx, "defghi", 6) == 0);

    // Message larger than ring capacity is rejected immediately.
    char big[128];
    memset(big, 'x', sizeof(big));
    TEST_EXPECT(xMessageBufferSend(mb, big, sizeof(big), 0) == 0);

    vMessageBufferDelete(mb);

    // Invalid-arg surface.
    TEST_EXPECT(xMessageBufferCreate(0) == NULL);
    // A buffer smaller than or equal to the 4-byte length header cannot
    // hold any message and must be rejected.
    TEST_EXPECT(xMessageBufferCreate(4) == NULL);
    TEST_EXPECT(xMessageBufferSend(NULL, "x", 1, 0) == 0);

    printf("[freertos_message_buffer] end" "\n");
}

// --- critical section / scheduler control -----------------------------

static void __test_freertos_critical(void)
{
    printf("[freertos_critical] begin" "\n");

    // --- taskENTER_CRITICAL / taskEXIT_CRITICAL, balanced ---
    // The shared counter increment is trivially atomic on a single-
    // core target; we only assert that the macro pair compiles and
    // executes without unbalancing the nest counter (a later Enter
    // would still succeed).
    int guarded = 0;
    taskENTER_CRITICAL();
        guarded++;
    taskEXIT_CRITICAL();
    TEST_EXPECT(guarded == 1);

    // --- Nested enter/exit ---
    taskENTER_CRITICAL();
        taskENTER_CRITICAL();
            guarded++;
        taskEXIT_CRITICAL();
        guarded++;
    taskEXIT_CRITICAL();
    TEST_EXPECT(guarded == 3);

    // --- portENTER_CRITICAL variadic forms: no-arg and with portMUX_TYPE ---
    portENTER_CRITICAL();
        guarded++;
    portEXIT_CRITICAL();

    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    vPortCPUInitializeMutex(&mux);
    portENTER_CRITICAL(&mux);
        guarded++;
    portEXIT_CRITICAL(&mux);
    TEST_EXPECT(guarded == 5);

    // --- FromISR variants roundtrip the saved state through the stack ---
    UBaseType_t s1 = taskENTER_CRITICAL_FROM_ISR();
        guarded++;
    taskEXIT_CRITICAL_FROM_ISR(s1);
    UBaseType_t s2 = vTaskEnterCriticalFromISR();
        guarded++;
    vTaskExitCriticalFromISR(s2);
    TEST_EXPECT(guarded == 7);

    // --- vTaskSuspendAll / xTaskResumeAll, balanced ---
    vTaskSuspendAll();
        // Suspend only gates the scheduler; the tick source still
        // advances, so xTaskGetTickCount must remain callable.
        (void)xTaskGetTickCount();
        guarded++;
    BaseType_t r = xTaskResumeAll();
    TEST_EXPECT(r == pdFALSE);      // shim never forces a switch
    TEST_EXPECT(guarded == 8);

    // --- Nested suspend ---
    vTaskSuspendAll();
        vTaskSuspendAll();
            guarded++;
        (void)xTaskResumeAll();
        guarded++;
    (void)xTaskResumeAll();
    TEST_EXPECT(guarded == 10);

    // --- Cross-composition: critical inside suspend, and vice versa ---
    // Verifies the two independent nest counters don't stomp on each
    // other. If they shared a counter, the outer Resume would try to
    // unlock while still inside the critical section (or vice versa),
    // and any imbalance would surface as a subsequent API misbehaving.
    vTaskSuspendAll();
        taskENTER_CRITICAL();
            guarded++;
        taskEXIT_CRITICAL();
    (void)xTaskResumeAll();

    taskENTER_CRITICAL();
        vTaskSuspendAll();
            guarded++;
        (void)xTaskResumeAll();
    taskEXIT_CRITICAL();
    TEST_EXPECT(guarded == 12);

    // After all the nesting, a fresh balanced pair must still work.
    taskENTER_CRITICAL();
        guarded++;
    taskEXIT_CRITICAL();
    TEST_EXPECT(guarded == 13);

    // Unbalanced Resume on a clean counter is tolerated (returns pdFALSE).
    TEST_EXPECT(xTaskResumeAll() == pdFALSE);

    printf("[freertos_critical] end" "\n");
}

// --- static (zero-heap) variants --------------------------------------

// Stack buffer for xTaskCreateStatic. Must be aligned to the arch page
// size (4096 on win/linux) and sized >= page + guardian. The byte count
// equals the array length in this build.
#if defined(_MSC_VER)
#   define __TEST_STACK_ALIGN(__n)  __declspec(align(__n))
#else
#   define __TEST_STACK_ALIGN(__n)  __attribute__((aligned(__n)))
#endif
__TEST_STACK_ALIGN(4096) static StackType_t __frt_static_task_stack[8192];
static StaticTask_t                     __frt_static_task_tcb;
static volatile int                     __frt_static_worker_hits;

static void __frt_static_worker(void *arg)
{
    int n = (int)(intptr_t)arg;
    for (int i = 0; i < n; i++) {
        __frt_static_worker_hits++;
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    // Fall through to the task wrapper, which terminates the task cleanly.
}

static volatile int __frt_static_timer_hits;
static void __frt_static_timer_cb(TimerHandle_t t)
{
    (void)t;
    __frt_static_timer_hits++;
}

static void __test_freertos_static(void)
{
    printf("[freertos_static] begin" "\n");

    // --- Static binary semaphore ---
    StaticSemaphore_t      sem_storage;
    SemaphoreHandle_t bs = xSemaphoreCreateBinaryStatic(&sem_storage);
    TEST_EXPECT(bs != NULL);
    TEST_EXPECT(xSemaphoreTake(bs, 0) == pdFAIL);
    TEST_EXPECT(xSemaphoreGive(bs) == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(bs, 0) == pdTRUE);
    vSemaphoreDelete(bs);
    TEST_EXPECT(xSemaphoreCreateBinaryStatic(NULL) == NULL);

    // --- Static counting semaphore ---
    StaticSemaphore_t      cnt_storage;
    SemaphoreHandle_t cs = xSemaphoreCreateCountingStatic(3, 2, &cnt_storage);
    TEST_EXPECT(cs != NULL);
    TEST_EXPECT(xSemaphoreTake(cs, 0) == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(cs, 0) == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(cs, 0) == pdFAIL);
    TEST_EXPECT(xSemaphoreGive(cs)   == pdTRUE);
    TEST_EXPECT(xSemaphoreTake(cs, 0) == pdTRUE);
    vSemaphoreDelete(cs);
    TEST_EXPECT(xSemaphoreCreateCountingStatic(0, 0, &cnt_storage) == NULL);
    TEST_EXPECT(xSemaphoreCreateCountingStatic(1, 0, NULL)         == NULL);

    // --- Static mutex ---
    StaticSemaphore_t      mtx_storage;
    SemaphoreHandle_t mx = xSemaphoreCreateMutexStatic(&mtx_storage);
    TEST_EXPECT(mx != NULL);
    TEST_EXPECT(xSemaphoreTake(mx, 0) == pdTRUE);
    TEST_EXPECT(xSemaphoreGive(mx)    == pdTRUE);
    vSemaphoreDelete(mx);
    TEST_EXPECT(xSemaphoreCreateMutexStatic(NULL) == NULL);

    // --- Static queue ---
    static uint8_t q_storage[4 * sizeof(uint32_t)];
    StaticQueue_t     q_tcb;
    QueueHandle_t q = xQueueCreateStatic(4, sizeof(uint32_t),
                                         q_storage, &q_tcb);
    TEST_EXPECT(q != NULL);
    for (uint32_t i = 0; i < 4; i++) {
        TEST_EXPECT(xQueueSend(q, &i, 0) == pdTRUE);
    }
    uint32_t overflow = 99;
    TEST_EXPECT(xQueueSend(q, &overflow, 0) == pdFAIL);
    uint32_t got = 0;
    TEST_EXPECT(xQueueReceive(q, &got, 0) == pdTRUE);
    TEST_EXPECT(got == 0);
    TEST_EXPECT(uxQueueMessagesWaiting(q) == 3);
    vQueueDelete(q);
    TEST_EXPECT(xQueueCreateStatic(0, 4, q_storage, &q_tcb)       == NULL);
    TEST_EXPECT(xQueueCreateStatic(4, 4, NULL,      &q_tcb)       == NULL);
    TEST_EXPECT(xQueueCreateStatic(4, 4, q_storage, NULL)         == NULL);

    // --- Static event group ---
    StaticEventGroup_t       eg_storage;
    EventGroupHandle_t eg = xEventGroupCreateStatic(&eg_storage);
    TEST_EXPECT(eg != NULL);
    TEST_EXPECT(xEventGroupSetBits(eg, 0x5) == 0x5);
    TEST_EXPECT((xEventGroupWaitBits(eg, 0x4, pdTRUE, pdTRUE, 0) & 0x4)
                == 0x4);
    TEST_EXPECT(xEventGroupGetBits(eg) == 0x1);
    vEventGroupDelete(eg);
    TEST_EXPECT(xEventGroupCreateStatic(NULL) == NULL);

    // --- Static timer ---
    StaticTimer_t       tmr_storage;
    __frt_static_timer_hits = 0;
    TimerHandle_t tmr = xTimerCreateStatic("t", pdMS_TO_TICKS(20), pdFALSE,
                                           NULL, __frt_static_timer_cb,
                                           &tmr_storage);
    TEST_EXPECT(tmr != NULL);
    TEST_EXPECT(xTimerStart(tmr, 0) == pdPASS);
    vTaskDelay(pdMS_TO_TICKS(60));
    TEST_EXPECT(__frt_static_timer_hits == 1);
    TEST_EXPECT(xTimerDelete(tmr, 0) == pdPASS);
    TEST_EXPECT(xTimerCreateStatic("t", 0,  pdFALSE, NULL,
                                   __frt_static_timer_cb, &tmr_storage)
                == NULL);
    TEST_EXPECT(xTimerCreateStatic("t", 10, pdFALSE, NULL, NULL,
                                   &tmr_storage) == NULL);
    TEST_EXPECT(xTimerCreateStatic("t", 10, pdFALSE, NULL,
                                   __frt_static_timer_cb, NULL) == NULL);

    // --- Static stream buffer ---
    static uint8_t sb_storage[64];
    StaticStreamBuffer_t     sb_tcb;
    StreamBufferHandle_t sb = xStreamBufferCreateStatic(
            sizeof(sb_storage), 1, sb_storage, &sb_tcb);
    TEST_EXPECT(sb != NULL);
    TEST_EXPECT(xStreamBufferSend(sb, "hello", 5, 0) == 5);
    uint8_t rx[16] = {0};
    TEST_EXPECT(xStreamBufferReceive(sb, rx, sizeof(rx),
                                     pdMS_TO_TICKS(10)) == 5);
    TEST_EXPECT(memcmp(rx, "hello", 5) == 0);
    vStreamBufferDelete(sb);
    TEST_EXPECT(xStreamBufferCreateStatic(sizeof(sb_storage), 1,
                                          NULL, &sb_tcb) == NULL);
    TEST_EXPECT(xStreamBufferCreateStatic(sizeof(sb_storage), 1,
                                          sb_storage, NULL) == NULL);

    // --- Static message buffer ---
    static uint8_t mb_storage[64];
    StaticMessageBuffer_t     mb_tcb;
    MessageBufferHandle_t mb = xMessageBufferCreateStatic(
            sizeof(mb_storage), mb_storage, &mb_tcb);
    TEST_EXPECT(mb != NULL);
    TEST_EXPECT(xMessageBufferSend(mb, "AB",  2, 0) == 2);
    TEST_EXPECT(xMessageBufferSend(mb, "CDE", 3, 0) == 3);
    uint8_t rx2[8] = {0};
    TEST_EXPECT(xMessageBufferReceive(mb, rx2, sizeof(rx2),
                                      pdMS_TO_TICKS(10)) == 2);
    TEST_EXPECT(memcmp(rx2, "AB", 2) == 0);
    TEST_EXPECT(xMessageBufferReceive(mb, rx2, sizeof(rx2),
                                      pdMS_TO_TICKS(10)) == 3);
    TEST_EXPECT(memcmp(rx2, "CDE", 3) == 0);
    vMessageBufferDelete(mb);
    // A buffer smaller than or equal to the 4-byte length header is
    // rejected just like the dynamic variant.
    TEST_EXPECT(xMessageBufferCreateStatic(4, mb_storage, &mb_tcb) == NULL);

    // --- Static task ---
    __frt_static_worker_hits = 0;
    TaskHandle_t th = xTaskCreateStatic(__frt_static_worker, "stask",
                                        sizeof(__frt_static_task_stack),
                                        (void *)(intptr_t)5, 0,
                                        __frt_static_task_stack,
                                        &__frt_static_task_tcb);
    TEST_EXPECT(th != NULL);
    vTaskDelay(pdMS_TO_TICKS(60));
    TEST_EXPECT(__frt_static_worker_hits == 5);

    // Invalid-arg surface; these reject before touching the caller
    // buffers so the live task above is not disturbed.
    TEST_EXPECT(xTaskCreateStatic(NULL, "bad", 8192, NULL, 0,
                                  __frt_static_task_stack,
                                  &__frt_static_task_tcb) == NULL);
    TEST_EXPECT(xTaskCreateStatic(__frt_static_worker, "bad", 8192, NULL, 0,
                                  NULL, &__frt_static_task_tcb) == NULL);
    TEST_EXPECT(xTaskCreateStatic(__frt_static_worker, "bad", 8192, NULL, 0,
                                  __frt_static_task_stack, NULL) == NULL);
    // Stack below the MIN / page+guardian floor is rejected.
    TEST_EXPECT(xTaskCreateStatic(__frt_static_worker, "bad", 1023, NULL, 0,
                                  __frt_static_task_stack,
                                  &__frt_static_task_tcb) == NULL);

    printf("[freertos_static] end" "\n");
}

// --- esp_event ---------------------------------------------------------

ESP_EVENT_DECLARE_BASE(TEST_EVENTS);
ESP_EVENT_DEFINE_BASE(TEST_EVENTS);

static volatile int __evt_any_hits;
static volatile int __evt_id1_hits;
static volatile int __evt_id2_hits;
static volatile int __evt_last_id;
static volatile uint32_t __evt_last_payload;
static volatile int __evt_instance_hits;

static void __evt_handler_any(void *arg, esp_event_base_t base,
                              int32_t id, void *data)
{
    (void)arg; (void)base;
    __evt_any_hits++;
    __evt_last_id = id;
    if (data != NULL) {
        __evt_last_payload = *(const uint32_t *)data;
    }
}

static void __evt_handler_id1(void *arg, esp_event_base_t base,
                              int32_t id, void *data)
{
    (void)arg; (void)base; (void)id; (void)data;
    __evt_id1_hits++;
}

static void __evt_handler_id2(void *arg, esp_event_base_t base,
                              int32_t id, void *data)
{
    (void)arg; (void)base; (void)id; (void)data;
    __evt_id2_hits++;
}

static void __evt_handler_instance(void *arg, esp_event_base_t base,
                                   int32_t id, void *data)
{
    (void)arg; (void)base; (void)id; (void)data;
    __evt_instance_hits++;
}

static void __test_esp_event(void)
{
    printf("[esp_event] begin" "\n");

    // Default loop was already created by the environment during
    // startup; a second create must report INVALID_STATE.
    TEST_EXPECT(esp_event_loop_create_default() == ESP_ERR_INVALID_STATE);

    // Register wildcard-id handler for TEST_EVENTS.
    TEST_EXPECT(esp_event_handler_register(TEST_EVENTS, ESP_EVENT_ANY_ID,
                                           __evt_handler_any, NULL) == ESP_OK);
    // Register id-specific handlers.
    TEST_EXPECT(esp_event_handler_register(TEST_EVENTS, 1,
                                           __evt_handler_id1, NULL) == ESP_OK);
    TEST_EXPECT(esp_event_handler_register(TEST_EVENTS, 2,
                                           __evt_handler_id2, NULL) == ESP_OK);

    __evt_any_hits = 0; __evt_id1_hits = 0; __evt_id2_hits = 0;
    __evt_last_id  = -99; __evt_last_payload = 0;

    // Post id=1 with a uint32_t payload: any + id1 must fire; id2 must not.
    uint32_t payload = 0xDEADBEEF;
    TEST_EXPECT(esp_event_post(TEST_EVENTS, 1, &payload, sizeof(payload),
                               pdMS_TO_TICKS(50)) == ESP_OK);
    // Post id=2 without payload: any + id2 must fire.
    TEST_EXPECT(esp_event_post(TEST_EVENTS, 2, NULL, 0,
                               pdMS_TO_TICKS(50)) == ESP_OK);

    // Wait for dispatcher to drain.
    __wait_ms(80);

    TEST_EXPECT(__evt_any_hits == 2);
    TEST_EXPECT(__evt_id1_hits == 1);
    TEST_EXPECT(__evt_id2_hits == 1);
    TEST_EXPECT(__evt_last_id  == 2);
    TEST_EXPECT(__evt_last_payload == 0xDEADBEEF);

    // Unregister id1 handler; next post id=1 must NOT increment id1 counter.
    TEST_EXPECT(esp_event_handler_unregister(TEST_EVENTS, 1,
                                             __evt_handler_id1) == ESP_OK);
    // Double-unregister returns NOT_FOUND.
    TEST_EXPECT(esp_event_handler_unregister(TEST_EVENTS, 1,
                                             __evt_handler_id1)
                == ESP_ERR_NOT_FOUND);

    int id1_before = __evt_id1_hits;
    int any_before = __evt_any_hits;
    TEST_EXPECT(esp_event_post(TEST_EVENTS, 1, NULL, 0,
                               pdMS_TO_TICKS(50)) == ESP_OK);
    __wait_ms(50);
    TEST_EXPECT(__evt_id1_hits == id1_before);      // unchanged
    TEST_EXPECT(__evt_any_hits == any_before + 1);  // wildcard still fires

    // Instance registration: distinct handle must allow targeted unregister.
    esp_event_handler_instance_t inst = NULL;
    TEST_EXPECT(esp_event_handler_instance_register(TEST_EVENTS, 3,
                                                    __evt_handler_instance,
                                                    NULL, &inst) == ESP_OK);
    TEST_EXPECT(inst != NULL);
    __evt_instance_hits = 0;
    TEST_EXPECT(esp_event_post(TEST_EVENTS, 3, NULL, 0,
                               pdMS_TO_TICKS(50)) == ESP_OK);
    __wait_ms(50);
    TEST_EXPECT(__evt_instance_hits == 1);
    TEST_EXPECT(esp_event_handler_instance_unregister(TEST_EVENTS, 3, inst)
                == ESP_OK);
    TEST_EXPECT(esp_event_post(TEST_EVENTS, 3, NULL, 0,
                               pdMS_TO_TICKS(50)) == ESP_OK);
    __wait_ms(50);
    TEST_EXPECT(__evt_instance_hits == 1);          // no further growth

    // Invalid-arg surface.
    TEST_EXPECT(esp_event_handler_register(TEST_EVENTS, 1, NULL, NULL)
                == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(esp_event_post(TEST_EVENTS, 1, NULL, 8, 0)
                == ESP_ERR_INVALID_ARG);

    // Clean up remaining registrations so subsequent test reruns start clean.
    TEST_EXPECT(esp_event_handler_unregister(TEST_EVENTS, 2,
                                             __evt_handler_id2) == ESP_OK);
    TEST_EXPECT(esp_event_handler_unregister(TEST_EVENTS, ESP_EVENT_ANY_ID,
                                             __evt_handler_any) == ESP_OK);

    printf("[esp_event] end" "\n");
}

// --- esp_partition ----------------------------------------------------

static void __test_esp_partition(void)
{
    printf("[esp_partition] begin" "\n");

    // --- Discovery ----------------------------------------------------
    // find_first with exact label must return the matching entry.
    const esp_partition_t *nvs =
        esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                 ESP_PARTITION_SUBTYPE_DATA_NVS, "nvs");
    TEST_EXPECT(nvs != NULL);
    if (nvs != NULL) {
        TEST_EXPECT(nvs->type    == ESP_PARTITION_TYPE_DATA);
        TEST_EXPECT(nvs->subtype == ESP_PARTITION_SUBTYPE_DATA_NVS);
        TEST_EXPECT(nvs->address == 0x0000);
        TEST_EXPECT(nvs->size    == 0x0400);
        TEST_EXPECT(strcmp(nvs->label, "nvs") == 0);
        TEST_EXPECT(!nvs->readonly);
    }

    // Label mismatch -> NULL.
    TEST_EXPECT(esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                         ESP_PARTITION_SUBTYPE_DATA_NVS,
                                         "no_such_label") == NULL);

    // ANY/ANY/NULL iterator walks every in-use slot. The static table
    // contributes four entries (nvs/phy_init/storage/lfs); no dynamic
    // entries are registered yet.
    esp_partition_iterator_t it =
        esp_partition_find(ESP_PARTITION_TYPE_ANY,
                           ESP_PARTITION_SUBTYPE_ANY, NULL);
    TEST_EXPECT(it != NULL);
    int seen = 0;
    bool saw_nvs = false, saw_phy = false, saw_storage = false, saw_lfs = false;
    while (it != NULL) {
        const esp_partition_t *p = esp_partition_get(it);
        TEST_EXPECT(p != NULL);
        if (p != NULL) {
            seen++;
            if (strcmp(p->label, "nvs")      == 0) { saw_nvs     = true; }
            if (strcmp(p->label, "phy_init") == 0) { saw_phy     = true; }
            if (strcmp(p->label, "storage")  == 0) { saw_storage = true; }
            if (strcmp(p->label, "lfs")      == 0) { saw_lfs     = true; }
        }
        it = esp_partition_next(it);
    }
    TEST_EXPECT(seen == 4);
    TEST_EXPECT(saw_nvs && saw_phy && saw_storage && saw_lfs);

    // check_identity: same pointer ok, different pointer rejected.
    const esp_partition_t *nvs_again =
        esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                 ESP_PARTITION_SUBTYPE_DATA_NVS, "nvs");
    TEST_EXPECT(esp_partition_check_identity(nvs, nvs_again));
    const esp_partition_t *phy =
        esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                 ESP_PARTITION_SUBTYPE_DATA_PHY, "phy_init");
    TEST_EXPECT(phy != NULL);
    TEST_EXPECT(!esp_partition_check_identity(nvs, phy));
    TEST_EXPECT(esp_partition_verify(nvs) == nvs);

    // --- Write then read-back on a rw partition ----------------------
    const esp_partition_t *storage =
        esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                 ESP_PARTITION_SUBTYPE_DATA_UNDEFINED,
                                 "storage");
    TEST_EXPECT(storage != NULL);
    if (storage != NULL) {
        uint8_t tx[64];
        uint8_t rx[64];
        for (size_t i = 0; i < sizeof(tx); i++) { tx[i] = (uint8_t)(i ^ 0x5A); }
        memset(rx, 0, sizeof(rx));

        TEST_EXPECT(esp_partition_write(storage, 128, tx, sizeof(tx)) == ESP_OK);
        TEST_EXPECT(esp_partition_read (storage, 128, rx, sizeof(rx)) == ESP_OK);
        TEST_EXPECT(memcmp(tx, rx, sizeof(tx)) == 0);

        // raw variants are identity-pass in this shim.
        memset(rx, 0xFF, sizeof(rx));
        TEST_EXPECT(esp_partition_read_raw(storage, 128, rx, sizeof(rx)) == ESP_OK);
        TEST_EXPECT(memcmp(tx, rx, sizeof(tx)) == 0);

        // Bounds: offset+size beyond the partition must be rejected.
        TEST_EXPECT(esp_partition_read(storage, storage->size - 4,
                                       rx, 16) == ESP_ERR_INVALID_SIZE);
        TEST_EXPECT(esp_partition_write(storage, storage->size,
                                        tx, 4) == ESP_ERR_INVALID_SIZE);

        // erase_range must clear the range and the subsequent read
        // must return all-0xFF across the erased window. Whether the
        // underlying storage physically erases or emulates a fill is an
        // environment-provider detail the test does not care about.
        TEST_EXPECT(esp_partition_erase_range(storage, 0, storage->size)
                    == ESP_OK);
        uint8_t eb[16];
        memset(eb, 0, sizeof(eb));
        TEST_EXPECT(esp_partition_read(storage, 0, eb, sizeof(eb)) == ESP_OK);
        bool all_ff = true;
        for (size_t i = 0; i < sizeof(eb); i++) {
            if (eb[i] != 0xFF) { all_ff = false; break; }
        }
        TEST_EXPECT(all_ff);
    }

    // A write on a readonly partition must be refused.
    if (phy != NULL) {
        uint8_t one = 0xA5;
        TEST_EXPECT(esp_partition_write(phy, 0, &one, 1)
                    == ESP_ERR_NOT_ALLOWED);
    }

    // --- mmap -------------------------------------------------------
    // Whether mmap is supported depends on how the environment backs
    // the partition. On this host build the backing storage does not
    // expose a memory-mapped view, so the shim must surface
    // ESP_ERR_NOT_SUPPORTED rather than fabricate a buffer.
    if (storage != NULL) {
        const void *mm = NULL;
        esp_partition_mmap_handle_t hnd = 0;
        esp_err_t rc = esp_partition_mmap(storage, 128, 64,
                                          ESP_PARTITION_MMAP_DATA,
                                          &mm, &hnd);
        TEST_EXPECT(rc == ESP_ERR_NOT_SUPPORTED);
        (void)mm;
        (void)hnd;
    }

    // --- Dynamic registration --------------------------------------
    // Park external partitions in the uppermost slice of the 64 KiB
    // window (0xE000+) so they never overlap the static lfs partition
    // at 0x1000..0x8FFF.
    const esp_partition_t *ext = NULL;
    TEST_EXPECT(esp_partition_register_external(NULL, 0xE000, 0x400,
                                                "ext0",
                                                ESP_PARTITION_TYPE_DATA,
                                                ESP_PARTITION_SUBTYPE_DATA_UNDEFINED,
                                                &ext) == ESP_OK);
    TEST_EXPECT(ext != NULL);
    if (ext != NULL) {
        TEST_EXPECT(strcmp(ext->label, "ext0") == 0);
        TEST_EXPECT(ext->address == 0xE000);
        TEST_EXPECT(ext->size    == 0x0400);

        // find_first must observe the new entry.
        const esp_partition_t *ext_find =
            esp_partition_find_first(ESP_PARTITION_TYPE_ANY,
                                     ESP_PARTITION_SUBTYPE_ANY, "ext0");
        TEST_EXPECT(ext_find == ext);

        // Duplicate label must be rejected.
        const esp_partition_t *dup = NULL;
        TEST_EXPECT(esp_partition_register_external(
                        NULL, 0xE400, 0x400, "ext0",
                        ESP_PARTITION_TYPE_DATA,
                        ESP_PARTITION_SUBTYPE_DATA_UNDEFINED,
                        &dup) == ESP_ERR_INVALID_ARG);

        // RW roundtrip on the external partition.
        uint8_t e_tx[32];
        uint8_t e_rx[32];
        for (size_t i = 0; i < sizeof(e_tx); i++) { e_tx[i] = (uint8_t)(0x80 + i); }
        memset(e_rx, 0, sizeof(e_rx));
        TEST_EXPECT(esp_partition_write(ext, 0, e_tx, sizeof(e_tx)) == ESP_OK);
        TEST_EXPECT(esp_partition_read (ext, 0, e_rx, sizeof(e_rx)) == ESP_OK);
        TEST_EXPECT(memcmp(e_tx, e_rx, sizeof(e_tx)) == 0);

        // Deregister removes the entry from the enumeration.
        TEST_EXPECT(esp_partition_deregister_external(ext) == ESP_OK);
        TEST_EXPECT(esp_partition_find_first(ESP_PARTITION_TYPE_ANY,
                                             ESP_PARTITION_SUBTYPE_ANY,
                                             "ext0") == NULL);
        TEST_EXPECT(esp_partition_deregister_external(ext)
                    == ESP_ERR_NOT_FOUND);
    }

    // Invalid-arg surface.
    TEST_EXPECT(esp_partition_read (NULL, 0, NULL, 0) == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(esp_partition_write(NULL, 0, NULL, 0) == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(esp_partition_register_external(NULL, 0, 0, NULL,
                                                ESP_PARTITION_TYPE_DATA,
                                                ESP_PARTITION_SUBTYPE_DATA_UNDEFINED,
                                                NULL)
                == ESP_ERR_INVALID_ARG);

    printf("[esp_partition] end" "\n");
}

// --- esp_nvs ---------------------------------------------------------------

#include "nvs.h"
#include "nvs_flash.h"

static void __test_esp_nvs(void)
{
    printf("[esp_nvs] begin" "\n");

    // 1. Init the default "nvs" partition (already in the static table).
    TEST_EXPECT(nvs_flash_init() == ESP_OK);
    // Double init is idempotent.
    TEST_EXPECT(nvs_flash_init() == ESP_OK);

    // 2. Open a namespace in RW mode.
    nvs_handle_t h = 0;
    TEST_EXPECT(nvs_open("test_ns", NVS_READWRITE, &h) == ESP_OK);

    // 3. Integer set / get round-trip.
    TEST_EXPECT(nvs_set_u8 (h, "u8",  0xAB)        == ESP_OK);
    TEST_EXPECT(nvs_set_i8 (h, "i8",  -42)         == ESP_OK);
    TEST_EXPECT(nvs_set_u16(h, "u16", 0x1234)      == ESP_OK);
    TEST_EXPECT(nvs_set_i16(h, "i16", -300)        == ESP_OK);
    TEST_EXPECT(nvs_set_u32(h, "u32", 0xDEADBEEF)  == ESP_OK);
    TEST_EXPECT(nvs_set_i32(h, "i32", -100000)     == ESP_OK);
    TEST_EXPECT(nvs_set_u64(h, "u64", 0x0102030405060708ULL) == ESP_OK);
    TEST_EXPECT(nvs_set_i64(h, "i64", -999999999LL) == ESP_OK);

    uint8_t  v_u8  = 0;
    int8_t   v_i8  = 0;
    uint16_t v_u16 = 0;
    int16_t  v_i16 = 0;
    uint32_t v_u32 = 0;
    int32_t  v_i32 = 0;
    uint64_t v_u64 = 0;
    int64_t  v_i64 = 0;
    TEST_EXPECT(nvs_get_u8 (h, "u8",  &v_u8)  == ESP_OK && v_u8  == 0xAB);
    TEST_EXPECT(nvs_get_i8 (h, "i8",  &v_i8)  == ESP_OK && v_i8  == -42);
    TEST_EXPECT(nvs_get_u16(h, "u16", &v_u16) == ESP_OK && v_u16 == 0x1234);
    TEST_EXPECT(nvs_get_i16(h, "i16", &v_i16) == ESP_OK && v_i16 == -300);
    TEST_EXPECT(nvs_get_u32(h, "u32", &v_u32) == ESP_OK && v_u32 == 0xDEADBEEF);
    TEST_EXPECT(nvs_get_i32(h, "i32", &v_i32) == ESP_OK && v_i32 == -100000);
    TEST_EXPECT(nvs_get_u64(h, "u64", &v_u64) == ESP_OK && v_u64 == 0x0102030405060708ULL);
    TEST_EXPECT(nvs_get_i64(h, "i64", &v_i64) == ESP_OK && v_i64 == -999999999LL);

    // 4. String set / get / length query.
    TEST_EXPECT(nvs_set_str(h, "greeting", "hello") == ESP_OK);
    size_t slen = 0;
    TEST_EXPECT(nvs_get_str(h, "greeting", NULL, &slen) == ESP_OK);
    TEST_EXPECT(slen == 6);   // strlen("hello") + 1
    char sbuf[16];
    slen = sizeof(sbuf);
    TEST_EXPECT(nvs_get_str(h, "greeting", sbuf, &slen) == ESP_OK);
    TEST_EXPECT(strcmp(sbuf, "hello") == 0);

    // 5. Blob set / get.
    uint8_t blob_tx[8] = {1,2,3,4,5,6,7,8};
    TEST_EXPECT(nvs_set_blob(h, "blob", blob_tx, sizeof(blob_tx)) == ESP_OK);
    size_t blen = 0;
    TEST_EXPECT(nvs_get_blob(h, "blob", NULL, &blen) == ESP_OK);
    TEST_EXPECT(blen == sizeof(blob_tx));
    uint8_t blob_rx[8] = {0};
    blen = sizeof(blob_rx);
    TEST_EXPECT(nvs_get_blob(h, "blob", blob_rx, &blen) == ESP_OK);
    TEST_EXPECT(memcmp(blob_tx, blob_rx, sizeof(blob_tx)) == 0);

    // 6. Type mismatch.
    TEST_EXPECT(nvs_get_u32(h, "u8", &v_u32) == ESP_ERR_NVS_TYPE_MISMATCH);

    // 7. Key not found.
    TEST_EXPECT(nvs_get_u8(h, "nokey", &v_u8) == ESP_ERR_NVS_NOT_FOUND);

    // 8. Commit (flush to partition).
    TEST_EXPECT(nvs_commit(h) == ESP_OK);

    // 9. Close handle.
    nvs_close(h);

    // 10. Persistence round-trip: deinit �?re-init �?re-read.
    TEST_EXPECT(nvs_flash_deinit() == ESP_OK);
    TEST_EXPECT(nvs_flash_init()   == ESP_OK);
    nvs_handle_t h2 = 0;
    TEST_EXPECT(nvs_open("test_ns", NVS_READONLY, &h2) == ESP_OK);
    v_u8 = 0;
    TEST_EXPECT(nvs_get_u8(h2, "u8", &v_u8) == ESP_OK && v_u8 == 0xAB);
    v_u32 = 0;
    TEST_EXPECT(nvs_get_u32(h2, "u32", &v_u32) == ESP_OK && v_u32 == 0xDEADBEEF);
    char s2[16]; slen = sizeof(s2);
    TEST_EXPECT(nvs_get_str(h2, "greeting", s2, &slen) == ESP_OK);
    TEST_EXPECT(strcmp(s2, "hello") == 0);
    // Read-only handle rejects writes.
    TEST_EXPECT(nvs_set_u8(h2, "x", 1) == ESP_ERR_NVS_READ_ONLY);
    nvs_close(h2);

    // 11. erase_key + erase_all.
    TEST_EXPECT(nvs_open("test_ns", NVS_READWRITE, &h) == ESP_OK);
    TEST_EXPECT(nvs_erase_key(h, "u8") == ESP_OK);
    TEST_EXPECT(nvs_get_u8(h, "u8", &v_u8) == ESP_ERR_NVS_NOT_FOUND);
    TEST_EXPECT(nvs_erase_key(h, "u8") == ESP_ERR_NVS_NOT_FOUND);
    TEST_EXPECT(nvs_erase_all(h) == ESP_OK);
    TEST_EXPECT(nvs_get_u32(h, "u32", &v_u32) == ESP_ERR_NVS_NOT_FOUND);
    TEST_EXPECT(nvs_commit(h) == ESP_OK);
    nvs_close(h);

    // 12. Namespace isolation: writes under "ns_a" are invisible in "ns_b".
    nvs_handle_t ha = 0, hb = 0;
    TEST_EXPECT(nvs_open("ns_a", NVS_READWRITE, &ha) == ESP_OK);
    TEST_EXPECT(nvs_open("ns_b", NVS_READWRITE, &hb) == ESP_OK);
    TEST_EXPECT(nvs_set_u8(ha, "val", 0x11) == ESP_OK);
    TEST_EXPECT(nvs_set_u8(hb, "val", 0x22) == ESP_OK);
    v_u8 = 0;
    TEST_EXPECT(nvs_get_u8(ha, "val", &v_u8) == ESP_OK && v_u8 == 0x11);
    v_u8 = 0;
    TEST_EXPECT(nvs_get_u8(hb, "val", &v_u8) == ESP_OK && v_u8 == 0x22);
    TEST_EXPECT(nvs_commit(ha) == ESP_OK);
    TEST_EXPECT(nvs_commit(hb) == ESP_OK);
    nvs_close(ha);
    nvs_close(hb);

    // 13. Iterator.
    nvs_iterator_t it = NULL;
    esp_err_t rc = nvs_entry_find(NVS_DEFAULT_PART_NAME, "ns_a",
                                   NVS_TYPE_ANY, &it);
    TEST_EXPECT(rc == ESP_OK && it != NULL);
    if (it != NULL) {
        nvs_entry_info_t info;
        TEST_EXPECT(nvs_entry_info(it, &info) == ESP_OK);
        TEST_EXPECT(strcmp(info.namespace_name, "ns_a") == 0);
        TEST_EXPECT(strcmp(info.key, "val") == 0);
        TEST_EXPECT(info.type == NVS_TYPE_U8);
        // Only one entry in ns_a.
        TEST_EXPECT(nvs_entry_next(&it) == ESP_ERR_NVS_NOT_FOUND);
        TEST_EXPECT(it == NULL);
        nvs_release_iterator(it);  // NULL is tolerated
    }

    // 14. Stats.
    nvs_stats_t stats;
    TEST_EXPECT(nvs_get_stats(NVS_DEFAULT_PART_NAME, &stats) == ESP_OK);
    // ns_a and ns_b each have 1 entry �?2 used, 2 namespaces.
    TEST_EXPECT(stats.used_entries == 2);
    TEST_EXPECT(stats.namespace_count == 2);

    size_t used = 0;
    TEST_EXPECT(nvs_open("ns_a", NVS_READONLY, &ha) == ESP_OK);
    TEST_EXPECT(nvs_get_used_entry_count(ha, &used) == ESP_OK);
    TEST_EXPECT(used == 1);
    nvs_close(ha);

    // 15. Invalid-arg surface.
    nvs_handle_t bad = 0;
    TEST_EXPECT(nvs_open(NULL, NVS_READWRITE, &bad) == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(nvs_commit(0)  == ESP_ERR_NVS_INVALID_HANDLE);
    TEST_EXPECT(nvs_set_u8(0, "k", 1) == ESP_ERR_NVS_INVALID_HANDLE);
    TEST_EXPECT(nvs_get_stats(NULL, NULL) == ESP_ERR_INVALID_ARG);

    // 16. Erase partition and verify empty.
    TEST_EXPECT(nvs_flash_erase() == ESP_OK);
    TEST_EXPECT(nvs_flash_init()  == ESP_OK);
    TEST_EXPECT(nvs_open("ns_a", NVS_READONLY, &ha) == ESP_OK);
    TEST_EXPECT(nvs_get_u8(ha, "val", &v_u8) == ESP_ERR_NVS_NOT_FOUND);
    nvs_close(ha);

    // Cleanup.
    TEST_EXPECT(nvs_flash_deinit() == ESP_OK);
    // Double deinit is an error.
    TEST_EXPECT(nvs_flash_deinit() == ESP_ERR_NVS_NOT_INITIALIZED);

    printf("[esp_nvs] end" "\n");
}

// --- esp_flash -------------------------------------------------------------

#include "esp_flash.h"

static void __test_esp_flash(void)
{
    printf("[esp_flash] begin" "\n");

    // --- Default chip must be wired up by the environment ------------
    // The environment provider is expected to install
    // esp_flash_default_chip before app_main runs. A NULL here means
    // the environment did not complete flash bring-up.
    TEST_EXPECT(esp_flash_default_chip != NULL);
    if (esp_flash_default_chip == NULL) {
        printf("[esp_flash] end" "\n");
        return;
    }

    // --- Size reporting ----------------------------------------------
    // The exact chip size is decided by the environment provider
    // (vsf_main.c). Here we only assert runtime consistency: the
    // reported size is non-zero and physical size matches it.
    uint32_t chip_size = 0;
    TEST_EXPECT(esp_flash_get_size(NULL, &chip_size) == ESP_OK);
    TEST_EXPECT(chip_size > 0);
    uint32_t phys_size = 0;
    TEST_EXPECT(esp_flash_get_physical_size(NULL, &phys_size) == ESP_OK);
    TEST_EXPECT(phys_size == chip_size);
    TEST_EXPECT(esp_flash_get_size(NULL, NULL) == ESP_ERR_INVALID_ARG);

    // --- Identity (stub returns configured chip_id, defaults to 0) ---
    uint32_t chip_id = 0xDEADBEEF;
    TEST_EXPECT(esp_flash_read_id(NULL, &chip_id) == ESP_OK);
    TEST_EXPECT(chip_id == 0);

    // --- esp_flash_init accepts both NULL and user-supplied chip -----
    TEST_EXPECT(esp_flash_init(NULL) == ESP_OK);
    TEST_EXPECT(esp_flash_init(esp_flash_default_chip) == ESP_OK);

    // --- Read/Write loopback -----------------------------------------
    // Partition layout (see __esp_part_table above):
    //   0x0000-0x0400  nvs
    //   0x0400-0x0800  phy_init
    //   0x0800-0x1000  storage
    //   0x1000-0x9000  lfs  (end-to-end mounted by __test_esp_vfs_littlefs)
    //   0xE000-...     ext0
    // 0x9000-0xE000 is an unallocated hole; pick an offset there so the
    // raw esp_flash read/write/erase loopback never stomps on the lfs
    // superblocks that the later littlefs test depends on.
    const uint32_t test_off = 0xD000u;
    uint8_t  wbuf[32];
    uint8_t  rbuf[32];
    for (size_t i = 0; i < sizeof(wbuf); i++) {
        wbuf[i] = (uint8_t)(0xA0 + i);
    }
    // Preserve the original bytes so we can restore them at the end --
    // the same backing file persists NVS, and other tests may rely on
    // that region being whatever the prior session left behind.
    uint8_t saved[32];
    TEST_EXPECT(esp_flash_read(NULL, saved, test_off, sizeof(saved)) == ESP_OK);

    TEST_EXPECT(esp_flash_write(NULL, wbuf, test_off, sizeof(wbuf)) == ESP_OK);
    memset(rbuf, 0, sizeof(rbuf));
    TEST_EXPECT(esp_flash_read(NULL, rbuf, test_off, sizeof(rbuf)) == ESP_OK);
    TEST_EXPECT(memcmp(wbuf, rbuf, sizeof(wbuf)) == 0);

    // Zero-length I/O must succeed without touching the mal.
    TEST_EXPECT(esp_flash_read(NULL, rbuf, test_off, 0) == ESP_OK);
    TEST_EXPECT(esp_flash_write(NULL, wbuf, test_off, 0) == ESP_OK);

    // Restore original bytes.
    TEST_EXPECT(esp_flash_write(NULL, saved, test_off, sizeof(saved)) == ESP_OK);

    // --- Encrypted aliases (no flash-encryption layer in this shim) --
    TEST_EXPECT(esp_flash_write_encrypted(NULL, test_off,
                                          wbuf, sizeof(wbuf)) == ESP_OK);
    memset(rbuf, 0, sizeof(rbuf));
    TEST_EXPECT(esp_flash_read_encrypted(NULL, test_off,
                                         rbuf, sizeof(rbuf)) == ESP_OK);
    TEST_EXPECT(memcmp(wbuf, rbuf, sizeof(wbuf)) == 0);
    TEST_EXPECT(esp_flash_write(NULL, saved, test_off, sizeof(saved)) == ESP_OK);

    // --- Bounds checking ---------------------------------------------
    TEST_EXPECT(esp_flash_read(NULL, rbuf, chip_size, 1)
                == ESP_ERR_INVALID_SIZE);
    TEST_EXPECT(esp_flash_write(NULL, wbuf, chip_size - 1, 2)
                == ESP_ERR_INVALID_SIZE);
    TEST_EXPECT(esp_flash_read(NULL, NULL, test_off, 4)
                == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(esp_flash_write(NULL, NULL, test_off, 4)
                == ESP_ERR_INVALID_ARG);

    // --- Erase: erase_region must clear the range to 0xFF. The test
    // verifies the erased bytes and then restores the saved contents
    // so later sub-tests keep observing a stable image.
    TEST_EXPECT(esp_flash_erase_region(NULL, test_off, 0x100) == ESP_OK);
    {
        uint8_t ebuf[16];
        memset(ebuf, 0, sizeof(ebuf));
        TEST_EXPECT(esp_flash_read(NULL, ebuf, test_off, sizeof(ebuf))
                    == ESP_OK);
        bool all_ff = true;
        for (uint32_t i = 0; i < sizeof(ebuf); i++) {
            if (ebuf[i] != 0xFFu) { all_ff = false; break; }
        }
        TEST_EXPECT(all_ff);
        // Restore the original bytes so subsequent reruns keep working.
        TEST_EXPECT(esp_flash_write(NULL, saved, test_off, sizeof(saved))
                    == ESP_OK);
    }
    // esp_flash_erase_chip would wipe the whole root image, which hosts
    // the lfs partition mounted later in this run -- skip the whole-chip
    // path and just verify the API is accepted.
    // TEST_EXPECT(esp_flash_erase_chip(NULL) == ESP_OK);  // destructive

    // --- Auxiliary OS hooks are accepted for API parity --------------
    int attached = -1;
    TEST_EXPECT(esp_flash_init_os_functions(esp_flash_default_chip,
                                             0, &attached) == ESP_OK);
    TEST_EXPECT(esp_flash_deinit_os_functions(esp_flash_default_chip) == ESP_OK);

    printf("[esp_flash] end" "\n");
}

// --- esp_vfs_littlefs / esp_vfs_fat ----------------------------------
//
// The lfs volume runs end-to-end on the dedicated "lfs" partition:
// the environment provides an erasable backing store so
// format-on-mount, open/read/write and persistence across an
// unregister/re-register cycle all work through the POSIX VFS.
//
// The fat volume is still API-shape-only. VSF's in-tree fatfs driver
// does not expose a public f_mkfs equivalent, so formatting a blank
// partition from the esp_vfs_fat bridge is not feasible -- the tests
// below cover argument validation and the mounted/unregister state
// machine only. Real FAT round-trips would require either a public
// mkfs entry in the VSF fatfs driver or a pre-populated partition
// image.
static void __test_esp_vfs_littlefs(void)
{
    printf("[esp_vfs_littlefs] begin" "\n");

    // --- Argument validation (no mount attempted) -----------------
    TEST_EXPECT(esp_vfs_littlefs_register(NULL) == ESP_ERR_INVALID_ARG);

    esp_vfs_littlefs_conf_t cfg = { 0 };
    TEST_EXPECT(esp_vfs_littlefs_register(&cfg) == ESP_ERR_INVALID_ARG);

    cfg.base_path       = "littlefs"; // must start with '/'
    cfg.partition_label = "lfs";
    TEST_EXPECT(esp_vfs_littlefs_register(&cfg) == ESP_ERR_INVALID_ARG);

    cfg.base_path       = "/lfs";
    cfg.partition_label = NULL;
    TEST_EXPECT(esp_vfs_littlefs_register(&cfg) == ESP_ERR_INVALID_ARG);

    cfg.partition_label = "";
    TEST_EXPECT(esp_vfs_littlefs_register(&cfg) == ESP_ERR_INVALID_ARG);

    cfg.partition_label = "no_such_part";
    TEST_EXPECT(esp_vfs_littlefs_register(&cfg) == ESP_ERR_NOT_FOUND);

    // --- Query helpers on an unknown volume -----------------------
    TEST_EXPECT(!esp_littlefs_mounted("no_such_part"));
    TEST_EXPECT(esp_vfs_littlefs_unregister("no_such_part") == ESP_ERR_NOT_FOUND);
    size_t total = 0, used = 0;
    TEST_EXPECT(esp_littlefs_info("no_such_part", &total, &used)
                == ESP_ERR_NOT_FOUND);
    TEST_EXPECT(esp_littlefs_format(NULL) == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(esp_littlefs_format("")   == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(esp_littlefs_format("no_such_part") == ESP_ERR_NOT_FOUND);

    // --- Explicit format on a blank volume ------------------------
    // The lfs partition may carry data from a previous run; an
    // explicit format resets it so the rest of the test starts from a
    // known state. esp_littlefs_format is expected to clear the
    // partition and lay down fresh superblocks.
    TEST_EXPECT(esp_littlefs_format("lfs") == ESP_OK);

    // esp_vfs_littlefs_register mounts via the underlying VFS which,
    // on POSIX-style hosts, requires the target directory to exist.
    // Pre-create /lfs here to mirror what the environment does for
    // /root, independent of how mount is implemented downstream.
    (void)mkdir("/lfs", 0);

    // --- Real register + mount -----------------------------------
    esp_vfs_littlefs_conf_t real_cfg = {
        .base_path              = "/lfs",
        .partition_label        = "lfs",
        .format_if_mount_failed = 1,
    };
    TEST_EXPECT(esp_vfs_littlefs_register(&real_cfg) == ESP_OK);
    TEST_EXPECT(esp_littlefs_mounted("lfs"));

    // Re-registering the same partition is rejected.
    TEST_EXPECT(esp_vfs_littlefs_register(&real_cfg) == ESP_ERR_INVALID_STATE);
    // Formatting a currently-mounted volume is rejected too.
    TEST_EXPECT(esp_littlefs_format("lfs") == ESP_ERR_INVALID_STATE);

    // esp_littlefs_info: total must equal the lfs partition size; used
    // is best-effort (current VSF port reports 0 -- documented).
    total = 0; used = 0xDEADBEEFu;
    TEST_EXPECT(esp_littlefs_info("lfs", &total, &used) == ESP_OK);
    TEST_EXPECT(total == 0x8000);
    TEST_EXPECT(used  == 0);

    // --- File I/O through POSIX VFS -------------------------------
    static const char __payload[] = "hello-littlefs-via-vsf";
    const size_t      __payload_len = sizeof(__payload) - 1;

    int fd = open("/lfs/persist.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    TEST_EXPECT(fd >= 0);
    if (fd >= 0) {
        ssize_t wr = write(fd, __payload, __payload_len);
        TEST_EXPECT((size_t)wr == __payload_len);
        TEST_EXPECT(close(fd) == 0);
    }

    fd = open("/lfs/persist.txt", O_RDONLY);
    TEST_EXPECT(fd >= 0);
    if (fd >= 0) {
        char rb[64] = { 0 };
        ssize_t rd = read(fd, rb, sizeof(rb) - 1);
        TEST_EXPECT((size_t)rd == __payload_len);
        TEST_EXPECT(memcmp(rb, __payload, __payload_len) == 0);
        TEST_EXPECT(close(fd) == 0);
    }

    // --- Unregister / re-register: data must survive --------------
    TEST_EXPECT(esp_vfs_littlefs_unregister("lfs") == ESP_OK);
    TEST_EXPECT(!esp_littlefs_mounted("lfs"));

    // A second register with format_if_mount_failed must NOT reformat
    // (mount succeeds against the just-written superblocks).
    esp_vfs_littlefs_conf_t rem_cfg = {
        .base_path              = "/lfs",
        .partition_label        = "lfs",
        .format_if_mount_failed = 1,
    };
    TEST_EXPECT(esp_vfs_littlefs_register(&rem_cfg) == ESP_OK);
    TEST_EXPECT(esp_littlefs_mounted("lfs"));

    fd = open("/lfs/persist.txt", O_RDONLY);
    TEST_EXPECT(fd >= 0);
    if (fd >= 0) {
        char rb[64] = { 0 };
        ssize_t rd = read(fd, rb, sizeof(rb) - 1);
        TEST_EXPECT((size_t)rd == __payload_len);
        TEST_EXPECT(memcmp(rb, __payload, __payload_len) == 0);
        TEST_EXPECT(close(fd) == 0);
    }

    // --- Cleanup --------------------------------------------------
    TEST_EXPECT(esp_vfs_littlefs_unregister("lfs") == ESP_OK);
    TEST_EXPECT(!esp_littlefs_mounted("lfs"));

    printf("[esp_vfs_littlefs] end" "\n");
}

static void __test_esp_vfs_fat(void)
{
    printf("[esp_vfs_fat] begin" "\n");

    esp_vfs_fat_mount_config_t mcfg = { 0 };
    wl_handle_t wl = WL_INVALID_HANDLE;

    // Null / malformed arguments.
    TEST_EXPECT(esp_vfs_fat_spiflash_mount_rw_wl(NULL, "storage", &mcfg, &wl)
                == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(esp_vfs_fat_spiflash_mount_rw_wl("fat", "storage", &mcfg, &wl)
                == ESP_ERR_INVALID_ARG); // not starting with '/'
    TEST_EXPECT(esp_vfs_fat_spiflash_mount_rw_wl("/fat", NULL, &mcfg, &wl)
                == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(esp_vfs_fat_spiflash_mount_rw_wl("/fat", "", &mcfg, &wl)
                == ESP_ERR_INVALID_ARG);
    TEST_EXPECT(wl == WL_INVALID_HANDLE);

    // Unknown partition -> NOT_FOUND (no fallback format attempted).
    TEST_EXPECT(esp_vfs_fat_spiflash_mount_rw_wl("/fat", "no_such_part",
                                                 &mcfg, &wl)
                == ESP_ERR_NOT_FOUND);
    TEST_EXPECT(esp_vfs_fat_spiflash_mount_ro("/fat", "no_such_part",
                                              &mcfg)
                == ESP_ERR_NOT_FOUND);

    // unmount on a base_path we never mounted.
    TEST_EXPECT(esp_vfs_fat_spiflash_unmount_rw_wl("/fat", WL_INVALID_HANDLE)
                == ESP_ERR_NOT_FOUND);
    TEST_EXPECT(esp_vfs_fat_spiflash_unmount_ro("/fat", "storage")
                == ESP_ERR_NOT_FOUND);

    // format path is intentionally not supported by the current VSF
    // fatfs driver; verify the port reflects that.
    TEST_EXPECT(esp_vfs_fat_spiflash_format_rw_wl("/fat", "storage")
                == ESP_ERR_NOT_SUPPORTED);

    // esp_vfs_register / register_fd_range / unregister are stubs.
    TEST_EXPECT(esp_vfs_register("/vfs", NULL, NULL) == ESP_ERR_NOT_SUPPORTED);
    TEST_EXPECT(esp_vfs_unregister("/vfs")            == ESP_ERR_NOT_SUPPORTED);

    printf("[esp_vfs_fat] end" "\n");
}

void app_main(void)
{
#if VSF_ESPIDF_CFG_USE_NETIF == ENABLED
    {
        extern void vsf_netif_demo_start(void);
        vsf_netif_demo_start();
    }
#endif

    __test_total = 0;
    __test_pass  = 0;

    __test_esp_err();
    __test_esp_log();
    __test_esp_system();
    __test_esp_timer();
    __test_esp_ringbuf();
    __test_esp_heap_caps();
    __test_freertos();
    __test_freertos_queue();
    __test_freertos_semphr();
    __test_freertos_event_groups();
    __test_freertos_notify();
    __test_freertos_timers();
    __test_freertos_stream_buffer();
    __test_freertos_message_buffer();
    __test_freertos_critical();
    __test_freertos_static();
    __test_esp_event();
    __test_esp_partition();
    __test_esp_nvs();
    __test_esp_flash();
    __test_esp_vfs_littlefs();
    __test_esp_vfs_fat();

#if VSF_ESPIDF_CFG_USE_USB_HOST == ENABLED
    __test_esp_usb_host();
#endif

#if VSF_ESPIDF_CFG_USE_LCD == ENABLED
    extern void vsf_espidf_lcd_test(void);
    vsf_espidf_lcd_test();
#endif

    if (__test_pass == __test_total) {
        printf("espidf tests: %d/%d PASSED" "\n",
                       __test_pass, __test_total);
    } else {
        printf("espidf tests: %d/%d PASSED, %d FAILED"
                        "\n",
                        __test_pass, __test_total,
                        __test_total - __test_pass);
    }

#if     VSF_ESPIDF_CFG_USE_HTTP_CLIENT == ENABLED                              \
    &&  VSF_ESPIDF_CFG_USE_NETIF == ENABLED
    /* Kick off HTTP client smoke test from the linux-process pthread
     * context; the demo thread waits internally for DHCP to finish. */
    {
        extern void vsf_http_client_demo_start(void);
        vsf_http_client_demo_start();
    }
#endif
}

/* EOF */