/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

class Blinky : public RadioBase, public ProgramInterface {
public:
	Blinky();
	void Startup(int argc, const char *argv[]);
	void Shutdown(void);
	bool Service(uint32_t pendirqs);
	int ExecuteCommand(void);
	const char *GetHelpText(void) { return helpText; };
private:
	static const char *helpText;
	DigitalOut *led;
	LowPowerTimeout *timeout;
	void BlinkkUpdate();
	static const int blinkDuration_ms = 20;
	static const int blinkInterval_ms = 2000;
};
