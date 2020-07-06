/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */


class LoRaPingPong : public ProgramInterface {
public:
	LoRaPingPong();
	void Startup(int argc, const char *argv[]);
	void Shutdown(void);
	bool Service(uint32_t pendirqs);
	int ExecuteCommand(void);
};
