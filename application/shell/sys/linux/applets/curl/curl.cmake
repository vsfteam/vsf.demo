# CMakeLists head

cmake_policy(SET CMP0079 NEW)

file(GLOB CURL_SOURCES
    raw/lib/vauth/*.c
    raw/lib/vquit/*.c
    raw/lib/vssh/*.c
    raw/lib/vtls/*.c
    raw/lib/*.c
    raw/src/*.c

    port/curl_port_vsf.c
)
vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    __VSF__
    VSF_USE_MBEDTLS=ENABLED

    HAVE_CONFIG_H
)
vsf_add_include_directories(
    port
    raw/include
    raw/lib

    $ENV{VSF_PATH}/source/component/3rd-party/mbedtls/raw/include
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
)
vsf_add_sources(
    ${CURL_SOURCES}
)
