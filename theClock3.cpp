#include "theClock3.h"
#include <myIOTLog.h>
#include <Adafruit_NeoPixel.h>
#include <AS5600.h>
#include <Wire.h>


// USES A TASK FOR RUNNING THE CLOCK
//
// there is too much critical timing in the clock run() method to allow it
// allow it to run in loop() on the same core as the webservers, etc.
// When just run from the default core(1), along with the Webserver, it
// regularly misses crossings, etc, esp when the myIOT WebSocket goes on and off.
//
// So I made it a task on the unused core 0. However, tasks are limited to 10ms
// time slices, and so it did not work at first.
//
// I was able to get it working. First I changed the call to vTaskDelay(0)
// as I read that would yield without delay.  No joy.  I set the priority
// of the task down to 1, no joy.  Finally I disabled the Watch Dog Timer,
// (see code below) and that seems to work.




//----------------------------
// AS5600 Sensor
//----------------------------
// With the AS5600, the return values are 0..4095 where 4096 is 360 degrees (i.e. 0).
// so each bit is 0.087890625 degrees.  For direction determination, we require a
// threshold change > AS4500_THRESHOLD.

#define AS4500_THRESHOLD    4

static int as5600_cur = 0;			// bits, not degrees
static int as5600_side = 0;			// trying zero crossing
static int as5600_direction = 0;	// on detection of THRESHOLD AS4500_THRESHOLD

static int as5600_min = 0;			// min and max are assigned on direction changes
static int as5600_max = 0;
static int as5600_temp_min = 10240;	// and the temps are reset to zero for next go round
static int as5600_temp_max = -10240;

// angles are calculated for convenience

static float as5600_cur_angle = 0;
static float as5600_min_angle = 0;
static float as5600_max_angle = 0;

AS5600 as5600;   //  uses default Wire


static float angle(int units)
{
	float retval = units * 3600;
	retval /= 4096;
	return floor(retval + 0.5) / 10;
}

//----------------------------
// motor
//----------------------------

#define PWM_FREQUENCY	5000
#define PWM_RESOLUTION	8

int motor_state = 0;

void motor(int state, int power)
	// note output to channel B for backward compatibility with V1 circuit board
{
	motor_state = state;
	int use_power = state ? power : 0;
	ledcWrite(0, use_power);
	digitalWrite(PIN_IN1,state == 1  ? 1 : 0);
	digitalWrite(PIN_IN2,state == -1 ? 1 : 0);
	// LOGD("motor(%d,%d)",state,power);
}


//----------------------------
// pixel(s)
//----------------------------

static Adafruit_NeoPixel pixels(NUM_PIXELS,PIN_LEDS);

static bool show_pixels = false;

void setPixel(int num, uint32_t color)
{
	pixels.setPixelColor(num,color);
	show_pixels = true;
}


//--------------------------------------------
// vars
//--------------------------------------------
// The algorithm works by using a PID controller to target the ANGLE
// by using "pushes" after crossing zero, varying the power delivered
// from POWER_LOW to POWER_HIGH.  Tbis depends on many parameters including
// the ZERO_ANGLE of the pendulum, the DEAD_ZONE about zero where we wait to
// initiate the pulse, the DUR_LEFT and DUR_RIGHT which determine the durations
// of the pulses and the PID variables, all of which have been heuristically
// determined and represented in the defaults.  We actually control for the
// average of the left and right maximum swings, since they are different
//
// The clock is tuned to run a little bit slow, on the order of an
// average of 10ms or so per second and we keep track of the current
// 'cur_cycle' (one full second) in milliseconds, and a cumulative error
// in 'millis_error'.
//
// We can speed it up by "pulling" the pendulum towards the center.
// When pulling the pulse starts just as soon as the push ends, and continues
// until we are within 2 deadzones of the center again.  The 'pull' has a
// separate constant power setting which has also been heuristically tuned.
//
// The application of the accumulated error is simple .. if the clock is
// running slow, we start pulling. If it is running fast we stop pulling.


static bool clock_started = 0;		// the clock is running

static bool push_motor = 0;			// push the pendulum after it leaves deadzone (determined at zero crossing)
static bool pull_motor = 0;			// pull motor after pushing (also determined at zero crossing)
static bool pulling = 0;			// we are in a pull at this time


// PID

static float total_error = 0;		// accumluated degrees of error
static float prev_p = 0;			// the previous error (for "D" determination)
static int pid_power = 0;			// power adjusted by pid algorithm


static int32_t cur_cycle = 0;		// millis in this 'cycle' (forward zero crossing)
static int32_t last_cycle = 0;		// millis at previous forward zero crossing
static int32_t millis_error = 0;



// Statistics

static uint32_t num_beats   = 0;
static int num_restarts     = 0;
static int num_stalls_left  = 0;
static int num_stalls_right = 0;
static int32_t low_error    = 0;
static int32_t high_error   = 0;
static int32_t low_dur      = 0;
static int32_t high_dur     = 0;
static int min_power_used 	= 0;
static int max_power_used   = 0;


// Buttons and pixel flashing

#define FUNCTION_NONE  			0
#define FUNCTION_STOPPED		1	// white		state
#define FUNCTION_STOPPING		2	// white		short press
#define FUNCTION_STARTING       3	// green		short press
#define FUNCTION_WIFI_ON        4	// blue			long press (over 2 seconds)
#define FUNCTION_WIFI_OFF       5	// magenta		long press (over 2 seconds)
#define FUNCTION_RESET			6	// red			super long press (over 8 seconds)

static int flash_fxn = FUNCTION_NONE;
static bool flash_on = 0;
static int flash_count = 0;
static uint32_t flash_dur = 0;
static uint32_t flash_time = 0;

static uint32_t button_chk = 0;
static uint32_t button_down = 0;



// virtual
void theClock::setup()	// override
{
	LOGU("theClock::setup() started");

	pixels.clear();
	for (int i=NUM_PIXELS-1; i>=0; i--)
	{
		pixels.setPixelColor(i,MY_LED_CYAN);
		pixels.show();
		delay(500);
	}
	pixels.clear();
	pixels.show();
	delay(500);

	pinMode(PIN_BUTTON1,INPUT_PULLUP);
	pinMode(PIN_BUTTON2,INPUT_PULLUP);

	ledcSetup(0, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(PIN_EN, 0);
	ledcWrite(0,0);
	pinMode(PIN_IN1,OUTPUT);
	pinMode(PIN_IN2,OUTPUT);
	digitalWrite(PIN_IN1,0);
	digitalWrite(PIN_IN2,0);

	// call myIOTDevice::setup()

	pixels.setPixelColor(PIXEL_MAIN,MY_LED_BLUE);
	pixels.show();
	myIOTDevice::setup();
	pixels.setPixelColor(PIXEL_MAIN,MY_LED_CYAN);
	pixels.show();
	delay(500);

	//--------------------------------------------------
	// INIT THE AS5600 and set zero angle if not yet set
	//--------------------------------------------------

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
				pixels.setPixelColor(PIXEL_MAIN,i&1?MY_LED_RED:MY_LED_BLACK);
				pixels.show();
				delay(300);
			}
		}
	}
	LOGD("AS5600 connected=%d",connected);

	// can't use values until after myIOTDevice::setup() has been called
	// May need to clear values from NVS (Preferences) if types change!
	// with call to clearValueById(ID_ZERO_ANGLE);
	// clearValueById(ID_ZERO_ANGLE);
	// clearValueById(ID_ZERO_ANGLE_F);
	// setInt(ID_ZERO_ANGLE,0);

	if (_zero_angle == 0)
		setZeroAngle();

	//------------------------------------------------
	// Start the clock task and away we go ...
	//------------------------------------------------

	pixels.setPixelColor(PIXEL_MAIN,MY_LED_GREEN);
	pixels.show();
	delay(500);

	LOGI("starting clockTask");
	xTaskCreatePinnedToCore(clockTask,
		"clockTask",
		8192,           // task stack
		NULL,           // param
		10,  	        // note that the priority is higher than one
		NULL,           // returned task handle
		ESP32_CORE_OTHER);

	if (_clock_running)
		startClock();
	else
		flash_fxn = FUNCTION_STOPPED;

	LOGU("theClock::setup() finished");
	pixels.clear();
	pixels.show();

}	// theClock::setup()



void theClock::init()
{
	clock_started = 0;

	total_error = 0;
	prev_p = 0;

	millis_error = 0;
	cur_cycle = 0;
	last_cycle = 0;

	as5600_cur = 0;
	as5600_side = 0;
	as5600_direction = 0;
	as5600_min = 0;
	as5600_max = 0;
	as5600_temp_min = 10240;
	as5600_temp_max = -10240;
	as5600_cur_angle = 0;
	as5600_min_angle = 0;
	as5600_max_angle = 0;

    the_clock->_cur_time = 0;
	the_clock->_time_start = 0;
	the_clock->_time_running = "";
	the_clock->_stat_beats = 0;
	the_clock->_stat_num_push = 0;
	the_clock->_stat_num_pull = 0;
	the_clock->_stat_pull_ratio = 0.5;
	the_clock->_stat_min_power = 255;
	the_clock->_stat_max_power = 0;
	the_clock->_stat_min_left = -360;
	the_clock->_stat_max_left = 360;
	the_clock->_stat_min_right = -360;
	the_clock->_stat_max_right = 360;
	the_clock->_stat_min_cycle = 32767;
	the_clock->_stat_max_cycle = -32767;
	the_clock->_stat_min_error = 32767;
	the_clock->_stat_max_error = -32767;

}


void theClock::clearStats()
{
	LOGU("STATISTICS CLEARED");

	total_error = 0;
	prev_p = 0;

	millis_error = 0;
	cur_cycle = 0;
	last_cycle = 0;

    the_clock->_cur_time = 0;
	the_clock->_time_start = 0;
	the_clock->_time_running = "";
	the_clock->_stat_beats = 0;
	the_clock->_stat_num_push = 0;
	the_clock->_stat_num_pull = 0;
	the_clock->_stat_pull_ratio = 0.5;
	the_clock->_stat_min_power = 255;
	the_clock->_stat_max_power = 0;
	the_clock->_stat_min_left = -360;
	the_clock->_stat_max_left = 360;
	the_clock->_stat_min_right = 360;
	the_clock->_stat_max_right = -360;
	the_clock->_stat_min_cycle = 32767;
	the_clock->_stat_max_cycle = -32767;
	the_clock->_stat_min_error = 32767;
	the_clock->_stat_max_error = -32767;

	// resetting the statistics also resets the
	// start time for display.

	if (clock_started)
		the_clock->setTime(ID_TIME_START,time(NULL));

}



void theClock::startClock()
{
	LOGU("startClock()");

	init();
	clock_started = 1;
	setPixel(PIXEL_MAIN, MY_LED_WHITE);
	pixels.show();
	motor(-1,_power_start);
	delay(_dur_start);
	motor(0,0);
	setPixel(PIXEL_MAIN, MY_LED_BLACK);
	pixels.show();

	the_clock->setTime(ID_TIME_START,time(NULL));

	pid_power = _power_high;
	flash_fxn = FUNCTION_NONE;
}


void theClock::stopClock()
{
	init();
	motor(0,0);
	flash_fxn = FUNCTION_STOPPED;
	flash_count = 0;
	flash_time = millis();
	LOGU("stopClock()");
}



void theClock::onClockRunningChanged(const myIOTValue *desc, bool val)
{
	LOGU("onClockRunningChanged(%d)",val);
	if (val)
		startClock();
	else
		stopClock();
}


void theClock::onPIDModeChanged(const myIOTValue *desc, bool val)
{
	LOGU("onPIDModeChanged(%d)",val);
	pid_power = _power_high;
}


void theClock::onPlotValuesChanged(const myIOTValue *desc, uint32_t val)
{
	if (val == 1)
		Serial.print("as5600,side,angle,min,max,err,motor");
}


void theClock::setZeroAngle()
{
	LOGU("Setting AS5600 zero angle ...");
	int zero = as5600.readAngle();
	float zero_f =  angle(zero);
	LOGU("AS5600 zero angle = %d, %03f", zero,zero_f);
	the_clock->setInt(ID_ZERO_ANGLE,zero);
	the_clock->setFloat(ID_ZERO_ANGLE_F,zero_f);
}



void theClock::onTestMotor(const myIOTValue *desc, int val)
{
	LOGU("onTestMotor %d",val);
	motor(val,_power_low);
}




//===================================================================
// run()
//===================================================================


void theClock::run()
	// This is called every 3 ms ...
	// The basic sensor code can run in about 1ms
{
	uint32_t now = millis();

	static uint32_t motor_start = 0;
	static uint32_t motor_dur = 0;

	//-------------------------------------------------
	// AS5600
	//-------------------------------------------------
	// Read, but don't necessariy use the as5600 angle

	int cur = as5600.readAngle() - _zero_angle;
	as5600_cur_angle = angle(cur);

	// min and max are intrinsicly debounced
	// set temp min and max per cycle

	if (cur < as5600_temp_min)
		as5600_temp_min = cur;
	if (cur > as5600_temp_max)
		as5600_temp_max = cur;

	// detect direction change or zero crossing
	// only if the posiion has changed significantly

	int dif = cur - as5600_cur;
	if (abs(dif) > AS4500_THRESHOLD)
	{
		as5600_cur = cur;

		// detect zero crossing

		if ((cur < 0 && as5600_side >= 0) ||
			(cur > 0 && as5600_side <= 0))
		{
			as5600_side = cur < 0 ? -1 : 1;

			// get full cycle time
			// and add it to the accumulated error if running
			// and pull the motor if in pid mode

			if (as5600_side > 0)
			{
				if (last_cycle)
					cur_cycle = now - last_cycle;
				last_cycle = now;

				if (clock_started && cur_cycle)
				{
					int err = cur_cycle;
					err -= 1000;
					millis_error += err;

					_stat_beats++;
					if (cur_cycle < _stat_min_cycle)
						_stat_min_cycle = cur_cycle;
					if (cur_cycle > _stat_max_cycle)
						_stat_max_cycle = cur_cycle;
					if (millis_error < _stat_min_error)
						_stat_min_error = millis_error;
					if (millis_error > _stat_max_error)
						_stat_max_error = millis_error;

					if (_pid_mode)
					{
						if (millis_error > 0)
							pull_motor = true;
						else
							pull_motor = false;
					}
				}
			}

			// defer a push if clock started

			if (clock_started)
				push_motor = true;
		}

		// detect direction change
		// if direction changed, assign min or max and clear temp variable

		int dir = as5600_direction;
		if (dif < 0)
			dir = -1;
		else if (dif > 0)
			dir = 1;
		if (as5600_direction != dir)
		{
			if (dir > 0)
			{
				as5600_min = as5600_temp_min;
				as5600_temp_min = 10240;
				as5600_min_angle = angle(as5600_min);

				if (as5600_min_angle < _stat_max_left)
					_stat_max_left = as5600_min_angle;
				if (as5600_min_angle > _stat_min_left)
					_stat_min_left = as5600_min_angle;
			}
			else
			{
				as5600_max = as5600_temp_max;
				as5600_temp_max = -10240;
				as5600_max_angle = angle(as5600_max);

				if (as5600_max_angle > _stat_max_right)
					_stat_max_right = as5600_max_angle;
				if (as5600_max_angle < _stat_min_right)
					_stat_min_right = as5600_max_angle;
			}

			as5600_direction = dir;
		}
	}


	//-------------------
	// MOTOR
	//-------------------
	// stop any pull if within 2 deadzones of zero

	if (pulling && abs(as5600_cur_angle) < 2 * _dead_zone)
	{
		pulling = false;
		motor(0,0);
	}

	// push if out of dead zone

	if (push_motor && abs(as5600_cur_angle) > _dead_zone)
	{
		push_motor = false;

		int use_power = _power_low;
		float avg_angle = (abs(as5600_min_angle) + abs(as5600_max_angle) ) / 2;

		if (_pid_mode)
		{
			float this_p = _target_angle - avg_angle;
			total_error += this_p;
			float this_i = total_error;
			float this_d = prev_p - this_p;;
			prev_p = this_p;

			this_p = this_p / 100;
			this_i = this_i / 100;
			this_d = this_d / 100;

			int old_power = pid_power;
			float factor = 1 + (_pid_P * this_p) + (_pid_I * this_i) + (_pid_D * this_d);
			float new_power = pid_power * factor;
			if (new_power > _power_max) new_power = _power_max;
			if (new_power  < _power_low) new_power = _power_low;
			pid_power = new_power;

			if (pid_power < min_power_used)
				min_power_used = pid_power;
			if (pid_power > max_power_used)
				max_power_used = pid_power;

			motor_dur = _dur_right;
			use_power = pid_power;
		}

		if (!_plot_values)
			LOGD("push(%-2d) pull(%d) %-4d %3.3f/%3.3f=%3.3f  target=%3.3f  accum=%3.3f  power=%d  err=%d",			// p=%3.3f i=%3.3f d=%3.3f
				 as5600_direction,
				 pull_motor,
				 cur_cycle,
				 as5600_min_angle,
				 as5600_max_angle,
				 avg_angle,
				 _target_angle,
				 // this_p,
				 // this_i,
				 // this_d,
				 total_error,
				 use_power,
				 millis_error);

		_stat_num_push++;
		if (use_power > _stat_max_power)
			_stat_max_power = use_power;
		if (use_power < _stat_min_power)
			_stat_min_power = use_power;

		motor_start = now;
		motor_dur = as5600_direction > 0 ? _dur_right : _dur_left;
		motor(-1,use_power);
	}


	// stop pushing and possibly start pulling

	if (motor_start && now - motor_start > motor_dur)
	{
		motor_start = 0;
		motor_dur = 0;
		motor(0,0);

		if (clock_started && pull_motor)
		{
			pulling = true;
			_stat_num_pull++;
			motor(1,_power_pull);
		}
	}


	// plotting

	if (_plot_values == 1)
	{
		float use_angle = as5600_cur_angle * 50;		// 20 degrees == 1000 in output
		float use_min = as5600_min_angle * 50;
		float use_max = as5600_max_angle * 50;

		int ang = use_angle;
		int min = use_min;
		int max = use_max;

		Serial.print(as5600_direction * 200);
		Serial.print(",");
		Serial.print(as5600_side * 250);
		Serial.print(",");
		Serial.print(ang);
		Serial.print(",");
		Serial.print(min);
		Serial.print(",");
		Serial.print(max);
		Serial.print(",");

		int err = cur_cycle;
		err -= 1000;
		Serial.print(err * 40);
		Serial.print(",");

		Serial.print(motor_state * 400);
		Serial.println(",1000,-1000");
	}


#if 0

	//----------------------------------------------------------
	// CYCLE
	//----------------------------------------------------------
	// The full cycle is when the pendulum goes from positioni -1 to 1 and should take 1000 ms.
	// A half cycle is determined by the transition from position -1 to 1 in either direction.

	static int last_position = 0;
	static uint32_t motor_start = 0;
	static uint32_t motor_dur = 0;

	// if there have been no changes for 3 seconds
	// restart the clock

	uint32_t now = millis();
	if (clock_started && (now - last_change > 3000))
	{
		num_restarts++;
		LOGE("CLOCK STOPPED! - restarting!!");
		startClock();
		last_change = now;
	}

	if (last_position != position) //  && (now - last_change > 20))
	{
		int use_power = 0;
		last_change = now;

		// moving left to right

		if (last_position == -1 && position == 1)
		{
			num_beats++;

			if (clock_started && max_right < 3)
			{
				LOGE("STALL_RIGHT",0);
				num_stalls_right++;
			}

			if (cycle_start)	// not the first time through
			{
				cycle_duration = now - cycle_start;
				total_error += cycle_duration - 1000;

				if (total_error > high_error)
					high_error = total_error;
				if (total_error < low_error)
					low_error = total_error;
				if (cycle_duration < low_dur)
					low_dur = cycle_duration;
				if (cycle_duration > high_dur)
					high_dur = cycle_duration;

				if (_pid_mode && clock_started)
				{
					float this_p =  cycle_duration;		// this error
					this_p = this_p - 1000;
					float this_i = total_error;			// total error at this time
					float this_d = 0;

					if (prev_p)
					{
						this_d = this_p;
						this_d -= prev_p;
					}
					prev_p = this_p;

					this_p = this_p / 1000;
					this_i = this_i / 1000;
					this_d = this_d / 1000;

					int old_power = pid_power;
					float factor = 1 + (_pid_P * this_p) + (_pid_I * this_i) + (_pid_D * this_d);
					float new_power = pid_power * factor;
					if (new_power > _power_max) new_power = _power_max;
					if (new_power  < _power_low) new_power = _power_low;
					pid_power = new_power;

					if (pid_power < min_power_used)
						min_power_used = pid_power;
					if (pid_power > max_power_used)
						max_power_used = pid_power;

					motor_dur = _dur_right;
					use_power = pid_power;
				}

				// static mode right

				else if (clock_started)
				{
					motor_dur = _dur_right;
					if (max_right == 3)
						use_power = _power_low;
					else
						use_power = _power_high;
				}

				if (clock_started && !button_down && !flash_fxn)
				{
					setPixel(PIXEL_MAIN,
						total_error > 2000 ? MY_LED_BLUE :
						total_error < -2000 ? MY_LED_RED :
						total_error > 200 ? MY_LED_BLUECYAN :
						total_error < -200 ? MY_LED_ORANGE :
						cycle_duration < 995 ?  MY_LED_YELLOW :
						cycle_duration > 1005 ? MY_LED_CYAN :
						MY_LED_GREEN);
				}

				if (!_plot_values)
					LOGI("RIGHT(%d) dur(%d) power(%d) cycle(%d) error(%d)",max_right,motor_dur,use_power,cycle_duration,total_error);

				max_right = 0;

			}
			cycle_start = now;
		}

		// moving right to left

		if (clock_started && TIMposition == 1 && position == -1)
		{
			if (max_left > -3)
			{
				LOGE("STALL_LEFT",0);
				num_stalls_left++;
			}

			if (_dur_left)
			{
				motor_dur = _dur_left;
				if (_pid_mode)
					use_power = pid_power;
				else if (max_right == 3)
					use_power = _power_low;
				else
					use_power = _power_high;

				if (!_plot_values)
					LOGI(" LEFT(%d) dur(%d) power(%d)",max_left,motor_dur,use_power);
			}

			max_left = 0;
		}

		// IMPULSE

		if (use_power)
		{
			motor_start = now;
			motor(-1,use_power);
		}

		last_position = position;
	}


	//------------------------------------------------
	// stop the impulse
	//------------------------------------------------

	if (motor_start && now - motor_start > motor_dur)
	{
		motor_start = 0;
		motor_dur = 0;
		motor(0,0);
	}

	if (show_pixels)
		pixels.show();

	//------------------------
	// plot values
	//------------------------

	if (_plot_values == 1)
	{
		Serial.print(hall_value);
		Serial.print(",");

		// Serial.print(cycle_duration);

		int err = cycle_duration - 1000;
		Serial.print(1000 + err * 20);
		Serial.print(",1000,-1400,1400,");
		Serial.println(position * 200);
	}

#endif	// 0

}	// theClock::run()



//===================================================================
// loop
//===================================================================




// virtual
void theClock::loop()	// override
{
	myIOTDevice::loop();

#if 0

	//-----------------------------
	// handle button
	//-----------------------------

	uint32_t now = millis();
	if (now - button_chk > 33)	// 30 times per second
	{
		button_chk = now;
		static int last_press = 0;
		bool val = !digitalRead(PIN_BUTTON1);
		if (button_down)
		{
			uint32_t dur = now - button_down;
			int press =
				dur > 8000 ? 3 :			// long press
				dur > 2000 ? 2 : 1;			// medium, short press
			int fxn =
				press == 3 ? FUNCTION_RESET :
				press == 2 ? getBool(ID_DEVICE_WIFI) ?
					FUNCTION_WIFI_OFF :
					FUNCTION_WIFI_ON :
				getBool(ID_RUNNING) ?
					FUNCTION_STOPPING :
					FUNCTION_STARTING;

			if (!val)
			{
				LOGD("button_up dur(%d) press(%d) fxn(%d)",dur,press,fxn);

				flash_fxn = fxn;
				button_down = 0;
				setPixel(PIXEL_MAIN,MY_LED_BLACK);
				pixels.show();
			}
			else if (last_press != press)
			{
				LOGD("button_down dur(%d) press(%d) fxn(%d)",dur,press,fxn);

				last_press = press;
				setPixel(PIXEL_MAIN,
					fxn == FUNCTION_RESET ? MY_LED_RED :
					fxn == FUNCTION_WIFI_OFF ? MY_LED_MAGENTA :
					fxn == FUNCTION_WIFI_ON ? MY_LED_BLUE :
					fxn == FUNCTION_STARTING ? MY_LED_GREEN :
					MY_LED_WHITE);
				pixels.show();
			}
		}
		else if (val && !button_down)
		{
			LOGD("button_down",0);
			button_down = now;
			flash_fxn = 0;
			flash_count = 0;
			flash_time = 0;
			last_press = 0;
		}
	}

	//------------------------
	// FLASH PIXELS
	//------------------------

	if (flash_fxn && now - flash_time > flash_dur)
	{
		uint32_t color;
		if (flash_on)
		{
			flash_on = 0;
			flash_dur = 400;
			flash_count++;
			color = MY_LED_BLACK;
		}
		else
		{
			flash_on = 1;
			flash_dur = 100;
			color =
				flash_fxn == FUNCTION_STARTING ? MY_LED_GREEN :
				flash_fxn == FUNCTION_WIFI_ON ? MY_LED_BLUE :
				flash_fxn == FUNCTION_WIFI_OFF ? MY_LED_MAGENTA :
				flash_fxn == FUNCTION_RESET ? MY_LED_RED :
				MY_LED_WHITE;	// STOPPED or STOPPING
				MY_LED_YELLOW;	// unknown function

		}
		flash_time = now;
		setPixel(PIXEL_MAIN,color);
		pixels.show();
		if (flash_count == 5)
		{
			int fxn = flash_fxn;
			flash_fxn = FUNCTION_NONE;

			if (fxn == FUNCTION_RESET)
				the_clock->factoryReset();
			else if (fxn == FUNCTION_WIFI_OFF)
				the_clock->setBool(ID_DEVICE_WIFI,0);
			else if (fxn == FUNCTION_WIFI_ON)
				the_clock->setBool(ID_DEVICE_WIFI,1);
			else if (fxn == FUNCTION_STARTING)
				the_clock->setBool(ID_RUNNING,1);
			else if (fxn == FUNCTION_STOPPING && clock_started)
			{
				the_clock->setBool(ID_RUNNING,0);
				flash_fxn = FUNCTION_STOPPED;
			}
			else if (fxn == FUNCTION_STOPPED)
				flash_fxn = FUNCTION_STOPPED;

			flash_dur = 5000;
			flash_count = 0;
		}
	}


	// Testing NTP
	// I am using the "pause" mode of _plot_values for this.
	// Typically I turn off the clock.
	// I am not sure if the ESP32 intercepts the UDP calls, but otherwise,
	//     there should be some drift between them over time.


	static uint32_t last_debug = 0;
	if (_plot_values == 2 && now > last_debug + 5000 )
	{
		last_debug = now;

		uint32_t sys_time = time(NULL);
		uint32_t ntp_time = getNtpTime();
		LOGD("---> sys_time=%d   ntp_time=%d",sys_time, ntp_time);
	}

#endif	// 0


	//---------------------------------
	// show stats every so many beats
	//---------------------------------

	static uint32_t last_num_beats;

	if (_stat_interval &&
		_stat_beats % _stat_interval == 0 &&
		!_plot_values &&
		clock_started &&
		_stat_beats != last_num_beats)
	{
		last_num_beats = _stat_beats;

		setTime(ID_CUR_TIME,time(NULL));

		uint32_t secs = _cur_time - _time_start;
		uint32_t mins = secs / 60;
		uint32_t hours = mins / 60;
		uint32_t save_secs = secs;
		secs = secs - mins * 60;
		mins = mins - hours * 60;

		static char buf[80];
		sprintf(buf,"%02d:%02d:%02d  == %d secs",hours,mins,secs,save_secs);
		setString(ID_TIME_RUNNING,buf);

		setInt	(ID_STAT_BEATS     	,_stat_beats);
		setInt	(ID_STAT_NUM_PUSH  	,_stat_num_push);
		setInt	(ID_STAT_NUM_PULL  	,_stat_num_pull);
		setInt	(ID_STAT_MIN_POWER 	,_stat_min_power);
		setInt	(ID_STAT_MAX_POWER 	,_stat_max_power);
		setFloat(ID_STAT_MIN_LEFT  	,_stat_min_left);
		setFloat(ID_STAT_MAX_LEFT  	,_stat_max_left);
		setFloat(ID_STAT_MIN_RIGHT 	,_stat_min_right);
		setFloat(ID_STAT_MAX_RIGHT 	,_stat_max_right);
		setInt	(ID_STAT_MIN_CYCLE 	,_stat_min_cycle);
		setInt	(ID_STAT_MAX_CYCLE 	,_stat_max_cycle);
		setInt	(ID_STAT_MIN_ERROR 	,_stat_min_error);
		setInt	(ID_STAT_MAX_ERROR 	,_stat_max_error);

		if (_stat_num_push || _stat_num_pull)
			setFloat(ID_STAT_PULL_RATIO	,((float)(_stat_num_pull))/((float)(_stat_num_pull +_stat_num_push)));

	}



}	// theClock::loop()



//------------------------------------------------------
// clockTask
//------------------------------------------------------
// disable the watchdog timer.
// the first param is number of seconds per check.
// at 30, I would get a WDT message, but the program
// kept running (as opposed to how it crashes normally
// on a WDT issue). Then I increased the delay to
// 0x0FFFFFFF (about 27 days).


#include <esp_task_wdt.h>

void theClock::clockTask(void *param)
{
	esp_task_wdt_init(0x0FFFFFFF, false);
    delay(1200);
    LOGI("starting clockTask loop on core(%d)",xPortGetCoreID());
    delay(1200);
    while (1)
    {
        vTaskDelay(0);		// 10 / portTICK_PERIOD_MS);
		uint32_t now = millis();
		static uint32_t last_sense = 0;

		// we have to allow time for other tasks to run
		// so we limit thi to 333 times a second, even though
		// the basic sensor reads only take about 1ms

		if (now - last_sense > 2)		// 3 ms works
		{
			last_sense = now;
			the_clock->run();
		}
	}
}
