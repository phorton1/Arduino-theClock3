// theClock3.cpp
//
// grrrrr - getting unresolved references to time variables in theClock3.cpp
// when I removed uneeded WebSockets.h from myIOTDevice.h.
// But they get fixed when I needlessly include WiFi.h in myIOTTypes.h.
// The only thing I could find was a slight difference in the order of includes
// in the two compile statements, with WiFi coming before Preferences.h when it
// worked, and after when it failed.  So I moved Preferences.h from myIOTValue.h
// to the cpp file, but alas, it did not 'fix' this weird problem.
// And it doesn't seem to help to includes that stuff first in this compile.

#include "theClock3.h"
#include <myIOTLog.h>
#include <Adafruit_NeoPixel.h>
#include <AS5600.h>
#include <Wire.h>


// TODO:
//
// Need to implement at least basic buttons.
// Should be able to do basic setup via buttons and leds.
// Would like to implement other pixel modes.
//
// POSSIBLE CHANGES:
//
// I'd like to refactor the code to separate out the AS5600, pixels,
// buttons, and make this file smaller and easier to understand.
// But there are a ton of incestuous static global variables.
//
// It may be preferable to adjust the RTC clock when it has drifted a certain amount.
// Same with NTP.
//
// It might be better to base the cycle time directly on the RTC and eliminate
// the notion of separately syncing to it.
//
// It still might be possible (and/or better) to have a single PID controller
// that directly corrects for the erorr.  However, that complicates the setup
// process that currently allows me to test power, and angles, separately
// from the 2nd 'time' PID controller.


#define MAX_INT		32767
#define MIN_INT		-32767

// USES A TASK FOR RUNNING THE CLOCK
//
// there is too much critical timing in the clock run() method to allow it
// allow it to run in loop() on the same core as the webservers, etc.
// When just run from the default core(1), along with the Webserver, it
// regularly misses crossings, etc, esp when the myIOT WebSocket goes on and off.
//
// So I made it a task on the unused core 0. However, usually tasks are limited
// to 10ms time slices, and so it did not work at first.
//
// I was able to get it working. First I changed the call to vTaskDelay(0)
// as I read that would yield without delay.  No joy.  I set the priority
// of the task down to 1, no joy.  Finally I disabled the Watch Dog Timer,
// (see code below) and that seems to work.


//----------------------------
// motor
//----------------------------

#define PWM_FREQUENCY	5000
#define PWM_RESOLUTION	8

static int motor_state = 0;

void motor(int state, int power)
{
	motor_state = state;
	int use_power = state ? power : 0;
	ledcWrite(0, use_power);
	digitalWrite(PIN_IN1,state == 1  ? 1 : 0);
	digitalWrite(PIN_IN2,state == -1 ? 1 : 0);
}


//----------------------------
// AS5600 Sensor
//----------------------------
// With the AS5600, the return values are 0..4095 where 4096 is 360 degrees (i.e. 0).
// so each bit is 0.087890625 degrees.  For hysterisis (debouncing) we only take new
// values when the change exceeds AS4500_THRESHOLD.

#define AS4500_THRESHOLD    4	// 5 required for change == about 0.5 degrees

static int as5600_cur;			// bits, not degrees
static int as5600_side;			// which side of zero is the pendulum on?
static int as5600_direction;	// which direction is it moving?

static int as5600_min;			// min and max are assigned on direction changes
static int as5600_max;
static int as5600_temp_min;		// and the temps are reset to zero for next go round
static int as5600_temp_max;

// angles are calculated from integers when they change

static float as5600_cur_angle;
static float as5600_min_angle;
static float as5600_max_angle;

AS5600 as5600;   //  uses default Wire


static float angle(int units)
{
	float retval = units * 3600;
	retval /= 4096;
	return floor(retval + 0.5) / 10;
}


//------------------------------------------
// Semaphore between AS5600 and pixels.show()
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


//----------------------------
// pixel(s)
//----------------------------
// Native pixel brightness is weird!!  0==max, 1=min, 255=max-1
// I change it so that 0=min, and 254=max-1 and never send 0


static int show_pixels = 0;

static Adafruit_NeoPixel pixels(NUM_PIXELS,PIN_LEDS);


void setPixel(int num, uint32_t color)
{
	pixels.setPixelColor(num,color);
}


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



//--------------------------------------------
// vars
//--------------------------------------------
// All times are stored and manipulated as UTC times so
// changes in daylight savings time do not affect the clock.
// This can be tested by changing timezones on a running clock.


#define CLOCK_STATE_NONE    	0
#define CLOCK_STATE_STATS   	1
#define CLOCK_STATE_START		2
#define CLOCK_STATE_STARTED		3
#define CLOCK_STATE_RUNNING		4

static int 	 	clock_state = 0;
static bool 	start_sync = 0;				// doing a synchronized start
static uint32_t last_change = 0;			// millis of last noticable pendulum movement
static  int32_t cur_cycle = 0;				// millis in this 'cycle' (forward zero crossing)
static uint32_t last_cycle = 0;				// millis at previous forward zero crossing
static uint32_t num_beats = 0;				// number of beats (while clock_started && !initial_pulse_time)

static uint32_t time_start = 0;
static uint32_t time_start_ms = 0;
static uint32_t time_zero = 0;				// ESP32 RTC time at zero crossing
static uint32_t time_zero_ms = 0;			// with milliseconds (from microseconds)
static uint32_t time_init = 0;
static uint32_t time_init_ms = 0;

static int32_t  total_millis_error = 0;		// cumulative number of millis total error
static int32_t  prev_millis_error = 0;
static float    pid_angle = 0;				// angle determined by second PID controller or other parameters

static float 	total_ang_error = 0;		// accumluated degrees of error (for "I")
static float 	prev_ang_error = 0;			// the previous error (for "D")
static int 	 	pid_power = 0;				// power adjusted by pid algorithm

// SYNC

static int sync_sign = 0;
static int sync_millis = 0;

// CONTROL

static uint32_t initial_pulse_time = 0;	// time at which we started initial clock starting pulse (push)
static bool push_motor = 0;				// push the pendulum next time after it leaves deadzone (determined at zero crossing)
static uint32_t motor_start = 0;
static uint32_t motor_dur = 0;

static uint32_t last_beat = 0xffffffff;
static uint32_t last_stats = 0;
static uint32_t last_sync = 0;
static uint32_t last_ntp = 0;


// Statistics

static char msg_buf[512];
	// generic buffer for loop() related messages

static bool 	update_stats = false;

static uint32_t stat_num_bad_reads;
static uint32_t stat_num_restarts;

static int 		stat_min_cycle;
static int 		stat_max_cycle;
static int  	stat_min_error;
static int  	stat_max_error;
static int  	stat_max_power;
static int  	stat_min_power;
static float	stat_min_left;
static float	stat_max_left;
static float	stat_min_right;
static float	stat_max_right;
static float	stat_min_target;
static float	stat_max_target;
static float	stat_min_total_ang_err;
static float	stat_max_total_ang_err;

static int 		stat_recent_min_cycle;
static int 		stat_recent_max_cycle;
static int  	stat_recent_min_error;
static int  	stat_recent_max_error;
static int  	stat_recent_max_power;
static int  	stat_recent_min_power;
static float	stat_recent_min_left;
static float	stat_recent_max_left;
static float	stat_recent_min_right;
static float	stat_recent_max_right;
static float	stat_recent_min_target;
static float	stat_recent_max_target;
static float	stat_recent_min_total_ang_err;
static float	stat_recent_max_total_ang_err;


static uint32_t stat_num_sync_checks = 0;
static uint32_t stat_num_sync_changes = 0;
static  int32_t stat_last_sync_change = 0;
static  int32_t stat_total_sync_changes = 0;
static uint32_t stat_total_sync_changes_abs = 0;
static uint32_t stat_num_ntp_checks = 0;
static uint32_t stat_num_ntp_fails = 0;
static uint32_t stat_num_ntp_changes = 0;
static  int32_t stat_last_ntp_change = 0;
static  int32_t stat_total_ntp_changes = 0;
static uint32_t stat_total_ntp_changes_abs = 0;



//-----------------------------------------------
// utilities
//-----------------------------------------------

int32_t timeDeltaMS(int32_t secs1, int32_t ms1, int32_t secs2, int32_t ms2)
{
	int32_t diff_secs = secs1 - secs2;
	int32_t diff_ms = ms1 - ms2;
	while (diff_ms < 0)
	{
		diff_secs--;
		diff_ms += 1000L;
	}
	while (diff_ms >= 1000L)
	{
		diff_secs++;
		diff_ms -= 1000L;
	}
	diff_ms += diff_secs * 1000L;
	return diff_ms;
}

void timeAddMS(int32_t *secs, int32_t *ms, int32_t ms_delta)
{
	*ms += ms_delta;
	while (*ms < 0)
	{
		*secs -= 1;
		*ms += 1000L;
	}
	while (*ms >= 1000L)
	{
		*secs += 1;
		*ms -= 1000L;
	}
}



//-----------------------------------------------
// setup
//-----------------------------------------------

// virtual
void theClock::setup()	// override
{
	LOGU("theClock::setup() started");

	pixels.clear();
	pixels.setBrightness(64);

	for (int i=NUM_PIXELS-1; i>=0; i--)
	{
		setPixel(i,MY_LED_CYAN);
		pixels.show();
		delay(300);
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

	setPixel(PIXEL_MAIN,MY_LED_ORANGE);
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
				setPixel(PIXEL_MAIN,i&1?MY_LED_RED:MY_LED_BLACK);
				pixels.show();
				delay(300);
			}
		}
	}

	LOGU("AS5600 connected=%d",connected);

	//------------------------------
	// call myIOTDevice::setup()
	//------------------------------

	setPixel(PIXEL_MAIN,MY_LED_CYAN);
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

	if (_zero_angle == 0)
	{
		setPixel(PIXEL_MAIN,MY_LED_PURPLE);
		pixels.show();
		setZeroAngle();
		delay(500);
	}
	LOGI("_zero_angle=%d == %0.3f degrees",_zero_angle,angle(_zero_angle));

	// debug_angle("in theClock::setup() before starting clockTask");

	pixels.setBrightness(_led_brightness);
	pixels.show();

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

	LOGU("theClock::setup() finished");
	pixels.clear();
	pixels.show();

	show_pixels = 2;

}	// theClock::setup()




void theClock::initAS5600()
{
	as5600_cur = _zero_angle;
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


void theClock::initMotor()
{
	motor(0,0);

	clock_state = 0;
	last_change = 0;
	initial_pulse_time = 0;
	push_motor = 0;
	motor_start = 0;
	motor_dur = 0;

	time_zero = 0;
	time_zero_ms = 0;
}


void theClock::initStats(bool restart)
{
	update_stats = false;

	if (!restart)
	{
		stat_num_bad_reads = 0;
		stat_num_restarts = 0;
	}

	cur_cycle = 0;
	last_cycle = 0;
	num_beats = 0;
	time_init = 0;
	time_init_ms = 0;

	total_ang_error = 0;
	prev_ang_error = 0;

	total_millis_error = 0;
	prev_millis_error = 0;

	sync_sign = 0;
	sync_millis = 0;

	last_sync = 0;
	last_stats = 0;
	last_ntp = 0;

	stat_min_cycle = MAX_INT;
	stat_max_cycle = MIN_INT;
	stat_min_error = MAX_INT;
	stat_max_error = MIN_INT;
	stat_min_power = 255;
	stat_max_power = 0;
	stat_min_left = MIN_INT;
	stat_max_left = MAX_INT;
	stat_min_right = MAX_INT;
	stat_max_right = MIN_INT;
	stat_min_target = MAX_INT;
	stat_max_target = MIN_INT;
	stat_min_total_ang_err = MAX_INT;
	stat_max_total_ang_err = MIN_INT;

	stat_recent_min_cycle = MAX_INT;
	stat_recent_max_cycle = MIN_INT;
	stat_recent_min_error = MAX_INT;
	stat_recent_max_error = MIN_INT;
	stat_recent_min_power = 255;
	stat_recent_max_power = 0;
	stat_recent_min_left = MIN_INT;
	stat_recent_max_left = MAX_INT;
	stat_recent_min_right = MAX_INT;
	stat_recent_max_right = MIN_INT;
	stat_recent_min_target = MAX_INT;
	stat_recent_max_target = MIN_INT;
	stat_recent_min_total_ang_err = MAX_INT;
	stat_recent_max_total_ang_err = MIN_INT;

	stat_num_sync_checks = 0;
	stat_num_sync_changes = 0;
	stat_last_sync_change = 0;
	stat_total_sync_changes = 0;
	stat_total_sync_changes_abs = 0;
	stat_num_ntp_checks = 0;
	stat_num_ntp_fails = 0;
	stat_num_ntp_changes = 0;
	stat_last_ntp_change = 0;
	stat_total_ntp_changes = 0;
	stat_total_ntp_changes_abs = 0;

	the_clock->setString(ID_STAT_MSG0,"");
	the_clock->setString(ID_STAT_MSG1,"");
	the_clock->setString(ID_STAT_MSG2,"");
	the_clock->setString(ID_STAT_MSG3,"");
	the_clock->setString(ID_STAT_MSG4,"");
	the_clock->setString(ID_STAT_MSG5,"");
	the_clock->setString(ID_STAT_MSG6,"");

}


void theClock::clearStats()
{
	LOGU("STATISTICS CLEARED");
	initStats(0);
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	time_start = tv_now.tv_sec;
	time_start_ms = tv_now.tv_usec / 1000L;
	update_stats = true;
}


void theClock::stopClock()
{
	start_sync = 0;
	LOGU("stopClock()");
	initMotor();
}


void theClock::onStartClockSynchronized()
{
	LOGU("StartClockSynchronized()");

	// The seconds hand is attached such that a tock falls on an even second:
	//
	// tick = left crossing = 1/2 second
	// tock = right crossing = even second
	//
	// The user positions the hands on the tock (after a right crossing) with the second hand
	// on zero and the minute/hour set to the next minute that will cross, and presses
	// the button during the minute before the next minute.
	//
	// We issue the initial pulse at 1 second before the minute crossing (59) plus
	// the START_DELAY parameter in MS.
	//
	// The first left crossing should take place about 500 ms after zero, establishing
	// the init_time, and the second left crossing about 1500ms after zero, establishing
	// beat #1
	//
	// Regardless of the clocks left/right starting bias, the START_DELAY *should*
	// be adjustable so that the first left crossing occurs about 500ms after zero.

	if (clock_state != CLOCK_STATE_NONE )
		LOGE("Attempt to call onStartClockSynchronized() while it's already running!");
	else
		start_sync = true;
}


void theClock::startClock(bool restart /*=0*/)
{
	LOGU("startClock(%d)",restart);

	initMotor();
	initAS5600();
	initStats(restart);

	if (_clock_mode == CLOCK_MODE_SENSOR_TEST)
	{
		pid_power = 0;
		pid_angle = 0;
		clock_state = CLOCK_STATE_STATS;
	}
	else
	{
		pid_power = _power_pid;
		pid_angle =
			_clock_mode == CLOCK_MODE_ANGLE_MIN ? _angle_min :
			_clock_mode == CLOCK_MODE_ANGLE_MAX ? _angle_max :
			_clock_mode == CLOCK_MODE_MIN_MAX ? _angle_min :
			_angle_start;

		if (!restart)
		{
			struct timeval tv_now;
			gettimeofday(&tv_now, NULL);
			time_start = tv_now.tv_sec;
			time_start_ms = tv_now.tv_usec / 1000L;
		}

		motor(-1,_power_start);
		initial_pulse_time = last_change = millis();
		clock_state = CLOCK_STATE_START;
	}
}


void theClock::onClockRunningChanged(const myIOTValue *desc, bool val)
{
	LOGU("onClockRunningChanged(%d)",val);
	if (!val) stopClock();
}


void theClock::onClockModeChanged(const myIOTValue *desc, uint32_t val)
{
	LOGU("onClockModeChanged(%d)",val);
	stopClock();
}


void theClock::onPlotValuesChanged(const myIOTValue *desc, uint32_t val)
{
	if (val)
		Serial.println("dir,side,angle,min,max,err,motor");
}


void theClock::onPixelModeChanged(const myIOTValue *desc, uint32_t val)
{
	LOGU("onPixelModeChanged(%d)",val);
	show_pixels = 2;
}

void theClock::onBrightnessChanged(const myIOTValue *desc, uint32_t val)
{
	LOGU("onBrightnessChanged(%d)",val);
	show_pixels = 2;
}


void theClock::setZeroAngle()
{
	LOGI("Setting AS5600 zero angle ...");
	int zero = as5600.readAngle();
	float zero_f =  angle(zero);
	LOGU("AS5600 zero angle=%d  %0.3f", zero,zero_f);
	the_clock->setInt(ID_ZERO_ANGLE,zero);
	the_clock->setFloat(ID_ZERO_ANGLE_F,zero_f);
}


void theClock::onTestMotor(const myIOTValue *desc, int val)
{
	LOGU("onTestMotor %d",val);
	motor(val,_power_min);
}


void theClock::onDiddleClock(const myIOTValue *desc, int val)
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	int orig_secs = tv_now.tv_sec;
	int orig_ms = tv_now.tv_usec / 1000L;

	if (!val)
	{
		LOGU("onDiddleClock(%d) orig=%d.%03d  NO CHANGE",val,orig_secs,orig_ms);
	}
	else
	{
		int new_ms = orig_ms;
		int new_secs = orig_secs;
		timeAddMS(&new_secs,&new_ms,val);
		LOGU("onDiddleClock(%d) orig=%d.%03d  new=%d.%0.3d",val,orig_secs,orig_ms,new_secs,new_ms);
		timeval e_time = {new_secs, new_ms * 1000};
		settimeofday((const timeval*)&e_time, 0);
	}
}



void theClock::onSyncRTC()
{
	if (clock_state != CLOCK_STATE_RUNNING )
	{
		LOGE("Attempt to call onSyncRTC() while clock is not running!");
		return;
	}

	// we no longer care about total_millis_error

	total_millis_error = 0;

	// time_init and time_init_ms was the 0th zero crossing
	// num_beats is the number of full ticks that have taken place since time_init
	// and the convention is that the zero crossing should take place at 500ms,
	// 'beat_time' == the time the clock is showing plus that 500 ms.

	int32_t beat_time = time_init + num_beats;
	int32_t beat_time_ms = 500;

	// then we have the 'actual' time (of the most recent zero crossing)
	// if the 'actual_time' is greater than the 'beat' time, then the clock needs to speed up.
	// sync_millis has the semantic of a 'correction', that is, a positive value will speed
	// the clock up, and a negative value will slow it down.

	sync_millis = timeDeltaMS(time_zero,time_zero_ms,beat_time,beat_time_ms);

	// therefore to speed up, we need to subtract beat_time from time_zero,
	// as we do in the above call, to result in a positive sync_millis to speed the clock up.

	sync_sign =
		sync_millis < 0 ? - 1 :
		sync_millis > 0 ? 1 : 0;

	stat_num_sync_checks++;
	if (sync_sign)
	{
		stat_num_sync_changes++;
		stat_last_sync_change = sync_millis;
		stat_total_sync_changes += sync_millis;
		stat_total_sync_changes_abs += abs(sync_millis);
		LOGU("onSyncRTC(%d/%d) beats=%d beat_time=%d.%03d time_zero=%d.%03d  sync_millis=%d sign=%d",
			stat_num_sync_changes,
			stat_num_sync_checks,
			num_beats,
			beat_time,
			beat_time_ms,
			time_zero,
			time_zero_ms,
			sync_millis,
			sync_sign);
	}
	else
		LOGU("onSyncRTC(%d/%d) no change",stat_num_sync_changes,stat_num_sync_checks);

	sprintf(msg_buf,"SYNC(%d/%d) last(%d) total(%d) abs(%d)",
		stat_num_sync_changes,
		stat_num_sync_checks,
		stat_last_sync_change,
		stat_total_sync_changes,
		stat_total_sync_changes_abs);
	the_clock->setString(ID_STAT_MSG5,msg_buf);
}



#if CLOCK_WITH_NTP
	void theClock::onSyncNTP()
	{
		// these stats are only approximate
		// as the final actual time will be determined
		// by ntp sync processes beyond our control

		stat_num_ntp_checks++;
		int32_t ntp_secs;
		int32_t ntp_ms;

		if (!getNtpTime(&ntp_secs,&ntp_ms))
		{
			stat_num_ntp_fails++;
			LOGE("getNtpTime() failed!!");
		}
		else
		{
			struct timeval tv_now;
			gettimeofday(&tv_now, NULL);
			int32_t now_secs = tv_now.tv_sec;
            int32_t now_ms = tv_now.tv_usec / 1000L;
			int32_t delta_ms = timeDeltaMS(ntp_secs,ntp_ms,now_secs,now_ms);
				// The delta is negative if our clock is faster, or positive if our clock is slower,
				// in keeping with the convention that these are CORRECTIONS to be made, so if our
				// clock is faster, and delta is negative, we will increase the cycle time to make
				// up for the difference.
			if (delta_ms)
			{
				stat_last_ntp_change = delta_ms;
				stat_num_ntp_changes++;
				stat_total_ntp_changes += delta_ms;
				stat_total_ntp_changes_abs += abs(delta_ms);
				LOGU("onSyncNTP(%d/%d) now(%d.%03d) ntp(%d.%03d) delta=%d",
					stat_num_ntp_changes,
					stat_num_ntp_checks,
					now_secs,
					now_ms,
					ntp_secs,
					ntp_ms,
					delta_ms);
				syncNTPTime();
			}
			else
				LOGU("onSyncNTP(%d/%d) no change",stat_num_ntp_changes,stat_num_ntp_checks);
		}

		sprintf(msg_buf,"NTP(%d/%d) fails(%d) last(%d) total(%d) abs(%d)",
			stat_num_ntp_changes,
			stat_num_ntp_checks,
			stat_num_ntp_fails,
			stat_last_ntp_change,
			stat_total_ntp_changes,
			stat_total_ntp_changes_abs);
		the_clock->setString(ID_STAT_MSG6,msg_buf);
	}

#endif	// CLOCK_WITH_NTP



//===================================================================
// PID CONTROLLERS
//===================================================================

float theClock::getPidAngle()  // APID parameters
{
	// this_p == current ms error
	// this_i == total ms error (including sync_millis)
	// this_d == delta ms error this cycle

	float this_p = cur_cycle;
	this_p -= 1000.0;
	float this_i = total_millis_error + sync_millis;
	float this_d = prev_millis_error - this_p;;
	prev_millis_error = this_p;

	this_p = this_p / 1000;
	this_i = this_i / 1000;
	this_d = this_d / 1000;

	float factor = 1 + (_apid_P * this_p) + (_apid_I * this_i) + (_apid_D * this_d);
	float new_angle = pid_angle * factor;
	if (new_angle > _angle_max) new_angle = _angle_max;
	if (new_angle  < _angle_min) new_angle = _angle_min;

	// LOGD("getPidAngle pid(%0.3f) cur(%d) P(%0.3f) I(%0.3f) D(%0.3f) factor(%0.3f) new(%0.3f)",
	// 	pid_angle,
	// 	cur_cycle,
	// 	this_p,
	// 	this_i,
	// 	this_d,
	// 	factor,
	// 	new_angle);

	pid_angle = new_angle;

	if (pid_angle < stat_min_target)
		stat_min_target = pid_angle;
	if (pid_angle > stat_max_target)
		stat_max_target = pid_angle;
	if (pid_angle < stat_recent_min_target)
		stat_recent_min_target = pid_angle;
	if (pid_angle > stat_recent_max_target)
		stat_recent_max_target = pid_angle;

	return pid_angle;
}


int theClock::getPidPower(float avg_angle)	// PID parameters
{
	// this_p == current angular error
	// this_i == total running angular error
	// this_d == delta angular error this cycle

	float this_p = pid_angle - avg_angle;
	total_ang_error += this_p;
	float this_i = total_ang_error;
	float this_d = prev_ang_error - this_p;;
	prev_ang_error = this_p;

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




//===================================================================
// run()
//===================================================================

void theClock::run()
	// This is called every 4 ms or so ...
	// Try not to call setXXX stuff while clock is running.
{
	// start the clock if start_sync and at a minute crossing,
	// or if the running variable turned on and not running

	if (clock_state == CLOCK_STATE_NONE)
	{
		if (start_sync)
		{
			if (time(NULL) % 60 == 59)
			{
				LOGI("starting synchronized delay=%d",_start_delay);
				if (_start_delay)
					delay(_start_delay);				// do it about 1/2 second early
				the_clock->setBool(ID_RUNNING,1);	    // set the UI bool
				startClock();							// and away we go
			}
		}
		else if (_clock_running)
			startClock();
		return;
	}

	// end the startup pulse if in 'starting' mode
	// and go to 'started' (if pid) or 'running' (otherwise)

	uint32_t now = millis();
	if (clock_state == CLOCK_STATE_START)
	{
		if (now - initial_pulse_time > _dur_start)
		{
			initial_pulse_time = 0;
			motor(0,0);

			// non motor-pid modes start off as 'running',
			// where as pid modes start off as 'started'
			// and only go to running later

			if (_clock_mode < CLOCK_MODE_ANGLE_START)
				clock_state = CLOCK_STATE_RUNNING;
			else
				clock_state = CLOCK_STATE_STARTED;
		}
		else
		{
			return;
		}
	}

	// Restart if necessary

	int res_millis = now - last_change;
	if (_clock_mode > CLOCK_MODE_SENSOR_TEST &&
		clock_state >= CLOCK_STATE_STARTED &&
		res_millis > _restart_millis)
	{
		LOGW("RESTARTING CLOCK!! now=%d last_change=%d  diff=%d  constant=%d",now,last_change,res_millis,_restart_millis);
		stat_num_restarts++;
		startClock(1);
		return;
	}


	//-------------------------------------------------
	// AS5600
	//-------------------------------------------------
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

	#define MAX_ANGLE  15.0

	WAIT_SEMAPHORE();
	int raw = as5600.readAngle();
	RELEASE_SEMAPHORE();

	int cur = raw - _zero_angle;
	float cur_angle = angle(cur);
	if (abs(cur_angle) > MAX_ANGLE)
	{
		stat_num_bad_reads++;
		LOGE("Bogus angle reading raw=%d cur=%d zero=%d angle=%0.3f",raw,cur,_zero_angle,cur_angle);
		return;
	}

	as5600_cur_angle = angle(cur);

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
	{
		as5600_cur = cur;
		last_change = now;

		if (clock_state == CLOCK_STATE_STATS && _plot_values == PLOT_OFF)
		{
			LOGU("as5600=%-4d  angle=%0.3f",cur,as5600_cur_angle);
		}

		// detect zero crossing

		else if ((cur < 0 && as5600_side >= 0) ||
			     (cur > 0 && as5600_side <= 0))
		{
			as5600_side = cur < 0 ? -1 : 1;
			push_motor = true;

			// Get full cycle time using uint32_t subtract to work at millis() overflow crossing.
			// Calculate signed error using int32_t and add it to the accumulated error if running
			// We use the left crossing, which is the tick to 1/2 second so that when we synchronize
			// to 500ms, the tock will fall more or less on the exact second
			//
			// We use millis() and a separate syncRTC() method rather than just using the RTC clock time
			// so that changes due to drift from the RTC clock time will not all get lumped into the
			// instantaneous error and this allows us to keep track of the millis() vs RTC drift.

			if (as5600_side < 0)
			{
				if (last_cycle)
					cur_cycle = now - last_cycle;
				last_cycle = now;

				if (cur_cycle)
				{
					// best guess of the actual RTC time at zero crossing

					struct timeval tv_now;
					gettimeofday(&tv_now, NULL);
					time_zero = tv_now.tv_sec;
                    time_zero_ms = tv_now.tv_usec / 1000L;

					int err = cur_cycle;
					err -= 1000;

					// if sync_sign, we are in a sync and the err is
					// added to the sync_millis until it changes sign,
					// at which point the sync is "turned off" and the
					// remaining error falls through to the 'total_millis_error'

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
						}

						// if the sync is not turned off, it ate all the error millis

						else
						{
							err = 0;
						}
					}

					// might be zero contribution while syncing ...

					total_millis_error += err;

					// we are either on the 0th beat (starting the clock)
					// or we start with the first beat one full cycle later
					// and subsequently increment it

					if (!time_init)
					{
						update_stats = true;
						time_init = time_zero;	// time(NULL);
						time_init_ms = time_zero_ms;
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
					if (total_millis_error < stat_min_error)
						stat_min_error = total_millis_error;
					if (total_millis_error > stat_max_error)
						stat_max_error = total_millis_error;

					if (cur_cycle < stat_recent_min_cycle)
						stat_recent_min_cycle = cur_cycle;
					if (cur_cycle > stat_recent_max_cycle)
						stat_recent_max_cycle = cur_cycle;
					if (total_millis_error < stat_recent_min_error)
						stat_recent_min_error = total_millis_error;
					if (total_millis_error > stat_recent_max_error)
						stat_recent_max_error = total_millis_error;

				}	// cycle has been established
			}	// left zero crossing
		}	// zero crossing

		// detect direction change
		// if direction changed, assign min or max and clear temp variable

		if (clock_state > CLOCK_STATE_STATS)
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
					as5600_min_angle = angle(as5600_min);

					if (as5600_min_angle < stat_max_left)
						stat_max_left = as5600_min_angle;
					if (as5600_min_angle > stat_min_left)
						stat_min_left = as5600_min_angle;

					if (as5600_min_angle < stat_recent_max_left)
						stat_recent_max_left = as5600_min_angle;
					if (as5600_min_angle > stat_recent_min_left)
						stat_recent_min_left = as5600_min_angle;
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

					if (as5600_max_angle > stat_recent_max_right)
						stat_recent_max_right = as5600_max_angle;
					if (as5600_max_angle < stat_recent_min_right)
						stat_recent_min_right = as5600_max_angle;
				}

			}	// direction changed
		}	// clock started

		//------------------------------------------------
		// state machine
		//------------------------------------------------
		// Switch to RUNNING if needed

		if (clock_state == CLOCK_STATE_STARTED &&
			as5600_max_angle >= _running_angle &&
			as5600_min_angle <= -_running_angle &&
			abs(total_ang_error) < _running_error)
		{
			clock_state = CLOCK_STATE_RUNNING;
			LOGU("Clock running!");
			// clearStats();
		}

		// do initial sync if running and we have a good set of times

		if (start_sync &&
			clock_state == CLOCK_STATE_RUNNING &&
			time_init && cur_cycle )
		{
			start_sync = false;
			last_sync = num_beats;
			onSyncRTC();
		}

		// Push the motor if asked to and out of the dead zone

		if (push_motor && abs(as5600_cur_angle) > _dead_zone)
		{
			push_motor = false;

			if (clock_state == CLOCK_STATE_RUNNING)
			{
				// in full PID_MODE get the pid angle or
				// possibly change the target angle in MIN_MAX mode

				if (_clock_mode == CLOCK_MODE_PID)
				{
					pid_angle = getPidAngle();
				}
				else if (_clock_mode == CLOCK_MODE_MIN_MAX)
				{
					if (pid_angle == _angle_min &&
						total_millis_error + sync_millis > _min_max_ms)
					{
						pid_angle = _angle_max;
					}
					else if (pid_angle == _angle_max &&
						total_millis_error + sync_millis < -_min_max_ms)
					{
						pid_angle = _angle_min;
					}
				}
			}

			// calculate the average angle even if we only use it in PID modes

			float avg_angle = (abs(as5600_min_angle) + abs(as5600_max_angle)) / 2;

			// calculate the power to use

			int use_power =
				_clock_mode == CLOCK_MODE_POWER_MIN ? _power_min :
				_clock_mode == CLOCK_MODE_POWER_MAX ? _power_max :
				getPidPower(avg_angle);

			// PULSE THE MOTOR!

			motor_start = now;
			motor_dur =_dur_pulse;
			motor(-1,use_power);

			// statistics

			if (use_power > stat_max_power)
				stat_max_power = use_power;
			if (use_power < stat_min_power)
				stat_min_power = use_power;

			if (use_power > stat_recent_max_power)
				stat_recent_max_power = use_power;
			if (use_power < stat_recent_min_power)
				stat_recent_min_power = use_power;

			if (total_ang_error < stat_min_total_ang_err)
				stat_min_total_ang_err = total_ang_error;
			if (total_ang_error > stat_max_total_ang_err)
				stat_max_total_ang_err = total_ang_error;
			if (total_ang_error < stat_recent_min_total_ang_err)
				stat_recent_min_total_ang_err = total_ang_error;
			if (total_ang_error > stat_recent_max_total_ang_err)
				stat_recent_max_total_ang_err = total_ang_error;

			if (_plot_values == PLOT_OFF)
			{
				LOGD("%-6s(%-2d) %-4d %3.3f/%3.3f=%3.3f  target=%3.3f  accum=%3.3f  power=%d  err=%d  sync=%d",
					 sync_sign ? "SYNC" : clock_state == CLOCK_STATE_RUNNING ? "run" : "start",
					 as5600_direction,
					 cur_cycle,
					 as5600_min_angle,
					 as5600_max_angle,
					 avg_angle,
					 pid_angle,
					 total_ang_error,
					 use_power,
					 total_millis_error,
					 sync_millis);
			}

		}	// push motor
	}	// angle threshold change exceeded

	// Stop the motor as needed based on duration,

	if (motor_start && now - motor_start > motor_dur)
	{
		motor_start = 0;
		motor_dur = 0;
		motor(0,0);
	}

	//----------------------
	// plotting
	//----------------------

	if (_plot_values == PLOT_WAVES)
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

		Serial.print(total_millis_error * 20);
		Serial.print(",");

		Serial.print(motor_state * 400);
		Serial.println(",1000,-1000");
	}

}	// theClock::run()



//===================================================================
// loop
//===================================================================

void formatTimeToBuf(char *buf, const char *label, uint32_t time_s, uint32_t time_ms)
{
	int at = strlen(buf);
	char *b = &buf[at];
    struct tm *ts = localtime((const time_t *)&time_s);
    sprintf(b,"%s(%04d-%02d-%02d  %02d:%02d:%02d.%03d)",
        label,
		ts->tm_year + 1900,
        ts->tm_mon + 1,
        ts->tm_mday,
        ts->tm_hour,
        ts->tm_min,
        ts->tm_sec,
		time_ms);
}



// virtual
void theClock::loop()	// override
{
	myIOTDevice::loop();

	// show the realtime clock

	if (clock_state == CLOCK_STATE_STATS)
	{
		static uint32_t last_seconds = 0;
		struct timeval tv_now;
		gettimeofday(&tv_now, NULL);
		uint32_t seconds = tv_now.tv_sec;
		if (last_seconds != seconds)
		{
			last_seconds = seconds;
			uint32_t ms = tv_now.tv_usec / 1000L;
			LOGU("tick  seconds=%d  ms=%d",seconds,ms);

		}
	}

	//--------------------------------------
	// PIXELS
	//--------------------------------------
	// In normal operation, pixels only change on each beat,
	// but system pixels can change at any time.

	uint32_t now = millis();
	static uint32_t last_pixels = 0;
	if (now - last_pixels > 100)
	{
		last_pixels = now;
		uint32_t new_pixels[NUM_PIXELS];
		memset(new_pixels,0,NUM_PIXELS * sizeof(uint32_t));

		if (_pixel_mode == PIXEL_MODE_DIAG)
		{
			iotConnectStatus_t status = getConnectStatus();
			bool wifi_on = getBool(ID_DEVICE_WIFI);
			new_pixels[PIXEL_MAIN] =
				status == IOT_CONNECT_ALL ? MY_LED_ORANGE :
				status == IOT_CONNECT_AP  ? MY_LED_PURPLE :
				status == IOT_CONNECT_STA ? MY_LED_GREEN :
				wifi_on ? MY_LED_RED :
				MY_LED_BLUE;

			new_pixels[PIXEL_STATE] =
				clock_state == CLOCK_STATE_RUNNING ? MY_LED_GREEN :
				clock_state == CLOCK_STATE_STARTED ? MY_LED_MAGENTA :
				clock_state == CLOCK_STATE_START ?   MY_LED_YELLOW :
				start_sync ? MY_LED_WHITE :
				clock_state == CLOCK_STATE_STATS ? MY_LED_ORANGE :
				MY_LED_BLACK;

			// accuracy and cycle move from green to red/blue
			// as they diverge by +/- _min_max_ms

			if (clock_state >= CLOCK_STATE_START)
			{
				new_pixels[PIXEL_ACCURACY] =
					total_millis_error >=  _min_max_ms 	? MY_LED_BLUE      :
					total_millis_error <= -_min_max_ms 	? MY_LED_RED   :
					scalePixel(total_millis_error,_min_max_ms,
						MY_LED_RED,
						MY_LED_GREEN,
						MY_LED_BLUE);

				int dif = cur_cycle - 1000;
				if (dif >= _min_max_ms)
					new_pixels[PIXEL_CYCLE] = MY_LED_BLUE;
				else if (dif <= -_min_max_ms)
					new_pixels[PIXEL_CYCLE] = MY_LED_RED;
				else
					new_pixels[PIXEL_CYCLE] = scalePixel(dif,_min_max_ms,
						MY_LED_RED,
						MY_LED_GREEN,
						MY_LED_BLUE);

				new_pixels[PIXEL_SYNC] =
						!sync_sign ? MY_LED_BLACK :
						scalePixel(sync_millis,_min_max_ms,
							MY_LED_RED,
							MY_LED_GREEN,
							MY_LED_BLUE);
			}
		}	// PIXEL_MODE_DIAG

		// set pixels and show if changed

		static uint32_t old_pixels[NUM_PIXELS];
		if (show_pixels == 2)
		{
			memset(old_pixels,0,NUM_PIXELS * sizeof(uint32_t));
			pixels.clear();
			pixels.setBrightness(_led_brightness + 1);
		}

		for (int i=0; i<=NUM_PIXELS-1; i++)
		{
			if (show_pixels == 2 || old_pixels[i] != new_pixels[i])
			{
				old_pixels[i] = new_pixels[i];
				setPixel(i,new_pixels[i]);
				if (!show_pixels)
					show_pixels = 1;
			}
		}

		// The use of pixels.canShow() may be superflous, but
		// there can be upto a 300 us delay at the top of pixels.show()
		// BEFORE the interrupts are disabled so I check it before
		// calling pixels.show()

		if (show_pixels && pixels.canShow())
		{
			show_pixels = 0;
			WAIT_SEMAPHORE();
			pixels.show();
			RELEASE_SEMAPHORE();
		}
	}	// now - 100 > last_pixels


	//------------------------------------------
	// THINGS BASED ON THE BEAT CHANGING
	//------------------------------------------
	// We base these on the beat changing to ensure that
	// they take place near a 500ms crossing, but that
	// may not be necessary.  As it stands right now
	// because they are based on the beat, rather than on a time,
	// we have to zero their 'last' values upon restarting.

	// 1. Error correction vs ESP32 clock
	// 2. Sbow statistics
	// 3. NTP vs ESP32 clock correction

	if (clock_state >= CLOCK_STATE_STARTED &&
		last_beat != num_beats)
	{
		last_beat = num_beats;

		// 1. Error correction vs ESP32 clock

		if (!sync_sign &&
			_sync_interval &&
			clock_state == CLOCK_STATE_RUNNING &&
			num_beats - last_sync >= _sync_interval)
		{
			last_sync = num_beats;
			onSyncRTC();
		}

		// 2. Sbow statistics

		else if (_stat_interval &&
				 _plot_values == PLOT_OFF &&
				 last_stats != num_beats && (
				 update_stats ||
				 num_beats % _stat_interval == 0))
		{
			update_stats = false;
			last_stats = num_beats;
			LOGI("--> stats");

			// show the value of the RTC at the last zero crossing

			msg_buf[0] = 0;
			formatTimeToBuf(msg_buf,"TIME_START",time_start,time_start_ms);
			strcat(msg_buf,"<br>");
			formatTimeToBuf(msg_buf,"TIME_INIT",time_init,time_init_ms);
			strcat(msg_buf,"<br>");
			formatTimeToBuf(msg_buf,"CUR_TIME",time_zero,time_zero_ms);
			setString(ID_STAT_MSG0,msg_buf);

			uint32_t full_secs = time_init ? time_zero - time_init : 0;
			uint32_t secs = full_secs;
			uint32_t mins = secs / 60;
			uint32_t hours = mins / 60;
			secs = secs - mins * 60;
			mins = mins - hours * 60;

			sprintf(msg_buf,"%s %02d:%02d:%02d  == %d SECS %d BEATS  num_bad(%d)  restarts(%d)",
				(clock_state == CLOCK_STATE_RUNNING ?"RUNNING":"STARTING"),
				hours,
				mins,
				secs,
				full_secs,
				num_beats,
				stat_num_bad_reads,
				stat_num_restarts);
			setString(ID_STAT_MSG1,msg_buf);

			if (cur_cycle)	// to prevent annoying -32767's on 0th cycle
			{
				sprintf(msg_buf,"ALL cycle(%d,%d) error(%d,%d) power(%d,%d) ang_error(%0.3f,%0.3f)<br>ANGLE target(%0.3f,%0.3f) left(%0.3f,%0.3f) right(%0.3f,%0.3f)",
					stat_min_cycle,
					stat_max_cycle,
					stat_min_error,
					stat_max_error,
					stat_min_power,
					stat_max_power,
					stat_min_total_ang_err,
					stat_max_total_ang_err,
					stat_min_target,
					stat_max_target,
					stat_max_left,
					stat_min_left,
					stat_min_right,
					stat_max_right);
				setString(ID_STAT_MSG2,msg_buf);

				sprintf(msg_buf,"RECENT cycle(%d,%d) error(%d,%d) power(%d,%d) ang_error(%0.3f,%0.3f)<br>ANGLE target(%0.3f,%0.3f) left(%0.3f,%0.3f) right(%0.3f,%0.3f)",
					stat_recent_min_cycle,
					stat_recent_max_cycle,
					stat_recent_min_error,
					stat_recent_max_error,
					stat_recent_min_power,
					stat_recent_max_power,
					stat_recent_min_total_ang_err,
					stat_recent_max_total_ang_err,
					stat_recent_min_target,
					stat_recent_max_target,
					stat_recent_max_left,
					stat_recent_min_left,
					stat_recent_min_right,
					stat_recent_max_right);
				setString(ID_STAT_MSG3,msg_buf);

				// sprintf(msg_buf,"");
				// setString(ID_STAT_MSG4,msg_buf);
			}

			// reset the 'recent' stats for the next _stat_interval

			stat_recent_min_cycle = MAX_INT;
			stat_recent_max_cycle = MIN_INT;
			stat_recent_min_error = MAX_INT;
			stat_recent_max_error = MIN_INT;
			stat_recent_min_power = 255;
			stat_recent_max_power = 0;
			stat_recent_min_left = MIN_INT;
			stat_recent_max_left = MAX_INT;
			stat_recent_min_right = MAX_INT;
			stat_recent_max_right = MIN_INT;
			stat_recent_min_target = MAX_INT;
			stat_recent_max_target = MIN_INT;
			stat_recent_min_total_ang_err = MAX_INT;
			stat_recent_max_total_ang_err = MIN_INT;

		}

		// 3. NTP vs ESP32 clock correction

		#if CLOCK_WITH_NTP
			else if (_ntp_interval &&
					clock_state == CLOCK_STATE_RUNNING &&
					num_beats - last_ntp >= _ntp_interval)
			{
				last_ntp = num_beats;
				onSyncNTP();
			}

		#endif	// CLOCK_WITH_NTP

	}	// things based on beat changing


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
    delay(100);
    LOGI("starting clockTask loop on core(%d)",xPortGetCoreID());
    delay(100);

    while (1)
    {
        vTaskDelay(0);		// 10 / portTICK_PERIOD_MS);
		uint32_t now = millis();
		static uint32_t last_sense = 0;

		// we have to allow time for other tasks to run
		// so we limit thi to 250 times a second, even though
		// the basic sensor reads only take about 1ms

		if (now - last_sense > 3)		// 3 ms works
		{
			last_sense = now;
			the_clock->run();
		}
	}
}
