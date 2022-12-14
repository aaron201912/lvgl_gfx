#include <time.h>
#include "lv_log.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static struct timespec ts1, ts2;
double posix_dur;
#define JDEC_PERF(ts1, ts2,cnt)                                                                                \
    {                                                                                                      \
        static int count = cnt;                                                                            \
        static double dur = 0.0;                                                                           \
        clock_gettime(CLOCK_MONOTONIC, &ts2);                                                     \
        posix_dur = 1000.0 * ts2.tv_sec + 1e-6 * ts2.tv_nsec - (1000.0 * ts1.tv_sec + 1e-6 * ts1.tv_nsec); \
        if (unlikely(dur - 0.0 < 0.001))                                                                   \
        {                                                                                                  \
            dur = posix_dur;                                                                               \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            dur = (dur + posix_dur) / 2;                                                                   \
        }                                                                                                  \
        count--;                                                                                           \
        if (unlikely(count == 0))                                                                          \
        {                                                                                                  \
            LV_LOG_TRACE("CPU time used (%s->%d): %.2f ms\n", __FUNCTION__,__LINE__, dur);                       \
            count = cnt;                                                                                   \
            dur = 0.0;                                                                                     \
        }                                                                                                  \
        clock_gettime(CLOCK_MONOTONIC, &ts1);                                                     \
    }

#define SINK_SURFACE (1)

typedef struct
{
    MI_GFX_Surface_t srcSurf;
    MI_GFX_Rect_t srcRect;
    MI_GFX_Surface_t dstSurf;
    MI_GFX_Rect_t dstRect;
} MI_GFX_DemoParam_t;

#ifndef MI_GFX_DEV
typedef MI_S32 MI_GFX_DEV;
#endif
