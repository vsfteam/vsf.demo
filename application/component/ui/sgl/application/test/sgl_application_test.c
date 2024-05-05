#include <sgl.h>

static void btn_event(sgl_obj_t *obj, void *data)
{
    if(sgl_ev_stat(obj) == SGL_EVENT_PRESSED) {
        SGL_LOG_INFO("buttton pressed\n");
    }
    else if(sgl_ev_stat(obj) == SGL_EVENT_RELEASED) {
        SGL_LOG_INFO("buttton released\n");
    };
}

void demo_setup(void *data)
{
    sgl_obj_t *main_page = (sgl_obj_t*)data;
    SGL_LOG_INFO("button demo");
    //定义一个样式，主体颜色为SGL_BLUE，文本颜色为SGL_RED，圆角半径为16
    sgl_style_t btn_style = {.body_color = SGL_BLUE, .text_color = SGL_RED, .radius = 16};
    //创建一个按钮对象
    sgl_obj_t* btn = sgl_button_create(main_page);
    //设置按钮的尺寸为宽98和高32
    sgl_obj_set_size(btn, 98, 32);
    //设置按钮的位置为x坐标100, y坐标50
    sgl_obj_set_pos(btn, 100, 50);
    //设置按钮的样式为btn_style
    sgl_obj_set_style(btn, &btn_style);
    //设置按钮的事件回调函数为btn_event，参数无
    sgl_obj_set_event_cb(btn, btn_event, NULL);
    //设置按钮的按下后的颜色为SGL_GREEN
    sgl_button_set_toggle_color(btn, SGL_GREEN);
    //设置按钮的字体为song12
    sgl_button_set_font(btn, &song12);
    //设置按钮的显示文本为Button
    sgl_button_set_text(btn, "Button");
}
