# CMakeLists head

cmake_policy(SET CMP0079 NEW)

file(GLOB OPENOCD_SOURCES
    port/jimtcl/_glob.c
    port/jimtcl/_initjimsh.c
    port/jimtcl/_jsonencode.c
    port/jimtcl/_load-static-exts.c
    port/jimtcl/_nshelper.c
    port/jimtcl/_oo.c
    port/jimtcl/_stdlib.c
    port/jimtcl/_tclcompat.c
    port/jimtcl/_tree.c

    raw/jimtcl/jsmn/jsmn.c
    raw/jimtcl/jim.c
    raw/jimtcl/jim-aio.c
    raw/jimtcl/jim-array.c
    raw/jimtcl/jim-clock.c
    raw/jimtcl/jim-eventloop.c
    raw/jimtcl/jim-exec.c
    raw/jimtcl/jim-file.c
    raw/jimtcl/jim-format.c
    raw/jimtcl/jim-history.c
    raw/jimtcl/jim-interactive.c
    raw/jimtcl/jim-interp.c
    raw/jimtcl/jimiocompat.c
    raw/jimtcl/jim-json.c
    raw/jimtcl/jim-load.c
    raw/jimtcl/jim-namespace.c
    raw/jimtcl/jim-pack.c
    raw/jimtcl/jim-package.c
    raw/jimtcl/jim-posix.c
    raw/jimtcl/jim-readdir.c
    raw/jimtcl/jimregexp.c
    raw/jimtcl/jim-regexp.c
    raw/jimtcl/jim-signal.c
    raw/jimtcl/jim-subcmd.c
    raw/jimtcl/jim-tty.c
    raw/jimtcl/linenoise.c
    raw/jimtcl/utf8.c

#    raw/src/flash/nand/arm_io.c
#    raw/src/flash/nand/at91sam9.c
    raw/src/flash/nand/core.c
#    raw/src/flash/nand/davinci.c
    raw/src/flash/nand/driver.c
    raw/src/flash/nand/ecc.c
    raw/src/flash/nand/ecc_kw.c
    raw/src/flash/nand/fileio.c
#    raw/src/flash/nand/lpc32xx.c
#    raw/src/flash/nand/lpc3180.c
#    raw/src/flash/nand/mx3.c
#    raw/src/flash/nand/msc.c
#    raw/src/flash/nand/nonce.c
#    raw/src/flash/nand/nuc910.c
#    raw/src/flash/nand/orion.c
#    raw/src/flash/nand/s3c24xx.c
#    raw/src/flash/nand/s3c2410.c
#    raw/src/flash/nand/s3c2412.c
#    raw/src/flash/nand/s3c2440.c
#    raw/src/flash/nand/s3c2443.c
#    raw/src/flash/nand/s3c6400.c
    raw/src/flash/nand/tcl.c

#    raw/src/flash/nor/aduc702x.c
#    raw/src/flash/nor/aducm360.c
#    raw/src/flash/nor/ambiqmicro.c
#    raw/src/flash/nor/at91sam3.c
#    raw/src/flash/nor/at91sam4.c
#    raw/src/flash/nor/at91sam4l.c
#    raw/src/flash/nor/at91sam7.c
#    raw/src/flash/nor/at91samd.c
#    raw/src/flash/nor/ath79.c
#    raw/src/flash/nor/atsame5.c
#    raw/src/flash/nor/atsamv.c
#    raw/src/flash/nor/avrf.c
#    raw/src/flash/nor/bluenrg-x.c
#    raw/src/flash/nor/cc26xx.c
#    raw/src/flash/nor/cc3220sf.c
    raw/src/flash/nor/cfi.c
    raw/src/flash/nor/core.c
    raw/src/flash/nor/drivers.c
#    raw/src/flash/nor/dsp5680xx_flash.c
#    raw/src/flash/nor/efm32.c
#    raw/src/flash/nor/em357.c
#    raw/src/flash/nor/esirisc_flash.c
#    raw/src/flash/nor/faux.c
#    raw/src/flash/nor/fespi.c
#    raw/src/flash/nor/fm3.c
#    raw/src/flash/nor/fm4.c
#    raw/src/flash/nor/jtagspi.c
#    raw/src/flash/nor/kinetis.c
#    raw/src/flash/nor/lpc288x.c
#    raw/src/flash/nor/lpc2000.c
#    raw/src/flash/nor/lpc2900.c
#    raw/src/flash/nor/lpcspifi.c
#    raw/src/flash/nor/mdr.c
#    raw/src/flash/nor/mrvlqspi.c
#    raw/src/flash/nor/msp432.c
#    raw/src/flash/nor/niietcm4.c
    raw/src/flash/nor/non_cfi.c
#    raw/src/flash/nor/npcx.c
#    raw/src/flash/nor/nrf5.c
#    raw/src/flash/nor/numicro.c
#    raw/src/flash/nor/ocl.c
#    raw/src/flash/nor/pic32mx.c
#    raw/src/flash/nor/psoc4.c
#    raw/src/flash/nor/psoc5lp.c
#    raw/src/flash/nor/psoc6.c
#    raw/src/flash/nor/renesas_rpchf.c
#    raw/src/flash/nor/rp2040.c
#    raw/src/flash/nor/sfdp.c
#    raw/src/flash/nor/sh_qspi.c
#    raw/src/flash/nor/sim3x.c
#    raw/src/flash/nor/spi.c
#    raw/src/flash/nor/stellaris.c
#    raw/src/flash/nor/stm32f1x.c
#    raw/src/flash/nor/stm32f2x.c
#    raw/src/flash/nor/stm32h7x.c
#    raw/src/flash/nor/stm32l4x.c
#    raw/src/flash/nor/stm32lx.c
#    raw/src/flash/nor/stmqspi.c
#    raw/src/flash/nor/stmsmi.c
#    raw/src/flash/nor/str7x.c
#    raw/src/flash/nor/str9x.c
#    raw/src/flash/nor/str9xpec.c
#    raw/src/flash/nor/swm050.c
    raw/src/flash/nor/tcl.c
#    raw/src/flash/nor/tms470.c
#    raw/src/flash/nor/virtual.c
#    raw/src/flash/nor/w600.c
#    raw/src/flash/nor/xcf.c
#    raw/src/flash/nor/xmc1xxx.c
#    raw/src/flash/nor/xmc4xxx.c

    raw/src/flash/common.c

    raw/src/helper/binarybuffer.c
    raw/src/helper/command.c
    raw/src/helper/configuration.c
    raw/src/helper/fileio.c
    raw/src/helper/jep106.c
    raw/src/helper/jim-nvp.c
    raw/src/helper/log.c
    raw/src/helper/options.c
    raw/src/helper/replacements.c
    raw/src/helper/time_support.c
    raw/src/helper/time_support_common.c
    raw/src/helper/util.c

    raw/src/jtag/drivers/cmsis_dap.c
    raw/src/jtag/drivers/cmsis_dap_usb_bulk.c
    raw/src/jtag/drivers/driver.c

    raw/src/jtag/adapter.c
    raw/src/jtag/commands.c
    raw/src/jtag/core.c
    raw/src/jtag/interface.c
    raw/src/jtag/interfaces.c
    raw/src/jtag/swim.c
    raw/src/jtag/tcl.c

    raw/src/pld/pld.c
    raw/src/pld/virtex2.c
    raw/src/pld/xilinx_bit.c

    raw/src/rtos/chibios.c
    raw/src/rtos/chromium-ec.c
    raw/src/rtos/eCos.c
    raw/src/rtos/embKernel.c
    raw/src/rtos/FreeRTOS.c
    raw/src/rtos/hwthread.c
    raw/src/rtos/linux.c
    raw/src/rtos/mqx.c
    raw/src/rtos/nuttx.c
    raw/src/rtos/riot.c
    raw/src/rtos/rtos.c
    raw/src/rtos/rtos_chibios_stackings.c
    raw/src/rtos/rtos_ecos_stackings.c
    raw/src/rtos/rtos_embkernel_stackings.c
    raw/src/rtos/rtos_mqx_stackings.c
    raw/src/rtos/rtos_riot_stackings.c
    raw/src/rtos/rtos_standard_stackings.c
    raw/src/rtos/rtos_ucos_iii_stackings.c
    raw/src/rtos/ThreadX.c
    raw/src/rtos/uCOS-III.c
    raw/src/rtos/zephyr.c

    raw/src/rtt/rtt.c
    raw/src/rtt/tcl.c

    raw/src/server/gdb_server.c
    raw/src/server/ipdbg.c
    raw/src/server/rtt_server.c
    raw/src/server/server.c
    raw/src/server/tcl_server.c
    raw/src/server/telnet_server.c

    raw/src/svf/svf.c

    raw/src/target/openrisc/jsp_server.c
    raw/src/target/openrisc/or1k.c
    raw/src/target/openrisc/or1k_du_adv.c
    raw/src/target/openrisc/or1k_tap_mohor.c
    raw/src/target/openrisc/or1k_tap_vjtag.c
    raw/src/target/openrisc/or1k_tap_xilinx_bscan.c
    raw/src/target/riscv/batch.c
    raw/src/target/riscv/program.c
    raw/src/target/riscv/riscv.c
    raw/src/target/riscv/riscv_semihosting.c
    raw/src/target/riscv/riscv-011.c
    raw/src/target/riscv/riscv-013.c
#    raw/src/target/a64_disassembler.c
#    raw/src/target/aarch64.c
    raw/src/target/adi_v5_dapdirect.c
    raw/src/target/adi_v5_jtag.c
    raw/src/target/adi_v5_swd.c
    raw/src/target/algorithm.c
    raw/src/target/arc.c
    raw/src/target/arc_cmd.c
    raw/src/target/arc_jtag.c
    raw/src/target/arc_mem.c
    raw/src/target/arm_adi_v5.c
    raw/src/target/arm_cti.c
    raw/src/target/arm_dap.c
    raw/src/target/arm_disassembler.c
    raw/src/target/arm_dpm.c
    raw/src/target/arm_jtag.c
    raw/src/target/arm_semihosting.c
    raw/src/target/arm_simulator.c
    raw/src/target/arm_tpiu_swo.c
    raw/src/target/arm7_9_common.c
    raw/src/target/arm7tdmi.c
    raw/src/target/arm9tdmi.c
    raw/src/target/arm11.c
    raw/src/target/arm11_dbgtap.c
    raw/src/target/arm720t.c
    raw/src/target/arm920t.c
    raw/src/target/arm926ejs.c
    raw/src/target/arm946e.c
    raw/src/target/arm966e.c
    raw/src/target/armv4_5.c
    raw/src/target/armv4_5_cache.c
    raw/src/target/armv4_5_mmu.c
    raw/src/target/armv7a.c
    raw/src/target/armv7a_cache.c
    raw/src/target/armv7a_cache_l2x.c
    raw/src/target/armv7a_mmu.c
    raw/src/target/armv7m.c
    raw/src/target/armv7m_trace.c
    raw/src/target/armv8.c
    raw/src/target/armv8_cache.c
    raw/src/target/armv8_dpm.c
    raw/src/target/armv8_opcodes.c
#    raw/src/target/avr32_ap7k.c
#    raw/src/target/avr32_jtag.c
#    raw/src/target/avr32_mem.c
#    raw/src/target/avr32_regs.c
#    raw/src/target/avrt.c
    raw/src/target/breakpoints.c
    raw/src/target/cortex_a.c
    raw/src/target/cortex_m.c
#    raw/src/target/dsp563xx.c
#    raw/src/target/dsp563xx_once.c
#    raw/src/target/dsp5680xx.c
    raw/src/target/embeddedice.c
#    raw/src/target/esirisc.c
#    raw/src/target/esirisc_jtag.c
#    raw/src/target/esirisc_trace.c
    raw/src/target/etb.c
    raw/src/target/etm.c
    raw/src/target/etm_dummy.c
#    raw/src/target/fa526.c
#    raw/src/target/feroceon.c
#    raw/src/target/hla_target.c
    raw/src/target/image.c
#    raw/src/target/lakemont.c
#    raw/src/target/ls1_sap.c
    raw/src/target/mem_ap.c
#    raw/src/target/mips_ejtag.c
#    raw/src/target/mips_m4k.c
#    raw/src/target/mips_mips64.c
#    raw/src/target/mips32.c
#    raw/src/target/mips32_dmaacc.c
#    raw/src/target/mips32_pracc.c
#    raw/src/target/mips64.c
#    raw/src/target/mips64_pracc.c
#    raw/src/target/nds32.c
#    raw/src/target/nds32_aice.c
#    raw/src/target/nds32_cmd.c
#    raw/src/target/nds32_disassembler.c
#    raw/src/target/nds32_reg.c
#    raw/src/target/nds32_tlb.c
#    raw/src/target/nds32_v2.c
#    raw/src/target/nds32_v3.c
#    raw/src/target/nds32_v3_common.c
#    raw/src/target/nds32_v3m.c
#    raw/src/target/quark_d20xx.c
#    raw/src/target/quark_x10xx.c
    raw/src/target/register.c
    raw/src/target/rtt.c
    raw/src/target/semihosting_common.c
    raw/src/target/smp.c
#    raw/src/target/stm8.c
    raw/src/target/target.c
    raw/src/target/target_request.c
#    raw/src/target/testee.c
    raw/src/target/trace.c
#    raw/src/target/x86_32_common.c
#    raw/src/target/xscale.c

    raw/src/transport/transport.c

    raw/src/xsvf/xsvf.c

    raw/src/hello.c
    raw/src/main.c
    raw/src/openocd.c
)
vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    __VSF__
    VSF_COMPATIBILITY=DISABLED

    HAVE_CONFIG_H
    SIGWINCH=28
)
vsf_add_include_directories(
    port
    port/jimtcl
    raw/jimtcl
    raw/src

    ${VSF_COMMON_INCLUDE_DIRECTORIES}
)
vsf_add_sources(
    ${OPENOCD_SOURCES}
)
