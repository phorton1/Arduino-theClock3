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

#define THE_CLOCK             "theClock3"
#define THE_CLOCK_VERSION     "3.0"

#define OVER_MAX_ANGLE  		14			// over design maximum degrees

#define DEFAULT_RUNNING			0			// factory reset == clock not running
#define DEFAULT_PID_MODE		1			// factory reset == PID mode turned on

#define DEFAULT_ZERO_ANGLE		0			// 0 means it's not yet set
#define DEFAULT_ZERO_ANGLE_F	0.00		// 0 means it's not yet set
#define DEFAULT_DEAD_ZONE		0.7			// dead degrees about zero
#define DEFAULT_TARGET_ANGLE 	8.0			// early pid control value

#define DEFAULT_POWER_MIN		120
#define DEFAULT_POWER_PID		210
#define DEFAULT_POWER_MAX		255
#define DEFAULT_POWER_PULL		210
#define DEFAULT_POWER_START     255

#define DEFAULT_DUR_LEFT		140
#define DEFAULT_DUR_RIGHT		140
#define DEFAULT_DUR_START		120

#define DEFAULT_PID_P			12.0		// mostly proportional
#define DEFAULT_PID_I			0.50		// very little integral
#define DEFAULT_PID_D			-9.0		// lots of negative feedback on rate of change

#define DEFAULT_STAT_INTERVAL	0


// what shows up on the "dashboard" UI tab

static valueIdType dash_items[] = {
	ID_RUNNING,
	ID_PID_MODE,
	ID_PULL_MODE,
	ID_PLOT_VALUES,

	ID_CLEAR_STATS,

	ID_CUR_TIME,
	ID_TIME_START,
	ID_TIME_RUNNING,
	ID_STAT_BEATS,

	ID_STAT_MIN_CYCLE,
	ID_STAT_MAX_CYCLE,
	ID_STAT_MIN_ERROR,
	ID_STAT_MAX_ERROR,

	ID_STAT_MIN_POWER,
	ID_STAT_MAX_POWER,
	ID_STAT_MIN_LEFT,
	ID_STAT_MAX_LEFT,
	ID_STAT_MIN_RIGHT,
	ID_STAT_MAX_RIGHT,

	ID_STAT_NUM_PUSH,
	ID_STAT_NUM_PULL,
	ID_STAT_PULL_RATIO,

	0,
};


// what shows up on the "device" UI tab

static valueIdType device_items[] = {
	ID_SET_ZERO_ANGLE,
	ID_ZERO_ANGLE,
	ID_ZERO_ANGLE_F,
	ID_DEAD_ZONE,
	ID_TARGET_ANGLE,
	ID_POWER_MIN,
    ID_POWER_PID,
	ID_POWER_MAX,
	ID_POWER_PULL,
	ID_POWER_START,
    ID_DUR_LEFT,
    ID_DUR_RIGHT,
	ID_DUR_START,
	ID_PID_P,
	ID_PID_I,
	ID_PID_D,
	ID_STAT_INTERVAL,
    ID_TEST_MOTOR,
	0
};


static enumValue plotAllowed[] = {
    "Off",
    "On",
    "Pause",
    0};


static enumValue pullAllowed[] = {
    "Use Both",
    "Push Only",
    "Pull Only",
    0};




// value descriptors for theClock

const valDescriptor theClock::m_clock_values[] =
{
    { ID_DEVICE_NAME,      VALUE_TYPE_STRING,    VALUE_STORE_PREF,     VALUE_STYLE_REQUIRED,   NULL,   NULL,   THE_CLOCK },
        // DEVICE_NAME overrides base class element

	{ ID_RUNNING,      		VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_clock_running,(void *) onClockRunningChanged, { .int_range = { DEFAULT_RUNNING }} },
	{ ID_PID_MODE,      	VALUE_TYPE_BOOL,     VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_mode, 	(void *) onPIDModeChanged, 		{ .int_range = { DEFAULT_PID_MODE }} },

	{ ID_PULL_MODE,      	VALUE_TYPE_ENUM,     VALUE_STORE_PUB,      VALUE_STYLE_NONE,       (void *) &_pull_mode, 	NULL,   						{ .enum_range = { 0, pullAllowed }} },
	{ ID_PLOT_VALUES,      	VALUE_TYPE_ENUM,     VALUE_STORE_PUB,      VALUE_STYLE_NONE,       (void *) &_plot_values, 	(void *) onPlotValuesChanged,   { .enum_range = { 0, plotAllowed }} },

	{ ID_SET_ZERO_ANGLE,  	VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    (void *) setZeroAngle },
	{ ID_ZERO_ANGLE,  		VALUE_TYPE_INT,    	 VALUE_STORE_PREF,     VALUE_STYLE_READONLY,   (void *) &_zero_angle,	NULL,  { .int_range = { DEFAULT_ZERO_ANGLE,  	0,  4095}} },
	{ ID_ZERO_ANGLE_F,  	VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_READONLY,   (void *) &_zero_angle_f,	NULL,  { .float_range = { DEFAULT_ZERO_ANGLE_F, 0,  360}} },
	{ ID_DEAD_ZONE,  		VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dead_zone,	NULL,  { .float_range = { DEFAULT_DEAD_ZONE,      0,  OVER_MAX_ANGLE}} },
	{ ID_TARGET_ANGLE,  	VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_target_angle,	NULL,  { .float_range = { DEFAULT_TARGET_ANGLE,   0,  OVER_MAX_ANGLE}} },

	{ ID_POWER_MIN,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_min,	NULL,  { .int_range = { DEFAULT_POWER_MIN,  	0,  255}} },
	{ ID_POWER_PID,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_pid,	NULL,  { .int_range = { DEFAULT_POWER_PID,   	0,  255}} },
	{ ID_POWER_MAX,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_max,	NULL,  { .int_range = { DEFAULT_POWER_MAX,   	0,  255}} },
	{ ID_POWER_PULL,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_pull,	NULL,  { .int_range = { DEFAULT_POWER_PULL,   	0,  255}} },
	{ ID_POWER_START,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_power_start,	NULL,  { .int_range = { DEFAULT_POWER_START,   	0,  255}} },

	{ ID_DUR_LEFT,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dur_left,		NULL,  { .int_range = { DEFAULT_DUR_LEFT,   	0,  1000}} },
	{ ID_DUR_RIGHT,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dur_right,	NULL,  { .int_range = { DEFAULT_DUR_RIGHT,   	0,  1000}} },
	{ ID_DUR_START,  		VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_dur_start,	NULL,  { .int_range = { DEFAULT_DUR_START,   	0,  255}} },

	{ ID_PID_P,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_P,		NULL,  { .float_range = { DEFAULT_PID_P,   	-1000,  1000}} },
	{ ID_PID_I,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_I,		NULL,  { .float_range = { DEFAULT_PID_I,    -1000,  1000}} },
	{ ID_PID_D,  			VALUE_TYPE_FLOAT,    VALUE_STORE_PREF,     VALUE_STYLE_NONE,       (void *) &_pid_D,		NULL,  { .float_range = { DEFAULT_PID_D,   	-1000,  1000}} },

	{ ID_CLEAR_STATS,       VALUE_TYPE_COMMAND,  VALUE_STORE_MQTT_SUB, VALUE_STYLE_NONE,       NULL,                    (void *) clearStats },

	{ ID_CUR_TIME,   		VALUE_TYPE_TIME,     VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_cur_time, },
	{ ID_TIME_START,   		VALUE_TYPE_TIME,     VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_time_start, },
	{ ID_TIME_RUNNING,      VALUE_TYPE_STRING,   VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_time_running, },
	{ ID_STAT_BEATS,  		VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_beats, 		NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_NUM_PUSH,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_num_push, 	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_NUM_PULL,  	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_num_pull, 	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },

	{ ID_STAT_PULL_RATIO,  	VALUE_TYPE_FLOAT,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_pull_ratio,	NULL,  { .float_range = { 0.5, 0, 1}} },

	{ ID_STAT_MIN_POWER,	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_min_power,	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_MAX_POWER,	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_max_power,	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_MIN_LEFT, 	VALUE_TYPE_FLOAT,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_min_left,	NULL,  { .float_range = { 0, -360, 360}} },
	{ ID_STAT_MAX_LEFT, 	VALUE_TYPE_FLOAT,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_max_left,	NULL,  { .float_range = { 0, -360, 360}} },
	{ ID_STAT_MIN_RIGHT,	VALUE_TYPE_FLOAT,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_min_right,	NULL,  { .float_range = { 0, -360, 360}} },
	{ ID_STAT_MAX_RIGHT,	VALUE_TYPE_FLOAT,    VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_max_right,	NULL,  { .float_range = { 0, -360, 360}} },
	{ ID_STAT_MIN_CYCLE,	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_min_cycle,	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_MAX_CYCLE,	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_max_cycle,	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_MIN_ERROR,	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_min_error,	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },
	{ ID_STAT_MAX_ERROR,	VALUE_TYPE_INT,      VALUE_STORE_PUB,      VALUE_STYLE_READONLY,   (void *) &_stat_max_error,	NULL,  { .int_range = { 0, -DEVICE_MAX_INT-1,DEVICE_MAX_INT}} },

	{ ID_STAT_INTERVAL,  	VALUE_TYPE_INT,      VALUE_STORE_PREF,     VALUE_STYLE_OFF_ZERO,   (void *) &_stat_interval, 	NULL,  { .int_range = { DEFAULT_STAT_INTERVAL,1,3600}} },
	{ ID_TEST_MOTOR,  		VALUE_TYPE_INT,    	 VALUE_STORE_PUB,      VALUE_STYLE_NONE,   		(void *) &_test_motor,	(void *) onTestMotor,  { .int_range = { 0, -1, 1}} },
};


#define NUM_CLOCK_VALUES (sizeof(m_clock_values)/sizeof(valDescriptor))


// static member data

bool 	theClock::_clock_running;
bool 	theClock::_pid_mode;
uint32_t theClock::_pull_mode;
uint32_t theClock::_plot_values;

int  	theClock::_zero_angle;
float  	theClock::_zero_angle_f;
float   theClock::_dead_zone;
float   theClock::_target_angle;


int  	theClock::_power_min;
int  	theClock::_power_pid;
int  	theClock::_power_max;
int  	theClock::_power_pull;
int  	theClock::_power_start;

int  	theClock::_dur_left;
int  	theClock::_dur_right;
int  	theClock::_dur_start;

float  	theClock::_pid_P;
float  	theClock::_pid_I;
float  	theClock::_pid_D;

uint32_t theClock::_cur_time;
uint32_t theClock::_time_start;
String 	 theClock::_time_running;
uint32_t theClock::_stat_beats;
uint32_t theClock::_stat_num_push;
uint32_t theClock::_stat_num_pull;
float	 theClock::_stat_pull_ratio;
uint32_t theClock::_stat_min_power;
uint32_t theClock::_stat_max_power;
float	 theClock::_stat_min_left;
float	 theClock::_stat_max_left;
float	 theClock::_stat_min_right;
float	 theClock::_stat_max_right;
int32_t	 theClock::_stat_min_cycle;
int32_t	 theClock::_stat_max_cycle;
int32_t  theClock::_stat_min_error;
int32_t  theClock::_stat_max_error;

uint32_t theClock::_stat_interval;
int 	 theClock::_test_motor;


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
