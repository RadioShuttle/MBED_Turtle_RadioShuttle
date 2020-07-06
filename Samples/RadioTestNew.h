/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */


class RadioTestNew : public RadioBase, public ProgramInterface {
public:
	RadioTestNew();
	void Startup(int argc, const char *argv[]);
	void Shutdown(void);
	bool Service(uint32_t pendirqs);
	int ExecuteCommand(void);
	const char *GetHelpText(void) { return helpText; };

	void MessageRecvHandler(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length);
	bool RadioUpdate(bool keyPressed);
private:
	static const char *helpText;
	static const int SENSOR_APP_ID = 1;
};
