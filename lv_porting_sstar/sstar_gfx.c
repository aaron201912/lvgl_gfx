/*
 * sstar_gfx.c
 */

#include<stdio.h>

#include "lv_log.h"
#include "lv_area.h"
#include "lv_draw_img.h"
#include "lv_draw_sw.h"

#include "mi_sys.h"
#include "sstar_memory.h"
#include "sstar_fbdev.h"
#include "sstar_gfx.h"

#include "mi_gfx_datatype.h"
#include "mi_gfx.h"

#ifndef SSTAR_IMG_ARR_SIZE
#define SSTAR_IMG_ARR_SIZE (16)
#endif
#define DEFAULT_GFX_DEV_ID (0)

static void __sstar_print_surface(const char *name, const MI_GFX_Surface_t *pstSurface)
{
    printf("%s > %llx, %d, %d, %d, %d\n", name, pstSurface->phyAddr, pstSurface->eColorFmt,
            pstSurface->u32Width, pstSurface->u32Height, pstSurface->u32Stride);
}
static void __sstar_print_rect(const char *name, const MI_GFX_Rect_t *pstRect)
{
    printf("%s > %d, %d, %d, %d\n", name, pstRect->s32Xpos, pstRect->s32Ypos,
            pstRect->u32Width, pstRect->u32Height);
}
static void __sstar_gfx_make_surface(const MI_PHY phy_addr, const lv_coord_t width, const lv_coord_t height,
        const lv_coord_t stride, unsigned int bpp, MI_GFX_Surface_t *pstGfxSurface)
{
    if (bpp == 32) {
        pstGfxSurface->eColorFmt = E_MI_GFX_FMT_ARGB8888;
    } else if (bpp == 16) {
        pstGfxSurface->eColorFmt = E_MI_GFX_FMT_RGB565;
    } else if (bpp == 8) {
        pstGfxSurface->eColorFmt = E_MI_GFX_FMT_I8;
    } else {
        printf("BUG_ON: NOT SUPPORT ");
    }
#if SSTAR_GFX_ROTATE 
    pstGfxSurface->phyAddr = phy_addr;
    pstGfxSurface->u32Width = height;//width;
    pstGfxSurface->u32Height = width;//height;
    pstGfxSurface->u32Stride = pstGfxSurface->u32Width * getBpp(E_MI_GFX_FMT_ARGB8888);//stride;
    
#else
	pstGfxSurface->phyAddr = phy_addr;
    pstGfxSurface->u32Width = width;
    pstGfxSurface->u32Height = height;
    pstGfxSurface->u32Stride = stride;
#endif
}
static void __sstar_gfx_make_rect(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, MI_GFX_Rect_t *pstGfxRect)
{
    pstGfxRect->s32Xpos = x;
    pstGfxRect->s32Ypos = y;
    pstGfxRect->u32Width = w;
    pstGfxRect->u32Height = h;
}

static void __sstar_bdma_fill(lv_color_t * dest_buf, const lv_area_t * dest_area, lv_coord_t dest_stride,
        lv_color_t color)
{
    MI_SYS_FrameData_t stBuf = {0};
    MI_SYS_WindowRect_t stRect = {0};

    unsigned int bpp = sstar_fbdev_get_bpp();
    if (bpp == 32) {
        stBuf.ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888;
    } else if (bpp == 16) {
        stBuf.ePixelFormat = E_MI_SYS_PIXEL_FRAME_RGB565;
    } else if (bpp == 8) {
        stBuf.ePixelFormat = E_MI_SYS_PIXEL_FRAME_I8;
    } else {
        printf("BUG_ON: NOT SUPPORT ");
    }
    stBuf.pVirAddr[0] = dest_buf;
#if SSTAR_GFX_ROTATE
	stBuf.phyAddr[0] = sstar_pool_va2pa(dest_buf);
#else
	stBuf.phyAddr[0] = sstar_fbdev_va2pa(dest_buf);
#endif  
    stBuf.u16Width = sstar_fbdev_get_xres();
    stBuf.u16Height = sstar_fbdev_get_yres();
    stBuf.u32Stride[0] = sstar_fbdev_get_xres() * sizeof(lv_color_t);

    stRect.u16X = dest_area->x1;
    stRect.u16Y = dest_area->y1;
    stRect.u16Width = lv_area_get_width(dest_area);
    stRect.u16Height = lv_area_get_height(dest_area);
    MI_SYS_BufFillPa(0, &stBuf, color.full, &stRect);
}

static void __sstar_gfx_fill(lv_color_t * dest_buf, const lv_area_t * dest_area, lv_coord_t dest_stride,
        lv_color_t color)
{
    MI_U16 u16Fence = 0;
    MI_GFX_Surface_t stDst = {0};
    MI_GFX_Rect_t stDstRect = {0};

#if SSTAR_GFX_ROTATE
	__sstar_gfx_make_surface(sstar_pool_va2pa(dest_buf), sstar_fbdev_get_xres(), sstar_fbdev_get_yres(),
            dest_stride * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stDst);
#else
	__sstar_gfx_make_surface(sstar_fbdev_va2pa(dest_buf), sstar_fbdev_get_xres(), sstar_fbdev_get_yres(),
        	dest_stride * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stDst);
#endif 

    __sstar_gfx_make_rect(dest_area->x1, dest_area->y1, lv_area_get_width(dest_area), lv_area_get_height(dest_area),
            &stDstRect);
	sstar_flush_cache();
    MI_GFX_QuickFill(DEFAULT_GFX_DEV_ID,&stDst, &stDstRect, (MI_U32)color.full, &u16Fence);
    MI_GFX_WaitAllDone(DEFAULT_GFX_DEV_ID,FALSE, u16Fence);
}

static void __sstar_gfx_blit(lv_color_t * dest_buf, const lv_area_t * dest_area, lv_coord_t dest_stride,
        const lv_color_t * src_buf, lv_coord_t src_stride, lv_opa_t opa, lv_blend_mode_t blend_mode)
{
    MI_U16 u16Fence = 0;
    MI_GFX_Opt_t stOpt = {0};
    MI_GFX_Surface_t stSrc = {0}, stDst = {0};
    MI_GFX_Rect_t stSrcRect = {0}, stDstRect = {0};

#if SSTAR_GFX_ROTATE
	__sstar_gfx_make_surface(sstar_pool_va2pa(dest_buf), sstar_fbdev_get_xres(), sstar_fbdev_get_yres(),
				dest_stride * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stDst);
#else
	__sstar_gfx_make_surface(sstar_fbdev_va2pa(dest_buf), sstar_fbdev_get_xres(), sstar_fbdev_get_yres(),
            dest_stride * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stDst);
#endif 
    __sstar_gfx_make_surface(sstar_pool_va2pa(src_buf), lv_area_get_width(dest_area), lv_area_get_height(dest_area),
            src_stride * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stSrc);
    __sstar_gfx_make_rect(dest_area->x1, dest_area->y1, lv_area_get_width(dest_area), lv_area_get_height(dest_area),
            &stDstRect);
    __sstar_gfx_make_rect(0, 0, lv_area_get_width(dest_area), lv_area_get_height(dest_area), &stSrcRect);
    stOpt.u32GlobalSrcConstColor = opa;
    stOpt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_SRC_PREMULTIPLY;
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_INVSRCALPHA;
    //__sstar_print_rect("Clip", &stOpt.stClipRect);
    //__sstar_print_surface("Dst", &stDst);
    //__sstar_print_rect("Dst", &stDstRect);
    //__sstar_print_surface("Src", &stSrc);
    //__sstar_print_rect("Src", &stSrcRect);
    MI_GFX_BitBlit(DEFAULT_GFX_DEV_ID,&stSrc, &stSrcRect, &stDst, &stDstRect, &stOpt, &u16Fence);
    MI_GFX_WaitAllDone(DEFAULT_GFX_DEV_ID,FALSE, u16Fence);
}

void sstar_bdma_copy(lv_color_t *dst, lv_color_t *src, lv_area_t *area)
{
    MI_SYS_FrameData_t stDstBuf = {0};
    MI_SYS_FrameData_t stSrcBuf = {0};
    MI_SYS_WindowRect_t stDstRect = {0};
    MI_SYS_WindowRect_t stSrcRect = {0};

#if SSTAR_GFX_ROTATE
	stSrcBuf.phyAddr[0]  = sstar_pool_va2pa(src);
#else
	stSrcBuf.phyAddr[0]  = sstar_fbdev_va2pa(src);
#endif 

#if SSTAR_GFX_ROTATE
	stDstBuf.phyAddr[0]  = sstar_pool_va2pa(dst);
#else
	stDstBuf.phyAddr[0]  = sstar_fbdev_va2pa(dst);
#endif 

    stDstBuf.u16Width     = stSrcBuf.u16Width     = sstar_fbdev_get_xres();
    stDstBuf.u16Height    = stSrcBuf.u16Height    = sstar_fbdev_get_yres();
    stDstBuf.u32Stride[0] = stSrcBuf.u32Stride[0] = sstar_fbdev_get_xres() * sizeof(lv_color_t);
    stDstBuf.ePixelFormat = stSrcBuf.ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888;

    stSrcRect.u16X      = stDstRect.u16X      = area->x1;
    stSrcRect.u16Y      = stDstRect.u16Y      = area->y1;
    stSrcRect.u16Width  = stDstRect.u16Width  = lv_area_get_width(area);
    stSrcRect.u16Height = stDstRect.u16Height = lv_area_get_height(area);
    MI_SYS_BufBlitPa(0,&stDstBuf, &stDstRect, &stSrcBuf, &stSrcRect);
}

void sstar_gfx_copy(lv_color_t *dst, lv_color_t *src, lv_area_t *area)
{
    MI_U16 u16Fence = 0;
    MI_GFX_Opt_t stOpt = {0};
    MI_GFX_Surface_t stSrc = {0}, stDst = {0};
    MI_GFX_Rect_t stSrcRect = {0}, stDstRect = {0};

#if SSTAR_GFX_ROTATE
	__sstar_gfx_make_surface(sstar_pool_va2pa(dst), sstar_fbdev_get_xres(), sstar_fbdev_get_yres(),
            sstar_fbdev_get_xres() * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stDst);
    __sstar_gfx_make_surface(sstar_pool_va2pa(src), sstar_fbdev_get_xres(), sstar_fbdev_get_yres(),
            sstar_fbdev_get_xres() * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stSrc);
#else
	__sstar_gfx_make_surface(sstar_fbdev_va2pa(dst), sstar_fbdev_get_xres(), sstar_fbdev_get_yres(),
            sstar_fbdev_get_xres() * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stDst);
    __sstar_gfx_make_surface(sstar_fbdev_va2pa(src), sstar_fbdev_get_xres(), sstar_fbdev_get_yres(),
            sstar_fbdev_get_xres() * sizeof(lv_color_t), sizeof(lv_color_t) << 3, &stSrc);
#endif 

    __sstar_gfx_make_rect(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area), &stDstRect);
    __sstar_gfx_make_rect(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area), &stSrcRect);
    stOpt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    stOpt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    MI_GFX_BitBlit(DEFAULT_GFX_DEV_ID ,&stSrc, &stSrcRect, &stDst, &stDstRect, &stOpt, &u16Fence);
}

void sstar_gfx_blend_cb(lv_draw_ctx_t * draw_ctx, const lv_draw_sw_blend_dsc_t * dsc)
{
    if (dsc->opa <= LV_OPA_MIN) {
        return;
    }
    if (dsc->mask_buf) {
        lv_draw_sw_blend_basic(draw_ctx, dsc);
        return;
    }
    if (dsc->blend_mode != LV_BLEND_MODE_NORMAL) {
        lv_draw_sw_blend_basic(draw_ctx, dsc);
        return;
    }

    lv_area_t blend_area;
    if(!_lv_area_intersect(&blend_area, dsc->blend_area, draw_ctx->clip_area)) {
        return;
    }

    if (lv_area_get_size(&blend_area) < 128) {
        lv_draw_sw_blend_basic(draw_ctx, dsc);
        return;
    }

    lv_color_t * dest_buf = draw_ctx->buf;
    lv_coord_t dest_stride = sstar_fbdev_get_xres();

    const lv_color_t * src_buf = dsc->src_buf;
    lv_coord_t src_stride;
    if(src_buf) {
        src_stride = lv_area_get_width(dsc->blend_area);
        src_buf += src_stride * (blend_area.y1 - dsc->blend_area->y1) + (blend_area.x1 - dsc->blend_area->x1);
    }
    else {
        src_stride = 0;
    }

    if (src_buf == NULL) {
        if (sizeof(lv_color_t) == 32) {
            lv_color_t color = dsc->color;
            color.ch.alpha = dsc->opa;
            __sstar_gfx_fill(dest_buf, &blend_area, dest_stride, color);
            return;
        } else if (dsc->opa >= LV_OPA_MAX) {
            __sstar_gfx_fill(dest_buf, &blend_area, dest_stride, dsc->color);
            return;
        }
    } else {
        if (0 == sstar_pool_check(src_buf)) {
            __sstar_gfx_blit(dest_buf, &blend_area, dest_stride, src_buf, src_stride, dsc->opa, dsc->blend_mode);
            return;
        }
    }

    lv_draw_sw_blend_basic(draw_ctx, dsc);
}

void sstar_gfx_wait()
{
    MI_GFX_WaitAllDone(DEFAULT_GFX_DEV_ID, TRUE, 0);
}

lv_res_t sstar_gfx_draw_img_cb(struct _lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * draw_dsc,
                     const lv_area_t * coords, const void * src)
{
    static struct {
        const void *key;
        lv_img_dsc_t img_dsc;
    }arr_img_dsc[SSTAR_IMG_ARR_SIZE] = {0};

    if (src == NULL) {
        return LV_RES_INV;
    }

    const lv_img_dsc_t *p_img_dsc = src;
    lv_img_src_t src_type = lv_img_src_get_type(src);
    if (src_type == LV_IMG_SRC_VARIABLE) {
        int i = 0;
        for (i = 0; i < SSTAR_IMG_ARR_SIZE && arr_img_dsc[i].key != NULL; ++i) {
            if (src == arr_img_dsc[i].key) {
                p_img_dsc = &arr_img_dsc[i].img_dsc;
                break;
            }
        }
        if (i >= SSTAR_IMG_ARR_SIZE) {
            LV_LOG_WARN("SSTAR_IMG_ARR_SIZE FULL\n");
            draw_ctx->draw_img = NULL;
            lv_draw_img(draw_ctx, draw_dsc, coords, p_img_dsc);
            draw_ctx->draw_img = sstar_gfx_draw_img_cb;
            return LV_RES_OK;
        }
        if (src != arr_img_dsc[i].key) {
            arr_img_dsc[i].key = src;
            arr_img_dsc[i].img_dsc = *(lv_img_dsc_t*)src;
            LV_LOG_INFO("SSTAR Allco memory %d\n", arr_img_dsc[i].img_dsc.data_size);
            void * new_data = lv_mem_alloc(arr_img_dsc[i].img_dsc.data_size);
            lv_memcpy(new_data, arr_img_dsc[i].img_dsc.data, arr_img_dsc[i].img_dsc.data_size);
            sstar_flush_cache();
            arr_img_dsc[i].img_dsc.data = new_data;
            p_img_dsc = &arr_img_dsc[i].img_dsc;
        }
    }
    draw_ctx->draw_img = NULL;
    lv_draw_img(draw_ctx, draw_dsc, coords, p_img_dsc);
    draw_ctx->draw_img = sstar_gfx_draw_img_cb;
    return LV_RES_OK;
}

int sstar_gfx_init()
{
    if (MI_SUCCESS != MI_GFX_Open(DEFAULT_GFX_DEV_ID)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

void sstar_gfx_deinit()
{
    MI_GFX_Close(DEFAULT_GFX_DEV_ID);
}


