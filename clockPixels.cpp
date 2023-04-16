// clockPixels.cpp
//
// Native pixel brightness is weird!!  0==max, 1=min, 255=max-1
// I change it so that 0=min, and 254=max-1 and never send 0

#include "clockPixels.h"
//#include <myIOTLog.h>
#include <Adafruit_NeoPixel.h>


int clock_show_pixels = 0;

static Adafruit_NeoPixel pixels(NUM_PIXELS,PIN_LEDS);

//------------------------------------------
// Semaphore between AS5600 and showPixels()
//------------------------------------------

static volatile int sem_count = 0;
static volatile int the_semaphore = 0;

void WAIT_SEMAPHORE()
{
	while (the_semaphore) {sem_count++;}
	the_semaphore = 1;
}

void RELEASE_SEMAPHORE()
{
	the_semaphore = 0;
}




//-----------------------------
// pass thrus
//-----------------------------


void showPixels()
{
	pixels.show();
}

void clearPixels()
{
	pixels.clear();
}

bool pixelsCanShow()
{
	return pixels.canShow();
}

void setPixelsBrightness(int val)
{
	pixels.setBrightness(val);
}

void setPixel(int num, uint32_t color)
{
	pixels.setPixelColor(num,color);
}


//-----------------------------
// functionality
//-----------------------------

uint32_t scalePixel(int amt, int scale, uint32_t color0, uint32_t color1, uint32_t color2)
{
	// LOGV("scalePixel(%d,%d,0x%06x,0x%06x,0x%06x)",amt,scale,color0,color1,color2);

	float fdif = amt;
	fdif /= ((float)scale);
	if (fdif < -1) fdif = -1;
	else if (fdif > 1) fdif = 1;
	// LOGV("   fdif=%0.3f",fdif);

	uint32_t retval = 0;
	uint8_t *ca = (uint8_t *)&color1;
	uint8_t *cb = (uint8_t *)&color2;

	if (fdif < 0)
	{
		fdif = -fdif;
		ca = (uint8_t *)&color1;
		cb = (uint8_t *)&color0;
	}

	for (int i=0; i<3; i++)
	{
		int base = ca[i];			// assume base is the middle
		int val = cb[i] - ca[i];    // and that it increaes as we move outward
		uint8_t byte =  base + (fdif * val);
		// LOGV("base=0x%02x  val=0x%02x   byte=0x%02x",base,val,byte);
		retval |= (byte << 8 * i);
	}
	// LOGV("    retval=0x%08x",retval);
	return retval;
}
