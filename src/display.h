#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>

extern volatile char labels[15][19];

void updateScreen();

#endif
