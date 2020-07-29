/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */
 
#include "main.h"

#include "sx1276-mbed-hal.h"
#include "RadioShuttle.h"
#include "RadioStatus.h"
#include "RadioSecurity.h"
#include "RadioBase.h"

#ifdef  FEATURE_NVPROPERTY
#include <NVPropertyProviderInterface.h>
#include "NVProperty.h"
#endif


#define CHECK_ERROR_RET(func, err) { \
	if (err) { \
		dprintf("Error in %s: %s", func, rs->StrError(err)); \
		return err; \
	} \
}


static void TempSensorRecvHandlerNew(int AppID, RadioShuttle::devid_t stationID, int msgID, int status, void *buffer, int length)
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



RadioBase::RadioBase()
{
	usePassword = false;
	useAES = false;
	appPassword = NULL;
	
	remoteDeviceID = 1;
	myDeviceID = 999;
	radioTypeMode = RadioShuttle::RS_Node_Offline;
}


int
RadioBase::InitRadio(RadioShuttle::AppRecvHandler appHandler, int myappid)
{
	RSCode err;
	
	myAppID = myappid;
	if (appHandler == 0)
		appHandler = &TempSensorRecvHandlerNew;
	
	InitSettings();
	InitLoRaChip();

	statusIntf = new MyRadioStatus();
	securityIntf = new RadioSecurity();
	
	rs = new RadioShuttle("MyRadioShuttle");
	
	rs->EnablePacketTrace(RadioShuttle::DEV_ID_ANY, true, true);
	
	err = rs->AddLicense(myDeviceID, myCode);
	CHECK_ERROR_RET("AddLicense", err);
	
	err = rs->AddRadio(radio, MODEM_LORA, myProfile);
	CHECK_ERROR_RET("AddRadio", err);
	dprintf("Radio: %.1f MHz, SF%d, %.f kHz", (float)myProfile[0].Frequency/1000000.0, myProfile[0].SpreadingFaktor, (float)myProfile[0].Bandwidth/1000.0);
	
	rs->AddRadioStatus(statusIntf);
	CHECK_ERROR_RET("AddRadioStatus", err);
	
	rs->AddRadioSecurity(securityIntf);
	CHECK_ERROR_RET("AddRadioSecurity", err);
	
	/*
	 * The password parameter can be NULL if no password is required
	 */
	err = rs->RegisterApplication(myAppID, appHandler,  (void *)appPassword);
	CHECK_ERROR_RET("RegisterApplication", err);
	
	err = rs->Startup(radioTypeMode);
	dprintf("Startup as a %s: %s ID=%d", isServer() ? "Server" : "Node",  rs->GetRadioName(rs->GetRadioType()), myDeviceID);
	if (!err && !isServer() && rs->AppRequiresAuthentication(myAppID) == RS_PasswordSet) {
		err = rs->Connect(myAppID, remoteDeviceID);
	}
	CHECK_ERROR_RET("Startup", err);
	return 0;
}

void
RadioBase::DeInitRadio(void)
{
    if (securityIntf) {
        delete securityIntf;
        securityIntf = NULL;
    }
    if (statusIntf) {
        delete statusIntf;
        statusIntf = NULL;
    }
    if (rs) {
        delete rs;
        rs = NULL;
    }
    if (radio) {
        delete radio;
        radio = NULL;
    }
}


void
RadioBase::InitSettings(void)
{
#ifdef  FEATURE_NVPROPERTY
	NVProperty prop;
	int value;
	
	myDeviceID = prop.GetProperty(prop.LORA_DEVICE_ID, 0);
	myCode = prop.GetProperty(prop.LORA_CODE_ID, 0);
	if ((value = prop.GetProperty(prop.LORA_RADIO_TYPE, 0)) != 0)
		radioTypeMode = (RadioShuttle::RadioType)value;
	
	if (myDeviceID == 0 || myCode == 0 || radioTypeMode == 0) {
		dprintf("LORA_DEVICE_ID or LORA_CODE_ID or LORA_RADIO_TYPE not set, use PropertyEditor to set this!");
		for(;;);
	}
	/*
	 * Here are optional properties for custom settings
	 */
	if ((value = prop.GetProperty(prop.LORA_REMOTE_ID, 0)) != 0)
		remoteDeviceID = value;
	if ((value = prop.GetProperty(prop.LORA_FREQUENCY, 0)) != 0)
		myProfile[0].Frequency = value;
	if ((value = prop.GetProperty(prop.LORA_BANDWIDTH, 0)) != 0)
		myProfile[0].Bandwidth = value;
	if ((value = prop.GetProperty(prop.LORA_SPREADING_FACTOR, 0)) != 0)
		myProfile[0].SpreadingFaktor = value;
	if ((value = prop.GetProperty(prop.LORA_TXPOWER, 0)) != 0)
		myProfile[0].TXPower = value;
	if ((value = prop.GetProperty(prop.LORA_FREQUENCY_OFFSET, 0)) != 0)
		myProfile[0].FrequencyOffset = value;
	if (!appPassword)
		appPassword = prop.GetProperty(prop.LORA_APP_PWD, (const char *)NULL);
#endif
}

void
RadioBase::InitLoRaChip(void)
{
#ifdef TARGET_DISCO_L072CZ_LRWAN1
	radio = new SX1276Generic(NULL, MURATA_SX1276,
							  LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
							  LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5,
							  LORA_ANT_RX, LORA_ANT_TX, LORA_ANT_BOOST, LORA_TCXO);
#elif defined(HELTECL432_REV1)
	radio = new SX1276Generic(NULL, HELTEC_L4_1276,
							  LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
							  LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5,
							  LORA_ANT_PWR);
#else // RFM95
	radio = new SX1276Generic(NULL, RFM95_SX1276,
							  LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
							  LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5);
#endif
}

void
RadioBase::InitLoRaChipWithShutdown(void)
{
#ifdef LORA_CS
	if (LORA_CS == NC)
	  return;

	InitLoRaChip();
	
	RadioEvents_t radioEvents;
	memset(&radioEvents, 0, sizeof(radioEvents));
	if (radio->Init(&radioEvents)) {
		radio->Sleep();
		delete radio;
	}
#endif
}


void
RadioBase::RadioContinuousTX(void)
{

	InitSettings();
	InitLoRaChip();

	dprintf("RadioContinuesTX test, press reset to abort");
	while(true) {
		int secs = 10;
		radio->SetTxContinuousWave(myProfile[0].Frequency, myProfile[0].TXPower, secs);
		wait_ms(secs * 1000);
		rprintf(".");
	}
}
