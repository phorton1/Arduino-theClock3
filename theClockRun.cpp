// theClockRun.cpp

#include "theClock3.h"
#include "clockAS5600.h"
#include "clockStats.h"
#include "clockPixels.h"
#include <myIOTLog.h>

//----------------------------
// motor
//----------------------------

static int cur_motor_power = 0;


void theClock::motor(int power)
{
	cur_motor_power = power;
#if CLOCK_COMPILE_VERSION == 2
	ledcWrite(0, power);
	digitalWrite(PIN_IN1,0);
	digitalWrite(PIN_IN2,power ? 1 : 0);
#else	// MOSFET DRIVER
	ledcWrite(0, power);
#endif
}


#if CLOCK_COMPILE_VERSION == 1
	void theClock::spring(int power)
		// ledc channel 1 is PWM2, the "spring"
	{
		ledcWrite(1, power);
	}
#endif



//===================================================================
// PID CONTROLLERS
//===================================================================

#if CLOCK_COMPILE_VERSION == 1

	int theClock::getSpringPower()  // APID parameters
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

		// we allow the PID controller to completely turn off the spring
		// but, since it multiplies for its results, we use an arbitrary
		// factor cutoff of 1, and if so, restore the spring_power to a
		// minimum of 10.

		if (factor > 1 && m_spring_power == 0)
			m_spring_power = 10;

		float new_power = m_spring_power;

		new_power *= factor;
		if (new_power > 255) new_power = 255;
		if (new_power  < 0) new_power =  0;

		// LOGD("getSpringPower start(%d) cycle(%d) P(%0.3f) I(%0.3f) D(%0.3f) factor(%0.3f) new(%0.3f)",
		// 	m_spring_power,
		// 	m_cur_cycle,
		// 	this_p,
		// 	this_i,
		// 	this_d,
		// 	factor,
		// 	new_power);

		m_spring_power = new_power;
		updateStatsPid2(m_spring_power);
		return m_spring_power;
	}

#else

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
		updateStatsPid2(m_pid_angle);
		return m_pid_angle;
	}
#endif



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

extern void timeAddMS(int32_t *secs, int32_t *ms, int32_t ms_delta);
	// in theClock3.cpp


void theClock::run()
	// This is called every 4 ms or so ...
	// Try not to call setXXX stuff while clock is running.
{
	// Semantics of START_DELAY
	//
	// It is now the number of milliseconds BEFORE the zero crossing at which we will
	// deliver the initial impulse.  It used to be the number of milliseconds AFTER -1
	// seconds (59 seconds) and used a delay() to effect the starting pulse.
	//
	// We assume this method will be called at least every 4-5 ms, esp when in
	// CLOCK_STATE_NONE.

	if (m_clock_state == CLOCK_STATE_NONE)
	{
		if (_start_sync)
		{
			// get the current RTC clock time

			struct timeval tv;
			gettimeofday(&tv, NULL);
			int32_t secs = tv.tv_sec % 60;
			int32_t ms = tv.tv_usec / 1000L;

			// calculate the time at which to start
			// as (negative) ms before or (positive) ms after zero crossing
			// upto 5000 ms in either direction

			bool start_it = 0;
			if (_start_delay > 0)
			{
				int32_t start_secs = 0;
				int32_t start_ms = 0;
				timeAddMS(&start_secs,&start_ms,_start_delay);
				start_it = secs < 6 && secs >= start_secs && ms >= start_ms;
			}
			else
			{
				int32_t start_secs = 60;
				int32_t start_ms = 0;
				timeAddMS(&start_secs,&start_ms,_start_delay);
				start_it = secs >= start_secs && ms >= start_ms;
			}

			if (start_it)
			{
				LOGI("start_sync at %d:%03d delay=%d",secs,ms,_start_delay);
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
			motor(0);

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


	// The rest of this is only done if we are running

	if (m_clock_state < CLOCK_STATE_STARTED)
		return;

	//-------------------------------------------------
	// CYCLE
	//-------------------------------------------------

	// get the current RTC clock time, and
	// if CLOCK_MODE_SENSOR_TEST show the realtime clock every second

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

		// CLOCK_MODE_SENSOR_TEST only needed because
		// "running" is required for plot values to be sent.

		if (_clock_mode == CLOCK_MODE_SENSOR_TEST && !_plot_data)
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

			int side_type = _clock_type ? -1 : 1;
			if (as5600_side == side_type)
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

			#if CLOCK_COMPILE_VERSION == 1
				if (_clock_mode == CLOCK_MODE_PID)
					m_spring_on = 1;
			#endif

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
					#if CLOCK_COMPILE_VERSION == 1
						if (as5600_side < 0)
						{
							m_spring_on = 1;
							m_spring_power = getSpringPower();
							spring(m_spring_power);
							m_pid_angle = _angle_start;
								// re-get the pref to allow real time changes
						}
					#else
						m_pid_angle = getPidAngle();
					#endif
				}
				else if (_clock_mode == CLOCK_MODE_MIN_MAX)
				{
					#if CLOCK_COMPILE_VERSION == 1
						if (!m_spring_on &&
							m_total_millis_error + m_sync_millis > _min_max_ms)
						{
							LOGD("SPRING ON");
							spring(m_spring_power);

							m_spring_on = 1;
						}
						else if (m_spring_on &&
							m_total_millis_error + m_sync_millis < -_min_max_ms)
						{
							LOGD("SPRING OFF");
							spring(0);
							m_spring_on = 0;
						}

					#else
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
					#endif
				}
			}

			// calculate the power to use
			// v1.4 with sensor might just use right (power) swing

			#if CLOCK_COMPILE_VERSION == 1
				#define USE_LEFT -1
				#define USE_AVG   0
				#define USE_RIGHT 1
				#define USE_SIDE	USE_LEFT

				// We use the left side to get consistent results
				// from turning the spring on and off, else the
				// pendulum swings past the coil and the spring
				// does not work correctly. With 11 degrees this
				// seems to reliably produce a tick on the right swing.

			 	float avg_angle =
					(USE_SIDE == USE_LEFT) ? abs(as5600_min_angle) :
					(USE_SIDE == USE_RIGHT) ? as5600_max_angle :
					getAS560AverageAngle();
			#else
			 	float avg_angle =
					getAS560AverageAngle();
			#endif

			int use_power =
				_clock_mode == CLOCK_MODE_POWER_MIN ? _power_min :
				_clock_mode == CLOCK_MODE_POWER_MAX ? _power_max :
				getPidPower(avg_angle);

			// PULSE THE MOTOR!

			m_motor_start = now;
			m_motor_dur =_dur_pulse;
			motor(use_power);

			updateStatsPowerAngle(use_power, m_total_ang_error);


			LOGD("%-6s%s %-4d %7.3f/%6.3f=%-6.3f  targ=%-6.3f  a_err=%-6.3f  power=%-3d  err=%-4d  sync=%-4d"

				#if CLOCK_COMPILE_VERSION == 1
					" spring(%d)"
				#endif
				 ,
				 m_sync_sign ? "SYNC" : m_clock_state == CLOCK_STATE_RUNNING ? "run" : "start",
				 as5600_direction==1?"+":"-",
				 m_cur_cycle,
				 as5600_min_angle,
				 as5600_max_angle,
				 avg_angle,
				 m_pid_angle,
				 m_total_ang_error,
				 use_power,
				 m_total_millis_error,
				 m_sync_millis

				#if CLOCK_COMPILE_VERSION == 1
					,(m_spring_on ? m_spring_power : 0)
					// ,(_clock_mode == CLOCK_MODE_MIN_MAX) ||
					// (m_spring_on && as5600_side < 0) ? m_spring_power : 0)
				#endif

				);


			#if WITH_WS

				if (_plot_data && _plot_type == PLOT_CYCLES)
				{
					static char plot_buf[255];

					// in order to produce a useful plot, to prevent everything
					// from writing in the same place, we use separate
					// ranges centered around integers with values normalized from
					// -1 to +1 about the integers.

					// Normalize all the values to +/- 0.5 or thereabouts

					float plot_power = use_power - 128;		// -128 (nwver) to 128 (most often)
					plot_power = plot_power/128;		    // 9 +/- 1

					// instantaneous error normalized to _cycle_range (50)

					float plot_cur_cycle = m_cur_cycle - 1000.0;
					plot_cur_cycle = plot_cur_cycle / _cycle_range;

					// cumulative ms error plus m_sync_millis
					// normalized to the _error_range (150)
					// will likely jump off the chart during a sync cycle.

					float plot_millis_err = m_total_millis_error + m_sync_millis;
					plot_millis_err = plot_millis_err / _error_range;

					// the minimum and maximum angles, normalized to 8..16 degrees to make sense and
					// point in opposite directions for plotting on the same line

					float plot_min_angle = as5600_min_angle + 8;
					plot_min_angle = plot_min_angle/8;

					float plot_max_angle = as5600_max_angle - 8;
					plot_max_angle = plot_max_angle/8;


					// the average angle and target angle are normalized to the
					// _min_angle to _max_angle as +/- 0.5 about the line

					float angle_range = _angle_max -_angle_min;
					float plot_avg_angle = avg_angle - _angle_min;
					plot_avg_angle = plot_avg_angle / angle_range - 0.5;

					float plot_targ_angle = m_pid_angle - _angle_min;
					plot_targ_angle = plot_targ_angle / angle_range - 0.5;

					// the angular error is arbitrarily normalized about 15 degrees

					float plot_ang_error = m_total_ang_error / 15.0;

					// create the json

					sprintf(plot_buf,"{\"plot_data\":[%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,0,8]}",
						7 + plot_power,			// red
						6 + plot_cur_cycle,     // green
						5 + plot_millis_err,    // blue
						4 + plot_ang_error,     // cyan
						3 + plot_targ_angle,    // magenta
						2 + plot_avg_angle,     // orange
						1 + plot_min_angle,     // purple
						1 + plot_max_angle      // lime
						);                      // teal
												// pink

					// broadcast the json

					the_clock->wsBroadcast(plot_buf);

				}	// if (_plot_data && _plot_type == PLOT_CYCLES)

			#endif	// WITH_WS

		}	// push motor
	}	// angle threshold change exceeded

	// Stop the motor as needed based on duration,

	if (m_motor_start && now - m_motor_start > m_motor_dur)
	{
		m_motor_start = 0;
		m_motor_dur = 0;
		motor(0);

		#if CLOCK_COMPILE_VERSION == 1
			// if we want the spring on, and the pendulum is moving to the left
			// set the timer to start the spring.
			if (0 && m_spring_on && as5600_side < 0 && m_spring_power > 0)
			{
				m_push_spring = (_clock_mode == CLOCK_MODE_MIN_MAX)  ? 0 : now;
			}
		#endif
	}

	#if 0 && CLOCK_COMPILE_VERSION == 1
		// if the spring start delay is up, turn on the spring
		// if the spring needs to be turned off, turn it off
		if (m_push_spring && now - m_push_spring >= _spring_delay)
		{
			m_push_spring = 0;
			m_spring_start = now;
			spring(m_spring_power);
			// LOGD("SPRING PUSH");
		}
		if (m_spring_start && now - m_spring_start > _spring_dur)
		{
			m_spring_start = 0;
			// LOGD("SPRING UNPUSH");
			spring(0);
		}
	#endif

	//----------------------
	// plotting
	//----------------------

	#if WITH_WS
		if (_plot_data && _plot_type == PLOT_WAVES)
		{
			static char plot_buf[120];

			float use_angle = as5600_cur_angle * 50;		// 20 degrees == 1000 in output
			float use_min = as5600_min_angle * 50;
			float use_max = as5600_max_angle * 50;
			int ang = use_angle;
			int min = use_min;
			int max = use_max;
			int plot_millis_error = _clock_mode >= CLOCK_MODE_MIN_MAX ?
				m_total_millis_error * 20 : 0;
			if (plot_millis_error > 1000)
				plot_millis_error = 1000;
			if (plot_millis_error < -1000)
				plot_millis_error = -1000;

			sprintf(plot_buf,"{\"plot_data\":[%d,%d,%d,%d,%d,%d,%d,1000,-1000]}",
				as5600_direction * 200,
				as5600_side * 250,
				ang,
				min,
				max,
				plot_millis_error,
				cur_motor_power);


			the_clock->wsBroadcast(plot_buf);

		}	// if (_plot_data && _plot_type == PLOT_WAVES)

	#endif // WITH_WS


}	// theClock::run()


// plotter legend constants are located here
// close to where we plot them for easier maintenance.
// The leegends must include place keepers for the constant values

const char *plot_legend_cycles = "power,cycle,err,ang_err,targ,avg,min,max,0,8";
const char *plot_legend_waves = "dir,side,ang,min,max,err,power,1000,-1000";

