/*
 * sstar_fbdev.h
 */

#ifndef _SSTAR_FBDEV_H_
#define _SSTAR_FBDEV_H_

int sstar_fbdev_init();
void sstar_fbdev_deinit();

void *sstar_fbdev_get_buffer(int buf_i);
unsigned int sstar_fbdev_get_xres();
unsigned int sstar_fbdev_get_yres();
unsigned int sstar_fbdev_get_bpp();
unsigned long sstar_fbdev_va2pa(void *ptr);

void sstar_fbdev_flush(const lv_color_t *color_p);
#endif /* _SSTAR_FBDEV_H_ */
