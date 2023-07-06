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

- Buttons
- LEDS
- Parameters, Values, and Commands
- WebUI General
- Telnet Serial Access
- OTA Firmware Upgrades



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

When you first start the clock, it will show a certain pattern of LEDs as it boots and possibly
connects to your WiFi network. It flashes all 5 LEDs **cyan** from left to right at initial start
up to ensure that the LEDs are working.

<img src='./images/LEDS_small-startup.gif' width='200ox'>

### Special Startup Patterns

There are several special patterns of LEDs that show having to do
with the **Angle Sensor** and the **set_zero_angle** command.

<table border='0'>
<tr><td width='200px'><img src='./images/LEDS_small-all_flash_red.gif'></td><td>
	if all of the LEDS are <font color='red'><b>flashing red</b></font>
	it means that there was a failure to initialize the <i>AS5600 Angle Sensor</i>
	and the clock <b>cannot run correctly</b>.
	This means there is either <i>bad cable or connection</i>, or that there is a
	problem with the <i>PCB, ESP32,</i> or the <i>Angle Sensor</i> module itself</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-all_white_green3.jpg'></td><td>
	this pattern, where four of the LEDS are white and one is
	<font color='green'><b>green</b></font> means that
	the <b>set_zero_angle</b> command has <b>succeeded</b>, and the clock has gotten
	a new, good <i>zero angle</i> from the sensor.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-all_white_red3.jpg'></td><td>
	this pattern, where four of the LEDS are white and one is
	<font color='red'><b>red</b></font> means that
	the <b>set_zero_angle</b> command has <b>failed</b>, to get
	a good <i>zero angle</i> from the sensor,
	and the clock <b>cannot run correctly</b>.</td></tr>
</table>




### 1. LED1 **WiFi status**

The 1st (leftmost) LED shows the **WiFi status**

<table border='0'>
<tr><td width='200px'><img src='./images/LEDS_small-cyan1.jpg'></td><td><font color='cyan'><b>cyan</b></font>
	means that the system is <i>initializing</i> (booting)/td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-green1.jpg'></td><td><font color='green'><b>green</b></font>
	means that it is correctly <b>connected</b> to your WiFi network</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-purple1.jpg'></td><td><font color='purple'><b>purple</b></font>
	means that it is not connected to your WiFi network, and is in <b>Access Point Mode</b>
	  so that you can connect directly to it from your phone, laptop, or computer.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-orange1.jpg'></td><td><font color='orange'><b>orange</b></font>
	is a temporary state after the clock has connected to your WiFi network.
	The clock is waiting for you to disconnect from the Access Point before turning the AP mode off.</td></tr>
<tr><td width='200px'><img  src='./images/LEDS_small-red1.jpg'></td><td><font color='red'><b>red</b></font>
	means that the WiFi is <b>not working</b> correctly.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-blue1.jpg'></td><td><font color='blue'><b>blue</b></font>
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
<tr><td width='200px'><img src='./images/LEDS_small-white2.jpg'></td><td><b>white</b>
	means the clock is waiting to <i>Start Synchronized</i>
	when the time crosses the next minute.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-yellow2.jpg'></td><td><font color='gold'><b>yellow</b></font>
	is very transient, lasting less than 1/2 second, and means that the
	clock is delivering the initial impulse to start the pendulum moving.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-cyan2.jpg'></td><td><font color='cyan'><b>cyan</b></font>
	means that the clock is </i>starting</i>.  It takes approximately 30 seconds for the
	pendulum to stabilize before the clock starts *running* normally.</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-green2.jpg'></td><td><font color='green'><b>green</b></font>
	means that the clock is <b>running normally</b></td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-orange2.jpg'></td><td><font color='orange'><b>orange</b></font>
	indicates the clock is in <i>SENSOR_TEST mode</i></td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-flashing_orange2.gif'></td><td><font color='orange'><b>flashing orange</b></font>
	indicates that the <b>Zero Angle has not been set</b> or is out of range
	and the <i>clock cannot work</i> correctly</td></tr>
<tr><td width='200px'><img src='./images/LEDS_small-flashing_red2.gif'></td><td><font color='red'><b>flashing red</b></font>
	indicates that the <b>Angle Sensor did not initialize</b> correctly
	and the <i>clock cannot work</i> correctly</td></tr>
</table>


### 3. LED3 **Overall Accuracy**

The 3rd (middle) LED shows the **overall accuracy** of the clock within 150 milliseconds,
where **green** is good, blue is 150ms or more slow, and red is 150 ms or more fast.

	- this LED will vary between **blue** to **green** to **red** as the clock speeds up or slows down and the **overall time accuracy** of the clock varies
	  from 150 milliseconds (slow) to -150 milliseconds (fast) from the correct time.  It is normal for it to be slightly non-green (cyanish as it goes a bit
	  slow and yellowish as it goes a bit fast), but it should not **stay** solid red or blue for extended periods of time.
	- The *span* of time covered by this indicator is parameterized and can be changed via the WebUI.  I set to +/- 300ms for problematic clocks :-)
	- **flashing** **blue** or **red**

- 4th LED - shows the *instantaneous accuracy* for each swing within 50 milliseconds (1/20 of a second), also where **green** is good, **blue** is 50ms or more slow, and **red** is 50ms or more fast.

	- this LED will very between **blue** to **green** to **red** as each swing is measured and compared to 1000 milliseconds (one second).
	- It is normal for it to be slightly non-green (cyanish as the pendulum swings a bit slower and yellowish as it swings a bit fast),
	   but it should not **stay** solid red or blue for extended periods of time.
	- You may notice that the 3rd and 4th LEDs alternate colors slightly.  For example, as the overall time slows and the 3rd LED turns bluish, the
	  pendulum *should* speed up and it's LED turn yellowish (towards red) until the time is corrected.
	- Thus from looking at these two LEDs you can see the clock is working correctly.
	- The span of 50ms is also parameterized and may be modified in the WebUI.

- 5th (right) LED - shows if the clock is undergoing a **synchronization** cycle, where **blue** means it is slow and the clock is trying to speed up, and **red** is fast and the clock is trying to slow down.

	- By default (parameterized) once per hour the clock will attempt to *synchronize* to the RTC (Real Time Clock) on the embedded computer.
	- It is normal for this to be red or blue for a few minutes at a time and, (as with the 3rd LED) for the *instantaneous error* to move in the opposite direction of the synchronization.
	- If this LED remains red or blue for more than a few minutes (say 10) it means that the clock probably needs maintenance!
	- If connected to your WiFi network, the clock syncrhonizes the RTC to NTP (Network Time Protocol) every two hours (parameterized)


If you see four green, or close to green, LEDs, all is well and the clock is running within 1/10 of a second of the correct time.

It is normal for the 5th LED to sometimes come on as red or blue.  It should transition slowly to green before turning off.

If any of the last three pixels remains BLUE or RED for more than 10-15 minutes, then it is likely the clock needs **maintenance** or **tuning**.


## 3. Parameters and Commands

### A. Clock Specific

The Parameters and Commands *specific* to **theClock** are shown in this table :

<table>
<tr><td valign='top'><b>CLOCK_TYPE</b></td><td valign='top'>ENUM</td><td valign='top'>The type of clock.
   <br><i>allowed</i> : <b>0</b>=Left, <b>1</b>=Right
   <br><i>default</i> : <b>0</b>=Left</td></tr>
<tr><td valign='top'><b>CLOCK_MODE</b></td><td valign='top'>ENUM</td><td valign='top'>The Clock Mode.
   <br><i>allowed</i> : <b>0</b>=Sensor_Test, <b>1</b>=Power_Min, <b>2</b>=Power_Max, <b>3</b>=Angle_Start, <b>4</b>=Angle_Min, <b>5</b>=Angle_Max, <b>6</b>=Min_Max, <b>7</b>=PID
   <br><i>default</i> : <b>7</b>=PID</td></tr>
<tr><td valign='top'><b>RUNNING</b></td><td valign='top'>BOOL</td><td valign='top'><i>default</i> : <b>0</b>=off
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>START_SYNC</b></td><td valign='top'>BOOL</td><td valign='top'><i>default</i> : <b>0</b>=off
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>START_DELAY</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>-500</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -5000&nbsp;&nbsp;&nbsp;<i>max</i> : 5000</td></tr>
<tr><td valign='top'><b>SET_ZERO_ANGLE</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>ZERO_ANGLE</b></td><td valign='top'>INT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>ZERO_ANGLE_F</b></td><td valign='top'>FLOAT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEAD_ZONE</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>0.300</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 15</td></tr>
<tr><td valign='top'><b>ANGLE_START</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>10.000</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 15</td></tr>
<tr><td valign='top'><b>ANGLE_MIN</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>9.000</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 15</td></tr>
<tr><td valign='top'><b>ANGLE_MAX</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>11.500</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 15</td></tr>
<tr><td valign='top'><b>POWER_MIN</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>60</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>POWER_PID</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>100</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>POWER_MAX</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>255</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>POWER_START</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>255</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>DUR_PULSE</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>120</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>DUR_START</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>250</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>PID_P</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>20.000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>PID_I</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>0.500</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>PID_D</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>-9.000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>APID_P</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>0.200</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>APID_I</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>0.025</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>APID_D</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>0.002</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -1000&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>RUNNING_ANGLE</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>4.000</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 12</td></tr>
<tr><td valign='top'><b>RUNNING_ERROR</b></td><td valign='top'>FLOAT</td><td valign='top'><i>default</i> : <b>2.000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 1&nbsp;&nbsp;&nbsp;<i>max</i> : 100</td></tr>
<tr><td valign='top'><b>MIN_MAX_MS</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>50</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 10&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>CYCLE_RANGE</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>50</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 10&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>ERROR_RANGE</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>150</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 10&nbsp;&nbsp;&nbsp;<i>max</i> : 5000</td></tr>
<tr><td valign='top'><b>LED_BRIGHTNESS</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>40</b>&nbsp;&nbsp;&nbsp;<i>max</i> : 254</td></tr>
<tr><td valign='top'><b>PLOT_VALUES</b></td><td valign='top'>ENUM</td><td valign='top'><i>allowed</i> : <b>0</b>=Off, <b>1</b>=Waves, <b>2</b>=Pause, <b>3</b>=Clock
   <br><i>default</i> : <b>0</b>=Off
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>SYNC_RTC</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>SYNC_INTERVAL</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>3600</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3000000</td></tr>
<tr><td valign='top'><b>SYNC_NTP</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>NTP_INTERVAL</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>14400</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3000000</td></tr>
<tr><td valign='top'><b>CLEAR_STATS</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>STAT_INTERVAL</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>30</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3000000</td></tr>
<tr><td valign='top'><b>STAT_MSG0</b></td><td valign='top'>STRING</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG1</b></td><td valign='top'>STRING</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG2</b></td><td valign='top'>STRING</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG3</b></td><td valign='top'>STRING</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG4</b></td><td valign='top'>STRING</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG5</b></td><td valign='top'>STRING</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>STAT_MSG6</b></td><td valign='top'>STRING</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>TEST_COILS</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>0</b>=off&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 255
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>CHANGE_CLOCK</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>0</b>&nbsp;&nbsp;&nbsp;<i>min</i> : -3000000&nbsp;&nbsp;&nbsp;<i>max</i> : 3000000
   <br><i>Memory Only</i></td></tr>
</table>


### B. myIOT General

The Parameters and Commands shown in this table are available in all
[**MyIOT**](https://github.com/phorton1/Arduino-libraries-myIOT) devices.

<table>
<tr><td valign='top'><b>REBOOT</b></td><td valign='top'>COMMAND</td><td valign='top'>Reboots the device.</td></tr>
<tr><td valign='top'><b>FACTORY_RESET</b></td><td valign='top'>COMMAND</td><td valign='top'>Performs a Factory Reset of the device, restoring all of the <i>parameters</i> to their initial values and rebooting</td></tr>
<tr><td valign='top'><b>VALUES</b></td><td valign='top'>COMMAND</td><td valign='top'>From Serial or Telnet monitors, shows a list of all of the current <i>parameter</i> <b>values</b></td></tr>
<tr><td valign='top'><b>PARAMS</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>JSON</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
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



## 4. WebUI General
## 5. Telnet Serial Access
## 6. OTA Firmware Upgrades


**Next:** [**Trouble Shooting**](troubles.md) potential problems with the clock ...
