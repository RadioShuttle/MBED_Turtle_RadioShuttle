/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#ifndef __PROGRAMINTERFACE_H__
#define __PROGRAMINTERFACE_H__

#include <vector>

class ProgramInterface;

extern std::vector<ProgramInterface *> _progs;
extern const char *defCmdLine;

class ProgramInterface {
public:
	ProgramInterface(ProgramInterface *proc) {
		 isActive = false;
		 _progs.push_back(proc);
	};
	
	/*
	 * fucntions which must be implemented.
	 * init everything needed in Startup
	 */
	virtual void Startup(int argc, const char *argv[]) = 0;
	
	/*
	 * de-init everything, release all resoruces, timers, IOs
	 */
	virtual void Shutdown(void) = 0;
	
	/*
	 * Service gets called by the system main loop on activity
	 * The pending IRQ signal which interrupts are pending (e.g. Timer, LoRa, ...)
	 * In deepsleep Service gets only called after an Interrupt gets issued.
	 * Use LowPowerTimer, LowPowerTimeout, InterruptIn (GPIO) to enable deepsleep
	 * and to wakeup from deeplseep.
	 * Regular Timer, Timeout will not enter into deepsleep!
	 */
	virtual bool Service(uint32_t pendirqs) = 0;

	/*
	 * Execute the command (called after Startup), there are multiple return options:
	 * E_TYPE_QUIT: 				leave the program, continue with command interreter
	 * E_TYPE_RESUME_WITH_COMMAND:	continue program, continue with command interreter
	 * E_TYPE_RESUME_LEAVE_COMMAND: continue program, leave command interreter
	 *                              a reset is required to return to the command interreter
	 */
	virtual int ExecuteCommand(void ) { return E_TYPE_QUIT; };

	virtual const char *GetHelpText(void) { return "sorry, no help available\r\n"; };

	void SetName(char s, const char *n, const char *d) {
		this->shortkey = s;
		this->name = n;
		this->desc = d;
	};
	
	const char *GetName() { return name; };
	const char *GetDescription() { return desc; };
	char GetShortkey() { return shortkey; };
	
	bool isActive;

	enum ExecuteType {
		E_TYPE_QUIT = 1,
		E_TYPE_RESUME_CONT_COMMAND,
		E_TYPE_RESUME_LEAVE_COMMAND,
	};
private:
	const char *name;
	const char *desc;
	char shortkey;
};


#endif // __PROGRAMINTERFACE_H__
