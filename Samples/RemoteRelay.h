/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */
 

class RemoteRelay : public RadioBase, public ProgramInterface {
public:
	RemoteRelay(std::vector<PinName> v);
	void Startup(int argc, const char *argv[]);
	void Shutdown(void);
	bool Service(uint32_t pendirqs);
	int ExecuteCommand(void);
	const char *GetHelpText(void) { return helpText; };

	void MessageRecvHandler(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length);
	bool RelayUpdate(bool keyPressed, bool statusUpdateTimer);

private:
	static const char *helpText;
	enum relayFlags {
		UPDATE_STATE = 0x01,
		REPEAT_STATE = 0x02,
		BATTERY_STATUS = 0x04,
	};
	struct relayDataRecord {
		uint8_t magic;		// a magic to allow different message versions
		uint8_t flags;		//
		uint16_t pinStatus;	// 
	  	uint16_t battery_mV;// the float battery * 100 to avoid floats
	};
	
	relayDataRecord relayData;
	uint16_t lastKnownPinStatus;

	static const int MAX_RLEAY_CHANNEL = 8;
	std::vector<PinName> inPins;
	InterruptIn *pinIntr[MAX_RLEAY_CHANNEL];
	
	void IntFunc(void) { InterruptMSG(INT_BUTTON_GRP1); };

	void RelayStatusUpdate(void);
	
	void ServerRelayUpdate(relayDataRecord *r);
	void LedUpdate(void);
	bool useASCIIMSG;
	char asciiTempData[17];
	LowPowerTimeout *timeout;
	LowPowerTimeout *ledSignalTimer;
	DigitalOut *ledStatus;
	
	static const int SENSOR_APP_ID = 1;
	static const int RELAY_DATA_MAGIC = 0x56;
	static const int BATTERY_INTERVAL_SECS = 10*60;   // every 10 minues.
	static const int LED_SERVER_INTERVAL_SECS = 2;
	static const int LED_SERVER_DURATION_MS = 2;
};
