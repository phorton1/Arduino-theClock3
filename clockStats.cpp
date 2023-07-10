// clockStats.cpp

#include "clockStats.h"
#include <myIOTLog.h>

// STATISTICS, ESP32_STATE, CLOCK_STATE, and CLOCK_MODE.
//
// 		Although we generally presume the clock will be running, there are
// 		certain statistics that are germaine to the life-cycle of the ESP32,
// 		like the number of power losses/restores and when they happen.
// 		A failure to init the AS5600 ... and a way to retry it ... might
// 		also fall into this category of ESP32_STATE.
//
// 		Then it is necessary to understand the difference between the UI
// 		RUNNING parameter and the CLOCK_STATE (m_clock_state), which is the
// 		actual STATE of the clock, which in turn relies on the CLOCK_MODE
// 		(m_clock_mode), which tells HOW to run the clock.
//
// 		Turning RUNNING off will call stopClock() and remove most 'statistics'.
// 		If the clock_state is NONE, and RUNNING is turned on, startClock(0=cold start) will
// 		be called.  This will clear the num_bad_reads value.
//
// 		CLOCK_MODE_SENSOR_TEST goes directly to CLOCK_STATE_RUNNING when startClock() is called.
//
// 		All other modes will include a starting pulse, and go to CLOCK_STATE_START after the
// 		initial impulse is delivered.  They then quickly go to a higher state when the impulse
// 		finishes. CLOCK_MODE_POWER_MIN/MAX go directly to CLOCK_STATE_RUNNING, whereas the
// 		rest of the (PID) modes go to CLOCK_STATE_STARTED, where they will wait for the clock
// 		angle PID controller to stabilize before going to CLOCK_STATE_RUNNING.
//
// 		In the special case of a synchronized start in MIN_MAX or PID modes, upon going to
// 		CLOCK_STATE_RUNNING, onSyncRTC() will be called.
//
// RESTARTS
//
//      The clock will be restarted for any modes > CLOCK_MODE_SENSOR_TEST if no significant
//		movment of the pendululm is detected after a given time.  Most stats, but not
//      num_restarts and num_bad_reads, will be cleared.
//
//
// CRITICAL TIMING
//
//		Because, for most modes, the stats show the number of beats versus the seconds, it is critical that
//      it is called soon after a cycle, hence the main statistics are generally updated based on
//      the number of beats changing, rather than on the RTC clock changing.
//
//      However, in CLOCK_MODE_SENSOR_TEST we update the stats based on the RTC.
//
// CLEAR_STATS
//
//      Clearing the statistics is a UI operation.  Hence it clears EVERYTHING including
//      the number of restarts, bad reads, and power loss statistics.



static char msg_buf[512];
	// generic buffer for loop() related messages


static uint32_t stat_num_bad_reads;

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

#if WITH_VOLTAGES
	static bool  any_volts;
	static float stat_volts_5v;
	static float stat_volts_vbus;
	static int stat_low_power_mode;
	static int stat_num_low_powers;
	static int32_t last_low_power_time;
	static int32_t last_restore_power_time;
#endif



//===================================================
// used in method
//===================================================

void initClockStats()
	// resets all STATISTICS, has nothing to do with
	// the variables that are used RUNNING the clock
{
	stat_num_bad_reads = 0;

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

	#if WITH_VOLTAGES
		any_volts = 0;
		stat_volts_5v = 0;
		stat_volts_vbus = 0;
		stat_low_power_mode = 0;
		stat_num_low_powers = 0;
		last_low_power_time = 0;
		last_restore_power_time = 0;
	#endif
}


const char *getStatBufRTC(int32_t sync_millis)
{
	stat_num_sync_checks++;
	if (sync_millis != 0)
	{
		stat_num_sync_changes++;
		stat_last_sync_change = sync_millis;
		stat_total_sync_changes += sync_millis;
		stat_total_sync_changes_abs += abs(sync_millis);
	}
	sprintf(msg_buf,"SYNC(%d/%d) last(%d) total(%d) abs(%d)",
		stat_num_sync_changes,
		stat_num_sync_checks,
		stat_last_sync_change,
		stat_total_sync_changes,
		stat_total_sync_changes_abs);
	return msg_buf;
}


#if CLOCK_WITH_NTP
	const char *getStatBufNTP(int32_t delta_ms, bool fail)
	{
		stat_num_ntp_checks++;
		if (fail)
		{
			stat_num_ntp_fails++;
		}
		else if (delta_ms)
		{
			stat_last_ntp_change = delta_ms;
			stat_num_ntp_changes++;
			stat_total_ntp_changes += delta_ms;
			stat_total_ntp_changes_abs += abs(delta_ms);
		}
		sprintf(msg_buf,"NTP(%d/%d) fails(%d) last(%d) total(%d) abs(%d)",
			stat_num_ntp_changes,
			stat_num_ntp_checks,
			stat_num_ntp_fails,
			stat_last_ntp_change,
			stat_total_ntp_changes,
			stat_total_ntp_changes_abs);
		return msg_buf;
	}
#endif	// CLOCK_WITH_NTP



void updateStatsPidAngle(float pid_angle)
{
	if (pid_angle < stat_min_target)
		stat_min_target = pid_angle;
	if (pid_angle > stat_max_target)
		stat_max_target = pid_angle;
	if (pid_angle < stat_recent_min_target)
		stat_recent_min_target = pid_angle;
	if (pid_angle > stat_recent_max_target)
		stat_recent_max_target = pid_angle;
}



//=============================================
// used in run()
//=============================================


void incStatBadReads()
{
	stat_num_bad_reads++;
}


void updateStatsCycle(int32_t cur_cycle, int32_t total_millis_error)
{
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
}


void updateStatsMinAngle(float min_angle)
{
	if (min_angle < stat_max_left)
		stat_max_left = min_angle;
	if (min_angle > stat_min_left)
		stat_min_left = min_angle;

	if (min_angle < stat_recent_max_left)
		stat_recent_max_left = min_angle;
	if (min_angle > stat_recent_min_left)
		stat_recent_min_left = min_angle;
}

void updateStatsMaxAngle(float max_angle)
{
	if (max_angle > stat_max_right)
		stat_max_right = max_angle;
	if (max_angle < stat_min_right)
		stat_min_right = max_angle;

	if (max_angle > stat_recent_max_right)
		stat_recent_max_right = max_angle;
	if (max_angle < stat_recent_min_right)
		stat_recent_min_right = max_angle;
}

void updateStatsPowerAngle(int power, float angle_error)
{
	if (power > stat_max_power)
		stat_max_power = power;
	if (power < stat_min_power)
		stat_min_power = power;

	if (power > stat_recent_max_power)
		stat_recent_max_power = power;
	if (power < stat_recent_min_power)
		stat_recent_min_power = power;

	if (angle_error < stat_min_total_ang_err)
		stat_min_total_ang_err = angle_error;
	if (angle_error > stat_max_total_ang_err)
		stat_max_total_ang_err = angle_error;
	if (angle_error < stat_recent_min_total_ang_err)
		stat_recent_min_total_ang_err = angle_error;
	if (angle_error > stat_recent_max_total_ang_err)
		stat_recent_max_total_ang_err = angle_error;
}


//=====================================
// used in loop()
//=====================================

void initStatBuf()
{
	msg_buf[0] = 0;
}

const char *getStatBuf()
{
	return msg_buf;
}

void formatTimeToStatBuf(const char *label, uint32_t time_s, uint32_t time_ms, bool with_br)
{
	int at = strlen(msg_buf);
	char *b = &msg_buf[at];
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
	if (with_br)
		strcat(msg_buf,"<br>");
}


const char *getStatBufMain()
{
	sprintf(msg_buf,"num_bad(%d)",stat_num_bad_reads);

	#if WITH_VOLTAGES
	if (any_volts)
		{
			char *b = &msg_buf[strlen(msg_buf)];
			sprintf(b," %s volts(%0.2f) vbus(%0.2f) num_low_powers(%d) %s",
				stat_low_power_mode == VOLT_MODE_LOW ? "LOW_POWER_MODE!!" :
				stat_low_power_mode == VOLT_DETECT_LOW ? "low power!" :
				"NORMAL",
				stat_volts_5v,
				stat_volts_vbus,
				stat_num_low_powers,
				stat_num_low_powers ? "<br>" : "");
			if (stat_num_low_powers)
			{
				formatTimeToStatBuf("LAST_DOWN", last_low_power_time, 0, last_restore_power_time);
				if (last_restore_power_time)
					formatTimeToStatBuf("LAST_UP", last_restore_power_time, 0, false);
			}
		}
	#endif

	return msg_buf;
}


const char *getStatBufAll()
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
	return msg_buf;
}


const char *getStatBufRecent()
{
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
	return msg_buf;
}


void initRecentStats()
{
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



//=====================================
// power mode stats
//=====================================

#if WITH_VOLTAGES

	void setStatsPower(int low_power_mode, float volts_5v, float volts_vbus)
	{
		stat_volts_5v = volts_5v;
		stat_volts_vbus = volts_vbus;

		LOGD("setStatsPower(%d,%0.2f,%02f) current=%d",low_power_mode,volts_5v,volts_vbus,stat_low_power_mode);

		if (stat_low_power_mode != low_power_mode)
		{
			if (!stat_low_power_mode && low_power_mode)
			{
				stat_num_low_powers++;
				last_low_power_time = time(NULL);
			}
			else if (stat_low_power_mode && !low_power_mode)
			{
				last_restore_power_time = time(NULL);
			}
			stat_low_power_mode = low_power_mode;
		}
	}

#endif	// WITH_VOLTAGES
