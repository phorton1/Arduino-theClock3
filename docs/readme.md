The pendulum has a magnet in it.  There is a coil in the box that can
**attract** (*pull*) the pendulum towards the center, or **repulse** (*push*)
it away.

The clock works by using a *PID controller* to target the ANGLE by using
"pushes" after crossing zero, or "pulls" after turning around at the extremes.

Pushes tend to slow the clock down a bit, and pulls tend to speed it up,
and we try to keep the angle consistent.


## Why pushes and pulls

To understand why pushes slow the clock down and pulls speed it up, one perpective
is that it is a bit like as if we are changing gravity for the pendulum,
making it lighter when pushing, or heavier when pulling.

From the basic formulas for a pendulum, increasing the gravity will decrease
the period, causing it to swing faster, or vice versa, decreasing the gravitational
component will cause the pendulum to swing slower,

Using the electromagnet, when we pull on it, it is similar to increasing the gravitational
field the pendulum is moving through, as there is more force acting on to return it to the
center.  So it swings a bit quicker.

Likewise, when we push on it, we are making it a bit "lighter" with respect to earth's
gravity. It is not in as much of a hurry to get back to center (while the magnet is on),
so the cycle is slightly lengthened and the pendulum swings a bit slower.

It's a fairly fine balance, and the pendulum, and paraemters of the program, must
be tuned fairly carefully in order to work reliably.


## Initial Tuning

ZERO_ANGLE must be set before anything else. And the coils (motor) direction
and basic strength can verified using the TEST_MOTOR (-1,0, or 1) UI.

Assuming the basic sensor and coil are working:

(1) The first order of business is to tune the clock so that it basically ticks
somewhere in a valid range.  This is done by turning PID_MODE off and setting
the "pull_mode" to PUSH_ONLY and then starting the clock.

This combination will cause a push of POWER_MIN on every push, and the clock
should be set so that POWER_MIN barely keeps it running (POWER_MAX is generally
left at 255).

*in other words, you can run the clock with the pid turned off and it will
just put constant impulses into the pendulum on each swing so that you can
get it basically working*

Of course the ZERO_ANGLE must be set, and an appropriate DEAD_ZONE must exist,
Other possible parameters at this stage include DUR_LEFT and DUR_RIGHT to balance
out the pendulum.  I found a dead zone of 0.7 degrees and durations of 140ms to
work in practice.

(2) Then the PID controller is turned on and tuned so that pushing homes in on an angle.
An appropriate operating angle (i.e. 9 degrees) is determined and set into TARGET_ANGLE.
Turn on PID_MODE and diddle	with the PID_D, PID_I, and PID_P, values as needed.

(3) Once the clock reliabily ticks to a given angle, then we need to adjust the lead weight
so that the clock is running a little bit slow when pushing (i.e. typically 5-10ms slow per beat).

(4) Can then try PULL_ONLY mode with PID_MODE off.  As with PUSH_ONLY, you can tune
PULL_POWER to ensure that the pendulum swings.  Then turn PID_MODE on PULL_ONLY should result
in the pendulum homing in on the target angle, with average cycles that are a little shorter
than one second.

Those are the basics.  We tune the clock so that it reliabily beats at a certain angle,
and so that if we **pull** it, it goes a little *faster* than 1 beat per second, and if we
**push** it, it goes a little *slower*.

**NOTE: THIS HAS TO BE DOABLE FROM THE BUTTONS AND LEDS**

Tuning the pid controller is beyond the capability of end-users with the buttons and leds.


## PID Controller

The PID controller that controls the angle of the pendulum is pretty simple.

We measure the angle of the pendulum about every 3 milliseconds. We use a
minimum movement threshold to avoid hysterisis (about 1/10 of a degree) and
only take new values when the pendulum has moved significantly.

The pushes start just after each **zero crossing** as determined by the *DEAD_ZONE*,
and go on for the parameterized DUR_LEFT and DUR_RIGHT millisecond durations.
Pulls, on the other hand, start at **the extreme** of the pendulum swing (or
just after the extreme, as the pendulum starts to fall back towards the center),
and the electromagnetic pulse from pulls last until the pendulum is (two times)
the *DEAD_ZONE* away from zero.

Otherwise, the two impulses use the same PID controller, and we just swtich
between pushing and pulling as described below in the next section.

Because we are trying to control two numbers with one PID controller ...
the pendulum extreme angle to both the left and the right ... we sum
and average the two and try to get the controller to make the AVERAGE OF
THE LEFT AND RIGHT SWINGS equal to the parameterized TARGET_ANGLE.

On each impulse the PID controller adds and averages the most recent maxium
left and right angles into the **average angle**, and compares that to
the TARGET_ANGLE, to get an *angle_error*.   The *angle_error* is the
first (Proportional) input to the PID controller.  We keep a running
sum *total_error* of the accumulated angle errors which is used as the
second (Integral) input to the PID controller.   We use a simple delta
between the previous angle error and the current angle error as the
third (Derivative) input to the PID controller.

The PID controller spits out the power (0.225) that we should apply
to the coil and the (pull or push) impulse is delivered for the duration
as described above.

Without changing from push to pull, the PID controller regularly
achieves +/- 0.1 degree control.  With the changes from push to
pull, the range is more like +/- 1 full degree.  So with a TARGET_ANGLE
of 9 degrees, we see the pendulum swinging from 8-10 degrees on each
side, or 16-20 degrees of total arc.


## Algorithm - When to push or pull?

We keep a lot of statistics about the clock, but one of the most important is the
**cumulative millisecond error** ... called **millis_error**, which is running sum
of the errors on each clock cyle.

When the clock ticks faster than 1 second, cycle times will measure like 992, 979, and 998,
and when it clicks slower than 1 second, you get get values like 1003, 1021, 1013 ms.

We subtract 1000 from the cycle time to get the *instantaneous error* which is
positive if the clock is running slow, or negative if the clock is running fast,
and add it to the running total **millis_error** ... which is the most basic measure
of how far off the clock is from the correct time.

Very simply, we switch from pushing to pulling or vice-versa when the *millies_error*
exceeds some threshold.   I started by just changing based on the sign of the millis
error, but that caused the pendulum to swing widely, and I determined it was best
to let it swing with "pushes" for a while, then with "pulls" for a while, as a natural
way to smooth out the overall movement of the pendulum.

So the threshold is currently set to 40ms.  When the cumulative error exceeds +/-
40 ms, we switch from pushing to pulling.   It takes 20-40 or so cycles for the
change to accumulate enough in the other direction to perform another switch.

Overall, this gives pretty smooth behavior and (unlike with clock #1) you don't
really perceive the change in the speed of the pendulum.   For a human it seems
to beat pretty regularly, with a pretty constant amplitude.



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
