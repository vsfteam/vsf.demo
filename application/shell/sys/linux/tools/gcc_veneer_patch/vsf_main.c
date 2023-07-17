/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*
 * gcc-veneer-patch tool
 * Based on linux sub-system
 * 
 * Patch veneer functions of GCC elf targets, avoid using PC-relative form while 
 *  accessing entries in got.plt, use static-base register(r9) instead
 *
 * Dependency:
 *
 * Board:
 *
 * Include Directories necessary for linux:
 *   vsf/source/shell/sys/linux/include
 *   vsf/source/shell/sys/linux/include/simple_libc if VSF_LINUX_USE_SIMPLE_LIBC is enabled
 *
 * Sources necessary for linux:
 *   vsf/source/shell/sys/linux/lib/3rd-party/fnmatch
 *   vsf/source/shell/sys/linux/lib/3rd-party/glob
 *   vsf/example/template/demo/linux/mount_demo.c
 *
 * Sources to modify:
 *   vsf/source/service/loader/elf/elf.h
 *      add #undef __VSF64__ to make elfloader suitable for CortexM
 *   vsf/source/service/loader/elf/vsf_elfloader.c
 *      skip relocating in vsf_elfloader_load
 *      // relocating
 *      if (linfo.has_dynamic && 0) {
 */

/*============================ INCLUDES ======================================*/

#define __VSF_ELFLOADER_CLASS_INHERIT__
#include <unistd.h>
#include <dlfcn.h>
#include <vsf_board.h>

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static int __elfpatch_main(int argc, char *argv[])
{
    argc = vsf_arch_argu(&argv);

    if (argc != 2) {
        printf("format: %s ELF_TARGET\n", argv[1]);
        return -1;
    }

    vsf_linux_dynloader_t *loader = (vsf_linux_dynloader_t *)dlopen(argv[1], 0);
    if (NULL == loader) {
        printf("fail to load %s\n", argv[1]);
        return -1;
    }
    if (loader->loader.generic.op != &vsf_elfloader_op) {
        printf("%s is not an elf\n", argv[1]);
        goto dlclose_and_fail;
    }

    vsf_elfloader_t *elfloader = &loader->loader.elfloader;
    vsf_loader_target_t *elftarget = &loader->target;

    Elf_Shdr plt_section;
    Elf_Shdr got_section;
    Elf_Shdr gotplt_section;
    if (    !vsf_elfloader_get_section(elfloader, elftarget, ".got", &got_section)
        ||  !vsf_elfloader_get_section(elfloader, elftarget, ".got.plt", &gotplt_section)
        ||  !vsf_elfloader_get_section(elfloader, elftarget, ".plt", &plt_section)) {
        printf("no got/gotplt/plt found in %s\n", argv[1]);
        goto dlclose_and_fail;
    }
    dlclose((void *)loader);

    FILE *f = fopen(argv[1], "r+");
    long fsize;
    uint8_t *fbuff;
    if (NULL == f) {
        printf("fail to open %s\n", argv[1]);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    fbuff = (uint8_t *)malloc(fsize);
    if (NULL == fbuff) {
        printf("fail to allocate file buffer\n");
        goto fclose_and_fail;
    }
    if(fread(fbuff, 1, fsize, f) != fsize) {
        printf("fail to read %s\n", argv[1]);
        goto fclose_and_fail;
    }

    const uint8_t eigenvaluen[] = {
        0xFC, 0x44,                     // ADD      R12, PC
        0xDC, 0xF8, 0x00, 0xF0,         // 1: LDR.W PC, [R12]
        0xFC, 0xE7,                     // B 1
    };
    uint8_t *plt = &fbuff[plt_section.sh_offset], *entry;
    uint32_t pltsize = plt_section.sh_size, offset;
    for (uint32_t i = 0; i <= pltsize - 16; i += 16) {
        entry = &plt[i];
        if (!memcmp(&entry[8], eigenvaluen, 8)) {
            // get offset to pc
            offset = 0;
            //  parse MOVW      R12, LOW16
            //  offset |= (entry[0] & (BITMASK << OFFSET)) << (16 - PREVBITS - BITLEN - OFFSET);
            offset |= (entry[1] & (0x01 << 2)) << (16 - 0 - 1 - 2);
            offset |= (entry[0] & (0x0F << 0)) << (16 - 1 - 4 - 0);
            offset |= (entry[3] & (0x07 << 4)) << (16 - 5 - 3 - 4);
            offset |= (entry[2] & (0xFF << 0)) << (16 - 8 - 8 - 0);
            //  parse MOVT      R12, HIGH16
            //  offset |= (entry[4 + n] & (BITMASK << OFFSET)) << (32 - PREVBITS - BITLEN - OFFSET);
            offset |= (entry[5] & (0x01 << 2)) << (32 - 0 - 1 - 2);
            offset |= (entry[4] & (0x0F << 0)) << (32 - 1 - 4 - 0);
            offset |= (entry[7] & (0x07 << 4)) << (32 - 5 - 3 - 4);
            offset |= (entry[6] & (0xFF << 0)) << (32 - 8 - 8 - 0);
            offset += plt_section.sh_addr + entry - plt + 12;

            // calculate offset to .got
            offset -= got_section.sh_addr;

            // patch entry
            //  set MOVW      R12, LOW16
            //  entry[n] = (entry[n] & ~(BITMASK << OFFSET)) | ((offset & (BITMASK << (16 - PREVBITS - BITLEN))) >> (16 - PREVBITS - BITLEN - OFFSET));
            entry[1] = (entry[1] & ~(0x01 << 2)) | ((offset & (0x01 << (16 - 0 - 1))) >> (16 - 0 - 1 - 2));
            entry[0] = (entry[0] & ~(0x0F << 0)) | ((offset & (0x0F << (16 - 1 - 4))) >> (16 - 1 - 4 - 0));
            entry[3] = (entry[3] & ~(0x07 << 4)) | ((offset & (0x07 << (16 - 5 - 3))) >> (16 - 5 - 3 - 4));
            entry[2] = (entry[2] & ~(0xFF << 0)) | ((offset & (0xFF << (16 - 8 - 8))) >> (16 - 8 - 8 - 0));
            //  set MOVT      R12, HIGH16
            //  entry[4 + n] = (entry[4 + n] & ~(BITMASK << OFFSET)) | ((offset & (BITMASK << (32 - PREVBITS - BITLEN))) >> (32 - PREVBITS - BITLEN - OFFSET));
            entry[5] = (entry[5] & ~(0x01 << 2)) | ((offset & (0x01 << (32 - 0 - 1))) >> (32 - 0 - 1 - 2));
            entry[4] = (entry[4] & ~(0x0F << 0)) | ((offset & (0x0F << (32 - 1 - 4))) >> (32 - 1 - 4 - 0));
            entry[7] = (entry[7] & ~(0x07 << 4)) | ((offset & (0x07 << (32 - 5 - 3))) >> (32 - 5 - 3 - 4));
            entry[6] = (entry[6] & ~(0xFF << 0)) | ((offset & (0xFF << (32 - 8 - 8))) >> (32 - 8 - 8 - 0));
            //  ADD      R12, PC  ==>>  ADD      R12, R9
            entry[8] = 0xCC;
        }
    }

    fseek(f, 0, SEEK_SET);
    if(fwrite(fbuff, 1, fsize, f) != fsize) {
        printf("fail to write %s\n", argv[1]);
        goto fclose_and_fail;
    }

    fclose(f);
    vsf_arch_shutdown();
    return 0;

fclose_and_fail:
    if (f != NULL) {
        fclose(f);
    }
    return -1;

dlclose_and_fail:
    dlclose((void *)loader);
    return -1;
}

int vsf_linux_create_fhs(void)
{
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();

    // 1. hardware driver

    // 2. fs
    busybox_install();

    // 3. install executables
    extern int mount_main(int argc, char *argv[]);
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/mount", mount_main);
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/elfpatch", __elfpatch_main);

    return 0;
}

// TODO: SDL require that main need argc and argv
int VSF_USER_ENTRY(int argc, char *argv[])
{
    vsf_board_init();
    vsf_start_trace();

    vsf_trace_info("start linux..." VSF_TRACE_CFG_LINEEND);
    vsf_linux_stdio_stream_t stream = {
        .in     = (vsf_stream_t *)&VSF_DEBUG_STREAM_RX,
        .out    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
        .err    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
    };
    vsf_linux_init(&stream);
    return 0;
}

/* EOF */