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
    //����һ����ʽ��������ɫΪSGL_BLUE���ı���ɫΪSGL_RED��Բ�ǰ뾶Ϊ16
    sgl_style_t btn_style = {.body_color = SGL_BLUE, .text_color = SGL_RED, .radius = 16};
    //����һ����ť����
    sgl_obj_t* btn = sgl_button_create(main_page);
    //���ð�ť�ĳߴ�Ϊ��98�͸�32
    sgl_obj_set_size(btn, 98, 32);
    //���ð�ť��λ��Ϊx����100, y����50
    sgl_obj_set_pos(btn, 100, 50);
    //���ð�ť����ʽΪbtn_style
    sgl_obj_set_style(btn, &btn_style);
    //���ð�ť���¼��ص�����Ϊbtn_event��������
    sgl_obj_set_event_cb(btn, btn_event, NULL);
    //���ð�ť�İ��º����ɫΪSGL_GREEN
    sgl_button_set_toggle_color(btn, SGL_GREEN);
    //���ð�ť������Ϊsong12
    sgl_button_set_font(btn, &song12);
    //���ð�ť����ʾ�ı�ΪButton
    sgl_button_set_text(btn, "Button");
}
