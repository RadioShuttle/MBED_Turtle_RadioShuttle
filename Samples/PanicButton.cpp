/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */


/*
 * The following are required properties for the operation
 * LORA_RADIO_TYPE  e.g.: s14=1 for RS_Node_Offline, s14=2 for RS_Node_Online
 * LORA_REMOTE_ID   e.g.: s12=123 (replace 123 with your server board ID)
 * Optionally:
 * LORA_APP_PWD, e.g.: s20=Hello (must be identical for all boards using this app)
 * The password will only allow clients to communicate with the same password
 * For AES128-bit content encryption, in addition to a password, set useAES = true (in line 46)
 */
 
#include "main.h"

#include "RadioShuttle.h"
#include "RadioStatus.h"
#include "RadioSecurity.h"

#include "ProgramInterface.h"
#include "RadioBase.h"
#include "PanicButton.h"

#include "Utils/mbed-util.h"

const char * PanicButton::helpText = \
	"When the User Button is pressed it will send a panic\r\n" \
	"message to the server. Every four hours the battery status\r\n" \
	"is sent to the server.\r\n" \
	"The client is in deepsleep to save energy.\r\n" \
	"The red client LED is flashing for 20 seconds to indicate\r\n" \
	"a call to the server.\r\n" \
	"Optionally, a transistor-driven buzzer can be used instead\r\n" \
	"           of the LED for ringing.\r\n" \
	"\r\nClient property settings:\r\n" \
	"  s12=xxx (LORA_REMOTE_ID of the server\r\n" \
	"  s14=1   (LORA_RADIO_TYPE set to RS_Node_Offline, or 3 = RS_Node_Online)\r\n" \
	"\r\nServer property settings:\r\n" \
	"  s14=4   (LORA_RADIO_TYPE set to RS_Station_Basic)\r\n" \
	"";

static PanicButton *myClass;

static void TempSensorRecvHandlerNew(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
{
	myClass->MessageRecvHandler(AppID, stationID, msgID, status, buffer, length);
	return;
}


PanicButton::PanicButton() : ProgramInterface(this)
{
	SetName('x', "PanicButton", "RadioShuttle panic button example");
	myClass = this;
	
	buzzer = NULL;
	timeout = NULL;
	counter = NULL;
}

void
PanicButton::Startup(int argc, const char *argv[])
{
	if (argc > 1 && argv[1][0] == 'a')
		useASCIIMSG = true;
	else
		useASCIIMSG = false;

	isActive = true;
	if (InitRadio(&TempSensorRecvHandlerNew, SENSOR_APP_ID) != 0)
		exit(1);

	buzzer = new DigitalOut(LED2);
	*buzzer = 0;
	timeout = new LowPowerTimeout;
	isActive = true;
}

void
PanicButton::Shutdown(void)
{
	DeInitRadio();
	if (buzzer)
		delete buzzer;
	if (timeout) {
		timeout->detach();
		delete timeout;
	}
	isActive = false;
}

int
PanicButton::ExecuteCommand(void)
{
	alarmSubmitConfirmed = false;
	if (!isServer())
		timeout->attach_us(callback(this, &PanicButton::StatusUpdate), SENSOR_INTERVAL_SECS * 1e6);
	return E_TYPE_RESUME_LEAVE_COMMAND; // leave command loop
}

bool
PanicButton::Service(uint32_t pendirqs)
{
	bool didWork = false;
	
	if (pendirqs & INT_BUTTON1) {
		RunSensorUpdate(true);
		didWork = true;
	}
	if (pendirqs & INT_TIMEOUT) {
		RunSensorUpdate(false);
		didWork = true;
	}
	if (pendirqs & INT_TIMEOUT2) {
		RunPanicUpdate();
		didWork = true;
	}
	if (pendirqs & INT_LORA) {
		rs->RunShuttle();
		didWork = true;
	}
	
	if (rs->RunShuttle())
		didWork = true;

	return didWork;
}

bool
PanicButton::RunSensorUpdate(bool userKeyPressed)
{
	if (isServer()) {
		rs->RunShuttle();
		return false; // nothing todo for the server case
	}
		
	int flags = 0;
	flags |= RadioShuttle::MF_NeedsConfirm; // optional
	if (rs->AppRequiresAuthentication(myAppID) == RS_PasswordSet)
	  flags |= RadioShuttle::MF_Encrypted; // optional enable encryption when desired
	
	memset(&tempData, 0, sizeof(tempData));
	tempData.magic = SIMPLE_PANIC_DATA_MAGIC;
	if (userKeyPressed)
	  tempData.flags |= PANIC_FLAGS_ALARM;

	tempData.battery_mV = BatteryVoltage() * 100;
	tempData.flags |= PANIC_FLAGS_INFO;

	if (tempData.flags & PANIC_FLAGS_ALARM) {
		dprintf("tempData.flags ALARM");
		rs->UpdateNodeStartup(RadioShuttle::RS_Node_Online);
		panicTimer.attach_us(callback(this, &PanicButton::PanicTimeoutFunc), PANIC_WAIT_DURATION_MS * 1000);
		counter = new Timer;
		counter->start();
		*buzzer = 0;
		buzzerTimer.attach_us(callback(this, &PanicButton::BuzzerSoundFunc), BUZZER_SOUND_INTERVAL_DIAL * 1000);
	} else {
	  dprintf("tempData.flags INFO");
	}
	
	if (useASCIIMSG) { // ASCII for IO-Brocker
		dprintf("Flags: %d", tempData.flags);
		snprintf(asciiTempData, sizeof(asciiTempData), "%c%c%03d", (tempData.magic >> 4) + 87, '0' + tempData.flags, tempData.battery_mV);
		rs->SendMsg(myAppID, &asciiTempData, sizeof(asciiTempData)-3, flags, remoteDeviceID);
	} else {
		rs->SendMsg(myAppID, &tempData, sizeof(tempData), flags, remoteDeviceID);
	}
	return true;
}

void
PanicButton::RunPanicUpdate(void)
{
	counter->stop();
	delete counter;
	counter = NULL;
	buzzerTimer.detach(); // de-actived timer
	*buzzer = 0;
	rs->UpdateNodeStartup(RadioShuttle::RS_Node_Offline);
	dprintf("panicTimerExired");
}

void
PanicButton::PanicTimeoutFunc(void)
{
	/*
	 * This timer callback function is called on interrupt level
	 * it is not good to do here stuff, forward a signal
	 * that the Service() routing gets called on user level from the main loop.
	 */
	InterruptMSG(INT_TIMEOUT2);
}

void
PanicButton::StatusUpdate()
{
	/*
	 * This timer callback function is called on interrupt level
	 * it is not good to do here stuff, forward a signal
	 * that the Service() routing gets called on user level from the main loop.
	 */
	InterruptMSG(INT_TIMEOUT);
	timeout->attach_us(callback(this, &PanicButton::StatusUpdate), SENSOR_INTERVAL_SECS * 1e6);
}

void
PanicButton::BuzzerSoundFunc()
{
	if (!alarmSubmitConfirmed) {
	  if (time_t(NULL) & 0x01) { // every second interval
		*buzzer = 0;
	  } else {
		*buzzer = !*buzzer;
	  }
	  buzzerTimer.attach_us(callback(this, &PanicButton::BuzzerSoundFunc), BUZZER_SOUND_INTERVAL_DIAL * 1000);
	} else {
		if (time(NULL) & 0x01 && counter->read_ms() & 0x80) { // every 4 second interval
			*buzzer = !*buzzer;
		} else {
			*buzzer = 0;
	  }
	  buzzerTimer.attach_us(callback(this, &PanicButton::BuzzerSoundFunc), BUZZER_SOUND_INTERVAL_FINISH * 1000);
	}
}


void
PanicButton::MessageRecvHandler(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
{
	bool gotData = false;
	UNUSED(stationID);
	UNUSED(buffer);
	UNUSED(AppID);

	switch (status) {
	case RadioShuttle::MS_SentCompleted:  // A SendMsg has been sent.
		dprintf("MSG_SentCompleted: id=%d  %d bytes", msgID, length);
		break;
	case RadioShuttle::MS_SentCompletedConfirmed:// A SendMsg has been sent and confirmed
		myClass->setAlarmSubmitConfirmed();
		dprintf("MSG_SentCompletedConfirmed: id=%d %d bytes", msgID, length);
		break;
	case RadioShuttle::MS_SentTimeout:    // A timeout occurred, number of retries exceeded
		dprintf("MSG_SentTimeout ID: %d", msgID);
		break;
	case RadioShuttle::MS_RecvData:     // a simple input message
		dprintf("MSG_RecvData ID: %d, len=%d", msgID, length);
		gotData = true;
		// dump("MSG_RecvData", buffer, length);
		break;
	case RadioShuttle::MS_RecvDataConfirmed:  // received a confirmed message
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
	case RadioShuttle::MS_StationConnected: // a confirmation that the connection was accepted
		dprintf("MSG_StationConnected");
		break;
	case RadioShuttle::MS_StationDisconnected:  // a confirmation that the disconnect was accepted
		dprintf("MSG_StationDisconnected");
		break;
	default:
		break;
	}
	
    if (gotData) {
    	struct simplePanicData *r = (struct simplePanicData *)buffer;
    	if (length == sizeof(simplePanicData) && r->magic == SIMPLE_PANIC_DATA_MAGIC) {
    		float volt = r->battery_mV / 100.0;
    		dprintf("Got new data: ID: %d Flags: %d (%s) Battery: %.2f V", (int)stationID, r->flags,
    		r->flags & PANIC_FLAGS_ALARM ? "ALARM" : "info",
    		volt);
		}
    }
}
