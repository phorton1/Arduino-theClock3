#pragma once

#include <myIOTDevice.h>

// #define CLOCK1_WV3SENSOR
	// Shoe-horned old v1.1 clock into v3 architecture by adding AS5600 sensor to it.
	// I didn't want the clock to just die, when perhaps with 1 day work, I could bring
	// it up to reasonable performance with V3 code and behavior.
	//
	// That circuit board had separate ENA, ENB, INA, and INB pins, one button, and
	// was retrofitted to have 5 leds.  Dunno why, but whereas clock 3 runs on virtually
	// any (the laptop) power supply, v1.3 REQUIRES a 3A power supply.  Because it only
	// has one button, you cannot change LED_BRIGHTNESS, PIXEL_MODE, or Set the Zero Angle
	// via the button and need to use the WebUI to do those things.

//---------------------------------
// pin assignments
//---------------------------------
// L293D motor driver

#ifdef CLOCK1_WV3SENSOR
	#define PIN_ENA		27
	#define PIN_INA1	25
	#define PIN_INA2	26
	#define PIN_ENB		4
	#define PIN_INB1	17
	#define PIN_INB2	5
#else
	#define PIN_IN1		27
	#define PIN_IN2		25
	#define PIN_EN		26
#endif


#ifdef CLOCK1_WV3SENSOR
	#define PIN_SDA		13
	#define PIN_SCL		32
#else
	// v3 Rotary Sensor(s) uses Wire Defaults
	// #define PIN_SDA	21
	// #define PIN_SCL	22
#endif

// Optical Mouse Sensor (unused at this time)
//
// #define PIN_SCK		32
// #define PIN_SDIO	33

// Leds and Buttons

#ifdef CLOCK1_WV3SENSOR
		#define PIN_BUTTON1 18
		#define PIN_LEDS	22
#else
	#if 1
		#define PIN_BUTTON1 16	// RX2
		#define PIN_BUTTON2 17	// TX2
		#define PIN_LEDS	23	// overuse MOSI
	#else	// reminder of previous built unused pcb
		#define PIN_BUTTON1 15
		#define PIN_BUTTON2 16	// RX2
		#define PIN_LEDS	17	// TX2
	#endif
#endif

// Pixels

#define PIXEL_MAIN		4
#define PIXEL_STATE     3
#define PIXEL_ACCURACY  2
#define PIXEL_CYCLE     1
#define PIXEL_SYNC      0

#define NUM_PIXELS		5


//----------------------------------
// available LED colors
//----------------------------------

#define MY_LED_BLACK    0x000000
#define MY_LED_RED      0x440000
#define MY_LED_GREEN    0x003300
#define MY_LED_BLUE     0x000044
#define MY_LED_CYAN     0x003322
#define MY_LED_YELLOW   0x333300
#define MY_LED_MAGENTA  0x440044
#define MY_LED_WHITE    0x444444
#define MY_LED_ORANGE   0x402200
#define MY_LED_REDMAG   0x400022
#define MY_LED_PURPLE   0x220022
#define MY_LED_BLUECYAN 0x002240


//--------------------------------
// from nptTime.cpp
//--------------------------------

#define CLOCK_WITH_NTP   1

#if CLOCK_WITH_NTP
	extern bool getNtpTime(int32_t *secs, int32_t *ms);
	extern void syncNTPTime();
#endif


//------------------------
// theClock definition
//------------------------

#define ID_START_CLOCK		"START_CLOCK"

#define ID_RUNNING			"RUNNING"
#define ID_CLOCK_MODE		"CLOCK_MODE"
#define ID_PLOT_VALUES		"PLOT_VALUES"
#define ID_PIXEL_MODE		"PIXEL_MODE"
#define ID_LED_BRIGHTNESS	"LED_BRIGHTNESS"

#define ID_SET_ZERO_ANGLE	"SET_ZERO_ANGLE"
#define ID_ZERO_ANGLE		"ZERO_ANGLE"
#define ID_ZERO_ANGLE_F		"ZERO_ANGLE_F"
#define ID_DEAD_ZONE		"DEAD_ZONE"
#define ID_ANGLE_START		"ANGLE_START"
#define ID_ANGLE_MIN		"ANGLE_MIN"
#define ID_ANGLE_MAX		"ANGLE_MAX"

#define ID_POWER_MIN      	"POWER_MIN"
#define ID_POWER_PID      	"POWER_PID"
#define ID_POWER_MAX      	"POWER_MAX"
#define ID_POWER_START      "POWER_START"

#define ID_DUR_PULSE      	"DUR_PULSE"
#define ID_DUR_START		"DUR_START"

#define ID_PID_P			"PID_P"
#define ID_PID_I			"PID_I"
#define ID_PID_D			"PID_D"

#define ID_APID_P			"APID_P"
#define ID_APID_I			"APID_I"
#define ID_APID_D			"APID_D"

#define ID_RUNNING_ANGLE	"RUNNING_ANGLE"
#define ID_RUNNING_ERROR	"RUNNING_ERROR"
#define ID_MIN_MAX_MS		"MIN_MAX_MS"
#define ID_RESTART_MILLIS	"RESTART_MILLIS"
#define ID_START_DELAY		"START_DELAY"

#define ID_CLEAR_STATS		"CLEAR_STATS"

#define ID_STAT_MSG0		"STAT_MSG0"
#define ID_STAT_MSG1		"STAT_MSG1"
#define ID_STAT_MSG2		"STAT_MSG2"
#define ID_STAT_MSG3		"STAT_MSG3"
#define ID_STAT_MSG4		"STAT_MSG4"
#define ID_STAT_MSG5		"STAT_MSG5"
#define ID_STAT_MSG6		"STAT_MSG6"


#define ID_STAT_INTERVAL	"STAT_INTERVAL"
#define ID_SYNC_INTERVAL	"SYNC_INTERVAL"
#define ID_SYNC_RTC			"SYNC_RTC"

#if CLOCK_WITH_NTP
	#define ID_NTP_INTERVAL	"NTP_INTERVAL"
	#define ID_SYNC_NTP		"SYNC_NTP"
#endif



#define ID_TEST_MOTOR		"MOTOR"
	// A CONFIGURATION COMMAND TO TEST THE MOTOR
	// value is only kept in memory and used once
	// Will call motor(direction,POWER_LOW) directly!!
#define ID_DIDDLE_CLOCK		"DIDDLE_CLOCK"
	// Will add given number of seconds to ESP32 clock
	// for testing sync code

// enumerated type values

#define CLOCK_STATE_NONE    	0
#define CLOCK_STATE_STATS   	1
#define CLOCK_STATE_START		2
#define CLOCK_STATE_STARTED		3
#define CLOCK_STATE_RUNNING		4

#define CLOCK_MODE_SENSOR_TEST	0
#define CLOCK_MODE_POWER_MIN	1
#define CLOCK_MODE_POWER_MAX    2
#define CLOCK_MODE_ANGLE_START	3
#define CLOCK_MODE_ANGLE_MIN 	4
#define CLOCK_MODE_ANGLE_MAX	5
#define CLOCK_MODE_MIN_MAX		6
#define CLOCK_MODE_PID          7

#define PLOT_OFF				0
#define PLOT_WAVES				1
#define PLOT_PAUSE				2
#define PLOT_CLOCK				3

#define PIXEL_MODE_OFF			0
#define PIXEL_MODE_DIAG			1
#define PIXEL_MODE_TIME			2


// theClock declaration

class theClock : public myIOTDevice
{
public:

    theClock();
    ~theClock() {}

    virtual void setup() override;
	virtual void loop() override;

private:

	// myIOT parameters

    static const valDescriptor m_clock_values[];

	static bool _clock_running;		// user interface variable !! (as opposed to control variable)
	static uint32_t _clock_mode;
	static uint32_t _plot_values;
	static uint32_t _pixel_mode;
	static int _led_brightness;

	static int _zero_angle;			// actual used value is in as5600 units
	static float _zero_angle_f;		// displayed value is a float
	static float _dead_zone;		// degrees dead for pushing about zero
	static float _angle_start;		// target angle for clock
	static float _angle_min;
	static float _angle_max;

	static int _power_min;			// PID mininum power, also power for !PID and motor test
	static int _power_pid;			// PID starting power
	static int _power_max;      	// PID maximum power
	static int _power_start;    	// power during startup pulse

	static int _dur_pulse;			// duration of power pulse for both PID and STATIC
	static int _dur_start;			// duration of startup pulse

	static float _pid_P;			// PID controller values
	static float _pid_I;
	static float _pid_D;

	static float _apid_P;			// PID controller values
	static float _apid_I;
	static float _apid_D;

	static float _running_angle;		// minimum angle for clock to be considered "running"
	static float _running_error;		// minimum accumulated angular error for clock to be considered running
	static int 	 _min_max_ms;			// ms threshold for min/max algorithm
	static uint32_t _restart_millis;	// millis for automatic restart (0 == off)
	static uint32_t _start_delay;

	static String 	_stat_msg0;		// messages
	static String 	_stat_msg1;
	static String 	_stat_msg2;
	static String 	_stat_msg3;
	static String 	_stat_msg4;
	static String 	_stat_msg5;
	static String 	_stat_msg6;

	static uint32_t _stat_interval;	// interval, in beats/seconds for statistic to be displayed
	static uint32_t _sync_interval;	// interval, in beats/seconds for beats vs ESP32 clock check
#if CLOCK_WITH_NTP
	static uint32_t _ntp_interval;	// interval, in beats/seconds for NTP vs ESP32 clock check
#endif

	static int _test_motor;		// memory only, only happens onChange
	static int _diddle_clock;	// memory only, only happens onChange


	// clock paramters

	static int 	 	m_clock_state;
	static bool 	m_start_sync;			// doing a synchronized start
	static uint32_t m_last_change;			// millis of last noticable pendulum movement
	static  int32_t m_cur_cycle;			// millis in this 'cycle' (forward zero crossing)
	static uint32_t m_last_cycle;			// millis at previous forward zero crossing
	static uint32_t m_num_beats;			// number of beats (while clock_started && !initial_pulse_time)

	static uint32_t m_time_start;
	static uint32_t m_time_start_ms;
	static uint32_t m_time_zero;			// ESP32 RTC time at zero crossing
	static uint32_t m_time_zero_ms;			// with milliseconds (from microseconds)
	static uint32_t m_time_init;
	static uint32_t m_time_init_ms;

	static int32_t  m_total_millis_error;	// cumulative number of millis total error
	static int32_t  m_prev_millis_error;
	static float    m_pid_angle;			// angle determined by second PID controller or other parameters

	static float 	m_total_ang_error;		// accumluated degrees of error (for "I")
	static float 	m_prev_ang_error;		// the previous error (for "D")
	static int 	 	m_pid_power;			// power adjusted by pid algorithm

	static int 		m_sync_sign;			// if in a sync, the sign of sync_millis, else zero
	static int 		m_sync_millis;			// if in a sync, the amount to sync, else zero

	static uint32_t m_initial_pulse_time;	// time at which we started initial clock starting pulse (push)
	static bool 	m_push_motor;			// push the pendulum next time after it leaves deadzone (determined at zero crossing)
	static uint32_t m_motor_start;
	static uint32_t m_motor_dur;

	static uint32_t m_last_beat;
	static uint32_t m_last_stats;
	static uint32_t m_last_sync;
	static uint32_t m_last_ntp;

	static bool		m_update_stats;


	// UI methods

	static void onStartClockSynchronized();
    static void onClockRunningChanged(const myIOTValue *desc, bool val);
    static void onClockModeChanged(const myIOTValue *desc, uint32_t val);
	static void onPlotValuesChanged(const myIOTValue *desc, uint32_t val);
	static void onPixelModeChanged(const myIOTValue *desc, uint32_t val);
	static void onBrightnessChanged(const myIOTValue *desc, uint32_t val);
	static void clearStats();
	static void setZeroAngle();
	static void onSyncRTC();

	#if CLOCK_WITH_NTP
		static void onSyncNTP();
	#endif

	// UI test methods

	static void onTestMotor(const myIOTValue *desc, int val);
	static void onDiddleClock(const myIOTValue *desc, int val);

	// Internal Private methods

	static void initMotor();
	static void initStats(bool restart);
	static void motor(int state, int power);

	static void run();
	static void clockTask(void *param);
	static void startClock(bool restart=0);
	static void stopClock();

	static float getPidAngle();
	static int getPidPower(float avg_angle);

	void doPixels();
	void doButtons();

};	// class theClock


extern theClock *the_clock;
