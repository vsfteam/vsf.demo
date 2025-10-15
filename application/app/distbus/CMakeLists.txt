vsf_add_include_directories(
    ./
)
vsf_add_sources(
    main.c
)

add_subdirectory(hal)
add_subdirectory(transport)
