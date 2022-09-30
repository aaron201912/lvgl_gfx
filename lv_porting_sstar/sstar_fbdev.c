/*
 * sstar_fbdev.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "lvgl.h"
#include "lv_conf.h"

#include "sstar_fbdev.h"
#include "tc_rotate.h"
#include "sstar_memory.h"
#include "verify_gfx.h"

#define MAX_H ((unsigned int)1<<32-1)
#define MAX_W ((unsigned int)1<<32-1)

static int fd;
static struct fb_fix_screeninfo finfo;
static struct fb_var_screeninfo vinfo;
static char *framebuffer;

void sstar_fbdev_flush(const lv_color_t *color_p)
{
#if SSTAR_GFX_ROTATE
    #if (LV_COLOR_DEPTH !=32)
    #error Sstar gfx open rotate but color depth is not ARGB8888
    #else
        MI_GFX_DemoParam_t stParam;
        MI_GFX_Rect_t dstRect;
        MI_GFX_Rect_t srcRect;
        MI_GFX_Surface_t dstSurf;
        MI_GFX_Surface_t srcSurf;
        MI_PHY dstPa;
        MI_PHY srcPa;

		sstar_flush_cache();

        memset(&srcSurf, 0x0, sizeof(srcSurf));
        memset(&dstSurf, 0x0, sizeof(dstSurf));
        memset(&srcRect, 0x0, sizeof(srcRect));
        memset(&dstRect, 0x0, sizeof(dstRect));

        dstRect.s32Xpos = 0;
        dstRect.s32Ypos =  0;
        dstRect.u32Height = vinfo.xres - 1;
        dstRect.u32Width = vinfo.yres - 1;

        srcRect.s32Xpos = 0;
        srcRect.s32Ypos =  0;
        srcRect.u32Height = vinfo.xres - 1;
        srcRect.u32Width = vinfo.yres - 1;

        if(vinfo.yoffset)
        {
			dstPa = sstar_fbdev_va2pa(framebuffer);
        }
        else
        {
            LV_LOG_TRACE("finfo.line_length = %d \n",finfo.line_length);
            dstPa = sstar_fbdev_va2pa(framebuffer + vinfo.yres * finfo.line_length);
        }
        dstSurf.eColorFmt = E_MI_GFX_FMT_ARGB8888;
        dstSurf.phyAddr = dstPa;
        dstSurf.u32Height = vinfo.xres;
        dstSurf.u32Width = vinfo.yres;
        dstSurf.u32Stride = dstSurf.u32Width * getBpp(srcSurf.eColorFmt);

	#if SSTAR_GFX_ROTATE
		srcPa = sstar_pool_va2pa(color_p);
	#else
		srcPa = sstar_fbdev_va2pa(color_p);
	#endif        
        srcSurf.eColorFmt = E_MI_GFX_FMT_ARGB8888;
        srcSurf.phyAddr = srcPa;
        srcSurf.u32Height = vinfo.xres;
        srcSurf.u32Width = vinfo.yres;
        srcSurf.u32Stride = srcSurf.u32Width * getBpp(srcSurf.eColorFmt);
        
        memset(&stParam, 0, sizeof(stParam));
        stParam.dstRect = dstRect;
        stParam.srcRect = srcRect;
        stParam.dstSurf = dstSurf;
        stParam.srcSurf = srcSurf; 

        #if SSTAR_GFX_ROTATE_ANGLE == 1
            __test_rotate_ARGB_90(1, stParam, NULL, 0);
        #elif SSTAR_GFX_ROTATE_ANGLE == 2
            __test_rotate_ARGB_180(1, stParam, NULL, 0);
        #elif SSTAR_GFX_ROTATE_ANGLE == 3
            __test_rotate_ARGB_270(1, stParam, NULL, 0);
        #else
            __test_rotate_ARGB_0(1, stParam, NULL, 0);
        #endif

    #endif
    uint32_t yoffset = vinfo.yoffset == 0 ? vinfo.yres : 0;
    if (yoffset == vinfo.yoffset) {
        return;
    }
    vinfo.yoffset = yoffset;
    if (-1 == ioctl(fd, FBIOPAN_DISPLAY, &vinfo)) {
        printf("BUG_ON: FBIOPAN_DISPLAY.\n");
        exit(-1);
    }
#else
    uint32_t yoffset = (char*)color_p == framebuffer ? 0 : vinfo.yres;
    if (yoffset == vinfo.yoffset) {
        return;
    }
    vinfo.yoffset = yoffset;
    if (-1 == ioctl(fd, FBIOPAN_DISPLAY, &vinfo)) {
        printf("BUG_ON: FBIOPAN_DISPLAY.\n");
        exit(-1);
    }
#endif
}


unsigned int sstar_fbdev_get_xres()
{
    return vinfo.xres;
}

unsigned int sstar_fbdev_get_yres()
{
    return vinfo.yres;
}

unsigned int sstar_fbdev_get_bpp()
{
    return vinfo.bits_per_pixel;
}

unsigned long sstar_fbdev_va2pa(void *ptr)
{
    return finfo.smem_start + ((char*)ptr - (char*)framebuffer);
}

void *sstar_fbdev_get_buffer(int buf_i)
{
    if (vinfo.yres_virtual >= vinfo.yres * buf_i) {
        return framebuffer + vinfo.yres * finfo.line_length * ( buf_i - 1 );
    }
    return NULL;
}

int sstar_fbdev_init()
{
    const char *fb_dev_name = "/dev/fb0";

    // Open fb_dev
    fd = open(fb_dev_name, O_RDWR);
    if (fd == -1) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        return -1;
    }

    // Get finfo and vinfo
    if (-1 == ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        close(fd);
        return -1;
    }
    if (-1 == ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        close(fd);
        return -1;
    }

    printf(">>>>>> %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    framebuffer =
        mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (NULL == framebuffer) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        close(fd);
        return -1;
    }

    printf("%p <-> %lx\n", framebuffer, finfo.smem_start);

    return 0;
}

void sstar_fbdev_deinit()
{
    munmap(framebuffer, finfo.smem_len);
    framebuffer = NULL;
    close(fd);
}

