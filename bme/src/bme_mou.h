#ifndef BME_MOU_H
#define BME_MOU_H

// BME mouse functions header file

void mou_init(void);
void mou_uninit(void);
void mou_getpos(unsigned *x, unsigned *y);
void mou_getmove(int *dx, int *dy);
unsigned mou_getbuttons(void);

#endif
