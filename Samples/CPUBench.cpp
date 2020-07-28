/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "main.h"

#include "RadioShuttle.h"

#include "ProgramInterface.h"
#include "RadioBase.h"
#include "CPUBench.h"

const char * CPUBench::helpText = \
	"A simple CPU benchmark for integer and float performance\r\n" \
	"The LoRa radio is turned off to save energy\n\r";

typedef void (*CPUProgressFunc)(char c);


void BenchProgress(char c)
{
  static int count;

  if (c == '\n') {
  	rprintf("\r\n");
  } else {
  	rprintf("%c", c);
  	if (usb)
  		usb->flush();
  }
  if (++count % 60 == 0)
    rprintf("\r\n");
}

int getMillis(void)
{
	static Timer *t = NULL;
	if (!t) {
		t = new Timer();
		t->start();
	}
	return t->read_ms();
}


/*
 * Determines "intuitive GFLOP" rating.
 * That is, computes what most people think of as "GFLOP" rating,
 * not actual LINPACK-style benchmark.
 * Based on code originally from Robert G. Brown, rgb@phy.duke.edu
 * (FLOP is defined as native-size floating point ops per second)
 * Returns Floating Point Operations per Cycle
 */

template <class T>
int MegaOperations(T, float *millions, int *m_secs, CPUProgressFunc m_progresscb, int cpuId)
{
	long SIZE = 1000; /* sizeof(T) * SIZE should fit in L1 cache */
	long COUNT = 20000;
	T *x;

	if ((x = (T *)malloc(sizeof(T) * SIZE)) == NULL)
		return -1;

	for(int i = 0; i < SIZE; i++) /* Initialize values, and move data into L1 cache */
		x[i] = 1;

	int start = getMillis();

	for(int k = 1; k <= COUNT; k++) {
		for(int i = 0; i < SIZE; i++) {
			// One Addition, One Subtraction, One Multiplcation, One Division
			x[i] = ((T)1 + x[i]) * ((T)2 - x[i]) / (T)2;
		}
		if ((k & 0xff) == 0) // every 256
			if (m_progresscb)
				m_progresscb('0' + cpuId);
	}
	m_progresscb('\n');
  
	int ms = getMillis() - start;
 
	float rate_ms = (float)(4 * SIZE * COUNT) / ms;

	free(x);

	*m_secs = ms;
	*millions =  rate_ms / 1000.0; // convert ms to usecs (millions)
    wait_ms(10 * cpuId); // to avoid concurrent prints of results

	return 0;
}


CPUBench::CPUBench() : ProgramInterface(this)
{
	SetName('B', "CPUBench", "example CPU benchmark");
	timeout = NULL;
	led = NULL;
}

void
CPUBench::Startup(int argc, const char *argv[])
{
#ifdef FEATURE_LORA
	InitLoRaChipWithShutdown(); // not needed here, turn off to save energy
#endif
	isActive = true;
	
	timeout = new Timeout();
	led = new DigitalOut(LED2);
	DigitalOut greenled(LED);
	greenled = 0;
	BlinkkUpdate();
	return;
}

void
CPUBench::Shutdown(void)
{
	if (timeout)
		delete timeout;
	if (led)
		delete led;
	isActive = false;
}

int
CPUBench::ExecuteCommand(void)
{
	const char *name = "CPU0";
	float f = 0;
	int m_secs = 0;
	int cpuID = 0;

	MegaOperations((int)1, &f, &m_secs, &BenchProgress, cpuID);
	dprintf("%s: Mega(Int) = %.2f/sec (%.3f secs)", name, f, (float)m_secs/1000.0);

	MegaOperations((int64_t)1, &f, &m_secs, &BenchProgress, cpuID);
	dprintf("%s: Mega(int64_t) = %.2f/sec (%.3f secs)", name, f, (float)m_secs/1000.0);

	MegaOperations((float)1, &f, &m_secs, &BenchProgress, cpuID);
	dprintf("%s: Mega(Float) = %.2f/sec (%.3f secs)", name, f, (float)m_secs/1000.0);

	MegaOperations((double)1, &f, &m_secs, &BenchProgress, cpuID);
	dprintf("%s: Mega(Double) = %.2f/sec (%.3f secs)", name, f, (float)m_secs/1000.0);

	timeout->detach();
	*led = 0;
	return E_TYPE_RESUME_LEAVE_COMMAND; // leave command loop
}

bool
CPUBench::Service(uint32_t pendirqs)
{
	/*
	 * Update any processing here on user level
	 */
	if (pendirqs & INT_TIMEOUT) {
	}
	return false;
}


void
CPUBench::BlinkkUpdate(void)
{
	timeout->attach_us(callback(this, &CPUBench::BlinkkUpdate), blinkInterval_ms * 1000);

	*led = !*led;
}
