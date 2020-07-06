/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */


class PanicButton : public RadioBase, public ProgramInterface {
public:
	PanicButton();
	void Startup(int argc, const char *argv[]);
	void Shutdown(void);
	bool Service(uint32_t pendirqs);
	int ExecuteCommand(void);
	const char *GetHelpText(void) { return helpText; };

	void setAlarmSubmitConfirmed(void) { alarmSubmitConfirmed = true; };
	void MessageRecvHandler(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length);

private:
	static const char *helpText;
	static const int SENSOR_APP_ID = 1;
	bool RunSensorUpdate(bool userKeyPressed);
	void RunPanicUpdate(void);
	void PanicTimeoutFunc(void);
	void StatusUpdate();
	void BuzzerSoundFunc();
	DigitalOut *buzzer;
	LowPowerTimeout *timeout;	// this needs to be low power to allow deepsleep
	Timeout buzzerTimer;
	Timeout panicTimer;
	Timer *counter;
	bool panicTimerExired;
	bool alarmSubmitConfirmed;
	bool useASCIIMSG;
	char asciiTempData[8];
	
	enum msfFlags {
		PANIC_FLAGS_ALARM = 0x01,	// the alarm has been issued
		PANIC_FLAGS_INFO  = 0x02,    // a status message e.g. for the battery voltage, keep alive
		SIMPLE_PANIC_DATA_MAGIC = 0xaa,
	};

	struct simplePanicData {
	  uint8_t magic;        // a magic to allow different message versions
	  uint8_t flags;
	  uint16_t battery_mV;  // the float battery * 100 to avoid floats
	} tempData;
	
	/*
	 * parameters
	 */
	static const int BUZZER_SOUND_INTERVAL_DIAL = 50;	// 50 ms
	static const int BUZZER_SOUND_INTERVAL_FINISH = 35;	// 35 ms
	static const int PANIC_WAIT_DURATION_MS = 20000;		// 20 sec
	static const int SENSOR_INTERVAL_SECS = 4*60*60;   // every 4 hours.
	// static const int SENSOR_INTERVAL_SECS = 10;   // 40 seconds for testing
};
