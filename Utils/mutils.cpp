/*
 * Copyright (c) 2020 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "main.h"
#ifdef  FEATURE_NVPROPERTY
#include <NVPropertyProviderInterface.h>
#include "NVProperty.h"
#endif
#include "GenericPingPong.h"
#include "RadioTest.h"
#include "ProgramInterface.h"
#include <vector>
#ifdef TOOLCHAIN_GCC
#include <malloc.h>
#endif
#include <cctype> // for isspace

volatile uint32_t PendingInterrupts;	// global interrupt mask of received interrupts

time_t cvt_date(char const *date, char const *time);

static float GetBrownOutVolt(void);
#ifdef FEATURE_SI7021
HELIOS_Si7021 *sensorSI7021;
#endif
BufferedSerial *ser;
#ifdef FEATURE_USBSERIAL
USBSerialBuffered *usb;
#endif
bool _useDprintf;

#ifdef FEATURE_USBSERIAL
static void InitUSBClocks()
{
#if defined(TARGET_STM32L4) && MBED_MAJOR_VERSION >= 5
	/*
	 * MBed OS 5 does not init the USB clocks, therefore we need to do it here
	 */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    /* Select MSI output as USB clock source, see system_lock.c:324  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_MSI; /* 48 MHz */
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
#endif
}
#endif

void InitSerial(int timeout, DigitalOut *led, InterruptIn *intr)
{
    _useDprintf = true;
    bool uartActive = true;
	
#ifdef FEATURE_USBSERIAL
	DigitalOut rx(USBRX);  // need to turn rx low to avoid floating signal
    rx = 0;
	DigitalIn uartRX(USBRX);
	uartActive = uartRX.read();
    if (!uartActive) {
        InitUSBClocks();
        usb = new USBSerialBuffered();
        Timer t;
        t.start();
        while(!usb->connected()) {
            if (led)
                *led = !*led;
            wait_ms(100);
            if (timeout) {
                if (t.read_ms() >= timeout || (intr && intr->read())) {
					delete usb;
					usb = NULL;
					DigitalOut rx(USBRX);
					rx = 0; // need to turn tx low to avoid floating signal
					break;
				}
			}
		}
	}
#endif
	if (uartActive) {
        ser = new BufferedSerial(USBTX, USBRX);
        ser->baud(230400);
        ser->format(8);
        /*
		 * unlock not needed for debugging via seriel
		 * for non debugger the UART will not be busy
		 */
        // sleep_manager_unlock_deep_sleep_internal();
    }

    time_t t = cvt_date(__DATE__, __TIME__);
    if (t > time(NULL)) {
        set_time(t);
    }
}

void RunStartup(void)
{
	rprintf("\r\n");
    int mbedversion = 9999;
#ifdef MBED_LIBRARY_VERSION // not available in mbed head compiles
    mbedversion = MBED_LIBRARY_VERSION;
#endif
    dprintf("Turtle: %d.%d (%s %s mbed: v%d)", MAJOR_VERSION, MINOR_VERSION, __DATE__, __TIME__, mbedversion);

    dprintf("SysClock: %u Hz.", (unsigned int)SystemCoreClock);
#ifdef __ARMCC_VERSION
    dprintf("ARM Compiler Version: 0x%x", __ARMCC_VERSION);
#elif __GNUC__
    dprintf("GCC Compiler Version: %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
	
	const char *errstr;
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST) != RESET)
		errstr = "RESET OCCURRED";
	else
	 	errstr = "initalized";
	
    dprintf("Brown Out Reset %s (%1.1f V)", errstr, GetBrownOutVolt());
    dprintf("Voltage: %.2f (%s powered)", BatteryVoltage(), BatterySource());
	dprintf("InitDefaults Done");
	MemoryAvailable(true);
    __HAL_RCC_CLEAR_RESET_FLAGS();
#ifdef FEATURE_SI7021
	sensorSI7021 = new HELIOS_Si7021(SI7021_SDA, SI7021_SCL);
	if (sensorSI7021->hasSensor()) {
		dprintf("%s: Rev(%d)  %.2f°C  Humidity: %.2f%%", sensorSI7021->getModelName(), sensorSI7021->getRevision(), sensorSI7021->readTemperature(), sensorSI7021->readHumidity());
	}
#endif

}
void printTimeStamp()
{
    static LowPowerTimer *timer;
    if (!timer) {
        timer = new LowPowerTimer();
        timer->start();
    }
    time_t seconds = time(NULL);
    struct tm *tm = localtime(&seconds);
    int usecs = timer->read_us();
    if (usecs < 0) {
        usecs = 0;
        timer->stop();
        timer->reset();
        timer->start();
    }
    int msecs = usecs % 1000000;
    
    rprintf("%02d:%02d:%02d.%06d ", tm->tm_hour, tm->tm_min, tm->tm_sec, msecs);
}

void dprintf(const char *format, ...)
{
    std::va_list arg;

    va_start(arg, format);
    VAprintf(true, true, _useDprintf, format, arg);
    va_end(arg);
}

void rprintf(const char *format, ...)
{
    std::va_list arg;

    va_start(arg, format);
    VAprintf(false, false, _useDprintf, format, arg);
    va_end(arg);   
}

void VAprintf(bool timstamp, bool newline, bool printEnabled, const char *format, va_list arg)
{
     if (!printEnabled)
        return;

    if (timstamp)
        printTimeStamp();
#ifdef FEATURE_USBSERIAL
    if (usb) {
        usb->vprintf_irqsafe(format, arg);
        if (newline)
            usb->printf_irqsafe("\r\n");
	}
#endif
    if (ser) {
        // serial jas 
        int r = 0;
        r = vsnprintf(NULL, 0, format, arg);
        if (r < 82) {
            char buffer[82+1];

            vsnprintf(buffer, sizeof(buffer), format, arg);
            r = ser->write(buffer, r);
        } else {
            char *buffer = new char[r+1];
            if (buffer) {
                vsnprintf(buffer, r+1, format, arg);
                r = ser->write(buffer, r);
                delete[] buffer;
            } else {
                error("%s %d cannot alloc memory (%d bytes)!\r\n", __FILE__, __LINE__, r+1);
                r = 0;
            }
        }
        if (newline)
            ser->write("\r\n", 2);
    }
}

char *ConsoleReadline(char *buf, int buflen, bool echo, int timeout_ms)
{
	int count = 0;
	memset(buf, 0, buflen);
	
#ifdef FEATURE_USBSERIAL
	if (usb == NULL && ser == NULL)
		return NULL;
#else
	if (ser == NULL)
		return NULL;
#endif
	
	Timer t;
	int start = 0;
	if (timeout_ms) {
		t.start();
		start = t.read_ms();
	}
	
#ifdef FEATURE_USBSERIAL
	if (usb) {
		usb->flush();
		while(usb->readable())
			usb->getc(); // flush old chars
	}
#endif
	if (ser) {
		while(ser->readable())
			ser->getc(); // flush old chars
	}
		
	while(true) {
		if (timeout_ms && t.read_ms() - start > timeout_ms)
			return NULL;
		int c = -2;
#ifdef FEATURE_USBSERIAL
		if (usb && usb->readable())
			c = usb->getc();
#endif
		if (ser && ser->readable())
			c = ser->getc();
		if (c == -2)
			continue;
		
		if (c == 0 || c == -1  || c == '\r' || c == '\n' ||	c == 3 || c == 4)
			break;
		if (c == '\b' || c == 0x7f) { // backspace
			if (count < 1)
				continue;
			buf[--count] = 0;
			if (echo)
				rprintf("\b \b");
#ifdef FEATURE_USBSERIAL
			if (usb)
				usb->flush();
#endif
			continue;
		}
		if (echo) {
			rprintf("%c", c);
#ifdef FEATURE_USBSERIAL
			if (usb)
				usb->flush();
#endif
		}
		
		start = t.read_ms();
		buf[count] = c;
		if (count++ >= buflen-2)
			break;
		// dprintf("Got char: '%c'(%d)", c, c);
	}
	
	if (echo)
		rprintf("\r\n");
	if (count)
		return buf;
	return NULL;
}


void dump(const char *title, void *data, int len)
{
	dump(title, data,  len, false);
}

void dump(const char *title, const void *data, int len, bool dwords)
{
    dprintf("dump(\"%s\", 0x%x, %d bytes)", title, (unsigned int)data, len);

    int i, j, cnt;
    unsigned char *u;
    const int width = 16;
    const int seppos = 7;

    cnt = 0;
    u = (unsigned char *)data;
    while (len > 0) {
        rprintf("%08x: ", (unsigned int)data + cnt);
        if (dwords) {
            unsigned int *ip = ( unsigned int *)u;
            rprintf(" 0x%08x\r\n", *ip);
            u+= 4;
            len -= 4;
            cnt += 4;
            continue;
        }
        cnt += width;
        j = len < width ? len : width;
        for (i = 0; i < j; i++) {
            rprintf("%2.2x ", *(u + i));
            if (i == seppos)
                rprintf(" ");
        }
        rprintf(" ");
        if (j < width) {
            i = width - j;
            if (i > seppos + 1)
                rprintf(" ");
            while (i--) {
                rprintf("%s", "   ");
            }
        }
        for (i = 0; i < j; i++) {
            int c = *(u + i);
            if (c >= ' ' && c <= '~')
                rprintf("%c", c);
            else
                rprintf(".");
            if (i == seppos)
                rprintf(" ");
        }
        len -= width;
        u += width;
        rprintf("\r\n");
		if (ser)
			wait_ms(5); // give the serial some time.
    }
    rprintf("--\r\n");
}

/*
 * Convert compile time to system time
 */
time_t
cvt_date(char const *date, char const *time)
{
    char s_month[5];
    int year;
    struct tm t;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", s_month, &t.tm_mday, &year);
    sscanf(time, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
    // Find where is s_month in month_names. Deduce month value.
    t.tm_mon = (strstr(month_names, s_month) - month_names) / 3;
    t.tm_year = year - 1900;
    return (int)mktime(&t);
}



void InterruptMSG(enum InterruptDevice irqid) {
        help_atomic_or_relaxed(&PendingInterrupts, irqid);
}


uint32_t readclrPendingInterrupts() {
	return help_atomic_readclr_relaxed(&PendingInterrupts);
}

uint32_t readPendingInterrupts() {
	return help_atomic_load_relaxed(&PendingInterrupts);
}

const char *
BatterySource(void)
{
    const char *pwrSource = "Battery";
#ifdef BATPOWER_EN
    {
        DigitalIn pwr(BATPOWER_EN);
        if (pwr == BATPOWER_EXT)
            pwrSource = "USB";
    }
#endif
	return pwrSource;
}


float
GetBrownOutVolt(void)
{
		unsigned int *FlashOptionRegister = (unsigned int *)0x1FFF7800;
		
		int val = *FlashOptionRegister >> 8 & 0x7; // masking out the BOR bits 9-11
		switch(val) {
			case 0:
				return 1.7;
			case 1:
				return 2.0;
			case 2:
				return 2.2;
			case 3:
				return 2.5;
			case 4:
				return 2.8;
			default:
				return 999;
		}
}

void MCUReset(void)
{
    #define AIRCR_VECTKEY_MASK    0x05FA0000
    SCB->AIRCR = AIRCR_VECTKEY_MASK | 0x04; // NVIC_GenerateSystemReset();
}


#define FREEMEM_CELL    100

struct elem { /* Definition of a structure that is FREEMEM_CELL bytes  in size.) */
    struct elem *next;
    char dummy[FREEMEM_CELL-2];
};

size_t
MemoryAvailable(bool print)
{
    size_t counter;
#ifdef TOOLCHAIN_GCC
    struct mallinfo mi = mallinfo();
    extern char end[];
    extern char _estack[];
    counter = (_estack - end) - mi.uordblks;
    if (print)
        dprintf("MemoryAvailable: %d kB (%d bytes)", counter/1024, counter);
    return counter;
#else
    struct elem *head, *current, *nextone;
    current = head = (struct elem*) malloc(sizeof(struct elem));
    if (head == NULL)
        return 0;      /*No memory available.*/
    counter = 0;
   // __disable_irq();
    do {
        counter++;
        current->next = (struct elem*) malloc(sizeof(struct elem));
        current = current->next;
    } while (current != NULL);
    /* Now counter holds the number of type elem
       structures we were able to allocate. We
       must free them all before returning. */
    current = head;
    do {
        nextone = current->next;
        free(current);
        current = nextone;
    } while (nextone != NULL);
   // __enable_irq();

    if (print)
        dprintf("MemoryAvailable: %d kB (%d bytes)", (counter*FREEMEM_CELL)/1024, counter*FREEMEM_CELL);
    return counter*FREEMEM_CELL;
#endif
}


static int setargs(char *args, char **argv)
{
   int count = 0;

   while (isspace(*args)) // remove leading
   	++args;
   	
   while (*args) {
	 if (argv)
	 	argv[count] = args;
	 while (*args && !isspace(*args))
	 	++args;
	 if (argv && *args)
	 	*args++ = '\0';
	 while (isspace(*args)) // remove trailing
	 	++args;
	 count++;
   }
   return count;
}

const char **parsedargs(char *args, int *argc)
{
   char **argv = NULL;
   int    argn = 0;

   if (args && *args
	&& (args = strdup(args))
	&& (argn = setargs(args,NULL))
	&& (argv = (char **)malloc((argn+1) * sizeof(char *)))) {
	  *argv++ = args;
	  argn = setargs(args,argv);
   }

   if (args && !argv)
   	free(args);

   *argc = argn;
   return (const char **)argv;
}

void freeparsedargs(const char **av)
{
	char **argv = (char **)av;
  	if (argv) {
		free(argv[-1]);
		free(argv-1);
  	}
}

static const char *cmds = \
	"\r\nThe following commands are available:\r\n\r\n" \
	" r -- Reset\r\n" \
	" d -- Hexdump of memory address [offset count]\r\n" \
	" h -- help for the command (e.g.: h b [help for blinky])\r\n" \
	" q -- quit program and continue\r\n";

void RunCommands(int timeout_ms) {
	ProgramInterface *prog = NULL;
	bool cmdLoop = true;
	while(cmdLoop) {
		char buf[32];

		rprintf(cmds);

		std::vector<ProgramInterface *>::iterator re;
		
		for(re = _progs.begin(); re != _progs.end(); re++) {
			rprintf(" %c -- %s %s\r\n", (*re)->GetShortkey(), (*re)->GetName(), (*re)->GetDescription());
		}
		rprintf(" c -- Continue with default program\r\n");
		rprintf("\r\nwaiting 10 secs ...\r\n\r\n");

		if (prog)
			rprintf("%s$ ", prog->GetName());
		else
			rprintf("Turtle$ ");
			
		if (ConsoleReadline(buf, sizeof(buf), true, timeout_ms) == NULL) {
			cmdLoop = false;
			break;
		}

		switch(buf[0]) {
			case 'c':
				cmdLoop = false;
				break;
			case 'q':
				if (prog) {
					prog->Shutdown();
					prog = NULL;
				}
				break;
			case 'r':
				MCUReset();
				break;
			case 'd':
				{
					char *addr = strchr(buf, ' ');
					if (addr) {
						*addr++ = 0;
						char *length = strchr(addr, ' ');
						if (length) {
							*length++ = 0;
						}
						unsigned long address = strtoll(addr, NULL, 0);
						unsigned long cnt = 32;
						if (length)
							cnt = strtoll(length, NULL, 0);
						dump("Hexdump", (void *)address, cnt);
					}
				}
				break;
			case 'h':
				{
					for(re = _progs.begin(); re != _progs.end(); re++) {
						if (buf[2] == (*re)->GetShortkey()) {
							rprintf("\r\nHelp for %s:\r\n\r\n\%s", (*re)->GetName(), (*re)->GetHelpText());
							rprintf("\r\nenter to continue...");
							ConsoleReadline(buf, sizeof(buf), true, 0);
							break;
						}
					}
				}
				break;
			default:
				if (prog == NULL) {
					for(re = _progs.begin(); re != _progs.end(); re++) {
						if (buf[0] ==  (*re)->GetShortkey()) {
							prog = (*re);
							rprintf("\r\nStarting: %s\r\n", prog->GetName());
							const char **av;
							int ac;
							av = parsedargs((char *)buf, &ac);
							prog->Startup(ac, av);
							ProgramInterface::ExecuteType ret = (ProgramInterface::ExecuteType) prog->ExecuteCommand();
							if (ret == ProgramInterface::E_TYPE_QUIT) {
								prog->Shutdown();
								freeparsedargs(av);
								prog = NULL;
							} else if (ret == ProgramInterface::E_TYPE_RESUME_CONT_COMMAND) {
								
							} else if (ret == ProgramInterface::E_TYPE_RESUME_LEAVE_COMMAND) {
								cmdLoop = false;
								break;
							} else {
								dprintf("invalid return vakue from ExecuteCommand()");
							}
						}
					}
				}
				break;
		}
	}
	rprintf("\r\n");

#ifdef  FEATURE_NVPROPERTY
	if (prog == NULL) {
		{
			NVProperty prop;
			const char *PropCmd = prop.GetProperty(prop.PROG_CMDLINE, (const char *)NULL);
			if (PropCmd)
				defCmdLine = PropCmd;
		}
	}
#endif
	if (prog == NULL) {
		std::vector<ProgramInterface *>::iterator re;
		for(re = _progs.begin(); re != _progs.end(); re++) {
			if (*defCmdLine ==  (*re)->GetShortkey()) {
				prog = (*re);
				rprintf("\r\nStarting: %s\r\n", prog->GetName());
				const char **av;
				int ac;
				av = parsedargs((char *)defCmdLine, &ac);
				prog->Startup(ac, av);
				ProgramInterface::ExecuteType ret = (ProgramInterface::ExecuteType)prog->ExecuteCommand();
				if (ret == ProgramInterface::E_TYPE_QUIT) {
					prog->Shutdown();
					freeparsedargs(av);
					prog = NULL;
				}
			}
		}
		if (prog == NULL)
			dprintf("Warning, default program: %s not found", defCmdLine);
	}
}
