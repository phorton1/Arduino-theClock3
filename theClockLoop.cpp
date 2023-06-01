// theClockLoop.cpp
//
// BUTTON functions
//
//   left button (1)
//
//			short press
//				if clock is running, stop it
//              else if 1st press start sync
//              else if 2nd press start immediate
//			medium press = turn wifi on/off
//			long press = factory reset and reboot
//
//   right button (2)
//
//			short press = cycle pixel brightness
//          medium press = zero pendulum
//
// PIXEL semantics
//    STARTUP in theClock,cpp::setup()
//          all leds lit CYAN from left to right
//			left pixel ORANGE before call to startAS5600
//          left pixel CYAN before call to myIOT::setup()
//          left pixel PURPLE before possible call to setZeroAngle
//          ... brightness is set
//          all pixels turned off at end of setup()
//
//    left == WIFI state
//			BLUE   == WIFI turned off
//          RED    == WIFI on, but no AP or STA ?!?
//			GREEN  == connected as STA
//          PURPLE == acting as AP
//          ORANGE == transient AP_STA mode
//    second == CLOCK state
//          BLACK  == nothing
//          WHITE  == waiting for start sync
//          YELLOW == initial impulse
//          CYAN   == started, but not yet "running within tolerance"
//          GREEN  == "running"
//          ORANGE == running is special "sensor test" mode
//          FLASHING RED == could not init AS5600
//          FLASHING ORANGE == zero angle out of range
//    third == CLOCK accuracy
//          RED(ish)  == running faster upto _error_range
//          GREEN     == running accurately
//          BLUE(ish) == running slower upto _error_range
//    fourth == CYCLE accuracy
//          RED(ish)  == running faster upto _cycle_range
//          GREEN     == running accurately
//          BLUE(ish) == running slower upto _cycle_range
//    fifth == SYNC_STATE
//          RED(ish)  == syncing from too fast (_cycle_range)
//          GREEN     == sync done
//          BLUE(ish) == syncing from too slow (_cycle_range)
//          FLASHING RED OR BLUE == more than 5 minutes out of sync
//              may just need a restart, but could indicate mechanical problems



#include "theClock3.h"
#include "clockStats.h"
#include "clockPixels.h"
#include "clockAS5600.h"
#include <myIOTLog.h>



#define MEDIUM_PRESS   3000
#define LONG_PRESS     8000
#define SECOND_PRESS   2000


static uint32_t button_start[2];



//--------------------------------------
// doButtons
//--------------------------------------
// We assume they only press one button at a time in the pixels

void theClock::doButtons()
{
	uint32_t now = millis();
	static uint32_t button_check = 0;

	if (now - button_check > 33)	// 30 times per second
	{
		button_check = now;
		for (int button=0; button<NUM_BUTTONS; button++)
		{
			uint32_t start = button_start[button];
			bool val = !digitalRead(button?PIN_BUTTON2:PIN_BUTTON1);

			if (val && !start)	// initial press
			{
				button_start[button] = now;
			}
			else if (start && !val)	// button up
			{
				uint32_t dur = now - start;
				button_start[button] = 0;

				if (button)		// BUTTON2 (right) up
				{
					// button2 long press NOT USED
					// button2 medium press = set zero angle

					if (dur >= MEDIUM_PRESS)
					{
						setZeroAngle();
					}

					// button2 short press = change pixel brightness
					// use smaller increments near zero

					else
					{
						if (_led_brightness == 254)
							_led_brightness = 0;
						else if (_led_brightness < 40)
							_led_brightness += 8;
						else
							_led_brightness += 25;
						if (_led_brightness > 254)
							_led_brightness = 254;
						setInt(ID_LED_BRIGHTNESS,_led_brightness);
					}
				}
				else	// BUTTON1 (left) up
				{
					if (dur >= LONG_PRESS)		// button1 long press = factory reset
					{
						factoryReset();
					}
					else if (dur >= MEDIUM_PRESS)	// button1 medium press = turn wifi off/on
					{
						setBool(ID_DEVICE_WIFI,!getBool(ID_DEVICE_WIFI));
					}

					// button1 short press
					//    if running (or sync_starting) first_press = stop clock, second press ignored
					//    otherwise first press = start_synchroniced, and second press = start running

					else
					{
						bool is_first = 1;
						static uint32_t first_press = 0;
						if (start - SECOND_PRESS > first_press)
							first_press = now;
						else
							is_first = 0;

						if (is_first)		// first press
						{
							LOGD("first press");
							if (m_clock_state || _start_sync)	// while running or sync started
							{
								// if the clock is running (m_clock_state)
								// we use the parameter to turn it off, otherwise
								// we call stop clock directly.

								if (m_clock_state)
									setBool(ID_RUNNING,0);
								else
									stopClock();
							}
							else
							{
								// otherwise, we start synchronized
								// start_sync is also a flag used on 2nd press

								setBool(ID_START_SYNC,1);
							}
						}
						else	// 2nd press
						{
							LOGD("second press");
							if (_start_sync)
							{
								// second press while starting synchronized
								// turns of start_sync and just starts the clock with parameter
								// and does nothing in a stop cycle

								setBool(ID_START_SYNC,0);
								setBool(ID_RUNNING,1);
							}
						}

					}	// short press
				}	// 2nd button
			}	// button up
		}	// for each button
	}	// every 33 ms
}	// doButtons()





//--------------------------------------
// doPxiels()
//--------------------------------------


void theClock::doPixels()
{
	// redisplay the pixels 10 times a second
	// but only if they changed

	uint32_t now = millis();
	static uint32_t last_pixels = 0;
	if (now - last_pixels > 100)
	{
		last_pixels = now;
		uint32_t new_pixels[NUM_PIXELS];
		memset(new_pixels,0,NUM_PIXELS * sizeof(uint32_t));

		// invariant 'flash' stste as needed

		static uint32_t flash_time;
		static bool flash_on;
		if (now - flash_time > 300)
		{
			flash_time = now;
			flash_on = !flash_on;
		}

		// The right most LED shows WHITE-MAGENTA-PURPLE if a button is pressed
		// We assume only one button is pressed at a time

		uint32_t button_time = button_start[0] ?
			button_start[0] : button_start[1];
		if (button_time)
		{
			uint32_t button_dur = now - button_time;
			new_pixels[PIXEL_SYNC] =
				button_dur >= LONG_PRESS ? MY_LED_MAGENTA :
				button_dur >= MEDIUM_PRESS ? MY_LED_PURPLE :
				MY_LED_WHITE;
		}

		// Leftmost pixel shows WIFI status

		iotConnectStatus_t status = getConnectStatus();
		bool wifi_on = getBool(ID_DEVICE_WIFI);
		new_pixels[PIXEL_MAIN] =
			status == IOT_CONNECT_ALL ? MY_LED_ORANGE :
			status == IOT_CONNECT_AP  ? MY_LED_PURPLE :
			status == IOT_CONNECT_STA ? MY_LED_GREEN :
			wifi_on ? MY_LED_RED :
			MY_LED_BLUE;

		// Second pixel shows the CLOCK state
		// if the zero is out of range we will toggle a bit every 300 ms for flashing

		bool zero_bad =
			(_zero_angle_f < MIN_ZERO_ANGLE) ||
			(_zero_angle_f > MAX_ZERO_ANGLE);


		new_pixels[PIXEL_STATE] =
			!as5600_connected ? (flash_on ? MY_LED_RED : MY_LED_BLACK) :
			zero_bad ? (flash_on ? MY_LED_ORANGE : MY_LED_BLACK) :
			m_clock_state == CLOCK_STATE_RUNNING ?
				_clock_mode == CLOCK_MODE_SENSOR_TEST ? MY_LED_ORANGE : MY_LED_GREEN :
			m_clock_state == CLOCK_STATE_STARTED ? MY_LED_CYAN :
			m_clock_state == CLOCK_STATE_START ?   MY_LED_YELLOW :
			_start_sync ? MY_LED_WHITE :
			MY_LED_BLACK;

		// Third pixel shows overall clock accuracy compared to _error_range
		// accuracy and cycle move from green to red/blue
		// as they diverge by +/- parameterized values

		if (m_clock_state >= CLOCK_STATE_START)
		{
			new_pixels[PIXEL_ACCURACY] =
				m_total_millis_error >=  _error_range 	? MY_LED_BLUE      :
				m_total_millis_error <= -_error_range 	? MY_LED_RED   :
				scalePixel3(m_total_millis_error,_error_range,
					MY_LED_RED,
					MY_LED_GREEN,
					MY_LED_BLUE);

			// Fourth pixel shows instantaneous error (per swing)
			// compared to _cycle_range parameter

			int dif = m_cur_cycle - 1000;
			if (dif >= _cycle_range)
				new_pixels[PIXEL_CYCLE] = MY_LED_BLUE;
			else if (dif <= -_cycle_range)
				new_pixels[PIXEL_CYCLE] = MY_LED_RED;
			else
				new_pixels[PIXEL_CYCLE] = scalePixel3(dif,_cycle_range,
					MY_LED_RED,
					MY_LED_GREEN,
					MY_LED_BLUE);

			// Fifth pixel shows SYNC state compared to _cycle_range
			// if a button is not pressed.  If it is extreme (more than 5 minutes)
			// it likely means they started the clock without NTP and then got NTP
			// so the clock thinks it is running 40 years slow and needs to be restarted,
			// but it could also mean other serious problems ... so we flash the LED

			if (!button_time)
			{
				#define EXTREME_SYNC   300000L
				bool extreme = m_sync_sign && (
					m_sync_millis > EXTREME_SYNC ||
					m_sync_millis < -EXTREME_SYNC);

				new_pixels[PIXEL_SYNC] =
					!m_sync_sign ? MY_LED_BLACK :
					extreme && !flash_on ? MY_LED_BLACK :
					scalePixel3(m_sync_millis,_cycle_range,
						MY_LED_RED,
						MY_LED_GREEN,
						MY_LED_BLUE);
			}
		}

		// set pixels and show if forced or changed

		static uint32_t old_pixels[NUM_PIXELS];
		if (m_force_pixels)
		{
			memset(old_pixels,0,NUM_PIXELS * sizeof(uint32_t));
			clearPixels();
			setPixelsBrightness(_led_brightness + 1);
		}

		// once a pixel is changed the rest are forced

		for (int i=0; i<=NUM_PIXELS-1; i++)
		{
			if (m_force_pixels ||
				old_pixels[i] != new_pixels[i])
			{
				old_pixels[i] = new_pixels[i];
				setPixel(i,new_pixels[i]);
				m_force_pixels = 1;
			}
		}

		// The use of pixels.canShow() may be superflous, but
		// there can be upto a 300 us delay at the top of showPixels()
		// BEFORE the interrupts are disabled so I check it before
		// calling showPixels(). JIC we can't show the pixels,
		// m_force_pixels remains in effect.

		if (m_force_pixels && pixelsCanShow())
		{
			m_force_pixels = 0;
			WAIT_SEMAPHORE();
			showPixels();
			RELEASE_SEMAPHORE();
		}
	}	// now - 100 > last_pixels
}


//------------------------------------------------------------------
// loop
//------------------------------------------------------------------

// virtual
void theClock::loop()	// override
{
	myIOTDevice::loop();

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

	if (_clock_mode > CLOCK_MODE_SENSOR_TEST &&
		m_clock_state >= CLOCK_STATE_STARTED &&
		m_last_beat != m_num_beats)
	{
		m_last_beat = m_num_beats;

		// 1. Error correction vs ESP32 clock if properly RUNNING

		if (!m_sync_sign &&
			_sync_interval &&
			m_clock_state == CLOCK_STATE_RUNNING &&
			m_num_beats - m_last_sync >= _sync_interval)
		{
			m_last_sync = m_num_beats;
			onSyncRTC();
		}

		// 2. Sbow statistics for most clock modes

		else if (_stat_interval &&
				 _plot_values == PLOT_OFF &&
				 m_last_stats != m_num_beats && (
				 m_update_stats ||
				 m_num_beats % _stat_interval == 0))
		{
			m_last_stats = m_num_beats;
			showStats();
		}

		// 3. NTP vs ESP32 clock correction

		#if CLOCK_WITH_NTP
			else if (_ntp_interval &&
					 getBool(ID_DEVICE_WIFI) &&
					 getConnectStatus() & IOT_CONNECT_STA &&
					 m_clock_state == CLOCK_STATE_RUNNING &&
					 m_num_beats - m_last_ntp >= _ntp_interval)
			{
				m_last_ntp = m_num_beats;
				onSyncNTP();
			}

		#endif	// CLOCK_WITH_NTP

	}	// things based on beat changing


	// do the pixels and buttons
	// supressed while setZeroAngle is being called

	if (!m_setting_zero)
	{
		doPixels();
		doButtons();
	}

	// do stats for CLOCK_MODE_SENSOR_TEST

	int32_t time_now = time(NULL);
	if (_clock_mode == CLOCK_MODE_SENSOR_TEST &&
		m_clock_state == CLOCK_STATE_RUNNING && (
		m_update_stats ||
		time_now - m_last_beat >= _stat_interval))
	{
		m_last_beat = time_now;		// overuse m_last_beat to hold last time for sensor test
		showStats();
	}

#if WITH_VOLT_CHECK

	static int32_t last_volt_check = 0;
	if (_volt_interval &&
		time_now - last_volt_check > _volt_interval)
	{
		last_volt_check = time_now;
		checkVoltage();
	}

	#define LOW_POWER_DELAY    10000

	if (m_low_power_time && millis() - m_low_power_time >= LOW_POWER_DELAY)
	{
		m_low_power_time = 0;
		setActualLowPowerMode(1);
	}

#endif


}	// theClock::loop()



void theClock::showStats()
{
	m_update_stats = false;
	LOGI("--> stats");

	// show the value of the RTC at the last zero crossing

	initStatBuf();
	formatTimeToStatBuf("TIME_START",m_time_start,m_time_start_ms,true);
	formatTimeToStatBuf("TIME_INIT",m_time_init,m_time_init_ms,true);
	formatTimeToStatBuf("CUR_TIME",m_time_zero,m_time_zero_ms,false);
	setString(ID_STAT_MSG1,getStatBuf());

	uint32_t full_secs = m_time_init ? m_time_zero - m_time_init : 0;
	uint32_t secs = full_secs;
	uint32_t mins = secs / 60;
	uint32_t hours = mins / 60;
	secs = secs - mins * 60;
	mins = mins - hours * 60;

	char *msg_buf = (char *) getStatBuf();
	sprintf(msg_buf,"%s %02d:%02d:%02d  == %d SECS %d BEATS",
		(m_clock_state == CLOCK_STATE_RUNNING ?"RUNNING":"STARTING"),
		hours,
		mins,
		secs,
		full_secs,
		m_num_beats);
	setString(ID_STAT_MSG2,msg_buf);

	setString(ID_STAT_MSG0,getStatBufMain());

	if (m_cur_cycle)	// to prevent annoying -32767's on 0th cycle
	{
		setString(ID_STAT_MSG3,getStatBufAll());
		setString(ID_STAT_MSG4,getStatBufRecent());
	}

	// reset the 'recent' stats for the next _stat_interval

	initRecentStats();

}
