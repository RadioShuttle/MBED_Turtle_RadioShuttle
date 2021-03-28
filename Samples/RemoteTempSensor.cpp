/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "main.h"

#include "ProgramInterface.h"
#include "RadioShuttle.h"
#include "RadioStatus.h"
#include "RadioSecurity.h"

#include "RadioBase.h"
#ifdef  FEATURE_NVPROPERTY
#include <NVPropertyProviderInterface.h>
#include "NVProperty.h"
#endif
#include "RemoteTempSensor.h"

const char * RemoteTempSensor::helpText = \
	"A RadioShuttle remote humidity and temperature sensor example.\r\n" \
	"The Si7021 sensor is connected via I2C to the GND/VDD/SCL/SDA pins.\r\n" \
	"The client sensor node will send temperature, humidity, and battery data\r\n"
	"every 10 minutes, or on a user button click to the server.\r\n" \
	"The client node is in deepsleep to save energy until activity is required.\r\n" \
	"The server node is in simple sleep to be able to receive messages.\r\n" \
	"Multiple sensor nodes are supported, the sensor data is packed to use\r\n" \
	"small data transfers which will significantly save energy.\r\n"
	"\r\nClient property settings:\r\n" \
	"  s12=xxx (LORA_REMOTE_ID of the server board\r\n" \
	"  s14=1   (LORA_RADIO_TYPE set to RS_Node_Offline, or 3 = RS_Node_Online)\r\n" \
	"  s91=30  (optional: SENSOR_INTERVAL to specify the periodic measure interval)\r\n" \
	"\r\nServer property settings:\r\n" \
	"  s14=4   (LORA_RADIO_TYPE set to RS_Station_Basic)\r\n" \
	"";

static RemoteTempSensor *myClass;

static void TempSensorRecvHandlerNew(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
{
	myClass->MessageRecvHandler(AppID, stationID, msgID, status, buffer, length);
	return;
}


RemoteTempSensor::RemoteTempSensor() : ProgramInterface(this)
{
	SetName('t', "RemoteTempSensor", "RadioShuttle remote temerature sensor");
	myClass = this;
}


void
RemoteTempSensor::Startup(int argc, const char *argv[])
{
	if (argc > 1 && argv[1][0] == 'a')
		useASCIIMSG = true;
	else
		useASCIIMSG = false;

	sensor_interval = SENSOR_INTERVAL_SECS;
	sensor_threshold = 0;
#ifdef  FEATURE_NVPROPERTY
	NVProperty prop;
	int value;
	value = prop.GetProperty(prop.SENSOR_INTERVAL, 0);
	if (value)
		sensor_interval = value;
	value = prop.GetProperty(prop.SENSOR_THRESHOLD, 0);
	if (value)
		sensor_threshold = value;
#endif
	isActive = true;
	if (InitRadio(&TempSensorRecvHandlerNew, SENSOR_APP_ID) != 0)
		exit(1);
	timeout = new LowPowerTimeout;
	ledSignalTimer = NULL;
	ledStatus = NULL;
	if (isServer()) {
		ledSignalTimer = new LowPowerTimeout;
		ledStatus = new DigitalOut(LED2);
		LedUpdate(); // start red LED status timer
	}
}


void
RemoteTempSensor::Shutdown(void)
{
	DeInitRadio();
	if (timeout)
		delete timeout;
	if (ledSignalTimer)
		delete ledSignalTimer;
	if (ledStatus) {
		*ledStatus = 0;
		delete ledStatus;
	}

	isActive = false;
}


int
RemoteTempSensor::ExecuteCommand(void)
{
	if (!isServer())
		timeout->attach_us(callback(this, &RemoteTempSensor::SensorUpdateTimeoutFunc), sensor_interval * 1e6);
		
	return E_TYPE_RESUME_LEAVE_COMMAND; // leave command loop
}

bool
RemoteTempSensor::Service(uint32_t pendirqs)
{
	bool didWork = false;
	
	if (pendirqs & INT_BUTTON1) {
		SensorUpdate(true); // pass the pressed user button to RadioShuttle
		didWork = true;
	}
	if (pendirqs & INT_TIMEOUT) {
		SensorUpdate(false); // pass periodic timeout to RadioShuttle
		didWork = true;
	}
	if (pendirqs & INT_LORA) {
		if (rs->RunShuttle())
			didWork = true;
	}

	if (rs->RunShuttle())
		didWork = true;

	return didWork;
}


bool
RemoteTempSensor::SensorUpdate(bool keyPressed)
{
    if (!rs) {
        return false;
	}

	if (isServer()) {
		rs->RunShuttle();
		return false;
	}

	memset(&sensorData, 0, sizeof(sensorData));
	sensorData.magic = TEMP_SENSOR_DATA_MAGIC;
#ifdef FEATURE_SI7021
	if (sensorSI7021->hasSensor()) {
		sensorData.humidity = sensorSI7021->readHumidity();
		sensorData.temperature = sensorSI7021->readTemperature() * 100;
	}
#endif
	sensorData.battery_mV = BatteryVoltage() * 100;
	
	int flags = 0;
	flags |= RadioShuttle::MF_NeedsConfirm;  // optional
	if (usePassword && useAES)
		flags |= RadioShuttle::MF_Encrypted;
	if (useASCIIMSG) { // ASCII for IO-Brocker
		// Format \HHTTTTTVVV, e.g. Y4407012329
		// 30.12 Grad via (07012 / 100)-40
		// 44% Humidit
		// 3.29 V (329 / 100)
		snprintf(asciiTempData, sizeof(asciiTempData), "%c%02d%05d%03d", (sensorData.magic >> 4) + 87, sensorData.humidity, (sensorData.temperature + 4000), sensorData.battery_mV);
		rs->SendMsg(myAppID, &asciiTempData, sizeof(asciiTempData)-5, flags, remoteDeviceID);
	} else {
		rs->SendMsg(myAppID, (void *) &sensorData, sizeof(sensorData), flags, remoteDeviceID);
	}

	return true;
}

void
RemoteTempSensor::SensorUpdateTimeoutFunc(void)
{
	/*
	 * This timer callback function is called on interrupt level
	 * it is not good to do here stuff, forward a signal
	 * that the Service() routing gets called on user level from the main loop.
	 */
	InterruptMSG(INT_TIMEOUT);
	timeout->attach_us(callback(this, &RemoteTempSensor::SensorUpdateTimeoutFunc), sensor_interval * 1e6);
}

void
RemoteTempSensor::LedUpdate(void)
{
	if (*ledStatus) {
		*ledStatus = 0;
		timeout->attach_us(callback(this, &RemoteTempSensor::LedUpdate), LED_SERVER_INTERVAL_SECS * 1e6);
	} else {
		*ledStatus = 1;
		timeout->attach_us(callback(this, &RemoteTempSensor::LedUpdate), LED_SERVER_DURATION_MS * 1000);
	}
}

void
RemoteTempSensor::MessageRecvHandler(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
{
	bool gotData = false;
	
    switch(status) {
        case RadioShuttle::MS_SentCompleted:	// A SendMsg has been sent.
            dprintf("MSG_SentCompleted: id=%d  %d bytes", msgID, length);
            break;
        case RadioShuttle::MS_SentCompletedConfirmed:// A SendMsg has been sent and confirmed
            dprintf("MSG_SentCompletedConfirmed: id=%d %d bytes", msgID, length);
            break;
        case RadioShuttle::MS_SentTimeout:		// A timeout occurred, number of retries exceeded
            dprintf("MSG_SentTimeout ID: %d", msgID);
            break;

        case RadioShuttle::MS_RecvData:			// a simple input message
            dprintf("MSG_RecvData ID: %d, len=%d", msgID, length);
            gotData = true;
            // dump("MSG_RecvData", buffer, length);
            break;
        case RadioShuttle::MS_RecvDataConfirmed:	// received a confirmed message
            dprintf("MSG_RecvDataConfirmed ID: %d, len=%d", msgID, length);
            gotData = true;
			// dump("MSG_RecvDataConfirmed", buffer, length);
            break;
        case RadioShuttle::MS_NoStationFound:
            dprintf("MSG_NoStationFound");
            break;
        case RadioShuttle::MS_NoStationSupportsApp:
            dprintf("MSG_NoStationSupportsApp");
            break;
        case RadioShuttle::MS_AuthenicationRequired: // the password does not match.
            dprintf("MSG_AuthenicationRequired");
            break;

        case RadioShuttle::MS_StationConnected:	// a confirmation that the connection was accepted
            dprintf("MSG_StationConnected");
            break;
        case RadioShuttle::MS_StationDisconnected:	// a confirmation that the disconnect was accepted
            dprintf("MSG_StationDisconnected");
            break;
        default:
            break;
    }
    if (gotData) {
    	struct sensorDataRecord *r = (struct sensorDataRecord *)buffer;
    	if (length == sizeof(sensorDataRecord) && r->magic == TEMP_SENSOR_DATA_MAGIC) {
    		float temp = r->temperature / 100.0;
    		float volt = r->battery_mV / 100.0;
    		char cel[3] = { 0xc2, 0xb0, 0x00 };
    		dprintf("Got new data: ID: %d Temperature: %.2f%sC  Humidity: %d%%  Battery: %.2f V", (int)stationID, temp, &cel[0], r->humidity, volt);
		}
    }
}
