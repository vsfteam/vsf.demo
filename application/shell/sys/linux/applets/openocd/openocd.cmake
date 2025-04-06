# CMakeLists head

cmake_policy(SET CMP0079 NEW)

file(GLOB OPENOCD_SOURCES
    ${OPENOCD_PATH}/port/jimtcl/_glob.c
    ${OPENOCD_PATH}/port/jimtcl/_initjimsh.c
    ${OPENOCD_PATH}/port/jimtcl/_jsonencode.c
    ${OPENOCD_PATH}/port/jimtcl/_load-static-exts.c
    ${OPENOCD_PATH}/port/jimtcl/_nshelper.c
    ${OPENOCD_PATH}/port/jimtcl/_oo.c
    ${OPENOCD_PATH}/port/jimtcl/_stdlib.c
    ${OPENOCD_PATH}/port/jimtcl/_tclcompat.c
    ${OPENOCD_PATH}/port/jimtcl/_tree.c

    ${OPENOCD_PATH}/raw/jimtcl/jsmn/jsmn.c
    ${OPENOCD_PATH}/raw/jimtcl/jim.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-aio.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-array.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-clock.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-eventloop.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-exec.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-file.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-format.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-history.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-interactive.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-interp.c
    ${OPENOCD_PATH}/raw/jimtcl/jimiocompat.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-json.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-load.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-namespace.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-pack.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-package.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-posix.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-readdir.c
    ${OPENOCD_PATH}/raw/jimtcl/jimregexp.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-regexp.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-signal.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-subcmd.c
    ${OPENOCD_PATH}/raw/jimtcl/jim-tty.c
    ${OPENOCD_PATH}/raw/jimtcl/linenoise.c
    ${OPENOCD_PATH}/raw/jimtcl/utf8.c

#    ${OPENOCD_PATH}/raw/src/flash/nand/arm_io.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/at91sam9.c
    ${OPENOCD_PATH}/raw/src/flash/nand/core.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/davinci.c
    ${OPENOCD_PATH}/raw/src/flash/nand/driver.c
    ${OPENOCD_PATH}/raw/src/flash/nand/ecc.c
    ${OPENOCD_PATH}/raw/src/flash/nand/ecc_kw.c
    ${OPENOCD_PATH}/raw/src/flash/nand/fileio.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/lpc32xx.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/lpc3180.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/mx3.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/msc.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/nonce.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/nuc910.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/orion.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/s3c24xx.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/s3c2410.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/s3c2412.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/s3c2440.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/s3c2443.c
#    ${OPENOCD_PATH}/raw/src/flash/nand/s3c6400.c
    ${OPENOCD_PATH}/raw/src/flash/nand/tcl.c

#    ${OPENOCD_PATH}/raw/src/flash/nor/aduc702x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/aducm360.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/ambiqmicro.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/at91sam3.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/at91sam4.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/at91sam4l.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/at91sam7.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/at91samd.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/ath79.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/atsame5.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/atsamv.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/avrf.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/bluenrg-x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/cc26xx.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/cc3220sf.c
    ${OPENOCD_PATH}/raw/src/flash/nor/cfi.c
    ${OPENOCD_PATH}/raw/src/flash/nor/core.c
    ${OPENOCD_PATH}/raw/src/flash/nor/drivers.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/dsp5680xx_flash.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/efm32.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/em357.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/esirisc_flash.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/faux.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/fespi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/fm3.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/fm4.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/jtagspi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/kinetis.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/lpc288x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/lpc2000.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/lpc2900.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/lpcspifi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/mdr.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/mrvlqspi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/msp432.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/niietcm4.c
    ${OPENOCD_PATH}/raw/src/flash/nor/non_cfi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/npcx.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/nrf5.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/numicro.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/ocl.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/pic32mx.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/psoc4.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/psoc5lp.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/psoc6.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/renesas_rpchf.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/rp2040.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/sfdp.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/sh_qspi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/sim3x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/spi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/stellaris.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/stm32f1x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/stm32f2x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/stm32h7x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/stm32l4x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/stm32lx.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/stmqspi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/stmsmi.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/str7x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/str9x.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/str9xpec.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/swm050.c
    ${OPENOCD_PATH}/raw/src/flash/nor/tcl.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/tms470.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/virtual.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/w600.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/xcf.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/xmc1xxx.c
#    ${OPENOCD_PATH}/raw/src/flash/nor/xmc4xxx.c

    ${OPENOCD_PATH}/raw/src/flash/common.c

    ${OPENOCD_PATH}/raw/src/helper/binarybuffer.c
    ${OPENOCD_PATH}/raw/src/helper/command.c
    ${OPENOCD_PATH}/raw/src/helper/configuration.c
    ${OPENOCD_PATH}/raw/src/helper/fileio.c
    ${OPENOCD_PATH}/raw/src/helper/jep106.c
    ${OPENOCD_PATH}/raw/src/helper/jim-nvp.c
    ${OPENOCD_PATH}/raw/src/helper/log.c
    ${OPENOCD_PATH}/raw/src/helper/options.c
    ${OPENOCD_PATH}/raw/src/helper/replacements.c
    ${OPENOCD_PATH}/raw/src/helper/time_support.c
    ${OPENOCD_PATH}/raw/src/helper/time_support_common.c
    ${OPENOCD_PATH}/raw/src/helper/util.c

    ${OPENOCD_PATH}/raw/src/jtag/drivers/cmsis_dap.c
    ${OPENOCD_PATH}/raw/src/jtag/drivers/cmsis_dap_usb_bulk.c
    ${OPENOCD_PATH}/raw/src/jtag/drivers/driver.c

    ${OPENOCD_PATH}/raw/src/jtag/adapter.c
    ${OPENOCD_PATH}/raw/src/jtag/commands.c
    ${OPENOCD_PATH}/raw/src/jtag/core.c
    ${OPENOCD_PATH}/raw/src/jtag/interface.c
    ${OPENOCD_PATH}/raw/src/jtag/interfaces.c
    ${OPENOCD_PATH}/raw/src/jtag/swim.c
    ${OPENOCD_PATH}/raw/src/jtag/tcl.c

    ${OPENOCD_PATH}/raw/src/pld/pld.c
    ${OPENOCD_PATH}/raw/src/pld/virtex2.c
    ${OPENOCD_PATH}/raw/src/pld/xilinx_bit.c

#    ${OPENOCD_PATH}/raw/src/rtos/chibios.c
#    ${OPENOCD_PATH}/raw/src/rtos/chromium-ec.c
#    ${OPENOCD_PATH}/raw/src/rtos/eCos.c
#    ${OPENOCD_PATH}/raw/src/rtos/embKernel.c
#    ${OPENOCD_PATH}/raw/src/rtos/FreeRTOS.c
    ${OPENOCD_PATH}/raw/src/rtos/hwthread.c
#    ${OPENOCD_PATH}/raw/src/rtos/linux.c
#    ${OPENOCD_PATH}/raw/src/rtos/mqx.c
#    ${OPENOCD_PATH}/raw/src/rtos/nuttx.c
#    ${OPENOCD_PATH}/raw/src/rtos/riot.c
    ${OPENOCD_PATH}/raw/src/rtos/rtos.c
#    ${OPENOCD_PATH}/raw/src/rtos/rtos_chibios_stackings.c
#    ${OPENOCD_PATH}/raw/src/rtos/rtos_ecos_stackings.c
#    ${OPENOCD_PATH}/raw/src/rtos/rtos_embkernel_stackings.c
#    ${OPENOCD_PATH}/raw/src/rtos/rtos_mqx_stackings.c
#    ${OPENOCD_PATH}/raw/src/rtos/rtos_riot_stackings.c
    ${OPENOCD_PATH}/raw/src/rtos/rtos_standard_stackings.c
#    ${OPENOCD_PATH}/raw/src/rtos/rtos_ucos_iii_stackings.c
#    ${OPENOCD_PATH}/raw/src/rtos/ThreadX.c
#    ${OPENOCD_PATH}/raw/src/rtos/uCOS-III.c
#    ${OPENOCD_PATH}/raw/src/rtos/zephyr.c

    ${OPENOCD_PATH}/raw/src/rtt/rtt.c
    ${OPENOCD_PATH}/raw/src/rtt/tcl.c

    ${OPENOCD_PATH}/raw/src/server/gdb_server.c
    ${OPENOCD_PATH}/raw/src/server/ipdbg.c
    ${OPENOCD_PATH}/raw/src/server/rtt_server.c
    ${OPENOCD_PATH}/raw/src/server/server.c
    ${OPENOCD_PATH}/raw/src/server/tcl_server.c
    ${OPENOCD_PATH}/raw/src/server/telnet_server.c

    ${OPENOCD_PATH}/raw/src/svf/svf.c

#    ${OPENOCD_PATH}/raw/src/target/openrisc/jsp_server.c
#    ${OPENOCD_PATH}/raw/src/target/openrisc/or1k.c
#    ${OPENOCD_PATH}/raw/src/target/openrisc/or1k_du_adv.c
#    ${OPENOCD_PATH}/raw/src/target/openrisc/or1k_tap_mohor.c
#    ${OPENOCD_PATH}/raw/src/target/openrisc/or1k_tap_vjtag.c
#    ${OPENOCD_PATH}/raw/src/target/openrisc/or1k_tap_xilinx_bscan.c
    ${OPENOCD_PATH}/raw/src/target/riscv/batch.c
    ${OPENOCD_PATH}/raw/src/target/riscv/program.c
    ${OPENOCD_PATH}/raw/src/target/riscv/riscv.c
    ${OPENOCD_PATH}/raw/src/target/riscv/riscv_semihosting.c
    ${OPENOCD_PATH}/raw/src/target/riscv/riscv-011.c
    ${OPENOCD_PATH}/raw/src/target/riscv/riscv-013.c
#    ${OPENOCD_PATH}/raw/src/target/a64_disassembler.c
#    ${OPENOCD_PATH}/raw/src/target/aarch64.c
    ${OPENOCD_PATH}/raw/src/target/adi_v5_dapdirect.c
    ${OPENOCD_PATH}/raw/src/target/adi_v5_jtag.c
    ${OPENOCD_PATH}/raw/src/target/adi_v5_swd.c
    ${OPENOCD_PATH}/raw/src/target/algorithm.c
#    ${OPENOCD_PATH}/raw/src/target/arc.c
#    ${OPENOCD_PATH}/raw/src/target/arc_cmd.c
#    ${OPENOCD_PATH}/raw/src/target/arc_jtag.c
#    ${OPENOCD_PATH}/raw/src/target/arc_mem.c
    ${OPENOCD_PATH}/raw/src/target/arm_adi_v5.c
    ${OPENOCD_PATH}/raw/src/target/arm_cti.c
    ${OPENOCD_PATH}/raw/src/target/arm_dap.c
    ${OPENOCD_PATH}/raw/src/target/arm_disassembler.c
    ${OPENOCD_PATH}/raw/src/target/arm_dpm.c
    ${OPENOCD_PATH}/raw/src/target/arm_jtag.c
    ${OPENOCD_PATH}/raw/src/target/arm_semihosting.c
    ${OPENOCD_PATH}/raw/src/target/arm_simulator.c
    ${OPENOCD_PATH}/raw/src/target/arm_tpiu_swo.c
    ${OPENOCD_PATH}/raw/src/target/arm7_9_common.c
    ${OPENOCD_PATH}/raw/src/target/arm7tdmi.c
    ${OPENOCD_PATH}/raw/src/target/arm9tdmi.c
    ${OPENOCD_PATH}/raw/src/target/arm11.c
    ${OPENOCD_PATH}/raw/src/target/arm11_dbgtap.c
    ${OPENOCD_PATH}/raw/src/target/arm720t.c
    ${OPENOCD_PATH}/raw/src/target/arm920t.c
    ${OPENOCD_PATH}/raw/src/target/arm926ejs.c
    ${OPENOCD_PATH}/raw/src/target/arm946e.c
    ${OPENOCD_PATH}/raw/src/target/arm966e.c
    ${OPENOCD_PATH}/raw/src/target/armv4_5.c
    ${OPENOCD_PATH}/raw/src/target/armv4_5_cache.c
    ${OPENOCD_PATH}/raw/src/target/armv4_5_mmu.c
    ${OPENOCD_PATH}/raw/src/target/armv7a.c
    ${OPENOCD_PATH}/raw/src/target/armv7a_cache.c
    ${OPENOCD_PATH}/raw/src/target/armv7a_cache_l2x.c
    ${OPENOCD_PATH}/raw/src/target/armv7a_mmu.c
    ${OPENOCD_PATH}/raw/src/target/armv7m.c
    ${OPENOCD_PATH}/raw/src/target/armv7m_trace.c
    ${OPENOCD_PATH}/raw/src/target/armv8.c
    ${OPENOCD_PATH}/raw/src/target/armv8_cache.c
    ${OPENOCD_PATH}/raw/src/target/armv8_dpm.c
    ${OPENOCD_PATH}/raw/src/target/armv8_opcodes.c
#    ${OPENOCD_PATH}/raw/src/target/avr32_ap7k.c
#    ${OPENOCD_PATH}/raw/src/target/avr32_jtag.c
#    ${OPENOCD_PATH}/raw/src/target/avr32_mem.c
#    ${OPENOCD_PATH}/raw/src/target/avr32_regs.c
#    ${OPENOCD_PATH}/raw/src/target/avrt.c
    ${OPENOCD_PATH}/raw/src/target/breakpoints.c
    ${OPENOCD_PATH}/raw/src/target/cortex_a.c
    ${OPENOCD_PATH}/raw/src/target/cortex_m.c
#    ${OPENOCD_PATH}/raw/src/target/dsp563xx.c
#    ${OPENOCD_PATH}/raw/src/target/dsp563xx_once.c
#    ${OPENOCD_PATH}/raw/src/target/dsp5680xx.c
    ${OPENOCD_PATH}/raw/src/target/embeddedice.c
#    ${OPENOCD_PATH}/raw/src/target/esirisc.c
#    ${OPENOCD_PATH}/raw/src/target/esirisc_jtag.c
#    ${OPENOCD_PATH}/raw/src/target/esirisc_trace.c
    ${OPENOCD_PATH}/raw/src/target/etb.c
    ${OPENOCD_PATH}/raw/src/target/etm.c
    ${OPENOCD_PATH}/raw/src/target/etm_dummy.c
#    ${OPENOCD_PATH}/raw/src/target/fa526.c
#    ${OPENOCD_PATH}/raw/src/target/feroceon.c
#    ${OPENOCD_PATH}/raw/src/target/hla_target.c
    ${OPENOCD_PATH}/raw/src/target/image.c
#    ${OPENOCD_PATH}/raw/src/target/lakemont.c
#    ${OPENOCD_PATH}/raw/src/target/ls1_sap.c
    ${OPENOCD_PATH}/raw/src/target/mem_ap.c
#    ${OPENOCD_PATH}/raw/src/target/mips_ejtag.c
#    ${OPENOCD_PATH}/raw/src/target/mips_m4k.c
#    ${OPENOCD_PATH}/raw/src/target/mips_mips64.c
#    ${OPENOCD_PATH}/raw/src/target/mips32.c
#    ${OPENOCD_PATH}/raw/src/target/mips32_dmaacc.c
#    ${OPENOCD_PATH}/raw/src/target/mips32_pracc.c
#    ${OPENOCD_PATH}/raw/src/target/mips64.c
#    ${OPENOCD_PATH}/raw/src/target/mips64_pracc.c
#    ${OPENOCD_PATH}/raw/src/target/nds32.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_aice.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_cmd.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_disassembler.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_reg.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_tlb.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_v2.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_v3.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_v3_common.c
#    ${OPENOCD_PATH}/raw/src/target/nds32_v3m.c
#    ${OPENOCD_PATH}/raw/src/target/quark_d20xx.c
#    ${OPENOCD_PATH}/raw/src/target/quark_x10xx.c
    ${OPENOCD_PATH}/raw/src/target/register.c
    ${OPENOCD_PATH}/raw/src/target/rtt.c
    ${OPENOCD_PATH}/raw/src/target/semihosting_common.c
    ${OPENOCD_PATH}/raw/src/target/smp.c
#    ${OPENOCD_PATH}/raw/src/target/stm8.c
    ${OPENOCD_PATH}/raw/src/target/target.c
    ${OPENOCD_PATH}/raw/src/target/target_request.c
#    ${OPENOCD_PATH}/raw/src/target/testee.c
    ${OPENOCD_PATH}/raw/src/target/trace.c
#    ${OPENOCD_PATH}/raw/src/target/x86_32_common.c
#    ${OPENOCD_PATH}/raw/src/target/xscale.c

    ${OPENOCD_PATH}/raw/src/transport/transport.c

    ${OPENOCD_PATH}/raw/src/xsvf/xsvf.c

    ${OPENOCD_PATH}/raw/src/hello.c
    ${OPENOCD_PATH}/raw/src/main.c
    ${OPENOCD_PATH}/raw/src/openocd.c
)
vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    __VSF__
    VSF_COMPATIBILITY=DISABLED

    HAVE_CONFIG_H
    SIGWINCH=28
)
vsf_add_include_directories(
    ${OPENOCD_PATH}/port
    ${OPENOCD_PATH}/port/jimtcl
    ${OPENOCD_PATH}/raw/jimtcl
    ${OPENOCD_PATH}/raw/src

    $ENV{VSF_PATH}/source/shell/sys/linux/include/libusb
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
)
vsf_add_sources(
    ${OPENOCD_SOURCES}
)
