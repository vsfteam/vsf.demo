#include <unistd.h>
#include <vsf_board.h>

#if VSF_USE_UI == ENABLED
#   include <sgl.h>
#   include <sgl_platform.h>

void demo_setup(void)
{
    sgl_obj_t *main_page = sgl_page_create();
    sgl_page_set_active(main_page);
}

int ui_main(int argc, char **argv)
{
    if (sgl_platform_bind_vsf(vsf_board.display_dev, true) != VSF_ERR_NONE) {
        printf("fail to bind vsf to sgl\n");
        return -1;
    }

    sgl_init();
    demo_setup();

    while (1) {
        sgl_tick_inc(5);
        sgl_task_handler();
        usleep(5 * 1000);
    }
}
#endif
