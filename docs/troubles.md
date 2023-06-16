# theClock3 - Trouble Shooting the clock (common problems)

**[Home](readme.md)** --
**[Design](design.md)** --
**[Build](build.md)** --
**[Wood](wood.md)** --
**[Coils](coils.md)** --
**[Electronics](electronics.md)** --
**[Firmware](firmware.md)** --
**[Plastic](plastic.md)** --
**[Assembly](assembly.md)** --
**[Tuning](tuning.md)** --
**[User Manual](user_manual.md)** --
**Trouble Shooting** --
**[Notes](notes.md)**

## Clock runs wonky after about a week or more

If you can feel heat on the box cover, then the ICs on the Printed circuit board
are probably getting really hot.

This indicates that the clock is running continuously at full power and is
unable to drive the pendulum hard enough.  It could be a mechanical and/or
lubrication problem, but it also could be an electrical connection and/or
a design flaw.

If you have checked all the mechanics and lubricated the clock and
it still runs wonky, it is possible, that the problem is with the micro-usb
plug. I have found that unplugging and re-plugging the USB cable seems to
fix this sometimes.

However, I think that **pushing the L293D past it's design specs** may be
causing heat and functional degradation with the clocks over longer time
scales. There is a [note](notes.md) about upgrading this in the future.



**Next:** Various [**Notes**](notes.md) about the clock ...
