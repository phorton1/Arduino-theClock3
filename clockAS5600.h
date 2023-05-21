#pragma once

#include "theClock3.h"

#define MAX_ALLOWABLE_ANGLE  	17.0

extern bool as5600_connected;	// whether the initial connect worked
	// if it does not work, then the user needs to reboot the clock

extern int as5600_cur;			// bits, not degrees
extern int as5600_side;			// which side of zero is the pendulum on?
extern int as5600_direction;	// which direction is it moving?

extern int as5600_min;			// min and max are assigned on direction changes
extern int as5600_max;
extern int as5600_temp_min;		// and the temps are reset to zero for next go round
extern int as5600_temp_max;

// angles are calculated from integers when they change

extern float as5600_cur_angle;
extern float as5600_min_angle;
extern float as5600_max_angle;

// methods

extern float angleOf(int units);
extern void startAS5600();
extern void initAS5600(int zero_angle);

extern int getAS5600Raw();
extern float getAS560AverageAngle();
extern int getAS5600Angle(int zero_angle, int *cur);
	// return -1 on error, 1 if significant change, and 0 if not
extern bool getAS5600ZeroCrossing();
	// should be called right after getAS5600Angle()