# Accurate Wooden Geared Clock with Electromagnetic Pendulum

This clock uses wooden gears and a simple electromagnetically driven pendulum
with the goal of keeping accurate time.

It uses an **angle sensor** to detect the position of the
pendulum, an **electromagnet and coil** to provide pulses of energy
to the pendulum to keep it moving, as well as a **magnetic spring** to
allow us to speed up, or slow down the pendulum.

It has a computer (an **ESP32**) in it, with a *Real Time Clock* (**RTC**)
that is fairly accurate (to within about 1 second per day).  Furthermore
the computer can connect to the internet to use Network Time Protocol (**NTP**)
to synchronize it's RTC within a few milliseconds of the *correct* international
**standard time**.

Roughly stated, the goal of this project is to produce a **wooden geared clock**
that will be accurate to within one second per day, or, when connected to the
internet, within one second at any given time over it's working lifetime.


## Basics

There is an **angle sensor** on the pendulum.  By comparing successive
measurments we can determine the pendulum's *direction*, when it *crosses zero*,
and the extreme *minumum and maximum* angle during any given swing.

The pendulum has a **magnet** in it that aligns with an **electromagnetic coil** in the
box that can *repulse* (push) the pendulum a little on each swing, providing the energy
to keep it moving.  By increasing the power provided to the coil we can make
the pendulum swing further, and by decreasing it, we can make it swing less widely.

There is another **pair of magnets**, one in the stem of the pendulum, and one
that is affixed to the clock, that is adjustable, that together, via magnetic
repulsion, act as a **spring** when the pendulum swings sufficiently far.
Because of this spring, the pendulum swings *faster* when it swings further
*and slower* when it swings less.

We establish a working **minumum and maximum angle** at which the clock
functions, by which we mean that it *ticks* and *tocks* reliably within
design constraints.

We adjust the weight on the pendulum so that the pendulum swings a little
slower than one second (about 1010 milliseconds) at the minimum angle.
We then adjust the spring so that it swings a little faster than one
second (about 990 ms) when it swings at the maximum angle.

The clock works by using a *PID controller* to swing at a given ANGLE by
increasing or decreasing the amount of power fed to the coil during
each swing.  We call this angle the **target angle**, and when working
correctly, the clock will reliably swing within a few tenths of a degree
of the given target angle.

We then use a second PID controller to adjust the target angle to minimize
the clock's **error** (in milliseconds).  On each swing, there is an instantaneous
error, which is how many ms faster, or slower, than 1000ms (one second) a particular
swing took.  These errors can accumulate, causing the clock to run slower,
or faster, overall, than the correct time.  The second PID controller is tuned to
minimize both the intantaneous, and the cumulative, errors, so that the clock beats
at very close to 1000ms per beat with a time very close to the 'correct' time
(as given by the RTC).

This two stage PID controller was an evolution. At first I merely tried to
use a single PID controller to directly minimize the instantaneous and
cumulative ms errors, but I found that it was better, both mechanically,
and aesthetically in engineering terms and in the experience of the 'tick-tock'
sound, to try to get the clock to first swing at a relatively constant angle, and
THEN to adjust that angle subtly to correct the time.  The single PID
controller approach resulted in much more noticable changes in the speed
and swing of the pendulum, and adding the second PID controller significantly
smoothed out the behavior of the clock.

The swing error is based on subsequent calls to the millis() function, rather
than comparing the time directly to the RTC clock.  This means that the algorithm
itself can drift from RTC time.  We allow this to happen and provide a separate
synchronization method, onSyncRTC(), to occasionally correct for this potential drift.
This allows us to keep track of the drift between the algorithm and the RTC and
isolates the basic swing PID controllers from changes in the RTC.

Finally, if connected to the internet, we occasionally synchronize the RTC
to NTP time to correct for the ESP32 clock drift.  Although we display
the local time in LOG messages, etc, the algorithms all *should* work
from UTC time to prevent changes in daylight savings time from affecting
the clock, with the notion that the user will manually correct for daylight
savings time by merely moving the hour hand as necessary.


## Pixel Mode - show current time

I designed the box with 5 LEDs.  How do I show the current time?
And I would simultaneously like to show at least the cumulative
error so I know the time is correct-ish.



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




## Starting the Clock (onStartClockSynchronized)

	// Tick == the right pawl falling into the seconds wheel (pendulum moving left)
	// Tock == the left pawl falling into the seconds wheel  (pendulum moving right)

	// The second hand is attached such that the Tock is zero, and the Ticks are
	// in the middle of seconds.

	// The user has put the second hand on zero (in a just completed 'tock" position)
	// with the other hands ready to go at the next minute, and they press
	// the button sometime during the minute before that.
	//
	// At 1 second before the crossing (59) plus the START_DELAY millis we issue
	// the initial starting pulse, which we duly record (but don't reallly use)
	// as the 'start_time' and 'start_time_ms'.

	// My first clock3 moves to the right on the initial pulse and the default
	// START_DELAY is 900ms.

	// What is important is that the initial left crossing occurs at about 500ms
	// after the starting second. This *should* move the seoncd hand to the
	// middle of the first second. We record this time as 'time_init' and 'time_init_ms'.
	//
	// The pendulum swings all the way to the left, and then swings back to the right (tocks)
	// moving the second hand to the first second at about 1000ms (one second).
	//
	// So, note that we are actually syncrhonizing to the MIDDLE of the seconds, not to
	// exact seconds crossings!!
	//
	// After that swing to the right (tock), the pendulum swings to the left again,
	// for it's second left crossing (and possibly it's first audible 'tick').
	// This second left crossing establishes BEAT NUMBER ONE at about 1500ms after
	// the desired start time, and henceforth we keep 'time_zero' and 'time_zero_ms'
	//
	// So, in proper operation,

	econd #1


	// so the first 'tick' is about 500 ms.
	// What is important is that the pendulum does left zero crossings (ticks) in the middle of seconds,
	// and right crossings (tocks) very close to the exact second.

	// So, in my example, after the starting pulse, the pendulum moves to the right, and does the first
	// tick (left zero crossing) at about 500ms. It should not actually 'tick' due to the position of the pawls.
	// but the first right crossing SHOULD "Tock", moving the


	, so that the first tickpendulum moves
	// to the right very close to the exact starting minute crossing.

	// The START_DELAY *should* be adjustable for a clock that typically
	// starts to the left but the worst case is that the clock is physicaly
	// 1 second off.

	// What is important is that the left crossings (tocks) occur in the middle
	// of the RTC clock seconds, around 500 ms



	It should not tick, but it may tock if the left pawl
	// was poised to fall.

	not already in first left crossing
	// occurs at about 500ms after the minute, and with luck, the
	// second left crossing, which establishes the 'time_init' and
	// 'time_init_ms' variables, corresponding to beat zero (0), will
	// occur about 1500ms after the minute.

	// What is important is that the left crossings occur at an even
	// second plus approximately 500ms ... well in the middle of the
	// RTC clock's 'second'.

	// Thereafter we keep track of the number of beats (num_beats) and
	// the time of each left zero crossing in the values 'time_zero' and
	// 'time_zero_ms' and are careful to only start RTC synchronization
	// very near that time (well in the middle of a second, close to 500ms).

	//
	(right after the num_beats changes)

	, because the pendulum has a frequency
	// of about 1/2 second, the.
	// The idea is that it will swing back, doing the first left
	// crossing approximatly 500ms after the minute, so that the
	// right crossing will be at about 1 second after the given minute,
	// and the second left crossing, which establishes the time_init
	// will occur at 1500 ms after

	// and that we will do what is necessary to get the clock aligned
	// back to that moment.

	// We are using left crossing to determine cycle time, and on
	// my initial clock, the starting pulse typically pushes the
	// pendulum to the right.  We set the START_DELAY parameter
	// such that the first left crossing occurs at about 500ms
	// after the minute, and the second one at about 1500ms,
	// establishing a 'init_time' of the minute crossing + 1
	// second.

	// What is important is that


	// We will give a starting after crossing the next '59' seconds, plus
	// some parameterized delay.

	// The difficulty is that we don't know when the actual first tick takes place.
	// We will assume it moves to the right on the pulse and will swing to the left
	// approximately 1/2 second after we issue the pulse.


	// It depends on the state of the hardware ... the clock may be predisposed
	// so that the pawls grabbing starting on the actual impulse with a forward tick,
	// or on the the next back swing, or some time later ... leading us to experiments
	// with the optical mouse sensor to detect the wheel movements.



## Controlling the time - ticks/beats versus ESP32 clock versus NTP

As described above, the clock manipulates the pendulum so that ticks average to 1000 ms
as based on the ESP32's millisecond timer.

By and large this means that the clock will keep time approximately as well as the ESP32,
whoes clock is said to be about accurate to about 10 parts in a million (or 1 in 100,000).
That sounds good, but in practice that means that the clock would be off by about 1 second
per day.

The solution, of course, is to use NTP (Network Time Protocol).

But how that is done is a bit complex to describe.

### Controlling for ESP32 clock changes

The correction algorithm, I call **SYNCING** happens by comparing the number of
ticks to the number of elapsed seconds on the ESP32 clock.

We count the number of beats the clock has ticked versus the seconds that have
passed on the ESP32's clock.   Normally, you'd expect those to be exactly the same,
but even without clock changes, there is some room in the algorithm for edge effects
and rounding error on the millesecond (CPU clock) versus the ESP32 RTC
(real time clock, which measures seconds).

So, in any case, if the number of seconds elapsed on the RTC clock is not the same
as the number of beats the clock has ticked, then it is either fast or slow, and
accordingly, we add (or subtract) 1000 times the error in seconds to the *millis_error*,
and let the clock push (or pull) to speed up (or slow down) for an extended amount
of time (it takes about 3 minutes to speed up or slow down 1 second).

Under the hood this is managed via a separate set of variables so that *millis_error*
remains sacrosanct (as we also use it to determine if the clock is running correctly
or not), and a certain number of "sync_millis" must be made up when a **SYNC**
is taking place.

Without NTP, at most one sync should take place of the instantation of a running clock,
just to eliminated aforementioned edge effects and rounding errors.

We set the clock to check for SYNC every 7200 seconds (two hours)

This can be turned off in the UI.

It might be necessary ... what about when it boots as an AP with no wifi?  That should
work ok, but hullo, if you connect to a wifi, or disconnect, while the clock is running,
I'll bet that can really screw things up ... fortunately the ESP32 keeps the RTC while
disconnecting, but I suspect if you went from Jan 1, 1970, to today's date, and just fed
that to the algorithm, something bad would happen.  Simple solution ... just "restart"
the clock.


### NTP Time

We have carefully verified that, by default, the ESP32's clock is not automatically
synchronized to NTP time during the normal functioning of the clock.

**Requires WIFI and the clock attached as a STATION to your home Wifi network with
access to the internet**

*The myIOT framework, in fact, synchronizes the RTC to NTP when it connects succesfully
as a station to a WiFi network, and that WiFi network is attached to the internet,
so the clock is typically synchronized to NTP upon a boot. **However** I have verified
that I do nothing 'automatically' in myIOT to resynchronize it, so I can actually
measure the ESP32 RTC clock drift versus NTP time*

Every so often (if attached blah blah blah) the clock gets the NTP time.  If it is
different than the RTC time, we note the fact and save the info for debugging,
but all we do at that point it tell the ESP32 to synchronize it's RTC to NTP.

Once that happens, or a short time thereafter, the above SYNC cycle will catch
that the RTC clock is different than the number of beats and enter a SYNC cycle.
