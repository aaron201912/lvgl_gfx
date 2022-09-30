/*
 * sstar_port.c
 */

#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>

#include "lvgl.h"

#include "evdev.h"
#include "lv_conf.h"
#include "lv_draw_sw.h"

#include "sstar_sys.h"
#include "sstar_memory.h"
#include "sstar_disp.h"
#include "sstar_fbdev.h"
#include "sstar_gfx.h"
#include "sstar_port.h"

void sstar_draw_ctx_init_cb(struct _lv_disp_drv_t * disp_drv, lv_draw_ctx_t * draw_ctx)
{
    lv_draw_sw_ctx_t * draw_sw_ctx = (lv_draw_sw_ctx_t *) draw_ctx;
    lv_draw_sw_init_ctx(disp_drv, draw_ctx);
    draw_sw_ctx->base_draw.draw_img = NULL;//sstar_gfx_draw_img_cb;
    draw_sw_ctx->blend = sstar_gfx_blend_cb;
}
void sstar_draw_ctx_deinit_cb(struct _lv_disp_drv_t * disp_drv, lv_draw_ctx_t * draw_ctx)
{
    lv_draw_sw_deinit_ctx(disp_drv, draw_ctx);
}

void sstar_flush_cb(struct _lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    LV_LOG_TRACE("%s-> Calling flush_cb on (%d;%d)(%d;%d) area with %p image pointer\033[0m\n",
            color_p == disp_drv->draw_buf->buf1 ? "\033[0;35m" : "\033[0;36m",
            area->x1, area->y1, area->x2, area->y2, (void *)color_p);
    //sstar_gfx_wait();
    if (disp_drv->full_refresh) {
        sstar_fbdev_flush(color_p);
        lv_disp_flush_ready(disp_drv);
        return;
    }
    if (disp_drv->draw_buf->flushing_last) {
        sstar_fbdev_flush(color_p);
    }
    lv_disp_t *disp = _lv_refr_get_disp_refreshing();
    lv_area_t dirty_area = {0};
    int i = 0;
    for (i = 0; i < disp->inv_p; ++i) {
        if (!disp->inv_area_joined[i]) {
            dirty_area = disp->inv_areas[i];
            break;
        }
    }
    lv_color_t *dst_buf, *src_buf;
    if (color_p == disp_drv->draw_buf->buf1) {
        dst_buf = disp_drv->draw_buf->buf2;
        src_buf = disp_drv->draw_buf->buf1;
    } else {
        dst_buf = disp_drv->draw_buf->buf1;
        src_buf = disp_drv->draw_buf->buf2;
    }
    for (int i = 0; i < disp->inv_p; ++i) {
        if (!disp->inv_area_joined[i]) {
            _lv_area_join(&dirty_area, &dirty_area, &disp->inv_areas[i]);
        }
    }
    if (color_p == disp_drv->draw_buf->buf1) {
        sstar_gfx_copy(disp_drv->draw_buf->buf2, color_p, &dirty_area);
    } else {
        sstar_gfx_copy(disp_drv->draw_buf->buf1, color_p, &dirty_area);
    }
    LV_LOG_TRACE("%s-> Calling flush_cb on (%d;%d)(%d;%d) area with %p image pointer, %d\033[0m\n",
            color_p == disp_drv->draw_buf->buf1 ? "\033[0;35m" : "\033[0;36m",
            dirty_area.x1, dirty_area.y1, dirty_area.x2, dirty_area.y2, (void *)color_p, disp_drv->draw_buf->flushing_last);
    sstar_gfx_wait();
    lv_disp_flush_ready(disp_drv);
}

void sstar_wait_cb(struct _lv_disp_drv_t * disp_drv)
{
    sstar_gfx_wait();
}
static void lvgl_disp_drv_init()
{
    static lv_disp_drv_t disp_drv = {0};
    static lv_indev_drv_t input_drv = {0};
    static lv_disp_draw_buf_t disp_buf = {0};

    lv_color_t *buf_1 = NULL;
    lv_color_t *buf_2 = NULL;
    unsigned int pixel_cnt = 0;

    // Lvgl init display buffer
    pixel_cnt = sstar_fbdev_get_xres() * sstar_fbdev_get_yres();
#if SSTAR_GFX_ROTATE
    buf_1 = lv_mem_alloc(pixel_cnt * 4);
    //buf_1 = sstar_fbdev_get_buffer(3);
    buf_2 = NULL; 
#else
    buf_1 = sstar_fbdev_get_buffer(1);
    buf_2 = sstar_fbdev_get_buffer(2);
#endif

    printf("--> %p, %p, %d\n", buf_1, buf_2, pixel_cnt);

    lv_disp_draw_buf_init(&disp_buf, buf_1, buf_2, pixel_cnt);

    // Lvgl init display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
#if (SSTAR_GFX_ROTATE_ANGLE == 1) || (SSTAR_GFX_ROTATE_ANGLE == 3)
    disp_drv.hor_res = sstar_fbdev_get_yres();
    disp_drv.ver_res = sstar_fbdev_get_xres();
#else
    disp_drv.hor_res = sstar_fbdev_get_xres();
    disp_drv.ver_res = sstar_fbdev_get_yres();
#endif
    disp_drv.full_refresh = 1;
    //disp_drv.direct_mode = 1;
    printf("disp_drv.hor_res = %d\n", disp_drv.hor_res);
	printf("disp_drv.ver_res = %d\n", disp_drv.ver_res);
    

    disp_drv.flush_cb = sstar_flush_cb;
    //disp_drv.wait_cb = sstar_wait_cb;
    disp_drv.draw_ctx_init = sstar_draw_ctx_init_cb;
    disp_drv.draw_ctx_deinit = sstar_draw_ctx_deinit_cb;
    disp_drv.draw_ctx_size = sizeof(lv_draw_sw_ctx_t);

    // disp_drv.sw_rotate = 1;
    // disp_drv.rotated = LV_DISP_ROT_270;

    lv_disp_drv_register(&disp_drv);

    // Lvgl init input driver
    lv_indev_drv_init(&input_drv);
    input_drv.type = LV_INDEV_TYPE_POINTER;
    input_drv.read_cb = evdev_read;

    lv_indev_drv_register(&input_drv);
}

static void lvgl_disp_drv_deinit(void)
{
}

int sstar_lv_init(void)
{
    if (0 != sstar_sys_init()) {
        goto ERR_SYS_INIT;
    }
    if (0 != sstar_fbdev_init()) {
        goto ERR_FBDEV_INIT;
    }
    if (0 != sstar_disp_init(0, "ttl", 1024, 600)) {
        goto ERR_DISP_INIT;
    }
    if (0 != sstar_gfx_init()) {
        goto ERR_GFX_INIT;
    }

    lv_init();
    evdev_init();
    lvgl_disp_drv_init();
    return 0;
ERR_GFX_INIT:
    sstar_disp_deinit(0, "ttl");
ERR_DISP_INIT:
    sstar_fbdev_deinit();
ERR_FBDEV_INIT:
    sstar_sys_deinit();
ERR_SYS_INIT:
    return -1;
}

void sstar_lv_deinit(void)
{
    lvgl_disp_drv_deinit();

    sstar_gfx_deinit();
    sstar_fbdev_deinit();
    sstar_disp_deinit(0, "ttl");
    sstar_pool_free();
    sstar_sys_deinit();
}

