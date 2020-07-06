/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

 /*
  * TODO:
  * Compiler Date/Time is not set correctly on startup using gcc
  */
#include "main.h"
#include "RadioTest.h"
#include <vector>
#include "ProgramInterface.h"

extern void RunCommands(int timeout_ms);

DigitalOut statusLED(LED);
DigitalOut redLED(LED2);
InterruptIn buttonIntr(USER_BUTTON);

void switchInput(void) {
	InterruptMSG(INT_BUTTON1);
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
	
#if defined (USER_BUTTON_RISE) // attach switchInput function to the rising or falling edge
	buttonIntr.rise(&switchInput);
#else
	buttonIntr.fall(&switchInput);
#endif

	 RunCommands(10000);	// check 10 secs for any commands

	/*
	 * Main event loop, process interrupts and goes to sleep when idle.
	 * the green statusLED indicates CPU activity
	 */
	while(true) {
        while ((readPendingInterrupts() == 0)) {
			statusLED = 0;
			sleep();
			statusLED = 1;
        }

        uint32_t pendirqs = readclrPendingInterrupts();
		std::vector<ProgramInterface *>::iterator re;
		for(re = _progs.begin(); re != _progs.end(); re++) {
			 if ((*re)->isActive) {
			 	(*re)->Service(pendirqs);
			}
		}
	}
}
