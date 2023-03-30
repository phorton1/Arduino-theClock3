#pragma once

#include <Arduino.h>
#include <myIOTDevice.h>

#define WITH_HALL    0


//---------------------------------
// pin assignments
//---------------------------------
// L293D motor driver


#define PIN_IN1		27
#define PIN_IN2		25
#define PIN_EN		26

// Hall Sensor (no longer used)

#if WITH_HALL
	#define PIN_HALL	35
#endif


// Rotary Sensor(s) uses Wire Defaults
//
// #define PIN_SDA	21
// #define PIN_SCL	22


// Optical Mouse Sensor (unused at this time)

#define PIN_SCK		32
#define PIN_SDIO	33

// Leds and Buttons

#if 1
	#define PIN_BUTTON1 16	// RX2
	#define PIN_BUTTON2 17	// TX2
	#define PIN_LEDS	23	// overuse MOSI
#else	// previous version (built obsolete board)
	#define PIN_BUTTON1 15
	#define PIN_BUTTON2 16	// RX2
	#define PIN_LEDS	17	// TX2
#endif

#define PIXEL_MAIN		4

#define NUM_PIXELS		5


//----------------------------------
// available LED colors
//----------------------------------

#define MY_LED_BLACK    0x000000
#define MY_LED_RED      0x440000
#define MY_LED_GREEN    0x003300
#define MY_LED_BLUE     0x000044
#define MY_LED_CYAN     0x003333
#define MY_LED_YELLOW   0x333300
#define MY_LED_MAGENTA  0x330033
#define MY_LED_WHITE    0x444444
#define MY_LED_ORANGE   0x402200
#define MY_LED_REDMAG   0x400022
#define MY_LED_BLUECYAN 0x002240


//--------------------------------
// from nptTime.cpp
//--------------------------------

extern uint32_t getNtpTime();


//------------------------
// theClock definition
//------------------------

#define ID_RUNNING			"RUNNING"
#define ID_PID_MODE			"PID_MODE"
#define ID_PLOT_VALUES		"PLOT_VALUES"

#define ID_SET_ZERO_ANGLE	"SET_ZERO_ANGLE"
#define ID_ZERO_ANGLE		"ZERO_ANGLE"
#define ID_ZERO_ANGLE_F		"ZERO_ANGLE_F"
#define ID_DEAD_ZONE		"DEAD_ZONE"
#define ID_TARGET_ANGLE		"TARGET_ANGLE"

#if WITH_HALL
	#define ID_CALIBRATE_HALL	"CALIBRATE_HALL"
	#define ID_HALL_ZERO		"HALL_ZERO"
	#define ID_HALL_THRESH		"HALL_THRESH"
#endif

#define ID_POWER_LOW      	"POWER_LOW"
#define ID_POWER_HIGH      	"POWER_HIGH"
#define ID_POWER_MAX      	"POWER_MAX"
#define ID_POWER_START      "POWER_START"

#define ID_DUR_LEFT      	"DUR_LEFT"
#define ID_DUR_RIGHT      	"DUR_RIGHT"
#define ID_DUR_START		"DUR_START"

#define ID_PID_P			"PID_P"
#define ID_PID_I			"PID_I"
#define ID_PID_D			"PID_D"

#define ID_CLEAR_STATS		"CLEAR_STATS"

#define ID_CUR_TIME			"CUR_TIME"
#define ID_TIME_LAST_START  "LAST_START"
#define ID_STAT_RUNTIME		"TIME_RUNNING"
#define ID_STAT_BEATS		"BEATS"
#define ID_STAT_RESTARTS	"RESTARTS"
#define ID_STAT_STALLS_L	"STALLS_LEFT"
#define ID_STAT_STALLS_R	"STALLS_RIGHT"
#define ID_STAT_ERROR_L		"ERROR_LOW"
#define ID_STAT_ERROR_H		"ERROR_HIGH"
#define ID_STAT_DUR_L		"DUR_LOW"
#define ID_STAT_DUR_H		"DUR_HIGH"
#define ID_MIN_POWER_USED	"MIN_POWER_USED"
#define ID_MAX_POWER_USED	"MAX_POWER_USED"

#define ID_TEST_MOTOR		"MOTOR"
	// A CONFIGURATION COMMAND TO TEST THE MOTOR
	// value is only kept in memory and used once
	// Will call motor(direction,POWER_LOW) directly!!
#define ID_STAT_INTERVAL	"STAT_INTERVAL"



class theClock : public myIOTDevice
{
public:

    theClock();
    ~theClock() {}

    virtual void setup() override;
	virtual void loop() override;

private:

    static const valDescriptor m_clock_values[];

	static bool _clock_running;
	static bool _pid_mode;
	static uint32_t _plot_values;

	static int _zero_angle;			// actual used value is in as5600 units
	static float _zero_angle_f;		// displayed value is a float
	static float _dead_zone;		// degrees dead for pushing about zero
	static float _target_angle;		// target angle for testing (if not zero)

#if WITH_HALL
	static int _hall_zero;
	static int _hall_thresh;
#endif

	static int _test_motor;		// memory only, only happens onChange

	static int _power_low;		// STATIC: power when sensor reached; PID: mininum power
	static int _power_high;		// STATIC: power when sensor not reached;  PID: starting power
	static int _power_max;      // PID: maximum power
	static int _power_start;    // power during startup pulse

	static int _dur_left;		// duration of left power pulse for both PID and STATIC
	static int _dur_right;		// duration of right power pulse for both PID and STATIC
	static int _dur_start;		// duration of startup pulse

	static float _pid_P;
	static float _pid_I;
	static float _pid_D;

    static uint32_t _time_last_start;
	static uint32_t _cur_time;
	static String _stat_time_running;
	static uint32_t _stat_beats;
	static uint32_t _stat_restarts;
	static uint32_t _stat_stalls_left;
	static uint32_t _stat_stalls_right;
	static int _stat_error_low;
	static int _stat_error_high;
	static int _stat_dur_low;
	static int _stat_dur_high;
	static uint32_t _min_power_used;
	static uint32_t _max_power_used;

	static uint32_t _stat_interval;

	// methods

	static void run();
	static void clockTask(void *param);

	static void startClock();
	static void stopClock();
	static void clearStats();

    static void onClockRunningChanged(const myIOTValue *desc, bool val);
    static void onPIDModeChanged(const myIOTValue *desc, bool val);
	static void onPlotValuesChanged(const myIOTValue *desc, uint32_t val);

	static void setZeroAngle();
	#if WITH_HALL
		static void calibrateHall();
	#endif

	static void onTestMotor(const myIOTValue *desc, int val);


};	// class theClock


extern theClock *the_clock;
