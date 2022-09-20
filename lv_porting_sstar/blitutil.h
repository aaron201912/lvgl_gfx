#include "mi_gfx_datatype.h"

#define RECTW(r) (r.right-r.left)
#define RECTH(r) (r.bottom-r.top)

typedef struct RECT
{
    int top;
    int bottom;
    int left;
    int right;
}RECT;

typedef struct Rect
{
    int x;
    int y;
    int w;
    int h;
}Rect;

typedef struct Surface
{
    int w;
    int h;
    int pitch;
    int BytesPerPixel;
    MI_GFX_ColorFmt_e eGFXcolorFmt;
    unsigned long long phy_addr;
}Surface;


void SstarBlitCCW(MI_GFX_DEV GfxDevId, Surface * pSrcSurface, Surface *pDstSurface, RECT* pRect);
void SstarBlitCW(MI_GFX_DEV GfxDevId, Surface * pSrcSurface, Surface *pDstSurface, RECT* pRect);
void SstarBlitHVFlip(MI_GFX_DEV GfxDevId, Surface * pSrcSurface, Surface *pDstSurface, RECT* pRect);
void SstarBlitNormal(MI_GFX_DEV GfxDevId, Surface * pSrcSurface, Surface *pDstSurface, RECT* pRect);

