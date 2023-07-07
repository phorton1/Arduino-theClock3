# theClock3 - User Interface

**[Home](readme.md)** --
**[Design](design.md)** --
**[Plan](plan.md)** --
**[Wood](wood.md)** --
**[Coils](coils.md)** --
**[Electronics](electronics.md)** --
**[Software](software.md)** --
**[Assemble](assemble.md)** --
**[Build](build.md)** --
**[Tuning](tuning.md)** --
**UI** --
**[Troubles](troubles.md)** --
**[Notes](notes.md)**

On the [Software](software.md) page we gave an
[Overview](./software.md#b-software-overview) of the Software
of the clock, and described how to use the
[Serial Monitor](./software.md#c-serial-monitor) and how to
[connect to the Webui](./software.md#e-connect-to-webui).

On this page, we describe the **User Interfaces** of the clock in terms of:

- the two [**Buttons**](#a-buttons) on the clock
- the five [**LEDS**](#b-leds) on the clock
- All of the [**Parameters and Commands**](#c-parameters-and-commands) available on the clock
- [**WebUI details**](#e-webui), including how to do [**OTA**](#3-ota) Firmware Upgrades
- the [**Telnet**](#f-telnet-serial-monitor) Serial Monitor


## A. Buttons

There are **two holes** front of the *plastic* **Box** into which an item like a toothpick
or small screw driver can be *inserted* to **press a button**..

A button press can be **short** (instant - less than 3 seconds), **medium** (more than 3 seconds, but less than 8 seconds), or **long** (more than 8 seconds).
When you press a button, the rightmost LED will turn **white**.   If you hold the button for more than 3 seconds, the LED will turn
<font color='cyan'><b>cyan</b></font>, and if you hold it for more than 8 seconds, the rightmost LED will turn
<font color='magenta'><b>magenta</b></font>.

![LEDS_small-button_leds.gif](images/LEDS_small-button_leds.gif)

The **left button** is used to *start and stop the clock*, turn the *WiFi on and off*, or in extreme cases, to
do a *Factory Reset*.  The **right button** is used to *change the brightness* of the LEDs or
to calibrate the angular sensor on the pendulum (*set zero angle*).

### Left Button

- a **short press** will  **start synchronized**, or **stop** the clock if it is running
- a **double press** will **start** the clock *immediately* (if it is not running and pressed twice within 2 seconds)
- a **medium press** will turn the **WiFi** off and on
- a **long press** will issue a **Factory Reset** and should only be done if all else fails!

### Right Button

- a **short press** will **change the LED brightness** - Pressing the button multiple times will
  increase the brightness of the LEDs until it cycles around and they go 'off'.
- a **medium press** will **set the zero angle** - Occasionally it may be necessary to re-calibrate the angular sensor.
  This is because the magnet on the end of the pendulum tube is not glued on, but merely compression fitted,
  (so that the clock can be disassembled if necessary), and may have slipped during transport or by rough
  handling. Be sure to *hold the clock still* with the *seconds hand straight down* when you issue this command.
- in ***optional* low power** mode, a *short press* will *exit low power mode* and *brighten the LEDs* and
  *turn the WiFi back on*, depending on the configuration

In order to **calibrate the angular sensor** (*set zero angle*) the clock must NOT be running and
the **the Pendulum should hanging, still, straight down**.  When that is the case
long-press the right button for 8+ seconds (until the rightmost LED turns to magenta) while being
*careful **NOT** to move the clock*.  The clock will read the sensor when you let up on the button,
which will determines the "zero position" of the Pendulum which is crucial for the clock to run correctly.

The *zero position* of the pendulum is used to determine when the pendulum has "crossed zero"
so that we can send an impulse to the pendulum on each swing to keep it moving and adjust it's
speed, as well as used in measuring the angle that the pendulum has traversed on each swing,
both of which are needed for the clock to keep good time.

Of course, all of these functions can also be accessed via the Serial Monitor or WebUI (below),
but the buttons allow you to use the clock without another computer.



## B. LEDS

If you see four <font color='green'><b>green</b></font> (or close to green) LEDs,
**all is well** and the clock is running within a few milliseconds of the correct time.

<img src='./images/LEDS_small-1234_green.jpg' width='200ox'>

It is normal for the 5th LED to sometimes come on as
<font color='red'><b>red</b></font> or <font color='blue'><b>blue</b></font>
as the clock does a **synchronization cycle**. The LED should
*transition* slowly to <font color='green'><b>green</b></font> as
the cycle proceeds, and then, when the cycle is *complete*, the
LED should turn **off**.

### Special Startup Patterns

When you first start the clock, it flashes all 5 LEDs **cyan**
from left to right to ensure that the LEDs are working.

<img src='./images/LEDS_small-startup.gif' width='200ox'>

There are several special patterns of LEDs that show having to do
with the **Angle Sensor** and the **set_zero_angle** command.

<table border='0'>
<tr><td width='200px'><img src='./images/LEDS_small-all_flash_red.gif'></td><td>
	if all of the LEDS are <font color='red'><b>flashing red</b></font>
	it means that there was a failure to initialize the <i>AS5600 Angle Sensor</i>
	and the clock <b>cannot run correctly</b>.
	This means there is either <i>bad cable or connection</i>, or that there is a
	problem with the <i>PCB, ESP32,</i> or the <i>Angle Sensor</i> module itself</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-all_white_3_green.jpg'></td><td>
	this pattern, where four of the LEDS are white and one is
	<font color='green'><b>green</b></font> means that
	the <b>set_zero_angle</b> command has <b>succeeded</b>, and the clock has gotten
	a new, good <i>zero angle</i> from the sensor.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-all_white_3_red.jpg'></td><td>
	this pattern, where four of the LEDS are white and one is
	<font color='red'><b>red</b></font> means that
	the <b>set_zero_angle</b> command has <b>failed</b> to get
	a good <i>zero angle</i> from the sensor
	and the clock <b>cannot run correctly</b>.</td></tr>
</table>




### 1. LED1 **WiFi status**

The 1st (leftmost) LED shows the **WiFi status**

<table border='0'>
<tr><td width='200px'><img src='./images/LEDS_small-1_cyan.jpg'></td><td><font color='cyan'><b>cyan</b></font>
	means that the system is <i>initializing</i> (booting)</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-1_green.jpg'></td><td><font color='green'><b>green</b></font>
	means that it is correctly <b>connected</b> to your WiFi network</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-1_purple.jpg'></td><td><font color='purple'><b>purple</b></font>
	means that it is not connected to your WiFi network, and is in <b>Access Point Mode</b>
	  so that you can connect directly to it from your phone, laptop, or computer.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-1_orange.jpg'></td><td><font color='orange'><b>orange</b></font>
	is a temporary state after the clock has connected to your WiFi network.
	The clock is waiting for you to disconnect from the Access Point before turning the AP mode off.</td></tr>
<tr><td width='200px'><img  src='./images/LEDS_small-1_red.jpg'></td><td><font color='red'><b>red</b></font>
	means that the WiFi is <b>not working</b> correctly.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-1_blue.jpg'></td><td><font color='blue'><b>blue</b></font>
	means that the <b>WiFi is off</b>.  In versions of the clock that include
	a battery backup system, the clock will automatically turn the WiFi off during a
	power outage to preserve the battery as long as possible and will turn the WiFi
	back on once the power has been restored.</td></tr>
</table>

### 2. LED2 **Clock status**

The 2nd LED from the left shows the **Clock status**, whether it is starting, running, and so on:

<table border='0'>
<tr><td width='200px'><img src='./images/LEDS_small-all_grey.jpg'></td><td><b>off</b>
	means the clock is not running and needs to be started</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-2_white.jpg'></td><td><b>white</b>
	means the clock is waiting to <i>Start Synchronized</i>
	when the time crosses the next minute.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-2_yellow.jpg'></td><td><font color='gold'><b>yellow</b></font>
	is very transient, lasting less than 1/2 second, and means that the
	clock is delivering the initial impulse to start the pendulum moving.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-2_cyan.jpg'></td><td><font color='cyan'><b>cyan</b></font>
	means that the clock is </i>starting</i>.  It takes approximately 30 seconds for the
	pendulum to stabilize before the clock starts *running* normally.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-2_green.jpg'></td><td><font color='green'><b>green</b></font>
	means that the clock is <b>running normally</b></td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-2_orange.jpg'></td><td><font color='orange'><b>orange</b></font>
	indicates the clock is in <i>SENSOR_TEST mode</i></td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-2_flashing_orange.gif'></td><td><font color='orange'><b>flashing orange</b></font>
	indicates that the <b>Zero Angle has not been set</b> or is out of range
	and the <i>clock cannot work</i> correctly</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-2_flashing_red.gif'></td><td><font color='red'><b>flashing red</b></font>
	indicates that the <b>Angle Sensor did not initialize</b> correctly
	and the <i>clock cannot work</i> correctly</td></tr>
</table>


### 3. LED3 **Overall Accuracy**

The 3rd (middle) LED shows the **overall accuracy** of the clock within 150 milliseconds.
<font color='green'><b>Green</b></font> means the clock is running to within a few
milliseconds of the correct time. The LED will *transition* to <font color='blue'><b>blue</b></font>
if the clock is running 150 ms slow, or *transition* to <font color='red'><b>red</b></font> is
if the clock is running 150 ms fast. You may need to pay attention to even see this
LED change because, as a rule, a well running clock will typically run within 10-20
ms of the correct time.

<table border='0'>
<tr><td width='200px'><img src='./images/LEDS_small-3_green.jpg'></td><td><font color='green'><b>green</b></font>
	indicates that the clock is running well, within a <b>few milliseconds</b> of the correct time</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-3_green_to_blue.gif'></td><td>
	the 3rd LED turns from <font color='green'><b>green</b></font> to <font color='blue'><b>blue</b></font>
	as the clock <b>slows down</b></td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-3_green_to_red.gif'></td><td>
	the 3rd LED turns from <font color='green'><b>green</b></font> to <font color='red'><b>red</b></font>
	as the clock <b>speeds up</b></td></tr>
</table>

Note that the <i>range</i> of this change (150 ms) is determined by the ERROR_RANGE paremeter
as described below.


### 4. LED4 **Cycle Accuracy**

The 4th LED shows the *instantaneous accuracy* for each **swing** (cycle) of the Pendulum
to within 50 ms (1/20 of a second) of 1000 milliseconds,
where <font color='green'><b>green</b></font> the cycle is within a few milliseconds,
and *transitioning* to <font color='blue'><b>blue</b></font> as the cycle slows to 1050ms per swing,
or transitioning to <font color='red'><b>red</b></font> cycle speeds up to 950 ms
per swing. Once again, you may need to pay attention to even see this
LED change because the LED changes *subtly* as the Pendulum speeds up
or slows down.

<table border='0'>
<tr><td width='200px'><img src='./images/LEDS_small-4_green.jpg'></td><td><font color='green'><b>green</b></font>
	indicates that the Pendulum is swinging well, within a <b>few milliseconds</b> of 1000ms per cycle</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-4_green_to_blue.gif'></td><td>
	the 4th LED turns from <font color='green'><b>green</b></font> to <font color='blue'><b>blue</b></font>
	as the cycle gets <b>slower</b> than 1000 ms per cycle</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-4_green_to_red.gif'></td><td>
	the 4th LED turns from <font color='green'><b>green</b></font> to <font color='red'><b>red</b></font>
	as the cycle gets <b>quicker</b> than 1000 ms per cycle</td></tr>
</table>

Note that the <i>range</i> of this change (1000 plus or minus 50 ms) is determined by the
CYCLE_RANGE paremeter as described below.

### 5. LED5 the **Sync** LED

The 5th (right) LED shows if the clock is undergoing a **synchronization** cycle,
where <font color='blue'><b>blue</b></font> means it is slow and the clock is trying to speed up,
and <font color='red'><b>red</b></font> means it is fast and the clock is trying to slow down.

<table border='0'>
<tr><td width='200px'><img src='./images/LEDS_small-all_grey.jpg'></td><td>
	if this LED is <b>off</b>, it indicates that clock is <b>not currently</b> doing a
	synchronization cycle
<tr><td width='200px'><img src='./images/LEDS_small-5_blue_to_green.gif'></td><td>
	the 5th LED turns from <font color='blue'><b>blue</b></font> to
	<font color='green'><b>green</b></font> as it performs a synchronization
	and the clock speeds up to the correct time.
<tr><td width='200px'><img src='./images/LEDS_small-5_red_to_green.gif'></td><td>
	the 5th LED turns from <font color='red'><b>red</b></font> to
	<font color='green'><b>green</b></font> as it performs a synchronization
	and the clock down slows down to the correct time.
<tr><td width='200px'><img src='./images/LEDS_small-5_flashing_blue.gif'></td><td>
	if the 5th LED turns is <font color='blue'><b>flashing blue</b></font> it
	indicates that the clock is running more than 5 seconds slow.
<tr><td width='200px'><img src='./images/LEDS_small-5_flashing_red.gif'></td><td>
	if the 5th LED turns is <font color='red'><b>flashing red</b></font> it
	indicates that the clock is running more than 5 seconds fast.
</table>

After the LED turns to <font color='green'><b>green</b></font> during the
synchroniztion cycle, it will then turn **off**, indicating the cycle
is complete. As with the *Cycle Accuracy* LED, the range of this change
(150 ms) is determined by the CYCLE_RANGE paremeter as described below.

- It is normal for this LED to be
	<font color='red'><b>red</b></font> or <font color='blue'><b>blue</b></font>
	for a few minutes at a time.
- If this LED remains
   <font color='red'><b>red</b></font> or <font color='blue'><b>blue</b></font>
   for more than a few minutes it means that the clock probably needs maintenance!

**If this LED is flashing**
   <font color='red'><b>red</b></font> or <font color='blue'><b>blue</b></font>
   it means the clock is running **5 seconds or more** fast or slow,
   and that it probably needs maintenance!

By default, the clock will try to synchronize to the RTC on the embedded computer once per hour (every 3600 seconds).
This interval is determined by the SYNC_INTERVAL parameter. If connected to your WiFi network, the clock
will syncrhonizes the RTC to NTP (Network Time Protocol) every two hours (7200 seconds)
as determined by the NTP_INTERVAL parameter.


## C. Parameters and Commands

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
<tr><td valign='top'><b>PLOT_VALUES</b></td><td valign='top'>ENUM</td><td valign='top'>If <b>not Off</b>, this parameter <i>suspends</i> normal Serial and Telnet Monitor output, and instead, outputs a series of numbers every few milliseconds for use with the Arduino <b>Serial Plotter</b> to visualize the swing of the Pendulum, the pulses delivered to the clock, and so on.
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
<tr><td valign='top'><b>LOW_POWER_EN</b></td><td valign='top'>BOOL</td><td valign='top'>With the <i>optional</i> <b>external power supply</b>, this parameter <b>enables</b> the clock to actually <i>go</i> into <i>low power mode</i>.  Note that if VOLT_INTERVAL is <b>not zero</b>, the clock will still <i>measure</i> the voltage and <b>display</b>> the results in the Serial monitor even if <i>low power mode</i> is <b>disabled</b>.  That allows you to test the <i>power sensing</i> and <i>functionality</i> <b>before</b> actually sending the clock into <b>low power mode</i> which also <b>Turns the Wifi off</b>.
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



## E. WebUI

We already described how to [connect to the Webui](./software.md#e-connect-to-webui) on the
[Software](software.md) page.   Here we will describe some details about the WebUI.

The WebUI makes use

- WebSockets

### 1. Panes
### 2. Clock Status Messages
### 3. OTA

## F. Telnet Serial Monitor


**Next:** [**Trouble Shooting**](troubles.md) potential problems with the clock ...
