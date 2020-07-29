/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "main.h"

#include "RadioShuttle.h"
#include "RadioBase.h"
#include "ProgramInterface.h"

#include "Blinky.h"
#include "NVPEditor.h"
#include "LoRaPingPong.h"
#include "RadioContinuousTX.h"
#include "RadioTestNew.h"
#include "PanicButton.h"
#include "RemoteRelay.h"
#include "RemoteTempSensor.h"
#include "CPUBench.h"

std::vector<ProgramInterface *> _progs;	// list off all apps.

const char *defCmdLine = "n"; // default prog

static CPUBench bench;

#ifdef FEATURE_NVPROPERTYEDITOR
static NVPEditor myEditor;
#endif

#ifdef FEATURE_LORA_PING_PONG
static LoRaPingPong myPingPong;
#endif

#ifdef FEATURE_LORA
static RadioContinuousTX myLoRaTXwave;
static Blinky myblinky;
static RadioTestNew myRadioTest;
static PanicButton myPanicButton;
static RemoteTempSensor myTempSensor;
static RemoteRelay myRelay( {PB_4, PB_5, PB_6, PB_7} );
#endif
