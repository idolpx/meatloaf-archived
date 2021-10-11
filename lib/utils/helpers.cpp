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

#include "helpers.h"


unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

void printProgress(uint16_t total, uint16_t current)
{
    if ( current == 0 )
        Serial.println("");

    uint16_t percentage = (current / total) * 100;
    if ( percentage % 10 != 0 )       
        Serial.printf("[%-10s] %d, %d, %d\r\n", "=", percentage, current, total);

}

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (uint8_t i =0; i < str.length(); i++){
      c=str.charAt(i);
    //   if (c == ' '){
    //     encodedString+= '+';
    //   } else 
      if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}

String urldecode(String str)
{
    
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (uint8_t i =0; i < str.length(); i++){
        c=str.charAt(i);
      if (c == '+'){
        encodedString+=' ';  
      }else if (c == '%') {
        i++;
        code0=str.charAt(i);
        i++;
        code1=str.charAt(i);
        c = (h2int(code0) << 4) | h2int(code1);
        encodedString+=c;
      } else{
        
        encodedString+=c;  
      }
      
      yield();
    }
    
   return encodedString;
}

/** IP to String? */
String ipToString ( IPAddress ip )
{
    String res = "";

    for ( uint8_t i = 0; i < 3; i++ )
    {
        res += String ( ( ip >> ( 8 * i ) ) & 0xFF ) + ".";
    }

    res += String ( ( ( ip >> 8 * 3 ) ) & 0xFF );
    return res;
}

String formatBytes ( size_t bytes )
{
    if ( bytes < 1024 )
    {
        return String ( bytes ) + " B";
    }
    else if ( bytes < ( 1024 * 1024 ) )
    {
        return String ( bytes / 1024.0 ) + " KB";
    }
    else if ( bytes < ( 1024 * 1024 * 1024 ) )
    {
        return String ( bytes / 1024.0 / 1024.0 ) + " MB";
    }
    else
    {
        return String ( bytes / 1024.0 / 1024.0 / 1024.0 ) + " GB";
    }
}

// String readLine(FS *fileSystem, String filename)
// {
// 	uint16_t i;
// 	char b[1];
//     String line;

// 	File file = fileSystem->open(filename, "r");
// 	if (!file.available())
// 	{
// 		Serial.printf("\r\nFile Not Found: %s\r\n", filename.c_str());
// 	}
// 	else
// 	{
// 		size_t len = file.size();
//         line.reserve(len);
// 		for(i = 0; i < len; i++) {
// 			line += (char)file.read();
// 		}
// 		file.close();
// 	}
//     return line;
// } // readLine

// void readFile(FS *fileSystem, String filename)
// {
// 	uint16_t i;
// 	char b[1];

// 	File file = fileSystem->open(filename, "r");
// 	if (!file.available())
// 	{
// 		Serial.printf("\r\nFile Not Found: %s\r\n", filename.c_str());
// 	}
// 	else
// 	{
// 		size_t len = file.size();
// 		Serial.printf("\r\n[%s] (%d bytes)\r\n================================\r\n", filename.c_str(), len);
// 		for(i = 0; i < len; i++) {
// 			file.readBytes(b, sizeof(b));
// 			Serial.print(b);
// 		}
// 		file.close();
// 	}
// } // readFile