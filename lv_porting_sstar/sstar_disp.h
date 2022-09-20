/*
 * sstar_disp.h
 */

#ifndef _SSTAR_DISP_H_
#define _SSTAR_DISP_H_

int sstar_disp_init(unsigned int dev, const char *interface, unsigned int width, unsigned int height);
void sstar_disp_deinit(unsigned int dev, const char *interface);

#endif /* _SSTAR_DISP_H_ */
