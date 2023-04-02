#include "theClock3.h"
#include <myIOTLog.h>
#include <Adafruit_NeoPixel.h>
#include <AS5600.h>
#include <Wire.h>

#define MAX_INT		32767
#define MIN_INT		-32767

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
#define RESTART_MILLIS   5000
	// after 5 seconds of no pendulum movement, we restart the clock

#define MIN_RUNNING_ANGLE  4.0
#define RUNNING_ERROR_THRESHOLD  2.0
	// The clock is considered 'running' after it has been 'started' and gotten
	// at least the minimum running angle and given total millis_error

#define MILLIS_ERROR_THRESHOLD		40
	// Once it IS running, we only switch from pushing to pulling if the
	// clock gets this many millis fast or slow (total millis_error)

// BASIC STATE

static bool clock_started = 0;		// the clock has been started
static bool clock_running = 0;		// the clock has achieved the target millis_error and so is considered 'running'
static uint32_t last_change = 0;	// millis of last noticible pendulum movement

static int32_t 	cur_cycle = 0;		// millis in this 'cycle' (forward zero crossing)
static int32_t 	last_cycle = 0;		// millis at previous forward zero crossing
static uint32_t num_beats = 0;		// number of beats (while running)
static int32_t millis_error = 0;	// IMPORTANT cumulative number of millis total error

// PID

static float total_error = 0;		// accumluated degrees of error
static float prev_p = 0;			// the previous error (for "D" determination)
static int pid_power = 0;			// power adjusted by pid algorithm

// CONTROL

static int  push_or_pull  = -1;		// -1==push, 1=pull.  We start out pushing in all cases until the clock is running
	// in order to smooth out frequency changes and prevent wild swings,
	// once the motor is running, we only change the state when the
	// error threshold has been exceeded

static bool push_motor = 0;			// push the pendulum next time after it leaves deadzone (determined at zero crossing)
static bool pull_motor = 0;			// pull motor immediately (could be local variable)
static bool pulling = 0;			// we are in a pull at this time

static uint32_t last_beat = 0xffffffff;
static uint32_t last_sync = 0;
static uint32_t last_stats = 0;
static uint32_t last_ntp = 0;


// NTP correction
// We merely correct the ESP32 clock to NTP time occasionally,
// and note the changes for debugging

static int32_t  last_ntp_diff = 0;
static uint32_t num_ntp_checks = 0;
static uint32_t num_ntp_fails = 0;
static uint32_t num_ntp_corrections = 0;
static uint32_t total_abs_ntp_corrections = 0;
static int32_t  total_ntp_corrections = 0;

// Error correction

static bool     in_sync = 0;
static int 		sync_sign = 0;
static int32_t  sync_millis = 0;
static uint32_t num_sync_checks = 0;
static uint32_t num_sync_changes = 0;
static uint32_t total_sync_changes = 0;
static uint32_t total_abs_sync_changes = 0;



// Statistics

static bool update_stats = false;

static uint32_t  stat_num_restarts;
static uint32_t  stat_num_push;
static uint32_t  stat_num_pull;
static float	 stat_pull_ratio;
static uint32_t  stat_min_power;
static uint32_t  stat_max_power;
static float	 stat_min_left;
static float	 stat_max_left;
static float	 stat_min_right;
static float	 stat_max_right;
static int32_t	 stat_min_cycle;
static int32_t	 stat_max_cycle;
static int32_t   stat_min_error;
static int32_t   stat_max_error;


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
	pixels.setBrightness(64);

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

	pixels.setPixelColor(PIXEL_MAIN,MY_LED_BLUE);
	pixels.show();
	delay(500);

	//--------------------------------------------------
	// INIT THE AS5600 before myIOTDevice::setup()
	//--------------------------------------------------
	// cuz it's better done when WIFI and all that stuff
	// has not yet been started ...

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


	//------------------------------
	// call myIOTDevice::setup()
	//------------------------------

	pixels.setPixelColor(PIXEL_MAIN,MY_LED_CYAN);
	pixels.show();
	myIOTDevice::setup();

	//-------------------------------
	// set zero angle if not yet set
	//-------------------------------
	// can't use values until after myIOTDevice::setup() has been called
	// May need to clear values from NVS (Preferences) if types change!
	// with call to clearValueById(ID_ZERO_ANGLE);
	// clearValueById(ID_ZERO_ANGLE);
	// clearValueById(ID_ZERO_ANGLE_F);
	// setInt(ID_ZERO_ANGLE,0);

	pixels.setPixelColor(PIXEL_MAIN,MY_LED_GREEN);
	pixels.show();
	if (_zero_angle == 0)
		setZeroAngle();
	delay(500);

	//------------------------------------------------
	// Start the clock task and away we go ...
	//------------------------------------------------

	LOGI("starting clockTask");
	xTaskCreatePinnedToCore(clockTask,
		"clockTask",
		8192,           // task stack
		NULL,           // param
		10,  	        // note that the priority is higher than one
		NULL,           // returned task handle
		ESP32_CORE_OTHER);

	if (_clock_running)
	{
		delay(1200);	// wait for things to settle, including WiFi, etc
		startClock();
	}
	else
		flash_fxn = FUNCTION_STOPPED;

	LOGU("theClock::setup() finished");
	pixels.clear();
	pixels.show();

}	// theClock::setup()



void theClock::init(bool cold)
{
	if (cold)
	{
		clock_started = 0;
		clock_running = !_pid_mode;
			// the clock is 'running' as soon as it's started
			// in !pid_mode
		push_or_pull = _pull_mode == PULL_ONLY ? 1 : -1;
			// start the clock preferentially pushing
		LOGD("COLD_INIT  clock_running(%d) push_or_pull(%d)",clock_running,push_or_pull);

		// we don't clear the min angle and max angle
		// if clearing stats because that causes an
		// artificial pulse of high power

		as5600_cur = 0;
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

	push_motor = 0;
	pull_motor = 0;
	pulling = 0;

	total_error = 0;
	prev_p = 0;

	cur_cycle = 0;
	last_cycle = 0;
	num_beats = 0;
	millis_error = 0;

	last_beat = 0xffffffff;
	last_sync = 0;
	last_stats = 0;
	last_ntp = 0;

	last_ntp_diff = 0;
	num_ntp_checks = 0;
	num_ntp_fails = 0;
	num_ntp_corrections = 0;
	total_abs_ntp_corrections = 0;
	total_ntp_corrections = 0;

	in_sync = 0;
	sync_sign = 0;
	sync_millis = 0;
	num_sync_checks = 0;
	num_sync_changes = 0;
	total_sync_changes = 0;
	total_abs_sync_changes = 0;

    the_clock->_cur_time = 0;
	the_clock->_time_start = 0;

	update_stats = false;

	stat_num_restarts = 0;
	stat_num_push = 0;
	stat_num_pull = 0;
	stat_pull_ratio = 0.5;
	stat_min_power = 255;
	stat_max_power = 0;
	stat_min_left = MIN_INT;
	stat_max_left = MAX_INT;
	stat_min_right = MAX_INT;
	stat_max_right = MIN_INT;
	stat_min_cycle = MAX_INT;
	stat_max_cycle = MIN_INT;
	stat_min_error = MAX_INT;
	stat_max_error = MIN_INT;
}



void theClock::clearStats()
{
	LOGU("STATISTICS CLEARED");
	init(0);
	update_stats = true;
}



void theClock::startClock()
{
	last_change = millis();
	flash_fxn = FUNCTION_NONE;
	LOGU("startClock()");

	the_clock->setString(ID_STAT_MSG1,"");
	the_clock->setString(ID_STAT_MSG2,"");
	the_clock->setString(ID_STAT_MSG3,"");
	the_clock->setString(ID_STAT_MSG4,"");
	the_clock->setString(ID_STAT_MSG5,"");
	the_clock->setString(ID_STAT_MSG6,"");

	// there is some question whether this single context free
	// impulse is sufficient to guarantee that the clock starts.
	// Another idea would be to use maximum pulses until the pendulum
	// changes direction ...

	init(1);
	clock_started = 1;

	setPixel(PIXEL_MAIN, MY_LED_WHITE);
	pixels.show();

	// give it a bit of a jiggle then a push

	motor(1,_power_start);
	delay(10);
	motor(-1,_power_start);
	delay(_dur_start);
	motor(0,0);

	setPixel(PIXEL_MAIN, MY_LED_BLACK);
	pixels.show();

	pid_power = _power_pid;
	last_change = millis();
}


void theClock::stopClock()
{
	init(0);

	clock_started = 0;
	clock_running = 0;

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
	pid_power = _power_pid;
}


void theClock::onPlotValuesChanged(const myIOTValue *desc, uint32_t val)
{
	if (val)
		Serial.println("dir,side,angle,min,max,err,motor");
}


void theClock::setZeroAngle()
{
	LOGD("Setting AS5600 zero angle ...");
	int zero = as5600.readAngle();
	float zero_f =  angle(zero);
	LOGU("AS5600 zero angle=%d  %0.3f", zero,zero_f);
	the_clock->setInt(ID_ZERO_ANGLE,zero);
	the_clock->setFloat(ID_ZERO_ANGLE_F,zero_f);
}



#include <sys/time.h>

void theClock::onTestMotor(const myIOTValue *desc, int val)
{
	LOGU("onTestMotor %d",val);

	// millis_error += 1000 * val;
	// motor(val,_power_min);

	if (val == 0)
	{
		syncNTPTime();
	}
	else
	{
		int32_t a_time = time(NULL) + val;
		timeval e_time = {a_time, 0};
		settimeofday((const timeval*)&e_time, 0);
	}

}




//===================================================================
// run()
//===================================================================

int theClock::setPidPower(float avg_angle)
{
	float this_p = _target_angle - avg_angle;
	total_error += this_p;

	float this_i = total_error;
	float this_d = prev_p - this_p;;
	prev_p = this_p;

	this_p = this_p / 100;
	this_i = this_i / 100;
	this_d = this_d / 100;

	float factor = 1 + (_pid_P * this_p) + (_pid_I * this_i) + (_pid_D * this_d);
	float new_power = pid_power * factor;
	if (new_power > _power_max) new_power = _power_max;
	if (new_power  < _power_min) new_power = _power_min;
	pid_power = new_power;

	return pid_power;
}



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

	bool zero_cross = 0;
	bool at_extreme = 0;

	int dif = cur - as5600_cur;
	if (abs(dif) > AS4500_THRESHOLD)
	{
		as5600_cur = cur;
		last_change = now;

		// detect zero crossing

		if ((cur < 0 && as5600_side >= 0) ||
			(cur > 0 && as5600_side <= 0))
		{
			as5600_side = cur < 0 ? -1 : 1;
			zero_cross = true;

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

					// if sync_sign, we are in a sync and the err is
					// added to the sync_millisuntil it changes sign,
					// at which point the sync is "turned off" and the
					// remaining error falls through to the 'millis_error'

					if (sync_sign)
					{
						sync_millis += err;
						int new_sign =
							sync_millis > 0 ? 1 :
							sync_millis < 0 ? -1 : 0;

						if (new_sign != sync_sign)
						{
							LOGU("SYNC_DONE - remainder=%d",sync_millis);
							err = sync_millis;
							sync_sign = 0;
							sync_millis = 0;
							setPixel(1,MY_LED_BLACK);
							pixels.show();
						}

						// if the sync is not turned off, it ate all the error millis

						else
						{
							err = 0;
						}
					}

					// might be zero contribution while syncing ...

					millis_error += err;

					// yet another attempt to get beats to match seconds
					// we are either on the 0th beat (starting the clock)
					// or we start with the first beat one full cycle later ..

					if (!_time_start)
					{
						update_stats = true;
						_time_start = time(NULL);
					}
					else
					{
						if (!num_beats)
							update_stats = true;
						num_beats++;
					}

					if (cur_cycle < stat_min_cycle)
						stat_min_cycle = cur_cycle;
					if (cur_cycle > stat_max_cycle)
						stat_max_cycle = cur_cycle;
					if (millis_error < stat_min_error)
						stat_min_error = millis_error;
					if (millis_error > stat_max_error)
						stat_max_error = millis_error;
				}
			}
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
			as5600_direction = dir;
			at_extreme = true;

			if (dir > 0)
			{
				as5600_min = as5600_temp_min;
				as5600_temp_min = MAX_INT;
				as5600_min_angle = angle(as5600_min);

				if (as5600_min_angle < stat_max_left)
					stat_max_left = as5600_min_angle;
				if (as5600_min_angle > stat_min_left)
					stat_min_left = as5600_min_angle;
			}
			else
			{
				as5600_max = as5600_temp_max;
				as5600_temp_max = MIN_INT;
				as5600_max_angle = angle(as5600_max);

				if (as5600_max_angle > stat_max_right)
					stat_max_right = as5600_max_angle;
				if (as5600_max_angle < stat_min_right)
					stat_min_right = as5600_max_angle;
			}
		}
	}

	//------------------------------------------------
	// state machine
	//------------------------------------------------
	// !clock_running implies pid_mode ...
	// We determine that the clock is 'running' when it has exceeded some
	// critical minimum angle (which is less than the target angle) AND
	// the accumulated error has been brought down to a reasonable number.
	// It used to use _target_angle being exceeded on both sides.

	if (clock_started)
	{
		if (!clock_running &&
			as5600_max_angle >= MIN_RUNNING_ANGLE &&
			as5600_min_angle <= -MIN_RUNNING_ANGLE &&
			abs(total_error) < RUNNING_ERROR_THRESHOLD)
		{
			clock_running = 1;
			LOGU("Clock running!");
			clearStats();
		}

		if (zero_cross)
		{
			if (clock_running &&
				_pull_mode != PUSH_ONLY &&
				push_or_pull == -1 &&
				millis_error + sync_millis > MILLIS_ERROR_THRESHOLD)
			{
				LOGD("SET STATE 1=PULL");
				push_or_pull = 1;
			}

			if (push_or_pull == -1)
				push_motor = true;
		}

		if (at_extreme)
		{
			if (clock_running &&
				_pull_mode != PULL_ONLY &&
				push_or_pull == 1 &&
				millis_error + sync_millis < -MILLIS_ERROR_THRESHOLD)
			{
				LOGD("SET STATE -1=PUSH");
				push_or_pull = -1;
			}

			if (push_or_pull == 1)
				pull_motor = true;
		}
	}



	//-------------------
	// MOTOR
	//-------------------
	// We use the same pid controller for pushing and pulling
	// and set motor_dir as a flag to actually start the motor below.
	// We only start push if out of dead zone, wheras we start pull immediately

	int motor_dir = 0;
	int use_power = 0;
	float avg_angle = (abs(as5600_min_angle) + abs(as5600_max_angle) ) / 2;

	if (pull_motor)
	{
		pull_motor = false;
		motor_dir = 1;
		use_power = _pid_mode ? setPidPower(avg_angle) : _power_pull;
		stat_num_pull++;
		pulling = true;
	}
	if (push_motor && abs(as5600_cur_angle) > _dead_zone)
	{
		push_motor = false;
		motor_dir = -1;
		use_power = _pid_mode ? setPidPower(avg_angle) : _power_min;
		stat_num_push++;

		motor_start = now;
		motor_dur = as5600_direction > 0 ? _dur_right : _dur_left;
		motor(-1,use_power);
	}

	// DO THE ACTUAL PUSH OR PULL
	// and display state and update stats

	if (motor_dir)
	{
		if (_plot_values == PLOT_OFF)
		{
			LOGD("%-6s %s(%-2d) %-4d %3.3f/%3.3f=%3.3f  target=%3.3f  accum=%3.3f  power=%d  err=%d",			// p=%3.3f i=%3.3f d=%3.3f
				 clock_running ? "run" : "start",
				 motor_dir > 0 ? "pull" : "push",
				 as5600_direction,
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

			if (sync_sign)
				LOGI("SYNC sign(%d) millis(%d)",sync_sign,sync_millis);
		}

		if (use_power > stat_max_power)
			stat_max_power = use_power;
		if (use_power < stat_min_power)
			stat_min_power = use_power;

		motor(motor_dir,use_power);
	}

	// Stop the motor as needed
	// We stop pushes based on duration,
	// and pulls based on 2 * _dead_zone

	if (motor_start && now - motor_start > motor_dur)
	{
		motor_start = 0;
		motor_dur = 0;
		motor(0,0);
	}
	if (pulling && abs(as5600_cur_angle) < 2 * _dead_zone)
	{
		pulling = false;
		motor(0,0);
	}

	// Restart if necessary

	if (clock_started &&
		now - last_change > RESTART_MILLIS)
	{
		LOGE("RESTARTING CLOCK!!");
		stat_num_restarts++;
		startClock();
	}


	//----------------------
	// plotting
	//----------------------

	if (_plot_values == PLOT_ON)
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

		Serial.print(millis_error * 20);
		Serial.print(",");

		Serial.print(motor_state * 400);
		Serial.println(",1000,-1000");
	}

}	// theClock::run()



//===================================================================
// loop
//===================================================================




// virtual
void theClock::loop()	// override
{
	myIOTDevice::loop();

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

	// handle the 'starting' vs 'running' main pixel

	static bool pixel_on = 0;
	if (clock_started &&
		!clock_running &&
		!pixel_on)
	{
		pixel_on = 1;
		setPixel(PIXEL_MAIN,MY_LED_MAGENTA);
		pixels.show();
	}
	else if (pixel_on && clock_running)
	{
		pixel_on = 0;
		setPixel(PIXEL_MAIN,MY_LED_BLACK);
		pixels.show();
	}


	//--------------------------------------
	// static pixels
	//--------------------------------------

	static uint32_t last_pixel_right = MY_LED_BLACK;
	uint32_t pixel_right =
		!clock_started ? MY_LED_BLACK :
		!clock_running ? MY_LED_MAGENTA :
		millis_error > 2 * MILLIS_ERROR_THRESHOLD ? MY_LED_BLUE :
		millis_error < -2 * MILLIS_ERROR_THRESHOLD ? MY_LED_RED :
		MY_LED_GREEN;
	if (last_pixel_right != pixel_right)
	{
		last_pixel_right = pixel_right;
		setPixel(0,pixel_right);
		pixels.show();
	}

	//------------------------------------------
	// THINGS BASED ON THE BEAT CHANGING
	//------------------------------------------
	// Do error correction time check

	if (clock_started &&
		last_beat != num_beats)
	{
		last_beat = num_beats;
		static char buf[255];

		// Do error correction

		if (!sync_sign &&
			_time_start &&
			clock_running &&
			_sync_interval &&
			num_beats - last_sync >= _sync_interval)
		{
			last_sync = num_beats;
			num_sync_checks++;

			_cur_time = time(NULL);
			int32_t num_secs = _cur_time - _time_start;
			int32_t diff = num_secs - num_beats;

			if (diff > 0)
				sync_sign = 1;
			else if (diff < 0)
				sync_sign = -1;

			if (sync_sign)
			{
				sync_millis = diff  * 1000;
				num_sync_changes++;
				total_sync_changes += sync_millis;
				total_abs_sync_changes += abs(sync_millis);
				LOGU("SYNC CHANGE!!  BEATS(%d) SECS(%d) diff(%d) sign=%d  millis=%d",
						num_beats,
						num_secs,
						diff,
						sync_sign,
						sync_millis);
				setPixel(1,sync_sign < 0 ? MY_LED_RED : MY_LED_BLUE);
				pixels.show();
			}

			sprintf(buf,"SYNC%s sign(%d) millis(%d)  num(%d) chgs(%d) total(%d) abs(%d)%s",
				(num_sync_changes ? "<b>" : ""),
				sync_sign,
				sync_millis,
				num_sync_checks,
				num_sync_changes,
				total_sync_changes,
				total_abs_sync_changes,
				(num_sync_changes ? "</b>" : ""));
			setString(ID_STAT_MSG5,buf);
		}


		// Do statistics

		else if (_stat_interval &&
				 _plot_values == PLOT_OFF &&
				 last_stats != num_beats && (
				 update_stats ||
				 num_beats % _stat_interval == 0))
		{
			update_stats = false;
			last_stats = num_beats;

			setTime(ID_CUR_TIME,time(NULL));
			setTime(ID_TIME_START,_time_start);
			uint32_t full_secs = _time_start ? _cur_time - _time_start : 0;

			uint32_t secs = full_secs;
			uint32_t mins = secs / 60;
			uint32_t hours = mins / 60;
			secs = secs - mins * 60;
			mins = mins - hours * 60;

			sprintf(buf,"%s %02d:%02d:%02d  == %d SECS %d BEATS",
				(clock_running?"RUNNING":"STARTING"),
				hours,
				mins,
				secs,
				full_secs,
				num_beats);
			setString(ID_STAT_MSG1,buf);

			if (num_beats)	// to prevent annoying -32767's on 0th cycle
			{
				float pull_ratio = stat_num_push + stat_num_pull ?
					((float)(stat_num_pull))/((float)(stat_num_pull + stat_num_push)) : 0;

				sprintf(buf,"restarts(%d) pushes(%d) pulls(%d) pull_ratio(%0.3f)",
						stat_num_restarts,
						stat_num_push,
						stat_num_pull,
						pull_ratio);
				setString(ID_STAT_MSG2,buf);

				sprintf(buf,"power min(%d) max(%d)  angle left(%0.3f,%0.3f) to right(%0.3f,%0.3f)",
					stat_min_power,
					stat_max_power,
					stat_max_left,
					stat_min_left,
					stat_min_right,
					stat_max_right);
				setString(ID_STAT_MSG3,buf);

				sprintf(buf,"cycle(%d,%d)  error(%d,%d)",
					stat_min_cycle,
					stat_max_cycle,
					stat_min_error,
					stat_max_error);
				setString(ID_STAT_MSG4,buf);
			}
		}

		// Do NPT clock correction

	#if CLOCK_WITH_NTP

		else if (clock_running &&
				_ntp_interval &&
				num_beats - last_ntp >= _ntp_interval)
		{
			last_ntp = num_beats;
			num_ntp_checks++;
			uint32_t ntp_time = getNtpTime();
			if (ntp_time)
			{
				uint32_t this_time = time(NULL);
				int32_t diff_time = this_time - ntp_time;
				const char *msg = "NTP_OK";

				if (diff_time)
				{
					msg = "NTP_DIF";
					LOGW("NTP TIME CHECK DIFFERENCE == %d",diff_time);
					num_ntp_corrections ++;
					last_ntp_diff = diff_time;
					total_ntp_corrections += diff_time;
					total_abs_ntp_corrections += abs(diff_time);
					syncNTPTime();
				}
				else
				{
					LOGU("NTP TIME CHECK - NTP and local time() are the same");
				}

				sprintf(buf,"NTP num(%d) fails(%d)%s CORRECTIONS(%d)%s last(%d) total(%d) total_abs(%d)",
					num_ntp_checks,
					num_ntp_fails,
					(num_ntp_corrections ? "<b>" : ""),
					num_ntp_corrections,
					(num_ntp_corrections ? "</b>" : ""),
					last_ntp_diff,
					total_ntp_corrections,
					total_abs_ntp_corrections);
				setString(ID_STAT_MSG6,buf);
			}
			else
			{
				num_ntp_fails++;
				LOGE("NTP TIME CHECK FAILURE");
			}
		}

	#endif

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
