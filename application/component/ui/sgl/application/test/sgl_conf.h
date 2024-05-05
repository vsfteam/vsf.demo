//debug mode
#define SGL_CONFIG_DEBUG                1
#define SGL_CONFIG_LOG_LEVEL            1

//poweron animation
#define SGL_CONFIG_POWERON_ANIM         0

// enable Anti-Aliasing 
#define SGL_CONFIG_AA_ENABLE            1

//配置字体抗锯齿深度，默认采用4bit抗锯齿
#define SGL_CONFIG_FONT_PIXEL_BIT       4

//显示屏面板的像素深度，默认为32bit
#define SGL_CONFIG_PANEL_PIXEL_DEPTH    16
#define SGL_CONFIG_FRAME_BUFFER_XRES    480

//RGB颜色交换
#define SGL_CONFIG_COLOR16_SWAP         0

//1: standard
//2: fast
#define SGL_CONFIG_MIXER_TYPE           2

//1: newton
//2: 
#define SGL_CONFIG_SQRT_ALGO            1

#define SGL_CONFIG_FRAMEBUFFER_MMAP     0

//定义事件的队列深度
#define SGL_CONFIG_EVQUEUE_DEPTH        16

//定义任务的队列深度
#define SGL_CONFIG_TASKQUEUE_DEPTH      8

//0: heap_0
//1: heap_1
//2: heap_2
//3: heap_3
//4: heap_4
//5: heap_tlsf
//6: libc
#define SGL_CONFIG_HEAP_POLICY          0
//定义内存池的大小，单位KByte
#define SGL_CONFIG_HEAP_POOL_SIZE       64


#define SGL_CONFIG_TEXT_UTF8            0

#define SGL_CONFIG_PAGE_COLOR           SGL_WHITE

#define SGL_CONFIG_FONT_CONSOLAS12      1
#define SGL_CONFIG_FONT_CONSOLAS15      1
#define SGL_CONFIG_FONT_SONG10          1
#define SGL_CONFIG_FONT_SONG12          1
#define SGL_CONFIG_FONT_SONG23          1
#define SGL_CONFIG_FONT_CASCADIA_MONO17 1
