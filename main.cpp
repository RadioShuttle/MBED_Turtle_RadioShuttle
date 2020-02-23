/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

 /*
  * TODO:
  * Compiler Date/Time is not set correctly on startup using gcc
  * USB Serial block deepsleep
  */
#include "main.h"
#include "RadioTest.h"

DigitalOut statusLED(LED);
// DigitalOut redLED(LED2);
InterruptIn buttonIntr(USER_BUTTON);
volatile int pressedCount;

void switchInput(void) {
	InterruptMSG(INT_BUTTON1);
}

void timerUpdate(void) {
	static LowPowerTimeout timeout;
	if (redLED == 0)
		timeout.attach_us(&timerUpdate, 20000); // setup to call timerUpdate after 20 millis
	else
    	timeout.attach_us(&timerUpdate, 2000000); // setup to call timerUpdate after 2 seconds

	InterruptMSG(INT_TIMEOUT);
}



int main() {
    /*
     * inits the Serial or USBSerial when available (230400 baud).
     * If the serial uart is not is not connected it swiches to USB Serial
     * blinking LED means USBSerial detected, waiting for a connect.
     * It waits up to 30 seconds for a USB terminal connections 
     */
    InitSerial(30*1000, &statusLED, &buttonIntr);
	RunStartup();
	dprintf("Welcome to RadioShuttle v%d.%d", RS_MAJOR, RS_MINOR);
    timerUpdate(); // start timer for status blinked, can be disalbed to save energy
#if defined (USER_BUTTON_RISE) // attach switchInput function to the rising or falling edge
	buttonIntr.rise(&switchInput);
#else
	buttonIntr.fall(&switchInput);
#endif
	
	RunCommands(10000);	// check 10 secs for any commands
	
#ifdef FEATURE_LORA
    InitRadio();
#endif

#if 0
	DigitalOut espVDD(ESP_POWER_SW);
	// espVDD = ESP_POWER_OFF;
	espVDD = ESP_POWER_ON;

	// sleep_manager_unlock_deep_sleep_internal(); // from SerialBase::attach
	DigitalIn espEN(PB_10);
	
	DigitalIn t1(LORA_SPI_MOSI);
	DigitalIn t2(LORA_SPI_MISO);
	DigitalIn t3(LORA_SPI_SCLK);

	// DigitalIn io26(PB_4);
#endif

	/*
	 * Main event loop, process interrupts and goes to sleep when idle.
	 * the green statusLED indicates CPU activity
	 * the red redLED indicates that low power timerUpdate function is running.
	 */
	while(true) {
        while ((readPendingInterrupts() == 0)) {
			statusLED = 0;
			sleep();
			statusLED = 1;
        }

        uint32_t pendirqs = readclrPendingInterrupts();
        if (pendirqs & INT_BUTTON1) {
#ifdef FEATURE_LORA
			statusLED = !statusLED;
            RadioUpdate(true); // pass the pressed user button to RadioShuttle
#endif
		}
        if (pendirqs & INT_LORA) {
#ifdef FEATURE_LORA
            RadioUpdate(false);
#endif
        }
        if (pendirqs & INT_TIMEOUT) {
			redLED = ! redLED;
		}
	}
}
