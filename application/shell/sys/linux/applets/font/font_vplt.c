#include <unistd.h>
#include <linux/font.h>

extern const struct font_desc font_vga_8x8, font_vga_8x16;

static const vsf_linux_font_vplt_t __vsf_linux_font_vplt = {
    VSF_APPLET_VPLT_INFO(vsf_linux_font_vplt_t, 0, 0, true),
    VSF_APPLET_VPLT_ENTRY_MOD(linux_font, __vsf_linux_font_vplt),

    VSF_APPLET_VPLT_ENTRY_VAR(font_vga_8x8),
    VSF_APPLET_VPLT_ENTRY_VAR(font_vga_8x16),
};

int main(int argc, char **argv)
{
    vsf_vplt_load_dyn((vsf_vplt_info_t *)&__vsf_linux_font_vplt.info);
    vsf_linux_loader_keep_ram();
    return 0;
}
