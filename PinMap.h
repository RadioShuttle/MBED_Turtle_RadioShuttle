/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#define RS_MAJOR    3
#define RS_MINOR    3
#define MAJOR_VERSION	1
#define MINOR_VERSION	1

#ifdef TARGET_STM32L432KC
 #define HELTECL432_REV1
 #define FEATURE_LORA
 #define FEATURE_RADIOTESTSAMPLE
 #define FEATURE_LORA_PING_PONG
 #define FEATURE_USBSERIAL
 #define FEATURE_NVPROPERTY
 #define FEATURE_NVPROPERTYEDITOR
 #define FEATURE_SI7021
 #define FEATURE_SSD1306
#endif

#ifdef TARGET_STM32L433xC
  #define ECOPOWER_REV2
  #define FEATURE_LORA
  #define FEATURE_RADIOTESTSAMPLE
  #define FEATURE_LORA_PING_PONG
  #define FEATURE_USBSERIAL
  #define FEATURE_NVPROPERTY
  #define FEATURE_NVPROPERTYEDITOR
  #define FEATURE_SI7021
  #define FEATURE_SSD1306
#endif

#ifdef HELTECL432_REV1
#define USER_BUTTON     (PinName) 0x73 // PH_3 boot pin
#define USER_BUTTON_RISE
#define STATUS_LED      PB_1	// green LED
#define LED2            PB_0	// red LED
#define LED				STATUS_LED
#define BATPOWER_EN		PA_8	// high indicates power source is battery, removable bridge
#define BATPOWER_EXT	1

#define SPI_LSM_MOSI    PA_7
#define SPI_LSM_MISO    PA_6
#define SPI_LSM_SCLK    PA_5
#define CS_FLASH        NC

#define EXT_POWER_SW	PA_3	// VEXT on/off
#define EXT_POWER_ON    0
#define EXT_POWER_OFF   1

#define LORA_SPI_MOSI   SPI_LSM_MOSI
#define LORA_SPI_MISO   SPI_LSM_MISO
#define LORA_SPI_SCLK   SPI_LSM_SCLK
#define LORA_CS			PA_4
#define LORA_RESET      PA_1
#define LORA_DIO0       PA_0	// used for Rx, Tx Interrupt
#define LORA_DIO1       NC 		// PB_6Fifo Level/Full, RxTimeout/Cad Detection Interrupt, unused in RadioShuttle
#define LORA_DIO2      	NC		// FhssChangeChannel when FreqHop is on, unused in RadioShuttle
#define LORA_DIO3       NC		// optionally Cad Detection in RS_Node_Offline/Checking mode
#define LORA_DIO4       NC		// FSK mode preamble detected, unused in RadioShuttle
#define LORA_DIO5       NC		// FSK mode ready / ClockOut, unused in RadioShuttle
#define LORA_ANT_PWR	EXT_POWER_SW // the analog switch is getting turned off go save energy

/* this following are available pins for custom use */
#define P_SWLCK		PA_14	// available only when no debugger is being used.
#define P_SWDIO		PA_13	// available only when no debugger is being used
#define P_SWO		PB_3	// available only when no SWO debugging output is being used
#define P_PA_2_TX	PA_2	// available only when no debug serial console is being used
#define P_PA_15_RX	PA_15	// available only when no debug serial console is being used
#define	P_PA_9_SCL	PA_9	// SCL includes 10k removable hardware pullup
#define P_PA_10_SDA	PA_10	// SCL includes 10k removable hardware pullup
#define	P_PB_4		PB_4
#define P_PB_5		PB_5
#define P_PB_6		PB_6
#define	P_PB_7		PB_7

#define SI7021_SDA	P_PA_10_SDA
#define SI7021_SCL	P_PA_9_SCL
#define DISPLAY_ADDRESS 0x3c
#define DISPLAY_SDA P_PA_10_SDA
#define DISPLAY_SCL P_PA_9_SCL

#define WatchDogUpdate() void()


#elif defined(ECOPOWER_REV2)

#define USER_BUTTON     PH_3 	// boot pin
#define USER_BUTTON_RISE
#define STATUS_LED      PC_13	// yellow LED
#define LED2            PB_11	// green LED
#define LED				STATUS_LED
#define BATPOWER_EN		PA_0	// high indicates power source is battery, removable bridge
#define BATPOWER_EXT	0		// 0 on eexterna power, 1 on battery power

#define EXT_POWER_SW	PA_XXX	// VEXT on/off
#define EXT_POWER_ON    0
#define EXT_POWER_OFF   1

#define ESP_POWER_SW	PB_15
#define ESP_POWER_ON    0
#define ESP_POWER_OFF   1


#define LORA_SPI_MOSI   PA_7
#define LORA_SPI_MISO   PA_6
#define LORA_SPI_SCLK   PA_5
#define LORA_CS			PA_8
#define LORA_RESET      PA_1
#define LORA_DIO0       PA_15	// used for Rx, Tx Interrupt
#define LORA_DIO1       NC 		// PB_6Fifo Level/Full, RxTimeout/Cad Detection Interrupt, unused in RadioShuttle
#define LORA_DIO2      	NC		// FhssChangeChannel when FreqHop is on, unused in RadioShuttle
#define LORA_DIO3       NC		// optionally Cad Detection in RS_Node_Offline/Checking mode
#define LORA_DIO4       NC		// FSK mode preamble detected, unused in RadioShuttle
#define LORA_DIO5       NC		// FSK mode ready / ClockOut, unused in RadioShuttle

#define SI7021_SDA	PA_10
#define SI7021_SCL	PA_9
#define DISPLAY_ADDRESS 0x3c
#define DISPLAY_SDA PA_10
#define DISPLAY_SCL PA_9

#define WatchDogUpdate() void()

#else
 #error "unknown board"
#endif
