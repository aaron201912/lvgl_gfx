/*
 * test.c
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "lv_obj.h"
#include "sstar_port.h"
#include "lv_label.h"
#include "lv_disp.h"
#include "lv_style.h"
#include "lv_demos.h"
#include <sys/time.h>

#include "widgets/lv_demo_widgets.h"
#include "music/lv_demo_music.h"
#include "benchmark/lv_demo_benchmark.h"
//#include "ui.h"


#define PATH_IMAGE	"/customer/"


void *tick_thread(void * data)
{
    (void)data;

    while(1) {
        usleep(1000);
        lv_tick_inc(1); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

unsigned int _GetTime0()
{
    struct timespec ts;
    unsigned int ms;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    if(ms == 0)
    {
        ms = 1;
    }
    return ms;
}
int main(int argc, char *argv[])
{
	char test_path[1024] = { 0 };
    pthread_t pt;
    if (0 != sstar_lv_init()) {
        printf("ERR: sstar_lv_init failed.\n");
        return -1;
    }

    if (0 == strcmp(argv[1], "a")) {
        lv_demo_music();
    } else if (0 == strcmp(argv[1], "b")) {
        lv_demo_widgets();
    } else if (0 == strcmp(argv[1], "c")) {
        lv_demo_benchmark();
    } else if (0 == strcmp(argv[1], "d")) {
		lv_example_png_1();
		lv_example_gif_1();
		//lv_example_qrcode_1();
		const char * data = "Hello world";

		/*Create a 100x100 QR code*/
		lv_obj_t * qr = lv_qrcode_create(lv_scr_act(), 200, lv_color_hex3(0x33f), lv_color_hex3(0xeef));

		/*Set data*/
		lv_qrcode_update(qr, data, strlen(data));
		
        //ui_init();
    }

    pthread_create(&pt, NULL, tick_thread, NULL);
    while(1) {
        unsigned int curr = _GetTime0();
        lv_task_handler();
        unsigned int time_diff = _GetTime0() - curr;
        if (time_diff < 10) {
            usleep(( 10 - time_diff ) * 1000);
        }
    }
    pthread_join(pt, NULL);

    sstar_lv_deinit();
    return 0;
}
