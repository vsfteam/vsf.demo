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
 * ESP-IDF USB Host Library smoke test.
 *
 * Exercises the usb/usb_host.h API bridged to VSF-USBH.
 *
 * Prerequisites (vc.espidf on Windows):
 *   1.  A USB device with a known VID/PID plugged into the host.
 *       Suggested: any USB flash drive (VID=0x0781 PID=0x5591 SanDisk
 *       or similar).
 *   2.  The board config (board/win/vsf_board_cfg.h) must have the
 *       device's VID/PID set in VSF_WINUSB_HCD_DEV0_VID / _DEV0_PID
 *       so WinUSB binds to that device.  See "Test setup" below.
 *   3.  The WinUSB driver must be installed for the device (use
 *       Zadig: https://zadig.akeo.ie/ to replace the default driver
 *       with WinUSB for the target device).
 *
 * Test flow:
 *   1.  Install USB Host Library.
 *   2.  Register a client that logs NEW_DEV / DEV_GONE events.
 *   3.  Open each connected device, print its device descriptor.
 *   4.  Wait 15 seconds, then tear down.
 */

/*============================ INCLUDES ======================================*/

#include "esp_err.h"
#include "esp_log.h"
#include "usb/usb_host.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>

/*============================ LOCAL VARIABLES ===============================*/

static const char *__usb_tag = "usb_test";

static SemaphoreHandle_t __test_done;

/*============================ TEST IMPLEMENTATION ===========================*/

/* Client event callback -- dispatched from usb_host_client_handle_events() */
static void __client_event_cb(const usb_host_client_event_msg_t *event_msg,
                               void *arg)
{
    if (event_msg->event == USB_HOST_CLIENT_EVENT_NEW_DEV) {
        ESP_LOGI(__usb_tag, "NEW_DEV: address=%d", event_msg->new_dev.address);

        /* Open device and print descriptors */
        usb_device_handle_t dev_hdl;
        esp_err_t err = usb_host_device_open((usb_host_client_handle_t)arg,
                                             event_msg->new_dev.address,
                                             &dev_hdl);
        if (err != ESP_OK) {
            ESP_LOGE(__usb_tag, "Failed to open device: 0x%x", err);
            return;
        }

        /* Device info */
        usb_device_info_t info;
        err = usb_host_device_info(dev_hdl, &info);
        if (err == ESP_OK) {
            ESP_LOGI(__usb_tag, "  Speed: %s",
                info.speed == 0 ? "Low" : info.speed == 1 ? "Full" : "High");
            ESP_LOGI(__usb_tag, "  Config value: %d", info.bConfigurationValue);
        }

        /* Device descriptor */
        const usb_device_desc_t *dev_desc;
        err = usb_host_get_device_descriptor(dev_hdl, &dev_desc);
        if (err == ESP_OK) {
            TEST_EXPECT(dev_desc->bLength >= 18);
            TEST_EXPECT(dev_desc->bDescriptorType == 0x01);
            ESP_LOGI(__usb_tag, "  VID=0x%04X PID=0x%04X class=0x%02X cfgCnt=%d",
                dev_desc->idVendor, dev_desc->idProduct,
                dev_desc->bDeviceClass, dev_desc->bNumConfigurations);
        }

        /* Config descriptor */
        const usb_config_desc_t *cfg_desc;
        err = usb_host_get_active_config_descriptor(dev_hdl, &cfg_desc);
        if (err == ESP_OK) {
            TEST_EXPECT(cfg_desc->bDescriptorType == 0x02);
            TEST_EXPECT(cfg_desc->bNumInterfaces > 0);
            ESP_LOGI(__usb_tag, "  Config: ifs=%d attrs=0x%02X power=%d",
                cfg_desc->bNumInterfaces, cfg_desc->bmAttributes,
                cfg_desc->bMaxPower);
        }

        usb_host_device_close((usb_host_client_handle_t)arg, dev_hdl);

    } else if (event_msg->event == USB_HOST_CLIENT_EVENT_DEV_GONE) {
        ESP_LOGI(__usb_tag, "DEV_GONE");
    }
}

void __test_esp_usb_host(void)
{
    printf("\n========== [usb_host] ESP-IDF USB Host Library Test ==========\n\n");

    /* ---- 1. Install USB Host ---- */
    ESP_LOGI(__usb_tag, "Installing USB Host Library...");
    usb_host_config_t host_cfg = { .skip_phy_setup = true };
    esp_err_t err = usb_host_install(&host_cfg);
    TEST_EXPECT(err == ESP_OK);
    if (err != ESP_OK) {
        ESP_LOGE(__usb_tag, "usb_host_install failed: 0x%x (check board cfg)", err);
        printf("\n  SUMMARY: %d / %d passed\n", __test_pass, __test_total);
        return;
    }

    /* ---- 2. Register client ---- */
    ESP_LOGI(__usb_tag, "Registering client...");
    usb_host_client_handle_t client_hdl = NULL;
    usb_host_client_config_t client_cfg = {
        .is_synchronous     = false,
        .max_num_event_msg  = 5,
        .async = {
            .client_event_callback = __client_event_cb,
            .callback_arg          = client_hdl,  /* will be set after register */
        },
    };
    err = usb_host_client_register(&client_cfg, &client_hdl);
    TEST_EXPECT(err == ESP_OK);
    TEST_EXPECT(client_hdl != NULL);
    if (err != ESP_OK) {
        usb_host_uninstall();
        printf("\n  SUMMARY: %d / %d passed\n", __test_pass, __test_total);
        return;
    }
    /* Fix up the callback arg (client_hdl assigned by register) */
    client_cfg.async.callback_arg = client_hdl;

    /* ---- 3. Wait for device events ---- */
    ESP_LOGI(__usb_tag, "Waiting for USB device (15 seconds)...");
    ESP_LOGI(__usb_tag, "  (Plug in a USB flash drive with WinUSB driver installed)");

    __test_done = xSemaphoreCreateBinary();
    if (!__test_done) {
        ESP_LOGE(__usb_tag, "Failed to create semaphore");
        goto cleanup;
    }

    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(15000);
    int event_count = 0;

    while (xTaskGetTickCount() < deadline) {
        /* Handle client events (non-blocking, 100ms timeout) */
        err = usb_host_client_handle_events(client_hdl, pdMS_TO_TICKS(100));
        if (err == ESP_OK) {
            event_count++;
        } else if (err != ESP_ERR_TIMEOUT) {
            ESP_LOGW(__usb_tag, "client_handle_events: 0x%x", err);
        }

        /* Also drive library events briefly */
        uint32_t lib_flags = 0;
        usb_host_lib_handle_events(0, &lib_flags);
    }

    TEST_EXPECT(event_count > 0);
    ESP_LOGI(__usb_tag, "Event loop finished, %d events processed", event_count);

    if (event_count == 0) {
        ESP_LOGW(__usb_tag, "No USB device detected. Check:");
        ESP_LOGW(__usb_tag, "  1. Device is plugged in");
        ESP_LOGW(__usb_tag, "  2. WinUSB driver installed (use Zadig)");
        ESP_LOGW(__usb_tag, "  3. VID/PID match board/win/vsf_board_cfg.h");
    }

    /* ---- 4. Lib info ---- */
    usb_host_lib_info_t lib_info;
    err = usb_host_lib_info(&lib_info);
    TEST_EXPECT(err == ESP_OK);
    ESP_LOGI(__usb_tag, "Library: %d device(s), %d client(s)",
             lib_info.num_devices, lib_info.num_clients);

cleanup:
    /* ---- 5. Tear down ---- */
    usb_host_client_deregister(client_hdl);
    usb_host_uninstall();
    if (__test_done) vSemaphoreDelete(__test_done);

    printf("\n==================== USB Host Test SUMMARY ====================\n");
    printf("  %d / %d passed\n", __test_pass, __test_total);
    printf("==============================================================\n\n");
}
