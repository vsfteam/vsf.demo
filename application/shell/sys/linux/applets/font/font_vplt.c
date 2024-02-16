#include <unistd.h>
#include <linux/font.h>

extern struct font_desc font_vga_8x8, font_vga_8x16;

__VSF_VPLT_DECORATOR__ vsf_linux_font_vplt_t vsf_linux_font_vplt = {
    VSF_APPLET_VPLT_INFO(vsf_linux_font_vplt_t, 0, 0, true),
    VSF_APPLET_VPLT_ENTRY_MOD(linux_font, vsf_linux_font_vplt),

    VSF_APPLET_VPLT_ENTRY_VAR(font_vga_8x8),
    VSF_APPLET_VPLT_ENTRY_VAR(font_vga_8x16),
};

int main(int argc, char **argv)
{
    vsf_vplt_load_dyn((vsf_vplt_info_t *)&vsf_linux_font_vplt.info);
    return 0;
}
