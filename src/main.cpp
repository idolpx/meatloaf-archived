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
#endif

#include "global_defines.h"
#include "global_fs.h"

#include "meat_io.h"

#include "iec.h"
#include "iec_device.h"
#include "ESPModem.h"
//#include "ml_httpd.h"
#include "ml_tests.h"

//MLHttpd www;

//void IRAM_ATTR isrCheckATN();

//#include "IECBus/Drive.h"
//#include "IECBus/DiskImage.h"
//#include "IECBus/DiskCaddy.h"

//#include "zimodem/zimodem.h"

String statusMessage;
bool initFailed = false;

static IEC iec;
static Interface drive ( iec, fileSystem );

// Drive drive;
// DiskImage diskImage;
// DiskCaddy diskCaddy;

//Zimodem modem;
ESPModem modem;

// #if defined(ESP8266)
// ADC_MODE ( ADC_VCC ); // Set ADC for Voltage Monitoring
// #endif

// enum class statemachine
// {
//     none,
//     check_atn
// };
// statemachine state = statemachine::none;
//uint8_t state_int;
//String state_string;


// ------------------------
void setup()
{

    delay ( 1000 );

    // Setup Modem
    modem.setup();


    Serial.println ( "\r\n\r\n==============================" );
    Serial.println ( "   " PRODUCT_ID " " FW_VERSION );
    Serial.println ( "------------------------------" );

    modem.start();

#if defined(ESP8266)
    // initialize selected file system
    fileSystemConfig.setAutoFormat ( AUTO_FORMAT );
#if defined USE_SDFS
    fileSystemConfig.setCSPin ( CHIP_SELECT_PIN );
    fileSystemConfig.setSPI ( SPI_SETTINGS );
#endif
    fileSystem->setConfig ( fileSystemConfig );
#endif

#if defined(ESP8266)

    if ( !fileSystem->begin() )
#elif defined(ESP32) && defined(USE_LITTLEFS)
    if ( !LITTLEFS.begin() ) // not sure why pointer is not working
#else
    if ( !SPIFFS.begin() ) // not sure why pointer is not working
#endif
    {
        // File System failed
        statusMessage = "Failed to initialize file system";
        Serial.print ( "ERROR: " );
        Serial.println ( statusMessage );
        initFailed = true;
    }
    else
    {
        Serial.println ( "Flash File System started" );

        // Start the Web Server with WebDAV
        //www.setup();


        // mDNS INIT
        if ( MDNS.begin ( HOSTNAME ) )
        {
            MDNS.addService ( "http", "tcp", SERVER_PORT );
            Serial.println ( "mDNS service started" );
            Serial.println ( ">>> http://" HOSTNAME ".local" );
        }
        else
        {
            Serial.println ( "mDNS service failed to start" );
            Serial.print ( ">>> http://" );
            Serial.println ( WiFi.localIP() );
        }

        // Setup IEC Bus
        // iec.enabledDevices = DEVICE_MASK;
        // iec.enableDevice(30);
        // iec.init();
        // Serial.println("IEC Bus Initialized");

        // drive.begin();
        // Serial.print("Virtual Device(s) Started: [ ");
        // for (byte i = 0; i < 31; i++)
        // {
        //  if (iec.isDeviceEnabled(i))
        //  {
        //      Serial.printf("%.02d ", i);
        //  }
        // }
        // Serial.println("]");
    }

    // // Setup callbacks for SerialCommand commands
    // cli.addDefaultHandler(unrecognized);     // Handler for command that isn't matched  (says "What?")

    // cli.addCommand("on",LED_on);             // Turns LED on
    // cli.addCommand("off",LED_off);           // Turns LED off
    // cli.addCommand("hello",SayHello);        // Echos the string argument back
    // cli.addCommand("p",process_command);     // Converts two arguments to integers and echos them back

    // cli.addCommand("$", listDirectory);
    // cli.addCommand("iec", iecCommand);

    // cli.addCommand("cat", catFile);
    // cli.addCommand("help", showHelp);

    //  attachInterrupt(digitalPinToInterrupt(IEC_PIN_ATN), isrCheckATN, FALLING);
    //  attachInterrupt(digitalPinToInterrupt(IEC_PIN_ATN), isrATNRising, RISING);

    pinMode ( LED_PIN, OUTPUT ); // Configure the onboard LED for output
    ledON();
    Serial.println ( "READY." );

    runTestsSuite();
}

// ------------------------
void loop()
{
#if defined(ESP8266)
    MDNS.update();
#endif

    //www.handleClient();

    //cli.readSerial();
    modem.service();

    //drive.loop();
}

// void isrCheckATN()
// {
//  state = statemachine::check_atn;
//  iec.init();
// }




