# theClock3 - Trouble Shooting the clock (common problems)

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
**[Software](software.md)** --
**Troubles** --
**[Notes](notes.md)**

This page describes some **problems** that might be encountered
with the clock, and some possible **solutions** to those problems.

As we have seen, when taken as a whole, the clock is a fairly *complex*
device, and can have a wide variety of problems, ranging from
[catastrophic](#a-catastrophic-problems)
failures to function, to
[mechanical issues](#b-mechanical-issues)
or a lack of
[lubrication](#c-lubrication)
that cause the clock to not tick and tock well, to subtle
[tuning issues](#d-tuning-issues)
that cause the clock to tick and tock, but not keep
accurate time.

If, after trying to *analyze* the problem and apply the *solutions*
listed here, you still cannot get the clock to perform reasonably,
please feel free to **contact me**.


## A. Catastrophic Problems

- *Nothing Happens*, and **no LEDS** light up after plugging clock into USB Power Supply
	- bad power supply - try a different **USB power supply**
	- broken USB connectors - replace the **ESP32** or try the **2nd USB connector**
	- ESP32 burnt out - replace the **ESP32**
	- bad LED cable - replace the **LED strip** and **cable**
	- LEDS burnt out - replace the **LED strip** and **cable**
	- bad PCB -	test for **short circuits** and/or replace the **PCB**
- All LEDs flashing <font color='red'><b>red</b></font> during boot and
  after booting the **2nd LED** continues flashing <font color='red'><b>red</b></font>
	- bad Angle Sensor Cable - replace the **cable**
	- bad AS5600 Angle Sensor Module - replace the **module**
	- bad PCB - test for **short circuits** and/or replace the **PCB**
- After booting the **2nd LED** continues flashing <font color='orange'><b>orange</b></font>
  indicating a failure to set the **Zero Angle** for the clock
	- set the *Zero Angle* using the [Buttons](ui.md#a-buttons)
	- set the *Zero Angle* using the [WebUI](ui.md#d-webui)
	- set the *Zero Angle* uisng the [Serial](tuning.md#b-set-zero-angle) or [Telnet](ui.md#e-telnet-serial-monitor) Monitors
- The Pendulum does **not** start **Swinging** after attempting to start the clock
    - check for, and resolve any *Mechanical* issues
	- try re-setting the *Zero Angle* using one of the above methods
	- increase the POWER_START parameter
	- increase or decrease the DUR_START parameter

## B. Mechanical Issues

In general, the **Gears, Cam, Pawls, and Arms** must be aligned, and
there must be a bit of *play* in the entire mechanism, including the
**Top Tube** and **Gear Train**, and everything must move *freely*
with no binding.

- Make sure everything is *aligned*,
  - that the **Pawls** are *riding* on the **Seconds Wheel**
  - that the *bearings* on the **Arms** are *riding* on the **Cam**
  - that the **Gears** are all engaging
- Make sure there is a bit of *play* in the **Top Tube** and **Gear Mechanism**
- Make sure that the **Arms and Pawls** are *free* to *swing*
  - *loosen* (or tighten as necessary) the *screws* holding the
    **Pawls** and **Arms**
- Make sure the **Pendulum** is swinging *freely* in the **Channel**
  - adjust the Pendulum on the **Top Tube** so that it is in the
    center of the Channel and does not **touch anything** as it swings

## C. Lubrication

It may be necessary to occasionallty **Lubricate** the *bearings* and *tubes*
to ensure they turn freely.

- remove the **Hands** from the clock
- lay the clock on it's *back*
- apply a *small* drop of *light machine oil* to the *Seconds and Minutes* **tubes** so that it will run down into the tubes
  - turn the *Gear Train* by hand to distribute the oil
- apply *small* drops of *light machine oil* to the various *bearings* on the clock
- replace the **Hands**

I have found that a small squire of **WD40** can also help with Lubrication, but
be advised that *WD40 is **not** a lubricant* and will actually *remove* the
lubrication from the clock.  A small amount, however, will *dilute* the existing
oil and perhaps make the clock run better.

In *extreme cases*, it may be necessary to **disassemble** the clock to
clean and lubricate the *tubes* and *bearings* more fully.

- remove the **Hands** and the front **Hours Spacer**
- remove the two screws and remove the **Face**
- remove the six screws and remove the **Bridge**
- *carefully* remove the **Gears** while keeping track
  of any *washers* or *spacers*
- clean the **insides** and outsides of the *tubes* with a **paper towel**
  on a toothpick, or even *lightly sand* the **insides** of the tubes
- apply fresh **light machine oil** to all bearings and tubes
- **reassemble** the clock.

I have no idea how long the clock will last, what it's **lifetime** will be.
However, I recommend that, at least *once per year* the clock is **lubricated**, and
that at least once *every five years* the clock is disassembled and a **deep clean**
and lubrication are performed.

## D. Tuning Issues

It extreme cases, it may be neccessary to do a complete [**Tuning Cycle**](tuning.md)
of the clock.  This mostly amounts to the following:

- adjust the **Pendulum Weight** so that the clock beats slightly slower than 1000ms per swing
  at the *minimum target angle* (ANGLE_MIN)
- adjust the **Magnetic Spring** so that the clock beats slightly quicker than 1000ms per swing
  at the *maximum target angle* (ANGLE_MAX)

However, *even then* there may be cases where the clock does not function correctly,
and cannot sufficiently **speed up** or **slow down** the Pendulum.  These cases
are indicated by the following **LED symptoms**.

- the *rightmost LED* is <font color='blue'><b>flashing blue</b></font> indicating that the clock
  is runniing *5 or more seconds* **slow**
- the *rightmost LED* is <font color='red'><b>flashing red</b></font> indicating that the clock
  is runniing *5 or more seconds* **fast**
- the *rightmost LED* remains <font color='blue'><b>blue</b></font> for more than a few minutes,
  indicating that the clock got stuck in a *synchronization cycle* and could not **speed up**
  sufficiently to finish the cycle
- the *rightmost LED* remains <font color='red'><b>red</b></font> for more than a few minutes,
  indicating that the clock got stuck in a *synchronization cycle* and could not **slow down**
  sufficiently to finish the cycle
- the *3rd or 4th LEDS** are consistently turning **solid**
  <font color='red'><b>red</b></font> or <font color='blue'><b>blue</b></font>
  indicating that the *individual cycles* are more than 50ms fast or slow,
  or that the *overall clock accuracy* is more than 150ms fast or slow

If, after a complete *Tuning Cycle* using the *factory reset* **default values**
for the (many) *parameters* on the clock, you cannot get it to keep good time, then
it *may* be necessary to **alter** various parameters to adjust for the idiosyncracies
of your particular clock.  You can try *changing* some of the following parameters:

- DUR_PULSE and DEAD_ZONE - affect how *long* and *when* pulses are delivered
  to the pendulum.  Increasing the DEAD_ZONE can help if the Pendulum is
  *jerky* in it's behavior; decreasing it will impart *more energy* to the
  Pendulum as the pulses will take place while the magnet is closer to the coils.
  Increasing or decreasing DUR_PULSE will cause more, or less
  energy to be transfered from the coils
  to the Pendulum, causing it, in turn, to swing more or less widely.
- POWER_MIN and POWER_MAX - adjusting the minimum and maximum power to higher or lower values
  will alter the speed with which the algorithms respond to changes in the *angular error*
  of the clock.  Try *various values* and observe how the clock *behavior* changes.
- ANGLE_MIN and ANGLE_MAX - will cause the Pendulum to swing more, or less widely,
  working in conjunction with the *magnetic spring*, to speed up, or slow down, the clock.
  *Small changes* of 1/10 of a degree in these parameters can have significant effects.
- ANGLE_START - can be adjusted to help the clock more quickly achieve a *stable
  angle* and change from CLOCK_STATE_STARTING to CLOCK_STATE_RUNNING
- PID and APID parameters - changing the **PID Controller** weights has a
  profound effect on the clock.  The *default values* were arrived at after
  considerable testing.  It is beyond the scope of this document to describe
  *how* to change these values, but changing them **can and will** result in different
  behavior and may solve problems with the clock.

In all cases, it is **highly recommended**, when modifying *any parameters* to use the
[Serial](tuning.md#b-set-zero-angle) or [Telnet](ui.md#e-telnet-serial-monitor) monitors
to see the **timing lines** so that you can watch **each beat** as you experiment.

Also, **once you achieve a new good set of parameters** it is a **good idea** to
write down the changes from the defaults as they will be **lost** if you
subsequently perform a **Factory Reset**.

Once again, if you have tried some or all of the solutions to the problems
listed on this page, and still cannot get your clock to keep *accurate time*,
please feel free to **contact me**.



**Next:** (and finally), various [**Notes**](notes.md) about the clock ...
