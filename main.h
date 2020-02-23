/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "mbed.h"
#include "PinMap.h"
#include "BufferedSerial.h"
#ifdef FEATURE_USBSERIAL
#include "USBSerialBuffered.h"
#endif
#include "arch.h"
#include "mbed-util.h"
#ifdef FEATURE_SI7021
#include "HELIOS_Si7021.h"
#endif


enum InterruptDevice {
    INT_BUTTON1 = 0x01,	// user button
    INT_UNUSED1 = 0x02,
    INT_UNUSED2 = 0x04,
    INT_LORA    = 0x08,	// LoRa needs to stay on 0x08
	INT_TIMEOUT	= 0x10, // Low power timer timeout
};

extern volatile uint32_t PendingInterrupts;
extern void InterruptMSG(enum InterruptDevice irqid);
extern uint32_t readclrPendingInterrupts(void);
extern uint32_t readPendingInterrupts(void);

extern BufferedSerial *ser;
#ifdef FEATURE_USBSERIAL
extern USBSerialBuffered *usb;
#endif
extern bool _useDprintf;
extern void InitSerial(int timeout, DigitalOut *led, InterruptIn *intr);
extern void RunStartup(void);
extern size_t MemoryAvailable(bool print);

extern void dump(const char *title, const void *data, int len, bool dwords = false);
extern const char *BatterySource(void);

extern void dprintf(const char *format, ...) __attribute__((format(printf,1,2)));
extern void rprintf(const char *format, ...) __attribute__((format(printf,1,2)));
extern void VAprintf(bool timstamp, bool newline, bool printEnabled, const char *format, va_list arg);
extern char *ConsoleReadline(char *buf, int buflen, bool echo, int timeout_ms = 0);
extern void NVPropertyEditor(void);
extern void MCUReset(void);
extern void RunCommands(int timeout_ms);

#ifdef FEATURE_SI7021
extern HELIOS_Si7021 *sensorSI7021;
#endif
