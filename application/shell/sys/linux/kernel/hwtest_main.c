#define __VSF_DISP_CLASS_INHERIT__
#include <unistd.h>
#include <vsf_board.h>

#if VSF_USE_UI == ENABLED
static void __vk_disp_on_ready(vk_disp_t *disp)
{
    static uint8_t *__disp_buffer = NULL;
    static uint8_t __disp_color_idx = 0;

    static const uint16_t __disp_colors_rgb565[] = { 0x1F << 11, 0x3F << 6, 0x1F };
    static const uint32_t __disp_colors_argb8888[] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF };

    uint32_t pixels = disp->param.height * disp->param.width;

    if (NULL == __disp_buffer) {
        __disp_buffer = vsf_heap_malloc(pixels * vsf_disp_get_pixel_bytesize(disp));
        if (NULL == __disp_buffer) {
            vsf_trace_error("fail to allocate pixel buffer" VSF_TRACE_CFG_LINEEND);
            VSF_ASSERT(false);
        }
    }

    switch (disp->param.color) {
    case VSF_DISP_COLOR_RGB565:
        for (uint32_t i = 0; i < pixels; i++) {
            ((uint16_t *)__disp_buffer)[i] = __disp_colors_rgb565[__disp_color_idx];
        }
        __disp_color_idx = (__disp_color_idx + 1) % dimof(__disp_colors_rgb565);
        break;
    case VSF_DISP_COLOR_ARGB8888:
        for (uint32_t i = 0; i < pixels; i++) {
            ((uint32_t *)__disp_buffer)[i] = __disp_colors_argb8888[__disp_color_idx];
        }
        __disp_color_idx = (__disp_color_idx + 1) % dimof(__disp_colors_argb8888);
        break;
    default:
        vsf_trace_error("color %d not supported" VSF_TRACE_CFG_LINEEND, disp->param.color);
        break;
    }
    vk_disp_refresh(disp, NULL, __disp_buffer);
}
#endif

#if VSF_HAL_USE_I2C == ENABLED
static void __i2c_isrhandler(void *target_ptr, vsf_i2c_t *i2c_ptr, vsf_i2c_irq_mask_t irq_mask)
{
    static uint8_t slave_addr, data_buffer[1] = { 0 };

    if (0 == irq_mask) {
        slave_addr = 3;
    } else {
        if (irq_mask & (VSF_I2C_IRQ_MASK_MASTER_ADDRESS_NACK | VSF_I2C_IRQ_MASK_MASTER_ERROR)) {
            vsf_trace_info("__ ");
        } else if (irq_mask & VSF_I2C_IRQ_MASK_MASTER_TRANSFER_COMPLETE) {
            vsf_trace_info("%02X ", slave_addr);
        }
        if (++slave_addr > 0x77) {
            vsf_eda_post_evt((vsf_eda_t *)target_ptr, VSF_EVT_USER);
            return;
        }
        if (!(slave_addr & 0x0F)) {
            vsf_trace_info(VSF_TRACE_CFG_LINEEND "%02X: ", slave_addr & 0xF0);
        }
    }

    vsf_i2c_master_request(vsf_board.i2c, slave_addr,
        VSF_I2C_CMD_START | VSF_I2C_CMD_WRITE | VSF_I2C_CMD_STOP | VSF_I2C_CMD_7_BITS,
        sizeof(data_buffer), data_buffer);
}
#endif

int hwtest_main(int argc, char **argv)
{
#if VSF_USE_UI == ENABLED
    vsf_board.display_dev->ui_on_ready = __vk_disp_on_ready;
    vk_disp_init(vsf_board.display_dev);
#endif

#if VSF_HAL_USE_I2C == ENABLED
    vsf_i2c_init(vsf_board.i2c, &(vsf_i2c_cfg_t){
        .mode           = VSF_I2C_MODE_MASTER | VSF_I2C_SPEED_FAST_MODE | VSF_I2C_ADDR_7_BITS,
        .clock_hz       = 400 * 1000,
        .isr            = {
            .handler_fn = __i2c_isrhandler,
            .target_ptr = vsf_eda_get_cur(),
            .prio       = vsf_arch_prio_0,
        },
    });
    vsf_i2c_enable(vsf_board.i2c);
    vsf_i2c_irq_enable(vsf_board.i2c,
            VSF_I2C_IRQ_MASK_MASTER_ADDRESS_NACK | VSF_I2C_IRQ_MASK_MASTER_ERROR
        |   VSF_I2C_IRQ_MASK_MASTER_NACK_DETECT | VSF_I2C_IRQ_MASK_MASTER_TRANSFER_COMPLETE);

    vsf_trace_info("i2cdetect:" VSF_TRACE_CFG_LINEEND);
    vsf_trace_info("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f" VSF_TRACE_CFG_LINEEND);
    vsf_trace_info("00:          ");
    __i2c_isrhandler(NULL, vsf_board.i2c, 0);
    vsf_thread_wfe(VSF_EVT_USER);
#endif

#if VSF_USE_USB_DEVICE == ENABLED
#endif

#if VSF_USE_USB_HOST == ENABLED
#endif

#if VSF_HAL_USE_MMC == ENABLED
#endif

    return 0;
}
