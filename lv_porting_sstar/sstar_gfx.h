/*
 * sstar_gfx.h
 */

#ifndef _SSTAR_GFX_H
#define _SSTAR_GFX_H

int sstar_gfx_init();
void sstar_gfx_deinit();
void sstar_gfx_blend_cb(lv_draw_ctx_t * draw_ctx, const lv_draw_sw_blend_dsc_t * dsc);
void sstar_gfx_wait();
void sstar_gfx_copy(lv_color_t *dst, lv_color_t *src, lv_area_t *area);
void sstar_bdma_copy(lv_color_t *dst, lv_color_t *src, lv_area_t *area);
lv_res_t sstar_gfx_draw_img_cb(struct _lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * draw_dsc,
                     const lv_area_t * coords, const void * src);

#endif
