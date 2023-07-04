# theClock3 - Installing and Building the Software (Build Continued)

**[Home](readme.md)** --
**[Design](design.md)** --
**[Plan](plan.md)** --
**[Wood](wood.md)** --
**[Coils](coils.md)** --
**[Electronics](electronics.md)** --
**Software** --
**[Assemble](assemble.md)** --
**[Build](build.md)** --
**[Tuning](tuning.md)** --
**[UI](ui.md)** --
**[Troubles](troubles.md)** --
**[Notes](notes.md)**


This page describes how to install and build the **software**, upload it to the **ESP32**
and do an initial **test** of the PCB cables and sub components.

## A. Install and Build

### 1. Install the [**myIOT**](https://github.com/phorton1/Arduino-libraries-myIOT/blob/master/docs/getting_started.md) Library

The software is built within the [**Arduino IDE**](https://www.arduino.cc/en/software) development environment and depends
on the [**myIOT**](https://github.com/phorton1/Arduino-libraries-myIOT) library.  Please follow the below link for
instructions on how to *prepare your Arduino IDE to build ESP32 projects* and install the *myIOT library*

[**Getting Started with the myIOT Library**](https://github.com/phorton1/Arduino-libraries-myIOT/blob/master/docs/getting_started.md)

As described there, you will also need to install the following dependencies:

- **WebSockets** by Markus Sattler - Version 2.3.6
- **ArduinoJson** by Benoit Blanchon - Version 6.18.5
- **ESP Telnet** by Lennart Hennings - Version 1.2.2
- [**Arduino ESP32 filesystem uploader**](https://github.com/me-no-dev/arduino-esp32fs-plugin)

### 2. Copy or clone **this** repository into your **Arduino Sketch Folder**

The location of your Arduino Sketch Folder can be found in the Arduino IDE via
the **File - Preferences** menu.

![soft-ide_location.jpg](images/soft-ide_location.jpg)

I have set mine to *C:\src\Arduino*, but yours will probably be something like **C:\Users\YOUR_NAME\Documents\Arduino**
(**Arduino** under your **documents** folder).

To install it from the **Zip File** go to the [main repository page](https://github.com/phorton1/Arduino-theClock3)
and press the green **CODE** button.  Download the **Arduino-theClock3-master.zip** file, and copy it's inner
folder (*Arduino-theClock3-master*) into your *Arduino Sketch Directory* and **rename the folder to "theClock3"**.

To clone it from github, open a **command prompt** window, change directories  to your *Arduino Sketch Directory*
and clone the repository

```
C> cd C:\Users\YOUR_NAME\Documents\Arduino
C> git clone https://github.com/phorton1/Arduino-theClock3.git theClock3
```

In either case you should now be able to open **theClock3.ino** within the Arduino IDE.

### 3. Install additional libraries

From the Arduino IDE **Library Manager**, install

- the **AS5600** library, version 0.3.5, by **Rob Tillart**
- the **Adafruit NeoPixels** library

### 4. Building and Uploading

Start with a **bare ESP32 Dev Module** that is **NOT** plugged into the PCB we created.

1. Connect the **ESP32 Dev Module** to your computer via a USB Cable
2. Open **theClock3.ino** in the *Arduino IDE*
3. In the Arduino IDE **Tools** Menu, set the *Board* to **ESP32 Dev Module**
4. Make sure that the *Partition Scheme* is set to **Default 4MB with spiffs (1.2MB App/1.5MB SPIFFS)**
5. Make sure that the correct **COM Port** for your ESP32 is selected.
6. Press the **Right Arrow** icon to **compile and upload** the **code** to the ESP32
7. Use the **ESP32 Sketch Data Upload** command from the *Tools* menu to upload the
   contents of the sketches' **data** folder to the **SPIFFS File System** on the ESP32

## B. Software Overview

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
  is *greater* than ID_MIN_MAX_MS (the clock is running slow), we set the
  *target angle* to ANGLE_MAX so that it will swing wider, start hitting
  the *magnetic spring* and speed up.  When the **cumulative ms error**
  is *less* than -ID_MIN_MAX_MS (negative means the clock is running fast),
  we set the *target angle* to ANGLE_MIN so that it will swing less and
  beat slower.  By alternating between these two *target angles* the
  clock keeps *relatively accurate* time.

- **PID** - And then finally we have the (default) **PID mode** which
  introduces the **secondary PID controller** which attempts to optimize
  the *target angle* so that the clock beats as close to **1000 ms per
  beat** as possible by minimizing the instantaneous and cumulative
  *milliseconds errors* simultaneously.

### 3. Typical Serial Monitor Output

There is a **lot** of Serial Output from the ESP32.  Use the *Arduino IDE
**Serial Monitor*** to see the output and to **type commands** to control the clock.
The Serial monitor must be set to **115200 baud** and to send **Both NL and
CR** (new line and carriage return).

The first time you boot it, you should see something like this in the
serial monitor:

![soft-start1.jpg](images/soft-start1.jpg)

When you boot a [**myIOT device**](https://github.com/phorton1/Arduino-libraries-myIOT),
like **theClock3**, the serial output tells you a lot about the device as it boots,
including providing a complete listing of the **ESP32 partitions** and **contents
of the SPIFFS file system**. Each line starts with **time stamp**.  Since we have
not yet connected to NTP (Network Time Protocol), the ESP32 does not know what time
it is, and so it defaults to starting at midnight on January 1, 1970.
The messages are **nested** (in many cases) by their **call level** within the code

The most important thing to note in the above log output,
as highlighted in <font color='red'><b>red</b></font>,
is that the ESP32
**failed to initialize the AS5600** angle sensor module.
That's ok!! We haven't *plugged it in yet* !!  The listing then continues:

![soft-start2.jpg](images/soft-start2.jpg)

A **myIOT device**, by default, sets itself up as a **WiFi Access Point**,
as highlighted in <font color='blue'><b>blue</b></font>
in the serial output. You can see that the *SSID* of the Access Point
is **theClock3.3** and the *IP Address* is 192.168.1.254. Also we see,
highlighted in <font color='red'><b>red</b></font> that the ESP32
**COULD NOT SET** the **AS5600 zero angle** five times.
That's ok too!  We just want to draw your attention to
these messages for later.



### 4. See a list of the Clock's PARAMETERS (Values)

If we now type the command **"values"**, followed by a carriage return,
into the serial monitor, the clock will display a list of all of it's
**PARAMETERS** and their values.

![soft-values.jpg](images/soft-values.jpg)

### 5. Try settting a PARAMETER and REBOOTING

There are a **lot** of different parameters available for controlling
and understanding the clock.  They will be described more fully as we
move forward through the [Tuning](tuning.md) and [User Interface](ui.md) pages,
but for now it is good to learn that you can **set parameters**
by typing their **name**, followed by an **equals sign**, followed
by the **value** you want to set them to.

**parameter = value &lt;carriage return>**

For instance (from the Serial Monitor), we can **turn the ESP32
WiFi off** and back **on** by typing the following into the Serial Monitor (where
each line is followed by a *carriage return*)

```
wifi=0
wifi=1
```

Note that you can type the word **"reboot"**, followed by a carriage
return, into the serial monitor to reboot the ESP32.

![soft-wifi_on_off_reboot.jpg](images/soft-wifi_on_off_reboot.jpg)

Try it!


### 6. Connect to theClock's WebUI

One thing that can be tested with the bare **ESP32 Dev module** at this point is
that you can properly attach to the ESP32 via WiFi and access the **browser based Web
User Interface**.

Please see **Sections 4** and **Section 10** in the
[**Quick Start Guide**](https://github.com/phorton1/Arduino-theClock3/blob/master/docs/QuickStartGuide.pdf)
for details on how to connect to, and find the WebUI on your local **WiFi network**.

It is not **necessary** to use the WebUI to test, control, and configure the clock,
but it can make it **much easier** than typing commands, long-hand, into the serial monitor.

![soft-WebUI.jpg](images/soft-WebUI.jpg)


## C. Submodule Test of the PCB

The point of introducing the **software** at this point in the documentation is so that
we can do a quick test of the **PCB** and some of the **peripherals and cables**
that were built on the **[Electronics](electronics.md)** page.

Particularly, we can test that the **LEDs** and **buttons** are working,
that we can connect to the **AS5600 angle sensor module**, and that the
**coils** *energize* and can *repel* a magnet.

To begin with, you should have already **uploaded the firmware and the
contents of the SPIFFS filesystem** to a bare ESP32 as described above, and
be able to connect to it and see, and type input into the **Serial Monitor**.

You should have already (also) checked the PCB out with a *multimeter* to
make sure there are **no short circuits**, particularly between any of the
power rails and ground,.


### 1. Unpower everything and plug the ESP32 into the PCB

**Plug the ESP32 into the PCB** and **power it up** by attaching it to your computer via a serial cable.

It should boot normally and not **explode** or overheat!

In the *Serial Monitor*, you should see the same thing as before.

**De-power the ESP32**.


### 2. Plug in the LED Strip and Repower

**Plug in the LED Strip** and **power up the ESP32**.

*note: the LEDs are in installed in the box from right to left, so
the **last** LED in the strip becomes the **leftmost** LED in the box
and the **first** LED in the strip becomes the **rightmost** LED in the box*.

You should see a series of 5 LEDs lighting up in <font color='cyan'><b>cyan</b></font>.
Then, because it cannot initialize the AS5600, all five LEDs should start flashing.
It flashes <font color='red'><b>red</b></font> **five times** for each failed attempt,
and *it will retry five times* to initialize the AS5600, so, in total you
*should* see **25 red flashes of all five LEDs** as it boots.

After 30 seconds or so the last LED in the strip should turn
<font color='green'><b>green</b></font>
if you connected the ESP32 to your home Wifi in *B6. Connect to theClock's WebUI* above, or
<font color='purple'><b>purple</b></font>
if you did not and the clock is in *Access Point Mode*.

In either case the 2nd to last LED should continue to flash <font color='red'><b>red</b></font>
to remind you that, *without the AS5600 module, the **clock will not work***.  That's
ok!  We can still test the PCB!



### 3. Test the button(s)

**Short Press** (quickly press and release) the **right** button a few times.
Each time you do it, the *1st LED* in the strip should turn **white** while the button
is pressed, and the LEDs should get brighter when you release it.
There are **15 levels** of brightness for the LEDs, then they will
turn off, then on again at their minimum level to repeat the cycle.

While you do this you should see various output in the console window.
if you are running the *Web UI* you should see the **LED BRIGHTNESS**
parameter change values each time you press the button.

To test the left button, **Medium Press** the **left** button by holding for two or more seconds
(but *less than 8 seconds!!*) until the 1st LED changes from **white**
to <font color='cyan'><b>cyan</b></font>.
When you let it up, you will toggle the **WIFI on and off**.

The last LED in the strip will turn <font color='blue'><b>blue</b></font> when
the Wifi is turned **off** and will turn back to <font color='green'><b>green</b></font>
when it connects to your Wifi, or to <font color='purple'><b>purple</b></font>
when it starts an Access Point.

If you are running the *WebUI* and you turn Wifi **off** you will notice that
the buttons and entry controls in the browser will be *greyed out* and *disabled*
while Wifi is turned off.  When you turn Wifi back **on** again, after a
few seconds, the WebUI *should* **reconnect** to the clock and *enable*
the buttons and entry controls.

You probably want to make sure that you *leave the ESP32* with the **WiFi** turned on,
and the **LEDS** at medium brightness after this step!!

When you are done testing the buttons, **De-power the ESP32**.


### 4. Test the AS5600 Connection

**Plug in the AS5600 cable** and *power up the ESP32**.

If we plug the AS5600 in with it's cable, and reboot the ESP32, the AS5600 should now initialize
properly, **no longer flashing RED 25 times**, and giving the **AS5600 connected=1** message
in the console as shown below:

![soft-AS5600_connected.jpg](images/soft-AS5600_connected.jpg)


The last LED in the strip should eventually turn
<font color='green'><b>green</b></font> or <font color='purple'><b>purple</b></font>,
as before, but now, instead of **red** the 2nd to last LED will flash <font color='orange'><b>orange</b></font>
indicating that, although the AS5600 initialized, we **still could not set the zero angle**.

That's because there's no **magnet** in front of the AS5600!  That's ok!!  All
we wanted to do here was test that we can connect to the AS5600 correctly via the cable.


### 5. Test the coils

Use a **multimeter** to ensure that the coils have some resistance (about 6 ohms) **BEFORE
CONNECTING THEM TO THE CIRCUIT**!!!

Have a **magnet** handy.

We plug the coils **in series** into the two **serial coil** connectors.
The orientation of the plugs depends on and/or determines the orientation of the magnets
that will be glued into the pendulum.  Plug the coils in such that the
same colors are on each side, with **orange** on the left and **white** on the right.

![soft-coil_plugs.jpg](images/soft-coil_plugs.jpg)

**De-power the ESP32** (unplug the serial cable), **plug in the coils**, and **power up**
the ESP32.

After it has finished booting, type the command **"motor=1"** into the Serial Monitor.
This command energizes the coils (at the **POWER_MIN** parameter).
You should be able to hold the magnet near the face of each coil and feel a
repulsion or attraction.

This is a good opportunity to determine the **orientation** of the *magnets and coils*.
We want to figure out how to arrange the coils and magnets such that
the magnet is repulsed by each coil when the coils are facing each other and the magnet
is in between them.   Once you have determined that, **mark** the **front coil** with
a piece of tape or felt marker, and **mark the front** of the magnet.

**Remember** (write down) the orientation of the coils, plugs, and magnets, so that
you can glue the magnets in the pendulum in the correct orientation in the
next steps of assembly!

De-energize the coils by typing **motor=0** into the Serial Monitor.


## D. Summary

That's about it for an introduction to the software.  We just want to make sure
the PCB basically works before we embed it, the LEDS, and the coils
into the **Box**, and the AS5600 cable and Sensor into **Frame**, as
we continue building the clock.



**Next:** - Finish the other [**Sub-Assemblies**](assemble.md) of the clock, including the **gears, pendulum, and box**
