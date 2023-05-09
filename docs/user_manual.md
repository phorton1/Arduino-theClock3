## User Manual

Welcome to theClock v3.2 !!

This clock has been especially **hand-crafted** for **YOU**.

It was entirely designed and produced by **yours truly**.  It consists of laser cut wooden parts,
3D printed plastic parts, hand cut brass tubes and threaded rods, a hand made and soldered circuit board,
hand wound electromagnetic coils, hand made connectors and cables, and, of course, I designed
and wrote all the software used by the internal computer as well as assembled, sanded, and
finished all of the wooden parts.

Not only did I design and produce the clock, but I also designed and produced the **machine
that laser cut** the wooden pieces, as well as the machine I used to **mill and create the
printed circuit board**, as well the circuit boards and most of the software in those machines,
as well as a few other **custom machines**, like a coil winder, that were used in the assembly.

I would like you to visit **github.com/phorton1/Arduino-theClock3** and view the web-pages in the repository
which show how the clock was built so that you can see a bit of the insane amount of effort in this seemingly
simple project.  There you will find ample detail about the clock, as well as links to the other
projects and machines that are involved.

Although you could theoretically merely unpack the clock, plug it in, and press the left button
twice to start it ticking, **I highly recommend that you first connect it to your local WiFi network**,
and use the **Synchronized Start** method described below in the *Quick Start* section to start the clock.

The clock is designed to **synchronize itself** to the Network Time Protocol (NTP) if it is given a
Wifi connection, with the idea that it should run within a second or so of the correct time for
the forseeable future (until at least the *Y2038 problem*, when unix time will overflow 32 bits).
If it is not connected to the internet, the best it can do is synchronize itself to the internal
RTC (Real Time Clock) on the embedded computer, which, at best, will only be correct to within
a few seconds a day, so I really hope you'll take the time to give it access to NTP.

A few notes:

- The **Wifi has been enabled by default**. It can be turned off if you are paranoid about your home network security.
  Substantial efforts were put into ensuring that the clock does not create any significant security risk
  to your home network, but just in case, I added the ability for you to disable the clock's Wifi access entirely.
- The **Time Zone** for the clock has been pre-customized to your location before I shipped it to you.
  The Time Zone will be cleared by a *Factory Reset*.
  So, if you do a *Factory Reset*, you will probably want to connect to the clock, go to the *WebUI*, and set the
  correct time zone for your clock.
- Although I have tried my best to produce a reliable clock, **some maintenance may be necessary**.  In particular
  it may be necessary once every few years, or more often, to lubricate the bearings and brass tubes.

## 1. Quick Start

- Unpacking
- Preparation
- Power Up!
- Connect to theClock
-- Change Default Password
-- Specify Wifi SSID and password
-- Verify Connection
- Start Clock Synchronized
- Stop the Clock

## 2. Meaning of the LEDs

There are two **LED Modes** that the clock can have, and as well, you can turn the LED's entirely off.

When you first start the clock, it will show a certain pattern of LEDs as it boots and possibly
connects to your Wifi network. It flashes all 5 LEDs **cyan** from left to right at initial start
up to ensure that the LEDs are working.  After that, the startup sequence uses the left most LED
to show the following:

- It briefly flashes **orange** for 1/2 second before attempting to connect to the *angular pendulum sensor*.
  The clock will try upto 5 times to connect to the sensor, which should work on the first attempt.
  If it fails to connect to the sensor, it will flash **red** 5 times for 1/2 second and try again.
  If it fails to connect to the sensor 5 times it will turn solid red for 2.5 seconds,
  **You should never see *red* during the boot sequence**. If you do then it probably means the
  cable is not connected correctly, in which case you can disconnect and reconnect the cable on
  both ends to correct the problem.
- It then turns to **cyan** to indicate that it is starting the **myIOT** layer of the software,
  which includes potentially connecting to your Wifi network.
- It *may* turn **purple** indicating that it is *zeroing the angle sensor*.  **THE PENDULUM
  SHOULD NOT BE MOVING** as the clock tries to determine the 'straight down' position of the
  pendulum.   If you see **purple** (which you generally should not), then you *may* want to
  manually **zero the angle sensor** using the buttons or WebUI as described below.
- At the end of the boot process, all LEDs are turned off, and it enters it's normal display mode.


By default, the clock uses the **Diagnostic LED Mode** which lets one see at a glance how the clock
is performing. The other *LED Mode* is **Time Display LED Mode** which shows the current time using
the pixels so that you can tell what time it is in the dark.

- *Time Display LED Mode* **also** shows certain indicators if a problem with the clock is detected,
  but I much prefer the *Diagnostic Mode* while testing and getting used to the clock.  *Time Display
  LED Mode* is described in an *Appendix* at the end of this document.

In the default **Diagnostic LED Mode** the LEDs (from left to right) have the following basic meanings:

- 1st (left) LED - shows the **system status** and particularly the **WIFI Status**

	- **green** means that it is correctly connected to your Wifi network
	- **purple** means that it is not connected to your Wifi network, and is in **Access Point Mode**
	  so that you can connect directly to it from your phone, laptop, or computer.
	- **orange** is a temporary state after it has been in AP mode and you have entered
	  your Wifi SSID and password, meaning that the clock has connected
	  to your Wifi network and is waiting a bit before turning the AP mode off.
	- **red** means that the Wifi is not working correctly.
	- **blue** means that the Wifi is turned off.  In versions of the clock that include
	  a battery backup system, the clock will automatically turn the Wifi off during a
	  power outage to preserve the battery as long as possible and will turn the Wifi
	  back on once the power has been restored.

- 2nd LED - shows the **clock status**, whether it is starting, running, or off

	- **off** means the clock is not running and needs to be started
	- **white** means the clock is waiting to *Start Synchronized* when the time crosses the next minute.
	- **yellow** is very transient, lasting less than 1/2 second, and means that the clock is delivering the initial impulse to start the pendulum moving.
	- **cyan** means that the clock is *starting*.  It takes approximately 30 seconds for the pendulum to stabilize before the clock starts *running* normally.
	- **green** means that the clock is *running* normally
	- **orange** is a special color indicating that you have placed the clock in one of its *tuning* modes, and is not normally shown.

- 3rd (middle) LED - shows the **overall accuracy** of the clock within 150 milliseconds, where **green** is good, **blue** is 150ms or more slow, and **red** is 150 ms or more fast.

	- this LED will vary between **blue** to **green** to **red** as the clock speeds up or slows down and the **overall time accuracy** of the clock varies
	  from 150 milliseconds (slow) to -150 milliseconds (fast) from the correct time.  It is normal for it to be slightly non-green (cyanish as it goes a bit
	  slow and yellowish as it goes a bit fast), but it should not **stay** solid red or blue for extended periods of time.
	- The *span* of time covered by this indicator is parameterized and can be changed via the WebUI.  I set to +/- 300ms for problematic clocks :-)

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
	- If connected to your Wifi network, the clock syncrhonizes the RTC to NTP (Network Time Protocol) every two hours (parameterized)


If you see four green, or close to green, LEDs, all is well and the clock is running within 1/10 of a second of the correct time.

It is normal for the 5th LED to sometimes come on as red or blue.  It should transition slowly to green before turning off.

If any of the last three pixels remains BLUE or RED for more than 10-15 minutes, then it is likely the clock needs **maintenance** or **tuning**.


## 3. Buttons

There are two buttons on the face of the clock that can be pressed by inserting a toothpick or other similar item into the holes.

A button press can be **short** (instant - less than 3 seconds), **medium** (more than 3 seconds, but less than 8 seconds), or *long* (more than 8 seconds).

When you press a button, the rightmost LED will turn **white**.   If you hold the button for more than 3 seconds, the LED will turn **purple**,
and if you hold it for more than 8 seconds, the rightmost LED will turn **magenta**.

The **left button** is used to *start and stop the clock*, turn the *Wifi on and off*, or in extreme cases, to
do a *Factory Reset*.  The right button is used to *change the brightness* of the LEDs, change the *LED display mode*, and/or
to calibrate the angular sensor on the pendulum (*set zero angle*).

### Left Button

- short press will "start synchronized", or "stop the clock" if it is running
- a double press will "start the clock immediately" (if it is not running and pressed twice within 2 seconds)
- a medium press will "turn wifi off/on" (3 seconds or more)
- a long press will issue a "factory reset" (8 seconds or more) and should only be done if all else fails!

### Right Button

- a short press will "change the LED brightness" - pressing the button multiple times will increase the brightness of the
  LEDs until it cycles around and they go 'off'.
- a medium press will "change the LED display mode" - pressing the button for 3+ seconds will change the LED display between
  *Diagnostic LED mode* and *Time Display mode*.  You can tell which is which by the resultant LED display (assuming
  you have not turned the LED brightness all the way off!)
- a long press will "set the zero angle" - Occasionally it may be necessary to re-calibrate the angular sensor.
  This is because the magnet on the end of the pendulum tube is not glued on, but merely compression fitted,
  (so that the clock can be disassembled if necessary), and may have slipped during transport or by rough
  handling.

In order to calibrate the angular sensor (*set zero angle*) the clock should NOT be running and
the pendulum should NOT be moving and should be hanging straight down.  When that is the case
long-press the right button for 8+ seconds (until the rightmost LED turns to magenta) while being
careful NOT to move the clock.  The clock will read the sensor when you let up on the button,
which determines the "zero position" of the pendulum which is crucial for the clock to run correctly.

The *zero position* of the pendulum is used to determine when the pendulum has "crossed zero"
so that we can send an impulse to the pendulum on each swing to keep it moving and adjust it's
speed, as well as used in measuring the angle that the pendulum has traversed on each swing,
both of which are needed for the clock to keep good time.

Of course all of these functions can also be accessed via the WebUI (below), but the buttons
allow you to use the clock without needing to access the WebUI.


## Appendices




### Appendix A - **LED Time Mode** display

### Appendix B - the WebUI

### Appendix C - Serial Interface

### Appendix D - Tuning the Clock

### Appendix E - Mechanical Maintenance
