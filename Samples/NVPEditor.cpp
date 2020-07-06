/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "main.h"

#include "ProgramInterface.h"
#include "NVPEditor.h"

extern void NVPropertyEditor(void);

NVPEditor::NVPEditor() : ProgramInterface(this)
{
	SetName('p', "Properties", "non volatile settings editor");
}

void
NVPEditor::Startup(int argc, const char *argv[])
{
	isActive = true;
}

void
NVPEditor::Shutdown(void)
{
	isActive = false;
}

int
NVPEditor::ExecuteCommand(void)
{
	NVPropertyEditor();
	return E_TYPE_QUIT; // stay in command line.
}

bool
NVPEditor::Service(uint32_t pendirqs)
{
	return false;
}

