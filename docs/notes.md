# theClock3 - General Notes

**[Home](readme.md)** --
**[Design](design.md)** --
**[Build](build.md)** --
**[Wood](wood.md)** --
**[Coils](coils.md)** --
**[Electronics](electronics.md)** --
**[Software](software.md)** --
**[Other](other.md)** --
**[Assembly](assembly.md)** --
**[Tuning](tuning.md)** --
**[User Manual](user_manual.md)** --
**[Trouble Shooting](troubles.md)** --
**Notes**

TODO:

- Readme.md - Clock3 ticking YouTube Video

Disassemble clock 3.33 and the box taking detailed photos while
disassembling, and re-assembling it.  Would like:

- Electronics - elec-CoilConnectors.jpg - photo of current coils glued in box
- Electronics - elec-LEDCable.jpg - photo of current coils glued in box
- Electronics - elec-AS5600Cable.jpg - photo of current coils glued in box
- Electronics - elec-AS5600_cable.jpg - Labelled image of ends of AS5600 cable
- Software - soft-testCoils.jpg - labelled image of coils plugged in

Additional Photos:

- Electronics - better elec-other.jpg parts of cables to be assembled

Incorporate New Video/Pics into pages


- Write Assemble: Arms and Pawls section
- Write Assemble: Top Tube, Pendulum, Cam, and End Cap section
- Write Assemble: Attach Arms and Pawls section
- Write Tuning Page
- Write User Manual Page
- Write Trouble Shooting Page


- Notes - add photos and better desription of 2nd USB option
- Notes - add photos and better desription of USB_UPS option






TODO2:

Build 3 more clocks, producing videos/pics along the way

- Wood - Wood Cutting, sanding, and Frame assembly YouTube video
- Coils - Coil Making YouTube Video
- Electronics - Making PCB YouTube Video
- Other/Assembly - Other and Assembly Build Montage YouTube video

Photos:

- Coils - correct rod orientation in coil winding pics
- Electronics - better pic of coil soldering
- Electronics - pic of coil heat shrink stress relief glue
- Electronics - image of completed LED strip
- Other - photo montage of gluing coils, led lenses, and led strip into the box



## Cam Mechanism and Fusion 360 [pyJoints Addin](https://github.com/phorton1/fusionAddIns-pyJoints)

It took *nearly a month* (and a lot of head scratching) to design the
**cam mechanism**.  I could find no existing examples or such a
mechanism despite many, many internet searches.

It was *very tricky* in Fusion 360 to get all the distances and angles correct (or close).

In order to get some idea of whether or not it would actually work I spent
a lot of time researching **physics simulators** and exploring the **animation
capabilities** of Fusion 360.   For all of it's sophistication, none of the
available tools in Fusion allowed me to animate the mechanism and get an
idea, before I built the clock, whether or not the mechanism would work.

As a result of not being able to otherwise physically model the mechanism,
I ended up spending another *three weeks* or so designing and implementing
an [**Addin**](https://github.com/phorton1/fusionAddIns-pyJoints)
to Fusion 360 to allow me to animate the design and visualize
the actual mechanism in action.

## Ugraded Electronics (pcb3.3 MOSFET vs previous pcb3.2 L293D circuit)

My earliest builds of this clock made use of a circuit that utilized
an L293D h-bridge integrated circuit to drive the coils, inherited from
my [previous clock](https://github.com/phorton1/Arduino-theClock) project.

I believe that the coils were occasionally drawing more current than the
L293D chip was rated for.  I noticed that after a week or more of running
perfectly, the clocks would start failing.  They seemed unable to push the
pendulum far enough.  By that point the PID algorithms had worked their way to
sending full power pulses (255 on a scale of 0 to 255), and yet the pendulum
was not swinging far enough to speed up, or would even come to a complete
stop.

When this happened, I noticed that the L293D chips were very hot.
The two coils were wired separately to the two circuits within the
L293D, and by my calculations the coils could draw upto 0.9 amps
each in that configuration.  The L293D is only rated to 600 ma.

Nonetheless I continued trying to work with the L293D circuit.
I added heat-sinks to the chips, and, after letting them cool down
and rebooting the clock, it would start working again, where full
power pulses would correctly swing the pendulum far enough to bang
against the frame, but after another week, sometimes, the same thing
would happen.

So I decided to redesign the circuit to use a high power MOSFET,
instead of the L293D, to drive the coils.

## Secondary USB Connector

TODO: Describe 2nd USB option

## Backup USB UPS (Uninteruptable Power Supply)

TODO: Describe UPS option and build of USB UPS

The choice of **batteries** for the UPS makes a **big** difference!

I tested it first with some *cheap yellow* chinese 18650 batteries
and it was only able to power the clock for about **30-45 minutes**
before the batteries dropped below 2.9 and the ESP32 would no longer
boot.

Then I switched to some new *high quality* **LG** batteries, and
it was able to power the clock for over **8 hours**.



-----------------------------------------------

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






**Done!!:** Back to the [**beginning**](readme.md) ...
