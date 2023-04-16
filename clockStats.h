#pragma once

#include "theClock3.h"

#define MAX_INT		32767
#define MIN_INT		-32767

// used in various methods

extern void initClockStats(bool restart);

extern const char *getStatBufRTC(int32_t sync_millis);
#if CLOCK_WITH_NTP
	extern const char *getStatBufNTP(int32_t delta_ms, bool fail);
#endif
extern void updateStatsPidAngle(float pid_angle);

// used in run();

extern void incStatRestarts();
extern void incStatBadReads();
extern void updateStatsCycle(int32_t cur_cycle, int32_t total_millis_error);
extern void updateStatsMinAngle(float min_angle);
extern void updateStatsMaxAngle(float max_angle);
extern void updateStatsPowerAngle(int power, float angle_error);

// used in loop()

extern void initStatBuf();
extern void formatTimeToStatBuf(const char *label, uint32_t time_s, uint32_t time_ms, bool with_br);
extern const char *getStatBuf();
extern const char *getStatBufBadReadsAndRestarts();
extern const char *getStatBufAll();
extern const char *getStatBufRecent();
extern void initRecentStats();