# The Clock - User Manual

**[Home](readme.md)** --
**[Design](design.md)** --
**[Wood](wood.md)** --
**[Plastic](plastic.md)** --
**[Electronics](electronics.md)** --
**[Coils](coils.md)** --
**[Assembly](assembly.md)** --
**[Tuning](tuning.md)** --
**[Quick Start](quick_start.md)** --
**User Manual** --
**[Trouble Shooting](troubles.md)** --
**[Notes](notes.md)**

Welcome to theClock v3.2 !!

This clock has been especially **hand-crafted** for **YOU**.

Please visit **https://github.com/phorton1/Arduino-theClock3** and view the web-pages in the repository
for many more details about the clock, it's design and construction, maintenance and operation.

Although you could theoretically merely unpack the clock, plug it in, and press the left button
twice to start it ticking, **it is highly recommended that you follow the Quick Start guide**
and **first connect it to your local WiFi network**, and use the **Synchronized Start** method
described below to start the clock.

The clock is designed to **synchronize itself** to the Network Time Protocol (NTP) if it is given a
Wifi connection, with the idea that it should run within a second or so of the correct time for
the forseeable future (until, hopefully, at least the *Y2038 problem*, when unix time will overflow 32 bits).
If it is not connected to the internet, the best it can do is synchronize itself to the internal
RTC (Real Time Clock) on the embedded computer, which, at best, will only be correct to within
a few seconds a day.

A few notes:

- The clock was shipped to you after a fresh **Factory Reset**.  You can, at any time, return the clock's
  software to it's initial state by issuing a Factory Reset as described below.
- The **Wifi has been enabled by default**. It can be turned off (as described below)
  if you are paranoid about your home network security.
  Substantial efforts were put into ensuring that the clock does not create any significant security risk
  to your home network, but just in case, I added the ability for you to disable the clock's Wifi access entirely.
- Although I have tried my best to produce a reliable clock, **some maintenance may be necessary**.  In particular
  it may be necessary once every few years, or more often, to lubricate the bearings and brass tubes.
- **PLEASE DO NOT TURN THE SECONDS HAND COUNTER-CLOCKWISE!!** The hands are merely compression fitted to the
  brass tubes, and the seconds hand was intentionally designed to be a tighter fit.  It is not intended to
  move relative to the tube and gears, whereas the hour and minute hands ARE intended to be moved
  relative to the tubes and gears, in order to for you to be able to set the time.
  **When moving the seconds hand** please always turn it **clockwise**!!  It should smoothly go
  "clickity-clack" as the pawls slide over the seconds wheel.
= **ANY FLASHING LEDS ARE INDICATIVE OF A PROBLEM**.  If you see any LEDS that are flashing
  regularly (once per second or so) it means something is wrong!!   Please see the online
  **[Trouble Shooting](trouble.md)** guide, or contact me if you see flashing LEDs!


## 1. Quick Start

- Unpacking
- Preparation
- Power Up!
- Connect to theClock
-- Change Default Password
-- Specify Wifi SSID and password
-- Verify Connection
- Reboot
- Start Clock Synchronized
- Stop the Clock

### 1A. Unpacking

- Open the box and remove the top piece of styrofoam
- Remove the **power supply** and **baggy with allen wrench** from the lower portion of the clock.
- Remove **the Clock** from the box and place it on a flat level surface
- Remove the **three pieces of cardboard at the top of the clock**
  labelled 1,2, and 3, that are securing the mechanism by following the arrows that are drawn on them.
- From the back **remove the piece of green tape** securing the pendulum to the bottom cardboard inserts.
- Remove **the two bottom cardboard inserts** by following the arrows drawn on them.

### 1B. Preparation

Efforts were made to ensure that the clock would not be damaged during transport.
However, some shifting may have taken place, and so, before anything else we need
to make sure that the mechanism is properly aligned and engaged, that the pendulum
swings freely, that the gears turn correctly, and that the hands are aligned so
that they don't collide.

- From the top of the clock, the two **pawls should be resting on top of the seconds wheel**
- The **pawls and arms should move freely**.
- The **bearings should be riding on the cam**
- The **hands** should be square to the tubes they are connected too with approximately equal spacing between them.
  Adjust them slightly as necessary to ensure that they don't hit each other as they turn.
- You should be able to **move the seconds hand clockwise** and the pawls should go "clickity-clack" as the second hand moves.
- There should be **very little resistance** when moving the second hand!!
- As you move the second hand clockwise multiple revolutions, the minute hand and hour hand should move
  slightly, but noticably.
- The **penduluum** should swing freely **in the middle of the channel** and not touch the plastic at any point.
- The **pendulum should swing 4-5 times** back and forth when released from an extreme posision.  It should NOT
  swing only once or twice and then stop.


At that point you should be able to swing the pendulum back and forth, and the clock should tick-tock,
with each pawl, in turn, grabbing and turning the seconds wheel clockwise.  You can put your index finger
into the hole on the top of the clock and move the pendulum back and forth and it should not take
much force to keep the pendulum moving and the clock ticking.


### 1C. Power Up

When the clock is mechanically ok and everything is nice and loosy-goosey it is time to connect the
power supply and boot it for the first time.

You can connect the USB power supply from the left, or right of the clock as you desire.

When the USB power supply is connected the computer will boot, and you will (should) see
the five LEDs on the front of the clock light up in **cyan** from left to right, followed
by various other LED colors as the clock boots.

When the clock has sucessfully booted for the first time, the **left most LED should be
purple** and no other LEDs should be lit, and particularly **NO LEDS SHOULD BE FLASHING!!**
Please see the online **[Trouble Shooting](trouble.md)** guide, or contact me if you see flashing LEDs!

The meaning of the LEDs is described more fully below, but for now suffice it to say
that the **Left Purple LED** indicates that the clock is in **AP (Access Point)) Mode**
which means that you can connect to it via Wifi.


### 1D. Connect to the Clock (in Access Point Mode)

When the clock is in **AP (Access Point)) Mode** (the left-most LED is **purple**)
it will show up on your home computer or phone as a **Wifi Network** to which you can attach.

You will **connect** to the clock, be required to **change it's wifi password**, and then
will tell it how to **connect to your home Wifi network**.  By connecting to your home
Wifi network (and presumably to the internet via that) the clock will have access to
the **NTP (Network Time Protocol)** from which it can find the correct time.

**APART FROM CONNECTING TO NTP THE CLOCK DOES NOT USE OR PRESENT ITSELF IN ANY WAY TO THE INTERNET**

However, once it is connected to your home Wifi network, **YOU** will be able to access it
via a browser based **WebUI**.

Please perform the following steps to connect the clock to your Wifi network.

- From a laptop or home computer (preferable) or phone/tablet connect to **theClock3** wifi network
- The default password is **11111111**  (eight ones).

![11-ConnectAP.png](images/08-user_manual/11-ConnectAP.png)

- After a few seconds your system **browser should automatically pop up** with
  a (redirect page and then) a page which requires you to set a **new password**.
- Enter (and re-enter) and **remember!!** a new password for your clock's AP Mode.

![12-SetPassword.png](images/08-user_manual/12-SetPassword.png)

- After you set the AP Mode password, you will be presented with a page that allows you to
  tell the clock the **SSID** and **password** of your home wifi network.
- These credentials are stored in an encrypted form on the clock's computer and
  will never be presented to anyone in a human readable form.

![13-ConnectWifi.png](images/08-user_manual/13-ConnectWifi.png)

- **Enter the SSID and password of your home Wifi network** and press the **Connect
  Button**.

That's it!   The only thing that can be done from **AP Mode** is to tell the clock
a wifi network to connect to.

**If, after 15 seconds or so, you do NOT receive an error message, then everything worked ok!!**

When the clock is connected to your home wifi network, it is said to be in **Station (STA) Mode**.

The clock *tries* to report when it has sucessfully connected to your home wifi network, but
because it cannot be connected in both AP and STA mode at the same time (unless your home
wifi network *happens* to be on the same "channel" as the clock's AP mode), typically
it is not able to report to the browser that it connected successfully.  However, you
may get lucky and see a message of the form "theClock3 succesfully connected to YOUR_SSID
at IP Addresss XXX.XXX.XXX.XXX".

![14-Connected.png](images/08-user_manual/14-Connected.png)

In any case, **if you DONT see an error message at this point**, the clock has succesfully connected
to your home wifi network.

If you **DO RECEIVE AN ERROR MESSAGE** please re-enter the SSID and password and try again. You
can reboot the clock (disconnect and reconnect the power supply) and try this again.  You may
need to reboot your laptop/tablet/phone, although everything *should* work the first time through.


### 1E. Reboot

Once you have connected the clock to your Wifi network, you may reconnect your Laptop/Tablet/Phone
to your home wifi network.  When you do so, after about 15 seconds, the leftmost LED should change
from **purple to green**.    When the leftmost LED is **green** it indicates the the clock is
correctly attached to your home wifi network in **STA Mode**.

If after 15 seconds or so the LED does NOT turn green, you may need to reboot the clock by
disconnecting and reconnecting the USB power supply.

**It is not a bad idea to reboot the clock in any case at this point**.

When you reboot now, the same pattern of LEDs (five LEDS light up in **cyan**,
and then varioius other LED color show) should occur, except for now the
left-most LED should be **green** after the clock has sucessfully booted
and attached to your wifi network.


















## 2. Meaning of the LEDs


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


**Next:** [**Trouble Shooting**](troubles.md) potential problems with the clock ...
