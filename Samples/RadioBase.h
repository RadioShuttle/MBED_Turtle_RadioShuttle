/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#ifndef __RADIOBASE_H__
#define __RADIOBASE_H__

#ifdef FEATURE_LORA

class RadioBase {
public:
	RadioBase();
	int InitRadio(RadioShuttle::AppRecvHandler ah = NULL, int myAppID = 1);
	void DeInitRadio(void);
	void InitSettings(void);
	void InitLoRaChip(void);
	int RadioUpdate(bool keyPressed);
	void InitLoRaChipWithShutdown(void);
	void RadioContinuousTX(void);
	bool isServer() {
		if (radioTypeMode >= RadioShuttle::RS_Station_Basic)
			return true;
		else
			return false;
	};

	struct simpleTempData {
	  uint8_t magic;        // a magic to allow different message versions
	  uint8_t humidity;     // 0-100%
	  int16_t temperature;  // the float temperature * 100 to avoid floats
	  uint16_t battery_mV;  // the float battery * 100 to avoid floats
	};
	
// private:
	static int const SIMPLE_TEMP_DATA_MAGIC	= 0x55;
	bool usePassword = false;	// password the can used indepenend of AES
	bool useAES = false;		// AES needs the usePassword option on
	const char *appPassword;	// the AES password
	
	/*
	 * For details review: SX1276GenericLib/sx1276/sx1276.h
	 * Supported spreading factors SF 7,8, 9, 10, 11, (12 does not work well)
	 * Working frequencies using the 125000 bandwidth which leaves
	 * sufficient distance to the neighbour channel
	 * EU: 868.1, 868.3, 868.5 (Default LoRaWAN EU channels)
	 * EU: 865.1, 865.3, 865.5, 865.7, 865.9 (additional channels)
	 * EU: 866.1, 866.3, 866.5, 866.7, 866.9 (additional channels)
	 * EU: 867.1, 867.3, 867.5, 867.7, 867.9 (additional channels)
	 * Utilisation of these channels should not exceed 1% per hour per node
	 * Bandwidth changes other than 125k requires different channels distances
	 */
	RadioShuttle::RadioProfile myProfile[2] =  {
		/*
		 * Our default profile
		 * frequency, bandwidth, TX power, spreading factor, frequency-offset
		 */
		{ 868100000, 125000, 14, 7, 0 },
		{ 0, 0, 0, 0, 0 },
	};
	
	int myDeviceID;
	int remoteDeviceID;
	uint32_t myCode;
	RadioShuttle::RadioType radioTypeMode;
	int myAppID;
	
	Radio *radio;
	RadioShuttle *rs;
	RadioStatusInterface *statusIntf;
	RadioSecurityInterface *securityIntf;
};

#endif // FEATURE_LORA

#endif // __RADIOBASE_H__
