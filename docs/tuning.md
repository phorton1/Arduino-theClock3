# theClock3 - Install Firmware, First Boot, and Tunning the Clock

**[Home](readme.md)** --
**[Design](design.md)** --
**[Build](build.md)** --
**[Wood](wood.md)** --
**[Coils](coils.md)** --
**[Electronics](electronics.md)** --
**[Software](software.md)** --
**[Box](box.md)** --
**[Assembly](assembly.md)** --
**Tuning** --
**[User Manual](user_manual.md)** --
**[Trouble Shooting](troubles.md)** --
**[Notes](notes.md)**


- Sensor
- Zero
- Power Swings
- Tune Pendulum
- Basic Min Max Swings (NO PID CHANGES!)
- Tune Pendulum
- Attach Magnet Spring

Try to physically adjust the clock
to work with the program defaults.
Minimize variations between clocks


## Initial Setup

After building the clock one must verify that the angle sensor and coils are
working correctly, and then setup the pendulum and test the clock.

### Basic connectivity

The initial setup is best done with the ESP32 connected to a computer with
a serial port monitor as well as connecting to it via WiFi to use the WebUI
to adjust parameters.

At this point of the documentation project, it is beyond scope to provide
much detail about the basic process of hooking up the ESP32 and WebUI.
However, here is a rough step by step description of the process.

1. Program a stand alone ESP32 (and the SPIFFS file system) with the Arduino IDE.
2. Use a serial port monitor with ANSI colors (like Putty) to see the serial output from the ESP32.
3. Using the captive portal Access Point, or by typing commands in the serial monitor, get
   the ESP32 connected to the same Wifi network as your main computer (laptop).
4. Bring up the webUI in a browser by going to it's IP address.  I usually
   establish a fixed IP address on my Wifi network for the MAC address of the ESP32.

After that, you can plug the ESP32 into the circuit board and boot it up, and
should see the LEDs flash their characteristic startup sequence.


### Fundamental Testing

1. Use the WebUI TEST_MOTOR (-1 == repulse, 0 == off, or 1 == attract) function
   to make sure the coil is wired correctly vis-a-vis the orientation of the
   magnet in the pendulum.

2. With the pendulum hanging straight down, and not moving, with the clock
   on a level flat surface, press the ZERO_ANGLE button to calibrate the
   angle sensor.   THE MAGNET ON THE PENDULUM MUST BE ORIENTED SUCH THAT
   IT IS APPROXIMATELY 180 degrees (well away from zero) WHEN HANGING
   STRAIGHT DOWN.    This *should* result in setting the zero angle to
   some value (like 123 degrees).  THE RAW ANGLE SENSOR CANNOT CROSS ZERO
   DURING NORMAL OPERATION!!!

3. Once the angle sensor is zeroed, you *should* be able to set the CLOCK_MODE
   to "Stats". toggle the CLOCK_RUNNING boolean to 'on' and move the pendulum
   with your hands and see reasonable angles, moving from -15 to 15 degrees or
   so at the extremes, displayed in the serial port.  Moving the pendulum to
   the right should increase the angle, and moving it to the left decreases
   the angle.


### POWER and ANGLE settings and WEIGHT and SPRING adjustments

The defaults for DEAD_ZONE, POWER_MIN, POWER_MAX, ANGLE_MIN, and ANGLE_MAX
should be pretty good, but each can, in turn, be tested and adjusted with
the UI.

POWER_MIN (tested with CLOCK_MODE = Power_Min) should be set so that the
clock reliably ticks and tocks at a minimum angle (about 5 degrees to
either side).

POWER_MAX is probably ok at the default of 255, and can be tested with
CLOCK_MODE=Power_Max.  The pendulum should swing increasingly widely
until it starts hitting the clock at the extremes, and the test terminated
as soon as it does.

The default ANGLE_MIN is 9.0 degrees.  Using CLOCK_MODE=Angle_Min should
cause the clock swing at roughly 9 degrees to either side.  The spring
should not come into play at the minimum angle. The weight
and should be adjusted so that the clock ticks slighly slower than
one second, on average, about 1010ms, at ANGLE_MIN.

The default ANGLE_MAX is 11.0 degrees.  Using CLOCK_MODE=Angle_Max should
cause the clock swing at roughly 11 degrees to either side.  The spring should
be adjusted so that the clock ticks slighly faster than one second, on average,
about 990ms, at ANGLE_MAX.

This process is repeated until the clock reliably ticks slower than 1 second
(on average) at ANGLE_MIN and faster than 1 second at ANGLE_MAX.

The default DEAD_ZONE (angle away from zero at which pulses begin) of 0.7 degrees,
and DUR_PULSE (pulse duration) of 150ms should work, but can be messed with to
get more or less power into the pendulum or to address jerkiness.

ONE MAY NEED TO SET THE ZERO ANGLE OCCASIONALLY as the sensor may *drift* over time.



## PID Controller

The PID controller that controls the angle of the pendulum is pretty simple.

We measure the angle of the pendulum every few  milliseconds. We use a
minimum movement threshold to avoid hysterisis (about 1/10 of a degree) and
only take new values when the pendulum has moved significantly.

The pushes start just after each **zero crossing** as determined by the *DEAD_ZONE*,
and go on for the parameterized DUR_PULSE millisecond duration.

On each impulse the PID controller adds and averages the most recent maxium
left and right angles into the **average angle**, and compares that to
the TARGET_ANGLE, to get an *angle_error*.   The *angle_error* is the
first (Proportional) input to the PID controller.  We keep a running
sum *total_angle_error* of the accumulated angle errors which is used as the
second (Integral) input to the PID controller.   We use a simple delta
between the previous angle error and the current angle error as the
third (Derivative) input to the PID controller.

The PID controller spits out the power (between POWER_MIN and POWER_MAX)
that we should apply to the coil and the impulse is delivered for the duration
as described above.

The PID controller *should* regularly
achieve +/- 0.1 degree control.


## Algorithm1 - MIN_MAX

We keep a lot of statistics about the clock, but one of the most important is the
**total_millis_error** ... which is running sum of the millisecond errors on each
clock cyle.

When the clock ticks faster than 1 second, cycle times will measure like 992, 979, and 998,
and when it clicks slower than 1 second, you get get values like 1003, 1021, 1013 ms.

We subtract 1000 from the cycle time to get the *instantaneous error* which is
positive if the clock is running slow, or negative if the clock is running fast,
and add it to the running **total_millis_error** ... which is the most basic measure
of how far off the clock is from the correct time.

MIN_MAX_MS defaults to 50 milliseconds, and is also used for the colors of the LEDs.

Very simply, if the total millis error is more than MIN_MAX_MS, we set the
target angle to MAX_ANGLE to speed the clock up, and if the total_millis_error is
less than negative MIN_MAX_MS, we set the target angle to MIN_ANGLE to slow it down,
so the clock cyclicly goes from plus 50 milliseconds to minus 50 milliscond from the
correct RTC time over and over.  The speed of the pendulum change is audibly noticable.


## Algorithm3 - PID_MODE




**Next:** [**User Manual**](user_manual.md) - a more comprehensive end-user manual for the clock ...
