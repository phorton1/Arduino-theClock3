// theClock3.ino
//
// contains the myIOT device definition, setup() and loop()
// as well as definitions of all static parameters (members
// that start with underscore).
//
// Note that the default factory_reset turns WIFI off.
// See /base/bat/platform.pm for compile time defines to the myIOT layer

#include "theClock3.h"
#include <myIOTLog.h>

#define OVER_MAX_ANGLE  		15			// over design maximum degrees

//------------------------
// theClock definition
//------------------------
// The STANDARD CLOCK 3 using MOSFET circuit board.
//
// Includes my Clock 3.3 (unlabeled 0/3) and Daniel's (labeled v2 3/3),
// which I renamed to theClock3.3.0 and theClock3.3-Daniels respectively.
// Note that the coils are wired in series.
//
// At this time the only difference with Daniels, is that I set the start
// delay to -500 instead of 200. I had a different setup for a while, shown
// below in comments "old 3.3"


#define DEFAULT_RUNNING			0				// factory reset == clock not running
#define DEFAULT_CLOCK_MODE		CLOCK_MODE_PID	// factory reset == PID mode turned on
#define DEFAULT_LED_BRIGHTNESS  40

#define DEFAULT_ZERO_ANGLE		0			// 0 means it's not yet set
#define DEFAULT_ZERO_ANGLE_F	0.00		// 0 means it's not yet set
#define DEFAULT_DEAD_ZONE		0.3			// dead degrees about zero

#define DEFAULT_ANGLE_START 	10.0		// starting value for clock_pid control
#define DEFAULT_ANGLE_MIN 		9.0
#define DEFAULT_ANGLE_MAX 		11.5

#define DEFAULT_POWER_MIN		60
#define DEFAULT_POWER_PID		100
#define DEFAULT_POWER_MAX		255
#define DEFAULT_POWER_START     255

#define DEFAULT_DUR_PULSE		120
#define DEFAULT_DUR_START		250

#define DEFAULT_PID_P			20.0
#define DEFAULT_PID_I			0.50
#define DEFAULT_PID_D			-9.0

#define DEFAULT_APID_P			0.2
#define DEFAULT_APID_I			0.025
#define DEFAULT_APID_D			0.002

#define DEFAULT_RUNNING_ANGLE   4.0
#define DEFAULT_RUNNING_ERROR   4.0

#define DEFAULT_MIN_MAX_MS		50
#define DEFAULT_START_DELAY     200			// plus/minus ms AFTER/BEFORE 0 crossing to start clock
	// Daniels 3.3 = -500

#define DEFAULT_CYCLE_RANGE 	50
#define DEFAULT_ERROR_RANGE 	150

#define DEFAULT_STAT_INTERVAL	30
#define DEFAULT_SYNC_INTERVAL	3600L	// one hour
#define DEFAULT_NTP_INTERVAL	14400L	// four hours

#if WITH_VOLTAGES
	#define DEFAULT_VOLT_INTERVAL	0
	#define DEFAULT_VOLT_SCALE		1.03
	#define DEFAULT_VOLT_CUTOFF		4.00
	#define DEFAULT_VOLT_RESTORE    4.50
#endif


#if CLOCK_COMPILE_VERSION == 3

	#pragma message "Compiling Clock v3.3"

	#define THE_CLOCK             	"theClock3.3"
	#define THE_CLOCK_VERSION     	"3.3"
	#define THE_CLOCK_URL			"https://github.com/phorton1/Arduino-theClock3"

#elif CLOCK_COMPILE_VERSION == 2

	// VERSION 3.2 USING LM293D motor driver
	// These are the two clocks I gifted on 2023-06-08 to Pamela and Chris.
	// They are labeled v3.2 1/3 and 2/3 respectively

	#pragma message "Compiling Clock v3.2"

	#define THE_CLOCK             	"theClock3.2"
	#define THE_CLOCK_VERSION     	"3.2"
	#define THE_CLOCK_URL			"https://github.com/phorton1/Arduino-theClock3"

	#define DEFAULT_START_DELAY     -100		// negative ms BEFORE 0 crossing to start clock

	#define DEFAULT_POWER_MIN		120
	#define DEFAULT_POWER_PID		210

#elif CLOCK_COMPILE_VERSION == 1

	// My original clock v1.1 retrofitted to PCB 1.4 MOSFET board
	// with explicit "spring" coil.
	//
	// This clock works by running at a set ANGLE_START (to ensure it ticks),
	// tuned to run at slightly over 1000 ms, and will modify the cycle time
	// by energizing a "spring" coil, causing the cycle time to go under 1000ms.
	//
	// It uses the same MIN and MAX power algorithm, to control the power to
	// maintain the given angle, unchanged from VERSION3.
	//
	// Instead of using using the 2nd PID controller to control the angle,
	// in order to modify the time of the swings, we use the 2nd PID to
	// control the power delivered to the spring.
	//
	// All values listed here are changed from VERSION 3

	#pragma message "Compiling Clock v1.4"

	#define THE_CLOCK             	"theClock1.4"
	#define THE_CLOCK_VERSION     	"1.4"
	#define THE_CLOCK_URL			"https://github.com/phorton1/Arduino-theClock3"

	#define DEFAULT_ANGLE_START 	11.0		// with USE_LEFT, more or less guarantees a tick
	#define DEFAULT_RUNNING_ANGLE   10.0		// start running at bigger angle, sooner
	#define DEFAULT_RUNNING_ERROR   25.0

	#define DEFAULT_APID_P			10.0
	#define DEFAULT_APID_I			7
	#define DEFAULT_APID_D			0.002

	// added for Spring

	// The spring will turned on after the left zero crossing
	// and the motor impulse stops with a delay, a duration,
	// and a power level. Based on a 120ms pulse somewhat
	// after the zero crossing, and 500ms per side, with a
	// 10ms delay, we turn it on for 200ms

	#define DEFAULT_SPRING_DELAY   10
	#define DEFAULT_SPRING_DUR     200
	#define DEFAULT_SPRING_POWER   180

#endif



// what shows up on the "dashboard" UI tab

static valueIdType dash_items[] = {
	ID_START_SYNC,
	ID_RUNNING,
	ID_CLOCK_MODE,
	ID_LED_BRIGHTNESS,
	ID_CLEAR_STATS,
	ID_CLEAR_ERRORS,
	ID_SYNC_RTC,
#if CLOCK_WITH_NTP
	ID_SYNC_NTP,
	ID_STAT_MSG0,
	ID_STAT_MSG1,
	ID_STAT_MSG2,
	ID_STAT_MSG3,
	ID_STAT_MSG4,
	ID_STAT_MSG5,
	ID_STAT_MSG6,
	ID_SET_ZERO_ANGLE,
	ID_ZERO_ANGLE,
	ID_ZERO_ANGLE_F,
#endif
	ID_PLOT_VALUES,
	ID_TEST_COILS,
#if CLOCK_COMPILE_VERSION == 1
	ID_TEST_SPRING,
#endif
	ID_CHANGE_CLOCK,
	0,
};


// what shows up on the "device" UI tab

static valueIdType device_items[] = {
	ID_CLOCK_TYPE,
	ID_START_DELAY,
	ID_ANGLE_START,
	ID_ANGLE_MIN,
	ID_ANGLE_MAX,
	ID_DEAD_ZONE,
#if CLOCK_COMPILE_VERSION == 1
	ID_SPRING_DELAY,
	ID_SPRING_DUR,
	ID_SPRING_POWER,
#endif
	ID_POWER_MIN,
    ID_POWER_PID,
	ID_POWER_MAX,
	ID_POWER_START,
    ID_DUR_PULSE,
	ID_DUR_START,
	ID_PID_P,
	ID_PID_I,
	ID_PID_D,
	ID_APID_P,
	ID_APID_I,
	ID_APID_D,
	ID_RUNNING_ANGLE,
	ID_RUNNING_ERROR,
	ID_MIN_MAX_MS,
	ID_CYCLE_RANGE,
	ID_ERROR_RANGE,
	ID_STAT_INTERVAL,
	ID_SYNC_INTERVAL,
#if CLOCK_WITH_NTP
	ID_NTP_INTERVAL,
#endif
#if WITH_VOLTAGES
	ID_VOLT_INTERVAL,
	ID_VOLT_CALIB,
	ID_VOLT_CUTOFF,
	ID_VOLT_RESTORE,
	ID_LOW_POWER_EN,
#endif
	0
};


static enumValue clockType[] = {
    "Right",
    "Left",
    0};


static enumValue clockAllowed[] = {
    "Sensor_Test",
    "Power_Min",
	"Power_Max",
	"Angle_Start",
	"Angle_Min",
	"Angle_Max",
	"Min_Max",
	"PID",
    0};

static enumValue plotAllowed[] = {
    "Off",
    "Waves",
    "Pause",
	"Clock",
    0};

static enumValue pixelsAllowed[] = {
    "Off",
    "Diag",
    "Time",
    0};




// value descriptors for theClock
// in order of presentation via UI 'values' command

const valDescriptor theClock::m_clock_values[] =
{
    { ID_DEVICE_NAME,      VALUE_TYPE_STRING,    VALUE_STORE_PREF,	VALUE_STYLE_REQUIRED,	NULL,   NULL,   THE_CLOCK },
        // DEVICE_NAME overrides base class element

	{ ID_CLOCK_TYPE,      	VALUE_TYPE_ENUM,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_clock_type,		NULL, 	{ .enum_range = { 0, clockType }} },
	{ ID_CLOCK_MODE,      	VALUE_TYPE_ENUM,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_clock_mode, 		(void *) onClockModeChanged, { .enum_range = { DEFAULT_CLOCK_MODE, clockAllowed }} },

	{ ID_RUNNING,      		VALUE_TYPE_BOOL,	VALUE_STORE_PUB, 	VALUE_STYLE_NONE,       (void *) &_clock_running,	(void *) onClockRunningChanged, },
	{ ID_START_SYNC,      	VALUE_TYPE_BOOL,	VALUE_STORE_PUB, 	VALUE_STYLE_NONE,       (void *) &_start_sync,		(void *) onStartSyncChanged, },
	{ ID_START_DELAY,  		VALUE_TYPE_INT, 	VALUE_STORE_PREF,	VALUE_STYLE_NONE,  		(void *) &_start_delay,	 	NULL, { .int_range = 	{ DEFAULT_START_DELAY,   		-5000,  5000}} },

	{ ID_SET_ZERO_ANGLE,  	VALUE_TYPE_COMMAND,	VALUE_STORE_SUB, 	VALUE_STYLE_NONE,       NULL,                    	(void *) setZeroAngle },
	{ ID_ZERO_ANGLE,  		VALUE_TYPE_INT,  	VALUE_STORE_PREF,	VALUE_STYLE_READONLY,   (void *) &_zero_angle,		NULL,  { .int_range = 	{ DEFAULT_ZERO_ANGLE,  	0,  4095}} },
	{ ID_ZERO_ANGLE_F,  	VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_READONLY,   (void *) &_zero_angle_f,	NULL,  { .float_range = { DEFAULT_ZERO_ANGLE_F, 0,  360}} },
	{ ID_DEAD_ZONE,  		VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_dead_zone,		NULL,  { .float_range = { DEFAULT_DEAD_ZONE,    0,  OVER_MAX_ANGLE}} },
	{ ID_ANGLE_START,  		VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_angle_start,		NULL,  { .float_range = { DEFAULT_ANGLE_START,  0,  OVER_MAX_ANGLE}} },
	{ ID_ANGLE_MIN,  		VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_angle_min,		NULL,  { .float_range = { DEFAULT_ANGLE_MIN,    0,  OVER_MAX_ANGLE}} },
	{ ID_ANGLE_MAX,  		VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_angle_max,		NULL,  { .float_range = { DEFAULT_ANGLE_MAX,    0,  OVER_MAX_ANGLE}} },

#if CLOCK_COMPILE_VERSION == 1
	{ ID_SPRING_DELAY,  	VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_spring_delay,	NULL,  { .int_range = 	{ DEFAULT_SPRING_DELAY, 0,  1000}} },
	{ ID_SPRING_DUR,  		VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_spring_dur,		NULL,  { .int_range = 	{ DEFAULT_SPRING_DUR,   0,  1000}} },
	{ ID_SPRING_POWER,  	VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_spring_power,	NULL,  { .int_range = 	{ DEFAULT_SPRING_POWER, 0,  255}} },
#endif


	{ ID_POWER_MIN,  		VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_power_min,		NULL,  { .int_range = 	{ DEFAULT_POWER_MIN,  	0,  255}} },
	{ ID_POWER_PID,  		VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_power_pid,		NULL,  { .int_range = 	{ DEFAULT_POWER_PID,   	0,  255}} },
	{ ID_POWER_MAX,  		VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_power_max,		NULL,  { .int_range = 	{ DEFAULT_POWER_MAX,   	0,  255}} },
	{ ID_POWER_START,  		VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_power_start,		NULL,  { .int_range = 	{ DEFAULT_POWER_START,  0,  255}} },

	{ ID_DUR_PULSE,  		VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_dur_pulse,		NULL,  { .int_range = 	{ DEFAULT_DUR_PULSE,   	0,  1000}} },
	{ ID_DUR_START,  		VALUE_TYPE_INT,		VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_dur_start,		NULL,  { .int_range = 	{ DEFAULT_DUR_START,   	0,  1000}} },

	{ ID_PID_P,  			VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_pid_P,			NULL,  { .float_range = { DEFAULT_PID_P,   	-1000,  1000}} },
	{ ID_PID_I,  			VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_pid_I,			NULL,  { .float_range = { DEFAULT_PID_I,    -1000,  1000}} },
	{ ID_PID_D,  			VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_pid_D,			NULL,  { .float_range = { DEFAULT_PID_D,   	-1000,  1000}} },

	{ ID_APID_P,  			VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_apid_P,			NULL,  { .float_range = { DEFAULT_APID_P,   -1000,  1000}} },
	{ ID_APID_I,  			VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_apid_I,			NULL,  { .float_range = { DEFAULT_APID_I,   -1000,  1000}} },
	{ ID_APID_D,  			VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_apid_D,			NULL,  { .float_range = { DEFAULT_APID_D,   -1000,  1000}} },

	{ ID_RUNNING_ANGLE,  	VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_running_angle, 	NULL, { .float_range = 	{ DEFAULT_RUNNING_ANGLE, 0, 12}} },
	{ ID_RUNNING_ERROR,  	VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,       (void *) &_running_error, 	NULL, { .float_range = 	{ DEFAULT_RUNNING_ERROR, 1.0, 100}} },

	{ ID_MIN_MAX_MS,  		VALUE_TYPE_INT,  	VALUE_STORE_PREF,	VALUE_STYLE_NONE,   	(void *) &_min_max_ms,	 	NULL, { .int_range = 	{ DEFAULT_MIN_MAX_MS,   	    10, 1000}} },

	{ ID_CYCLE_RANGE,  		VALUE_TYPE_INT,  	VALUE_STORE_PREF,	VALUE_STYLE_NONE,   	(void *) &_cycle_range,	 	NULL, { .int_range = 	{ DEFAULT_CYCLE_RANGE,   	    10, 1000}} },
	{ ID_ERROR_RANGE, 		 VALUE_TYPE_INT,  	VALUE_STORE_PREF,	VALUE_STYLE_NONE,   	(void *) &_error_range,	 	NULL, { .int_range = 	{ DEFAULT_ERROR_RANGE,   	    10, 5000}} },

	{ ID_LED_BRIGHTNESS,  	VALUE_TYPE_INT, 	VALUE_STORE_PREF,     VALUE_STYLE_NONE,		(void *) &_led_brightness,	(void *) onBrightnessChanged, { .int_range = { DEFAULT_LED_BRIGHTNESS,  	0,  254}} },
	{ ID_PLOT_VALUES,      	VALUE_TYPE_ENUM,	VALUE_STORE_PUB,      VALUE_STYLE_NONE,		(void *) &_plot_values, 	(void *) onPlotValuesChanged, { .enum_range = { 0, plotAllowed }} },

	{ ID_SYNC_RTC,  		VALUE_TYPE_COMMAND,	VALUE_STORE_SUB,      VALUE_STYLE_NONE,    	NULL,                    	(void *) onSyncRTC },
	{ ID_SYNC_INTERVAL,  	VALUE_TYPE_INT,    	VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,	(void *) &_sync_interval, 	NULL,  { .int_range = { DEFAULT_SYNC_INTERVAL,0,3000000L}} },

#if CLOCK_WITH_NTP
	{ ID_SYNC_NTP,  		VALUE_TYPE_COMMAND,	VALUE_STORE_SUB,      VALUE_STYLE_NONE,    	NULL,                    	(void *) onSyncNTP },
	{ ID_NTP_INTERVAL,  	VALUE_TYPE_INT,    	VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,	(void *) &_ntp_interval, 	NULL,  { .int_range = { DEFAULT_NTP_INTERVAL,0,3000000L}} },
#endif

	{ ID_CLEAR_STATS,       VALUE_TYPE_COMMAND,	VALUE_STORE_SUB,      VALUE_STYLE_NONE,    	NULL,                       (void *) clearStats },
	{ ID_CLEAR_ERRORS,      VALUE_TYPE_COMMAND,	VALUE_STORE_SUB,      VALUE_STYLE_NONE,    	NULL,                       (void *) clearErrors },
	{ ID_STAT_INTERVAL,  	VALUE_TYPE_INT,    	VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,	(void *) &_stat_interval, 	NULL,  { .int_range = { DEFAULT_STAT_INTERVAL,0,3000000L}} },

	{ ID_STAT_MSG0,      	VALUE_TYPE_STRING, 	VALUE_STORE_PUB,      VALUE_STYLE_READONLY,	(void *) &_stat_msg0, },
	{ ID_STAT_MSG1,      	VALUE_TYPE_STRING, 	VALUE_STORE_PUB,      VALUE_STYLE_READONLY,	(void *) &_stat_msg1, },
	{ ID_STAT_MSG2,      	VALUE_TYPE_STRING, 	VALUE_STORE_PUB,      VALUE_STYLE_READONLY,	(void *) &_stat_msg2, },
	{ ID_STAT_MSG3,      	VALUE_TYPE_STRING, 	VALUE_STORE_PUB,      VALUE_STYLE_READONLY,	(void *) &_stat_msg3, },
	{ ID_STAT_MSG4,      	VALUE_TYPE_STRING, 	VALUE_STORE_PUB,      VALUE_STYLE_READONLY,	(void *) &_stat_msg4, },
	{ ID_STAT_MSG5,      	VALUE_TYPE_STRING, 	VALUE_STORE_PUB,      VALUE_STYLE_READONLY,	(void *) &_stat_msg5, },
	{ ID_STAT_MSG6,      	VALUE_TYPE_STRING, 	VALUE_STORE_PUB,      VALUE_STYLE_READONLY,	(void *) &_stat_msg6, },

	{ ID_TEST_COILS,  		VALUE_TYPE_INT,    	VALUE_STORE_PUB,      VALUE_STYLE_OFF_ZERO,	(void *) &_test_coils,		(void *) onTestCoils, 		{ .int_range = { 0, 0, 255}} },
#if CLOCK_COMPILE_VERSION == 1
	{ ID_TEST_SPRING,  		VALUE_TYPE_INT,    	VALUE_STORE_PUB,      VALUE_STYLE_OFF_ZERO,	(void *) &_test_spring,		(void *) onTestSpring, 		{ .int_range = { 0, 0, 255}} },
#endif

	{ ID_CHANGE_CLOCK,  	VALUE_TYPE_INT,    	VALUE_STORE_PUB,      VALUE_STYLE_NONE,   	(void *) &_change_clock,	(void *) onChangeClock,  	{ .int_range = { 0, -3000000L, 3000000L}} },

#if WITH_VOLTAGES
	{ ID_VOLT_INTERVAL,  	VALUE_TYPE_INT,    	VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,	(void *) &_volt_interval, 	NULL,  { .int_range = 	{ DEFAULT_VOLT_INTERVAL, 0,  86400}} },
	{ ID_VOLT_CALIB,      	VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,     VALUE_STYLE_NONE,		(void *) &_volt_calib, 		NULL,  { .float_range = { DEFAULT_VOLT_SCALE,    0,  2}} },
	{ ID_VOLT_CUTOFF,      	VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,     VALUE_STYLE_NONE,		(void *) &_volt_cutoff, 	NULL,  { .float_range = { DEFAULT_VOLT_CUTOFF,   0,  120}} },
	{ ID_VOLT_RESTORE,      VALUE_TYPE_FLOAT,	VALUE_STORE_PREF,     VALUE_STYLE_NONE,		(void *) &_volt_restore, 	NULL,  { .float_range = { DEFAULT_VOLT_RESTORE,  0,  120}} },
	{ ID_LOW_POWER_EN,  	VALUE_TYPE_BOOL, 	VALUE_STORE_PREF,     VALUE_STYLE_NONE,		(void *)&_low_power_enable, NULL,  },
#endif

};


#define NUM_CLOCK_VALUES (sizeof(m_clock_values)/sizeof(valDescriptor))

static const char *clock_tooltips[] = {
    ID_CLOCK_TYPE,
		"Determines which <b>direction</b> will be used for measuring the "
		"<i>zero crossing</i> for a <i>full 1000 ms cycle</i>.",
    ID_CLOCK_MODE,
		"Allows you to set different <b>modes</b> of operation "
		"for <i>tuning</i> and <i>experimenting</i> with the clock.",
	ID_RUNNING,
		"Starts <b>running</b> the clock in the current <i>mode</i>. "
		"Is set automatically, at the correct time if START_SYNC is used.",
	ID_START_SYNC,
		"<b>Starts</b> the clock at the next <i>minute crossing</i> "
		"based on START_DELAY.",
	ID_START_DELAY,
		"Determines, in <i>milliseconds</i>, <b>before</b> or <b>after</b> the "
		"minute crossing, the time at which <i>Start Sync</i> will take place.",
	ID_SET_ZERO_ANGLE,
		"Sets the <b>Zero Angle</b> for the <i>Angle Sensor</i>.",
	ID_ZERO_ANGLE,
		"The <i>raw integer</i> value of the zero angle reading, "
		"on a scale from <i>0 to 4095</i>.",
	ID_ZERO_ANGLE_F,
		"The <i>floating point</i> value of the zero angle reading, "
		"on a scale from <i>0 to 360 degrees</i>.",
	ID_DEAD_ZONE,
		"An angle, in <i>degrees about zero</i> where the coils will <b>not</b> "
		"be energized on each swing.",
	ID_ANGLE_START,
		"The starting <i>target angle</i> in <i>degrees</i> "
		"for PID and MIN_MAX clock modes.",
	ID_ANGLE_MIN,
		"The <i>minimum</i> target angle in <i>degrees</i> "
		"that will be used in PID and MIN_MAX clock modes.",
	ID_ANGLE_MAX,
		"The <i>maximum</i> target angle in <i>degrees</i> "
		"that will be used in PID and MIN_MAX clock modes.",
	ID_POWER_MIN,
		"The <i>minimum</i> <b>power</b>, on a scale from 0 to 255, "
		"that will be delivered to the coils.",
	ID_POWER_PID,
		"The <b>power</b>, on a scale from 0 to 255, that will be used "
		"as the starting input "
		"for the <b>1st PID controller</b> which tries to get the Pendulum to swing "
		"at the <i>target angle</i> by modifying the <i>power</i>.",
	ID_POWER_MAX,
		"The <i>maxium</i> <b>power</b>, on a scale from 0 to 255, "
		"that will be delivered to the coils.",
	ID_POWER_START,
		"The <b>power</b>, on a scale from 0 to 255, that will be used "
		"for the <b>initial starting pulse</b> to the Pendulum.",
	ID_DUR_PULSE,
		"The duration, in <i>milliseconds</i>, for the <b>pulses</b> "
		"delivered each time the Pendulum <i>crosses zero</i> (in either direction).",
	ID_DUR_START,
		"The duration, in <i>milliseconds</i>, for the <i>initial starting pulse</i>.",
	ID_PID_P,
		"The <i>proportional</i> factor, which is muliplied by the <i>instantaneous angular error</i>, "
		"for the <b>1st PID controller</b>, which tries to get the Pendulum to swing "
		"at the <i>target angle</i> by modifying the <i>power</i>.",
	ID_PID_I,
		"The <i>integral</i> factor, which is muliplied by the <i>cumulative angular error</i>, "
		"for the <b>1st PID controller</b>, which tries to get the Pendulum to swing "
		"at the <i>target angle</i> by modifying the <i>power</i>.",
	ID_PID_D,
		"The <i>derivative</i> factor. which is multiplied by the <i>difference</i> "
		"in the change in the degrees from the previous swing, "
		"for the <b>1st PID controller</b>, which tries to get the Pendulum to swing "
		"at the <i>target angle</i> by modifying the <i>power</i>.",
	ID_APID_P,
		"The <i>proportional</i> factor, which is muliplied by the <i>instantaneous cycle error</i>, "
		"for the <b>2nd PID controller</b>, which tries to get the Pendulum to swing "
		"at the <i>the correct rate</i> by modifying the <i>target angle</i>.",
	ID_APID_I,
		"The <i>integral</i> factor, which is muliplied by the <i>cumulative milliseconds error</i>, "
		"for the <b>2nd PID controller</b>, which tries to get the Pendulum to swing "
		"at the <i>the correct rate</i> by modifying the <i>target angle</i>.",
	ID_APID_D,
		"The <i>derivative</i> factor. which is multiplied by the <i>difference</i> "
		"in the milliseconds per swing from the previous swing, "
		"for the <b>2nd PID controller</b>, which tries to get the Pendulum to swing "
		"at the <i>the correct rate</i> by modifying the <i>target angle</i>.",
	ID_RUNNING_ANGLE,
		"The <b>angle</b>, in <i>degrees</i> at which the Pendulum must swing "
		"in clock_modes ANGLE_START and higher "
		"before the clock will change to CLOCK_STATE_RUNNING.",
	ID_RUNNING_ERROR,
		"The <b>cumulative angular error</b>, in <i>degrees</i> which the clock must fall under "
		"in clock_modes ANGLE_START and higher "
		"before the clock will change to CLOCK_STATE_RUNNING.",
	ID_MIN_MAX_MS,
		"The number of <i>milliseconds</i> fast, or slow, that the clock must be running "
		"for a change between the minimum and maxium <i>target angles</i> "
		"in MIN_MAX mode.",
	ID_CYCLE_RANGE,
		"The <b>instantaneous</b> number of <i>milliseconds</i> fast, or slow, for the current swing, "
		"that will cause the <i>4th LED</i> to change from <b>green</b> to <b>red</b> or <b>blue</b>.",
	ID_ERROR_RANGE,
		"The <b>cumulative</b> number of <i>milliseconds</i> fast, or slow, that will cause "
		"the <i>3rd LED</i> to change from <b>green</b> to <b>red</b> or <b>blue</b>.",
	ID_LED_BRIGHTNESS,
		"Sets the brightness, on a scale of 0 to 254, where 0 turns the LEDS <b>off</b>.",
	ID_PLOT_VALUES,
		"If <b>not Off</b>, this parameter <i>suspends</i> normal Serial and Telnet Monitor output, "
		"and instead, outputs a series of numbers every few milliseconds for use with the "
		"Arduino <b>Serial Plotter</b> to visualize the swing of the Pendulum, the pulses "
		"delivered to the clock, and so on.",
	ID_SYNC_RTC,
		"A <b>manual command</b> that allows you to immediatly trigger a synchronization "
		"of the <b>Clock</b> to the <b>RTC</b> (embedded Real Time Clock). "
		"Only can be done while the clock is <i>Running</i>. ",
	ID_SYNC_INTERVAL,
		"How often, in <i>seconds</i>, between attempts to synchronize the "
		"<b>Clock</b> to the <b>RTC</b> (embedded Real Time Clock).",
	ID_SYNC_NTP,
		"A <b>manual command</b> that allows you to immediatly trigger a synchronization "
		"of the <b>RTC</b> (Real Time Clock) to <b>NTP</b> (Network Time Protocol). "
		"Only can be done while the clock connected to <b>WiFi</b> in <i>Station Mode</i>.",
	ID_NTP_INTERVAL,
		"How often, in <i>seconds</i>, between attempts to synchronize the "
		"<b>RTC</b> (Real Time Clock) to <b>NTP</b> (Network Time Protocol). ",
	ID_CLEAR_STATS,
		"A command that will <i>clear</i> the accumulated <b>statistics</b> "
		"that are sent to the <i>WebUI</i> as if the clock was freshly <b>started</b>",
	ID_CLEAR_ERRORS,
		"A command that will <i>clear</i> the accumulated <b>angular and ms errors</b> "
		"and will also clear the statistics.",
	ID_STAT_INTERVAL,
		"How often, in <i>seconds</i> between sending "
		"updated <b>statistics</b> to the <i>WebUI</i>.",
	ID_STAT_MSG0,
		"A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.",
	ID_STAT_MSG1,
		"A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.",
	ID_STAT_MSG2,
		"A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.",
	ID_STAT_MSG3,
		"A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.",
	ID_STAT_MSG4,
		"A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.",
	ID_STAT_MSG5,
		"A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.",
	ID_STAT_MSG6,
		"A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.",
	ID_TEST_COILS,
		"A <i>control</i>, used for <i>testing</i>, that lets you directly energize the <b>coils</b> "
		"by sending a value for the <b>power</b>, on a scale of <i>0 to 255</i>. "
		"BE SURE TO RETURN THIS TO ZERO AFTER USE!!</b>.",
#if CLOCK_COMPILE_VERSION == 1
	ID_SPRING_DELAY,    "Delay after coil turns off on left swing before starting Spring to speed up.",
	ID_SPRING_DUR,      "Duration of spring power cycle.",
	ID_SPRING_POWER,    "Power to apply to spring coil during its cycle."
	ID_TEST_SPRING,		"Test the Spring Coil",
#endif
	ID_CHANGE_CLOCK,
		"A <i>control</i> that lets you <b>add</b> a positive, or <b>subtract</b> a negative "
		"<i>number of milliseconds</i> to the RTC (Real Time Clock). "
		"This can be used to <i>test</i> syncing to <b>NTP</b> or other "
		"aspects of the clock's <i>algorithms</i>.",
	ID_VOLT_INTERVAL,
		"With the <i>optional</i> <b>external power supply</b>, this parameter "
		"tells how often, <i>in seconds</i> to check for <i>low power</i> or "
		"<i>power restored</i> conditions.",
	ID_VOLT_CALIB,
		"With the <i>optional</i> <b>external power supply</b>, this parameter "
		"is <i>multiplied</i> by the <i>calculated voltage</i> to get a "
		"more reasonable <b>voltage</b> for use in the <i>low power sensing</i> algorithm.",
	ID_VOLT_CUTOFF,
		"With the <i>optional</i> <b>external power supply</b>, this parameter "
		"defines the <b>voltage</b> under which the clock will switch to <i>low power mode</i>",
	ID_VOLT_RESTORE,
		"With the <i>optional</i> <b>external power supply</b>, this parameter "
		"defines the <b>voltage</b> over which the clock will leave <i>low power mode</i>",
	ID_LOW_POWER_EN,
		"With the <i>optional</i> <b>external power supply</b>, this parameter "
		"<b>enables</b> the clock to actually <i>go</i> into <i>low power mode</i>. ",


	0 };

// EXTRA TEXT added to description in PARAM command (but not for tooltips)

const char *clock_extra_text[] = {
    ID_CLOCK_TYPE,
		"<b>Right</b> means that it will measure the cycle when the Pendulum crosses from "
		"<i>left to right</i> through <i>zero</i> and "
		"<b>left</b> means that it will measure the cycle when the Pendulum crosses from "
		"<i>right to left</i> through <i>zero</i>.",
	ID_START_DELAY,
		"A <i>negative</i> number will start the clock <i>before</i> the minute crossing, and "
		"a <i>positive</i> number will start the clock <i>after</i> the minute crossing.",
	ID_DEAD_ZONE,
		"This is necessary so that we don't send a <i>pulse</i> to the"
		"coil until <b>after</b> the Pendulum has crossed zero.",
	ID_ANGLE_START,
		"The clock will attempt to reach this angle before changing "
		"state from CLOCK_STARTING to CLOCK_RUNNING. This value should be set <i>between</i>  "
		"ANGLE_MIN and ANGLE_MAX.",
	ID_LOW_POWER_EN,
		"Note that if VOLT_INTERVAL is <b>not zero</b>, the clock will still <i>measure</i> "
		"the voltage and <b>display</b> the results in the Serial monitor even if "
		"<i>low power mode</i> is <b>disabled</b>.  That allows you to test the <i>power sensing</i> "
		"and <i>functionality</i> <b>before</b> actually sending the clock into "
		"<b>low power mode</b> which also <b>Turns the Wifi off</b>.",

	0 };



//--------------------------------------------------0
// params in this file, working vars in cpp
//--------------------------------------------------

int		theClock::_clock_type;
bool 	theClock::_start_sync;
bool 	theClock::_clock_running;
uint32_t theClock::_clock_mode;
uint32_t theClock::_plot_values;
int  	theClock::_led_brightness;

int  	theClock::_zero_angle;
float  	theClock::_zero_angle_f;
float   theClock::_dead_zone;
float   theClock::_angle_start;
float 	theClock::_angle_min;
float 	theClock::_angle_max;

#if CLOCK_COMPILE_VERSION == 1
	int theClock::_spring_delay;
	int theClock::_spring_dur;
	int theClock::_spring_power;
#endif

int  	theClock::_power_min;
int  	theClock::_power_pid;
int  	theClock::_power_max;
int  	theClock::_power_start;

int  	theClock::_dur_pulse;
int  	theClock::_dur_start;

float  	theClock::_pid_P;
float  	theClock::_pid_I;
float  	theClock::_pid_D;

float  	theClock::_apid_P;
float  	theClock::_apid_I;
float  	theClock::_apid_D;

float  	theClock::_running_angle;
float  	theClock::_running_error;
int	    theClock::_min_max_ms;
int 	theClock::_start_delay;

int	    theClock::_cycle_range;
int	    theClock::_error_range;


#if WITH_VOLTAGES
	uint32_t theClock::_volt_interval;
	float	 theClock::_volt_calib;
	float	 theClock::_volt_cutoff;
	float	 theClock::_volt_restore;
	bool	 theClock::_low_power_enable;
#endif


String 	 theClock::_stat_msg0;
String 	 theClock::_stat_msg1;
String 	 theClock::_stat_msg2;
String 	 theClock::_stat_msg3;
String 	 theClock::_stat_msg4;
String 	 theClock::_stat_msg5;
String 	 theClock::_stat_msg6;

uint32_t theClock::_stat_interval;
uint32_t theClock::_sync_interval;
#if CLOCK_WITH_NTP
	uint32_t theClock::_ntp_interval;
#endif

// remember, 'int' is 32 bits on ESP32 !!

int 	theClock::_test_coils;
#if CLOCK_COMPILE_VERSION == 1
	int theClock::_test_spring;
#endif
int		theClock::_change_clock;


// ctor

theClock::theClock()
{
    addValues(m_clock_values,NUM_CLOCK_VALUES);
    setTabLayouts(dash_items,device_items);
	addDerivedToolTips(clock_tooltips,clock_extra_text);
}


//--------------------------------
// main
//--------------------------------

theClock *the_clock;


void setup()
{
    delay(500);
	Serial.begin(115200);
    delay(500);

    theClock::setDeviceType(THE_CLOCK);
    theClock::setDeviceVersion(THE_CLOCK_VERSION);
	theClock::setDeviceUrl(THE_CLOCK_URL);


    LOGU("");
    LOGU("");
    LOGU("the_clock.ino setup() started on core(%d)",xPortGetCoreID());

    the_clock = new theClock();
    the_clock->setup();

    LOGU("the_clock.ino setup() finished");
}



void loop()
{
    the_clock->loop();
}
