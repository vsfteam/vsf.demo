//! \note User Level Application Configuration

#ifndef __VSF_USR_CFG_H__
#define __VSF_USR_CFG_H__

/*============================ INCLUDES ======================================*/
/*============================ MACROS ========================================*/

// components

#define VSF_USE_TRACE                                   ENABLED
#define VSF_USE_MAL                                     ENABLED
#define VSF_USE_FS                                      ENABLED
#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_USB_HOST                                ENABLED
#define VSF_USE_HEAP                                    ENABLED

// linux

#define VSF_USE_LINUX                                   ENABLED
#   define VSF_LINUX_LIBC_USE_ENVIRON                   ENABLED
#   define VSF_LINUX_USE_SIMPLE_LIBC                    ENABLED
#   define VSF_LINUX_USE_LIBUSB                         VSF_USE_USB_HOST
#   define VSF_LINUX_USE_SOCKET                         ENABLED
#   define VSF_LINUX_USE_APPLET                         ENABLED

// APP configuration

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif
/* EOF */
