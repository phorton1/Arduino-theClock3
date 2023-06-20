# theClock3 - Build Overview


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
**[Trouble Shooting](trouble.md)** --
**[Notes](notes.md)**


## Build Overview

The process of building the clock involves a number of different steps
and processes.  I have tried to group these into six pages in addition
to this one.

- Laser cut and prep the **[Wood](wood.md)** parts
	- assemble and clear-coat the **frame**
	- assemble, sand, and clear-coat the **face**
	- **sand** the gears and other wooden parts
	- **clear coat** the gears other parts
    - sand the gears and certain mechanism parts again
	- align and glue to the **pairs of gears** together
- Wind the electromagnet **[Coils](coils.md)**
- Create the **[electronics](electronics.md)** **PCB** (printed circuit board) and cables/connectors
    - PCB isolation routing, epoxy coating, laser curing, drilling, milling, and cutout
	- **continuity test** the PCB
	- **solder components** to the PCB
	- **make cables/connectors** for the LEDs, Coils, AS5600 angle sensor
	  and secondary micro-usb connector.
- Install and Build the **[Software](software.md)**
	- install the *Arduino IDE* and necessary **libraries**
    - build the **code** and upload it, and the *SPIFFS file system* to the ESP32
	- this page also has a quick *introduction and overview* of the **Software**
	- do a quick **submodule test** of PCB (Buttons, LEDs, Coils, and Angle Sensor, etc)
- Finish the **[Other](other.md)** sub-assemblies:
	- cut and sand the **brass tubes** and **threaded rods** to length
	- 3D print the **Box**
    - epoxy glue the coils into the box
	- epoxy glue LED lenses into the box
	- hot glue the LEDs into the box
	- screw the PCB and secondary micro-usb module into the box
	- epoxy glue the *magnets* into the **pendulum** and assemble it
	- epoxy glue the *magnets* into the **magnetic spring**
- [Final Assembly](assembly.md) includes:
	- epoxy glue the *tubes* into the **gears**
	- press the *bearings* into the **frame**
	- attach the *angle sensor* to the **frame**
	- assemble the **gear train** and add the **bridge**
	- assemble the **arms and weights** and attach them to the frame
	- test and adjust the **mechanics**
	- attach the **face** and **box** to finish the clock

After the clock is physically assembled and roughly working, then
we can proceed to a step by step guide to **[Tuning](tuning.md)**
the clock and will be prepared to present the **[User Manual](user_manual.md)**
and **[Trouble Shooting Guide](trouble.md)** on subsequent pages.

## Bill of Materials

It is probably best to try to gather as many of the materials and parts
before starting to build the clock.  Therefore in this section I detail
a list of the materials and parts that are needed to build one of these.

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

I also used brand new 30 gauge magnet wire for the coils, but this could easily be
replaced with reclaimed wire from an old wall-wart transformer or similar.

Without further adoo, here is the **BOM** (Bill of Materials):

<style>
td {
	vertical-align: top;
}
</style>

<table>
<tr>
	<th>Category</th>
	<th>Descrip</th>
	<th>Qty</th>
	<th>Cost</th>
	<th>Notes</th>
</tr>

<tr>
	<td colspan='5'>Wood</td>
</tr>
<tr>
	<td></td>
	<td>
		frame parts:  back, middle, front,
		bridge, frame top, house top, house bottom,
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
	<tr><td></td><td>box			</td><td>73.6m</td><td>$6.60</td><td>at $20/kg</td></tr>
	<tr><td></td><td>coil set       </td><td>4.3m</td><td>$0.39	</td><td></td></tr>
	<tr><td></td><td>sensor housing	</td><td>3.4m</td><td>$0.30	</td><td></td></tr>
	<tr><td></td><td>magnet holder  </td><td>1.1m</td><td>$0.10	</td><td></td></tr>
	<tr><td></td><td>washers        </td><td>0.3m</td><td>$0.03	</td><td></td></tr>
	<tr><td></td><td>wire tie		</td><td>0.2m</td><td>$0.02	</td><td></td></tr>
	<tr><td></td><td>LED lenses		</td><td>0.5</td><td>$0.01	</td><td>clear</td></tr>

<tr>
	<td colspan='5'>Tubes, Bearings, Weights</td>
</tr>
	<tr><td></td><td>Brass Tube, 6mm	</td><td>approx 28mm (1.3")</td><td>$7.00</td><td rowspan='3'>you can buy a set of telescoping brass tubes for about $12-14. ang get two clocks per set from them</td></tr>
	<tr><td></td><td>Brass Tube, 5mm	</td><td>approx 130mm (5.1")</td><td></td><td></td></tr>
	<tr><td></td><td>Brass Tube, 4mm	</td><td>approx 140mm (5.5")</td><td></td><td></td></tr>
	<tr><td></td><td>m3 threaded rod	</td><td>approx 140mm (5.5")</td><td>$2.00  </td><td></td></tr>
	<tr><td></td><td>Bearing, 3x9x4	 mm	</td><td>4	</td><td>$1.25	</td><td rowspan='4'>the prices of the bearings depends on how many you buy at a time</td></tr>
	<tr><td></td><td>Bearing, 4x11x4 mm	</td><td>1	</td><td>$0.50	</td><td></td></tr>
	<tr><td></td><td>Bearing, 5x16x5 mm	</td><td>2	</td><td>$1.00  </td><td></td></tr>
	<tr><td></td><td>Bearing, 6x15x5 mm	</td><td>1	</td><td>$1.00  </td><td></td></tr>
	<tr><td></td><td>Arm Weights 1/4 oz	</td><td>2	</td><td>$1.00  </td><td>6-8mm in diameter, with hole through center</td></tr>
	<tr><td></td><td>Pendulum Weight 4 oz</td><td>1	</td><td>$2.00  </td><td>16-17mm in diameter, with hole through center</td></tr>
<tr>
	<td colspan='5'>Screws, Washers, Nuts</td>
</tr>
	<tr><td></td><td>m3 x 45 machine	</td><td>1	</td><td>$1.00</td>	</td><td>can use M3 threaded rod instead</td></tr>
	<tr><td></td><td>m3 x 15 machine 	</td><td>6	</td><td></td>		</td><td>weights, arms, box front</td></tr>
	<tr><td></td><td>m3 x 12 machine	</td><td>8	</td><td></td>		</td><td>cam rollers, pawls, box back</td></tr>
	<tr><td></td><td>m2 x 6 machine		</td><td>4	</td><td></td>		</td><td>mag holder, sensor housing</td></tr>
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
	<tr><td></td><td>Coils							</td><td>2	</td><td>$1.00</td><td>approx 18' 30 gauge wire each</td></tr>
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
	<tr><td></td><td>30 gauge wire					</td><td>	</td><td>$0.50	</td><td>approx 36'</td></tr>
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
