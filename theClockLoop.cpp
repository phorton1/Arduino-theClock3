// theClockLoop.cpp
//
// Button functions
//
//   left button (1)
//
//			short press
//
//				if clock is running, stop it
//              else if 1st press start sync
//              else if 2nd press start immediate
//
//			medium press = turn wifi on/off
//			long press = factory reset and reboot
//
//   right button (2)
//
//			short press = cycle pixel brightness
//          medium press = change pixel mode
//          long press = zero pendulum


#include "theClock3.h"
#include "clockStats.h"
#include "clockPixels.h"
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
		#ifdef USEV1_PINS
			for (int button=0; button<1; button++)
		#else
			for (int button=0; button<2; button++)
		#endif
		{
			uint32_t start = button_start[button];
			#if USEV1_PINS
				bool val = !digitalRead(PIN_BUTTON1);
			#else
				bool val = !digitalRead(button?PIN_BUTTON2:PIN_BUTTON1);
			#endif

			if (val && !start)	// initial press
			{
				button_start[button] = now;
			}
			else if (start && !val)	// button up
			{
				uint32_t dur = now - start;
				button_start[button] = 0;

				if (button)
				{
					// button2 long press = zero angle

					if (dur >= LONG_PRESS)
					{
						setZeroAngle();
					}

					// button2 medium press = change pixel mode
					// between modes

					else if (dur >= MEDIUM_PRESS)
					{
						if (_pixel_mode == PIXEL_MODE_DIAG)
							_pixel_mode = PIXEL_MODE_TIME;
						else
							_pixel_mode = PIXEL_MODE_DIAG;
						setEnum(ID_PIXEL_MODE,_pixel_mode);
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
				else
				{
					if (dur >= LONG_PRESS)		// button1 long press = factory reset
					{
						factoryReset();
					}
					else if (dur >= MEDIUM_PRESS)	// button1 medium press = turn wifi off/on
					{
						setBool(ID_DEVICE_WIFI,!getBool(ID_DEVICE_WIFI));
					}

					// button1 short press =
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
// EXPLANATION OF PIXEL_TIME_MODE (Pixels generally need work for !as5600_connected && m_low_power_mode:
//
// The 0th pixel should be black unless there is a problem and
// should flash if lit to differentiate it from the other pixels.
//
//       yellow       = restarted
//       red/blue     = (serious) more than 5 * error_range ms off total time
//       cyan/purple  = (not serious) more than error_range ms off total time
//       green        = lost station connection (if STA_SSID specified and not IOT_CONNECT_STA)
//
//  flashing yellow menas the time is not reliable and the clock must be manually restarted
//  note that we have to add sync_millis to total_millis_error in the if statement.
//
//  The remaining four pixels are set as follows:
//
//  0,12 	= BBBB
//
//  12:30   = CBBB			4:30    = YGGG			8:30    = MRRR
//
//  1 		= GBBB      	5 		= RGGG          9		= BRRR
//  1:30    = GMBB      	5:30    = RYGG          9:30    = BMRR
//  2 		= GGBB      	6 		= RRGG          10      = BBRR
//  2:30    = GGCB      	6:30    = RRYG          10:30   = BBMR
//  3 		= GGGB      	7 		= RRRG          11		= BBBR
//  3:30    = GGGC      	7:30    = RRRY          11:30   = BBBM
//  4 		= GGGG      	8       = RRRR          12      = BBBB

void localTimeToPixels(uint32_t *pix)
{
	// get hour and minute
	// remember that pixels are backwards and we are using the
	// first four in reverse order, sheesh, so time_pixel0 is
	// actual pixel #3, hence we subtract i from PIXEL_STATE

	time_t t = time(NULL);
    struct tm *ts = localtime(&t);	// &m_time_zero);
	int hour = ts->tm_hour % 12;
	int num = hour % 4;
		// number of full pixels to be set with the time pixel
		// the next one is the scale pixel
	int mins = ts->tm_min;

	float pct = mins;
	pct /= 60;

	// LOGD("timeZeroToPixels called hour=%d mins=%d num=%d pct=%0.3f",hour,mins,num,pct);

	uint32_t fill_pixel = 0;
	uint32_t time_pixel = 0;

	if (hour >= 8)
	{
		fill_pixel = MY_LED_RED;
		time_pixel = MY_LED_BLUE;
	}
	else if (hour >= 4)
	{
		fill_pixel = MY_LED_GREEN;
		time_pixel = MY_LED_RED;
	}
	else
	{
		fill_pixel = MY_LED_BLUE;
		time_pixel = MY_LED_GREEN;
	}

	// fill all pixels

	for (int i=0; i<4; i++)
	#if REVERSE_PIXELS
		pix[PIXEL_STATE-i] = fill_pixel;
	#else
		pix[i + 1] = fill_pixel;
	#endif

	// set full hour pixels

	for (int i=0; i<num; i++)
	#if REVERSE_PIXELS
		pix[PIXEL_STATE-i] = time_pixel;
	#else
		pix[i + 1] = fill_pixel;
	#endif

	// do the scale pixel

	#if REVERSE_PIXELS
		pix[PIXEL_STATE - num]
	#else
		pix[num + 1]
	#endif
		= scalePixel(pct,fill_pixel,time_pixel);
}




void theClock::doPixels()
{
	static uint32_t pixel_flash_time = 0;
	static bool 	pixel_flash_on = 0;
	static uint32_t time_error_pixel = 0;

	// In normal operation, pixels only change on each beat,
	// but system pixels can change at any time.

	uint32_t now = millis();
	static uint32_t last_pixels = 0;
	if (now - last_pixels > 100)
	{
		last_pixels = now;
		uint32_t new_pixels[NUM_PIXELS];
		memset(new_pixels,0,NUM_PIXELS * sizeof(uint32_t));

		// PRESSED BUTTONS GO INTO THE LAST PIXEL (PIXEL SYNC)
		// in either case we show pixels for buttons
		// take the earlier start, if any

		uint32_t early_button = button_start[0];
		if (!early_button || (button_start[1] && button_start[1] > early_button))
			early_button = button_start[1];

		if (early_button)
		{
			uint32_t button_dur = now - early_button;
			new_pixels[PIXEL_SYNC] =
				button_dur >= LONG_PRESS ? MY_LED_MAGENTA :
				button_dur >= MEDIUM_PRESS ? MY_LED_PURPLE :
				MY_LED_WHITE;
		}


		if (_pixel_mode == PIXEL_MODE_TIME)
		{
			if (m_clock_state >= CLOCK_STATE_STARTED)
			{
				// handle the error pixel

				uint32_t err_pixel =
					getNumRestarts() ? MY_LED_YELLOW :
					m_total_millis_error + m_sync_millis > 5 * _error_range 	? MY_LED_BLUE  :
					m_total_millis_error + m_sync_millis < -5 * _error_range 	? MY_LED_RED   :
					m_total_millis_error + m_sync_millis > _error_range 		? MY_LED_CYAN  :
					m_total_millis_error + m_sync_millis < -_error_range 		? MY_LED_PURPLE :
						getBool(ID_DEVICE_WIFI) &&
						getString(ID_STA_SSID) != "" &&
						!(getConnectStatus() & IOT_CONNECT_STA) ? MY_LED_GREEN :
					MY_LED_BLACK;

				if (time_error_pixel != err_pixel)
				{
					pixel_flash_on = 0;
					pixel_flash_time = 0;
					LOGI("TIME_ERROR_PIXEL changing from 0x%06x to 0x%06x",time_error_pixel,err_pixel);
					time_error_pixel = err_pixel;
				}

				if (time_error_pixel != MY_LED_BLACK)
				{
					if (now - pixel_flash_time > 500)
					{
						pixel_flash_time = now;
						pixel_flash_on = !pixel_flash_on;
					}

					new_pixels[PIXEL_MAIN] = pixel_flash_on ? time_error_pixel : MY_LED_BLACK;
				}


				else
					new_pixels[PIXEL_MAIN] = MY_LED_BLACK;


				// handle the four time pixels via
				// simple call to localTime()

				if (!early_button)
					localTimeToPixels(new_pixels);

			}

			// show start sync even in PIXEL_TIME mode

			else if (_start_sync && !(m_clock_state >= CLOCK_STATE_STARTED))
				new_pixels[PIXEL_MAIN] = MY_LED_WHITE;
		}

		else if (_pixel_mode == PIXEL_MODE_DIAG)
		{
			pixel_flash_on = 0;
			pixel_flash_time = 0;
			time_error_pixel = 0;

			iotConnectStatus_t status = getConnectStatus();
			bool wifi_on = getBool(ID_DEVICE_WIFI);
			new_pixels[PIXEL_MAIN] =
				status == IOT_CONNECT_ALL ? MY_LED_ORANGE :
				status == IOT_CONNECT_AP  ? MY_LED_PURPLE :
				status == IOT_CONNECT_STA ? MY_LED_GREEN :
				wifi_on ? MY_LED_RED :
				MY_LED_BLUE;

			new_pixels[PIXEL_STATE] =
				m_clock_state == CLOCK_STATE_RUNNING ?
					_clock_mode == CLOCK_MODE_SENSOR_TEST ? MY_LED_ORANGE : MY_LED_GREEN :
				m_clock_state == CLOCK_STATE_STARTED ? MY_LED_CYAN :
				m_clock_state == CLOCK_STATE_START ?   MY_LED_YELLOW :
				_start_sync ? MY_LED_WHITE :

				MY_LED_BLACK;

			// accuracy and cycle move from green to red/blue
			// as they diverge by +/- _min_max_ms

			if (m_clock_state >= CLOCK_STATE_START)
			{
				new_pixels[PIXEL_ACCURACY] =
					m_total_millis_error >=  _error_range 	? MY_LED_BLUE      :
					m_total_millis_error <= -_error_range 	? MY_LED_RED   :
					scalePixel3(m_total_millis_error,_error_range,
						MY_LED_RED,
						MY_LED_GREEN,
						MY_LED_BLUE);

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

				if (!early_button)
					new_pixels[PIXEL_SYNC] =
						!m_sync_sign ? MY_LED_BLACK :
						scalePixel3(m_sync_millis,_cycle_range,
							MY_LED_RED,
							MY_LED_GREEN,
							MY_LED_BLUE);
			}
		}	// PIXEL_MODE_DIAG

		// set pixels and show if changed

		static uint32_t old_pixels[NUM_PIXELS];
		if (clock_show_pixels == 2)
		{
			memset(old_pixels,0,NUM_PIXELS * sizeof(uint32_t));
			clearPixels();
			setPixelsBrightness(_led_brightness + 1);
		}

		for (int i=0; i<=NUM_PIXELS-1; i++)
		{
			if (clock_show_pixels == 2 || old_pixels[i] != new_pixels[i])
			{
				old_pixels[i] = new_pixels[i];
				setPixel(i,new_pixels[i]);
				if (!clock_show_pixels)
					clock_show_pixels = 1;
			}
		}

		// The use of pixels.canShow() may be superflous, but
		// there can be upto a 300 us delay at the top of showPixels()
		// BEFORE the interrupts are disabled so I check it before
		// calling showPixels()

		if (clock_show_pixels && pixelsCanShow())
		{
			clock_show_pixels = 0;
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

	doPixels();
	doButtons();

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