# CMakeLists head

cmake_policy(SET CMP0079 NEW)

vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
#    __unix__
)
vsf_add_include_directories(
    ./port
    raw/src
)
vsf_add_sources(
    raw/src/main.c
    raw/src/cjson.c
    raw/src/iperf_api.c
    raw/src/iperf_error.c
    raw/src/iperf_auth.c
    raw/src/iperf_client_api.c
    raw/src/iperf_locale.c
    raw/src/iperf_server_api.c
    raw/src/iperf_tcp.c
    raw/src/iperf_udp.c
    raw/src/iperf_util.c
    raw/src/iperf_time.c
    raw/src/dscp.c
    raw/src/net.c
    raw/src/tcp_info.c
    raw/src/timer.c
    raw/src/units.c
)