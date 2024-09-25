# theClock3 - Software

**[Home](readme.md)** --
**[Design](design.md)** --
**[Plan](plan.md)** --
**[Wood](wood.md)** --
**[Coils](coils.md)** --
**[Electronics](electronics.md)** --
**[Firmware](firmware.md)** --
**[Assemble](assemble.md)** --
**[Build](build.md)** --
**[Tuning](tuning.md)** --
**[UI](ui.md)** --
**Software** --
**[Troubles](troubles.md)** --
**[Notes](notes.md)**

This *reference* page gives an **Overview** of the software, and
list all of the [**Parameters and Commands**](#b-parameters-and-commands)
available on the clock.




## A. Software Overview

The software is divided into a number of separate **classes** and **files**.

- **theClock3.ino** - contains the main *Arduino Sketch*, with the main **setup()** and **loop()**
  methods, and the *static declarations* of the *user interface* and **parameters**
  of the **theClock** *myIOTDevice*
- **theClock3.h** - contains all **common defines** and the class declaration of
  of the **theClock** *myIOTDevice*
- **theClock3.cpp** - contains much of the **implementation**
  of the **theClock** *myIOTDevice*
- **theClockRun.cpp** - contains the **run()** method
  for the **theClock** *myIOTDevice*, which is called as
  an ESP32 **task** and contains the **time critical** functions
  for running the clock.
- **theClockLoop.cpp** - contains the **loop()** method
  for the **theClock** *myIOTDevice*, which is called from the
  main Arduino loop() method, and which includes the *non time critical*
  functions for handling **pixels** and **buttons**
- **clockPixels.h and cpp** - are wrappers around the *Adafruit Neopixels* library
  that minimize it's API, and that also add functions like *blending and averaging* pixels
- **clockAS5600.h and cpp** - are wrappers around the *AS5600* library
  that abstract out it's functionality into a minimized API
- **clockStats.h and cpp** - contain static variables and methods for
  generating and maintaining **statistics** about how the clock is running
  and to aid in presenting them to the **WebUI**
- **nptTime.cpp** contains code to send and receive **UDP** packets for obtaining
  the **NTP** (Network Time Protocol) time with *Seconds and Milliseconds* accuracy

### 1. Clock State Machine

The clock can be in four possible states:

- **NONE** - doing nothing, just sitting there,
- **START** - in the process of delivering a single initial *impulse* to start the pendulum moving
- **STARTED** - ticking, but not yet necessarily stabilized at a given *target angle*
- **RUNNING** - ticking and/or having achieved the desired *target angle*

```
#define CLOCK_STATE_NONE      0
#define CLOCK_STATE_START     1
#define CLOCK_STATE_STARTED   2
#define CLOCK_STATE_RUNNING   3
```

How it moves between these states is determined by the **clock mode** described below.

### 2. Clock Modes

By default, the clock is set to run in the **PID Mode** which, as described
in the [*Basic Design*](design.md), works to keep the clock running at, or near,
the correct time.

However, as the **maker** of one of these clocks, there are a lot of other things
you will want, and need, to do, and know, about the clock.  For instance, we will
want to start by just seeing if the clock can deliver impulses to the pendulum
and make it move.  Then we will want to ensure that it can run reliably at
the *minimum angle* before we *tune* it's period to just over 1000ms,
add the *magnetic spring* and make sure it runs at under 1000ms at
the *maximum angle* when the spring comes into play, and so on.

Therefore the clock supports the following **modes** of operation:

```
#define CLOCK_MODE_SENSOR_TEST  0
#define CLOCK_MODE_POWER_MIN    1
#define CLOCK_MODE_POWER_MAX    2
#define CLOCK_MODE_ANGLE_START  3
#define CLOCK_MODE_ANGLE_MIN    4
#define CLOCK_MODE_ANGLE_MAX    5
#define CLOCK_MODE_MIN_MAX      6
#define CLOCK_MODE_PID          7
```

In all of the modes except *SENSOR TEST* an *initial impulse* is delivered to the
pendulum to start it moving.

- **SENSOR_TEST** - no *impulses* are sent to the pendulum, but the *serial port* reports
  the RTC seconds as they occur and shows **any movements of the pendulum** as determined
  by the *AS5600 angle sensor*.  This mode goes directly to **CLOCK_STATE_RUNNING** as
  soon as the clock is **started** and is used to verify, when moving the pendulum by hand,
  that the *angle sensor is working*.

The next two modes don't use any **PID Controllers**.  They just deliver the initial
impulse (CLOCK_STATE_START), skip directly to CLOCK_STATE_RUNNING and start delivering
constant *impulses* at each *zero crossing* as as determined by the **PULSE_DUR** (pulse duration)
*parameter*.

- **POWER_MIN** - At each *zero crossing* we merely deliver an impulse to the pendulum
   determined by the **POWER_MIN** parameter.  This is the first step used to make sure we don't blow
   up the clock and that it will tick at the minimum specified power. The clock *should barely*
   tick and tock reliably at this setting.
- **POWER_MAX** - We **briefly** test the pendulum at **POWER_MAX** which *should* cause it
   to quickly start *banging* the pendulum from side to side against the limits of the clock.

The next three modes bring in the **Primary PID Controller** and test whether or not
the clock can maintain the starting, minimum, and maximum *target angles*.

- **ANGLE_START** - set the *target angle* to the **ANGLE_START** parameter
- **ANGLE_MIN** - set the *target angle* to the **ANGLE_MIN** parameter
- **ANGLE_MAX** - set the *target angle* to the **ANGLE_MAX** parameter

The above 3 modes deliver the *initial impulse* with the CLOCK_STATE_START,
and then enter the CLOCK_STATE_STARTED as the **primary PID Controller**
attempts to stabilize the pendulum at the given *target angle*.  This
usually takes about 30 seconds as the controller attempts to minimize
the instantaneous and cumulative **angular error** of the pendulum.

There are another set of parameters, RUNNING_ANGLE (default: 4 degrees)
and RUNNING_ERROR (default: 2 degrees), that determine when the clock
changes to CLOCK_STATE_RUNNING.  When the clock has started to swing
at least RUNNING_ANGLE (about the center) **AND** the cumulative
angular error has dropped below RUNNING_ERROR (absolute), then the
clock is considered to be **RUNNING**.

Finally, there are two ways that the clock can then attempt to
vary the *target angle* so as to keep accurate time based on
the instantaneous and cumulative **milliseconds error**.

- **MIN_MAX** -	uses a simple algorithm based on the ID_MIN_MAX_MS parameter,
  which defaults to **50 milliseconds**.  When the **cumulative ms error**
  is *greater* than MIN_MAX_MS (the clock is running slow), we set the
  *target angle* to ANGLE_MAX so that it will swing wider, start hitting
  the *magnetic spring* and speed up.  When the **cumulative ms error**
  is *less* than -MIN_MAX_MS (negative means the clock is running fast),
  we set the *target angle* to ANGLE_MIN so that it will swing less and
  beat slower.  By alternating between these two *target angles* the
  clock keeps *relatively accurate* time.

- **PID** - And then finally we have the (default) **PID mode** which
  introduces the **secondary PID controller** which attempts to optimize
  the *target angle* so that the clock beats as close to **1000 ms per
  beat** as possible by minimizing the instantaneous and cumulative
  *milliseconds errors* simultaneously.


## B. Parameters and Commands

Many parameters can be *modified* to *experiment* with, or possibly to *correct problems*
with the clock.

### 1. Clock Specific

All Parameters and Commands *specific* to **theClock** are shown in this section.
They are available via the **Serial Monitor**, the **Telnet Monitor**, or on the
**Dashboard** and **Config** pages of the *WebUI*.


<table>
<tr><td valign='top'><b>CLOCK_TYPE</b></td><td valign='top'>ENUM</td><td valign='top'>Determines which <b>direction</b> will be used for measuring the <i>zero crossing</i> for a <i>full 1000 ms cycle</i>. <b>Right</b> means that it will measure the cycle when the Pendulum crosses from <i>left to right</i> through <i>zero</i> and <b>left</b> means that it will measure the cycle when the Pendulum crosses from <i>right to left</i> through <i>zero</i>.
   <br><i>allowed</i> : <b>0</b>=Right, <b>1</b>=Left
   <br><i>default</i> : <b>0</b>=Right</td></tr>
<tr><td valign='top'><b>CLOCK_MODE</b></td><td valign='top'>ENUM</td><td valign='top'>Allows you to set different <b>modes</b> of operation for <i>tuning</i> and <i>experimenting</i> with the clock.
   <br><i>allowed</i> : <b>0</b>=Sensor_Test, <b>1</b>=Power_Min, <b>2</b>=Power_Max, <b>3</b>=Angle_Start, <b>4</b>=Angle_Min, <b>5</b>=Angle_Max, <b>6</b>=Min_Max, <b>7</b>=PID
   <br><i>default</i> : <b>7</b>=PID</td></tr>
<tr><td valign='top'><b>RUNNING</b></td><td valign='top'>BOOL</td><td valign='top'>Starts <b>running</b> the clock in the current <i>mode</i>. Is set automatically, at the correct time if START_SYNC is used.
   <br><i>default</i> : <b>0</b>=off
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>START_SYNC</b></td><td valign='top'>BOOL</td><td valign='top'><b>Starts</b> the clock at the next <i>minute crossing</i> based on START_DELAY.
   <br><i>default</i> : <b>0</b>=off
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>START_DELAY</b></td><td valign='top'>INT</td><td valign='top'>Determines, in <i>milliseconds</i>, <b>before</b> or <b>after</b> the minute crossing that the <i>Start Sync</i> will take place. will take placed compared to the <i>minute crossing</i>. A <i>negative</i> number will start the clock <i>before</i> the minute crossing, and a <i>positive</i> number will start the clock <i>after</i> the minute crossing.
   <br><i>default</i> : <b>200</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -5000&nbsp;&nbsp;&nbsp;<i>max</i> : 5000</td></tr>
<tr><td valign='top'><b>SET_ZERO_ANGLE</b></td><td valign='top'>COMMAND</td><td valign='top'>Sets the <b>Zero Angle</b> for the <i>Angle Sensor</i>.</td></tr>
<tr><td valign='top'><b>ZERO_ANGLE</b></td><td valign='top'>INT</td><td valign='top'>The <i>raw integer</i> value of the zero angle reading, on a scale from <i>0 to 4095</i>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>ZERO_ANGLE_F</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>floating point</i> value of the zero angle reading, on a scale from <i>0 to 360 degrees</i>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEAD_ZONE</b></td><td valign='top'>FLOAT</td><td valign='top'>An angle, in <i>degrees about zero</i> where the coils will <b>not</b> be energized on each swing. This is necessary so that we don't send a <i>pulse</i> to thecoil until <b>after</b> the Pendulum has crossed zero.
   <br><i>default</i> : <b>0.300</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 15</td></tr>
<tr><td valign='top'><b>ANGLE_START</b></td><td valign='top'>FLOAT</td><td valign='top'>The starting <i>target angle</i> in <i>degrees</i> for PID and MIN_MAX clock modes. The clock will attempt to reach this angle before changing state from CLOCK_STARTING to CLOCK_RUNNING. This value should be set <i>between</i>  ANGLE_MIN and ANGLE_MAX.
   <br><i>default</i> : <b>10.000</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 15</td></tr>
<tr><td valign='top'><b>ANGLE_MIN</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>minimum</i> target angle in <i>degrees</i> that will be used in PID and MIN_MAX clock modes.
   <br><i>default</i> : <b>9.000</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 15</td></tr>
<tr><td valign='top'><b>ANGLE_MAX</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>maximum</i> target angle in <i>degrees</i> that will be used in PID and MIN_MAX clock modes.
   <br><i>default</i> : <b>11.500</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 15</td></tr>
<tr><td valign='top'><b>POWER_MIN</b></td><td valign='top'>INT</td><td valign='top'>The <i>minimum</i> <b>power</b>, on a scale from 0 to 255, that will be delivered to the coils.
   <br><i>default</i> : <b>60</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>POWER_PID</b></td><td valign='top'>INT</td><td valign='top'>The <b>power</b>, on a scale from 0 to 255, that will be used as the starting input for the <b>1st PID controller</b> which tries to get the Pendulum to swing at the <i>target angle</i> by modifying the <i>power</i>.
   <br><i>default</i> : <b>100</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>POWER_MAX</b></td><td valign='top'>INT</td><td valign='top'>The <i>maxium</i> <b>power</b>, on a scale from 0 to 255, that will be delivered to the coils.
   <br><i>default</i> : <b>255</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>POWER_START</b></td><td valign='top'>INT</td><td valign='top'>The <b>power</b>, on a scale from 0 to 255, that will be used for the <b>initial starting pulse</b> to the Pendulum.
   <br><i>default</i> : <b>255</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>DUR_PULSE</b></td><td valign='top'>INT</td><td valign='top'>The duration, in <i>milliseconds</i>, for the <b>pulses</b> delivered each time the Pendulum <i>crosses zero</i> (in either direction).
   <br><i>default</i> : <b>120</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>DUR_START</b></td><td valign='top'>INT</td><td valign='top'>The duration, in <i>milliseconds</i>, for the <i>initial starting pulse</i>.
   <br><i>default</i> : <b>250</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>PID_P</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>proportional</i> factor, which is muliplied by the <i>instantaneous angular error</i>, for the <b>1st PID controller</b>, which tries to get the Pendulum to swing at the <i>target angle</i> by modifying the <i>power</i>.
   <br><i>default</i> : <b>20.000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>PID_I</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>integral</i> factor, which is muliplied by the <i>cumulative angular error</i>, for the <b>1st PID controller</b>, which tries to get the Pendulum to swing at the <i>target angle</i> by modifying the <i>power</i>.
   <br><i>default</i> : <b>0.500</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>PID_D</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>derivative</i> factor. which is multiplied by the <i>difference</i> in the change in the degrees from the previous swing, for the <b>1st PID controller</b>, which tries to get the Pendulum to swing at the <i>target angle</i> by modifying the <i>power</i>.
   <br><i>default</i> : <b>-9.000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>APID_P</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>proportional</i> factor, which is muliplied by the <i>instantaneous cycle error</i>, for the <b>2nd PID controller</b>, which tries to get the Pendulum to swing at the <i>the correct rate</i> by modifying the <i>target angle</i>.
   <br><i>default</i> : <b>0.200</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>APID_I</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>integral</i> factor, which is muliplied by the <i>cumulative milliseconds error</i>, for the <b>2nd PID controller</b>, which tries to get the Pendulum to swing at the <i>the correct rate</i> by modifying the <i>target angle</i>.
   <br><i>default</i> : <b>0.025</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>APID_D</b></td><td valign='top'>FLOAT</td><td valign='top'>The <i>derivative</i> factor. which is multiplied by the <i>difference</i> in the milliseconds per swing from the previous swing, for the <b>2nd PID controller</b>, which tries to get the Pendulum to swing at the <i>the correct rate</i> by modifying the <i>target angle</i>.
   <br><i>default</i> : <b>0.002</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>RUNNING_ANGLE</b></td><td valign='top'>FLOAT</td><td valign='top'>The <b>angle</b>, in <i>degrees</i> at which the Pendulum must swing in clock_modes ANGLE_START and higher before the clock will change to CLOCK_STATE_RUNNING.
   <br><i>default</i> : <b>4.000</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 12</td></tr>
<tr><td valign='top'><b>RUNNING_ERROR</b></td><td valign='top'>FLOAT</td><td valign='top'>The <b>cumulative angular error</b>, in <i>degrees</i> which the clock must fall under in clock_modes ANGLE_START and higher before the clock will change to CLOCK_STATE_RUNNING.
   <br><i>default</i> : <b>2.000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 1&nbsp;&nbsp;&nbsp;<i>max</i> : 100</td></tr>
<tr><td valign='top'><b>MIN_MAX_MS</b></td><td valign='top'>INT</td><td valign='top'>The number of <i>milliseconds</i> fast, or slow, that the clock must be running for a change between the minimum to the maxium <i>target angle</i> in MIN_MAX mode.
   <br><i>default</i> : <b>50</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 10&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>CYCLE_RANGE</b></td><td valign='top'>INT</td><td valign='top'>The <b>instantaneous</b> number of <i>milliseconds</i> fast, or slow, for the current swing, that will cause the <i>4th LED</i> to change from <b>green</b> to <b>red</b> or <b>blue</b>.
   <br><i>default</i> : <b>50</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 10&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>ERROR_RANGE</b></td><td valign='top'>INT</td><td valign='top'>The <b>cumulative</b> number of <i>milliseconds</i> fast, or slow, that will cause the <i>3rd LED</i> to change from <b>green</b> to <b>red</b> or <b>blue</b>.
   <br><i>default</i> : <b>150</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 10&nbsp;&nbsp;&nbsp;<i>max</i> : 5000</td></tr>
<tr><td valign='top'><b>LED_BRIGHTNESS</b></td><td valign='top'>INT</td><td valign='top'>Sets the brightness, on a scale of 0 to 254, where 0 turns the LEDS <b>off</b>.
   <br><i>default</i> : <b>40</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 254</td></tr>
<tr><td valign='top'><b>PLOT_TYPE</b></td><td valign='top'>ENUM</td><td valign='top'>If <b>not Off</b>, this parameter <i>suspends</i> normal Serial and Telnet Monitor output, and instead, outputs a series of numbers every few milliseconds for use with the Arduino <b>Serial Plotter</b> to visualize the swing of the Pendulum, the pulses delivered to the clock, and so on.
   <br><i>allowed</i> : <b>0</b>=Off, <b>1</b>=Waves, <b>2</b>=Pause, <b>3</b>=Clock
   <br><i>default</i> : <b>0</b>=Off
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>SYNC_RTC</b></td><td valign='top'>COMMAND</td><td valign='top'>A <b>manual command</b> that allows you to immediatly trigger a synchronization of the <b>Clock</b> to the <b>RTC</b> (embedded Real Time Clock). Only can be done while the clock is <i>Running</i>. </td></tr>
<tr><td valign='top'><b>SYNC_INTERVAL</b></td><td valign='top'>INT</td><td valign='top'>How often, in <i>seconds</i>, between attempts to synchronize the <b>Clock</b> to the <b>RTC</b> (embedded Real Time Clock).
   <br><i>default</i> : <b>3600</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3000000</td></tr>
<tr><td valign='top'><b>SYNC_NTP</b></td><td valign='top'>COMMAND</td><td valign='top'>A <b>manual command</b> that allows you to immediatly trigger a synchronization of the <b>RTC</b> (Real Time Clock) to <b>NTP</b> (Network Time Protocol). Only can be done while the clock connected to <b>WiFi</b> in <i>Station Mode</i>.</td></tr>
<tr><td valign='top'><b>NTP_INTERVAL</b></td><td valign='top'>INT</td><td valign='top'>How often, in <i>seconds</i>, between attempts to synchronize the <b>RTC</b> (Real Time Clock) to <b>NTP</b> (Network Time Protocol).
   <br><i>default</i> : <b>14400</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3000000</td></tr>
<tr><td valign='top'><b>CLEAR_STATS</b></td><td valign='top'>COMMAND</td><td valign='top'>A command that will <i>clear</i> the accumulated <b>statistics</b> that are sent to the <i>WebUI</i> as if the clock was freshly <b>started</b></td></tr>
<tr><td valign='top'><b>STAT_INTERVAL</b></td><td valign='top'>INT</td><td valign='top'>How often, in <i>seconds</i> between sending updated <b>statistics</b> to the <i>WebUI</i>.
   <br><i>default</i> : <b>30</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3000000</td></tr>
<tr><td valign='top'><b>STAT_MSG0</b></td><td valign='top'>STRING</td><td valign='top'>A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG1</b></td><td valign='top'>STRING</td><td valign='top'>A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG2</b></td><td valign='top'>STRING</td><td valign='top'>A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG3</b></td><td valign='top'>STRING</td><td valign='top'>A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG4</b></td><td valign='top'>STRING</td><td valign='top'>A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG5</b></td><td valign='top'>STRING</td><td valign='top'>A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG6</b></td><td valign='top'>STRING</td><td valign='top'>A <i>String</i> that is sent to the WebUI to display <b>statistics</b>.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>TEST_COILS</b></td><td valign='top'>INT</td><td valign='top'>A <i>control</i>, used for <i>testing</i>, that lets you directly energize the <b>coils</b> by sending a value for the <b>power</b>, on a scale of <i>0 to 255</i>. <br><b>BE SURE TO RETURN THIS TO ZERO AFTER USE!!</b>.
   <br><i>default</i> : <b>0</b>=off&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 255
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>CHANGE_CLOCK</b></td><td valign='top'>INT</td><td valign='top'>A <i>control</i> that lets you <b>add</b> a positive, or <b>subtract</b> a negative <i>number of milliseconds</i> to the RTC (Real Time Clock). This can be used to <i>test</i> syncing to <b>NTP</b> or other aspects of the clock's <i>algorithms</i>.
   <br><i>default</i> : <b>0</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -3000000&nbsp;&nbsp;&nbsp;<i>max</i> : 3000000
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>VOLT_INTERVAL</b></td><td valign='top'>INT</td><td valign='top'>With the <i>optional</i> <b>external power supply</b>, this parameter tells how often, <i>in seconds</i> to check for <i>low power</i> or <i>power restored</i> conditions.
   <br><i>default</i> : <b>0</b>=off&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 86400</td></tr>
<tr><td valign='top'><b>VOLT_CALIB</b></td><td valign='top'>FLOAT</td><td valign='top'>With the <i>optional</i> <b>external power supply</b>, this parameter is <i>multiplied</i> by the <i>calculated voltage</i> to get a more reasonable <b>voltage</b> for use in the <i>low power sensing</i> algorithm.
   <br><i>default</i> : <b>1.030</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 2</td></tr>
<tr><td valign='top'><b>VOLT_CUTOFF</b></td><td valign='top'>FLOAT</td><td valign='top'>With the <i>optional</i> <b>external power supply</b>, this parameter defines the <b>voltage</b> under which the clock will switch to <i>low power mode</i>
   <br><i>default</i> : <b>4.000</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 120</td></tr>
<tr><td valign='top'><b>VOLT_RESTORE</b></td><td valign='top'>FLOAT</td><td valign='top'>With the <i>optional</i> <b>external power supply</b>, this parameter defines the <b>voltage</b> over which the clock will leave <i>low power mode</i>
   <br><i>default</i> : <b>5.000</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 120</td></tr>
<tr><td valign='top'><b>LOW_POWER_EN</b></td><td valign='top'>BOOL</td><td valign='top'>With the <i>optional</i> <b>external power supply</b>, this parameter <b>enables</b> the clock to actually <i>go</i> into <i>low power mode</i>.  Note that if VOLT_INTERVAL is <b>not zero</b>, the clock will still <i>measure</i> the voltage and <b>display</b> the results in the Serial monitor even if <i>low power mode</i> is <b>disabled</b>.  That allows you to test the <i>power sensing</i> and <i>functionality</i> <b>before</b> actually sending the clock into <b>low power mode</i> which also <b>Turns the Wifi off</b>.
   <br><i>default</i> : <b>0</b>=off</td></tr>
</table>


### 2. myIOT General

The Parameters and Commands shown in this table are common to all
[**MyIOT**](https://github.com/phorton1/Arduino-libraries-myIOT) devices.
They are available via the **Serial Monitor**, the **Telnet Monitor**, or on the
**Device** page of the *WebUI*.

<table>
<tr><td valign='top'><b>REBOOT</b></td><td valign='top'>COMMAND</td><td valign='top'>Reboots the device.</td></tr>
<tr><td valign='top'><b>FACTORY_RESET</b></td><td valign='top'>COMMAND</td><td valign='top'>Performs a Factory Reset of the device, restoring all of the <i>parameters</i> to their initial values and rebooting</td></tr>
<tr><td valign='top'><b>VALUES</b></td><td valign='top'>COMMAND</td><td valign='top'>From Serial or Telnet monitors, shows a list of all of the current <i>parameter</i> <b>values</b></td></tr>
<tr><td valign='top'><b>DEVICE_NAME</b></td><td valign='top'>STRING</td><td valign='top'><i>User Modifiable</i> <b>name</b> of the device that will be shown in the <i>WebUI</i>, as the <i>Access Point</i> name, and in </i>SSDP</i> (Service Search and Discovery)
   <br><b>Required</b> (must not be blank)
   <br><i>default</i> : theClock3.3</td></tr>
<tr><td valign='top'><b>DEVICE_TYPE</b></td><td valign='top'>STRING</td><td valign='top'>The <b>type</b> of the device as determined by the implementor
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_VERSION</b></td><td valign='top'>STRING</td><td valign='top'>The <b>version number</b> of the device as determined by the implementor
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_UUID</b></td><td valign='top'>STRING</td><td valign='top'>A <b>unique identifier</b> for this device.  The last 12 characters of this are the <i>MAC Address</i> of the device
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_URL</b></td><td valign='top'>STRING</td><td valign='top'>The <b>url</b> of a webpage for this device.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_IP</b></td><td valign='top'>STRING</td><td valign='top'>The most recent WiFi <b>IP address</b> of the device. Assigned by the WiFi router in <i>Station mode</i> or hard-wired in <i>Access Point</i> mode.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_BOOTING</b></td><td valign='top'>BOOL</td><td valign='top'>A value that indicates that the device is in the process of <b>rebooting</b>
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEBUG_LEVEL</b></td><td valign='top'>ENUM</td><td valign='top'>Sets the amount of detail that will be shown in the <i>Serial</i> and <i>Telnet</i> output.
   <br><i>allowed</i> : <b>0</b>=NONE, <b>1</b>=USER, <b>2</b>=ERROR, <b>3</b>=WARNING, <b>4</b>=INFO, <b>5</b>=DEBUG, <b>6</b>=VERBOSE
   <br><i>default</i> : <b>5</b>=DEBUG</td></tr>
<tr><td valign='top'><b>LOG_LEVEL</b></td><td valign='top'>ENUM</td><td valign='top'>Sets the amount of detail that will be shown in the <i>Logfile</i> output. <b>Note</b> that a logfile is only created if the device is built with an <b>SD Card</b> on which to store it!!
   <br><i>allowed</i> : <b>0</b>=NONE, <b>1</b>=USER, <b>2</b>=ERROR, <b>3</b>=WARNING, <b>4</b>=INFO, <b>5</b>=DEBUG, <b>6</b>=VERBOSE
   <br><i>default</i> : <b>0</b>=NONE</td></tr>
<tr><td valign='top'><b>LOG_COLORS</b></td><td valign='top'>BOOL</td><td valign='top'>Sends standard <b>ansi color codes</b> to the <i>Serial and Telnet</i> output to highlight <i>errors, warnings,</i> etc
   <br><i>default</i> : <b>0</b>=off</td></tr>
<tr><td valign='top'><b>LOG_DATE</b></td><td valign='top'>BOOL</td><td valign='top'>Shows the <b>date</b> in Logfile and Serial output
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>LOG_TIME</b></td><td valign='top'>BOOL</td><td valign='top'>Shows the current <b>time</b>, including <i>milliseconds</i> in Logfile and Serial output
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>LOG_MEM</b></td><td valign='top'>BOOL</td><td valign='top'>Shows the <i>current</i> and <i>least</i> <b>memory available</b>, in <i>KB</i>, on the ESP32, in Logfile and Serial output
   <br><i>default</i> : <b>0</b>=off</td></tr>
<tr><td valign='top'><b>WIFI</b></td><td valign='top'>BOOL</td><td valign='top'>Turns the device's <b>WiFi</b> on and off
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>AP_PASS</b></td><td valign='top'>STRING</td><td valign='top'>The <i>encrypted</i> <b>Password</b> for the <i>Access Point</i> when in AP mode
   <br><i>default</i> : 11111111</td></tr>
<tr><td valign='top'><b>STA_SSID</b></td><td valign='top'>STRING</td><td valign='top'>The <b>SSID</b> (name) of the WiFi network the device will attempt to connect to as a <i>Station</i>.  Setting this to <b>blank</b> force the device into <i>AP</i> (Access Point) mode</td></tr>
<tr><td valign='top'><b>STA_PASS</b></td><td valign='top'>STRING</td><td valign='top'>The <i>encrypted</i> <b>Password</b> for connecting in <i>STA</i> (Station) mode</td></tr>
<tr><td valign='top'><b>SSDP</b></td><td valign='top'>BOOL</td><td valign='top'>Turns <b>SSDP</b> (Service Search and Discovery Protocol) on and off.  SSDP allows a device attached to WiFi in <i>Station mode</i> to be found by other devices on the LAN (Local Area Network). Examples include the <b>Network tab</b> in <i>Windows Explorer</i> on a <b>Windows</b>
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>TIMEZONE</b></td><td valign='top'>ENUM</td><td valign='top'>Sets the <b>timezone</b> for the RTC (Real Time Clock) when connected to WiFi in <i>Station mode</i>. There is a very limited set of timezones currently implemented.
   <br><i>allowed</i> : <b>0</b>=EST - Panama, <b>1</b>=EDT - New York, <b>2</b>=CDT - Chicago, <b>3</b>=MST - Phoenix, <b>4</b>=MDT - Denver, <b>5</b>=PDT - Los Angeles
   <br><i>default</i> : <b>0</b>=EST - Panama</td></tr>
<tr><td valign='top'><b>NTP_SERVER</b></td><td valign='top'>STRING</td><td valign='top'>Specifies the NTP (Network Time Protocol) <b>Server</b> that will be used when connected to WiFi as a <i>Station</i>
   <br><i>default</i> : pool.ntp.org</td></tr>
<tr><td valign='top'><b>LAST_BOOT</b></td><td valign='top'>TIME</td><td valign='top'>The <b>time</b> at which the device was last rebooted.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>UPTIME</b></td><td valign='top'>INT</td><td valign='top'>LAST_BOOT value as integer seconds since Jan 1, 1970.  Displayed as he number of <i>hours, minutes, and seconds</i> since the device was last rebooted in the WebUI
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>RESET_COUNT</b></td><td valign='top'>INT</td><td valign='top'>The number of times the <b>Factory Reset</b> command has been issued on this device
   <br><i>default</i> : <b>0</b></td></tr>
</table>




**Next:** [**Trouble Shooting**](troubles.md) potential problems with the clock ...
