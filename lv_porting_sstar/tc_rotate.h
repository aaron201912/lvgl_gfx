#include <mi_gfx.h>
#include <mi_gfx_datatype.h>
#include <mi_sys.h>

#include "verify_gfx_type.h"

int __create_src_surface_I8(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *srcRect, char **data);
int __create_src_surface_ARGB(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect, char **data);
int __refill_dst_surface_ARGB(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect);
int __create_dst_surface_ARGB(MI_GFX_DEV GfxDevId, MI_GFX_Surface_t *srcSurf, MI_GFX_Rect_t *Rect, char **data);
int __fill_surface_YUV420SP(MI_GFX_Surface_t *srcSurf, MI_GFX_Surface_t *dstSurf, char *srcData, char *dstData);
int __test_rotate_ARGB_0(MI_GFX_DEV GfxDevId, MI_GFX_DemoParam_t stParam, char *dstData, MI_BOOL bSinkSurf);
int __test_rotate_ARGB_90(MI_GFX_DEV GfxDevId, MI_GFX_DemoParam_t stParam, char *dstData, MI_BOOL bSinkSurf);
int __test_rotate_ARGB_180(MI_GFX_DEV GfxDevId, MI_GFX_DemoParam_t stParam, char *dstData, MI_BOOL bSinkSurf);
int __test_rotate_ARGB_270(MI_GFX_DEV GfxDevId, MI_GFX_DemoParam_t stParam, char *dstData, MI_BOOL bSinkSurf);
int __test_rotate_YUV420SP_0(MI_GFX_DEV GfxDevId, MI_GFX_DemoParam_t stParam, char *dstData, MI_BOOL bSinkSurf);
int __test_rotate_YUV420SP_90(MI_GFX_DEV GfxDevId, MI_GFX_DemoParam_t stParam, char *dstData, MI_BOOL bSinkSurf);
int __test_rotate_YUV420SP_180(MI_GFX_DEV GfxDevId, MI_GFX_DemoParam_t stParam, char *dstData, MI_BOOL bSinkSurf);
int __test_rotate_YUV420SP_270(MI_GFX_DEV GfxDevId, MI_GFX_DemoParam_t stParam, char *dstData, MI_BOOL bSinkSurf);
