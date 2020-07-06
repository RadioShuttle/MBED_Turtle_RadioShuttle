/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "main.h"

#include "ProgramInterface.h"
#include "LoRaPingPong.h"

#include "GenericPingPong.h"

LoRaPingPong::LoRaPingPong() : ProgramInterface(this)
{
	SetName('l', "LoRaPingPong", "raw LoRa ping pong example");
}

void
LoRaPingPong::Startup(int argc, const char *argv[])
{
	isActive = true;
}

void
LoRaPingPong::Shutdown(void)
{
	isActive = false;
}

int
LoRaPingPong::ExecuteCommand(void)
{
	SX1276PingPong();
	return E_TYPE_QUIT; // stay in command line.
}

bool
LoRaPingPong::Service(uint32_t pendirqs)
{
	return false;
}

