#pragma once

#include "theClock3.h"

extern int clock_show_pixels;

extern void showPixels();
extern void clearPixels();
extern bool pixelsCanShow();
extern void setPixelsBrightness(int val);
extern void setPixel(int num, uint32_t color);
extern uint32_t scalePixel(int amt, int scale, uint32_t color0, uint32_t color1, uint32_t color2);

// Semaphore between AS5600 and showPixels()

extern void WAIT_SEMAPHORE();
extern void RELEASE_SEMAPHORE();



