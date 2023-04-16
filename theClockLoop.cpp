// theClockLoop.cpp

#include "theClock3.h"
#include "clockStats.h"
#include "clockPixels.h"
#include <myIOTLog.h>


//--------------------------------------
// doButtons
//--------------------------------------

void theClock::doButtons()
{

}


//--------------------------------------
// doPxiels()
//--------------------------------------

void theClock::doPixels()
{
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
				m_clock_state == CLOCK_STATE_RUNNING ? MY_LED_GREEN :
				m_clock_state == CLOCK_STATE_STARTED ? MY_LED_MAGENTA :
				m_clock_state == CLOCK_STATE_START ?   MY_LED_YELLOW :
				m_start_sync ? MY_LED_WHITE :
				m_clock_state == CLOCK_STATE_STATS ? MY_LED_ORANGE :
				MY_LED_BLACK;

			// accuracy and cycle move from green to red/blue
			// as they diverge by +/- _min_max_ms

			if (m_clock_state >= CLOCK_STATE_START)
			{
				new_pixels[PIXEL_ACCURACY] =
					m_total_millis_error >=  _min_max_ms 	? MY_LED_BLUE      :
					m_total_millis_error <= -_min_max_ms 	? MY_LED_RED   :
					scalePixel(m_total_millis_error,_min_max_ms,
						MY_LED_RED,
						MY_LED_GREEN,
						MY_LED_BLUE);

				int dif = m_cur_cycle - 1000;
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
						!m_sync_sign ? MY_LED_BLACK :
						scalePixel(m_sync_millis,_min_max_ms,
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

	// show the realtime clock

	if (m_clock_state == CLOCK_STATE_STATS)
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

	if (m_clock_state >= CLOCK_STATE_STARTED &&
		m_last_beat != m_num_beats)
	{
		m_last_beat = m_num_beats;

		// 1. Error correction vs ESP32 clock

		if (!m_sync_sign &&
			_sync_interval &&
			m_clock_state == CLOCK_STATE_RUNNING &&
			m_num_beats - m_last_sync >= _sync_interval)
		{
			m_last_sync = m_num_beats;
			onSyncRTC();
		}

		// 2. Sbow statistics

		else if (_stat_interval &&
				 _plot_values == PLOT_OFF &&
				 m_last_stats != m_num_beats && (
				 m_update_stats ||
				 m_num_beats % _stat_interval == 0))
		{
			m_update_stats = false;
			m_last_stats = m_num_beats;
			LOGI("--> stats");

			// show the value of the RTC at the last zero crossing

			initStatBuf();
			formatTimeToStatBuf("TIME_START",m_time_start,m_time_start_ms,true);
			formatTimeToStatBuf("TIME_INIT",m_time_init,m_time_init_ms,true);
			formatTimeToStatBuf("CUR_TIME",m_time_zero,m_time_zero_ms,false);
			setString(ID_STAT_MSG0,getStatBuf());

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
			setString(ID_STAT_MSG1,msg_buf);

			setString(ID_STAT_MSG2,getStatBufBadReadsAndRestarts());

			if (m_cur_cycle)	// to prevent annoying -32767's on 0th cycle
			{
				setString(ID_STAT_MSG3,getStatBufAll());
				setString(ID_STAT_MSG4,getStatBufRecent());
			}

			// reset the 'recent' stats for the next _stat_interval

			initRecentStats();

		}

		// 3. NTP vs ESP32 clock correction

		#if CLOCK_WITH_NTP
			else if (_ntp_interval &&
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

}	// theClock::loop()
