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
P 4400 4800
F 0 "#PWR029" H 4400 4550 50  0001 C CNN
F 1 "GND" V 4400 4600 50  0000 C CNN
F 2 "" H 4400 4800 50  0001 C CNN
F 3 "" H 4400 4800 50  0001 C CNN
	1    4400 4800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR019
U 1 1 6376865C
P 4400 3800
F 0 "#PWR019" H 4400 3550 50  0001 C CNN
F 1 "GND" V 4400 3600 50  0000 C CNN
F 2 "" H 4400 3800 50  0001 C CNN
F 3 "" H 4400 3800 50  0001 C CNN
	1    4400 3800
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR01
U 1 1 6378C0B0
P 3850 2100
F 0 "#PWR01" H 3850 1950 50  0001 C CNN
F 1 "+5V" V 3850 2300 50  0000 C CNN
F 2 "" H 3850 2100 50  0001 C CNN
F 3 "" H 3850 2100 50  0001 C CNN
	1    3850 2100
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR011
U 1 1 637B8BF0
P 6800 2100
F 0 "#PWR011" H 6800 1850 50  0001 C CNN
F 1 "GND" V 6800 1900 50  0000 C CNN
F 2 "" H 6800 2100 50  0001 C CNN
F 3 "" H 6800 2100 50  0001 C CNN
	1    6800 2100
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR012
U 1 1 637B8BF6
P 7000 2100
F 0 "#PWR012" H 7000 1950 50  0001 C CNN
F 1 "+5V" V 7000 2300 50  0000 C CNN
F 2 "" H 7000 2100 50  0001 C CNN
F 3 "" H 7000 2100 50  0001 C CNN
	1    7000 2100
	-1   0    0    1   
$EndComp
Text GLabel 6900 2100 3    50   Input ~ 0
LEDS
$Comp
L power:GND #PWR02
U 1 1 6378D57F
P 3550 2100
F 0 "#PWR02" H 3550 1850 50  0001 C CNN
F 1 "GND" V 3550 1900 50  0000 C CNN
F 2 "" H 3550 2100 50  0001 C CNN
F 3 "" H 3550 2100 50  0001 C CNN
	1    3550 2100
	-1   0    0    -1  
$EndComp
Text GLabel 4900 3800 1    50   Input ~ 0
PWM1
$Comp
L Switch:SW_SPST SW1
U 1 1 6381C2AE
P 3400 4000
F 0 "SW1" H 3400 4235 50  0000 C CNN
F 1 "SW" H 3400 4144 50  0000 C CNN
F 2 "0_my_footprints:myButtonAngled" H 3400 4000 50  0001 C CNN
F 3 "~" H 3400 4000 50  0001 C CNN
	1    3400 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR023
U 1 1 638214D0
P 3600 4000
F 0 "#PWR023" H 3600 3750 50  0001 C CNN
F 1 "GND" V 3600 3800 50  0000 C CNN
F 2 "" H 3600 4000 50  0001 C CNN
F 3 "" H 3600 4000 50  0001 C CNN
	1    3600 4000
	1    0    0    -1  
$EndComp
Text GLabel 3200 4000 0    50   Input ~ 0
SW1
Text GLabel 4800 4800 3    50   Input ~ 0
SW1
$Comp
L Connector_Generic:Conn_01x03 J8
U 1 1 63746FC8
P 6900 1900
F 0 "J8" V 7100 1850 50  0000 L CNN
F 1 "leds" V 7000 1850 50  0000 L CNN
F 2 "0_my_footprints:myJSTx03" H 6900 1900 50  0001 C CNN
F 3 "~" H 6900 1900 50  0001 C CNN
	1    6900 1900
	0    -1   -1   0   
$EndComp
Text GLabel 4900 4800 3    50   Input ~ 0
SW2
$Comp
L Switch:SW_SPST SW2
U 1 1 6415ABC4
P 3250 4600
F 0 "SW2" H 3250 4835 50  0000 C CNN
F 1 "SW" H 3250 4744 50  0000 C CNN
F 2 "0_my_footprints:myButtonAngled" H 3250 4600 50  0001 C CNN
F 3 "~" H 3250 4600 50  0001 C CNN
	1    3250 4600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR027
U 1 1 6415ABCA
P 3050 4600
F 0 "#PWR027" H 3050 4350 50  0001 C CNN
F 1 "GND" V 3050 4400 50  0000 C CNN
F 2 "" H 3050 4600 50  0001 C CNN
F 3 "" H 3050 4600 50  0001 C CNN
	1    3050 4600
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J5
U 1 1 6415D57E
P 5850 1900
F 0 "J5" V 6050 1900 50  0000 R CNN
F 1 "Sensor" V 5950 2000 50  0000 R CNN
F 2 "0_my_footprints:myJSTx04" H 5850 1900 50  0001 C CNN
F 3 "~" H 5850 1900 50  0001 C CNN
	1    5850 1900
	0    -1   -1   0   
$EndComp
Text GLabel 5950 2100 3    50   Input ~ 0
SCL
Text GLabel 6050 2100 3    50   Input ~ 0
SDA
$Comp
L power:+3.3V #PWR05
U 1 1 64171C94
P 5750 2100
F 0 "#PWR05" H 5750 1950 50  0001 C CNN
F 1 "+3.3V" V 5750 2350 50  0000 C CNN
F 2 "" H 5750 2100 50  0001 C CNN
F 3 "" H 5750 2100 50  0001 C CNN
	1    5750 2100
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR06
U 1 1 6417292B
P 5850 2100
F 0 "#PWR06" H 5850 1850 50  0001 C CNN
F 1 "GND" V 5850 1900 50  0000 C CNN
F 2 "" H 5850 2100 50  0001 C CNN
F 3 "" H 5850 2100 50  0001 C CNN
	1    5850 2100
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 641834E2
P 3350 2650
F 0 "C1" H 3468 2696 50  0000 L CNN
F 1 "330uf" H 3468 2605 50  0000 L CNN
F 2 "0_my_footprints:CP_my100uf" H 3388 2500 50  0001 C CNN
F 3 "~" H 3350 2650 50  0001 C CNN
	1    3350 2650
	0    -1   1    0   
$EndComp
$Comp
L power:+5V #PWR013
U 1 1 641834E8
P 3200 2650
F 0 "#PWR013" H 3200 2500 50  0001 C CNN
F 1 "+5V" V 3200 2850 50  0000 C CNN
F 2 "" H 3200 2650 50  0001 C CNN
F 3 "" H 3200 2650 50  0001 C CNN
	1    3200 2650
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR014
U 1 1 641834EE
P 3500 2650
F 0 "#PWR014" H 3500 2400 50  0001 C CNN
F 1 "GND" V 3500 2450 50  0000 C CNN
F 2 "" H 3500 2650 50  0001 C CNN
F 3 "" H 3500 2650 50  0001 C CNN
	1    3500 2650
	-1   0    0    -1  
$EndComp
$Comp
L 0_my_symbols:ESP32_DEV_0 U2
U 1 1 641F53B3
P 5100 4350
F 0 "U2" H 5828 4446 50  0000 L CNN
F 1 "ESP32_DEV_0" H 4700 4500 50  0000 L CNN
F 2 "0_my_footprints:myESP32DEV0" H 4550 4450 50  0001 C CNN
F 3 "" H 4550 4450 50  0001 C CNN
	1    5100 4350
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR028
U 1 1 64207DBE
P 4300 4800
F 0 "#PWR028" H 4300 4650 50  0001 C CNN
F 1 "+3.3V" V 4300 5050 50  0000 C CNN
F 2 "" H 4300 4800 50  0001 C CNN
F 3 "" H 4300 4800 50  0001 C CNN
	1    4300 4800
	-1   0    0    1   
$EndComp
Text GLabel 5300 4800 3    50   Input ~ 0
SDA
Text GLabel 5600 4800 3    50   Input ~ 0
SCL
Text GLabel 5700 4800 3    50   Input ~ 0
LEDS
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 6374560A
P 3650 1900
F 0 "J1" V 3850 1800 50  0000 L CNN
F 1 "power" V 3750 1750 50  0000 L CNN
F 2 "0_my_footprints:myJSTx04" H 3650 1900 50  0001 C CNN
F 3 "~" H 3650 1900 50  0001 C CNN
	1    3650 1900
	0    -1   -1   0   
$EndComp
Text GLabel 3750 2100 3    50   Input ~ 0
VBUS
Text GLabel 5300 3800 1    50   Input ~ 0
SBAT
Text GLabel 5500 3800 1    50   Input ~ 0
S5V
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
Text GLabel 6000 3000 0    50   Input ~ 0
VBUS
Text GLabel 6300 2700 2    50   Input ~ 0
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
Text GLabel 4350 3050 0    50   Input ~ 0
VBAT
Text GLabel 4600 2750 2    50   Input ~ 0
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
Text GLabel 3650 2100 3    50   Input ~ 0
VBAT
$Comp
L Transistor_FET:IRF9540N Q2
U 1 1 649281FE
P 9000 2450
F 0 "Q2" H 9204 2404 50  0000 L CNN
F 1 "IRF9540N" H 9204 2495 50  0000 L CNN
F 2 "0_my_footprints:myThreeLegged" H 9200 2375 50  0001 L CIN
F 3 "http://www.irf.com/product-info/datasheets/data/irf9540n.pdf" H 9000 2450 50  0001 L CNN
	1    9000 2450
	1    0    0    1   
$EndComp
$Comp
L Diode:1N47xxA D1
U 1 1 64928A41
P 9100 3000
F 0 "D1" V 9054 3080 50  0000 L CNN
F 1 "1N4741" V 9145 3080 50  0000 L CNN
F 2 "0_my_footprints:myZenerDiode" H 9100 2825 50  0001 C CNN
F 3 "https://www.vishay.com/docs/85816/1n4728a.pdf" H 9100 3000 50  0001 C CNN
	1    9100 3000
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 6492AC48
P 6450 1950
F 0 "R2" H 6250 2000 50  0000 L CNN
F 1 "10K" V 6450 1950 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6380 1950 50  0001 C CNN
F 3 "~" H 6450 1950 50  0001 C CNN
	1    6450 1950
	1    0    0    -1  
$EndComp
Text GLabel 6450 2100 3    50   Input ~ 0
SCL
$Comp
L power:+5V #PWR0105
U 1 1 64945E3E
P 6450 1800
F 0 "#PWR0105" H 6450 1650 50  0001 C CNN
F 1 "+5V" V 6450 2000 50  0000 C CNN
F 2 "" H 6450 1800 50  0001 C CNN
F 3 "" H 6450 1800 50  0001 C CNN
	1    6450 1800
	-1   0    0    -1  
$EndComp
$Comp
L Transistor_BJT:BC548 Q1
U 1 1 64954A41
P 8450 2850
F 0 "Q1" H 8641 2896 50  0000 L CNN
F 1 "BC548" H 8641 2805 50  0000 L CNN
F 2 "0_my_footprints:myTransistor" H 8650 2775 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC547.pdf" H 8450 2850 50  0001 L CNN
	1    8450 2850
	1    0    0    -1  
$EndComp
Text GLabel 7700 2850 0    50   Input ~ 0
PWM1
$Comp
L Device:R R8
U 1 1 6495E5F2
P 8000 2850
F 0 "R8" H 7800 2900 50  0000 L CNN
F 1 "10K" V 8000 2850 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7930 2850 50  0001 C CNN
F 3 "~" H 8000 2850 50  0001 C CNN
	1    8000 2850
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R9
U 1 1 649855B2
P 8550 2100
F 0 "R9" H 8350 2150 50  0000 L CNN
F 1 "10K" V 8550 2100 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 8480 2100 50  0001 C CNN
F 3 "~" H 8550 2100 50  0001 C CNN
	1    8550 2100
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR0106
U 1 1 6499107E
P 8550 1800
F 0 "#PWR0106" H 8550 1650 50  0001 C CNN
F 1 "+5V" V 8550 2000 50  0000 C CNN
F 2 "" H 8550 1800 50  0001 C CNN
F 3 "" H 8550 1800 50  0001 C CNN
	1    8550 1800
	-1   0    0    -1  
$EndComp
Text GLabel 9550 2750 2    50   Input ~ 0
COIL1+
$Comp
L power:GND #PWR0107
U 1 1 649971A5
P 8850 3400
F 0 "#PWR0107" H 8850 3150 50  0001 C CNN
F 1 "GND" V 8850 3200 50  0000 C CNN
F 2 "" H 8850 3400 50  0001 C CNN
F 3 "" H 8850 3400 50  0001 C CNN
	1    8850 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	8550 2250 8550 2450
Wire Wire Line
	8800 2450 8550 2450
Connection ~ 8550 2450
Wire Wire Line
	8550 2450 8550 2650
Wire Wire Line
	9100 2650 9100 2750
Wire Wire Line
	9100 2750 9550 2750
Connection ~ 9100 2750
Wire Wire Line
	9100 2750 9100 2850
Wire Wire Line
	9100 3150 9100 3250
Wire Wire Line
	9100 3250 8850 3250
Wire Wire Line
	8550 3250 8550 3050
Wire Wire Line
	8850 3250 8850 3400
Connection ~ 8850 3250
Wire Wire Line
	8850 3250 8550 3250
Wire Wire Line
	8550 1800 8550 1900
Wire Wire Line
	7700 2850 7850 2850
Wire Wire Line
	8150 2850 8250 2850
Wire Wire Line
	9100 2250 9100 1900
Wire Wire Line
	9100 1900 8550 1900
Connection ~ 8550 1900
Wire Wire Line
	8550 1900 8550 1950
Text GLabel 3450 4600 2    50   Input ~ 0
SW2
$Comp
L power:GND #PWR04
U 1 1 6493BD8E
P 3500 3150
F 0 "#PWR04" H 3500 2900 50  0001 C CNN
F 1 "GND" V 3500 2950 50  0000 C CNN
F 2 "" H 3500 3150 50  0001 C CNN
F 3 "" H 3500 3150 50  0001 C CNN
	1    3500 3150
	-1   0    0    -1  
$EndComp
$Comp
L Device:CP C2
U 1 1 6493BD82
P 3350 3150
F 0 "C2" H 3468 3196 50  0000 L CNN
F 1 "100 uf" H 3468 3105 50  0000 L CNN
F 2 "0_my_footprints:CP_my100uf" H 3388 3000 50  0001 C CNN
F 3 "~" H 3350 3150 50  0001 C CNN
	1    3350 3150
	0    -1   1    0   
$EndComp
$Comp
L power:+3.3V #PWR03
U 1 1 6494A255
P 3200 3150
F 0 "#PWR03" H 3200 3000 50  0001 C CNN
F 1 "+3.3V" V 3200 3400 50  0000 C CNN
F 2 "" H 3200 3150 50  0001 C CNN
F 3 "" H 3200 3150 50  0001 C CNN
	1    3200 3150
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR0108
U 1 1 6493C022
P 4300 3800
F 0 "#PWR0108" H 4300 3650 50  0001 C CNN
F 1 "+5V" V 4300 4000 50  0000 C CNN
F 2 "" H 4300 3800 50  0001 C CNN
F 3 "" H 4300 3800 50  0001 C CNN
	1    4300 3800
	-1   0    0    -1  
$EndComp
Text GLabel 5400 2100 3    50   Input ~ 0
COIL1+
$Comp
L Connector_Generic:Conn_01x02 J6
U 1 1 6494D066
P 5300 1900
F 0 "J6" V 5500 1800 50  0000 L CNN
F 1 "coil2" V 5400 1750 50  0000 L CNN
F 2 "0_my_footprints:myPinSocket_1x02" H 5300 1900 50  0001 C CNN
F 3 "~" H 5300 1900 50  0001 C CNN
	1    5300 1900
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR08
U 1 1 6494D06C
P 4950 2100
F 0 "#PWR08" H 4950 1850 50  0001 C CNN
F 1 "GND" V 4950 1900 50  0000 C CNN
F 2 "" H 4950 2100 50  0001 C CNN
F 3 "" H 4950 2100 50  0001 C CNN
	1    4950 2100
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J4
U 1 1 6494D072
P 4950 1900
F 0 "J4" V 5150 1800 50  0000 L CNN
F 1 "coil1" V 5050 1750 50  0000 L CNN
F 2 "0_my_footprints:myPinSocket_1x02" H 4950 1900 50  0001 C CNN
F 3 "~" H 4950 1900 50  0001 C CNN
	1    4950 1900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5050 2100 5300 2100
Text Notes 5050 1550 0    50   ~ 0
series
$Comp
L Transistor_FET:IRF9540N Q3
U 1 1 659B32AF
P 8100 4400
F 0 "Q3" H 8304 4354 50  0000 L CNN
F 1 "IRF9540N" H 8304 4445 50  0000 L CNN
F 2 "0_my_footprints:myThreeLegged" H 8300 4325 50  0001 L CIN
F 3 "http://www.irf.com/product-info/datasheets/data/irf9540n.pdf" H 8100 4400 50  0001 L CNN
	1    8100 4400
	1    0    0    1   
$EndComp
$Comp
L Diode:1N47xxA D2
U 1 1 659B32B5
P 8200 4950
F 0 "D2" V 8154 5030 50  0000 L CNN
F 1 "1N4741" V 8245 5030 50  0000 L CNN
F 2 "0_my_footprints:myZenerDiode" H 8200 4775 50  0001 C CNN
F 3 "https://www.vishay.com/docs/85816/1n4728a.pdf" H 8200 4950 50  0001 C CNN
	1    8200 4950
	0    1    1    0   
$EndComp
$Comp
L Transistor_BJT:BC548 Q4
U 1 1 659B32BB
P 7550 4800
F 0 "Q4" H 7741 4846 50  0000 L CNN
F 1 "BC548" H 7741 4755 50  0000 L CNN
F 2 "0_my_footprints:myTransistor" H 7750 4725 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC547.pdf" H 7550 4800 50  0001 L CNN
	1    7550 4800
	1    0    0    -1  
$EndComp
Text GLabel 6800 4800 0    50   Input ~ 0
PWM2
$Comp
L Device:R R11
U 1 1 659B32C2
P 7100 4800
F 0 "R11" H 6900 4850 50  0000 L CNN
F 1 "10K" V 7100 4800 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7030 4800 50  0001 C CNN
F 3 "~" H 7100 4800 50  0001 C CNN
	1    7100 4800
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R10
U 1 1 659B32C8
P 7650 4050
F 0 "R10" H 7450 4100 50  0000 L CNN
F 1 "10K" V 7650 4050 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7580 4050 50  0001 C CNN
F 3 "~" H 7650 4050 50  0001 C CNN
	1    7650 4050
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR0109
U 1 1 659B32CE
P 7650 3750
F 0 "#PWR0109" H 7650 3600 50  0001 C CNN
F 1 "+5V" V 7650 3950 50  0000 C CNN
F 2 "" H 7650 3750 50  0001 C CNN
F 3 "" H 7650 3750 50  0001 C CNN
	1    7650 3750
	-1   0    0    -1  
$EndComp
Text GLabel 8650 4700 2    50   Input ~ 0
COIL2+
$Comp
L power:GND #PWR0110
U 1 1 659B32D5
P 7950 5350
F 0 "#PWR0110" H 7950 5100 50  0001 C CNN
F 1 "GND" V 7950 5150 50  0000 C CNN
F 2 "" H 7950 5350 50  0001 C CNN
F 3 "" H 7950 5350 50  0001 C CNN
	1    7950 5350
	1    0    0    -1  
$EndComp
Wire Wire Line
	7650 4200 7650 4400
Wire Wire Line
	7900 4400 7650 4400
Connection ~ 7650 4400
Wire Wire Line
	7650 4400 7650 4600
Wire Wire Line
	8200 4600 8200 4700
Wire Wire Line
	8200 4700 8650 4700
Connection ~ 8200 4700
Wire Wire Line
	8200 4700 8200 4800
Wire Wire Line
	8200 5100 8200 5200
Wire Wire Line
	8200 5200 7950 5200
Wire Wire Line
	7650 5200 7650 5000
Wire Wire Line
	7950 5200 7950 5350
Connection ~ 7950 5200
Wire Wire Line
	7950 5200 7650 5200
Wire Wire Line
	7650 3750 7650 3850
Wire Wire Line
	6800 4800 6950 4800
Wire Wire Line
	7250 4800 7350 4800
Wire Wire Line
	8200 4200 8200 3850
Wire Wire Line
	8200 3850 7650 3850
Connection ~ 7650 3850
Wire Wire Line
	7650 3850 7650 3900
Text GLabel 4400 2100 3    50   Input ~ 0
COIL2+
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 659B6FF4
P 4300 1900
F 0 "J2" V 4500 1800 50  0000 L CNN
F 1 "coil3" V 4400 1750 50  0000 L CNN
F 2 "0_my_footprints:myPinSocket_1x02" H 4300 1900 50  0001 C CNN
F 3 "~" H 4300 1900 50  0001 C CNN
	1    4300 1900
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0111
U 1 1 659BAE5C
P 4300 2100
F 0 "#PWR0111" H 4300 1850 50  0001 C CNN
F 1 "GND" V 4300 1900 50  0000 C CNN
F 2 "" H 4300 2100 50  0001 C CNN
F 3 "" H 4300 2100 50  0001 C CNN
	1    4300 2100
	1    0    0    -1  
$EndComp
Text GLabel 4800 3800 1    50   Input ~ 0
PWM2
Text GLabel 5400 3800 1    50   Input ~ 0
SBUS
Text Notes 5800 3800 0    51   ~ 10
NOTE LACK OF 10uf\nBOOT CAPACITOR!!
$EndSCHEMATC
