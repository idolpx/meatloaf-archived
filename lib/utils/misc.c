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

#include "misc.h"

// void LED_on()
// {
//   Debug_printv("LED on");
//   digitalWrite(LED_PIN, LED_ON);
// }

// void LED_off()
// {
//   Debug_printv("LED off");
//   digitalWrite(LED_PIN, LED_OFF);
// }

// void SayHello()
// {
//   char *arg;
//   arg = cli.next();    // Get the next argument from the SerialCommand object buffer
//   if (arg != NULL)      // As long as it existed, take it
//   {
//     Serial.print("Hello ");
//     Debug_printv(arg);
//   }
//   else {
//     Debug_printv("Hello, whoever you are");
//   }
// }

// void process_command()
// {
//   uint8_t aNumber;
//   char *arg;

//   Debug_printv("We're in process_command");
//   arg = cli.next();
//   if (arg != NULL)
//   {
//     aNumber=atoi(arg);    // Converts a char string to an integer
//     Serial.print("First argument was: ");
//     Debug_printv(aNumber);
//   }
//   else {
//     Debug_printv("No arguments");
//   }

//   arg = cli.next();
//   if (arg != NULL)
//   {
//     aNumber=atol(arg);
//     Serial.print("Second argument was: ");
//     Debug_printv(aNumber);
//   }
//   else {
//     Debug_printv("No second argument");
//   }

// }

// // // This gets set as the default handler, and gets called when no other command matches.
// void unrecognized()
// {
//   Debug_printv("What?");
// }

// void listDirectory()
// {
// 	Dir dir = fileSystem->openDir("/");
// 	// or Dir dir = LittleFS.openDir("/data");
// 	while (dir.next()) {
// 		//Serial.print(dir.fileName());
// 		if(dir.fileSize()) {
// 			File f = dir.openFile("r");
// 			Debug_printf("%s\t%d\r\n", dir.fileName().c_str(), (f.size()/256));
// 		}
// 		else
// 		{
// 			Debug_printf("%s\r\n", dir.fileName().c_str());
// 		}
// 	}
// }

// void iecCommand()
// {
//   char *arg;
//   arg = cli.next();    // Get the next argument from the SerialCommand object buffer

//   if (strcmp_P(arg, "init"))
//   {
// 	  iec.init();
// 	  Serial.printf_P("IEC Interface initialized\r\n");
//   }

// }

// void readFile(char *filename)
// {
// 	uint16_t i;
// 	char b[1];

// 	File file = fileSystem->open(filename, "r");
// 	if (!file.available())
// 	{
// 		Debug_printf("\r\nFile Not Found: %s\r\n", filename);
// 	}
// 	else
// 	{
// 		size_t len = file.size();
// 		Debug_printf("\r\n[%s] (%d bytes)\r\n================================\r\n", filename, len);
// 		for(i = 0; i < len; i++) {
// 			file.readBytes(b, sizeof(b));
// 			Serial.print(b);
// 		}
// 		file.close();
// 	}
// } // readFile

// void catFile()
// {
//   	readFile(cli.next());
// } // catFile

// void showHelp()
// {
//   	readFile((char *)"/.sys/help.txt");
// } // showHelp
