// Meatloaf - A Commodore 64/128 multi-device emulator
// https://github.com/idolpx/meatloaf
// Copyright(C) 2020 James Johnston
//
// Meatloaf is free software : you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Meatloaf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Meatloaf. If not, see <http://www.gnu.org/licenses/>.


#if defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//#include <ESP8266WebServer.h>
#endif

#include <ESPWebDAV.h>
//#include <WebDav4WebServer.h>

#include "global_defines.h"

// Setup FileSystem Object
#if defined(USE_SPIFFS)
	#if defined(ESP32)
		#include <SPIFFS.h>
	#endif
	#include <FS.h>
	FS* fileSystem = &SPIFFS;
	
	#if defined(ESP8266)
		SPIFFSConfig fileSystemConfig = SPIFFSConfig();
	#endif
#elif defined USE_LITTLEFS
	#if defined(ESP8266)
		#include <LittleFS.h>
		FS* fileSystem = &LittleFS;
		LittleFSConfig fileSystemConfig = LittleFSConfig();	
	#endif
	#if defined(ESP32)
		#include <LITTLEFS.h>
		FS* fileSystem = &LITTLEFS;
	#endif
#elif defined USE_SDFS
	#include <SDFS.h>
	#define CHIP_SELECT_PIN	15
	#define SPI_SETTINGS SPI_FULL_SPEED
	FS* fileSystem = &SDFS;
	SDFSConfig fileSystemConfig = SDFSConfig();	
#else
	#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif

#include "iec.h"
#include "iec_device.h"
#include "ESPModem.h"
#include "meat_io.h"

//void ICACHE_RAM_ATTR isrCheckATN();

//#include "IECBus/Drive.h"
//#include "IECBus/DiskImage.h"
//#include "IECBus/DiskCaddy.h"

//#include "zimodem/zimodem.h"

//ESP8266WebServer server(80);
//ESPWebDAVCore dav;
WiFiServer tcp(80);
ESPWebDAV dav;

String statusMessage;
bool initFailed = false;

static IEC iec;
static Interface drive(iec, fileSystem);

// Drive drive;
// DiskImage diskImage;
// DiskCaddy diskCaddy;

//Zimodem modem;
ESPModem modem;

#if defined(ESP8266)
ADC_MODE(ADC_VCC); // Set ADC for Voltage Monitoring
#endif

// enum class statemachine
// {
//     none,
//     check_atn
// };
// statemachine state = statemachine::none;
//uint8_t state_int;
//String state_string;

void testLittleFS();

// ------------------------
void setup()
{

	delay(1000);

	// Setup Modem
	modem.setup();


	Serial.printf("\r\n\r\n==============================\r\n");
	Serial.printf("   %s %s\r\n", PRODUCT_ID, FW_VERSION);
	Serial.println("------------------------------");

	modem.start();

#if defined(ESP8266)
	// initialize selected file system
	fileSystemConfig.setAutoFormat(AUTO_FORMAT);
#if defined USE_SDFS
	fileSystemConfig.setCSPin(CHIP_SELECT_PIN);
	fileSystemConfig.setSPI(SPI_SETTINGS);
#endif
	fileSystem->setConfig(fileSystemConfig);
#endif

#if defined(ESP8266)
	if (!fileSystem->begin())
#elif defined(ESP32) && defined(USE_LITTLEFS)
	if (!LITTLEFS.begin()) // not sure why pointer is not working
#else
	if (!SPIFFS.begin()) // not sure why pointer is not working
#endif
	{
		// File System failed
		statusMessage = "Failed to initialize file system";
		Serial.print("ERROR: ");
		Serial.println(statusMessage);
		initFailed = true;
	}
	else
	{
		Serial.println("Flash File System started");

		// start the WebDAV server
		tcp.begin();
		dav.begin(&tcp, fileSystem);
		dav.setTransferStatusCallback([](const char* name, int percent, bool receive)
		{
			Serial.printf("WebDAV %s: '%s': %d%%\n", receive ? "recv" : "send", name, percent);
		});
		//server.addHook(hookWebDAVForWebserver("/.www", dav));
		//server.begin();

		// if (!dav.init(SERVER_PORT, fileSystem))
		// {
		// 	Serial.println("ERROR: WebDAV Server failed");
		// 	initFailed = true;
		// }
		// else
		{
			//Serial.println("HTTP server started");
			Serial.println("WebDAV server started");

			// mDNS INIT
			if (MDNS.begin(HOSTNAME))
			{
				MDNS.addService("http", "tcp", SERVER_PORT);
				Serial.println("mDNS service started");
				Serial.print(">>> http://");
				Serial.print(HOSTNAME);
				Serial.println(".local");
			}
			else
			{
				Serial.println("mDNS service failed to start");
				Serial.print(">>> http://");
				Serial.println(WiFi.localIP());
			}
		}


	// initialize LittleFS for file operations
	MFSOwner::mount("/");

	testLittleFS();


		// Setup IEC Bus
		iec.enabledDevices = DEVICE_MASK;
		iec.init();
		Serial.println("IEC Bus Initialized");

		drive.begin();
		Serial.print("Virtual Device(s) Started: [ ");
		for (byte i = 0; i < 31; i++)
		{
			if (iec.isDeviceEnabled(i))
			{
				Serial.printf("%.02d ", i);
			}
		}
		Serial.println("]");

		// // Set initial d64 image
		// Dir disk = fileSystem->openDir("/UTILS/FB64.d64");
		// if (diskCaddy.Insert(disk, false))
		// {
		// 	Debug_printf("Disk Mounted: %s", disk.fileName().c_str());
		// }

		// Serial.println("==================================");

		// File testFile = fileSystem->open(DEVICE_DB, "r");
		// if (testFile){
		// 	Serial.println("Read file content!");
		// 	/**
		// 	 * File derivate from Stream so you can use all Stream method
		// 	 * readBytes, findUntil, parseInt, println etc
		// 	 */
		// 	Serial.println(testFile.readString());
		// 	testFile.close();
		// }else{
		// 	Serial.println("Problem on read file!");
		// }

		// testFile = fileSystem->open(DEVICE_DB, "r");
		// if (testFile){
		// 	/**
		// 	 * mode is SeekSet, position is set to offset bytes from the beginning.
		// 	 * mode is SeekCur, current position is moved by offset bytes.
		// 	 * mode is SeekEnd, position is set to offset bytes from the end of the file.
		// 	 * Returns true if position was set successfully.
		// 	 */
		// 	Serial.println("Position inside the file at 9 byte!");
		// 	testFile.seek(9, SeekSet);

		// 	Serial.println("Read file content!");
		// 	Serial.println(testFile.readStringUntil('\0'));
		// 	testFile.close();
		// }else{
		// 	Serial.println("Problem on read file!");
		// }
	}

	// // Setup callbacks for SerialCommand commands
	// cli.addDefaultHandler(unrecognized);  	// Handler for command that isn't matched  (says "What?")

	// cli.addCommand("on",LED_on);          	// Turns LED on
	// cli.addCommand("off",LED_off);        	// Turns LED off
	// cli.addCommand("hello",SayHello);     	// Echos the string argument back
	// cli.addCommand("p",process_command);  	// Converts two arguments to integers and echos them back

	// cli.addCommand("$", listDirectory);
	// cli.addCommand("iec", iecCommand);

	// cli.addCommand("cat", catFile);
	// cli.addCommand("help", showHelp);

	//	attachInterrupt(digitalPinToInterrupt(IEC_PIN_ATN), isrCheckATN, FALLING);
	//	attachInterrupt(digitalPinToInterrupt(IEC_PIN_ATN), isrATNRising, RISING);

	pinMode(LED_PIN, OUTPUT); // Configure the onboard LED for output
	ledON();
	Serial.println("READY.");
}

// ------------------------
void loop()
{
#if defined(ESP8266)
	MDNS.update();
#endif

	// if (dav.isClientWaiting())
	// {
	// 	if (initFailed)
	// 		return dav.rejectClient(statusMessage);

	// 	// call handle if server was initialized properly
	// 	dav.handleClient();
	// }
	//server.handleClient();
	dav.handleClient();

	//cli.readSerial();
	modem.service();
	
	drive.loop();

}

// void isrCheckATN()
// {
// 	state = statemachine::check_atn;
// 	iec.init();
// }

