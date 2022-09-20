/*
 * sstar_memory.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mi_sys_datatype.h"
#include "mi_sys.h"

#include "sstar_memory.h"

static MI_PHY g_phyAddr;
static void* g_virAddr;
static size_t g_size;

MI_PHY sstar_pool_va2pa(const void *ptr)
{
    return g_phyAddr + ( (char*)ptr - (char*)g_virAddr );
}

int sstar_pool_check(const void *ptr)
{
    return ( ptr < g_virAddr || ptr >= (void*)( g_virAddr + g_size ) ) ? -1 : 0;
}

void sstar_flush_cache()
{
    MI_SYS_FlushInvCache(g_virAddr, g_size);
}

void *sstar_pool_malloc(size_t size)
{
    if (g_virAddr != NULL || g_phyAddr != 0) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        return NULL;
    }
    if (size == 0) {
        return NULL;
    }
    if (MI_SUCCESS != MI_SYS_MMA_Alloc(0, NULL, size, &g_phyAddr)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        exit(-1);
    }
    if (MI_SUCCESS != MI_SYS_Mmap(g_phyAddr, size, (void**)&g_virAddr, TRUE)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        exit(-1);
    }
    g_size = size;
    printf("sstar_pool_malloc : virAddr> %p, phyAddr> %llx, size> %lu\n", g_virAddr, g_phyAddr, g_size);
    return g_virAddr;
}

void sstar_pool_free()
{
    if (g_virAddr == NULL || g_phyAddr == 0) {
        return;
    }
    if (MI_SUCCESS != MI_SYS_Munmap(g_virAddr, g_size)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        return ;
    }
    if (MI_SUCCESS != MI_SYS_MMA_Free(0, g_phyAddr)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        return ;
    }
    g_virAddr = 0;
    g_phyAddr = 0;
    g_size = 0;
}

