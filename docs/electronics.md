# theClock3 - Electronics Schematics and PCB (Build Continued)

**[Home](readme.md)** --
**[Design](design.md)** --
**[Build](build.md)** --
**[Wood](wood.md)** --
**[Coils](coils.md)** --
**Electronics** --
**[Firmware](firmware.md)** --
**[Plastic](plastic.md)** --
**[Assembly](assembly.md)** --
**[Tuning](tuning.md)** --
**[User Manual](user_manual.md)** --
**[Trouble Shooting](trouble.md)** --
**[Notes](notes.md)**


The **L293D** integrated circuit is rated at **600ma** from 4 to 36 volts,
so I am **pushing the chip past it's design specs**, sigh. However,
it is worth noting that the typical parameterized driving impulse
is only **120 ms** in duration, so the **overal duty cycle** of the circuit
is about only 10%.  and the **PWM output** from the ESP32 typically
runs at about 50%, so (for the time being) I am using this circuit.
There is a [note](notes.md) about upgrading this in the future.

### PCB (Printed Circuit Board) and Fusion 360

It is worth noting that I have modified my **KiCAD** environment a
bit, generating custom *symbols* and *footprints* with *3D components*
so that when I design the PCB, I can also export a *3D Model* (**STEP file**)
of it from KiCad.

I used to have to keep track of, and manually redraw, all of the dimensions
of the PCB in Fusion 360, but now, with the STEP file from KiCad,
I am then able to import that 3D model *directly into my Fusion 360 design*
which makes it **much easier** to do things like creating screw holes for
the PCB mount and things like locating the cutouts in the box for the
USB connector.


**Next:** Installing the [**Firmware**](firmware.md) and testing the PCB ...
