// theClock3.ino
//
// contains the myIOT device definition, setup() and loop()
// Note that the default factory_reset turns WIFI off.
// See /base/bat/platform.pm for compile time defines to the myIOT layer

#include "theClock3.h"
#include <myIOTLog.h>

//------------------------
// theClock definition
//------------------------

#define MAX_ANGLE  				12			// over design maximum degrees

#define THE_CLOCK             "theClock3"
#define THE_CLOCK_VERSION     "3.0"

#define DEFAULT_RUNNING			 0			// factory reset == clock not running
#define DEFAULT_PID_MODE		 1			// factory reset == PID mode turned on

#define DEFAULT_ZERO_ANGLE		0			// 0 means it's not yet set
#define DEFAULT_ZERO_ANGLE_F	0.00		// 0 means it's not yet set
#define DEFAULT_DEAD_ZONE		0.7			// dead degrees about zero
#define DEFAULT_TARGET_ANGLE 	8.0			// early pid control value

#ifdef WITH_HALL
	#define DEFAULT_HALL_ZERO		1795		// good initial guess
	#define DEFAULT_HALL_THRESH		60			// parameterized but never really changed
#endif

#define DEFAULT_POWER_LOW		120			// v1(187) STATIC: power when sensor reached; PID: mininum power
#define DEFAULT_POWER_HIGH		210			// v1(240) STATIC: power when sensor not reached;  PID: starting power
#define DEFAULT_POWER_MAX		255			// PID: maximum power
#define DEFAULT_POWER_START     255			// power during startup pulse

#define DEFAULT_DUR_LEFT		120			// v1(30) duration of left power pulse for both PID and STATIC
	// default of zero *should* be identical to V2.0 shipped to Pamela
	// a value of 60 is roughly comparable to old V1.1
#define DEFAULT_DUR_RIGHT		120			// v1(150) duration of right power pulse for both PID and STATIC
#define DEFAULT_DUR_START		120			// duration of startup pulse

#define DEFAULT_PID_P			12.0		// mostly proportional
#define DEFAULT_PID_I			0.05		// very little integral
#define DEFAULT_PID_D			-9.0		// lots of negative feedback on rate of change

#define DEFAULT_STAT_INTERVAL	0			// v2 was shipped with constant 10


// what shows up on the "dashboard" UI tab

static valueIdType dash_items[] = {
	ID_RUNNING,
	ID_PID_MODE,
	ID_PLOT_VALUES,
	ID_TARGET_ANGLE,

	ID_CLEAR_STATS,
	ID_CUR_TIME,
	ID_TIME_LAST_START,
	ID_STAT_RUNTIME,
	ID_STAT_BEATS,
	ID_STAT_RESTARTS,
	ID_STAT_STALLS_L,
	ID_STAT_STALLS_R,
	ID_STAT_ERROR_L,
	ID_STAT_ERROR_H,
	ID_STAT_DUR_L,
	ID_STAT_DUR_H,
	ID_MIN_POWER_USED,
	ID_MAX_POWER_USED,

	0,
};


// what shows up on the "device" UI tab

static valueIdType device_items[] = {
	ID_SET_ZERO_ANGLE,
	ID_ZERO_ANGLE,
	ID_ZERO_ANGLE_F,
	ID_DEAD_ZONE,
	#if WITH_HALL
		ID_CALIBRATE_HALL,
		ID_HALL_ZERO,
		ID_HALL_THRESH,
	#endif
	ID_TEST_MOTOR,
	ID_POWER_LOW,
    ID_POWER_HIGH,
	ID_POWER_MAX,
	ID_POWER_START,
    ID_DUR_LEFT,
    ID_DUR_RIGHT,
	ID_DUR_START,
	ID_PID_P,
	ID_PID_I,
	ID_PID_D,
	ID_STAT_INTERVAL,
    0
};


static enumValue plotAllowed[] = {
    "Off",
    "On",
    "Pause",
    0};




// value descriptors for theClock

const valDescriptor theClock::m_clock_values[] =
{
    { ID_DEVICE_NAME,      VALUE_TYPE_STRING,    VALUE_STORE_PREF,     VALUE_STYLE_REQUIRED,   NULL,   NULL,   THE_CLOCK },
        // DEVICE_NAME overrides base class element

	{ ID_RUNNING,      		VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_clock_running,(void *) onClockRunningChanged, { .int_range = { DEFAULT_RUNNING }} },
	{ ID_PID_MODE,      	VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_mode, 	(void *) onPIDModeChanged, 		{ .int_range = { DEFAULT_PID_MODE }} },
	{ ID_PLOT_VALUES,      	VALUE_TYPE_ENUM,     VALUE_STORE_PUB,      VALUE_STYLE_NONE,       (void *) &_plot_values, 	(void *) onPlotValuesChanged,   { .enum_range = { 0, plotAllowed }} },

	{ ID_SET_ZERO_ANGLE,  	VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    (void *) setZeroAngle },
	{ ID_ZERO_ANGLE,  		VALUE_TYPE_INT,    	 VALUE_STORE_PREF,     VALUE_STYLE_READONLY,   (void *) &_zero_angle,	NULL,  { .int_range = { DEFAULT_ZERO_ANGLE,  	0,  4095}} },
	{ ID_ZERO_ANGLE_F,  	VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_READONLY,   (void *) &_zero_angle_f,	NULL,  { .float_range = { DEFAULT_ZERO_ANGLE_F, 0,  360}} },
	{ ID_DEAD_ZONE,  		VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dead_zone,	NULL,  { .float_range = { DEFAULT_DEAD_ZONE,      0,  MAX_ANGLE}} },
	{ ID_TARGET_ANGLE,  	VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_target_angle,	NULL,  { .float_range = { DEFAULT_TARGET_ANGLE,   0,  MAX_ANGLE}} },

#if WITH_HALL
	{ ID_CALIBRATE_HALL,  	VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    (void *) calibrateHall },
	{ ID_HALL_ZERO,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_READONLY,   (void *) &_hall_zero,	NULL,  { .int_range = { DEFAULT_HALL_ZERO,  	0,  4095}} },
	{ ID_HALL_THRESH,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_hall_thresh,	NULL,  { .int_range = { DEFAULT_HALL_THRESH,  	0,  255}} },
#endif

	{ ID_TEST_MOTOR,  		VALUE_TYPE_INT,    	 VALUE_STORE_PUB,      VALUE_STYLE_NONE,   		(void *) &_test_motor,	(void *) onTestMotor,  { .int_range = { 0, -1, 1}} },

	{ ID_POWER_LOW,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_low,	NULL,  { .int_range = { DEFAULT_POWER_LOW,  	0,  255}} },
	{ ID_POWER_HIGH,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_high,	NULL,  { .int_range = { DEFAULT_POWER_HIGH,   	0,  255}} },
	{ ID_POWER_MAX,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_max,	NULL,  { .int_range = { DEFAULT_POWER_MAX,   	0,  255}} },
	{ ID_POWER_START,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_start,	NULL,  { .int_range = { DEFAULT_POWER_START,   	0,  255}} },
	{ ID_DUR_LEFT,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dur_left,		NULL,  { .int_range = { DEFAULT_DUR_LEFT,   	0,  1000}} },
	{ ID_DUR_RIGHT,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dur_right,	NULL,  { .int_range = { DEFAULT_DUR_RIGHT,   	0,  1000}} },
	{ ID_DUR_START,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dur_start,	NULL,  { .int_range = { DEFAULT_DUR_START,   	0,  255}} },

	{ ID_PID_P,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_P,		NULL,  { .float_range = { DEFAULT_PID_P,   	-1000,  1000}} },
	{ ID_PID_I,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_I,		NULL,  { .float_range = { DEFAULT_PID_I,    -1000,  1000}} },
	{ ID_PID_D,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_D,		NULL,  { .float_range = { DEFAULT_PID_D,   	-1000,  1000}} },

	{ ID_CLEAR_STATS,       VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    (void *) clearStats },
	{ ID_CUR_TIME,   		VALUE_TYPE_TIME,     VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_cur_time, },
	{ ID_TIME_LAST_START,   VALUE_TYPE_TIME,     VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_time_last_start, },
	{ ID_STAT_RUNTIME,      VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_time_running, },

	{ ID_STAT_BEATS,  		VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_beats, 		NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_RESTARTS,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_restarts, 	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_STALLS_L,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_stalls_left, NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_STALLS_R,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_stalls_right,NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_ERROR_L,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_error_low, 	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_ERROR_H,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_error_high, 	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_DUR_L,  		VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_dur_low, 	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_DUR_H,  		VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_dur_high, 	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_MIN_POWER_USED,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_min_power_used,   NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_MAX_POWER_USED,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_max_power_used, 	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },

	{ ID_STAT_INTERVAL,  	VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_stat_interval, 	NULL,  { .int_range = { DEFAULT_STAT_INTERVAL,1,3600}} },
};


#define NUM_CLOCK_VALUES (sizeof(m_clock_values)/sizeof(valDescriptor))


// static member data

bool 	theClock::_clock_running;
bool 	theClock::_pid_mode;
uint32_t theClock::_plot_values;

int  	theClock::_zero_angle;
float  	theClock::_zero_angle_f;
float   theClock::_dead_zone;
float   theClock::_target_angle;

#if WITH_HALL
	int		theClock::_hall_zero;
	int  	theClock::_hall_thresh;
#endif

int 	theClock::_test_motor;

int  	theClock::_power_low;
int  	theClock::_power_high;
int  	theClock::_power_max;
int  	theClock::_power_start;

int  	theClock::_dur_left;
int  	theClock::_dur_right;
int  	theClock::_dur_start;

float  	theClock::_pid_P;
float  	theClock::_pid_I;
float  	theClock::_pid_D;

uint32_t theClock::_cur_time;
uint32_t theClock::_time_last_start;
String   theClock::_stat_time_running;
uint32_t theClock::_stat_beats;
uint32_t theClock::_stat_restarts;
uint32_t theClock::_stat_stalls_left;
uint32_t theClock::_stat_stalls_right;
int      theClock::_stat_error_low;
int      theClock::_stat_error_high;
int      theClock::_stat_dur_low;
int      theClock::_stat_dur_high;
uint32_t theClock::_min_power_used;
uint32_t theClock::_max_power_used;

uint32_t theClock::_stat_interval;


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
