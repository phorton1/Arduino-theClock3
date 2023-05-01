// theClockRun.cpp

#include "theClock3.h"
#include "clockAS5600.h"
#include "clockStats.h"
#include "clockPixels.h"
#include <myIOTLog.h>

//----------------------------
// motor
//----------------------------

static int motor_state = 0;

void theClock::motor(int state, int power)
{
	motor_state = state;
	int use_power = state ? power : 0;
#if USEV1_PINS
	ledcWrite(0, use_power);
	digitalWrite(PIN_INA1,state == 1  ? 1 : 0);
	digitalWrite(PIN_INA2,state == -1 ? 1 : 0);
	ledcWrite(1, use_power);
	digitalWrite(PIN_INB1,state == 1  ? 1 : 0);
	digitalWrite(PIN_INB2,state == -1 ? 1 : 0);
#else
	ledcWrite(0, use_power);
	digitalWrite(PIN_IN1,state == 1  ? 1 : 0);
	digitalWrite(PIN_IN2,state == -1 ? 1 : 0);
#endif
}



//===================================================================
// PID CONTROLLERS
//===================================================================

float theClock::getPidAngle()  // APID parameters
{
	// this_p == current ms error
	// this_i == total ms error (including m_sync_millis)
	// this_d == delta ms error this cycle

	float this_p = m_cur_cycle;
	this_p -= 1000.0;
	float this_i = m_total_millis_error + m_sync_millis;
	float this_d = m_prev_millis_error - this_p;;
	m_prev_millis_error = this_p;

	this_p = this_p / 1000;
	this_i = this_i / 1000;
	this_d = this_d / 1000;

	float factor = 1 + (_apid_P * this_p) + (_apid_I * this_i) + (_apid_D * this_d);
	float new_angle = m_pid_angle * factor;
	if (new_angle > _angle_max) new_angle = _angle_max;
	if (new_angle  < _angle_min) new_angle = _angle_min;

	// LOGD("getPidAngle pid(%0.3f) cur(%d) P(%0.3f) I(%0.3f) D(%0.3f) factor(%0.3f) new(%0.3f)",
	// 	m_pid_angle,
	// 	m_cur_cycle,
	// 	this_p,
	// 	this_i,
	// 	this_d,
	// 	factor,
	// 	new_angle);

	m_pid_angle = new_angle;
	updateStatsPidAngle(m_pid_angle);
	return m_pid_angle;
}


int theClock::getPidPower(float avg_angle)	// PID parameters
{
	// this_p == current angular error
	// this_i == total running angular error
	// this_d == delta angular error this cycle

	float this_p = m_pid_angle - avg_angle;
	m_total_ang_error += this_p;
	float this_i = m_total_ang_error;
	float this_d = m_prev_ang_error - this_p;;
	m_prev_ang_error = this_p;

	this_p = this_p / 100;
	this_i = this_i / 100;
	this_d = this_d / 100;

	float factor = 1 + (_pid_P * this_p) + (_pid_I * this_i) + (_pid_D * this_d);
	float new_power = m_pid_power * factor;
	if (new_power > _power_max) new_power = _power_max;
	if (new_power  < _power_min) new_power = _power_min;
	m_pid_power = new_power;

	return m_pid_power;
}




//===================================================================
// run()
//===================================================================

void theClock::run()
	// This is called every 4 ms or so ...
	// Try not to call setXXX stuff while clock is running.
{
	// start the clock if m_start_sync and at a minute crossing,
	// or if the running variable turned on and not running

	if (m_clock_state == CLOCK_STATE_NONE)
	{
		if (_start_sync)
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
	if (m_clock_state == CLOCK_STATE_START)
	{
		if (now - m_initial_pulse_time > _dur_start)
		{
			m_initial_pulse_time = 0;
			motor(0,0);

			// non motor-pid modes start off as 'running',
			// where as pid modes start off as 'started'
			// and only go to running later

			if (_clock_mode < CLOCK_MODE_ANGLE_START)
				setClockState(CLOCK_STATE_RUNNING);
			else
				setClockState(CLOCK_STATE_STARTED);
		}
		else
		{
			return;
		}
	}

	// Restart if necessary

	int res_millis = now - m_last_change;
	if (_clock_mode > CLOCK_MODE_SENSOR_TEST &&
		m_clock_state >= CLOCK_STATE_STARTED &&
		res_millis > _restart_millis)
	{
		LOGW("RESTARTING CLOCK!! now=%d m_last_change=%d  diff=%d  constant=%d",now,m_last_change,res_millis,_restart_millis);
		incStatRestarts();
		startClock(1);
		return;
	}


	// The rest of this is only done if we are running

	if (m_clock_state < CLOCK_STATE_STARTED)
		return;

	//-------------------------------------------------
	// CYCLE
	//-------------------------------------------------

	// get the current RTC clock time, and
	// if CLOCK_STATE_STATS show the realtime clock every second

	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	if (_clock_mode == CLOCK_MODE_SENSOR_TEST)
	{
		static int32_t last_seconds = 0;
		if (last_seconds != tv_now.tv_sec)
		{
			last_seconds = tv_now.tv_sec;
			uint32_t ms = tv_now.tv_usec / 1000L;
			LOGU("tick  seconds=%d  ms=%d",tv_now.tv_sec,ms);

		}
	}

	// get angle changes

	int cur = 0;
	int rslt = getAS5600Angle(_zero_angle,&cur);
	if (rslt == -1)
		return;

	if (rslt)	// significant movement
	{
		m_last_change = now;

		if (_clock_mode == CLOCK_MODE_SENSOR_TEST && _plot_values == PLOT_OFF)
		{
			LOGU("as5600=%-4d  angle=%0.3f",cur,as5600_cur_angle);
		}

		// started ... detect zero crossing

		if (getAS5600ZeroCrossing())
		{
			if (_clock_mode > CLOCK_MODE_SENSOR_TEST)
				m_push_motor = true;

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
				if (m_last_cycle)
					m_cur_cycle = now - m_last_cycle;
				m_last_cycle = now;

				if (m_cur_cycle)
				{
					// best guess of the actual RTC time at zero crossing

					// struct timeval tv_now;
					// gettimeofday(&tv_now, NULL);
					m_time_zero = tv_now.tv_sec;
                    m_time_zero_ms = tv_now.tv_usec / 1000L;

					int err = m_cur_cycle;
					err -= 1000;

					// if m_sync_sign, we are in a sync and the err is
					// added to the m_sync_millis until it changes sign,
					// at which point the sync is "turned off" and the
					// remaining error falls through to the 'm_total_millis_error'

					if (m_sync_sign)
					{
						m_sync_millis += err;
						int new_sign =
							m_sync_millis > 0 ? 1 :
							m_sync_millis < 0 ? -1 : 0;

						if (new_sign != m_sync_sign)
						{
							LOGU("SYNC_DONE - remainder=%d",m_sync_millis);
							err = m_sync_millis;
							m_sync_sign = 0;
							m_sync_millis = 0;

							// to ease the transition, we also set the total_ang_error to zero
							// in case the clock has built up a large one doing a big correction

							m_total_ang_error = 0;
						}

						// if the sync is not turned off, it ate all the error millis

						else
						{
							err = 0;
						}
					}

					// might be zero contribution while syncing ...

					m_total_millis_error += err;

					// we are either on the 0th beat (starting the clock)
					// or we start with the first beat one full cycle later
					// and subsequently increment it

					if (!m_time_init)
					{
						m_update_stats = true;
						m_time_init = m_time_zero;	// time(NULL);
						m_time_init_ms = m_time_zero_ms;
					}
					else
					{
						if (!m_num_beats)
							m_update_stats = true;
						m_num_beats++;
					}

					updateStatsCycle(m_cur_cycle, m_total_millis_error);

				}	// cycle has been established
			}	// left zero crossing
		}	// zero crossing


		//------------------------------------------------
		// state machine
		//------------------------------------------------
		// Switch to RUNNING if needed

		if (m_clock_state == CLOCK_STATE_STARTED &&		// only true on PID modes
			as5600_max_angle >= _running_angle &&
			as5600_min_angle <= -_running_angle &&
			abs(m_total_ang_error) < _running_error)
		{
			setClockState(CLOCK_STATE_RUNNING);
			// clearStats();
		}

		// do initial sync if running and we have a good set of times

		if (_start_sync &&
			m_clock_state == CLOCK_STATE_RUNNING &&
			m_time_init && m_cur_cycle )
		{
			the_clock->setBool(ID_START_SYNC,0);
			m_last_sync = m_num_beats;
			onSyncRTC();
		}

		// Push the motor if asked to and out of the dead zone

		if (m_push_motor && abs(as5600_cur_angle) > _dead_zone)
		{
			m_push_motor = false;

			if (m_clock_state == CLOCK_STATE_RUNNING)
			{
				// in full PID_MODE get the pid angle or
				// possibly change the target angle in MIN_MAX mode

				if (_clock_mode == CLOCK_MODE_PID)
				{
					m_pid_angle = getPidAngle();
				}
				else if (_clock_mode == CLOCK_MODE_MIN_MAX)
				{
					if (m_pid_angle == _angle_min &&
						m_total_millis_error + m_sync_millis > _min_max_ms)
					{
						m_pid_angle = _angle_max;
					}
					else if (m_pid_angle == _angle_max &&
						m_total_millis_error + m_sync_millis < -_min_max_ms)
					{
						m_pid_angle = _angle_min;
					}
				}
			}

			// calculate the power to use
			// weird v1.3 with sensor just uses right (power) swing

			#if USEV1_BEHAVIOR
				float avg_angle = as5600_max_angle;
			#else
				float avg_angle = getAS560AverageAngle();
			#endif
			int use_power =
				_clock_mode == CLOCK_MODE_POWER_MIN ? _power_min :
				_clock_mode == CLOCK_MODE_POWER_MAX ? _power_max :
				getPidPower(avg_angle);

			// PULSE THE MOTOR!

			m_motor_start = now;
			m_motor_dur =_dur_pulse;
			motor(-1,use_power);

			updateStatsPowerAngle(use_power, m_total_ang_error);

			if (_plot_values == PLOT_OFF)
			{
				LOGD("%-6s(%-2d) %-4d %3.3f/%3.3f=%3.3f  target=%3.3f  accum=%3.3f  power=%d  err=%d  sync=%d",
					 m_sync_sign ? "SYNC" : m_clock_state == CLOCK_STATE_RUNNING ? "run" : "start",
					 as5600_direction,
					 m_cur_cycle,
					 as5600_min_angle,
					 as5600_max_angle,
					 avg_angle,
					 m_pid_angle,
					 m_total_ang_error,
					 use_power,
					 m_total_millis_error,
					 m_sync_millis);
			}

		}	// push motor
	}	// angle threshold change exceeded

	// Stop the motor as needed based on duration,

	if (m_motor_start && now - m_motor_start > m_motor_dur)
	{
		m_motor_start = 0;
		m_motor_dur = 0;
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

		Serial.print(m_total_millis_error * 20);
		Serial.print(",");

		Serial.print(motor_state * 400);
		Serial.println(",1000,-1000");
	}

}	// theClock::run()
