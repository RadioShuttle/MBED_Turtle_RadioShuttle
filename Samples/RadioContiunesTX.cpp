/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "main.h"

#include "RadioShuttle.h"
#include "RadioStatus.h"
#include "RadioSecurity.h"

#include "ProgramInterface.h"
#include "RadioBase.h"
#include "RadioContiunesTX.h"

RadioContiunesTX::RadioContiunesTX() : ProgramInterface(this)
{
	SetName('w', "RadioContiunesTX", "LoRa radio continues wave TX test");
}

void
RadioContiunesTX::Startup(int argc, const char *argv[])
{
	isActive = true;
}

void
RadioContiunesTX::Shutdown(void)
{
	isActive = false;
}

int
RadioContiunesTX::ExecuteCommand(void)
{
	RadioContinuesTX();
	return E_TYPE_QUIT; // leave command loop
}

bool
RadioContiunesTX::Service(uint32_t pendirqs)
{
	return false;
}

void
RadioContiunesTX::BlinkkUpdate()
{
}
