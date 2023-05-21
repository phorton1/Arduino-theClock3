## Build Process

### Parts

- Make PCB
  - Preparation
  - Meshing
  - Cross Hairs
  - Isolation
  - Copper Cleanup
  - Apply Epoxy
  - Laser
  - Remove Epoxy
  - UV Light
  - Drilling 0.9
  - Drilling 1.0
  - Milling Holes
  - Cutout
  - Removal
  - Washing
- Laser Cut Wood
  - Thick Ply 1
  - Thick Ply 2
  - Gear Ply
  - Thin Ply
  - Walnut
  - Maple
- 3D Print Plastic
  - Box
  - Sensor Housing
  - Magnet Holder
  - Wire Tie
  - LED Lenses
- Hand Cut Metal
  - 6 mm tube
  - 5 mm tubes
  - 4 mm tubes
  - threaded rods

### PCB Soldering

- Pad Sanding
- Cut and Fit Wires
- Solder Wires
- Resistors
- L293D
- JST Connectors
- Dupont Connectors
- Switches
- Capacitors
- (Epoxy)


### Pre-Assembly

- Initial
  - Clean Big Pieces with Paper Towel
  - Glue Numbers into Face (detail work and drying time)
  - Glue Frame Together (with clamps and drying time)
- Sanding (messy)
  - All Gear Teeth (5 large, 10 small)
  - Cam Risers
  - Gear Interior Cutouts
  - All Parts front and back
    - Pendulum & Parts
	- Gears and Cams
	- Arms and Pawls
	- Second, Minutes, and Hours hands
- Paint
  - Face (4 coats)
    - then sand back and apply one more coat (5 total to front)
  - Frames (4 coats)
  - Pendulum, Hands, Box Top, Rider (4 coats)
  - Mechanism (Gears, Cam, Arms, Pawls) (4 coats)

- Finish Sand Mechanism
  - Cams, Pawls and Seconds Wheel
- Spin Balance and Glue Double Gears

- Build Pendulum
  - Drill and Tap Lead Ball
  - Glue nuts Weight and Rod in place
  - Screw on covers
  - Glue Magnet

- Build Sensor Cable
  - cut cable to length, strip ends
  - small JST Connector
  - large JST Connector

- Glue End Stops to Top Tube
- Glue Sensor Magnet and Wooden Washers

- Clean & Lubricate Bearings
- Dry Fit Gears on Tubes



### INITIAL ASSEMBLY

- Bearings into Frame

- Mount Magnetic Sensor
  - Attach Sensor to Housing
  - Attach Housing to Frame
  - Run Sensor Wire w/"tie" to Rider
  - Hot Glue Wire as needed

- Assemble Gears
- Adjust
- Disassemble
- Super Glue Gears on Tubes
- Epoxy Bearings in Place

- Final Assemble Gears
  - Dry Lube Gears, Pawls
  - Oil Tubes

- Build Arms and Weights
  - Press Bearings
  - Drill and Attach Weights

- Mount Pendulum, Cam, Gears
- Adjust Pendulum, Cam Spacing
- Add Arms
  - FIRST TICK

- Align & Epoxy Arm Spacers

- Refine Ticks
  - more dry lube
  - detail adjustments



### BOX ASSEMBLY

Starting with assumption we will use Micro USB connector (TBD)

- LED Strip Soldering
	- change wire polarity
- Solder Coil Wires
  - epoxy wires in place

- Load Program and SPIFFS on ESP32
- Basic Elec Test Circuit Board
    - Jumper 5V and VCC
	- Ext LED Test
	- Ext Sensor Test
    - WIFI Connection (30 seconds if no sensor)
	- Ext Coil Test
	- Mark Coil Connectors (Red/Black)
	- Epoxy Dupont connectors on Board

- Epoxy Coils in Place
- Epoxy LED Lenses
- Hot Glue LED Strip
- Mount Circuit Board

- Glue Magnets in Magnet Holder
- Attach Face
- Attach Hands
- Mount (no screws) Frame to Box


### INITIAL INTEGRATION AND TUNING

- Sensor
- Zero
- Power Swings
- Tune Pendulum
- Basic Min Max Swings (NO PID CHANGES!)
- Tune Pendulum
- Attach Magnet Spring

Try to physically adjust the clock
to work with the program defaults.
Minimize variations between clocks


### Test Phase (before shipping)

Test for at least one week.  Probably need a different power
connector and ends on the power supplies (plus new JST connector)
