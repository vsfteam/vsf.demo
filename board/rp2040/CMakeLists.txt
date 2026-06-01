# Generate test parameters header from YAML (run manually or via CMake)
set(TEST_PARAMS_YML ${CMAKE_CURRENT_SOURCE_DIR}/test_params.yml)
set(TEST_PARAMS_H   ${CMAKE_CURRENT_SOURCE_DIR}/test_params_generated.h)
set(VSF_TEST_GLOBAL_PARAMS_DIR $ENV{VSF_PATH}/test/vsf_test/params)

find_program(PYTHON3_CMD python3 REQUIRED)
set(Python3_EXECUTABLE ${PYTHON3_CMD})

# Discover YAML files automatically.  Adding/removing a .yml requires
# re-running CMake (standard for test-params generation anyway).
# Local YAMLs override global YAMLs with the same name.
file(GLOB PERIPH_YMLS   ${CMAKE_CURRENT_SOURCE_DIR}/*.yml)
file(GLOB SCENARIO_YMLS ${CMAKE_CURRENT_SOURCE_DIR}/scenario/*.yml)
file(GLOB GLOBAL_PERIPH_YMLS   ${VSF_TEST_GLOBAL_PARAMS_DIR}/*.yml)
file(GLOB GLOBAL_SCENARIO_YMLS ${VSF_TEST_GLOBAL_PARAMS_DIR}/scenario/*.yml)

add_custom_command(
    OUTPUT ${TEST_PARAMS_H}
    COMMAND ${CMAKE_COMMAND} -E env VSF_TEST_GLOBAL_PARAMS_DIR=${VSF_TEST_GLOBAL_PARAMS_DIR}
        PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/../../vsf/test
        ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../vsf/test/vsf_bench/test_params_codegen.py
        ${TEST_PARAMS_YML} ${TEST_PARAMS_H}
    DEPENDS
        ${TEST_PARAMS_YML}
        ${PERIPH_YMLS}
        ${SCENARIO_YMLS}
        ${GLOBAL_PERIPH_YMLS}
        ${GLOBAL_SCENARIO_YMLS}
    COMMENT "Generating test parameters header"
)

add_custom_target(vsf_test_gen_params DEPENDS ${TEST_PARAMS_H})

vsf_add_include_directories(
    ./
)
vsf_add_sources(
    vsf_board.c
    vsf_board_test.c
)
