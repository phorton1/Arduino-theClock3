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
// So I made it a task on the unused core 0. However, usually tasks are limited
// to 10ms time slices, and so it did not work at first.
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

#define AS4500_THRESHOLD    4		// 5 required for change == about 0.5 degrees

static int as5600_cur = 0;			// bits, not degrees
static int as5600_side = 0;			// trying zero crossing
static int as5600_direction = 0;	// on detection of THRESHOLD AS4500_THRESHOLD

static int as5600_min = 0;			// min and max are assigned on direction changes
static int as5600_max = 0;
static int as5600_temp_min = 10240;	// and the temps are reset to zero for next go round
static int as5600_temp_max = -10240;

// angles are calculated from integers when they change

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
}


//----------------------------
// pixel(s)
//----------------------------

static Adafruit_NeoPixel pixels(NUM_PIXELS,PIN_LEDS);

void setPixel(int num, uint32_t color)
{
	pixels.setPixelColor(num,color);
}


uint32_t scalePixel(int amt, int scale, uint32_t color0, uint32_t color1, uint32_t color2)
{
	// LOGU("scalePixel(%d,%d,0x%06x,0x%06x,0x%06x)",amt,scale,color0,color1,color2);

	float fdif = amt;
	fdif /= ((float)scale);
	if (fdif < -1) fdif = -1;
	else if (fdif > 1) fdif = 1;
	// LOGU("   fdif=%0.3f",fdif);

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
		// LOGU("base=0x%02x  val=0x%02x   byte=0x%02x",base,val,byte);
		retval |= (byte << 8 * i);
	}
	// LOGU("    retval=0x%08x",retval);
	return retval;
}




//--------------------------------------------
// vars
//--------------------------------------------

#define CLOCK_STATE_NONE    	0
#define CLOCK_STATE_STATS   	1
#define CLOCK_STATE_START		2
#define CLOCK_STATE_STARTED		3
#define CLOCK_STATE_RUNNING		4


static int clock_state = 0;
static bool start_sync = 0;			// doing a synchronized start

static uint32_t last_change = 0;	// millis of last noticible pendulum movement
static int32_t 	cur_cycle = 0;		// millis in this 'cycle' (forward zero crossing)
static int32_t 	last_cycle = 0;		// millis at previous forward zero crossing
static uint32_t num_beats = 0;		// number of beats (while clock_started && !initial_pulse_time)

static uint32_t time_start = 0;
static uint32_t time_start_ms = 0;
static uint32_t time_zero = 0;		// ESP32 RTC time at zero crossing
static uint32_t time_zero_ms = 0;	// with milliseconds (from microseconds)
static uint32_t time_init = 0;
static uint32_t time_init_ms = 0;

static int32_t  total_millis_error = 0;	// important cumulative number of millis total error
static int32_t  prev_millis_error = 0;
static float pid_angle = 0;				// angle determined by second PID controller or other parameters

static float total_ang_error = 0;	// accumluated degrees of error (for "I")
static float prev_ang_error = 0;	// the previous error (for "D")
static int pid_power = 0;			// power adjusted by pid algorithm

// SYNC

static int sync_sign = 0;
static int sync_millis = 0;

static uint32_t num_sync_checks = 0;
static uint32_t num_sync_changes = 0;
static  int32_t last_sync_change = 0;
static  int32_t total_sync_changes = 0;
static uint32_t total_sync_changes_abs = 0;
static uint32_t num_ntp_checks = 0;
static uint32_t num_ntp_fails = 0;
static uint32_t num_ntp_changes = 0;
static  int32_t last_ntp_change = 0;
static  int32_t total_ntp_changes = 0;
static uint32_t total_ntp_changes_abs = 0;



// CONTROL

volatile bool the_semaphore = 0;
	// semaphore between showPixels() and as5600.readAngle()
	// dont read the angle while writing pixels or vice-versa!

static uint32_t initial_pulse_time = 0;	// time at which we started initial clock starting pulse (push)
static bool push_motor = 0;			// push the pendulum next time after it leaves deadzone (determined at zero crossing)
static uint32_t motor_start = 0;
static uint32_t motor_dur = 0;

static uint32_t last_beat = 0xffffffff;
static uint32_t last_stats = 0;
static uint32_t last_sync = 0;
static uint32_t last_ntp = 0;

static char msg_buf[512];
	// generic buffer for loop() related messages


// Statistics

static bool update_stats = false;

static uint32_t  stat_num_bad_reads;
static uint32_t  stat_num_restarts;

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

	pixels.setPixelColor(PIXEL_MAIN,MY_LED_ORANGE);
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

	if (_zero_angle == 0)
	{
		pixels.setPixelColor(PIXEL_MAIN,MY_LED_PURPLE);
		pixels.show();
		setZeroAngle();
		delay(500);
	}
	LOGU("_zero_angle=%d == %0.3f degrees",_zero_angle,angle(_zero_angle));

	// debug_angle("in theClock::setup() before starting clockTask");

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
	initial_pulse_time = 0;	// time at which we started initial clock starting pulse (push)
	push_motor = 0;			// push the pendulum next time after it leaves deadzone (determined at zero crossing)
	motor_start = 0;
	motor_dur = 0;

	cur_cycle = 0;
	last_cycle = 0;

	num_beats = 0;
	time_zero = 0;
	time_zero_ms = 0;
	time_init = 0;
	time_init_ms = 0;
}


void theClock::initStats()
{
	update_stats = false;

	// nothing reset in here should *really* affect
	// the running of the clock ... these are only
	// statistics and cumulative errors ... although
	// doing this in the middle of a sync will stop
	// the sync.

	total_ang_error = 0;
	prev_ang_error = 0;

	total_millis_error = 0;
	prev_millis_error = 0;

	sync_sign = 0;
	sync_millis = 0;

	num_sync_checks = 0;
	num_sync_changes = 0;
	last_sync_change = 0;
	total_sync_changes = 0;
	total_sync_changes_abs = 0;
	num_ntp_checks = 0;
	num_ntp_fails = 0;
	num_ntp_changes = 0;
	last_ntp_change = 0;
	total_ntp_changes = 0;
	total_ntp_changes_abs = 0;

	last_beat = 0xffffffff;
	last_sync = 0;
	last_stats = 0;
	last_ntp = 0;

	stat_num_bad_reads = 0;
	stat_num_restarts = 0;

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
	initStats();
	update_stats = true;
}


void theClock::stopClock()
{
	start_sync = 0;
	LOGU("stopClock()");
	initMotor();
}


void theClock::startClock()
{
	LOGU("startClock()");

	initMotor();
	initAS5600();
	initStats();

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

		struct timeval tv_now;
		gettimeofday(&tv_now, NULL);
		time_start = tv_now.tv_sec;
		time_start_ms = tv_now.tv_usec / 1000L;

		motor(-1,_power_start);
		initial_pulse_time = last_change = millis();
		clock_state = CLOCK_STATE_START;
	}
}


void theClock::onStartClockSynchronized()
{
	LOGU("StartClockSynchronized()");
	// The idea here is that the user has put the second hand on zero
	// with the other hands ready to go at the next 0 seconds crossing,
	// and they press the button somewhat before that zero crossing
	// and that we will do what is necessary to get the clock aligned
	// back to that moment.

	// We will give a starting after crossing the next '59' seconds, plus some delay.
	// The difficulty is that we don't know when the actual first tick takes place.
	// We will assume it moves to the right on the pulse and will swing to the left
	// approximately 1/2 second after we issue the pulse.

	// We are using left crossing to determine cycle time.

	// It depends on the state of the hardware ... the clock may be predisposed
	// so that the pawls grabbing starting on the actual impulse with a forward tick,
	// or on the the next back swing, or some time later ... leading us to experiments
	// with the optical mouse sensor to detect the wheel movements.

	if (clock_state != CLOCK_STATE_NONE )
		LOGE("Attempt to call onStartClockSynchronized() while it's already running!");
	else
		start_sync = true;
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


#include <sys/time.h>
void theClock::onDiddleClock(const myIOTValue *desc, int val)
{
	LOGU("onDiddleClock %d",val);
	int32_t a_time = time(NULL) + val;
	timeval e_time = {a_time, 0};
	settimeofday((const timeval*)&e_time, 0);
}




void theClock::onSyncRTC()
{
	if (clock_state != CLOCK_STATE_RUNNING )
	{
		LOGE("Attempt to call onSyncRTC() while clock is not running!");
		return;
	}

	int32_t dif_ms = time_zero_ms - 500;
		// we want the tock to align with 500 ms no matter what

	int32_t run_secs = time_zero;		// number of seconds the clock has been running
	run_secs -= time_init;				// is most recent zero crossing - initial zero crossing
	int32_t dif_secs = num_beats - run_secs;
		// the difference in whole seconds is the number of beats
		// minus the amount of actual time the clock has been running.
		// so it is larger than 0 if we are beating fast or less than
		// zero if we are beeting slow.

	sync_millis = (dif_secs * 1000) + dif_ms;
	sync_sign =
		sync_millis < 0 ? - 1 :
		sync_millis > 0 ? 1 : 0;

	num_sync_checks++;
	if (sync_sign)
	{
		num_sync_changes++;
		last_sync_change = sync_millis;
		total_sync_changes += sync_millis;
		total_sync_changes_abs += abs(sync_millis);
		LOGU("onSyncRTC(%d/%d) run_secs=%d  beats=%d  diff=%d  ms=%d  sign=%d  chg=%d",
			num_sync_changes,
			num_sync_checks,
			run_secs,
			num_beats,
			dif_secs,
			dif_ms,
			sync_sign,
			sync_millis);
	}
	else
		LOGU("onSyncRTC(%d/%d) no change",num_sync_changes,num_sync_checks);

	sprintf(msg_buf,"SYNC(%d/%d) last(%d) total(%d) abs(%d)",
		num_sync_changes,
		num_sync_checks,
		last_sync_change,
		total_sync_changes,
		total_sync_changes_abs);
	the_clock->setString(ID_STAT_MSG5,msg_buf);
}



#if CLOCK_WITH_NTP
	void theClock::onSyncNTP()
	{
		if (clock_state != CLOCK_STATE_RUNNING )
		{
			LOGE("Attempt call onSyncRTC() while clock is not running!");
			return;
		}

		// these stats are only approximate
		// as the final actual time will be determined
		// by ntp sync processes beyond our control

		num_ntp_checks++;
		int32_t ntp_time = getNtpTime();
		if (!ntp_time)
		{
			num_ntp_fails++;
			LOGE("getNtpTime() failed!!");
		}
		else
		{
			int32_t now = time(NULL);
			int32_t secs = ntp_time - now;
			if (secs)
			{
				int32_t chg = secs * 1000;
				last_ntp_change = chg;
				num_ntp_changes++;
				total_ntp_changes += chg;
				total_ntp_changes_abs += abs(chg);
				LOGU("onSyncNTP(%d/%d)  chg=%d",
					num_ntp_changes,
					num_ntp_checks,
					chg);

				syncNTPTime();
			}
			else
				LOGU("onSyncNTP(%d/%d) no change",num_ntp_changes,num_ntp_checks);
		}

		sprintf(msg_buf,"NTP(%d/%d) fails(%d) last(%d) total(%d) abs(%d)",
			num_ntp_changes,
			num_ntp_checks,
			num_ntp_fails,
			last_ntp_change,
			total_ntp_changes,
			total_ntp_changes_abs);
		the_clock->setString(ID_STAT_MSG6,msg_buf);
	}

#endif	// CLOCK_WITH_NTP



//===================================================================
// run()
//===================================================================

float theClock::getPidAngle()
{
	float this_p = cur_cycle - 1000;
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
	pid_angle = new_angle;

	return pid_angle;
}

int theClock::getPidPower(float avg_angle)
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


void theClock::run()
	// This is called every 4 ms ...
	// Try not to call setXXX stuff while clock is running.
{
	// start the clock if start_sync at at a minute crossing,
	// or if the running variable turned on and not running

	if (clock_state == CLOCK_STATE_NONE)
	{
		if (start_sync)
		{
			if (time(NULL) % 60 == 59)
			{
				LOGU("starting synchronized delay=%d",_start_delay);
				the_clock->setBool(ID_RUNNING,1);	    // set the UI bool
				if (_start_delay)
					delay(_start_delay);				// do it about 1/2 second early
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

	if (_clock_mode > CLOCK_MODE_SENSOR_TEST &&
		clock_state >= CLOCK_STATE_STARTED &&
		now - last_change > _restart_millis)
	{
		LOGW("RESTARTING CLOCK!!");
		stat_num_restarts++;
		startClock();
		return;
	}


	//-------------------------------------------------
	// AS5600
	//-------------------------------------------------
	// Read, but don't necessariy use the as5600 angle
	// Note that sometimes, particularly when first starting, I get bogus readings here.
	// I think they *may* be related to neopixels disabling interrupts
	// or something going on in Wifi etc.
	// Using a semaphore around readAngle() and showPixels() did not seem to help.
	// Current solution is compare the angle to some arbitrary value (14 degrees)
	// and bail on this time through the loop if it's larger than that.

	#define MAX_ANGLE  14.0

	volatile int count = 0;
	while (the_semaphore) {count++;}
	the_semaphore = 1;

	int raw = as5600.readAngle();

	the_semaphore = 0;

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
			LOGD("as5600=%d  angle=0.3f",cur,as5600_cur_angle);
		}

		// detect zero crossing

		else if ((cur < 0 && as5600_side >= 0) ||
			     (cur > 0 && as5600_side <= 0))
		{
			as5600_side = cur < 0 ? -1 : 1;
			push_motor = true;

			// get full cycle time
			// and add it to the accumulated error if running
			// we use the left crossing, which is the tick to 1/2 second
			// so that when we synchronize to 500ms, the tock will fall on the
			// exact second

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
					// added to the sync_millisuntil it changes sign,
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
				}
			}
		}

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
		// Switch to RUNNING if needed

		if (clock_state == CLOCK_STATE_STARTED &&
			as5600_max_angle >= _running_angle &&
			as5600_min_angle <= -_running_angle &&
			abs(total_ang_error) < _running_error)
		{
			clock_state = CLOCK_STATE_RUNNING;
			LOGU("Clock running!");
			clearStats();
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
				// FLIP the angle in MIN_MAX mode

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

			float avg_angle = (abs(as5600_min_angle) + abs(as5600_max_angle)) / 2;

			int use_power =
				_clock_mode == CLOCK_MODE_POWER_MIN ? _power_min :
				_clock_mode == CLOCK_MODE_POWER_MAX ? _power_max :
				getPidPower(avg_angle);

			motor_start = now;
			motor_dur =_dur_pulse;
			motor(-1,use_power);

			if (use_power > stat_max_power)
				stat_max_power = use_power;
			if (use_power < stat_min_power)
				stat_min_power = use_power;

			if (_plot_values == PLOT_OFF)
			{
				LOGI("%-6s(%-2d) %-4d %3.3f/%3.3f=%3.3f  target=%3.3f  accum=%3.3f  power=%d  err=%d  sync=%d",
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
		}
	}

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

	//--------------------------------------
	// PIXELS
	//--------------------------------------

	uint32_t now = millis();
	static uint32_t last_pixels = 0;
	if (now - last_pixels > 50)
	{
		last_pixels = now;

		// MAIN_PIXEL is green if connected as STATION,
		// purple if AP, yellow if ALL, and red if NONE

		uint32_t new_pixels[NUM_PIXELS];
		memset(new_pixels,0,NUM_PIXELS * sizeof(uint32_t));

		iotConnectStatus_t status = getConnectStatus();
		new_pixels[PIXEL_MAIN] =
			status == IOT_CONNECT_ALL ? MY_LED_YELLOW :
			status == IOT_CONNECT_AP ? MY_LED_PURPLE :
			status == IOT_CONNECT_STA ? MY_LED_GREEN :
			/* status == IOT_CONNECT_NONE ? */ MY_LED_RED;

		new_pixels[PIXEL_STATE] =
			clock_state == CLOCK_STATE_RUNNING ? MY_LED_GREEN :
			clock_state == CLOCK_STATE_STARTED ? MY_LED_MAGENTA :
			clock_state == CLOCK_STATE_START ? MY_LED_WHITE :
			start_sync ? MY_LED_YELLOW :
			clock_state == CLOCK_STATE_STATS ? MY_LED_ORANGE :
			MY_LED_BLACK;

		// accuracy and cycle move from green to red/blue
		// as they diverge by 2 * _min_max_ms

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


		// set pixels and show if changed

		bool show_pixels = 0;
		static uint32_t old_pixels[NUM_PIXELS];
		for (int i=0; i<=NUM_PIXELS; i++)
		{
			if (old_pixels[i] != new_pixels[i])
			{
				old_pixels[i] = new_pixels[i];
				setPixel(i,new_pixels[i]);
				show_pixels = 1;
			}
		}
		if (show_pixels)
		{
			volatile int count = 0;
			while (the_semaphore) {count++;}
			the_semaphore = 1;
			pixels.show();
			the_semaphore = 0;
		}
	}


	//------------------------------------------
	// THINGS BASED ON THE BEAT CHANGING
	//------------------------------------------
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

			sprintf(msg_buf,"%s %02d:%02d:%02d  == %d SECS %d BEATS",
				(clock_state == CLOCK_STATE_RUNNING ?"RUNNING":"STARTING"),
				hours,
				mins,
				secs,
				full_secs,
				num_beats);
			setString(ID_STAT_MSG1,msg_buf);

			if (num_beats)	// to prevent annoying -32767's on 0th cycle
			{
				sprintf(msg_buf,"num_bad(%d) restarts(%d)",
						stat_num_bad_reads,
						stat_num_restarts);
				setString(ID_STAT_MSG2,msg_buf);

				sprintf(msg_buf,"power min(%d) max(%d)  angle left(%0.3f,%0.3f) to right(%0.3f,%0.3f)",
					stat_min_power,
					stat_max_power,
					stat_max_left,
					stat_min_left,
					stat_min_right,
					stat_max_right);
				setString(ID_STAT_MSG3,msg_buf);

				sprintf(msg_buf,"cycle(%d,%d)  error(%d,%d)",
					stat_min_cycle,
					stat_max_cycle,
					stat_min_error,
					stat_max_error);
				setString(ID_STAT_MSG4,msg_buf);
			}
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
	// debug_angle("in clockTask");

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
