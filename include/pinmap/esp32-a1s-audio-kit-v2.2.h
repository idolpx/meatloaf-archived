//
// ESP32-A1S Audio Development Kit
//
// https://github.com/marcel-licence/AC101
// https://forums.slimdevices.com/showthread.php?113804-ESP32-A1S-Audio-Kit-V2-2
// https://docs.ai-thinker.com/_media/esp32-audio-kit_v2.2_sch.pdf
// https://github.com/donny681/esp-adf
// 
//
// amplifier: GPIO21
// key2: GPIO13, key3: GPIO19, key4: GPIO23, key5: GPIO18, key6: GPIO5 (to be confirmed with dip switches)
// key1: not sure, using GPIO36 in a matrix
// jack insertion: GPIO39 (inserted low)
// D4 -> GPIO22 used for green LED (active low)
// D5 -> GPIO19 (muxed with key3)
// The IO connector also brings GPIO5, GPIO18, GPIO19, GPIO21, GPIO22 and GPIO23 (don't forget it's muxed with keys!)
// The JTAG connector uses GPIO 12, 13, 14 and 15 (see dip switch) but these are also used for SD-card (and GPIO13 is key2 as well)


#ifndef PINMAP_ESP32_A1S_AUDIO_KIT_V2_2
#define PINMAP_ESP32_A1S_AUDIO_KIT_V2_2

#ifdef PINMAP_A1S_AUDIOKIT

/* SD Card */
// pins 12-15 are used to interface with the JTAG debugger
// so leave them alone if we're using JTAG
#ifndef JTAG
#define PIN_CARD_DETECT 34 // fnSystem.h
#define PIN_CARD_DETECT_FIX 15 // fnSystem.h
#endif

#define PIN_SD_HOST_CS GPIO_NUM_13
#define PIN_SD_HOST_MISO GPIO_NUM_2
#define PIN_SD_HOST_MOSI GPIO_NUM_15
#define PIN_SD_HOST_SCK GPIO_NUM_14


/* UART */
#define PIN_UART0_RX 3  // fnUART.cpp
#define PIN_UART0_TX 1
#define PIN_UART1_RX 9
#define PIN_UART1_TX 10
#define PIN_UART2_RX 33
#define PIN_UART2_TX 21

/* Buttons */
#define PIN_BUTTON_A 36 // keys.cpp
#define PIN_BUTTON_B 36
#define PIN_BUTTON_C 36

/* LEDs */
#define PIN_LED_WIFI 22 // led.cpp
#define PIN_LED_BUS  22
#define PIN_LED_BT   22

/* Audio Output */
#define PIN_DAC1 25 // samlib.h

/* Commodore IEC Pins */
// CLK & DATA lines in/out are split between two pins
//#define IEC_SPLIT_LINES

// Line values are inverted (7406 Hex Inverter Buffer)
//#define IEC_INVERTED_LINES

// Reset line is available
#define IEC_HAS_RESET

// CBM IEC Serial Port
#define PIN_IEC_ATN         GPIO_NUM_21
#define PIN_IEC_SRQ			GPIO_NUM_23
#define PIN_IEC_RESET       GPIO_NUM_18

// IEC_SPLIT_LINES
#ifndef IEC_SPLIT_LINES
// NOT SPLIT - Bidirectional Lines
#define PIN_IEC_CLK_IN		GPIO_NUM_22
#define PIN_IEC_CLK_OUT	    GPIO_NUM_22
#define PIN_IEC_DATA_IN    	GPIO_NUM_19
#define PIN_IEC_DATA_OUT   	GPIO_NUM_19
#else
// SPLIT - Seperate Input & Output lines
#define PIN_IEC_CLK_IN		GPIO_NUM_22
#define PIN_IEC_CLK_OUT	    GPIO_NUM_22
#define PIN_IEC_DATA_IN    	GPIO_NUM_19
#define PIN_IEC_DATA_OUT   	GPIO_NUM_19
#endif


/* Modem/Parallel Switch */
#define PIN_MDMPAR_SW1       2  // High = Modem enabled
#define PIN_MDMPAR_SW2       15 // High = UP9600 enabled

/* I2C GPIO Expander */
#define PIN_GPIOX_SDA         GPIO_NUM_21
#define PIN_GPIOX_SCL         GPIO_NUM_22
#define PIN_GPIOX_INT         GPIO_NUM_34
//#define GPIOX_ADDRESS     0x20  // PCF8575
#define GPIOX_ADDRESS     0x24  // PCA9673
//#define GPIOX_SPEED       400   // PCF8575 - 400Khz
#define GPIOX_SPEED       1000  // PCA9673 - 1000Khz / 1Mhz

#endif // PINMAP_A1S_AUDIOKIT
#endif // PINMAP_ESP32_A1S_AUDIO_KIT_V2_2