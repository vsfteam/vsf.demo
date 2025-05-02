cmake_policy(SET CMP0079 NEW)

set(USBIPD_PATH ${USBIPD_APPLET_PATH}/raw)
file(GLOB_RECURSE USBIPD_SOURCES ${USBIPD_PATH}/*.c)

vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    VSF_COMPATIBILITY=DISABLED
)
vsf_add_include_directories(
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
    ${USBIPD_PATH}/include
    $ENV{VSF_PATH}/source/shell/sys/linux/include/libusb
)
vsf_add_sources(
    ${USBIPD_SOURCES}
)
