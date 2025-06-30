#define __VSF_LINUX_FS_CLASS_INHERIT__      // for vsf_linux_fs_get_target
#include <unistd.h>
#include <btstack.h>
#include "btstack_run_loop_posix.h"
#include "hci_transport.h"
#include "hci_transport_h4.h"
#include "hci_dump_posix_stdout.h"

// implement btstack_main if not in applet
#ifndef __VSF_APPLET__
#   define main         btstack_main
#else
VSF_CAL_WEAK(btstack_main)
int btstack_main(int argc, char **argv)
{
    vsf_trace_warning("please add btstack default application\n");
    hci_power_control(HCI_POWER_OFF);
    return 0;
}
#endif

int main(int argc, char *argv[])
{
    int (*entry)(int argc, char **argv) = NULL;

    if (!strcmp(argv[0], "btstack_scan")) {
        extern int btstack_scan_main(int, char**);
        entry = btstack_scan_main;
    } else if (!strcmp(argv[0], "btstack")) {
#ifndef __VSF_APPLET__
        printf("do not call %s directly\n", argv[0]);
#else
        // normal btstack application entry
        extern int btstack_main(int, char**);
        entry = btstack_main;
#endif
    } else {
        printf("not supported btstack command: %s\n", argv[0]);
        return -1;
    }

    char *devhci = argc == 2 ? argv[1] : NULL;

    if (entry != NULL) {
        btstack_memory_init();

        if (devhci != NULL) {
            vsf_linux_bthci_t *bthci;
            if (vsf_linux_fs_get_target(devhci, (void **)&bthci) < 0) {
                printf("failed to open %s\n", devhci);
                return -1;
            }

#   if defined(ENABLE_LOG_INFO) || defined(ENABLE_LOG_DEBUG) || defined(ENABLE_LOG_ERROR)
            hci_dump_init(hci_dump_posix_stdout_get_instance());
#   endif

            btstack_run_loop_init(btstack_run_loop_posix_get_instance());
            hci_init(hci_transport_h4_instance_for_uart(btstack_uart_posix_instance()), &(hci_transport_config_uart_t){
                .device_name    = devhci,
                .type           = HCI_TRANSPORT_CONFIG_UART,
                .baudrate_init  = 115200,
                .baudrate_main  = 0,
                .flowcontrol    = 1,
            });
            hci_set_chipset(bthci->chipset_instance);
        } else {
#   if defined(ENABLE_LOG_INFO) || defined(ENABLE_LOG_DEBUG) || defined(ENABLE_LOG_ERROR)
            extern const hci_dump_t hci_dump_vsf_impl;
            hci_dump_init(&hci_dump_vsf_impl);
#   endif

            extern const btstack_run_loop_t * app_btstack_get_run_loop(void);
            extern const hci_transport_t * app_btstack_get_hci_transport(void);
            extern const btstack_chipset_t * app_btstack_get_chipset(void);
            extern void * app_btstack_get_hci_transport_config(void);
            btstack_run_loop_init(app_btstack_get_run_loop());
            hci_init(app_btstack_get_hci_transport(), app_btstack_get_hci_transport_config());
            hci_set_chipset(app_btstack_get_chipset());
        }

        hci_set_link_key_db(btstack_link_key_db_memory_instance());

        entry(argc, argv);

        btstack_run_loop_deinit();
    }
    return 0;
}
