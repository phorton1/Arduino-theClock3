// clockAS5600.cpp

#include "clockAS5600.h"
#include "clockStats.h"
#include "clockPixels.h"
#include <myIOTLog.h>
#include <AS5600.h>
#include <Wire.h>


#define AS4500_THRESHOLD    4	// 5 required for change == about 0.5 degrees

int as5600_cur;			// bits, not degrees
int as5600_side;			// which side of zero is the pendulum on?
int as5600_direction;	// which direction is it moving?

int as5600_min;			// min and max are assigned on direction changes
int as5600_max;
int as5600_temp_min;		// and the temps are reset to zero for next go round
int as5600_temp_max;

// angles are calculated from integers when they change

float as5600_cur_angle;
float as5600_min_angle;
float as5600_max_angle;

AS5600 as5600;   //  uses default Wire


float angleOf(int units)
{
	float retval = units * 3600;
	retval /= 4096;
	return floor(retval + 0.5) / 10;
}


void startAS5600()
{
	bool connected = false;
	while (!connected)
	{
		as5600.begin();  //  set direction pin.
		as5600.setDirection(AS5600_CLOCK_WISE);  // default, just be explicit.
		connected = as5600.isConnected();
		if (!connected)
		{
			LOGE("Could not connect to AS5600");
			for (int i=0; i<11; i++)
			{
				setPixel(PIXEL_MAIN,i&1?MY_LED_RED:MY_LED_BLACK);
				showPixels();
				delay(300);
			}
		}
	}
	LOGU("AS5600 connected=%d",connected);
}


void initAS5600(int zero_angle)
{
	as5600_cur = zero_angle;
		// initial angle starts as biased zero
	as5600_side = 0;
	as5600_direction = 0;
	as5600_min = 0;
	as5600_max = 0;
	as5600_temp_min = MAX_INT;
	as5600_temp_max = MIN_INT;
	as5600_cur_angle = 0;
	as5600_min_angle = 0;
	as5600_max_angle = 0;
}


int getAS5600Raw()
{
	WAIT_SEMAPHORE();
	int raw = as5600.readAngle();
	RELEASE_SEMAPHORE();
	return raw;
}


float getAS560AverageAngle()
{
	float avg_angle = (abs(as5600_min_angle) + abs(as5600_max_angle)) / 2;
	return avg_angle;
}


int getAS5600Angle(int zero_angle, int *retval, bool update_direction)
{
	// Read, but don't necessariy use the as5600 angle
	// Note that sometimes, particularly when first starting, I get bogus readings here.
	//
	// I think the bogus readings *may* be related to neopixels disabling interrupts
	// or something going on in Wifi etc.
	//
	// Using a semaphore around readAngle() and showPixels() seems to help,
	// but does not eliminate the problem, so we also compare the angle to
	// some arbitrary value (15 degrees) and bail on this time through
	// the loop if it's larger than that.

	int raw = getAS5600Raw();
	int cur = raw - zero_angle;
	float cur_angle = angleOf(cur);
	if (abs(cur_angle) > MAX_ALLOWABLE_ANGLE)
	{
		incStatBadReads();
		LOGE("Bogus angle reading raw=%d cur=%d zero=%d angle=%0.3f",raw,cur,zero_angle,cur_angle);
		return -1;
	}

	as5600_cur_angle = angleOf(cur);

	// min and max are intrinsicly debounced
	// set temp min and max per cycle

	if (cur < as5600_temp_min)
		as5600_temp_min = cur;
	if (cur > as5600_temp_max)
		as5600_temp_max = cur;

	// detect direction change or zero crossing
	// only if the position has changed significantly

	int dif = cur - as5600_cur;
	if (abs(dif) > AS4500_THRESHOLD)
		as5600_cur = cur;
	else
		dif = 0;

	// handle direction changes too ...

	if (dif && update_direction)
	{
		int dir = as5600_direction;
		if (dif < 0)
			dir = -1;
		else if (dif > 0)
			dir = 1;
		if (as5600_direction != dir)
		{
			as5600_direction = dir;

			if (dir > 0)
			{
				as5600_min = as5600_temp_min;
				as5600_temp_min = MAX_INT;
				as5600_min_angle = angleOf(as5600_min);
				updateStatsMinAngle(as5600_min_angle);
			}
			else
			{
				as5600_max = as5600_temp_max;
				as5600_temp_max = MIN_INT;
				as5600_max_angle = angleOf(as5600_max);
				updateStatsMaxAngle(as5600_max_angle);
			}
		}	// direction changed
	}

	*retval = cur;
	return dif ? 1 : 0;
}


bool getAS5600ZeroCrossing()
	// based on most recent call to getAS5600Angle()
{
	if ((as5600_cur < 0 && as5600_side >= 0) ||
		(as5600_cur > 0 && as5600_side <= 0))
	{
		as5600_side = as5600_cur < 0 ? -1 : 1;
		return 1;
	}
	return 0;
}
