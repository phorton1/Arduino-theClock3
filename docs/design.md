# The Clock - Design and Build Overview

**[Home](readme.md)** --
**Design** --
**[Wood](wood.md)** --
**[Plastic](plastic.md)** --
**[Electronics](electronics.md)** --
**[Coils](coils.md)** --
**[Assembly](assembly.md)** --
**[Tuning](tuning.md)** --
**[User Manual](user_manual.md)** --
**[Trouble Shooting](trouble.md)** --
**[Notes](notes.md)**

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

**TODO: ANIMATED GIF OF FUSION CLOCK TICKING**

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


**TODO: Link to TheClock3 - Design Overview YouTube Video**


## Build Overview

**TODO:  Insert 3D build explosion animated gif here**

I visualize the process of creating the clock as the following series of major steps:

- Laser cut (and prep/sand) the **[Wood](wood.md)** parts
- 3D print (and cleanup) the **[Plastic](plastic.md)** parts
- Create the **[PCB](electronics.md)** (printed circuit board)
- Wind the electromagnet **[Coils](coils.md)**
- Solder/Create cables connectors for the LEDs, Coils, and AS5600 angle sensor
- [Assemble](assembly.md) the Frame
- [Assemble](assembly.md) the Gear Train
- [Assemble](assembly.md) the Clock itself
- Burn the firmware to the ESP32 and **[test](tuning.md)** the circuit board, LEDs, Coils, and Angle Sensor
- Setup and **[Tune](tuning.md)** the clock
- Test and [Adjust](trouble.md) the physical mechanism and software until satisfied.


**TODO: Link to TheClock3 - Basic Assembly YouTube Video**


## Bill of Materials

The clock is made mostly of 3/16" and 1/8" plywood which I measured as 4.8mm and 3.175mm thick, respectively.
You can probably swap 5mm plywood for 3/16" with no design changes.

The main **metal** components are 4mm, 5mm, and 6mm **telescoping brass tubes**, a small variety
of metal **ball bearings** (two 3x9x4mm, one 4x11x4mm, two 5x16x5mm, and one 6x15x5mm), a length
of **3mm threaded rod**, three small **lead fishing weights**, and a variety of 2mm and 3mm
wood and machine **screws, nuts, and bolts**.   There is an **M3x45mm bolt** in my implementation
which could be done with a section of 3mm threaded rod instead.

The main electronic components include an **ESP32 Dev Board**, an **AS5600** magnetic angle sensor
module, and a **L293D** H Bridge Integrated Circuit, a short length of a **WS2812b** LED Strip,
along with various connectors, buttons, and cables.  I also separately cost a good quality 2A
5V micro-usb power supply at about $7.00.

I estimate the cost of the materials to be about $80 to produce this clock as I have designed it.
Of course, the actual cost will depend on what parts you already have on hand, and how you obtain
the parts.   For example, the tiny 3x9x4mm bearings probably cost about $3 for 10, but maybe
$5 for 50!!   When I am sourcing these small cheap parts I usually prefer to buy the larger packages
so that I have them on hand for other projects.

I also used brand new 30 guage magnet wire for the coils, but this could easily be
replaced with reclaimed wire from an old wall-wart transormer or similar.

Without further adoo, here is the **BOM** (Bill of Materials):

<style>
td {
	vertical-align: top;
}
</style>

<table>
<tr>
	<th>Category</td>
	<th>Descrip</td>
	<th>Qty</td>
	<th>Cost</td>
	<th>Notes</td>
</tr>

<tr>
	<td colspan='5'>Wood</td>
</tr>
<tr>
	<td></td>
	<td>
		frame parts:  back, middle, front,
		rider, frame top, house top, house bottom,
		and box top
	</td>
	<td></td>
	<td>$3.50</td>
	<td>
		I used 3 pieces of 12x16" cheap 3/16" local plywood for these parts.
		Since I cut them from 4x8 foot sheets that cost me
		about $14 each here, I can get about 4 clocks out
		of one sheet of 4x8 feet
	</td>
</tr>
<tr>
	<td></td>
	<td>
		mechanism parts: 5 x big gears,
		8 x small gears, cam, arms, pawls,
		spacers, stops, and washers
	</td>
	<td></td>
	<td>$8.00</td>
	<td>
		one 12x12 piece of 3/16" imported birch plywood, bulk price.
		smaller parts can be cut inside the gears
	</td>
</tr>
<tr>
	<td></td>
	<td>pendulum</td>
	<td></td>
	<td>$2.50</td>
	<td>
		I used some nice 3/16" walnut plywood for this
		that cost me about $12.
		I can get about 4 pendulums from one piece
	</td>
</tr>
<tr>
	<td></td>
	<td>face, pendulum plates</td>
	<td></td>
	<td>$6.00</td>
	<td>one 12x12" piece of 1/8" imported birch plywood, bulk price</td>
</tr>
<tr>
	<td></td>
	<td>numbers, seconds hand</td>
	<td></td>
	<td>$2.50</td>
	<td>about 1/4 of piece of 4x24" 1/8" walnut that cost about $10</td>
</tr>
<tr>
	<td></td>
	<td>minute and hour hands</td>
	<td></td>
	<td>$1.25</td>
	<td>about 1/8 of piece of 4x24" 1/8" maple that cost about $10</td>
</tr>

<tr>
	<td colspan='5'>Plastic</td>
</tr>
	<tr><td></td><td>box			</td><td>1	</td><td>$4.00	</td><td>rough guesses of my plastic filament costs ($20/kg)
	<tr><td></td><td>sensor housing	</td><td>1	</td><td>$0.50	</td><td></td></tr>
	<tr><td></td><td>wire tie		</td><td>1	</td><td>$0.25	</td><td></td></tr>
	<tr><td></td><td>bobbin parts	</td><td>2	</td><td>$0.40	</td><td></td></tr>
	<tr><td></td><td>LED lenses		</td><td>5	</td><td>$0.25	</td><td>clear</td></tr>

<tr>
	<td colspan='5'>Tubes, Bearings, Weights</td>
</tr>
	<tr><td></td><td>Brass Tube, 6mm	</td><td>1	</td><td>		</td><td rowspan='3'>you can buy a set of telescoping brass tubes for about $12-14. a single clock uses about 1/2 of a set of 12" tubes</td></tr>
	<tr><td></td><td>Brass Tube, 5mm	</td><td>2	</td><td>		</td><td></td></tr>
	<tr><td></td><td>Brass Tube, 4mm	</td><td>3	</td><td>$7.00	</td><td></td></tr>
	<tr><td></td><td>m3 threaded rod	</td><td>2	</td><td>$2.00  </td><td></td></tr>
	<tr><td></td><td>Bearing, 394		</td><td>4	</td><td>$1.25	</td><td rowspan='4'>the prices of the bearings depends on how many you buy at a time</td></tr>
	<tr><td></td><td>Bearing, 4114		</td><td>1	</td><td>$0.50	</td><td></td></tr>
	<tr><td></td><td>Bearing, 5165		</td><td>2	</td><td>$1.00  </td><td></td></tr>
	<tr><td></td><td>Bearing, 6155		</td><td>1	</td><td>$1.00  </td><td></td></tr>
	<tr><td></td><td>Arm Weights		</td><td>2	</td><td>$1.00  </td><td></td></tr>
	<tr><td></td><td>Pendulum Weight	</td><td>1	</td><td>$2.00  </td><td></td></tr>

<tr>
	<td colspan='5'>Screws, Washers, Nuts</td>
</tr>
	<tr><td></td><td>m3 x 45 machine	</td><td>1	</td><td>$1.00</td>	</td><td>can use M3 threaded rod instead</td></tr>
	<tr><td></td><td>m3 x 15 machine 	</td><td>6	</td><td></td>		</td><td>weights, arms, box front</td></tr>
	<tr><td></td><td>m3 x 12 machine	</td><td>8	</td><td></td>		</td><td>cam rollers, pawls, box back</td></tr>
	<tr><td></td><td>m2 x 8 machine		</td><td>4	</td><td></td>		</td><td>mag holder, sensor housing</td></tr>
	<tr><td></td><td>m3 x 6 wood		</td><td>9	</td><td></td>		</td><td>pcb, sensor</td></tr>
	<tr><td></td><td>m3 x 12 wood		</td><td>6	</td><td></td>		</td><td>face, front</td></tr>
	<tr><td></td><td>m3 square nut		</td><td>2	</td><td></td>		</td><td>pendulum rod</td></tr>
	<tr><td></td><td>m3 hex nut			</td><td>1	</td><td></td>		</td><td>threaded rod</td></tr>
	<tr><td></td><td>m3 washers			</td><td>12 </td><td></td>      </td><td></td></tr>
	<tr><td></td><td>m4 washers			</td><td>2  </td><td></td>      </td><td></td></tr>
	<tr><td></td><td>m5 washers			</td><td>1	</td><td>$2.50</td>	</td><td>approximate cost of above when bought in sets or in bulk</td></tr>

<tr>
	<td colspan='5'>Electronics</td>
</tr>
	<tr><td></td><td>ESP32 Dev0						</td><td>1	</td><td>$7.00</td><td></td></tr>
	<tr><td></td><td>AS5600 magnetic angle sensor	</td><td>1	</td><td>$4.00</td><td></td></tr>
	<tr><td></td><td>PCB							</td><td>1	</td><td>$2.00</td><td>copper board, UV epoxy</td></tr>
	<tr><td></td><td>LEDs							</td><td>5	</td><td>$0.50</td><td>1/12th of a WS2812b (60/meter) LED strip</td></tr>
	<tr><td></td><td>Coils							</td><td>2	</td><td>$1.00</td><td>approx 18' 30 guage wire each</td></tr>
	<tr><td></td><td>Buttons						</td><td>2	</td><td>$1.00</td><td></td></tr>
	<tr><td></td><td>L293D driver IC				</td><td>1	</td><td>$1.00</td><td></td></tr>
	<tr><td></td><td>10K resistor					</td><td>7	</td><td>$0.40</td><td></td></tr>
	<tr><td></td><td>100 uf electrolytic cap		</td><td>2	</td><td>$0.75</td><td></td></tr>
	<tr><td></td><td>10 uf electrolytic cap			</td><td>1	</td><td>$0.10</td><td></td></tr>
	<tr><td></td><td>15 pin female header			</td><td>2	</td><td>$0.50</td><td></td></tr>
	<tr><td></td><td>2 pin female header			</td><td>2	</td><td>$0.50</td><td></td></tr>
	<tr><td></td><td>JST 4 pin header				</td><td>2	</td><td>$0.25</td><td></td></tr>
	<tr><td></td><td>JST 3 pin header				</td><td>1	</td><td>$0.25</td><td></td></tr>
	<tr><td></td><td>connecting wires				</td><td>13	</td><td>$0.25</td><td></td></tr>
<tr>
	<td colspan='5'>Cables</td>
</tr>
	<tr><td></td><td>4 conductor sensor cable		</td><td>1	</td><td>$1.00</td><td>approx 18"  </td></tr>
	<tr><td></td><td>JST 4 pin connector			</td><td>5	</td><td>$0.25</td><td>1 x 5 pieces</td></tr>
	<tr><td></td><td>Micro JST 4 pin connector		</td><td>5	</td><td>$0.75</td><td>1 x 5 pieces</td></tr>
	<tr><td></td><td>3 pin JST wire header			</td><td>1	</td><td>$1.00</td><td>for LEDs    </td></tr>
<tr>
	<td colspan='5'>Other</td>
</tr>
	<tr><td></td><td>5x5x2 mm magnet				</td><td>2	</td><td>$0.50</td><td>bought 50 for $12.50</td></tr>
	<tr><td></td><td>3x3x2 mm magnet				</td><td>3  </td><td>$0.54</td><td>bought 50 for $9.00</td></tr>
	<tr><td></td><td>6mm diametric magnet			</td><td>1	</td><td>		</td><td>included with as5600 sensors</td></tr>
	<tr><td></td><td>30 guage wire					</td><td>	</td><td>$0.50	</td><td>approx 36'</td></tr>
	<tr><td></td><td>power supply					</td><td>1	</td><td>$7.00  </td><td></td></tr>

<tr>
	<th>TOTAL</th>
	<td>approx 200 individual parts !!</td>
	<td></td>
	<th style="whitespace:nowrap;">$79.36</th>
	<td></td>
</tr>
</table>



**Next:** Laser cutting the [**Wooden Parts**](wood.md) of the clock ...
