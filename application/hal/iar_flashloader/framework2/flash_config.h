// Flash loader configuration for BH1098 IAR flashloader.
//
// Copied from IAR framework2 template (AT32 demo
// at32f402_405_qspi_algorithm_demo/iar_out/inc/flash_config.h) and
// kept in sync with the FlashInit/Write/Erase signatures implemented
// in ../flash_algorithm.c.

// When non-zero, FlashInit takes extra (argc, argv) arguments. The
// current flash_algorithm.c is prototyped for this mode and simply
// ignores argc/argv.
#define USE_ARGC_ARGV           1

#if USE_ARGC_ARGV
// Maximum argv entries reserved by framework2 in __argv[].
#define MAX_ARGS                5
// Maximum combined size of argv payload (including trailing NULLs).
#define MAX_ARG_SIZE            64
#endif

// When non-zero, flash address parameters are passed as (void *).
// Cortex-M code pointers fit in a uint32_t and are data-addressable,
// so the void * form is canonical.
#define CODE_ADDR_AS_VOID_PTR   1
