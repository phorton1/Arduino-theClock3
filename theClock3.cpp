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
// TODO:
//
// 		Implement PIXEL_TIME mode
//
//		There is an issue that if the clock is booted and started without internet
//      whether it is turned off, or it could not at first connect, and set to 'running'
//      and then connects, it will think it is running 40 years slow and will run as
//      fast as possible forever.
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

int 	 theClock::m_clock_state;
bool 	 theClock::m_start_sync;
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

	clearPixels();
	setPixelsBrightness(64);

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
#if !USEV1_PINS
	pinMode(PIN_BUTTON2,INPUT_PULLUP);
#endif

#if USEV1_PINS
	#define PIN_ENA		27
	#define PIN_INA1	25
	#define PIN_INA2	26
	#define PIN_ENB		4
	#define PIN_INB1	17
	#define PIN_INB2	5

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
#else
	ledcSetup(0, PWM_FREQUENCY, PWM_RESOLUTION);
	ledcAttachPin(PIN_EN, 0);
	ledcWrite(0,0);
	pinMode(PIN_IN1,OUTPUT);
	pinMode(PIN_IN2,OUTPUT);
	digitalWrite(PIN_IN1,0);
	digitalWrite(PIN_IN2,0);
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
	clearPixels();
	showPixels();

	clock_show_pixels = 2;

}	// theClock::setup()




void theClock::initMotor()
{
	motor(0,0);

	m_clock_state = 0;
	m_last_change = 0;
	m_initial_pulse_time = 0;
	m_push_motor = 0;
	m_motor_start = 0;
	m_motor_dur = 0;

	m_time_zero = 0;
	m_time_zero_ms = 0;
}


void theClock::initStats(bool restart)
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

	initClockStats(restart);

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
	m_time_start = tv_now.tv_sec;
	m_time_start_ms = tv_now.tv_usec / 1000L;
	m_update_stats = true;
}


void theClock::stopClock()
{
	m_start_sync = 0;
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

	if (m_clock_state != CLOCK_STATE_NONE )
		LOGE("Attempt to call onStartClockSynchronized() while it's already running!");
	else
		m_start_sync = true;
}


void theClock::startClock(bool restart /*=0*/)
{
	LOGU("startClock(%d)",restart);

	initMotor();
	initAS5600(_zero_angle);
	initStats(restart);

	if (_clock_mode == CLOCK_MODE_SENSOR_TEST)
	{
		m_pid_power = 0;
		m_pid_angle = 0;
		m_clock_state = CLOCK_STATE_STATS;
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
		m_clock_state = CLOCK_STATE_START;
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
	clock_show_pixels = 2;
}

void theClock::onBrightnessChanged(const myIOTValue *desc, uint32_t val)
{
	LOGU("onBrightnessChanged(%d)",val);
	clock_show_pixels = 2;
}


void theClock::setZeroAngle()
{
	LOGI("Setting AS5600 zero angle ...");
	int zero = getAS5600Raw();
	float zero_f =  angleOf(zero);
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
	if (m_clock_state != CLOCK_STATE_RUNNING )
	{
		LOGE("Attempt to call onSyncRTC() while clock is not running!");
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
