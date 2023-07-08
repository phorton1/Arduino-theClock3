# theClock3 - Accurate Electromagnetic Wooden Geared Clock

**Home** --
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
**[Troubles](troubles.md)** --
**[Notes](notes.md)**

This clock uses wooden gears and a simple electromagnetically driven pendulum
with the goal of keeping accurate time.

[![YoutubeThumb_Ticking.jpg](images/YoutubeThumb_Ticking.jpg)](https://youtu.be/lsMbbkOouPc)


It uses an an **electromagnet** and **permanent magnet** to provide pulses of energy
to the pendulum to keep it moving, an **angle sensor** to detect the position
of the pendulum, and a **magnetic spring** to to speed up, or slow down, the pendulum.

It uses a specially designed **cam driven pawl** mechanism to
reliably convert the motion of the pendulum in each direction
to the rotation of the **seconds wheel** and **gear train**.

It has a computer (an [**ESP32**](https://en.wikipedia.org/wiki/ESP32)) in it,
with a *Real Time Clock* (**RTC**) that is fairly accurate (to within about 1 second per day).
Furthermore the computer can connect to the internet to use Network Time Protocol
([**NTP**](https://en.wikipedia.org/wiki/Network_Time_Protocol))
to synchronize it's RTC within a few milliseconds of the *correct* international
**standard time**.

Roughly stated, the goal of this project is to produce a **wooden geared clock**
that will be accurate to within one second per day, or, when connected to the
internet, within one second at any given time over it's working lifetime.

This documentation is mostly geared towards makers who might like to build
one of these clocks. Though it is not a trivial task, this repository should
contain all of the information needed for you to build one.

## Documentation Outline

If you have received one of these clocks as a gift, please see the
**[Quick Start Guide](QuickStartGuide.pdf)** and the
**[User Interface](ui.md)** page for information on how to unpack,
setup, start, and use your clock.

The following pages of documentation are intended to as a *tutorial* for
a **maker** who is *building* a clock, and describes the step-by-step
process of constructing the clock and getting it running:

- **[Design](design.md)** - An overview of the **requirements** and **design** of the clock
- **[Plan](plan.md)** - A *plan* for **building** the clock, and the **BOM** (Bill of Materials)
- **[Wood](wood.md)** - **Laser Cutting** the wood and building the *Frame*, *Face*, and gluing the *Gears* together
- **[Coils](coils.md)** - **Winding** the *electromagnet coils*
- **[Electronics](electronics.md)** - Creating the custom **PCB** (printed circuit board), **Cables**, and **Connectors**
- **[Firmware](firmware.md)** - Compiling and installing the **Firmware** and doing a  **hardware test** of the electronics
- **[Assemble](assemble.md)** - Build the **sub-assemblies** of the clock, including **cutting the tubes** and assembling the **Gears, Pendulum,** and **Box**
- **[Build](build.md)** - **Final Build** of the clock, bringing all the *sub-assemblies* together into the finished *Clock*
- **[Tuning](tuning.md)** - Describes the process of getting the clock to **run accurately**

The remaining pages are considered **reference** materials for more
deeply understanding the clock and it's software:

- **[User Interface](ui.md)** - A detailed guide to the **User Interface** of the clock, including the *Buttons and LEDs*, and connecting to *WiFi* to use the *WebUI* and *Telnet*
- **[Software](software.md)** - A deeper look the **Software** of the clock, including an *Overview* of the functionality and a description of all of the *Parameters* for the clock
- **[Trouble Shooting](troubles.md)** - Some common potential **problems** and possible **solutions**
- **[Notes](notes.md)** - Other Notes and information about the clock


## Files Available in this repository

This clock was *designed* using the **Fusion 360** CAD program and
the [**KiCAD**](https://www.kicad.org/) schematic and PCB design program.
The main outputs from Fusion 360 are the **DXF Files** (drawings) used to laser
cut the wood, and the **STL Files** (3D models) used to 3D print the plastic parts.

The clock was *implemented* by using the **Lightburn** laser CNC machine
control program to cut the wood from the Fusion DXF files using my
[**20mm CNC machine**](https://github.com/phorton1/Arduino-esp32_cnc20mm),
and the **Prusa Slicer** and **OctoPrint** to slice the Fusion STL files and send
them to my **Prusa MK3S 3d printer** to make the plastic parts.

The PCB (printed circuit board) is almost it's own sub-project.
The outputs from *KiCad* include the basic **schematic**, and
the **Gerber** and **Excellon** files used to make the PCB.
The program [**FlatCAM**](https://bitbucket.org/jpcgt/flatcam/downloads/) \
was used to generate the GCODE files from those files for running my
[**cnc3018**](https://github.com/phorton1/Arduino-esp32_cnc3018)
to **mill, epoxy coat, drill, and cut out** the PCB (printed circuit board.

The following sub-folders and their children can be found within this 'docs' folder

- [**fusion**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/fusion) -
   contains the main *Fusion 360* design **clock3.2.f3z**, as well as a few
   other files that support the design, including a CSV file containing parameters for
   the design, and a couple of **pyjoints** animations (which make use of the
   Fusion 360 [pyJoints Animation Plugin](https://github.com/phorton1/fusionAddIns-pyJoints)
   I wrote) that allowed me to visually test the design before implementing it.
  - [**dxf**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/fusion/dxf) -
    contains all of the DXF files used with **Lightburn** to laser cut the wooden parts
  - [**stl**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/fusion/stl) -
    contains STL files of all the plastic parts that I 3D printed
- [**kicad/pcb3.1**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/kicad/pcb3.1) -
  contains the **kiCAD** project, electronics schematics, and PCB design files
  - [**plot**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/kicad/pcb3.1/plot) -
    contains the Gerber/Excelon files output from kiCAD, along with the
    **FlatCAM** project used to produce the gcode files for milling, epoxy coating,
	drilling, and cutting out the PCB
    - [**gcode**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/kicad/pcb3.1/gcode) -
	contains the final output gcode files for the PCB
- [**lightburn**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/lightburn) -
   contains **LBRN2** Lightburn files that contain all of
   the wooden parts to be laser cut, as well as individual files to cut
   the different thicknesses or types of wood
- [**prusa**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/prusa) -
   contains the Prusa **3mf** projects for the plastic parts I sliced
  - [**gcode**](https://github.com/phorton1/Arduino-theClock3/tree/master/docs/prusa/gcode) -
    contains the final output *Prusa MK3s* gcode files that I used to
    3D print the plastic parts


If you are a **maker** you do not necessarily need to mill your own PCB as I did.
You can implement this simple circuit using **perfboard**, or can use the
kicad files to (fairly quickly) design your own PCB to be created
from your favorite PCB supplier.   I simply choose to mill my own PCB's
because I don't live in a good place for shipments and am too impatient
to wait weeks (or months) to get a PCB.  You might be able to get one
from China in a few days, but it takes much longer for me where I live.

Another example is that you don't **need** to laser cut the wooden parts.
It should be possible to use a small CNC machine (3018) to mill the gears, and
either a slightly larger CNC machine to mill the frame parts, or they could
even be cut using templates and a jigsaw or scroll saw.

**HOW YOU BUILD IT IS UP TO YOU !!**

The reason I provide so much information in these pages (rather than just the bare minimum)
is specifically to allow some flexibility for your design/build workflow.
When I went to make my first wooden (and especially laser cut) clock, I could not find
any complete designs that I could build.  So I am hoping that the info here will
provide you both with the materials you need, and perhaps the impetus, to build
one of your own!


## Credits

This project directly relies upon the public open source Arduino development system,
the Espressif ESP32 online documentation and additions to the Arduino IDE, as
well as a number open source Arduino libraries

I would like to thank the countless individuals who have contributed to making these
development tools, source codes, and documentation available for free public use, including,
but not limited to:

- [KiCad](https://www.kicad.org/)
- [FlatCAM](https://bitbucket.org/jpcgt/flatcam/downloads/) by **Juan Pablo Caram**
- [Arduino IDE and Libraries](https://www.arduino.cc/)
- [Espressif Systems](https://www.espressif.com/en/products/socs/esp32)
- [Adafruit NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel)
- [AS5600 Library](https://github.com/RobTillaart/AS5600) by **Rob Tillaart**



## License

This program, project, and repository is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License Version 3 as published by
the Free Software Foundation.

These materials are distributed in the hope that they will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR ANY PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Please see [LICENSE.TXT](../LICENSE.TXT) for more information.


## Please Also See

This project directly or indirectly brings together a number of other projects of mine.
It supersedes my previous clock projects: [**theClocks 1&2**](https://github.com/phorton1/Arduino-theClock)
and makes direct use of my
[**MyIOT**](https://github.com/phorton1/Arduino-libraries-myIOT)
("my internet of things*) repository, which provides a generalized approach to building parameter
driven ESP32 devices that include a Web UI as the basis for the software.

Another project that also make user of the *myIOT* repository is the
[**Bilge Alarm**](https://github.com/phorton1/Arduino-bilgeAlarm)
that I built for my boat.

The PCB (printed circuit board) was **milled** using the
[**cnc3018**](https://github.com/phorton1/Arduino-esp32_cnc3018) CNC
machine that I built, and the wooden parts were cut using the laser configuration of the larger
[**20mm CNC machine**](https://github.com/phorton1/Arduino-esp32_cnc20mm) that I subsequently built.
Both of those projects depend on my port of the
[**FluidNC**](https://github.com/phorton1/Arduino-libraries-FluidNC)
ESP32 based CNC controller firmware, and include my
[**FluidNC UI**](https://github.com/phorton1/Arduino-libraries-FluidNC_UI)
(touchscreen user interface) and
[**other extensions**](https://github.com/phorton1/Arduino-libraries-FluidNC_Extensions)
that I added to the FluidNC library.

There is also a quick and dirty
[**Coil Winding Machine**](https://github.com/phorton1/Arduino-CoilWindingMachine)
project repository for the machine I made to wind the coils for these clocks and
a [Fusion 360 Plugin](https://github.com/phorton1/fusionAddIns-pyJoints)
thatI created which allowed me to do animations of the clock to visually see how the
mechanism might work before actually building it.

I hope you will take a few minutes to check out some of these other pages too!!




**Next:** A [**Design Overview**](design.md) of the clock ...
