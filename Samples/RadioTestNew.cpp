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
#include "RadioTestNew.h"

const char * RadioTestNew::helpText = \
	"A basic peer-to-peer message example.\r\n" \
	"When the user button is pressed RadioTest will send a\r\n" \
	"simple text message to the other node.\r\n" \
	"The client node is in deepsleep to save energy.\r\n" \
	"The server node is in simple sleep to be able to receive messages.\r\n" \
	"Optionally, the client node can be in Node-Online mode to receive\r\n" \
	"            messages as well.\r\n" \
	"\r\nClient property settings:\r\n" \
	"  s12=xxx (LORA_REMOTE_ID of the server board\r\n" \
	"  s14=1   (LORA_RADIO_TYPE set to RS_Node_Offline\r\n" \
	"           Alternatively, s14=3 Node-Online to receive messages.\r\n" \
	"\r\nServer property settings:\r\n" \
	"  s12=xxx (LORA_REMOTE_ID of the client board)\r\n" \
	"  s14=3   (LORA_RADIO_TYPE set to RS_Node_Online)\r\n" \
	"";

static RadioTestNew *myClass;

static void TempSensorRecvHandlerNew(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
{
	myClass->MessageRecvHandler(AppID, stationID, msgID, status, buffer, length);
	return;
}


RadioTestNew::RadioTestNew() : ProgramInterface(this)
{
	SetName('n', "RadioTestNew", "RadioShuttle test example");
	myClass = this;
}


void
RadioTestNew::Startup(int argc, const char *argv[])
{
	isActive = true;
	if (InitRadio(&TempSensorRecvHandlerNew, SENSOR_APP_ID) != 0)
		exit(1);
}


void
RadioTestNew::Shutdown(void)
{
	DeInitRadio();
	isActive = false;
}


int
RadioTestNew::ExecuteCommand(void)
{
	return E_TYPE_RESUME_LEAVE_COMMAND; // leave command loop
}

bool
RadioTestNew::Service(uint32_t pendirqs)
{
	/*
	 * Update any processing here on user level
	 */
	bool didWork = false;
	
	if (pendirqs & INT_BUTTON1) {
		RadioUpdate(true); // pass the pressed user button to RadioShuttle
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
RadioTestNew::RadioUpdate(bool keyPressed)
{
	bool wasActive = false;
    if (!rs) 
        return 0;

    if (keyPressed) {
        int flags = 0;
        flags |= RadioShuttle::MF_NeedsConfirm;  // optional
        if (usePassword && useAES)
            flags |= RadioShuttle::MF_Encrypted;
        if (isServer()) {
            static const char msg[] = "The server feels very good today";
            rs->SendMsg(myAppID, (void *)msg, sizeof(msg)-1, flags, remoteDeviceID);
        } else {
            static const char msg[] = "Hello, the temperature is 26 celsius";
            rs->SendMsg(myAppID, (void *) msg, sizeof(msg)-1, flags, remoteDeviceID);
        }
        wasActive = true;
    }

	return wasActive;
}


void
RadioTestNew::MessageRecvHandler(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
{
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
            // dump("MSG_RecvData", buffer, length);
            break;
        case RadioShuttle::MS_RecvDataConfirmed:	// received a confirmed message
            dprintf("MSG_RecvDataConfirmed ID: %d, len=%d", msgID, length);
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

}
