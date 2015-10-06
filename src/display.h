#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>

#define SCR_HEIGHT 240
#define SCR_WIDTH  320
#define BOX_HEIGHT 122
#define BOX_WIDTH  202
#define CODE_WIDTH (18*6 + 2)
#define BOX_TOP (SCR_HEIGHT / 2 - BOX_HEIGHT / 2)
#define BOX_BOTTOM (SCR_HEIGHT / 2 + BOX_HEIGHT / 2)
#define BOX_LEFT (SCR_WIDTH / 2 - BOX_WIDTH / 2)
#define BOX_RIGHT (SCR_WIDTH / 2 + BOX_WIDTH / 2)
#define BOX_SPLIT (BOX_LEFT + CODE_WIDTH)

#define INFO_HEIGHT (BOX_HEIGHT/5)
#define INFO_WIDTH (BOX_WIDTH - (BOX_SPLIT - BOX_LEFT))

extern volatile char labels[15][19];

void initScreen();
void redrawScreen();
void updateScreen();

#endif
