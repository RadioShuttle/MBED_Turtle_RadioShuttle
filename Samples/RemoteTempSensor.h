/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

class RemoteTempSensor : public RadioBase, public ProgramInterface {
public:
	RemoteTempSensor();
	void Startup(int argc, const char *argv[]);
	void Shutdown(void);
	bool Service(uint32_t pendirqs);
	int ExecuteCommand(void);
	const char *GetHelpText(void) { return helpText; };

	bool SensorUpdate(bool keyPressed);
	void SensorUpdateTimeoutFunc(void);
	void MessageRecvHandler(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length);

private:
	static const char *helpText;

	struct sensorDataRecord {
	  uint8_t magic;        // a magic to allow different message versions
	  uint8_t humidity;     // 0-100%
	  int16_t temperature;  // the float temperature * 100 to avoid floats
	  uint16_t battery_mV;  // the float battery * 100 to avoid floats
	};
	
	sensorDataRecord sensorData;

	void LedUpdate(void);
	bool useASCIIMSG;
	char asciiTempData[16];
	LowPowerTimeout *timeout;
	LowPowerTimeout *ledSignalTimer;
	DigitalOut *ledStatus;
	int sensor_interval;
	int sensor_threshold;

	static const int SENSOR_APP_ID = 1;
	static const int TEMP_SENSOR_DATA_MAGIC = 0x55;
	static const int SENSOR_INTERVAL_SECS = 10*60;   // every 10 minutes.
	static const int LED_SERVER_INTERVAL_SECS = 2;
	static const int LED_SERVER_DURATION_MS = 2;
};
