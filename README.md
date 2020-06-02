# Turtle Board Software

![Turtle Board](/docs/Turtle_Board.png)

The Turtle board is a highly energy-efficient LoRa peer-to-peer turnkey solution which runs up to 10 years on batteries. It consists of an STM32L432KC MCU with an SX1276 LoRa radio chip, and extensive software support made by the RadioShuttle developers. This repository includes the complete software required to develop solutions for the Turtle board.

Helmut Tschemernjak
www.radioshuttle.de

## Prerequisites using the Turtle board
- A Turtle board with a micro-USB cable
- A second Turtle, ECO Power or LongRa RadioShuttle board for a second node/server
- A Windows, Mac or Linux computer
- An Arm Mbed OS account (sign up here: https://os.mbed.com)
- Tera Term terminal emulator (Windows only):  https://osdn.net/projects/ttssh2/releases
- RadioShuttle STM32 Utility installation https://www.radioshuttle.de/en/turtle-en/commissioning-en/)

## Supported develoment platforms
The software uses the Arm Mbed OS which is a free, open-source embedded operating system designed specifically for the Internet of Things. It includes all the features needed to develop a connected product based on an Arm Cortex-M microcontroller, including security, connectivity, an RTOS, and drivers for sensors and I/O devices. 

There are multiple options to develop with ARM Mbed OS:
- Mbed Online Compiler (professional web based IDE, the easiest way to get started)
- Mbed Studio IDE (professional desktop IDE for Mbed OS, at present in beta status)
- Mbed CLI (professional command line tool for Mbed OS)

The Mbed Online and the Studio IDE includes are ARM professional compiler free of charge (a 3000€ value). The Mbed CLI environment includes the free GNU GCC compiler toolchain. Optionally the Mbed CLI supports the Arm MDK or the IAR Compiler, booth subject to a fee.
 
Support for the Arm Mbed OS is provided by the Arm community support forum free of charge. This is the best place to discuss questions around Mbed OS. The Mbed OS website includes extensive documentation.

__NOTE:__ The Turtle board is compatible with the Mbed OS target __STM32L432KC__, therefore select the STM32L432KC target within Mbed OS.
 
 
 ##  Libraries utilized by the Turtle software
 The entire Turtle software consists of a main example applications and half a dozen of referenced libraries which are automatically included when importing the project into the Mbed Compiler URL: https://github.com/RadioShuttle/MBED_Turtle_RadioShuttle
  
  The detailed file sets are:
- Turtle example application (including main.cpp/h, PinMap.h, RadioTestSample.cpp and the Utils folder with additional support files)
- NVProperty.lib (a property storage system for storing non-volatile settings in MCU flash or OTP memory)
- MBED_USBDeviceHT (a USB device library from Mbed OS supporting virtual COMs, buffered)
- BufferedSerial.lib (a buffered serial implementation)
- SX1276GenericLib: (a common driver supporting SX1276 based modules)
- RadioShuttleLib.lib (RadioShuttle peer-to-peer LoRa wireless protocol software)
- HELIOS_Si7021.lib (a common driver supporting the Si7021 temperature and humidity sensor)
- OLED_SSD1306.lib (a common driver for the SSD1306 based OLED displays)

Most of the above library has been developed by the RadioShuttle.de team to support the RadioShuttle boards.

## The development procedure
The development is straightforward: basically compile the software, flash it onto the board, done. A USB connected terminal can be used to output messages. Here are some higher level steps to get new software for the board:

- Connect the Turtle board to the host computer via USB
- Open the RadioShuttle STM32 Utility
- Setup an Arm Mbed account
- Open the Online Compiler on https://www.mbed.com (alternatively open the Mbed Studio IDE)
- Select "Import", select "Click here" and enter the URL https://github.com/RadioShuttle/MBED_Turtle_RadioShuttle. This will import the Turtle RadioShuttle software and its connected libraries into the project
- Select the target "NUCLEO-L432KC" (this board is compatible with the Turtle board)
- Click on "Compile". After this is done a program.bin file is ready for flashing
- Turn the board into programming mode:
  Hold the "User" button and press the "Reset" button once, release the "User" button. Now the RadioShuttle STM32 Utility should show the connected board status
  - Drag & Drop the program.bin file into the STM32 Utility window. After it is done the board starts with the new program
  - Windows: Open Tera Term and connect to the virtual COM port
  - Mac: Select the terminal menu in the RadioShuttle STM32 Utility
Now the connected Termin will show the board startup messages

![Mbed Online Compiler](/docs/Mbed_Online_Compiler.png)
![STM32 Utilitiy](/docs/RadioShuttle_STM32_Utility.png)
![Tera Terminal](/docs/Tera_Term_Console.png)
![Tera Terminal](/docs/Mbed_Studio_IDE.png)

## License and contributions
This software is provided under the Apache 2.0 license, see individual file headers. Contributions to this project are accepted under the same license. All connected libraries (except RadioShuttle.lib) are under the Apache 2.0 license as well. The RadioShuttle.cpp of the RadioShuttle.lib is under copyright of Helmut Tschemernjak and is subject to a license fee for any use. RadioShuttle boards customers are already licensed.

One protocol source file, RadioShuttle.cpp, is under copyright of Helmut Tschemernjak and is subject to a license fee for any use. RadioShuttle boards customers are already licensed.

## Credits
This protocol implementation has initially been written by the RadioShuttle engineers (www.radioshuttle.de). Many thanks to everyone who helped bringing this project forward.


## Links
- RadioShuttle website: https://www.radioshuttle.de/
- Turtle board: https://www.radioshuttle.de/en/turtle-en/turtle-board-en/
- The RadioShuttle wireless protocol: https://www.radioshuttle.de/en/radioshuttle-en/protocol/
- Supported radio chips: https://github.com/RadioShuttle/SX1276GenericLib
- LoRa basics: https://www.radioshuttle.de/en/lora-en/basics/
- MQTT basics: https://www.radioshuttle.de/en/mqtt-en/the-basics/
- MQTT Push Client (app) https://www.radioshuttle.de/en/mqtt-en/mqtt-push-client-app/
