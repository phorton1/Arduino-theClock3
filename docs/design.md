# theClock3 - Design Overview

**[Home](readme.md)** --
**Design** --
**[Build](build.md)** --
**[Wood](wood.md)** --
**[Coils](coils.md)** --
**[Electronics](electronics.md)** --
**[Firmware](firmware.md)** --
**[Plastic](plastic.md)** --
**[Assembly](assembly.md)** --
**[Tuning](tuning.md)** --
**[User Manual](user_manual.md)** --
**[Trouble Shooting](trouble.md)** --
**[Notes](notes.md)**

I designed and built **theClock3** after [**clocks #1 and #2**](https://github.com/phorton1/Arduino-theClock).
This design includes the requirements listed there and makes use of **lessons learned** in the process
of building those earlier clocks.  Some of the most important changes from those previous clocks include:

- It uses a **cam driven mechanism** to move the pawls within prescribed limits to eliminate *double grabs* and *work more reliably*
- The *cam driven mechanism* moves the pawls so that the pendulum does *work* in **both directions** as it swings
- It uses **smaller gears** to lessen the *mass*, *friction* and amount of *work* the clock must do.
- It uses an **angle sensor** instead of *hall sensors* for more accuracy in controlling the pendulum.
- It uses a **box-joint assembly** rather than *wooden dowls* to provide a more rigid and accurate frame.
- More care was taken to **balance** the *center of mass* of the **hands** so that they don't add extra *work* for the clock as they spin
- It has an **adjustable pendulum** weight for much more flexibility in *tuning* the clock
- The **magnetic spring** has been separated from the *main driving magnet* to prevent *twisting moments* on the *pendulum*


## Basics

There is an **angle sensor** on the pendulum.  By comparing successive
measurments we can determine the pendulum's *direction*, when it *crosses zero*,
and the extreme *maximum angle* it achieves during any given swing.

The pendulum has a **magnet** in it that passes between a pair of **electromagnetic coils**
on each swing.   The coils *repulse* (push) the pendulum a little on each swing,
providing the energy to keep it moving.  By increasing the power provided to the coil
we can make the pendulum swing further, and by decreasing it, we can generally make it swing
less widely.

The **cam mechanism** has a **minimum design angle** of **5 degrees** (about the center,
or 10 degrees overall).  It is designed such that if the pendulum swings at least this far,
the *pawls* will properly engage the *seconds wheel* and advance the gear train.
Furthermore the mechanism is designed such that, beyond this distance, the pendulum
does no further work so that it is impossible for the pawls to grab
more than one tooth at a time or advance the clock more than one second per swing,
regardless of how far the pendulum swings.

There is another **pair of magnets**, one in the stem of the pendulum, and one
that is affixed to the clock, that is adjustable, that together, via magnetic
repulsion, act as a **magnetic spring**. Because of this spring, the pendulum
swings *faster* when it swings sufficently far to **bounce** off of this spring,
and it swings *slower* when it swings less and the spring does not come into play.

We establish a **working minumum angle** at which the clock functions, by which we
mean that it actually *ticks* and *tocks* reliably. For prudence we set this to a
few degrees wider than the *minimum design angle* at approximately **8 degrees**
about center, or 16 degrees overall.

By design, the pendulum can swing about 12.5 degrees about center or 25 degrees
overall before it *bangs* up against the frame.  We define a **working maximum angle
of 11 degrees** about center or 22 degrees overall where the clock will not bang
on the frame.

We **adjust the weight** on the pendulum so that the pendulum swings a little
*slower* than one second (about 1010 milliseconds) at the minimum angle.
We then **adjust the spring** so that it swings a little *faster* than one
second (about 990 ms) when it swings at the maximum angle.

The clock (code) works by using a **primary PID controller** to swing at a
**given ANGLE** by increasing or decreasing the amount of power fed to the
coil during each swing.  We call this angle the **target angle**, and when working
correctly, the clock will reliably swing within a few tenths of a degree
of the given target angle.

We then use a **second PID controller** to adjust the **target angle** to minimize
the clock's *error* (in milliseconds).  On each swing, there is an instantaneous
error, which is how many ms faster, or slower, than 1000ms (one second) a particular
swing took.  These errors can accumulate, causing the clock to run slower,
or faster, overall, than the correct time.  If the clock is running slow, we
gently increase the target angle towards the **maximum working angle** so that the
pendulum starts to be influenced by the magnetic spring, and speeds up.
Or if it is running fast, we gently decrease the target angle so that it beats
slower, but still always swings at least at the **minimum working angle**.

This two stage PID controller was an evolution.
On [earlier clocks](https://github.com/phorton1/Arduino-theClock)
I merely tried to use a single PID controller to directly minimize the
instantaneous and cumulative ms errors by changing the power delivered
to the pendulum, but I found that it was better, both mechanically, and aesthetically,
to try to get the clock to first swing at a relatively constant angle, and THEN
to adjust that angle subtly to correct the time.
The single PID controller approach resulted in much more noticable changes in the
speed and swing of the pendulum, and adding the second PID controller significantly
smoothed out the behavior of the clock.


## Synchronization

The swing error is based on subsequent calls to the millis() function, rather
than comparing the time directly to the RTC clock.  This means that the algorithm
itself can drift from RTC time.  We allow this to happen and provide a separate
synchronization method, **onSyncRTC()**, to occasionally correct for this potential drift.
This allows us to keep track of the drift between the algorithm and the RTC and
isolates the basic swing PID controllers from changes in the RTC.

Finally, if connected to the internet, we occasionally synchronize the RTC
to **NTP** (Network Time Protocol) to correct for the ESP32 clock drift
via another method **onSyncNTP()**.


## [myIOT framework](https://github.com/phorton1/Arduino-libraries-myIOT) and parameterization

The clock makes use of a library that I created, called
[myIOT](https://github.com/phorton1/Arduino-libraries-myIOT) for
creating parameterized devices with ESP32's.   By using this library
the clock presents a **User Interface** over the **Serial Port** and/or
a browser based **WebUI** and/or **Telnet** UI via **Wi-Fi** that
lets you control and monitor the clock.

Much of the **behavior** of the clock can be modified using these UI's.
For instance, the **minimum and maximum working angle** and the
**duration of the pulses** are parameterized and can be modified
at run-time in order to better [Tune](tuning.md) the clock.

Likewise, the **PID controller values** for both PID controllers are also
parameterized. as well as things like **the interval for syncing
the RTC and NTP clocks**.  In fact the firmware can even be **re-compiled
and uploaded** to the ESP32 via Wifi with **OTA** (Over the Air) updates.

Please see the **[User Manual](user_manual.md)** for more details on
the capabilites of the software and the various user interfaces.



**Next:** An overview of the [**Build Process**](build.md) for the clock ...
