#include <unistd.h>
#include <btstack.h>

// implement btstack_main if not in applet
#ifndef __VSF_APPLET__
#   define main         btstack_main
#endif

extern const btstack_run_loop_t * app_btstack_get_run_loop(void);
extern const hci_transport_t * app_btstack_get_hci_transport(void);
extern const btstack_chipset_t * app_btstack_get_chipset(void);

int main(int argc, char *argv[])
{
    int (*entry)(int argc, char **argv) = NULL;

    if (!strcmp(argv[0], "btstack_scan")) {
        extern int btstack_scan_main(int, char**);
        entry = btstack_scan_main;
    } else if (!strcmp(argv[0], "btstack")) {
        printf("do not call %s directly\n", argv[0]);
    } else {
        printf("not supported btstack command: %s\n", argv[0]);
    }

    if (entry != NULL) {
        btstack_memory_init();
        btstack_run_loop_init(app_btstack_get_run_loop());
        hci_init(app_btstack_get_hci_transport(), NULL);
        hci_set_chipset(app_btstack_get_chipset());
        hci_set_link_key_db(btstack_link_key_db_memory_instance());

        entry(argc, argv);
    }
    return 0;
}
