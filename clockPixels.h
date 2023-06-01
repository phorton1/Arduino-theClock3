#pragma once

#include "theClock3.h"

extern void showPixels();
extern void clearPixels();
extern bool pixelsCanShow();
extern void setPixelsBrightness(int val);
extern void setPixel(int num, uint32_t color);

extern uint32_t scalePixel(float pct, uint32_t color0, uint32_t color1);
	// scale pixels a percentage 0..1 between two colors
extern uint32_t scalePixel3(int amt, int scale, uint32_t color0, uint32_t color1, uint32_t color2);
	// scales a pixel given a value amt  from -scale to +scale

// Semaphore between AS5600 and showPixels()

extern void WAIT_SEMAPHORE();
extern void RELEASE_SEMAPHORE();
