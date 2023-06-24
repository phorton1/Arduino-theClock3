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

#if CLOCK_COMPILE_VERSION == 1

	// My original clock v1.1 retrofitted with angle sensor and 5 LEDS

	#pragma message "Compiling Clock v1.3"

	#define THE_CLOCK             	"theClock1.3"
	#define THE_CLOCK_VERSION     	"1.3"

	#define DEFAULT_START_DELAY     600			// now defined as MS BEFORE 0 crossing to start clock
		// Note that clock 1.3 firmware has NOT been updated to this new code!

	#define DEFAULT_ANGLE_START 	10.5 		// starting value for clock_pid control
	#define DEFAULT_ANGLE_MIN 		10.3
	#define DEFAULT_ANGLE_MAX 		11.0
	#define DEFAULT_DEAD_ZONE		0.5			// dead degrees about zero

	#define DEFAULT_POWER_START     255
	#define DEFAULT_POWER_MIN		140
	#define DEFAULT_POWER_PID		210
	#define DEFAULT_POWER_MAX		240

	#define DEFAULT_DUR_PULSE		120

	#define DEFAULT_PID_P			20.0
	#define DEFAULT_PID_I			0.5
	#define DEFAULT_PID_D			0.01

	#define DEFAULT_APID_P			0.09
	#define DEFAULT_APID_I			0.05
	#define DEFAULT_APID_D			0.005

	#define DEFAULT_RUNNING_ANGLE   6.0
	#define DEFAULT_RUNNING_ERROR   4.0

	#define DEFAULT_CYCLE_RANGE 	80
	#define DEFAULT_ERROR_RANGE 	300

#elif CLOCK_COMPILE_VERSION == 2

	// PREVIOUS VERSION 3 USING DEFAULT CONFIGURATION PARAMETERS
	// Thee are the clocks I gifted on 2023-06-08 and my previous
	// clock 3.1 (though 3.0 did not use the voltage stuff) which
	// is now turned off in all clocks

	#pragma message "Compiling Clock v3.2"

	#define THE_CLOCK             	"theClock3.2"
	#define THE_CLOCK_VERSION     	"3.2"

	#define DEFAULT_START_DELAY     100			// now defined as MS BEFORE 0 crossing to start clock
	#define DEFAULT_ANGLE_START 	10.0		// starting value for clock_pid control
	#define DEFAULT_ANGLE_MIN 		9.5
	#define DEFAULT_ANGLE_MAX 		11.5
	#define DEFAULT_DEAD_ZONE		0.3			// dead degrees about zero

	#define DEFAULT_POWER_START     255
	#define DEFAULT_POWER_MIN		120
	#define DEFAULT_POWER_PID		210
	#define DEFAULT_POWER_MAX		255

	#define DEFAULT_DUR_PULSE		120

	#define DEFAULT_PID_P			20.0
	#define DEFAULT_PID_I			0.50
	#define DEFAULT_PID_D			-9.0

	#define DEFAULT_APID_P			0.2
	#define DEFAULT_APID_I			0.025
	#define DEFAULT_APID_D			0.002

	#define DEFAULT_RUNNING_ANGLE   4.0
	#define DEFAULT_RUNNING_ERROR   2.0

	#define DEFAULT_CYCLE_RANGE 	50
	#define DEFAULT_ERROR_RANGE 	150


#else	// CLOCK_V3

	// MY ORIGINAL CLOCK 3.2 which is now using MOSFET circuit board
	// has different defaults for ANGLE_MIN and POWER_MIN from version 3 gifts.
	// Note that the coils are wired in series.

	#pragma message "Compiling Clock v3.3"

	#define THE_CLOCK             	"theClock3.3"
	#define THE_CLOCK_VERSION     	"3.3"

	#define DEFAULT_START_DELAY     -500			// now defined as MS BEFORE 0 crossing to start clock
	#define DEFAULT_ANGLE_START 	10.0		// starting value for clock_pid control
	#define DEFAULT_ANGLE_MIN 		9.0			// !!! MOSFET CIRCUIT BOARD
	#define DEFAULT_ANGLE_MAX 		11.5
	#define DEFAULT_DEAD_ZONE		0.3			// dead degrees about zero

	#define DEFAULT_POWER_START     255
	#define DEFAULT_POWER_MIN		60			// !!! MOSFET CIRCUIT BOARD
	#define DEFAULT_POWER_PID		100
	#define DEFAULT_POWER_MAX		255

	#define DEFAULT_DUR_PULSE		120

	#define DEFAULT_PID_P			20.0
	#define DEFAULT_PID_I			0.50
	#define DEFAULT_PID_D			-9.0

	#define DEFAULT_APID_P			0.2
	#define DEFAULT_APID_I			0.025
	#define DEFAULT_APID_D			0.002

	#define DEFAULT_RUNNING_ANGLE   4.0
	#define DEFAULT_RUNNING_ERROR   2.0

	#define DEFAULT_CYCLE_RANGE 	50
	#define DEFAULT_ERROR_RANGE 	150

#endif


// common

#define DEFAULT_RUNNING			0				// factory reset == clock not running
#define DEFAULT_CLOCK_MODE		CLOCK_MODE_PID	// factory reset == PID mode turned on
#define DEFAULT_LED_BRIGHTNESS  40

#define DEFAULT_ZERO_ANGLE		0			// 0 means it's not yet set
#define DEFAULT_ZERO_ANGLE_F	0.00		// 0 means it's not yet set

#define DEFAULT_DUR_START		250

#define DEFAULT_MIN_MAX_MS		50
#define DEFAULT_RESTART_MILLIS  0

#define DEFAULT_STAT_INTERVAL	30
#define DEFAULT_SYNC_INTERVAL	3600L	// one hour
#define DEFAULT_NTP_INTERVAL	14400L	// four hours

#if WITH_VOLTAGES
	#define DEFAULT_VOLT_INTERVAL	30
	#define DEFAULT_VOLT_SCALE		1.03
	#define DEFAULT_VOLT_CUTOFF		4.00
	#define DEFAULT_VOLT_RESTORE    5.00
#endif


// what shows up on the "dashboard" UI tab

static valueIdType dash_items[] = {
	ID_START_SYNC,
	ID_RUNNING,
	ID_CLOCK_MODE,
	ID_PLOT_VALUES,
	ID_LED_BRIGHTNESS,
	ID_CLEAR_STATS,
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
	ID_TEST_MOTOR,
	ID_DIDDLE_CLOCK,
	0,
};


// what shows up on the "device" UI tab

static valueIdType device_items[] = {
	ID_START_DELAY,
	ID_ANGLE_START,
	ID_ANGLE_MIN,
	ID_ANGLE_MAX,
	ID_DEAD_ZONE,
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
	ID_RESTART_MILLIS,
	ID_DIAG_CYCLE_RANGE,
	ID_DIAG_ERROR_RANGE,
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
	ID_LOW_POWER_ENABLE,
#endif
	0
};


#define CLOCK_MODE_SENSOR_TEST		0
#define CLOCK_MODE_POWER_MIN		1
#define CLOCK_MODE_POWER_MAX        2
#define CLOCK_MODE_ANGLE_START	    3
#define CLOCK_MODE_ANGLE_LOW 		4
#define CLOCK_MODE_ANGLE_HIGH		5
#define CLOCK_MODE_MIN_MAX			6
#define CLOCK_MODE_PID              7

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

const valDescriptor theClock::m_clock_values[] =
{
    { ID_DEVICE_NAME,      VALUE_TYPE_STRING,    VALUE_STORE_PREF,     VALUE_STYLE_REQUIRED,   NULL,   NULL,   THE_CLOCK },
        // DEVICE_NAME overrides base class element

	{ ID_START_SYNC,      	VALUE_TYPE_BOOL,     VALUE_STORE_PUB,      VALUE_STYLE_NONE,       (void *) &_start_sync,	(void *) onStartSyncChanged, },
	{ ID_RUNNING,      		VALUE_TYPE_BOOL,     VALUE_STORE_PUB,      VALUE_STYLE_NONE,       (void *) &_clock_running,(void *) onClockRunningChanged, },
	{ ID_CLOCK_MODE,      	VALUE_TYPE_ENUM,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_clock_mode, 	(void *) onClockModeChanged, 	{ .enum_range = { DEFAULT_CLOCK_MODE, clockAllowed }} },
	{ ID_PLOT_VALUES,      	VALUE_TYPE_ENUM,     VALUE_STORE_PUB,      VALUE_STYLE_NONE,       (void *) &_plot_values, 	(void *) onPlotValuesChanged,   { .enum_range = { 0, plotAllowed }} },
	{ ID_LED_BRIGHTNESS,  	VALUE_TYPE_INT,    	 VALUE_STORE_PREF,     VALUE_STYLE_NONE,   	   (void *) &_led_brightness,(void *) onBrightnessChanged, 	{ .int_range = { DEFAULT_LED_BRIGHTNESS,  	0,  254}} },

	{ ID_SET_ZERO_ANGLE,  	VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    (void *) setZeroAngle },
	{ ID_ZERO_ANGLE,  		VALUE_TYPE_INT,    	 VALUE_STORE_PREF,     VALUE_STYLE_READONLY,   (void *) &_zero_angle,	NULL,  { .int_range = { DEFAULT_ZERO_ANGLE,  	0,  4095}} },
	{ ID_ZERO_ANGLE_F,  	VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_READONLY,   (void *) &_zero_angle_f,	NULL,  { .float_range = { DEFAULT_ZERO_ANGLE_F, 0,  360}} },
	{ ID_DEAD_ZONE,  		VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dead_zone,	NULL,  { .float_range = { DEFAULT_DEAD_ZONE,      0,  OVER_MAX_ANGLE}} },
	{ ID_ANGLE_START,  		VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_angle_start,	NULL,  { .float_range = { DEFAULT_ANGLE_START,   0,  OVER_MAX_ANGLE}} },
	{ ID_ANGLE_MIN,  		VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_angle_min,	NULL,  { .float_range = { DEFAULT_ANGLE_MIN,   0,  OVER_MAX_ANGLE}} },
	{ ID_ANGLE_MAX,  		VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_angle_max,	NULL,  { .float_range = { DEFAULT_ANGLE_MAX,   0,  OVER_MAX_ANGLE}} },

	{ ID_POWER_MIN,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_min,	NULL,  { .int_range = { DEFAULT_POWER_MIN,  	0,  255}} },
	{ ID_POWER_PID,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_pid,	NULL,  { .int_range = { DEFAULT_POWER_PID,   	0,  255}} },
	{ ID_POWER_MAX,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_max,	NULL,  { .int_range = { DEFAULT_POWER_MAX,   	0,  255}} },
	{ ID_POWER_START,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_start,	NULL,  { .int_range = { DEFAULT_POWER_START,   	0,  255}} },

	{ ID_DUR_PULSE,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dur_pulse,	NULL,  { .int_range = { DEFAULT_DUR_PULSE,   	0,  1000}} },
	{ ID_DUR_START,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dur_start,	NULL,  { .int_range = { DEFAULT_DUR_START,   	0,  1000}} },

	{ ID_PID_P,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_P,		NULL,  { .float_range = { DEFAULT_PID_P,   	-1000,  1000}} },
	{ ID_PID_I,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_I,		NULL,  { .float_range = { DEFAULT_PID_I,    -1000,  1000}} },
	{ ID_PID_D,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_D,		NULL,  { .float_range = { DEFAULT_PID_D,   	-1000,  1000}} },

	{ ID_APID_P,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_apid_P,		NULL,  { .float_range = { DEFAULT_APID_P,   -1000,  1000}} },
	{ ID_APID_I,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_apid_I,		NULL,  { .float_range = { DEFAULT_APID_I,   -1000,  1000}} },
	{ ID_APID_D,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_apid_D,		NULL,  { .float_range = { DEFAULT_APID_D,   -1000,  1000}} },

	{ ID_RUNNING_ANGLE,  	VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_running_angle, NULL, { .float_range = { DEFAULT_RUNNING_ANGLE, 0, 12}} },
	{ ID_RUNNING_ERROR,  	VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_running_error, NULL, { .float_range = { DEFAULT_RUNNING_ERROR, 1.0, 100}} },
	{ ID_MIN_MAX_MS,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,   	   (void *) &_min_max_ms,	 NULL, { .int_range = { DEFAULT_MIN_MAX_MS,   	    10, 1000}} },
	{ ID_RESTART_MILLIS,  	VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_restart_millis,NULL, { .int_range = { DEFAULT_RESTART_MILLIS,   	0,  60000}} },
	{ ID_START_DELAY,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,  	   (void *) &_start_delay,	 NULL, { .int_range = { DEFAULT_START_DELAY,   		-5000,  5000}} },

	{ ID_DIAG_CYCLE_RANGE,  VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,   	   (void *) &_cycle_range,	 NULL, { .int_range = { DEFAULT_CYCLE_RANGE,   	    10, 1000}} },
	{ ID_DIAG_ERROR_RANGE,  VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,   	   (void *) &_error_range,	 NULL, { .int_range = { DEFAULT_ERROR_RANGE,   	    10, 5000}} },

#if WITH_VOLTAGES
	{ ID_VOLT_INTERVAL,  	VALUE_TYPE_INT,       VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,  (void *) &_volt_interval, 	NULL,  { .int_range = { DEFAULT_VOLT_INTERVAL, 0, 86400}} },
	{ ID_VOLT_CALIB,      	VALUE_TYPE_FLOAT,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,      (void *) &_volt_calib, 		NULL,  { .float_range = { DEFAULT_VOLT_SCALE,  0,  2}} },
	{ ID_VOLT_CUTOFF,      	VALUE_TYPE_FLOAT,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,  	   (void *) &_volt_cutoff, 		NULL,  { .float_range = { DEFAULT_VOLT_CUTOFF,  0,  120}} },
	{ ID_VOLT_RESTORE,      VALUE_TYPE_FLOAT,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,  	   (void *) &_volt_restore, 	NULL,  { .float_range = { DEFAULT_VOLT_RESTORE,  0,  120}} },
	{ ID_LOW_POWER_ENABLE,  VALUE_TYPE_BOOL,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,      (void *)&_low_power_enable,  NULL,  },
#endif

	{ ID_CLEAR_STATS,       VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    (void *) clearStats },

	{ ID_STAT_MSG0,      	VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_msg0, },
	{ ID_STAT_MSG1,      	VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_msg1, },
	{ ID_STAT_MSG2,      	VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_msg2, },
	{ ID_STAT_MSG3,      	VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_msg3, },
	{ ID_STAT_MSG4,      	VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_msg4, },
	{ ID_STAT_MSG5,      	VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_msg5, },
	{ ID_STAT_MSG6,      	VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_msg6, },

	{ ID_STAT_INTERVAL,  	VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_stat_interval, 	NULL,  { .int_range = { DEFAULT_STAT_INTERVAL,1,3000000L}} },
	{ ID_SYNC_INTERVAL,  	VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_sync_interval, 	NULL,  { .int_range = { DEFAULT_SYNC_INTERVAL,1,3000000L}} },
	{ ID_SYNC_RTC,  		VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    	(void *) onSyncRTC },

#if CLOCK_WITH_NTP
	{ ID_NTP_INTERVAL,  	VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_ntp_interval, 	NULL,  { .int_range = { DEFAULT_NTP_INTERVAL,1,3000000L}} },
	{ ID_SYNC_NTP,  		VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    	(void *) onSyncNTP },
#endif

	{ ID_TEST_MOTOR,  		VALUE_TYPE_INT,    	 VALUE_STORE_PUB,      VALUE_STYLE_NONE,   	   (void *) &_test_motor,		(void *) onTestMotor,  { .int_range = { 0, -1, 1}} },
	{ ID_DIDDLE_CLOCK,  	VALUE_TYPE_INT,    	 VALUE_STORE_PUB,      VALUE_STYLE_NONE,   	   (void *) &_diddle_clock,		(void *) onDiddleClock,  { .int_range = { 0, -3000000L, 3000000L}} },
};


#define NUM_CLOCK_VALUES (sizeof(m_clock_values)/sizeof(valDescriptor))


//--------------------------------------------------
// params in this file, working vars in cpp
//--------------------------------------------------
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
uint32_t theClock::_restart_millis;
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

int 	theClock::_test_motor;
int		theClock::_diddle_clock;


// ctor

theClock::theClock()
{
    addValues(m_clock_values,NUM_CLOCK_VALUES);
    setTabLayouts(dash_items,device_items);
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
