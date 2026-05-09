/*
 * Example adapter — aggregates test/demo source files when no specific
 * example is being tested.
 *
 * This translation unit #include-s the test .c files from test/ so that
 * they are compiled into the default build.  When a user provides their
 * own example (replacing this file in the vcxproj), the test code is
 * simply not pulled in.
 *
 * Import order matters: the main test suite (vsf_espidf_test.c) provides
 * app_main() and must come first; sub-system demos that export functions
 * called from app_main follow and are guarded by the same config switches
 * that appear in app_main.
 */

#include <vsf.h>
#include <vsf_board.h>

/* ---- main test suite (provides app_main) ------------------------------ */
#include "test/vsf_espidf_test.c"

/* ---- sub-system demos (export functions called from app_main) --------- */

#if VSF_ESPIDF_CFG_USE_NETIF == ENABLED
#   include "test/vsf_netif_demo.c"
#endif

#if VSF_ESPIDF_CFG_USE_USB_HOST == ENABLED
#   include "test/vsf_espidf_usb_test.c"
#endif

#if VSF_ESPIDF_CFG_USE_LCD == ENABLED
#   include "test/vsf_espidf_lcd_test.c"
#endif

#if     VSF_ESPIDF_CFG_USE_HTTP_CLIENT == ENABLED                              \
    &&  VSF_ESPIDF_CFG_USE_NETIF == ENABLED
#   include "test/vsf_http_client_demo.c"
#endif
