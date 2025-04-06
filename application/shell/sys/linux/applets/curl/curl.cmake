# CMakeLists head

cmake_policy(SET CMP0079 NEW)

file(GLOB CURL_SOURCES
    ${CURL_PATH}/raw/lib/vauth/*.c
    ${CURL_PATH}/raw/lib/vquit/*.c
    ${CURL_PATH}/raw/lib/vssh/*.c
    ${CURL_PATH}/raw/lib/vtls/*.c
    ${CURL_PATH}/raw/lib/*.c
    ${CURL_PATH}/raw/src/*.c

    ${CURL_PATH}/port/curl_port_vsf.c
)
vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    __VSF__
    VSF_USE_MBEDTLS=ENABLED

    HAVE_CONFIG_H
)
vsf_add_include_directories(
    ${CURL_PATH}/port
    ${CURL_PATH}/raw/include
    ${CURL_PATH}/raw/lib

    $ENV{VSF_PATH}/source/component/3rd-party/mbedtls/raw/include
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
)
vsf_add_sources(
    ${CURL_SOURCES}
)
