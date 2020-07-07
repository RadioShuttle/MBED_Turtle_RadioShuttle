/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

/*
 * TODO:
 * - receive handler, keep current state, store current state in RTC mem
 *   RTC State storage in Startup und ServerUpdate
 *   de-bounce the signals
 */

#include "main.h"

#include "ProgramInterface.h"
#include "RadioShuttle.h"
#include "RadioStatus.h"
#include "RadioSecurity.h"

#include "RadioBase.h"
#include "RemoteRelay.h"

#include <bitset>

const char * RemoteRelay::helpText = \
	"The RemoteRelay example allows mirroring up to eight pins\r\n" \
	"from the client to the server that has a relays connected to\n\r" \
	"these pins. Every 10 minutes, or upon pressing the user button,\n\r" \
	"the client repeats the current pin state including the\n\r" \
	"battery status for energy monitoring. The client input pins\r\n" \
	"are pulled to high to avoid noise signals on open wires.\n\r" \
	"The relay input pin will be switched to low (for low), or to\r\n" \
	"input (for high). This is how relays with internal pull-up work.\r\n" \
	"The client node is in deepsleep to save energy.\r\n" \
	"Configured pins per default: PB_4, PB_5, PB_6, PB_7\r\n" \
	"\r\nClient property settings:\r\n" \
	"  s12=xxx (LORA_REMOTE_ID of the server board)\r\n" \
	"  s14=1   (LORA_RADIO_TYPE set to RS_Node_Offline, or 3 = RS_Node_Online)\r\n" \
	"\r\nServer property settings:\r\n" \
	"  s14=4   (LORA_RADIO_TYPE set to RS_Station_Basic)\r\n" \
	"";


static RemoteRelay *myClass;

static void TempSensorRecvHandlerNew(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
{
	myClass->MessageRecvHandler(AppID, stationID, msgID, status, buffer, length);
	return;
}

RemoteRelay::RemoteRelay(std::vector<PinName> v) : ProgramInterface(this)
{
	SetName('k', "RemoteRelay", "RadioShuttle remote relay control");
	myClass = this;
	inPins = v;
}


void
RemoteRelay::Startup(int argc, const char *argv[])
{
	if (argc > 1 && argv[1][0] == 'a')
		useASCIIMSG = true;
	else
		useASCIIMSG = false;

	isActive = true;
	if (InitRadio(&TempSensorRecvHandlerNew, SENSOR_APP_ID) != 0)
		exit(1);
		
	MBED_ASSERT(inPins.size() <= MAX_RLEAY_CHANNEL);

	lastKnownPinStatus = 0xffff;
	int cnt = 0;
    for(PinName n : inPins) {
		if (isServer()) {
			DigitalIn pinIn(n);
		} else {
			pinIntr[cnt] = new InterruptIn(n);
			pinIntr[cnt]->mode(PullUp);
			pinIntr[cnt]->fall(callback(this, &RemoteRelay::IntFunc));
			pinIntr[cnt]->rise(callback(this, &RemoteRelay::IntFunc));
			cnt++;
		}
    }
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
RemoteRelay::Shutdown(void)
{
	DeInitRadio();
	isActive = false;
	if (timeout)
		delete timeout;
	if (ledSignalTimer)
		delete ledSignalTimer;
	if (ledStatus) {
		*ledStatus = 0;
		delete ledStatus;
	}
}


int
RemoteRelay::ExecuteCommand(void)
{
	if (!isServer()) {
		timeout->attach_us(callback(this, &RemoteRelay::RelayStatusUpdate), BATTERY_INTERVAL_SECS * 1e6);
		RelayUpdate(true, true);
	}

	return E_TYPE_RESUME_LEAVE_COMMAND; // leave command loop
}

bool
RemoteRelay::Service(uint32_t pendirqs)
{
	bool didWork = false;
	
	if (pendirqs & INT_BUTTON1) {
		RelayUpdate(true, true); // pass the pressed user button to RadioShuttle
		didWork = true;
	}
	if (pendirqs & INT_TIMEOUT) {
		RelayUpdate(false, true); // pass the pressed user button to RadioShuttle
		didWork = true;
	}
	if (pendirqs & INT_LORA) {
		if (rs->RunShuttle())
			didWork = true;
	}
	if (pendirqs & INT_BUTTON_GRP1) {
		RelayUpdate(false, false);
		didWork = true;
	}

	if (rs->RunShuttle())
		didWork = true;

	return didWork;
}


bool
RemoteRelay::RelayUpdate(bool keyPressed, bool statusUpdateTimer)
{
    if (!rs) {
        return false;
	}

	if (isServer())
		return false; // nothing to do for the server.
		
	if (!keyPressed && !statusUpdateTimer)
		wait_ms(20); // wait to seatle GPIO spikes
		
	memset(&relayData, 0, sizeof(relayData));
	relayData.magic = RELAY_DATA_MAGIC;
	relayData.flags = UPDATE_STATE;
	
	std::bitset<MAX_RLEAY_CHANNEL> bitmap;

	for (int i = 0; i < (int)inPins.size(); i++) {
		if (pinIntr[i]->read())
			bitmap[i] = 1;
		else
			bitmap[i] = 0;
	}
	relayData.pinStatus = bitmap.to_ulong();
	
	if (keyPressed || statusUpdateTimer) {
		relayData.flags |= REPEAT_STATE;
		relayData.flags |= BATTERY_STATUS;
		relayData.battery_mV = BatteryVoltage() * 100;
	}
	
	if (relayData.flags == UPDATE_STATE && relayData.pinStatus == lastKnownPinStatus)
		return false; // no timer/button event, same result, nothing to do.
	dprintf("ServerRelayUpdate: lastKnownPinStatus: 0x%x newpins: 0x%x", lastKnownPinStatus, relayData.pinStatus);
	lastKnownPinStatus = relayData.pinStatus;
	
	int flags = 0;
	flags |= RadioShuttle::MF_NeedsConfirm;  // optional
	if (usePassword && useAES)
		flags |= RadioShuttle::MF_Encrypted;
	
	if (useASCIIMSG) { // ASCII for IO-Brocker
		char tbuf[MAX_RLEAY_CHANNEL+1];
		for (int i = 0; i < MAX_RLEAY_CHANNEL; i++) {
			if (bitmap[i])
				tbuf[i] = '1';
			else
				tbuf[i] = '0';
			tbuf[i+1] = 0; // terminate string.
		}
		snprintf(asciiTempData, sizeof(asciiTempData), "%c%c%s%03d", (relayData.magic >> 4) + 87, '0' + relayData.flags, tbuf, relayData.battery_mV);
		rs->SendMsg(myAppID, &asciiTempData, sizeof(asciiTempData)-4, flags, remoteDeviceID);
	} else {
		rs->SendMsg(myAppID, (void *) &relayData, sizeof(relayData), flags, remoteDeviceID);
	}
	
	return true;
}




void
RemoteRelay::RelayStatusUpdate(void)
{
	/*
	 * This timer callback function is called on interrupt level
	 * it is not good to do here stuff, forward a signal
	 * that the Service() routing gets called on user level from the main loop.
	 */
	InterruptMSG(INT_TIMEOUT);
	timeout->attach_us(callback(this, &RemoteRelay::RelayStatusUpdate), BATTERY_INTERVAL_SECS * 1e6);
}


void
RemoteRelay::ServerRelayUpdate(relayDataRecord *r)
{
	uint16_t lastpins = lastKnownPinStatus;
	uint16_t newpins = r->pinStatus;
	
	dprintf("ServerRelayUpdate: lastpins: 0x%x newpins: 0x%x", lastpins, newpins);
	
	for (int i = 0; i < (int)inPins.size(); i++) {
		//if ((lastpins & 1) == (newpins & 1)) // unchanged on or off
		//	continue;
		if (lastpins & 1 && !(newpins & 1)) { // turn on
			DigitalOut out(inPins[i]);
			out = 0;
		}
		if (!(lastpins & 1) && newpins & 1) { // turn off
			DigitalIn in(inPins[i]); // input, high via Pullup
		}
		lastpins >>= 1;
		newpins >>= 1;
	}
	lastKnownPinStatus = r->pinStatus;
}

void
RemoteRelay::LedUpdate(void)
{
	if (*ledStatus) {
		*ledStatus = 0;
		timeout->attach_us(callback(this, &RemoteRelay::LedUpdate), LED_SERVER_INTERVAL_SECS * 1e6);
	} else {
		*ledStatus = 1;
		timeout->attach_us(callback(this, &RemoteRelay::LedUpdate), LED_SERVER_DURATION_MS * 1000);
	}
}

void
RemoteRelay::MessageRecvHandler(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
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
    	struct relayDataRecord *r = (struct relayDataRecord *)buffer;
    	if (length == sizeof(relayDataRecord) && r->magic == RELAY_DATA_MAGIC) {
    		ServerRelayUpdate(r);
    	}
	}
}

