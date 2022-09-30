/*
 * sstar_sys.c
 */

#include <stdio.h>
#include "sstar_sys.h"

#include "mi_common_datatype.h"
#include "mi_sys.h"

int sstar_sys_init()
{
#ifdef CHIP_i2m
    if (MI_SUCCESS != MI_SYS_Init()) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        return -1;
    }
#else
    if (MI_SUCCESS != MI_SYS_Init(0)) {
        printf("ERR %s -> [%d]", __FILE__, __LINE__);
        return -1;
    }
#endif

    return 0;
}

void sstar_sys_deinit()
{
#ifdef CHIP_i2m
    MI_SYS_Exit();
#else
    MI_SYS_Exit(0);
#endif
}
