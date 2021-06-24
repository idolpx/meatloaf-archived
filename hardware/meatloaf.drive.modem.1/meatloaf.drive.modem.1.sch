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
L power:GND #PWR0101
U 1 1 60C0085A
P 4150 3650
F 0 "#PWR0101" H 4150 3400 50  0001 C CNN
F 1 "GND" H 4155 3477 50  0000 C CNN
F 2 "" H 4150 3650 50  0001 C CNN
F 3 "" H 4150 3650 50  0001 C CNN
	1    4150 3650
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0102
U 1 1 60C0123E
P 4150 1200
F 0 "#PWR0102" H 4150 1050 50  0001 C CNN
F 1 "+3.3V" H 4165 1373 50  0000 C CNN
F 2 "" H 4150 1200 50  0001 C CNN
F 3 "" H 4150 1200 50  0001 C CNN
	1    4150 1200
	1    0    0    -1  
$EndComp
$Comp
L RF_Module:ESP-12F U1
U 1 1 60C027D6
P 4150 2500
F 0 "U1" H 4150 3481 50  0000 C CNN
F 1 "ESP-12F" H 4150 3390 50  0000 C CNN
F 2 "RF_Module:ESP-12E" H 4150 2500 50  0001 C CNN
F 3 "http://wiki.ai-thinker.com/_media/esp8266/esp8266_series_modules_user_manual_v1.1.pdf" H 3800 2600 50  0001 C CNN
	1    4150 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 3200 4150 3650
$Comp
L RETRO_COMMODORE64C:Conn_02x12_Edge J2
U 1 1 60C0B88A
P 7000 2900
F 0 "J2" H 7000 3667 50  0000 C CNN
F 1 "User Port" H 7000 3576 50  0000 C CNN
F 2 "" H 6950 2950 50  0001 C CNN
F 3 "" H 6950 2950 50  0001 C CNN
	1    7000 2900
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-6 J1
U 1 1 60C12451
P 6200 5250
F 0 "J1" H 6200 4769 50  0000 C CNN
F 1 "IEC SERIAL PORT" H 6200 4860 50  0000 C CNN
F 2 "" H 6200 5250 50  0001 C CNN
F 3 "http://www.mouser.com/ds/2/18/40_c091_abd_e-75918.pdf" H 6200 5250 50  0001 C CNN
	1    6200 5250
	-1   0    0    1   
$EndComp
Wire Wire Line
	4150 1200 4150 1350
Wire Wire Line
	7250 2650 7300 2650
Wire Wire Line
	7300 2650 7300 3700
Wire Wire Line
	7250 3150 7400 3150
Wire Wire Line
	7400 3150 7400 3800
Wire Wire Line
	6300 2300 4750 2300
Wire Wire Line
	7250 3350 7650 3350
Wire Wire Line
	7650 3350 7650 3900
Wire Wire Line
	7650 3900 6600 3900
Wire Wire Line
	6750 2750 6600 2750
Wire Wire Line
	6600 2750 6600 3900
$Comp
L power:GND #PWR0103
U 1 1 60C3D0E0
P 6750 6050
F 0 "#PWR0103" H 6750 5800 50  0001 C CNN
F 1 "GND" H 6755 5877 50  0000 C CNN
F 2 "" H 6750 6050 50  0001 C CNN
F 3 "" H 6750 6050 50  0001 C CNN
	1    6750 6050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6500 5350 6750 5350
Wire Wire Line
	6750 5350 6750 6050
Wire Wire Line
	4750 2700 5450 2700
Wire Wire Line
	5450 2700 5450 5700
Wire Wire Line
	6200 5700 6200 5550
Wire Wire Line
	4750 2500 5550 2500
Wire Wire Line
	5550 2500 5550 5350
Wire Wire Line
	4750 2600 5650 2600
Wire Wire Line
	5650 2600 5650 5150
Wire Wire Line
	6500 5150 6850 5150
Wire Wire Line
	6750 2950 6500 2950
Wire Wire Line
	5450 5700 6200 5700
Wire Wire Line
	5550 5350 5900 5350
Wire Wire Line
	5650 5150 5900 5150
Wire Wire Line
	6400 2400 6400 3700
Wire Wire Line
	6300 2300 6300 3800
Wire Wire Line
	4750 2200 6600 2200
Wire Wire Line
	6600 2200 6600 2750
Connection ~ 6600 2750
Wire Wire Line
	4750 2400 6400 2400
Wire Wire Line
	7250 2950 7650 2950
Wire Wire Line
	7650 2950 7650 1900
Wire Wire Line
	7650 1900 4750 1900
Wire Wire Line
	7250 2450 7300 2450
Connection ~ 7300 2450
Wire Wire Line
	6500 2950 6500 2000
Wire Wire Line
	6500 2000 4750 2000
Wire Wire Line
	6750 3450 6750 3600
Wire Wire Line
	6750 3600 7550 3600
Wire Wire Line
	7250 2350 7550 2350
Wire Wire Line
	7550 2350 7550 3450
Wire Wire Line
	6750 1800 7550 1800
Wire Wire Line
	7550 1800 7550 2200
Connection ~ 7550 2350
Wire Wire Line
	7300 2650 8050 2650
Connection ~ 7300 2650
Wire Wire Line
	7250 2750 8050 2750
Wire Wire Line
	7250 2850 8050 2850
Wire Wire Line
	7650 2950 8050 2950
Connection ~ 7650 2950
Wire Wire Line
	7250 3050 8050 3050
Wire Wire Line
	7400 3150 8050 3150
Connection ~ 7400 3150
Wire Wire Line
	7250 3250 7450 3250
Text Label 7850 2550 0    50   ~ 0
PB0
Text Label 7850 2650 0    50   ~ 0
PB1
Text Label 7850 2750 0    50   ~ 0
PB2
Text Label 7850 2850 0    50   ~ 0
PB3
Text Label 7850 2950 0    50   ~ 0
PB4
Text Label 7850 3050 0    50   ~ 0
PB5
Text Label 7850 3150 0    50   ~ 0
PB6
Text Label 7850 3250 0    50   ~ 0
PB7
Text Label 6550 5150 0    50   ~ 0
SRQ
Text Label 6550 5350 0    50   ~ 0
GND
Text Label 6000 5700 0    50   ~ 0
ATN
Text Label 5700 5350 0    50   ~ 0
CLK
Text Label 5700 5150 0    50   ~ 0
DATA
Text Label 6550 4950 0    50   ~ 0
RESET
Wire Wire Line
	4150 1350 4150 1700
Wire Wire Line
	7250 3450 7550 3450
Connection ~ 7550 3450
Wire Wire Line
	7550 3450 7550 3600
Wire Wire Line
	6750 2350 6750 1800
Text Label 7850 2000 0    50   ~ 0
TXD
Wire Wire Line
	8050 2100 6600 2100
Wire Wire Line
	6600 2100 6600 2200
Connection ~ 6600 2200
Text Label 7850 2100 0    50   ~ 0
RXD
Wire Wire Line
	8050 5000 7050 5000
Wire Wire Line
	7050 5000 7050 5700
Wire Wire Line
	7050 5700 6200 5700
Connection ~ 6200 5700
Wire Wire Line
	8050 5100 7150 5100
Wire Wire Line
	7150 5100 7150 5800
Wire Wire Line
	7150 5800 5550 5800
Wire Wire Line
	5550 5800 5550 5350
Connection ~ 5550 5350
Wire Wire Line
	8050 5200 7250 5200
Wire Wire Line
	7250 5200 7250 5900
Wire Wire Line
	7250 5900 5650 5900
Wire Wire Line
	5650 5900 5650 5150
Connection ~ 5650 5150
Wire Wire Line
	8050 5300 7650 5300
Wire Wire Line
	7650 5300 7650 5500
Wire Wire Line
	7650 5500 6850 5500
Wire Wire Line
	6850 5500 6850 5150
Wire Wire Line
	6950 4950 6200 4950
Wire Wire Line
	8050 5400 7750 5400
Wire Wire Line
	7750 5400 7750 5600
Wire Wire Line
	7750 5600 6950 5600
Wire Wire Line
	6950 5600 6950 4950
Text Label 7850 5000 0    50   ~ 0
ATN
Text Label 7850 5100 0    50   ~ 0
CLK
Text Label 7850 5200 0    50   ~ 0
DATA
Text Label 7850 5300 0    50   ~ 0
SRQ
Text Label 7850 5400 0    50   ~ 0
RESET
Wire Wire Line
	6300 3800 7400 3800
Wire Wire Line
	6400 3700 7300 3700
Wire Wire Line
	3550 2100 3450 2100
Wire Wire Line
	3450 2100 3450 1350
Wire Wire Line
	3450 1350 4150 1350
Wire Wire Line
	4750 2800 4850 2800
$Comp
L power:GND #PWR0104
U 1 1 60D95544
P 2000 6700
F 0 "#PWR0104" H 2000 6450 50  0001 C CNN
F 1 "GND" H 2005 6527 50  0000 C CNN
F 2 "" H 2000 6700 50  0001 C CNN
F 3 "" H 2000 6700 50  0001 C CNN
	1    2000 6700
	1    0    0    -1  
$EndComp
Connection ~ 4150 1350
$Comp
L LED:WS2812B D1
U 1 1 60D83541
P 1600 2150
F 0 "D1" H 1256 2196 50  0000 R CNN
F 1 "STATUS" H 1256 2105 50  0000 R CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 1650 1850 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 1700 1775 50  0001 L TNN
	1    1600 2150
	0    -1   1    0   
$EndComp
Wire Wire Line
	4750 2100 4850 2100
Wire Wire Line
	8050 2200 7550 2200
Text Label 7850 2200 0    50   ~ 0
GND
Connection ~ 3000 2700
Wire Wire Line
	3000 3100 3000 2700
Wire Wire Line
	2850 3100 3000 3100
Wire Wire Line
	2450 1900 2200 1900
Connection ~ 2450 1900
Wire Wire Line
	2200 1900 2200 1850
Connection ~ 2200 1900
Wire Wire Line
	2550 1900 2450 1900
Wire Wire Line
	2200 2300 2200 2250
Connection ~ 2200 2300
Wire Wire Line
	2450 2300 2550 2300
Connection ~ 2450 2300
Wire Wire Line
	2200 2300 2450 2300
Wire Wire Line
	2450 2700 2200 2700
Connection ~ 2450 2700
Wire Wire Line
	2200 2700 2200 2650
Connection ~ 2200 2700
Wire Wire Line
	2550 2700 2450 2700
Wire Wire Line
	2450 3100 2200 3100
Connection ~ 2450 3100
Wire Wire Line
	2200 3100 2200 3050
Wire Wire Line
	2550 3100 2450 3100
Wire Wire Line
	2200 2750 2200 2700
Wire Wire Line
	2200 2350 2200 2300
Wire Wire Line
	2200 1950 2200 1900
$Comp
L Device:R R4
U 1 1 60F24DEE
P 2200 2900
F 0 "R4" H 2130 2854 50  0000 R CNN
F 1 "R" H 2130 2945 50  0000 R CNN
F 2 "" V 2130 2900 50  0001 C CNN
F 3 "~" H 2200 2900 50  0001 C CNN
	1    2200 2900
	-1   0    0    1   
$EndComp
$Comp
L Switch:SW_MEC_5G SW4
U 1 1 60F247A8
P 2650 3100
F 0 "SW4" H 2650 3385 50  0000 C CNN
F 1 "MODE / RESET" H 2650 3294 50  0000 C CNN
F 2 "" H 2650 3300 50  0001 C CNN
F 3 "http://www.apem.com/int/index.php?controller=attachment&id_attachment=488" H 2650 3300 50  0001 C CNN
	1    2650 3100
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 60CA2A86
P 2200 1700
F 0 "R1" H 2130 1654 50  0000 R CNN
F 1 "R" H 2130 1745 50  0000 R CNN
F 2 "" V 2130 1700 50  0001 C CNN
F 3 "~" H 2200 1700 50  0001 C CNN
	1    2200 1700
	-1   0    0    1   
$EndComp
Wire Wire Line
	3000 2300 3000 2700
Connection ~ 3000 2300
Wire Wire Line
	2850 2700 3000 2700
Wire Wire Line
	2850 2300 3000 2300
Wire Wire Line
	3000 1900 2850 1900
Wire Wire Line
	3000 2300 3000 1900
$Comp
L Device:R R3
U 1 1 60CA3CB1
P 2200 2500
F 0 "R3" H 2130 2454 50  0000 R CNN
F 1 "R" H 2130 2545 50  0000 R CNN
F 2 "" V 2130 2500 50  0001 C CNN
F 3 "~" H 2200 2500 50  0001 C CNN
	1    2200 2500
	-1   0    0    1   
$EndComp
$Comp
L Device:R R2
U 1 1 60CA35A5
P 2200 2100
F 0 "R2" H 2130 2054 50  0000 R CNN
F 1 "R" H 2130 2145 50  0000 R CNN
F 2 "" V 2130 2100 50  0001 C CNN
F 3 "~" H 2200 2100 50  0001 C CNN
	1    2200 2100
	-1   0    0    1   
$EndComp
$Comp
L Switch:SW_MEC_5G SW3
U 1 1 60CA1F59
P 2650 2700
F 0 "SW3" H 2650 2985 50  0000 C CNN
F 1 "NEXT" H 2650 2894 50  0000 C CNN
F 2 "" H 2650 2900 50  0001 C CNN
F 3 "http://www.apem.com/int/index.php?controller=attachment&id_attachment=488" H 2650 2900 50  0001 C CNN
	1    2650 2700
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_MEC_5G SW2
U 1 1 60CA124A
P 2650 2300
F 0 "SW2" H 2650 2585 50  0000 C CNN
F 1 "SELECT" H 2650 2494 50  0000 C CNN
F 2 "" H 2650 2500 50  0001 C CNN
F 3 "http://www.apem.com/int/index.php?controller=attachment&id_attachment=488" H 2650 2500 50  0001 C CNN
	1    2650 2300
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_MEC_5G SW1
U 1 1 60C9FC9C
P 2650 1900
F 0 "SW1" H 2650 2185 50  0000 C CNN
F 1 "PREVIOUS" H 2650 2094 50  0000 C CNN
F 2 "" H 2650 2100 50  0001 C CNN
F 3 "http://www.apem.com/int/index.php?controller=attachment&id_attachment=488" H 2650 2100 50  0001 C CNN
	1    2650 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 2300 3000 2300
Wire Wire Line
	2200 1550 2200 1350
Wire Wire Line
	2200 1350 3450 1350
Connection ~ 3450 1350
$Comp
L Device:Buzzer BZ1
U 1 1 60C9C159
P 2550 3700
F 0 "BZ1" H 2555 3990 50  0000 C CNN
F 1 "Buzzer" H 2555 3899 50  0000 C CNN
F 2 "" V 2525 3800 50  0001 C CNN
F 3 "~" V 2525 3800 50  0001 C CNN
	1    2550 3700
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4850 2800 4850 3300
Wire Wire Line
	8050 2450 7300 2450
Text Label 7850 2450 0    50   ~ 0
FLAG2
Wire Wire Line
	6750 3050 6500 3050
Wire Wire Line
	6500 3050 6500 4000
Wire Wire Line
	6500 4000 7750 4000
Wire Wire Line
	7750 4000 7750 3350
Wire Wire Line
	7750 3350 8050 3350
Text Label 7850 3350 0    50   ~ 0
PC2
Wire Wire Line
	6750 2450 6650 2450
Wire Wire Line
	6650 2450 6650 1050
$Comp
L power:+5V #PWR?
U 1 1 61057287
P 6650 1050
F 0 "#PWR?" H 6650 900 50  0001 C CNN
F 1 "+5V" H 6665 1223 50  0000 C CNN
F 2 "" H 6650 1050 50  0001 C CNN
F 3 "" H 6650 1050 50  0001 C CNN
	1    6650 1050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 610706CA
P 7550 4200
F 0 "#PWR?" H 7550 3950 50  0001 C CNN
F 1 "GND" H 7555 4027 50  0000 C CNN
F 2 "" H 7550 4200 50  0001 C CNN
F 3 "" H 7550 4200 50  0001 C CNN
	1    7550 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 4200 7550 3600
Connection ~ 7550 3600
Text Notes 1050 1150 0    50   ~ 10
Controls\nAudio, Visual Indicators (optional)
Wire Notes Line
	5350 4650 5350 6300
Wire Notes Line
	5350 6300 8250 6300
Wire Notes Line
	8250 4650 5350 4650
Text Notes 5350 4600 0    50   ~ 10
IEC Serial Interface
Connection ~ 7550 2200
Wire Wire Line
	7550 2200 7550 2350
Wire Notes Line
	5150 1200 5150 4000
Wire Notes Line
	5150 4000 3350 4000
Wire Notes Line
	3350 4000 3350 1200
Wire Notes Line
	3350 1200 5150 1200
Text Notes 3350 1150 0    50   ~ 10
ESP8266 MCU
Wire Notes Line
	8200 1650 8200 4450
Wire Notes Line
	8200 4450 6200 4450
Wire Notes Line
	6200 4450 6200 1650
Text Notes 6200 800  0    50   ~ 10
User Port - Serial / Parallel Transfer Interface
Wire Wire Line
	6750 2850 6700 2850
Wire Wire Line
	6700 2850 6700 4100
Wire Wire Line
	6700 4100 7450 4100
Wire Wire Line
	7450 4100 7450 3250
Connection ~ 7450 3250
Wire Wire Line
	7450 3250 8050 3250
Text Notes 7050 7100 0    79   ~ 16
Meatloaf - A Commodore 64/128/VIC20/+4 \nWiFi Modem and IEC Serial Floppy Drive multi-device emulator \n\nhttps://github.com/idolpx/meatloaf
Wire Notes Line
	8250 6300 8250 4650
Text Notes 7400 7500 0    59   ~ 0
MEATLOAF CBM (NICE SLICE)
Text Notes 8150 7650 0    59   ~ 0
2020/06/08
Text Notes 10600 7650 0    59   ~ 0
1.00.005
Wire Wire Line
	1300 2150 1150 2150
Wire Wire Line
	1900 2150 2000 2150
$Comp
L LED:WS2812B D?
U 1 1 60C99683
P 1600 3100
F 0 "D?" H 1256 3146 50  0000 R CNN
F 1 "STATUS" H 1256 3055 50  0000 R CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 1650 2800 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 1700 2725 50  0001 L TNN
	1    1600 3100
	0    -1   1    0   
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60C9BA40
P 1600 4050
F 0 "D?" H 1256 4096 50  0000 R CNN
F 1 "STATUS" H 1256 4005 50  0000 R CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 1650 3750 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 1700 3675 50  0001 L TNN
	1    1600 4050
	0    -1   1    0   
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60C9D420
P 1600 5050
F 0 "D?" H 1256 5096 50  0000 R CNN
F 1 "STATUS" H 1256 5005 50  0000 R CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 1650 4750 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 1700 4675 50  0001 L TNN
	1    1600 5050
	0    -1   1    0   
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60CCD228
P 1600 6000
F 0 "D?" H 1256 6046 50  0000 R CNN
F 1 "STATUS" H 1256 5955 50  0000 R CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 1650 5700 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 1700 5625 50  0001 L TNN
	1    1600 6000
	0    -1   1    0   
$EndComp
Wire Wire Line
	1300 3100 1150 3100
Wire Wire Line
	1150 3100 1150 2150
Wire Wire Line
	1900 3100 2000 3100
Wire Wire Line
	2000 2150 2000 3100
Wire Wire Line
	1900 4050 2000 4050
Connection ~ 2000 3100
Wire Wire Line
	1900 5050 2000 5050
Connection ~ 2000 4050
Wire Wire Line
	1900 6000 2000 6000
Wire Wire Line
	2000 6000 2000 5050
Connection ~ 2000 5050
Wire Wire Line
	1300 6000 1150 6000
Wire Wire Line
	1150 6000 1150 5050
Connection ~ 1150 3100
Wire Wire Line
	1300 4050 1150 4050
Connection ~ 1150 4050
Wire Wire Line
	1150 4050 1150 3100
Wire Wire Line
	1300 5050 1150 5050
Connection ~ 1150 5050
Wire Wire Line
	1150 5050 1150 4050
Wire Wire Line
	1600 5700 1600 5350
Wire Wire Line
	1600 4750 1600 4350
Wire Wire Line
	1600 3750 1600 3400
Wire Wire Line
	1600 2800 1600 2450
Wire Wire Line
	2650 3600 2750 3600
Wire Wire Line
	2750 3600 2750 3300
Wire Wire Line
	2750 3300 4850 3300
Wire Wire Line
	2000 6700 2000 6000
Connection ~ 2000 6000
Wire Wire Line
	2000 3100 2000 4050
Wire Wire Line
	2650 4050 2000 4050
Wire Wire Line
	2650 3800 2650 4050
Wire Wire Line
	2000 4050 2000 5050
Wire Wire Line
	1150 2150 1150 1350
Wire Wire Line
	1150 1350 2200 1350
Connection ~ 1150 2150
Connection ~ 2200 1350
Wire Wire Line
	4850 2100 4850 1450
Wire Wire Line
	4850 1450 1600 1450
Wire Wire Line
	1600 1450 1600 1850
Wire Notes Line
	3100 1200 1050 1200
Wire Notes Line
	1050 1200 1050 6400
Wire Notes Line
	1050 6400 3100 6400
Wire Notes Line
	3100 6400 3100 1200
$Comp
L Transistor_FET:BSS84 Q?
U 1 1 60E3B961
P 7550 1500
F 0 "Q?" V 7892 1500 50  0000 C CNN
F 1 "BSS84" V 7801 1500 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 7750 1425 50  0001 L CIN
F 3 "http://assets.nexperia.com/documents/data-sheet/BSS84.pdf" H 7550 1500 50  0001 L CNN
	1    7550 1500
	0    1    -1   0   
$EndComp
Wire Wire Line
	7250 2550 7750 2550
Connection ~ 7750 2550
Wire Wire Line
	7750 2550 8050 2550
Wire Wire Line
	7750 1350 7750 1400
Connection ~ 7750 1400
Wire Wire Line
	7750 1400 7750 2000
$Comp
L Transistor_FET:BSS84 Q?
U 1 1 60E90E11
P 7050 1500
F 0 "Q?" V 7392 1500 50  0000 C CNN
F 1 "BSS84" V 7301 1500 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 7250 1425 50  0001 L CIN
F 3 "http://assets.nexperia.com/documents/data-sheet/BSS84.pdf" H 7050 1500 50  0001 L CNN
	1    7050 1500
	0    1    -1   0   
$EndComp
Wire Wire Line
	7250 1400 7300 1400
Wire Wire Line
	7300 1400 7300 2450
Connection ~ 7300 1400
Wire Wire Line
	7300 1400 7350 1400
Wire Wire Line
	7550 1700 7050 1700
Wire Wire Line
	6850 1400 6500 1400
Wire Wire Line
	6500 1400 6500 2000
Connection ~ 6500 2000
Wire Wire Line
	8050 2000 7750 2000
Connection ~ 7750 2000
Wire Wire Line
	7750 2000 7750 2550
$EndSCHEMATC
