// clockStats.cpp

#include "clockStats.h"

// Statistics

static char msg_buf[512];
	// generic buffer for loop() related messages


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


//===================================================
// used in method
//===================================================

void initClockStats(bool restart)
{
	if (!restart)
	{
		stat_num_bad_reads = 0;
		stat_num_restarts = 0;
	}

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

void incStatRestarts()
{
	stat_num_restarts++;
}

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


const char *getStatBufBadReadsAndRestarts()
{
	sprintf(msg_buf,"num_bad(%d)  restarts(%d)",
		stat_num_bad_reads,
		stat_num_restarts);
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
