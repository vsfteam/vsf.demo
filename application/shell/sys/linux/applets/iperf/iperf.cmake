# CMakeLists head

cmake_policy(SET CMP0079 NEW)

vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
#    __unix__
)
vsf_add_include_directories(
    ${IPERF_PATH}/port
    ${IPERF_PATH}/raw/src
)
vsf_add_sources(
    ${IPERF_PATH}/raw/src/main.c
    ${IPERF_PATH}/raw/src/cjson.c
    ${IPERF_PATH}/raw/src/iperf_api.c
    ${IPERF_PATH}/raw/src/iperf_error.c
    ${IPERF_PATH}/raw/src/iperf_auth.c
    ${IPERF_PATH}/raw/src/iperf_client_api.c
    ${IPERF_PATH}/raw/src/iperf_locale.c
    ${IPERF_PATH}/raw/src/iperf_server_api.c
    ${IPERF_PATH}/raw/src/iperf_tcp.c
    ${IPERF_PATH}/raw/src/iperf_udp.c
    ${IPERF_PATH}/raw/src/iperf_util.c
    ${IPERF_PATH}/raw/src/iperf_time.c
    ${IPERF_PATH}/raw/src/dscp.c
    ${IPERF_PATH}/raw/src/net.c
    ${IPERF_PATH}/raw/src/tcp_info.c
    ${IPERF_PATH}/raw/src/timer.c
    ${IPERF_PATH}/raw/src/units.c
)