vsf_add_include_directories(
    ./
    $ENV{VSF_PATH}
)

# Application entry point only — test framework + suites are pulled in
# automatically via the vsf_test CMake library below.
vsf_add_sources(main.c)

# Pull in the VSF test framework library (framework + all peripheral suites).
add_subdirectory($ENV{VSF_PATH}/test/vsf_test_suite ${CMAKE_CURRENT_BINARY_DIR}/vsf_test_suite_bin)

# Kernel profile selection. Pass via cmake -DVSF_KERNEL_PROFILE=VSF_TEST_KERNEL_PROFILE_<NAME>.
# Defaults to baremetal when undefined (see vsf_usr_cfg.h).
if(DEFINED VSF_KERNEL_PROFILE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC ${VSF_KERNEL_PROFILE})
endif()
