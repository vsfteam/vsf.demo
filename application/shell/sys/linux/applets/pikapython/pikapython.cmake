# CMakeLists head

cmake_policy(SET CMP0079 NEW)

if(NOT DEFINED ENV{PIKA_PATH})
    set(ENV{PIKA_PATH} ${PIKAPYTHON_PATH}/raw)
endif()
message(STATUS "PIKA_PATH: $ENV{PIKA_PATH}")

vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    __linux=1

    # vsf use pikapython
    VSF_USE_PIKAPYTHON=1
)
vsf_add_include_directories(
    $ENV{PIKA_PATH}/pikapython/pikascript-core
    $ENV{PIKA_PATH}/pikapython/pikascript-api
)
file(GLOB_RECURSE PIKA_FILES $ENV{PIKA_PATH}/pikapython/*.c)
vsf_add_sources(
#    $ENV{VSF_PATH}/source/component/script/python/module/os/vsf_python_module_os.c

    ${PIKA_FILES}
    $ENV{PIKA_PATH}/main.c
)