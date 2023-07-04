// theClock3.cpp

#include "theClock3.h"
#include "clockAS5600.h"
#include "clockStats.h"
#include "clockPixels.h"
#include <myIOTLog.h>
	// grrrrr - getting unresolved references to time variables in theClock3.cpp
	// when I removed uneeded WebSockets.h from myIOTDevice.h.
	// But they get fixed when I needlessly include WiFi.h in myIOTTypes.h.
	// The only thing I could find was a slight difference in the order of includes
	// in the two compile statements, with WiFi coming before Preferences.h when it
	// worked, and after when it failed.  So I moved Preferences.h from myIOTValue.h
	// to the cpp file, but alas, it did not 'fix' this weird problem.
	// And it doesn't seem to help to includes that stuff first in this compile.

// motor setup

#define PWM_FREQUENCY	5000
#define PWM_RESOLUTION	8


// NOTES: USES A TASK FOR RUNNING THE CLOCK
//
// 		there is too much critical timing in the clock run() method to allow it
// 		allow it to run in loop() on the same core as the webservers, etc.
// 		When just run from the default core(1), along with the Webserver, it
// 		regularly misses crossings, etc, esp when the myIOT WebSocket goes on and off.
//
// 		So I made it a task on the unused core 0. However, usually tasks are limited
// 		to 10ms time slices, and so it did not work at first.
//
// 		I was able to get it working. First I changed the call to vTaskDelay(0)
// 		as I read that would yield without delay.  No joy.  I set the priority
// 		of the task down to 1, no joy.  Finally I disabled the Watch Dog Timer,
// 		(see code below) and that seems to work.
//
// STARTUP WITH NO NTP:
//
//		There is an issue that if the clock is booted at about the same time as the
//      wifi router, it *may* succesfully connect to the router before the router has
//      had a chance to connect to the internet, and myIOT will fail to set NTP time.
//      Then, if the clock is started, it could be on the cannonical 1970 clock, but
//      the wifi may subsequently get the correct NTP time, so the clock will think
//      it is running 40 years slow.
//
//      Although it is simple to merely restart the clock, the issue is that naive end
//      users will not notice this situation and the clock will run incorrectly.
//
//		Therefore clock sync LED will flash if it is more than 5 minutes off,
//      in which case the user should reboot the clock.
//
// POSSIBLE CHANGES:
//
// 		It may be preferable to adjust the RTC clock when it has drifted a certain amount.
// 		Same with NTP.
//
// 		It might be better to base the cycle time directly on the RTC and eliminate
// 		the notion of separately syncing to it.
//
// 		It still might be possible (and/or better) to have a single PID controller
// 		that directly corrects for the erorr.  However, that complicates the setup
// 		process that currently allows me to test power, and angles, separately
// 		from the 2nd 'time' PID controller.


//-----------------------------------------------------
// vars - working vars in this file; params in ino
//-----------------------------------------------------
// All times are stored and manipulated as UTC times so
// changes in daylight savings time do not affect the clock.
// This can be tested by changing timezones on a running clock.

int 	 theClock::m_clock_state = CLOCK_STATE_NONE;
uint32_t theClock::m_last_change;
 int32_t theClock::m_cur_cycle;
uint32_t theClock::m_last_cycle;
uint32_t theClock::m_num_beats;

uint32_t theClock::m_time_start;
uint32_t theClock::m_time_start_ms;
uint32_t theClock::m_time_zero;
uint32_t theClock::m_time_zero_ms;
uint32_t theClock::m_time_init;
uint32_t theClock::m_time_init_ms;

int32_t  theClock::m_total_millis_error;
int32_t  theClock::m_prev_millis_error;
float    theClock::m_pid_angle;

float 	 theClock::m_total_ang_error;
float 	 theClock::m_prev_ang_error;
int 	 theClock::m_pid_power;

int 	 theClock::m_sync_sign;
int 	 theClock::m_sync_millis;

uint32_t theClock::m_initial_pulse_time;
bool 	 theClock::m_push_motor;
uint32_t theClock::m_motor_start;
uint32_t theClock::m_motor_dur;

uint32_t theClock::m_last_beat;
uint32_t theClock::m_last_stats;
uint32_t theClock::m_last_sync;
uint32_t theClock::m_last_ntp;

bool 	 theClock::m_update_stats;
bool     theClock::m_force_pixels;
bool     theClock::m_setting_zero;


#if WITH_VOLTAGES

	float theClock::m_volts_5v;
	float theClock::m_volts_vbus;
	int theClock::m_low_power_mode;
	uint32_t theClock::m_low_power_time;

	float getVolts(const char * what, int pin, float calib)
	{
		#define NUM_VOLT_SAMPLES 4
			// alogorithm skips 1st sample

		uint32_t val = 0;
		for (int i=0; i<NUM_VOLT_SAMPLES+1; i++)
		{
			int j = analogRead(pin);
			if (i) val += j;
			delayMicroseconds(100);
		}
		float raw = (val / NUM_VOLT_SAMPLES);
		float pct = raw / 4096.0;
		float scaled = pct * 6.6;
		float full = scaled * calib;
		float volts = (full * 100.0) + 0.5;
		volts = floor(volts);
		volts /= 100.0;

		// LOGD("Volts(%s) val(%d) raw(%0.3f) pct=(%0.3f) scaled(%0.3f) full(%0.3f)  volts=%0.3f",what,val,raw,pct,scaled,full,volts);

		return volts;
	}


	void theClock::checkVoltage()
		// With equal resistors ~ 0.5, ao it should be good to 6.6V or so 0..4095 = 0-6.6V
		// We round to two places and only update when the value changes
	{
		static float last_5v;
		static float last_vbus;
		m_volts_5v = getVolts(" 5V ",PIN_VOLTS_5V,_volt_calib);
		m_volts_vbus = getVolts("VBUS",PIN_VOLTS_VBUS,_volt_calib);

		if (last_5v != m_volts_5v ||
			last_vbus != m_volts_vbus)
		{
			last_5v = m_volts_5v;
			last_vbus = m_volts_vbus;

			// note that the boolean m_low_power_mode is set on transitions
			// but we only actually go into low power mode if _low_power_enable is set

			if (!m_low_power_mode && m_volts_vbus < _volt_cutoff)
			{
				LOGU("low power detected");
				m_low_power_mode = VOLT_DETECT_LOW;
				if (_low_power_enable)
					m_low_power_time = millis();
			}
			else if (m_low_power_mode && m_volts_vbus >= _volt_restore)
			{
				LOGU("restore power detected");
				m_low_power_time = 0;
				if (m_low_power_mode == VOLT_MODE_LOW)
					setLowPowerMode(0);
				m_low_power_mode = VOLT_MODE_NORMAL;
			}

			setStatsPower(m_low_power_mode, m_volts_5v, m_volts_vbus);
			setString(ID_STAT_MSG0,getStatBufMain());
		}
	}


	void theClock::setLowPowerMode(bool low)
	{
		static bool save_wifi;
		static int save_brightness;

		LOGU("ENTERING %s POWER MODE",low?"LOW":"NORMAL");
		if (low)
		{
			m_low_power_mode = VOLT_MODE_LOW;

			// change variables in memory
			// so if reboot, the prefs will be re-read
			// but for the duration, wifi and brightness can change

			save_wifi = _device_wifi;
			save_brightness = _led_brightness;
			if (_led_brightness > 6)
			{
				_led_brightness = 6;
				onBrightnessChanged(NULL,_led_brightness);
			}
			if (_device_wifi)
			{
				_device_wifi = 0;
				onChangeWifi(NULL,false);
			}
		}
		else
		{
			_led_brightness = save_brightness;
			onBrightnessChanged(NULL,_led_brightness);
			if (save_wifi)
			{
				_device_wifi = 1;
				onChangeWifi(NULL,true);
				save_wifi = false;
			}
		}

		setStatsPower(m_low_power_mode, m_volts_5v, m_volts_vbus);
		setString(ID_STAT_MSG0,getStatBufMain());
	}

#endif	// WITH_VOLTAGES


//-----------------------------------------------
// utilities
//-----------------------------------------------

void theClock::setClockState(int state)
{
	LOGI("CLOCK_STATE(%d) %s",state,
		 state == CLOCK_STATE_RUNNING ? "RUNNING" :
		 state == CLOCK_STATE_STARTED ? "STARTED" :
		 state == CLOCK_STATE_START   ? "START" : "NONE");
	m_clock_state = state;
}


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

	clearPixels();
	setPixelsBrightness(64);
	showPixels();
	delay(500);

	// Note that the pixels are backwards (right to left)
	// and we want to go left to right

	for (int i=NUM_PIXELS-1; i>=0; i--)
	{
		setPixel(i,MY_LED_CYAN);
		showPixels();
		delay(300);
	}
	clearPixels();
	showPixels();
	delay(500);

	pinMode(PIN_BUTTON1,INPUT_PULLUP);
#if CLOCK_COMPILE_VERSION > 1
	pinMode(PIN_BUTTON2,INPUT_PULLUP);
#endif

// 1 and 2 use L293D with enable pins
// 3 uses MOSFET and just PIN_PWM

#if CLOCK_COMPILE_VERSION == 1
	ledcSetup(0, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcSetup(1, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(PIN_ENA, 0);
	ledcAttachPin(PIN_ENB, 0);
	ledcWrite(0,0);
	ledcWrite(1,0);
	pinMode(PIN_INA1,OUTPUT);
	pinMode(PIN_INA2,OUTPUT);
	pinMode(PIN_INB1,OUTPUT);
	pinMode(PIN_INB2,OUTPUT);
	digitalWrite(PIN_INA1,0);
	digitalWrite(PIN_INA2,0);
	digitalWrite(PIN_INB1,0);
	digitalWrite(PIN_INB2,0);
#elif CLOCK_COMPILE_VERSION == 2
	ledcSetup(0, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(PIN_EN, 0);
	ledcWrite(0,0);
	pinMode(PIN_IN1,OUTPUT);
	pinMode(PIN_IN2,OUTPUT);
	digitalWrite(PIN_IN1,0);
	digitalWrite(PIN_IN2,0);
#else
	ledcSetup(0, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(PIN_PWM, 0);
	ledcWrite(0,0);
#endif

	setPixel(PIXEL_MAIN,MY_LED_ORANGE);
	showPixels();
	delay(500);

	//--------------------------------------------------
	// START THE AS5600 before myIOTDevice::setup()
	//--------------------------------------------------
	// cuz it's better done when WIFI and all that stuff
	// has not yet been started ..

	startAS5600();

	//------------------------------
	// call myIOTDevice::setup()
	//------------------------------

	setPixel(PIXEL_MAIN,MY_LED_CYAN);
	showPixels();
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
		showPixels();
		setZeroAngle();
		delay(500);
	}
	LOGI("_zero_angle=%d == %0.3f degrees",_zero_angle,angleOf(_zero_angle));

	// debug_angle("in theClock::setup() before starting clockTask");

	setPixelsBrightness(_led_brightness);
	showPixels();

	// sample and set the voltage

	#if WITH_VOLTAGES
		pinMode(PIN_VOLTS_5V,INPUT);
		pinMode(PIN_VOLTS_VBUS,INPUT);
		if (_volt_interval)
			checkVoltage();
	#endif

	//------------------------------------------------
	// Start the clock task and away we go ...
	//-------------------------------------------------

	LOGI("starting clockTask");
	xTaskCreatePinnedToCore(clockTask,
		"clockTask",
		8192,           // task stack
		NULL,           // param
		10,  	        // note that the priority is higher than one
		NULL,           // returned task handle
		ESP32_CORE_OTHER);

	LOGU("theClock::setup() finished");
	clearPixels();
	showPixels();

	m_force_pixels = 1;

}	// theClock::setup()




void theClock::initMotor()
{
	motor(0,0);

	setClockState(CLOCK_STATE_NONE);
	m_last_change = 0;
	m_initial_pulse_time = 0;
	m_push_motor = 0;
	m_motor_start = 0;
	m_motor_dur = 0;

	m_time_zero = 0;
	m_time_zero_ms = 0;
}


void theClock::initStats(int how)
{
	m_update_stats = false;

	m_cur_cycle = 0;
	m_last_cycle = 0;
	m_num_beats = 0;
	m_time_init = 0;
	m_time_init_ms = 0;

	m_total_ang_error = 0;
	m_prev_ang_error = 0;

	m_total_millis_error = 0;
	m_prev_millis_error = 0;

	m_sync_sign = 0;
	m_sync_millis = 0;

	m_last_sync = 0;
	m_last_stats = 0;
	m_last_ntp = 0;

	initClockStats(how);

	if (how > INIT_STATS_RESTART)
		the_clock->setString(ID_STAT_MSG0,"");

	the_clock->setString(ID_STAT_MSG1,"");
	the_clock->setString(ID_STAT_MSG2,"");
	the_clock->setString(ID_STAT_MSG3,"");
	the_clock->setString(ID_STAT_MSG4,"");
	the_clock->setString(ID_STAT_MSG5,"");
	the_clock->setString(ID_STAT_MSG6,"");

}


void theClock::clearStats()
	// ONLY called from the UI!!
{
	LOGU("STATISTICS CLEARED");

	initStats(INIT_STATS_ALL);
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	m_time_start = tv_now.tv_sec;
	m_time_start_ms = tv_now.tv_usec / 1000L;
	m_update_stats = true;
}


void theClock::stopClock()
{
	the_clock->setBool(ID_START_SYNC,0);
	LOGU("stopClock()");
	initMotor();
}


void theClock::onStartSyncChanged(const myIOTValue *desc, bool val)
{
	LOGU("onStartSyncChanged(%d)",val);

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

	if (val && (
		_clock_mode < CLOCK_MODE_MIN_MAX ||
		m_clock_state != CLOCK_STATE_NONE ))
	{
		LOGW("Attempt to call onStartClockSynchronized() in mode(%d) and state(%d)!!",_clock_mode,m_clock_state);
		the_clock->setBool(ID_START_SYNC,0);
	}
}


void theClock::startClock(bool restart /*=0*/)
{
	LOGU("startClock(%d)",restart);

	initMotor();
	initAS5600(_zero_angle);
	initStats(restart ? INIT_STATS_RESTART : INIT_STATS_START_CLOCK);

	if (_clock_mode == CLOCK_MODE_SENSOR_TEST)
	{
		m_pid_power = 0;
		m_pid_angle = 0;
		setClockState(CLOCK_STATE_RUNNING);
	}
	else
	{
		m_pid_power = _power_pid;
		m_pid_angle =
			_clock_mode == CLOCK_MODE_ANGLE_MIN ? _angle_min :
			_clock_mode == CLOCK_MODE_ANGLE_MAX ? _angle_max :
			_clock_mode == CLOCK_MODE_MIN_MAX ? _angle_min :
			_angle_start;

		if (!restart)
		{
			struct timeval tv_now;
			gettimeofday(&tv_now, NULL);
			m_time_start = tv_now.tv_sec;
			m_time_start_ms = tv_now.tv_usec / 1000L;
		}

		motor(-1,_power_start);
		m_initial_pulse_time = m_last_change = millis();
		setClockState(CLOCK_STATE_START);
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
	m_force_pixels = 1;
}

void theClock::onBrightnessChanged(const myIOTValue *desc, uint32_t val)
{
	LOGU("onBrightnessChanged(%d)",val);
	m_force_pixels = 1;
}


void theClock::setZeroAngle()
{
	LOGI("Setting AS5600 zero angle...",sizeof(int));

	// As with initializing the AS5600, the clock will just flat out NOT run
	// if the zero angle cannot be set. We will try upto 5 times to set it.
	// The zero angle MUST be between 45 and 235 degrees.

	the_clock->setInt(ID_ZERO_ANGLE,0);
    the_clock->setFloat(ID_ZERO_ANGLE_F,0.0);
	delay(100);

	// Setting the zero angle is important enough to warrant taking
	// over all the LEDs, so we set the static global "m_setting_zero"
	// to prevent loop() from modifying the pixels

	m_setting_zero = 1;
	for (int i=0; i<NUM_PIXELS; i++)
		setPixel(i,MY_LED_WHITE);
	showPixels();

	// All pixels set to white.
	// On success we will show the center pixel (PIXEL_ACCURACY) green for 3 seconds
	// On failure we will show it as red and retry upto 3 times.
	// Subsequently, if the zero was not set, the PIXEL_STATE led will flash red

	int count = 0;
	int zero = getAS5600Raw();
	float zero_f =  angleOf(zero);
	bool ok = (zero_f >= MIN_ZERO_ANGLE) && (zero_f <= MAX_ZERO_ANGLE);

	while (!ok && count++ < 5)
	{
		LOGE("COULD NOT SET AS5600 zero angle=%d  %0.3f", zero,zero_f);
		setPixel(2,MY_LED_RED);
		showPixels();
		delay(500);
		zero = getAS5600Raw();
		zero_f =  angleOf(zero);
		ok = (zero_f >= MIN_ZERO_ANGLE) && (zero_f <= MAX_ZERO_ANGLE);
	}

	// we set the values even though they might be incorrect
	// so that loop() can start flashing the state pixel red

	if (ok)
	{
		LOGU("Set AS5600 zero angle=%d  %0.3f", zero,zero_f);
		the_clock->setInt(ID_ZERO_ANGLE,zero);
		the_clock->setFloat(ID_ZERO_ANGLE_F,zero_f);
		setPixel(2,MY_LED_GREEN);
		showPixels();
		delay(3000);
	}

	// turn off the loop supressor and force the pixels to be redisplayed

	m_setting_zero = 0;
	m_force_pixels = 1;
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
	if (_clock_mode < CLOCK_MODE_MIN_MAX ||
		m_clock_state != CLOCK_STATE_RUNNING)
	{
		LOGE("Attempt to call onSyncRTC() with mode(%d) in state(%d)!!",_clock_mode,m_clock_state);
		return;
	}

	// we no longer care about m_total_millis_error

	m_total_millis_error = 0;

	// m_time_init and m_time_init_ms was the 0th zero crossing
	// m_num_beats is the number of full ticks that have taken place since m_time_init
	// and the convention is that the zero crossing should take place at 500ms,
	// 'beat_time' == the time the clock is showing plus that 500 ms.

	int32_t beat_time = m_time_init + m_num_beats;
	int32_t beat_time_ms = 500;

	// then we have the 'actual' time (of the most recent zero crossing)
	// if the 'actual_time' is greater than the 'beat' time, then the clock needs to speed up.
	// m_sync_millis has the semantic of a 'correction', that is, a positive value will speed
	// the clock up, and a negative value will slow it down.

	m_sync_millis = timeDeltaMS(m_time_zero,m_time_zero_ms,beat_time,beat_time_ms);

	// therefore to speed up, we need to subtract beat_time from m_time_zero,
	// as we do in the above call, to result in a positive m_sync_millis to speed the clock up.

	m_sync_sign =
		m_sync_millis < 0 ? - 1 :
		m_sync_millis > 0 ? 1 : 0;

	if (m_sync_sign)
	{
		LOGU("onSyncRTC() beats=%d beat_time=%d.%03d m_time_zero=%d.%03d  m_sync_millis=%d sign=%d",
			m_num_beats,
			beat_time,
			beat_time_ms,
			m_time_zero,
			m_time_zero_ms,
			m_sync_millis,
			m_sync_sign);
	}
	else
		LOGU("onSyncRTC()");

	const char *msg = getStatBufRTC(m_sync_millis);
	the_clock->setString(ID_STAT_MSG5,msg);
}



#if CLOCK_WITH_NTP
	void theClock::onSyncNTP()
	{
		// these stats are only approximate
		// as the final actual time will be determined
		// by ntp sync processes beyond our control

		int32_t ntp_ms;
		int32_t ntp_secs;
		int32_t delta_ms = 0;
		bool    failed = 0;

		if (!the_clock->getBool(ID_DEVICE_WIFI) ||
			!(the_clock->getConnectStatus() & IOT_CONNECT_STA))
		{
			LOGE("Attempt to sync to NTP when WIFI=%d and connect_status=0x%02x",
				 the_clock->getBool(ID_DEVICE_WIFI),
				 the_clock->getConnectStatus());

		}
		if (!getNtpTime(&ntp_secs,&ntp_ms))
		{
			failed = 1;
			LOGE("getNtpTime() failed!!");
		}
		else
		{
			struct timeval tv_now;
			gettimeofday(&tv_now, NULL);
			int32_t now_secs = tv_now.tv_sec;
            int32_t now_ms = tv_now.tv_usec / 1000L;
			delta_ms = timeDeltaMS(ntp_secs,ntp_ms,now_secs,now_ms);
				// The delta is negative if our clock is faster, or positive if our clock is slower,
				// in keeping with the convention that these are CORRECTIONS to be made, so if our
				// clock is faster, and delta is negative, we will increase the cycle time to make
				// up for the difference.
			if (delta_ms)
			{
				LOGU("onSyncNTP() now(%d.%03d) ntp(%d.%03d) delta=%d",
					now_secs,
					now_ms,
					ntp_secs,
					ntp_ms,
					delta_ms);
				syncNTPTime();
			}
			else
				LOGU("onSyncNTP() no change");
		}

		const char *msg = getStatBufNTP(delta_ms, failed);
		the_clock->setString(ID_STAT_MSG6,msg);
	}

#endif	// CLOCK_WITH_NTP




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
