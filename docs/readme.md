The algorithm works by using a PID controller to target the ANGLE by using
"pushes" after crossing zero, or "pulls" after turning around at the extremes.
Pushes tend to slow the clock down a bit, and pulls tend to speed it up.

(1) The first order of business is to tune the clock so that it basically ticks
    somewhere in a valid range.  This is done by turning PID_MODE off and setting
    the "pull_mode" to PUSH_ONLY and then starting the clock.

	This combination will cause a push of POWER_MIN on every push, and the clock
    should be set so that POWER_MIN barely keeps it running (POWER_MAX is generally
    left at 255).

    Of course the ZERO_ANGLE must be set, and an appropriate DEAD_ZONE must exist,
    Other possible parameters at this stage include DUR_LEFT and DUR_RIGHT to balance
    out the pendulum.  I found a dead zone of 0.7 degrees and durations of 120ms to
    work in practice.

(2) Then the PID controller is turned on and tuned so that pushing homes in on an angle.
	Turn on PID_MODE and diddle	with the values as needed.
    An appropriate operating angle (i.e. 9 degrees) is determined and set.

(3) Once the clock reliabily ticks to a given angle, then we need to adjust the lead weight
	so that the clock is running a little bit slow when pushing (i.e. typically 5-10ms slow per beat).

(4) Can then try "pull mode" to see that it also ticks regularly at the correct angle.
    Pull_mode should result in average cycles that are a little shorter than one second.
	There is alsoa PULL_POWER that can be adjusted.  Ahem.  It should be adjusted, without
	PID_MODE, so that the pendulum swings wide, but not past design considerations.




