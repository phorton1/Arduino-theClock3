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
L power:GND #PWR029
U 1 1 637681D2
P 4700 5000
F 0 "#PWR029" H 4700 4750 50  0001 C CNN
F 1 "GND" V 4700 4800 50  0000 C CNN
F 2 "" H 4700 5000 50  0001 C CNN
F 3 "" H 4700 5000 50  0001 C CNN
	1    4700 5000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR019
U 1 1 6376865C
P 4700 4000
F 0 "#PWR019" H 4700 3750 50  0001 C CNN
F 1 "GND" V 4700 3800 50  0000 C CNN
F 2 "" H 4700 4000 50  0001 C CNN
F 3 "" H 4700 4000 50  0001 C CNN
	1    4700 4000
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR01
U 1 1 6378C0B0
P 3250 2150
F 0 "#PWR01" H 3250 2000 50  0001 C CNN
F 1 "+5V" V 3250 2350 50  0000 C CNN
F 2 "" H 3250 2150 50  0001 C CNN
F 3 "" H 3250 2150 50  0001 C CNN
	1    3250 2150
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR011
U 1 1 637B8BF0
P 7100 2150
F 0 "#PWR011" H 7100 1900 50  0001 C CNN
F 1 "GND" V 7100 1950 50  0000 C CNN
F 2 "" H 7100 2150 50  0001 C CNN
F 3 "" H 7100 2150 50  0001 C CNN
	1    7100 2150
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR012
U 1 1 637B8BF6
P 7300 2150
F 0 "#PWR012" H 7300 2000 50  0001 C CNN
F 1 "+5V" V 7300 2350 50  0000 C CNN
F 2 "" H 7300 2150 50  0001 C CNN
F 3 "" H 7300 2150 50  0001 C CNN
	1    7300 2150
	-1   0    0    1   
$EndComp
Text GLabel 7200 2150 3    50   Input ~ 0
LEDS
$Comp
L power:+3.3V #PWR017
U 1 1 637DC4FB
P 6300 3750
F 0 "#PWR017" H 6300 3600 50  0001 C CNN
F 1 "+3.3V" V 6300 4000 50  0000 C CNN
F 2 "" H 6300 3750 50  0001 C CNN
F 3 "" H 6300 3750 50  0001 C CNN
	1    6300 3750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 6378D57F
P 3150 2150
F 0 "#PWR02" H 3150 1900 50  0001 C CNN
F 1 "GND" V 3150 1950 50  0000 C CNN
F 2 "" H 3150 2150 50  0001 C CNN
F 3 "" H 3150 2150 50  0001 C CNN
	1    3150 2150
	-1   0    0    -1  
$EndComp
Text GLabel 5300 4000 1    50   Input ~ 0
I2
Text GLabel 5100 4000 1    50   Input ~ 0
I1
Text GLabel 5200 4000 1    50   Input ~ 0
EN
$Comp
L Device:CP C2
U 1 1 639AE459
P 3400 3450
F 0 "C2" H 3518 3496 50  0000 L CNN
F 1 "470uf" H 3518 3405 50  0000 L CNN
F 2 "0_my_footprints:CP_my100uf" H 3438 3300 50  0001 C CNN
F 3 "~" H 3400 3450 50  0001 C CNN
	1    3400 3450
	0    -1   1    0   
$EndComp
$Comp
L Device:CP C3
U 1 1 639B4A3D
P 6150 3900
F 0 "C3" H 5900 3950 50  0000 L CNN
F 1 "10uf" H 5800 3850 50  0000 L CNN
F 2 "0_my_footprints:CP_my10uf" H 6188 3750 50  0001 C CNN
F 3 "~" H 6150 3900 50  0001 C CNN
	1    6150 3900
	0    1    1    0   
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
Text GLabel 4750 2150 3    50   Input ~ 0
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
P 3550 3450
F 0 "#PWR016" H 3550 3200 50  0001 C CNN
F 1 "GND" V 3550 3250 50  0000 C CNN
F 2 "" H 3550 3450 50  0001 C CNN
F 3 "" H 3550 3450 50  0001 C CNN
	1    3550 3450
	-1   0    0    -1  
$EndComp
$Comp
L Switch:SW_SPST SW1
U 1 1 6381C2AE
P 3000 4400
F 0 "SW1" H 3000 4635 50  0000 C CNN
F 1 "SW" H 3000 4544 50  0000 C CNN
F 2 "0_my_footprints:myButtonAngled" H 3000 4400 50  0001 C CNN
F 3 "~" H 3000 4400 50  0001 C CNN
	1    3000 4400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR023
U 1 1 638214D0
P 2800 4400
F 0 "#PWR023" H 2800 4150 50  0001 C CNN
F 1 "GND" V 2800 4200 50  0000 C CNN
F 2 "" H 2800 4400 50  0001 C CNN
F 3 "" H 2800 4400 50  0001 C CNN
	1    2800 4400
	1    0    0    -1  
$EndComp
Text GLabel 3200 4400 2    50   Input ~ 0
SW1
Text GLabel 5100 5000 3    50   Input ~ 0
SW1
$Comp
L Connector_Generic:Conn_01x03 J8
U 1 1 63746FC8
P 7200 1950
F 0 "J8" V 7400 1900 50  0000 L CNN
F 1 "leds" V 7300 1900 50  0000 L CNN
F 2 "0_my_footprints:myJSTx03" H 7200 1950 50  0001 C CNN
F 3 "~" H 7200 1950 50  0001 C CNN
	1    7200 1950
	0    -1   -1   0   
$EndComp
Text GLabel 4850 2150 3    50   Input ~ 0
OA2
Wire Wire Line
	6800 4800 7000 4800
Text GLabel 5200 2150 3    50   Input ~ 0
OB2
Text GLabel 5100 2150 3    50   Input ~ 0
OB1
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 6387F0FB
P 5100 1950
F 0 "J3" V 5300 1850 50  0000 L CNN
F 1 "coil2" V 5200 1800 50  0000 L CNN
F 2 "0_my_footprints:myPinSocket_1x02" H 5100 1950 50  0001 C CNN
F 3 "~" H 5100 1950 50  0001 C CNN
	1    5100 1950
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 6387E6E7
P 4750 1950
F 0 "J2" V 4950 1850 50  0000 L CNN
F 1 "coil1" V 4850 1800 50  0000 L CNN
F 2 "0_my_footprints:myPinSocket_1x02" H 4750 1950 50  0001 C CNN
F 3 "~" H 4750 1950 50  0001 C CNN
	1    4750 1950
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
Text GLabel 5200 5000 3    50   Input ~ 0
SW2
$Comp
L Switch:SW_SPST SW2
U 1 1 6415ABC4
P 3000 4900
F 0 "SW2" H 3000 5135 50  0000 C CNN
F 1 "SW" H 3000 5044 50  0000 C CNN
F 2 "0_my_footprints:myButtonAngled" H 3000 4900 50  0001 C CNN
F 3 "~" H 3000 4900 50  0001 C CNN
	1    3000 4900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR027
U 1 1 6415ABCA
P 2800 4900
F 0 "#PWR027" H 2800 4650 50  0001 C CNN
F 1 "GND" V 2800 4700 50  0000 C CNN
F 2 "" H 2800 4900 50  0001 C CNN
F 3 "" H 2800 4900 50  0001 C CNN
	1    2800 4900
	1    0    0    -1  
$EndComp
Text GLabel 3200 4900 2    50   Input ~ 0
SW2
$Comp
L Connector_Generic:Conn_01x04 J5
U 1 1 6415D57E
P 6000 1950
F 0 "J5" V 6200 1950 50  0000 R CNN
F 1 "Sensor" V 6100 2050 50  0000 R CNN
F 2 "0_my_footprints:myJSTx04" H 6000 1950 50  0001 C CNN
F 3 "~" H 6000 1950 50  0001 C CNN
	1    6000 1950
	0    -1   -1   0   
$EndComp
Text GLabel 6100 2150 3    50   Input ~ 0
SCL
Text GLabel 6200 2150 3    50   Input ~ 0
SDA
$Comp
L power:+3.3V #PWR05
U 1 1 64171C94
P 5900 2150
F 0 "#PWR05" H 5900 2000 50  0001 C CNN
F 1 "+3.3V" V 5900 2400 50  0000 C CNN
F 2 "" H 5900 2150 50  0001 C CNN
F 3 "" H 5900 2150 50  0001 C CNN
	1    5900 2150
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR06
U 1 1 6417292B
P 6000 2150
F 0 "#PWR06" H 6000 1900 50  0001 C CNN
F 1 "GND" V 6000 1950 50  0000 C CNN
F 2 "" H 6000 2150 50  0001 C CNN
F 3 "" H 6000 2150 50  0001 C CNN
	1    6000 2150
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 641834E2
P 2900 3450
F 0 "C1" H 3018 3496 50  0000 L CNN
F 1 "100uf" H 3018 3405 50  0000 L CNN
F 2 "0_my_footprints:CP_my100uf" H 2938 3300 50  0001 C CNN
F 3 "~" H 2900 3450 50  0001 C CNN
	1    2900 3450
	0    -1   1    0   
$EndComp
$Comp
L power:+5V #PWR013
U 1 1 641834E8
P 2750 3450
F 0 "#PWR013" H 2750 3300 50  0001 C CNN
F 1 "+5V" V 2750 3650 50  0000 C CNN
F 2 "" H 2750 3450 50  0001 C CNN
F 3 "" H 2750 3450 50  0001 C CNN
	1    2750 3450
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR014
U 1 1 641834EE
P 3050 3450
F 0 "#PWR014" H 3050 3200 50  0001 C CNN
F 1 "GND" V 3050 3250 50  0000 C CNN
F 2 "" H 3050 3450 50  0001 C CNN
F 3 "" H 3050 3450 50  0001 C CNN
	1    3050 3450
	-1   0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR015
U 1 1 641987B2
P 3250 3450
F 0 "#PWR015" H 3250 3300 50  0001 C CNN
F 1 "+3.3V" V 3250 3700 50  0000 C CNN
F 2 "" H 3250 3450 50  0001 C CNN
F 3 "" H 3250 3450 50  0001 C CNN
	1    3250 3450
	-1   0    0    1   
$EndComp
$Comp
L 0_my_symbols:ESP32_DEV_0 U2
U 1 1 641F53B3
P 5400 4550
F 0 "U2" H 6128 4646 50  0000 L CNN
F 1 "ESP32_DEV_0" H 5000 4700 50  0000 L CNN
F 2 "0_my_footprints:myESP32DEV0" H 4850 4650 50  0001 C CNN
F 3 "" H 4850 4650 50  0001 C CNN
	1    5400 4550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR031
U 1 1 64207DB8
P 5350 6000
F 0 "#PWR031" H 5350 5750 50  0001 C CNN
F 1 "GND" V 5350 5800 50  0000 C CNN
F 2 "" H 5350 6000 50  0001 C CNN
F 3 "" H 5350 6000 50  0001 C CNN
	1    5350 6000
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR028
U 1 1 64207DBE
P 4600 5000
F 0 "#PWR028" H 4600 4850 50  0001 C CNN
F 1 "+3.3V" V 4600 5250 50  0000 C CNN
F 2 "" H 4600 5000 50  0001 C CNN
F 3 "" H 4600 5000 50  0001 C CNN
	1    4600 5000
	-1   0    0    1   
$EndComp
Wire Wire Line
	6000 3900 6000 4000
Wire Wire Line
	6300 3900 6300 3750
Text GLabel 5600 5000 3    50   Input ~ 0
SDA
Text GLabel 5900 5000 3    50   Input ~ 0
SCL
Text GLabel 6000 5000 3    50   Input ~ 0
LEDS
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 6374560A
P 3250 1950
F 0 "J1" V 3450 1850 50  0000 L CNN
F 1 "power" V 3350 1800 50  0000 L CNN
F 2 "0_my_footprints:myJSTx04" H 3250 1950 50  0001 C CNN
F 3 "~" H 3250 1950 50  0001 C CNN
	1    3250 1950
	0    -1   -1   0   
$EndComp
Text GLabel 4600 4000 1    50   Input ~ 0
VBUS
Text GLabel 3350 2150 3    50   Input ~ 0
VBUS
Text GLabel 5600 4000 1    50   Input ~ 0
SBUS
Text GLabel 5800 4000 1    50   Input ~ 0
S5V
Text GLabel 5700 4000 1    50   Input ~ 0
SBAT
$Comp
L Device:R R3
U 1 1 644B6700
P 4500 2900
F 0 "R3" H 4350 2550 50  0000 L CNN
F 1 "10K" V 4500 2900 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 4430 2900 50  0001 C CNN
F 3 "~" H 4500 2900 50  0001 C CNN
	1    4500 2900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 644B6706
P 4500 3050
F 0 "#PWR0101" H 4500 2800 50  0001 C CNN
F 1 "GND" V 4500 2850 50  0000 C CNN
F 2 "" H 4500 3050 50  0001 C CNN
F 3 "" H 4500 3050 50  0001 C CNN
	1    4500 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 644B7DDA
P 4400 2900
F 0 "R1" H 4350 2650 50  0000 L CNN
F 1 "10K" V 4400 2900 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 4330 2900 50  0001 C CNN
F 3 "~" H 4400 2900 50  0001 C CNN
	1    4400 2900
	1    0    0    -1  
$EndComp
Text GLabel 4300 3050 0    50   Input ~ 0
VBUS
Text GLabel 4600 2750 2    50   Input ~ 0
SBUS
Wire Wire Line
	4300 3050 4400 3050
Wire Wire Line
	4400 2750 4500 2750
Wire Wire Line
	4600 2750 4500 2750
Connection ~ 4500 2750
$Comp
L Device:R R5
U 1 1 644C141F
P 5350 2900
F 0 "R5" H 5200 2550 50  0000 L CNN
F 1 "10K" V 5350 2900 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 5280 2900 50  0001 C CNN
F 3 "~" H 5350 2900 50  0001 C CNN
	1    5350 2900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 644C1425
P 5350 3050
F 0 "#PWR0102" H 5350 2800 50  0001 C CNN
F 1 "GND" V 5350 2850 50  0000 C CNN
F 2 "" H 5350 3050 50  0001 C CNN
F 3 "" H 5350 3050 50  0001 C CNN
	1    5350 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 644C142B
P 5250 2900
F 0 "R4" H 5200 2650 50  0000 L CNN
F 1 "10K" V 5250 2900 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 5180 2900 50  0001 C CNN
F 3 "~" H 5250 2900 50  0001 C CNN
	1    5250 2900
	1    0    0    -1  
$EndComp
Text GLabel 5450 2750 2    50   Input ~ 0
S5V
Wire Wire Line
	5150 3050 5250 3050
Wire Wire Line
	5250 2750 5350 2750
Wire Wire Line
	5450 2750 5350 2750
Connection ~ 5350 2750
$Comp
L Device:R R7
U 1 1 644C225C
P 6200 2850
F 0 "R7" H 6050 2500 50  0000 L CNN
F 1 "10K" V 6200 2850 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6130 2850 50  0001 C CNN
F 3 "~" H 6200 2850 50  0001 C CNN
	1    6200 2850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 644C2262
P 6200 3000
F 0 "#PWR0103" H 6200 2750 50  0001 C CNN
F 1 "GND" V 6200 2800 50  0000 C CNN
F 2 "" H 6200 3000 50  0001 C CNN
F 3 "" H 6200 3000 50  0001 C CNN
	1    6200 3000
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 644C2268
P 6100 2850
F 0 "R6" H 6050 2600 50  0000 L CNN
F 1 "10K" V 6100 2850 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6030 2850 50  0001 C CNN
F 3 "~" H 6100 2850 50  0001 C CNN
	1    6100 2850
	1    0    0    -1  
$EndComp
Text GLabel 6000 3000 0    50   Input ~ 0
VBAT
Text GLabel 6300 2700 2    50   Input ~ 0
SBAT
Wire Wire Line
	6000 3000 6100 3000
Wire Wire Line
	6100 2700 6200 2700
Wire Wire Line
	6300 2700 6200 2700
Connection ~ 6200 2700
$Comp
L power:+5V #PWR0104
U 1 1 644CA8A8
P 5150 3050
F 0 "#PWR0104" H 5150 2900 50  0001 C CNN
F 1 "+5V" V 5150 3250 50  0000 C CNN
F 2 "" H 5150 3050 50  0001 C CNN
F 3 "" H 5150 3050 50  0001 C CNN
	1    5150 3050
	0    -1   1    0   
$EndComp
Text GLabel 3450 2150 3    50   Input ~ 0
VBAT
$EndSCHEMATC
