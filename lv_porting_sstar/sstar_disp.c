/*
 * sstar_disp.c
 */

#include <stdio.h>
#include <string.h>

#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"

#include "sstar_disp.h"

#ifndef SSTAR_DISP_DEV
#define SSTAR_DISP_DEV (0)
#endif

#ifndef SSTAR_DISP_INTF
#define SSTAR_DISP_INTF "ttl"
#endif

#ifndef SSTAR_DISP_WIDTH
#define SSTAR_DISP_WIDTH (1024)
#endif

#ifndef SSTAR_DISP_HEIGHT
#define SSTAR_DISP_HEIGHT (600)
#endif

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)

int sstar_disp_init(unsigned int dev, const char *interface, unsigned int width, unsigned int height)
{
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;

    memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
    stPubAttr.u32BgColor = YUYV_BLACK;

    memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = width;
    stLayerAttr.stVidLayerDispWin.u16Height = height;
    stLayerAttr.stVidLayerSize.u16Width = width;
    stLayerAttr.stVidLayerSize.u16Height = height;
    stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
    stInputPortAttr.stDispWin.u16X = 0;
    stInputPortAttr.stDispWin.u16Y = 0;
    stInputPortAttr.stDispWin.u16Width = width;
    stInputPortAttr.stDispWin.u16Height = height;
    stInputPortAttr.u16SrcWidth = width;
    stInputPortAttr.u16SrcHeight = height;

    if (0 == strcmp(interface, "ttl")) {
        stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
        stPubAttr.eIntfType = E_MI_DISP_INTF_TTL;
    }
    else if(0 == strcmp(interface, "mipi"))
    {
        stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
        stPubAttr.eIntfType = E_MI_DISP_INTF_MIPIDSI;
    }

    if (MI_SUCCESS != MI_DISP_SetPubAttr(dev, &stPubAttr)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        goto DISP_ENABLE_ERR;
    }
    if (MI_SUCCESS != MI_DISP_Enable(dev)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        goto DISP_ENABLE_ERR;
    }
    if (MI_SUCCESS != MI_DISP_BindVideoLayer(0, dev)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        goto DISP_BIND_LAYER_ERR;
    }
    if (MI_SUCCESS != MI_DISP_SetVideoLayerAttr(0, &stLayerAttr)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        goto DISP_SET_LAYER_ATTR_ERR;
    }
    if (MI_SUCCESS != MI_DISP_EnableVideoLayer(0)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        goto DISP_ENABLE_LAYER_ERR;
    }
    if (MI_SUCCESS != MI_DISP_SetInputPortAttr(0, 0, &stInputPortAttr)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        goto DISP_SET_PORT_ERR;
    }
    if (MI_SUCCESS != MI_DISP_EnableInputPort(0, 0)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        goto DISP_ENABLE_PORT_ERR;
    }

	if (0 == strcmp(interface, "ttl")) {
        if (MI_SUCCESS != MI_PANEL_Init(E_MI_PNL_INTF_TTL)) {
            printf("ERR %s -> [%d]", __FILE__, __LINE__);
            goto PANEL_INIT_ERR;
        }
    }
    else if(0 == strcmp(interface, "mipi"))
    {
        if (MI_SUCCESS != MI_PANEL_Init(E_MI_PNL_INTF_MIPI_DSI)) {
            printf("ERR %s -> [%d]", __FILE__, __LINE__);
            goto PANEL_INIT_ERR;
        }
    }

    return 0;
PANEL_INIT_ERR:
	//MI_DISP_DisableInputPort(0, 0);
DISP_ENABLE_PORT_ERR:
DISP_SET_PORT_ERR:
    MI_DISP_DisableVideoLayer(0);
DISP_ENABLE_LAYER_ERR:
DISP_SET_LAYER_ATTR_ERR:
    MI_DISP_UnBindVideoLayer(0, dev);
DISP_BIND_LAYER_ERR:
    MI_DISP_Disable(dev);
DISP_ENABLE_ERR:

    return -1;
}

void sstar_disp_deinit(unsigned int dev, const char *interface)
{
	if (0 == strcmp(interface, "ttl") || 0 == strcmp(interface, "mipi")) {
        MI_PANEL_DeInit();
    }
    //MI_DISP_DisableInputPort(0, 0);
    MI_DISP_DisableVideoLayer(0);
    MI_DISP_UnBindVideoLayer(0, dev);
    MI_DISP_Disable(dev);
}


