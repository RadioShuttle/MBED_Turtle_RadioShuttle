/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "main.h"

#include "RadioShuttle.h"

#include "ProgramInterface.h"
#include "RadioBase.h"
#include "Blinky.h"

const char * Blinky::helpText = \
	"The red LED will blink every two seconds for 20 milli seconds\r\n" \
	"Turning on/off the LED2 is done via a LowPowerTimer wakeup which\r\n" \
	"ensures that during the entire idle time the MCU goes into deepsleep.\r\n" \
	"The LoRa radio is turned off to save energy\n\r";

Blinky::Blinky() : ProgramInterface(this)
{
	SetName('b', "Blinky", "example without LoRa");
}

void
Blinky::Startup(int argc, const char *argv[])
{
#ifdef FEATURE_LORA
	InitLoRaChipWithShutdown(); // not needed here, turn off to save energy
#endif
	led = new DigitalOut(LED2);
	timeout = new LowPowerTimeout;
	isActive = true;
	return;
}

void
Blinky::Shutdown(void)
{
	*led = 0;
	if (led)
		delete led;
	if (timeout) {
		timeout->detach();
		delete timeout;
	}
	isActive = false;
}

int
Blinky::ExecuteCommand(void)
{
	*led = 1; // is off, turn it on, restart timer in 20 milli seconds
	BlinkkUpdate();
	return E_TYPE_RESUME_LEAVE_COMMAND; // leave command loop
}

bool
Blinky::Service(uint32_t pendirqs)
{
	/*
	 * Update any processing here on user level
	 */
	if (pendirqs & INT_TIMEOUT) {
		if (*led) {	// is on, turn it off, restart timer in 2 sec
			*led = 0;
			timeout->attach_us(callback(this, &Blinky::BlinkkUpdate), blinkInterval_ms * 1000);
		} else {	// is off, turn it on, restart timer in 20 milli seconds
			*led = 1;
			timeout->attach_us(callback(this, &Blinky::BlinkkUpdate), blinkDuration_ms * 1000);
		}
		dprintf("%s LED: %s", this->GetName(), *led ? "ON" : "OFF");
		return true;
	}
	return false;
}

void
Blinky::BlinkkUpdate()
{
	/*
	 * This timer callback function is called on interrupt level
	 * it is not good to do here stuff, forward a signal
	 * that the Service() routing gets called on user level from the main loop.
	 */
	InterruptMSG(INT_TIMEOUT);
}
