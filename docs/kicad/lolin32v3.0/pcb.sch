EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GND #PWR011
U 1 1 637B8BF0
P 5250 5750
F 0 "#PWR011" H 5250 5500 50  0001 C CNN
F 1 "GND" V 5250 5550 50  0000 C CNN
F 2 "" H 5250 5750 50  0001 C CNN
F 3 "" H 5250 5750 50  0001 C CNN
	1    5250 5750
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR012
U 1 1 637B8BF6
P 5050 5750
F 0 "#PWR012" H 5050 5600 50  0001 C CNN
F 1 "+5V" V 5050 5950 50  0000 C CNN
F 2 "" H 5050 5750 50  0001 C CNN
F 3 "" H 5050 5750 50  0001 C CNN
	1    5050 5750
	1    0    0    -1  
$EndComp
Text GLabel 5150 5750 1    50   Input ~ 0
LEDS
$Comp
L Connector_Generic:Conn_01x02 J1
U 1 1 6374560A
P 1950 4000
F 0 "J1" V 2150 3900 50  0000 L CNN
F 1 "power" V 2050 3850 50  0000 L CNN
F 2 "0_my_footprints:myPinHeader_1x02" H 1950 4000 50  0001 C CNN
F 3 "~" H 1950 4000 50  0001 C CNN
	1    1950 4000
	0    -1   -1   0   
$EndComp
$Comp
L Device:CP C2
U 1 1 639AE459
P 2350 2950
F 0 "C2" H 2468 2996 50  0000 L CNN
F 1 "100uf" H 2468 2905 50  0000 L CNN
F 2 "0_my_footprints:CP_my100uf" H 2388 2800 50  0001 C CNN
F 3 "~" H 2350 2950 50  0001 C CNN
	1    2350 2950
	0    -1   1    0   
$EndComp
$Comp
L cnc3018_Library:L293D U1
U 1 1 63741DEA
P 7400 4200
F 0 "U1" H 7150 4000 50  0000 L CNN
F 1 "L293D" H 7400 4000 50  0000 L CNN
F 2 "0_my_footprints:myDip16" H 7200 4000 50  0001 C CNN
F 3 "" H 7200 4000 50  0001 C CNN
	1    7400 4200
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 63742574
P 6800 4950
F 0 "R2" H 6600 5000 50  0000 L CNN
F 1 "10K" V 6800 4950 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6730 4950 50  0001 C CNN
F 3 "~" H 6800 4950 50  0001 C CNN
	1    6800 4950
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR020
U 1 1 6376D044
P 7000 4050
F 0 "#PWR020" H 7000 3900 50  0001 C CNN
F 1 "+5V" V 7000 4250 50  0000 C CNN
F 2 "" H 7000 4050 50  0001 C CNN
F 3 "" H 7000 4050 50  0001 C CNN
	1    7000 4050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR024
U 1 1 6376FE8F
P 7300 4800
F 0 "#PWR024" H 7300 4550 50  0001 C CNN
F 1 "GND" V 7300 4600 50  0000 C CNN
F 2 "" H 7300 4800 50  0001 C CNN
F 3 "" H 7300 4800 50  0001 C CNN
	1    7300 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR025
U 1 1 63770177
P 7400 4800
F 0 "#PWR025" H 7400 4550 50  0001 C CNN
F 1 "GND" V 7400 4600 50  0000 C CNN
F 2 "" H 7400 4800 50  0001 C CNN
F 3 "" H 7400 4800 50  0001 C CNN
	1    7400 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR021
U 1 1 63770AB5
P 7300 4050
F 0 "#PWR021" H 7300 3800 50  0001 C CNN
F 1 "GND" V 7300 3850 50  0000 C CNN
F 2 "" H 7300 4050 50  0001 C CNN
F 3 "" H 7300 4050 50  0001 C CNN
	1    7300 4050
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR022
U 1 1 63770D99
P 7400 4050
F 0 "#PWR022" H 7400 3800 50  0001 C CNN
F 1 "GND" V 7400 3850 50  0000 C CNN
F 2 "" H 7400 4050 50  0001 C CNN
F 3 "" H 7400 4050 50  0001 C CNN
	1    7400 4050
	-1   0    0    1   
$EndComp
Text GLabel 7200 4800 3    50   Input ~ 0
OA1
Text GLabel 7500 4800 3    50   Input ~ 0
OA2
Text GLabel 7600 4800 3    50   Input ~ 0
I2
Text GLabel 7100 4800 3    50   Input ~ 0
I1
Text GLabel 7000 4800 3    50   Input ~ 0
EN
$Comp
L power:GND #PWR030
U 1 1 638F1E09
P 6800 5100
F 0 "#PWR030" H 6800 4850 50  0001 C CNN
F 1 "GND" V 6800 4900 50  0000 C CNN
F 2 "" H 6800 5100 50  0001 C CNN
F 3 "" H 6800 5100 50  0001 C CNN
	1    6800 5100
	1    0    0    -1  
$EndComp
Text GLabel 7350 5800 3    50   Input ~ 0
OA1
$Comp
L power:+5V #PWR026
U 1 1 637CD82C
P 7700 4800
F 0 "#PWR026" H 7700 4650 50  0001 C CNN
F 1 "+5V" V 7700 5000 50  0000 C CNN
F 2 "" H 7700 4800 50  0001 C CNN
F 3 "" H 7700 4800 50  0001 C CNN
	1    7700 4800
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR016
U 1 1 637FD29B
P 2500 2950
F 0 "#PWR016" H 2500 2700 50  0001 C CNN
F 1 "GND" V 2500 2750 50  0000 C CNN
F 2 "" H 2500 2950 50  0001 C CNN
F 3 "" H 2500 2950 50  0001 C CNN
	1    2500 2950
	-1   0    0    -1  
$EndComp
$Comp
L Switch:SW_SPST SW1
U 1 1 6381C2AE
P 3900 5800
F 0 "SW1" H 3900 6035 50  0000 C CNN
F 1 "SW" H 3900 5944 50  0000 C CNN
F 2 "cnc3018-PCB:right_angle_button" H 3900 5800 50  0001 C CNN
F 3 "~" H 3900 5800 50  0001 C CNN
	1    3900 5800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR023
U 1 1 638214D0
P 3700 5800
F 0 "#PWR023" H 3700 5550 50  0001 C CNN
F 1 "GND" V 3700 5600 50  0000 C CNN
F 2 "" H 3700 5800 50  0001 C CNN
F 3 "" H 3700 5800 50  0001 C CNN
	1    3700 5800
	1    0    0    -1  
$EndComp
Text GLabel 4100 5800 2    50   Input ~ 0
SW1
$Comp
L Connector_Generic:Conn_01x03 J8
U 1 1 63746FC8
P 5150 5950
F 0 "J8" V 5350 5900 50  0000 L CNN
F 1 "leds" V 5250 5900 50  0000 L CNN
F 2 "0_my_footprints:myJSTx03" H 5150 5950 50  0001 C CNN
F 3 "~" H 5150 5950 50  0001 C CNN
	1    5150 5950
	0    1    1    0   
$EndComp
Text GLabel 7450 5800 3    50   Input ~ 0
OA2
Wire Wire Line
	6800 4800 7000 4800
Text GLabel 7400 3100 3    50   Input ~ 0
OB2
Text GLabel 7300 3100 3    50   Input ~ 0
OB1
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 6387F0FB
P 7300 2900
F 0 "J3" V 7500 2800 50  0000 L CNN
F 1 "coil2" V 7400 2750 50  0000 L CNN
F 2 "0_my_footprints:myPinHeader_1x02" H 7300 2900 50  0001 C CNN
F 3 "~" H 7300 2900 50  0001 C CNN
	1    7300 2900
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 6387E6E7
P 7350 5600
F 0 "J2" V 7550 5500 50  0000 L CNN
F 1 "coil1" V 7450 5450 50  0000 L CNN
F 2 "0_my_footprints:myPinHeader_1x02" H 7350 5600 50  0001 C CNN
F 3 "~" H 7350 5600 50  0001 C CNN
	1    7350 5600
	0    -1   -1   0   
$EndComp
Text GLabel 7200 4050 1    50   Input ~ 0
OB1
Text GLabel 7100 4050 1    50   Input ~ 0
I1
Text GLabel 7600 4050 1    50   Input ~ 0
I2
Text GLabel 7500 4050 1    50   Input ~ 0
OB2
Text GLabel 7700 4050 1    50   Input ~ 0
EN
$Comp
L Switch:SW_SPST SW2
U 1 1 6415ABC4
P 3900 6300
F 0 "SW2" H 3900 6535 50  0000 C CNN
F 1 "SW" H 3900 6444 50  0000 C CNN
F 2 "cnc3018-PCB:right_angle_button" H 3900 6300 50  0001 C CNN
F 3 "~" H 3900 6300 50  0001 C CNN
	1    3900 6300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR027
U 1 1 6415ABCA
P 3700 6300
F 0 "#PWR027" H 3700 6050 50  0001 C CNN
F 1 "GND" V 3700 6100 50  0000 C CNN
F 2 "" H 3700 6300 50  0001 C CNN
F 3 "" H 3700 6300 50  0001 C CNN
	1    3700 6300
	1    0    0    -1  
$EndComp
Text GLabel 4100 6300 2    50   Input ~ 0
SW2
$Comp
L Connector_Generic:Conn_01x04 J5
U 1 1 6415D57E
P 6000 5950
F 0 "J5" V 6200 5950 50  0000 R CNN
F 1 "Sens1" V 6100 6050 50  0000 R CNN
F 2 "0_my_footprints:myJSTx04" H 6000 5950 50  0001 C CNN
F 3 "~" H 6000 5950 50  0001 C CNN
	1    6000 5950
	0    1    1    0   
$EndComp
Text GLabel 5900 5750 1    50   Input ~ 0
SCL
Text GLabel 5800 5750 1    50   Input ~ 0
SDA
$Comp
L power:+3.3V #PWR05
U 1 1 64171C94
P 6100 5750
F 0 "#PWR05" H 6100 5600 50  0001 C CNN
F 1 "+3.3V" V 6100 6000 50  0000 C CNN
F 2 "" H 6100 5750 50  0001 C CNN
F 3 "" H 6100 5750 50  0001 C CNN
	1    6100 5750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 6417292B
P 6000 5750
F 0 "#PWR06" H 6000 5500 50  0001 C CNN
F 1 "GND" V 6000 5550 50  0000 C CNN
F 2 "" H 6000 5750 50  0001 C CNN
F 3 "" H 6000 5750 50  0001 C CNN
	1    6000 5750
	-1   0    0    1   
$EndComp
$Comp
L Device:CP C1
U 1 1 641834E2
P 1700 2950
F 0 "C1" H 1818 2996 50  0000 L CNN
F 1 "100uf" H 1818 2905 50  0000 L CNN
F 2 "0_my_footprints:CP_my100uf" H 1738 2800 50  0001 C CNN
F 3 "~" H 1700 2950 50  0001 C CNN
	1    1700 2950
	0    -1   1    0   
$EndComp
$Comp
L power:+5V #PWR013
U 1 1 641834E8
P 1550 2950
F 0 "#PWR013" H 1550 2800 50  0001 C CNN
F 1 "+5V" V 1550 3150 50  0000 C CNN
F 2 "" H 1550 2950 50  0001 C CNN
F 3 "" H 1550 2950 50  0001 C CNN
	1    1550 2950
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR014
U 1 1 641834EE
P 1850 2950
F 0 "#PWR014" H 1850 2700 50  0001 C CNN
F 1 "GND" V 1850 2750 50  0000 C CNN
F 2 "" H 1850 2950 50  0001 C CNN
F 3 "" H 1850 2950 50  0001 C CNN
	1    1850 2950
	-1   0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR015
U 1 1 641987B2
P 2200 2950
F 0 "#PWR015" H 2200 2800 50  0001 C CNN
F 1 "+3.3V" V 2200 3200 50  0000 C CNN
F 2 "" H 2200 2950 50  0001 C CNN
F 3 "" H 2200 2950 50  0001 C CNN
	1    2200 2950
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR031
U 1 1 64207DB8
P 6000 4950
F 0 "#PWR031" H 6000 4700 50  0001 C CNN
F 1 "GND" V 6000 4750 50  0000 C CNN
F 2 "" H 6000 4950 50  0001 C CNN
F 3 "" H 6000 4950 50  0001 C CNN
	1    6000 4950
	1    0    0    -1  
$EndComp
$Comp
L 0_my_symbols:LOLIN32 U?
U 1 1 644575DB
P 5000 4500
F 0 "U?" H 4400 4600 50  0000 L CNN
F 1 "LOLIN32" H 4600 4600 50  0000 L CNN
F 2 "" H 4650 4600 50  0001 C CNN
F 3 "" H 4650 4600 50  0001 C CNN
	1    5000 4500
	1    0    0    -1  
$EndComp
Text GLabel 4600 4950 3    50   Input ~ 0
SW1
Text GLabel 4700 4950 3    50   Input ~ 0
SW2
Text GLabel 5500 4950 3    50   Input ~ 0
SDA
Text GLabel 5600 4950 3    50   Input ~ 0
SCL
Text GLabel 5100 4950 3    50   Input ~ 0
LEDS
Text GLabel 5200 3950 1    50   Input ~ 0
I2
Text GLabel 5000 3950 1    50   Input ~ 0
I1
Text GLabel 5100 3950 1    50   Input ~ 0
EN
$Comp
L power:+3.3V #PWR?
U 1 1 6447CE6B
P 4800 4950
F 0 "#PWR?" H 4800 4800 50  0001 C CNN
F 1 "+3.3V" V 4800 5200 50  0000 C CNN
F 2 "" H 4800 4950 50  0001 C CNN
F 3 "" H 4800 4950 50  0001 C CNN
	1    4800 4950
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 6447DF6E
P 6000 3950
F 0 "#PWR?" H 6000 3800 50  0001 C CNN
F 1 "+3.3V" V 6000 4200 50  0000 C CNN
F 2 "" H 6000 3950 50  0001 C CNN
F 3 "" H 6000 3950 50  0001 C CNN
	1    6000 3950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 64480274
P 4300 4950
F 0 "#PWR?" H 4300 4700 50  0001 C CNN
F 1 "GND" V 4300 4750 50  0000 C CNN
F 2 "" H 4300 4950 50  0001 C CNN
F 3 "" H 4300 4950 50  0001 C CNN
	1    4300 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 64480976
P 5300 4950
F 0 "#PWR?" H 5300 4700 50  0001 C CNN
F 1 "GND" V 5300 4750 50  0000 C CNN
F 2 "" H 5300 4950 50  0001 C CNN
F 3 "" H 5300 4950 50  0001 C CNN
	1    5300 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 64480DF9
P 5400 4950
F 0 "#PWR?" H 5400 4700 50  0001 C CNN
F 1 "GND" V 5400 4750 50  0000 C CNN
F 2 "" H 5400 4950 50  0001 C CNN
F 3 "" H 5400 4950 50  0001 C CNN
	1    5400 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 6448192E
P 4500 3950
F 0 "#PWR?" H 4500 3700 50  0001 C CNN
F 1 "GND" V 4500 3750 50  0000 C CNN
F 2 "" H 4500 3950 50  0001 C CNN
F 3 "" H 4500 3950 50  0001 C CNN
	1    4500 3950
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR?
U 1 1 64484DF1
P 5900 3250
F 0 "#PWR?" H 5900 3100 50  0001 C CNN
F 1 "+3.3V" V 5900 3500 50  0000 C CNN
F 2 "" H 5900 3250 50  0001 C CNN
F 3 "" H 5900 3250 50  0001 C CNN
	1    5900 3250
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C?
U 1 1 64484DF7
P 5900 3400
F 0 "C?" H 5650 3450 50  0000 L CNN
F 1 "10uf" H 5550 3350 50  0000 L CNN
F 2 "0_my_footprints:CP_my10uf" H 5938 3250 50  0001 C CNN
F 3 "~" H 5900 3400 50  0001 C CNN
	1    5900 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	5900 3550 5900 3950
$Comp
L 0_my_symbols:BUCK01 M?
U 1 1 644D8253
P 2850 4350
F 0 "M?" H 2825 3985 50  0000 C CNN
F 1 "BUCK01" H 2825 4076 50  0000 C CNN
F 2 "" H 2800 4650 50  0001 C CNN
F 3 "" H 2800 4650 50  0001 C CNN
	1    2850 4350
	1    0    0    1   
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 644E1355
P 1600 4400
F 0 "BT?" H 1718 4496 50  0000 L CNN
F 1 "Battery_Cell" H 1718 4405 50  0000 L CNN
F 2 "" V 1600 4460 50  0001 C CNN
F 3 "~" V 1600 4460 50  0001 C CNN
	1    1600 4400
	1    0    0    -1  
$EndComp
Text GLabel 4100 3950 1    50   Input ~ 0
BAT-
Text GLabel 4200 3950 1    50   Input ~ 0
BAT+
Text GLabel 1000 4500 0    50   Input ~ 0
BAT-
Text GLabel 1000 4200 0    50   Input ~ 0
BAT+
$Comp
L power:+5V #PWR?
U 1 1 6453DD6D
P 3200 4000
F 0 "#PWR?" H 3200 3850 50  0001 C CNN
F 1 "+5V" V 3200 4200 50  0000 C CNN
F 2 "" H 3200 4000 50  0001 C CNN
F 3 "" H 3200 4000 50  0001 C CNN
	1    3200 4000
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 64542CEA
P 2450 4700
F 0 "#PWR?" H 2450 4450 50  0001 C CNN
F 1 "GND" V 2450 4500 50  0000 C CNN
F 2 "" H 2450 4700 50  0001 C CNN
F 3 "" H 2450 4700 50  0001 C CNN
	1    2450 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 645433B5
P 3200 4700
F 0 "#PWR?" H 3200 4450 50  0001 C CNN
F 1 "GND" V 3200 4500 50  0000 C CNN
F 2 "" H 3200 4700 50  0001 C CNN
F 3 "" H 3200 4700 50  0001 C CNN
	1    3200 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 4200 1100 4200
Wire Wire Line
	1500 4200 1600 4200
Connection ~ 1600 4200
Wire Wire Line
	1000 4500 1600 4500
Connection ~ 1600 4500
Wire Wire Line
	1600 4500 2450 4500
Wire Wire Line
	2450 4500 2450 4700
Connection ~ 2450 4500
Wire Wire Line
	3200 4500 3200 4700
Wire Wire Line
	3200 4000 3200 4200
$Comp
L Switch:SW_SPST SW?
U 1 1 64531ACE
P 1300 4200
F 0 "SW?" H 1300 4435 50  0000 C CNN
F 1 "SW" H 1300 4344 50  0000 C CNN
F 2 "cnc3018-PCB:right_angle_button" H 1300 4200 50  0001 C CNN
F 3 "~" H 1300 4200 50  0001 C CNN
	1    1300 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1600 4200 1950 4200
Wire Wire Line
	2050 4200 2450 4200
Text GLabel 5400 3950 1    50   Input ~ 0
BAT_SENSE
$Comp
L power:GND #PWR?
U 1 1 6447B518
P 4000 2850
F 0 "#PWR?" H 4000 2600 50  0001 C CNN
F 1 "GND" V 4000 2650 50  0000 C CNN
F 2 "" H 4000 2850 50  0001 C CNN
F 3 "" H 4000 2850 50  0001 C CNN
	1    4000 2850
	1    0    0    -1  
$EndComp
Text GLabel 4500 2250 2    50   Input ~ 0
BAT+
Text GLabel 4500 2550 2    50   Input ~ 0
BAT_SENSE
$Comp
L Device:R R?
U 1 1 6448B65B
P 4000 2400
F 0 "R?" H 3800 2450 50  0000 L CNN
F 1 "33K" V 4000 2400 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 3930 2400 50  0001 C CNN
F 3 "~" H 4000 2400 50  0001 C CNN
	1    4000 2400
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 6448B81C
P 4000 2700
F 0 "R?" H 3800 2750 50  0000 L CNN
F 1 "100K" V 4000 2700 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 3930 2700 50  0001 C CNN
F 3 "~" H 4000 2700 50  0001 C CNN
	1    4000 2700
	-1   0    0    1   
$EndComp
Wire Wire Line
	4000 2250 4500 2250
Wire Wire Line
	4000 2550 4500 2550
Connection ~ 4000 2550
$EndSCHEMATC
