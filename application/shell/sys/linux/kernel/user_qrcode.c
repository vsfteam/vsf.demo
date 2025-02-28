// for disp->ui_on_ready
#define __VSF_DISP_CLASS_INHERIT__

#include <unistd.h>
#include <vsf_board.h>

#include "component/3rd-party/qrcode/raw/src/qrcode.h"

#if VSF_USE_QRCODE == ENABLED
#   include <linux/font.h>

#   define TITLE_PIXEL_HEIGHT               64
#   define TITLE_QRCODE_MARGIN              16

#   define DISP_COLOR_BLACK                 0x00000000
#   define DISP_COLOR_WHITE                 0xFFFFFFFF

vsf_linux_font_vplt_t * vsf_app_load_linux_font(void)
{
    vsf_linux_font_vplt_t *font_vplt = vsf_vplt_link(NULL, "linux_font");
    if (NULL == font_vplt) {
        system("load_font");
        font_vplt = vsf_vplt_link(NULL, "linux_font");
    }
    return font_vplt;
}

void vsf_app_disp_render_char(struct font_desc *font, uint8_t *disp_buff, uint8_t pixel_bytesize, uint16_t pitch, char c)
{
    uint8_t font_byte_width = (font->width + 7) >> 3;
    uint8_t *data = (uint8_t *)font->data + font->height * font_byte_width * c;
    uint32_t line_data, line_msb = 1 << (font->width - 1), pixel;
    uint8_t *disp_buff_tmp;

    for (uint8_t i = 0; i < font->height; i++) {
        switch (font_byte_width) {
        case 1: line_data = *(uint8_t *)data;           break;
        case 2: line_data = get_unaligned_be16(data);   break;
        case 4: line_data = get_unaligned_be32(data);   break;
        default:
            VSF_ASSERT(false);
        }
        data += font_byte_width;

        disp_buff_tmp = disp_buff;
        for (uint8_t j = 0; j < font->width; j++, line_data <<= 1) {
            pixel = (line_data & line_msb) ? DISP_COLOR_BLACK : DISP_COLOR_WHITE;
            switch (pixel_bytesize) {
            case 1: *(uint8_t *) disp_buff_tmp = pixel; break;
            case 2: *(uint16_t *)disp_buff_tmp = pixel; break;
            case 4: *(uint32_t *)disp_buff_tmp = pixel; break;
            default:
                VSF_ASSERT(false);
            }
            disp_buff_tmp += pixel_bytesize;
        }
        disp_buff += pitch;
    }
}

void vsf_app_disp_render_string(struct font_desc *font, uint8_t *disp_buff, uint8_t pixel_bytesize, uint16_t pitch, char *str)
{
    ssize_t len = strlen(str);
    while (len-- > 0) {
        vsf_app_disp_render_char(font, disp_buff, pixel_bytesize, pitch, *str++);
        disp_buff += font->width * pixel_bytesize;
    }
}
#endif

#if VSF_USE_UI == ENABLED
static void __vsf_app_qrcode_disp_on_ready(vk_disp_t *disp)
{
    vsf_eda_post_evt((vsf_eda_t *)disp->ui_data, VSF_EVT_USER);
}
#endif

int display_qrcode_main(int argc, char **argv)
{
    if ((argc < 2) || (argc > 3)) {
        printf("format: %s TITLE STRING\n", argv[0]);
        return -1;
    }

#if VSF_USE_UI == ENABLED
    VSF_ASSERT(vsf_board.display_dev != NULL);
    uint16_t disp_width = vsf_disp_get_width(vsf_board.display_dev);
    // reserve 64 lines for title
    uint16_t disp_height = vsf_disp_get_height(vsf_board.display_dev) - TITLE_PIXEL_HEIGHT;
    uint8_t disp_pixel_size = vsf_disp_get_pixel_bytesize(vsf_board.display_dev);
    system("fill_screen 0xFFFFFFFF");
#endif

    if (argc > 2) {
#if VSF_USE_UI == ENABLED
        char *title = argv[1];
        argv++;
        do {
            vsf_linux_font_vplt_t *font_vplt = vsf_app_load_linux_font();
            if (NULL == font_vplt) {
                printf("fail to load linux font\n");
                break;
            }

            struct font_desc *font = (struct font_desc *)VSF_APPLET_VPLT_ENTRY_VAR_ENTRY(font_vplt, font_vga_8x16);
            if (NULL == font) {
                printf("fail to load vga_8x16 font\n");
                break;
            }

            uint16_t title_width = strlen(title) * font->width;
            uint16_t title_height = font->height;
            uint8_t *disp_buff = malloc(title_width * title_height * disp_pixel_size);
            if (NULL == disp_buff) {
                printf("fail to allocate display buffer for title\n");
                break;
            }

            vsf_app_disp_render_string(font, disp_buff, disp_pixel_size, title_width * disp_pixel_size, title);

            vsf_board.display_dev->ui_data = vsf_eda_get_cur();
            vsf_board.display_dev->ui_on_ready = __vsf_app_qrcode_disp_on_ready;
            if (title_width > disp_width) {
                title_width = disp_width;
            }
            vk_disp_refresh(vsf_board.display_dev, &(vk_disp_area_t){
                .size.x     = title_width,
                .size.y     = title_height,
                .pos.x      = (disp_width - title_width) >> 1,
                .pos.y      = (TITLE_PIXEL_HEIGHT - title_height) >> 1,
            }, disp_buff);
            vsf_thread_wfe(VSF_EVT_USER);

            free(disp_buff);
        } while (false);
#else
        printf("QRCode for %s\n", argv[1]);
        argv++;
#endif
    }

    QRCode qrcode;
    uint8_t qrcode_data[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcode_data, 3, ECC_LOW, argv[1]);

#if VSF_USE_UI == ENABLED
    uint16_t qr_size = vsf_min(disp_width, disp_height) - TITLE_QRCODE_MARGIN;
    uint16_t qr_pos_x = (disp_width - qr_size) >> 1;
    uint16_t qr_pos_y = TITLE_PIXEL_HEIGHT + ((disp_height - qr_size) >> 1);

    uint16_t block_size = qr_size / qrcode.size;
    uint16_t block_size_remain = qr_size % qrcode.size;
    qr_pos_x += block_size_remain >> 1;
    qr_pos_y += block_size_remain >> 1;

    uint16_t block_pixel_size = disp_pixel_size * block_size * block_size;
    uint8_t *disp_block_buff = malloc(block_pixel_size);
    if (NULL == disp_block_buff) {
        printf("fail to allocate display buffer for qrcode block\n");
        return -1;
    }

    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                memset(disp_block_buff, DISP_COLOR_BLACK, block_pixel_size);
            } else {
                memset(disp_block_buff, DISP_COLOR_WHITE, block_pixel_size);
            }

            vsf_board.display_dev->ui_data = vsf_eda_get_cur();
            vsf_board.display_dev->ui_on_ready = __vsf_app_qrcode_disp_on_ready;
            vk_disp_refresh(vsf_board.display_dev, &(vk_disp_area_t){
                .size.x     = block_size,
                .size.y     = block_size,
                .pos.x      = qr_pos_x + x * block_size,
                .pos.y      = qr_pos_y + y * block_size,
            }, disp_block_buff);
            vsf_thread_wfe(VSF_EVT_USER);
        }
    }
    free(disp_block_buff);
#else
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                printf("\033[47m");
            } else {
                printf("\033[40m");
            }
            printf("  ");
        }
        printf("\033[40m\r\n");
    }
    printf("\r\n");
#endif

    return 0;
}
