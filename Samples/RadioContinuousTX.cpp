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
#include "RadioContinuousTX.h"

RadioContinuousTX::RadioContinuousTX() : ProgramInterface(this)
{
	SetName('w', "RadioContinuousTX", "LoRa radio continues wave TX test");
}

void
RadioContinuousTX::Startup(int argc, const char *argv[])
{
	isActive = true;
}

void
RadioContinuousTX::Shutdown(void)
{
	isActive = false;
}

int
RadioContinuousTX::ExecuteCommand(void)
{
	RadioContinuousTX();
	return E_TYPE_QUIT; // leave command loop
}

bool
RadioContinuousTX::Service(uint32_t pendirqs)
{
	return false;
}

void
RadioContinuousTX::BlinkkUpdate()
{
}
