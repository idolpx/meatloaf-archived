// // 
// // WiC64 - C64 connected to ESP32
// // 
// // written by KiWI 2020-2022 
// // 
// // Using ESP32 Arduino Release 2.0.2 based on ESP-IDE v4.4
// //          WiC64 Hardware & Software - Copyright (c) 2021
// //
// //               Thomas "GMP" Müller <gmp@wic64.de>
// //             Sven Oliver "KiWi" Arke <kiwi@wic64.de>
// //          Hardy "Lazy Jones" Ullendahl <lazyjones@wic64.de>
// //             Henning "Yps" Harperath <yps@wic64.de>
// //
// //
// //          All rights reserved.
// //
// //Redistribution and use in source and binary forms, with or without
// //modification, are permitted provided that the following conditions are
// //met:
// //
// //1. Redistributions of source code must retain the above copyright
// //   notice, this list of conditions and the following disclaimer.
// //
// //2. Redistributions in binary form must reproduce the above copyright
// //   notice, this list of conditions and the following disclaimer in the
// //   documentation and/or other materials provided with the
// //   distribution.
// //
// //THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// //"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// //LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// //A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// //OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// //SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// //LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// //DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// //THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// //(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// //OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// //
// // C64 Pinout -> ESP Wroom >>30<< Pin Version 
// // 
// // https://www.c64-wiki.de/wiki/Userport
// //
// // https://randomnerdtutorials.com/getting-started-with-esp32/      30 Pin Version !!! Nicht die 36er !!!
// //
// //      C64   ESP32 // C64 Pin Number


// #define PB0   16    // PB0   = C  - Datenleitungen 0 bis 7 - C64 PIN Name <-> ESP32 Pin Nummer
// #define PB1   17    // PB1   = D
// #define PB2   18    // PB2   = E
// #define PB3   19    // PB3   = F
// #define PB4   21    // PB4   = H
// #define PB5   22    // PB5   = J
// #define PB6   23    // PB6   = K
// #define PB7   25    // PB7   = L
// #define PC2   14    // PC2   = 8  - Signal vom C64 zum NodeMCU - C64 löst PC2 IRQ aus wenn CIA Datenregister gelesen ODER geschrieben wird -
// #define PA2   27    // PA2   = M  - Signal vom C64 zum NodeMCU - LOW = NodeMCU darf an den C64 senden - HIGH = C64 sendet gerade zum NodeMCU (Poweron C64 = HIGH)
// #define FLAG2 26    // FLAG2 = B  - Signal vom NodeMCU zum C64 - geht kurz von HIGH auf LOW wenn ein Byte am Datenbus für den C64 anliegt - Löst IRQ beim C64 aus (Byte zum Abholen bereit)

// // Firmware compile options
// // firmware 1 = //define DEBUG - Core debug level = none
// // firmware 2 = define DEBUG   - Core debug level = Info
// // firmware 3 = define DEBUG   - Core debug level = Debug

// #define DEBUG            // Debug aktivieren - Hexprint der IO Daten
// #define LED_BUILTIN 2    // LED des ESP an Pin2 angeschlossen
// #define espbootbutton 0  // Taste Boot0 auf dem ESP32 Board
// #define specialbutton 33 // Spezialtaste auf WiC64 Board

// #include <WiFi.h>
// #include <WiFiGeneric.h>
// #include <Preferences.h>
// #include <WiFiUdp.h>
// #include "time.h"
// #include <WiFiClient.h>
// #include <WiFiClientSecure.h>
// #include <WebServer.h>
// #include <HTTPClient.h>
// #include <HTTPUpdate.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <Fonts/FreeSerif9pt7b.h>
// #include "wic64.h"
// #include "wic64webserver.h"
// #include "wic64display.h"
              

// void setup() {     
//       firmwareversion = "0033";
      
//       pinMode(PB0, INPUT);                  // Beim Kaltstart erst alle Pins auf Input setzten bis C64 die Kontrolle übernimmt
//       pinMode(PB1, INPUT);
//       pinMode(PB2, INPUT);
//       pinMode(PB3, INPUT);
//       pinMode(PB4, INPUT);
//       pinMode(PB5, INPUT);
//       pinMode(PB6, INPUT);
//       pinMode(PB7, INPUT);
//       pinMode(LED_BUILTIN,OUTPUT);          // LED kann geschrieben werden
//       digitalWrite(LED_BUILTIN, HIGH);      // Eingebaute LED auf dem Developer Board einschalten
//       inputmode=true; payload=""; payloadsize=0; 
//       count=0; transferdata = false;        // Beim Kaltstart alles auf INPUT schalten - nicht das beide Senden !
      
//       pinMode(espbootbutton, INPUT);        // Taste Boot0 auf dem ESP32 Board auf Eingabe 
//       pinMode(specialbutton, INPUT_PULLDOWN);        // Taste Boot0 auf dem ESP32 Board auf Eingabe 

//       pinMode(PA2, INPUT);                  // PC2 Signal sagt dem ESP32 ob er vom C64 lesen soll oder an den C64 senden darf - Signal bei Kaltstart HIGH = ESP soll LESEN
//       pinMode(PC2, INPUT);                  // PA2 Signal vom C64 an den ESP - LOW = Daten können abgeholt werden
 

//       pinMode(FLAG2, OUTPUT);
//       digitalWrite(FLAG2, HIGH);            // FLAG2 Signal vom ESP an den C64 - Löst beim C64 NMI aus = Handshake
    

//       Serial.begin(115200);                              // 115.200 Baud für Debugging output 
      
//       log_i("Version: " __DATE__ " " __TIME__ "\r\n");   // Compiler Variablen einsetzen - Damit die Build-Version einwandfrei identifiziert werden kann.
//       Wire.begin(I2C_SDA, I2C_SCL);                             // Display Pins festlegen und I2C Bus initialisieren
//       if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { log_i("SSD1306 allocation failed"); } else { display.clearDisplay();  display.drawBitmap(0, 0,  bitmap_wic64_2, 128, 64, WHITE); display.display(); displayattached=true;} // Hostnamen beim Booten auf Display ausgeben} // Prüfen ob Display angeschlossen wurde

//       preferences.begin("credentials", false);
//       displayrotate = preferences.getBool("displayrotate", false);            // Rotate Displa 180 degrees for U2 users 
//       killswitch = preferences.getBool("killswitch", false);
//       if (killswitch == true ) { log_i("kill true"); } else { log_i("kill false"); }
//       killled = preferences.getBool("killled", false);
//       if (killled == true ) { log_i("LED DISABLED !"); digitalWrite(LED_BUILTIN, LOW); } else { log_i("LED ENABLED !"); }     
//       gmtOffset_sec = preferences.getLong("gmtoffset", false);
//       log_i("boot time zone %i", gmtOffset_sec);
     
//       if (killswitch == false) {
//       WiFi.onEvent(WiC64connected, SYSTEM_EVENT_STA_CONNECTED);         // Displaystuff aufrufen beim reconnect zum WLAN
//       WiFi.onEvent(WiC64disconnected, SYSTEM_EVENT_STA_DISCONNECTED );  // Displaystuff aufrufen sollte das WLAN mal disconnecten (Router reboot/powerdown etc)
//       WiFi.onEvent(WiC64ipconfig, SYSTEM_EVENT_STA_GOT_IP );            // Displaystuff IP vom AP zugewiesen
//       attachInterrupt(digitalPinToInterrupt(PA2), PA2irq, CHANGE);      // ESP-IRQ Pins festlegen PA2 C64
//       attachInterrupt(digitalPinToInterrupt(PC2), PC2irq, HIGH);        // ESP-IRQ Pins festlegen PC2 C64
        
//       wic64hostname = "WiC64-" + WiFi.macAddress();     // ESP32 Hostnamen erstellen aus WiC64-MA:CA:DR:ES:SE
//       WiFi.mode(WIFI_STA);                              // STA Mode - An Accesspoint anmelden
//       WiFi.begin();                                     // WLAN starten
//       WiFi.setHostname(wic64hostname.c_str());          // WiC64 Hostnamen setzten damit er im WLAN / Netzwerk einwandfrei identifiziert werden kann
//       server.on("/", handleRoot);                       // Code anspringen der bei / root Aufruf des Webservers ausgeführt wird
//       server.on("/update", handleUpdate);               // ESP Autoupdate starten
//       server.on("/developer", handleDeveloper);         // ESP Autoupdate developer channel
//       server.on("/developer2", handleDeveloper2);       // ESP SECRET ! Autoupdate developer channel 
//       server.on("/downgrade", handleDowngrade  );       // ESP SECRET ! Autoupdate developer channel 
//       server.begin();                                   // Webserver starten

//       log_i( "Waiting for WiFi" );
//       for (int i = 0; i <50; i++) { if (WiFi.status() != WL_CONNECTED) { delay(50); } else { i=50; } }

//       if (WiFi.status() == WL_CONNECTED) {
      
//       log_i( "Connected to: SSID: %s IP: %s RSSI: %d", WiFi.SSID(), WiFi.localIP().toString().c_str(), WiFi.RSSI()  );                         // Print SSID + IP + RSSI level

       
//       setserver = preferences.getString("server", "");                  // default server name aus dem flash laden
//       if (setserver == "" ) { preferences.putString("server", defaultserver); setserver = defaultserver; log_i("Setserver saved: %s", setserver.c_str()); }
//       setsaveserver = preferences.getString("saveserver", "");                  // default server name aus dem flash laden
//       if (setsaveserver == "" ) { preferences.putString("saveserver", defaultsaveserver); setserver = defaultserver; log_i("Setserver saved: %s", setserver.c_str()); }
      
// //      payload.reserve(65535);
// //      payloadB.reserve(65535);
// //      input.reserve(1024);
// //      lastinput.reserve(1024);       // Memory leak Arduino bugfix - Beseitigt misteriöse Guru's des ESP32 RTOS
//       input="";

//        log_i( "Waiting for C64 to power on" );
//        for (int i = 0; i <50; i++) { if (digitalRead(PA2)==LOW) { delay(50); } else { i=50; log_i( "C64 powered on."); }  }

//        HTTPClient http;
//        WiFiClient client;
//        sectokenname=""; // No default security token name
       
//       display.clearDisplay();
//       displaystuff(wic64hostname);


//      } else 
//            { log_i("Not connected.");     }                     // WLan connect fehlgeschlagen
//       } else {
//         log_i("Killswitch set - Boot without wlan"); 
//         WiFi.mode(WIFI_OFF);
//         displaydisabled("DISABLED !");
//         pinMode(FLAG2, INPUT);
//           pinMode(specialbutton, INPUT);        // Taste Boot0 auf dem ESP32 Board auf Eingabe 
//       }
      
// }
       
// void loop(){
      
//       if (digitalRead(espbootbutton) == LOW) { buttontimeA++; } else { buttontimeA=0;} //Boot0 deaktivert das WiC64 am Userport - Z.B. wg SpeedDos / ProfDos Kollisionen
//       if (digitalRead(specialbutton) == HIGH){ buttontimeB++; } else { buttontimeBcount=buttontimeB; buttontimeB=0;}
//       if (buttontimeA >= 20) { 
//         buttontimeA=0; log_i("Boot0 pressed.");
//         if (killswitch == true){ killswitch = false; preferences.putBool("killswitch", killswitch); log_i("WiC64 ENABLED !");  ESP.restart(); } else { disablewic(); ESP.restart(); }
//       }
//       if (buttontimeBcount > 5 && buttontimeBcount < 20 ) { buttontimeBcount=0; log_i("SpecialButton 2 seconds pressed."); 
//         if (killled == true){ killled = false; preferences.putBool("killled", killled); log_i("LED ENABLED !");  } else { killled = true; preferences.putBool("killled", killled);  digitalWrite(LED_BUILTIN, LOW); log_i("LED DISABLED !"); }
//       }
//       if (buttontimeBcount > 30  ) { buttontimeBcount=0; log_i("SpecialButton 5 seconds pressed."); 
//         if (displayrotate == true){ displayrotate = false; preferences.putBool("displayrotate", displayrotate); log_i("Rotate display disabled !"); displaystuff("rotate off");  } else { displayrotate = true; preferences.putBool("displayrotate", displayrotate);   log_i("Rotate display enabled !"); displaystuff("rotate off"); }
//       }
//       buttontimeBcount=0;

//       if (killswitch == false) {

//       server.handleClient();                    // HTTP requests vom Webserver checken & ggf. beantworten  
//       if (millis() > timeout && transferdata == true)  { Serial.println(count);Serial.println(millis() - timeout); Serial.println(millis()); Serial.println(timeout); log_i("Transfer ESP -> C64 TIMEOUT %i",count ); transferdata = false; payloadsize=0; count=0;  }  // Timeout erreicht -  C64 holt die Daten nicht ab 
//       if (millis() > timeout && pending == false && input.length() > 0) {    log_i("\nTransfer C64 -> ESP TIMEOUT: %i", transsize); stringtohexdump(input); input=""; transsize=0; }  // Timeout erreicht -  Dateneingabe vom C64 nicht komplett Empfangen - daher Eingabepuffer von Schrott befreien 
//       if (lastinput.length() > 2 ) {      // Kommando muss mindestens 4 Byte haben - sonst Schrott empfangen
//         ex=false;
      
//       if (lastinput.startsWith ("W"))  {            // Commando startet mit W = Richtig


// #ifdef DEBUG    
//       Serial.print("Message from C64: "); stringtohexdump(lastinput); Serial.print("Message size: "); Serial.println(lastinput.length());
      
// #endif
//       if (lastinput.charAt(3) ==  0 )   { ex=true; displaystuff("get FW version"); sendmessage("WIC64FWV:" + firmwareversion ); }       
//       if (lastinput.charAt(3) ==  1 )   { ex=true; displaystuff("loading http"); loader(lastinput); if (messagetoc64 !="") { sendmessage(messagetoc64); } }
//       if (lastinput.charAt(3) ==  2 )   { ex=true; displaystuff("config wifi"); httpstring=lastinput;  sendmessage(setwlan()); delay(3000); displaystuff("config changed"); }
//       if (lastinput.charAt(3) ==  3 )   { ex=true; displaystuff("FW update 1"); handleUpdate(); }    // Normal SW update - no debug messages on serial
//       if (lastinput.charAt(3) ==  4 )   { ex=true; displaystuff("FW update 2"); handleDeveloper();   } // Developer SW update - debug output to serial
//       if (lastinput.charAt(3) ==  5 )   { ex=true; displaystuff("FW update 3"); handleDeveloper2();   } // Developer SW update - debug output to serial
//       if (lastinput.charAt(3) ==  6 )   { ex=true; displaystuff("get ip"); String ipaddress = WiFi.localIP().toString(); sendmessage(ipaddress); }
//       if (lastinput.charAt(3) ==  7 )   { ex=true; displaystuff("get stats"); String stats = __DATE__ " " __TIME__; sendmessage(stats); }
//       if (lastinput.charAt(3) ==  8 )   { ex=true; displaystuff("set server"); lastinput.remove(0,4); setserver=lastinput; preferences.putString("server", lastinput);  }
//       if (lastinput.charAt(3) ==  9 )   { ex=true; displaystuff("REM"); Serial.println(lastinput); } // REM Send messages to debug console.
//       if (lastinput.charAt(3) == 10 )   { ex=true; displaystuff("get upd"); sendmessage(getudpmsg());  } // Get UDP data and return them to c64
//       if (lastinput.charAt(3) == 11 )   { ex=true; displaystuff("send udp"); sendudpmsg(lastinput); } // Send UDP data to IP
//       if (lastinput.charAt(3) == 12 )   { ex=true; displaystuff("scanning wlan"); sendmessage(getWLAN()); } // wlan scanner
//       if (lastinput.charAt(3) == 13 )   { ex=true; displaystuff("config wifi id"); httpstring=lastinput; sendmessage(setWLAN_list()); displaystuff("config wifi set");} // wlan setup via scanlist
//       if (lastinput.charAt(3) == 14 )   { ex=true; displaystuff("change udp port"); httpstring=lastinput; startudpport(); }
//       if (lastinput.charAt(3) == 15 )   { ex=true; displaystuff("loading httpchat"); loader(lastinput); if (messagetoc64 !="") { sendmessage(messagetoc64); } } // Chatserver string decoding 
//       if (lastinput.charAt(3) == 16 )   { ex=true; displaystuff("get ssid"); sendmessage(WiFi.SSID()); }
//       if (lastinput.charAt(3) == 17 )   { ex=true; displaystuff("get rssi"); sendmessage(String( WiFi.RSSI() )); }
//       if (lastinput.charAt(3) == 18 )   { ex=true; displaystuff("get server"); if (setserver !="") {sendmessage(setserver);} else { sendmessage("no server set"); } }
//       if (lastinput.charAt(3) == 19 )   { ex=true; displaystuff("get external ip"); loader("XXXXhttp://sk.sx-64.de/wic64/ip.php"); if (messagetoc64 !="") { sendmessage(messagetoc64); } }  // XXXX 4 bytes header for padding !
//       if (lastinput.charAt(3) == 20 )   { ex=true; displaystuff("get mac"); sendmessage(WiFi.macAddress()); }
//       if (lastinput.charAt(3) == 21 )   { ex=true; displaystuff("get time and date"); getLocalTime(); sendmessage(acttime); }
//       if (lastinput.charAt(3) == 22 )   { ex=true; displaystuff("set timezone"); httpstring=lastinput; settimezone(); }
//       if (lastinput.charAt(3) == 23 )   { ex=true; displaystuff("get timezone"); sendmessage( String (gmtOffset_sec, DEC)); }
// #ifdef DEBUG    
//       if (lastinput.charAt(3) == 24 )   { ex=true; displaystuff("check update"); loader("XXXXhttp://sk.sx-64.de/wic64-d2/version.txt"); messagetoc64=payload; messagetoc64.remove(0,2); int v1=messagetoc64.toInt(); int v2 =firmwareversion.toInt(); if (v1 > v2) { sendmessage("2"); } else sendmessage("0"); }
// #else
//       if (lastinput.charAt(3) == 24 )   { ex=true; displaystuff("check update"); loader("XXXXhttp://sk.sx-64.de/wic64/version.txt"); messagetoc64=payload; messagetoc64.remove(0,2); int v1=messagetoc64.toInt(); int v2 =firmwareversion.toInt(); if (v1 > v2) { sendmessage("1"); } else sendmessage("0"); }
// #endif

//       if (lastinput.charAt(3) == 25 )   { ex=true; displaystuff("read prefs"); httpstring=lastinput;  sendmessage(getprefs()); }
//       if (lastinput.charAt(3) == 26 )   { ex=true; displaystuff("save prefs"); httpstring=lastinput;  sendmessage(setprefs()); }
            
//       if (lastinput.charAt(3) == 30 )   { ex=true; displaystuff("get tcp"); getudpmsg(); if (messagetoc64 !="") { sendmessage(messagetoc64); }  } // Get TCP data and return them to c64 INCOMPLETE
//       if (lastinput.charAt(3) == 31 )   { ex=true; displaystuff("send tcp"); sendudpmsg(lastinput); sendmessage("");  log_i("tcp send %s", lastinput); } // Get TCP data and return them to c64 INCOMPLETE
//       if (lastinput.charAt(3) == 32 )   { ex=true; displaystuff("set tcp port"); httpstring=lastinput; settcpport();  }
      
//       if (lastinput.charAt(3) == 33 )   { ex=true; displaystuff("connect tcp1"); sendmessage(connecttcp1());  }
//       if (lastinput.charAt(3) == 34 )   { ex=true; displaystuff("get tcp1"); sendmessage(gettcp1());  }
//       if (lastinput.charAt(3) == 35 )   { ex=true; displaystuff("send tcp1");  sendmessage(sendtcp1()); }
//       if (lastinput.charAt(3) == 36 )   { ex=true; displaystuff("httppost");  sendmessage(httppost()); }
//       if (lastinput.charAt(3) == 37 )   { ex=true; displaystuff("loading bighttp"); bigloader(lastinput); if (messagetoc64 !="") { sendmessage(messagetoc64); } }

//       if (lastinput.charAt(3) == 99 )   { ex=true; displaystuff("factory reset"); WiFi.begin("-","-"); WiFi.disconnect(true);   preferences.putString("server", defaultserver); preferences.putLong("gmtoffset", 0); display.clearDisplay(); delay (3000); ESP.restart();}
//       }

//         if (ex == false) { log_i("Command error.");  sendmessage("command error.");  } // Commando wurde nicht erkannt - String defekt ?
//         if (ex == true) { input=""; lastinput =""; transsize=0; crashcounter++; }
//         if (pending == true) { pending = false; handshake_flag2(); delay(200); timeout=millis()+2000; }
        
//         if (payloadsize > 0 && pending == false) handshake_flag2(); // Transfer anschieben 
//         } 
        
        
//       } // Ende von Killswitch
//       delay (100);
      
// } // Ende von LOOP
     
// /////// Subs IRQ

// void IRAM_ATTR PA2irq() {                                              // PA2 vom C64 schaltet den ESP auf Input oder Output Modus

//   if (digitalRead(PA2)==LOW) {       
//       pinMode(PB0, OUTPUT);
//       pinMode(PB1, OUTPUT);
//       pinMode(PB2, OUTPUT);
//       pinMode(PB3, OUTPUT);
//       pinMode(PB4, OUTPUT);
//       pinMode(PB5, OUTPUT);
//       pinMode(PB6, OUTPUT);
//       pinMode(PB7, OUTPUT);
//       digitalWrite(LED_BUILTIN, LOW);      // Eingebaute LED auf dem Developer Board ausschalten
//       count=0; inputmode=false;
     
//       } else  {
  
//       pinMode(PB0, INPUT);
//       pinMode(PB1, INPUT);
//       pinMode(PB2, INPUT);
//       pinMode(PB3, INPUT);
//       pinMode(PB4, INPUT);
//       pinMode(PB5, INPUT);
//       pinMode(PB6, INPUT);
//       pinMode(PB7, INPUT);
//       if (killled == false) { digitalWrite(LED_BUILTIN, HIGH); }     // Eingebaute LED auf dem Developer Board einschalten
//       count=0; inputmode=true; transsize=0;
//       payload=""; payloadsize=0; transferdata= false;
      
//       }
// }
// void IRAM_ATTR PC2irq() {                     // PC2 IRQ wurde ausgelöst weil C64 vom Userpot gelesen hat oder Daten geschrieben hat
//     if (inputmode == true){           // C64 sendet an den ESP32
//             databyte = 0;
//             if ((GPIO.in >> PB0) & 1) databyte |= 1UL << 0;       // Datenleitungen lesen und in databyte speichern
//             if ((GPIO.in >> PB1) & 1) databyte |= 1UL << 1;
//             if ((GPIO.in >> PB2) & 1) databyte |= 1UL << 2;
//             if ((GPIO.in >> PB3) & 1) databyte |= 1UL << 3;
//             if ((GPIO.in >> PB4) & 1) databyte |= 1UL << 4;
//             if ((GPIO.in >> PB5) & 1) databyte |= 1UL << 5;
//             if ((GPIO.in >> PB6) & 1) databyte |= 1UL << 6;
//             if ((GPIO.in >> PB7) & 1) databyte |= 1UL << 7;
//             timeout=millis()+2000;                                // Timeout 2 Sekunden - Wenn Timeout erreicht wurde und die Daten vom C64 nicht komplett Empfangen wurden ist dieser ggf. Abgestürtz - daher Eingabepuffer von Schrott befreien 
            
//             input += char(databyte);                              // Byte empfangen und zu input hinzufügen
//             if ((input.length() == 3)  && (input.startsWith ("W"))) { unsigned int sizeh = input.charAt(2); unsigned int sizel = input.charAt(1); transsize = sizeh*256+ sizel; }      // Transfergröße der Daten vom C64 ermitteln
//             if ((input.length() > 1) && (input.length() == transsize)) { lastinput=input; input=""; pending = true; }     // Anzahl der zu erwartendne Bytes Empfangen -  Kommando in den Ausführungspuffer kopieren und Eingabepuffer löschen
//             if (transsize > 1 && (input.length() > transsize)) { Serial.println("send data overflow !"); Serial.println(transsize); Serial.println(input.length());}
//             if (pending == false) { handshake_flag2(); }                                  // Handshake an C64 das Byte vom Userport gelesen wurde - es sei denn es ist das letzte - dann pending = C64 warten lassen !
//            }   // Inputmode 1 = Send data from C64 to ESP
            
            
//       if (inputmode == false){      // C64 empfängt vom ESP32 {
//         if (transferdata == true) { // Nur wenn Daten zum Transfer vorhanden sind 
         
//         timeout=millis()+2000;
//         if (count <= payloadsize) {      
//         databyte = payload[count];
//         if (databyte & (1 << 0)) GPIO.out_w1ts = (1 << PB0); else GPIO.out_w1tc = (1 << PB0);
//         if (databyte & (1 << 1)) GPIO.out_w1ts = (1 << PB1); else GPIO.out_w1tc = (1 << PB1);
//         if (databyte & (1 << 2)) GPIO.out_w1ts = (1 << PB2); else GPIO.out_w1tc = (1 << PB2);
//         if (databyte & (1 << 3)) GPIO.out_w1ts = (1 << PB3); else GPIO.out_w1tc = (1 << PB3);
//         if (databyte & (1 << 4)) GPIO.out_w1ts = (1 << PB4); else GPIO.out_w1tc = (1 << PB4);
//         if (databyte & (1 << 5)) GPIO.out_w1ts = (1 << PB5); else GPIO.out_w1tc = (1 << PB5);
//         if (databyte & (1 << 6)) GPIO.out_w1ts = (1 << PB6); else GPIO.out_w1tc = (1 << PB6);
//         if (databyte & (1 << 7)) GPIO.out_w1ts = (1 << PB7); else GPIO.out_w1tc = (1 << PB7);
//         count++;  total++;
//         }  else {log_i("Request Overflow ! Buffer end: %d of %d - S: %d PL2: %d PL1: %d",count,total,swap,pay2size,payloadsize); }     // C64 requested mehr Bytes als im Puffer sind - daher am Ende des Puffers bleiben !

//         if (count == payloadsize ) { log_i("Buffer end: %d of %d - S: %d PL2: %d",count,total,swap,pay2size);    // Puffer wurde komplett übertragen 
//         if (pay2size >0) { payload=payloadB; payloadsize=pay2size; pay2size=0; payloadB=""; count=0; if (swap==2) {swap=1; log_i("swapped"); }  if (swap==3) {swap=3; log_i("End of file"); } } // 
//         }

//         if (count == payloadsize && swap == 3 && pay2size == 0) { payloadsize=0; count=0; transferdata = false; log_i("Bytes sent: %i",total); }
//         handshake_flag2();                                  // Handshake an C64 das Byte vom Userport abgeholt werden kann

//         }
//       }     // Inputmode false = Receive from ESP
//   }

// void handshake_flag2()
// {
//     GPIO.out_w1ts = (1 << FLAG2);         // Flag2 flippen um am C64 FLAG IRQ auszulösen - Der holt dann Daten ab oder darf neue Daten an den Bus anlegen
//     delayMicroseconds(5);
//     GPIO.out_w1tc = (1 << FLAG2);
// }   


// /////// Subs Kommandos HTTP & Error codes 

// void sendmessage(String messagetoc64 ){    // Aus messagetoc64 einen String machen der dann als Antwort auf das gesendete Kommando an den C64 gesendet wird (Load error etc.)

//     int dsize = messagetoc64.length();
//     log_i("Datasize %i", dsize);
    
//     String datasize;
//     char c = ((dsize) / 256); datasize += c;
//     c = ((dsize) % 256); datasize += c;
//     payload = datasize + messagetoc64; count=0; payloadsize = payload.length(); 
//     if (pending == true) { pending = false; handshake_flag2(); delay(200); timeout=millis()+2000; } 
//     transferdata = true; 
// #ifdef DEBUG    
//       Serial.print("Message to C64: "); stringtohexdump(payload);
// #endif
//     messagetoc64 ="";
// }

// void disablewic(){
//      preferences.putBool("killswitch", true);
//      log_i("WiC64 DISABLED !");  
//      ESP.restart();
// }

// String setwlan(){                         // Seperator $01 - !! ACHTUNG !! SENSIBLE DATEN WIE PASSWÖRTER AUF SERIELLER SCHNITTSTELLE
       
//         lastinput.remove(0,4);
//         ssiddata = getValue(lastinput, sep, 0);
//         passworddata = getValue(lastinput, sep, 1); passworddata = convertspecial(passworddata);
//         log_i("WLAN Config updated: %s - %s ", ssiddata, passworddata); 
//         ssiddata.toCharArray(ssid, ssiddata.length()+1 );
//         passworddata.toCharArray(password, passworddata.length()+1 );
//         if (ssiddata != "") { WiFi.begin(ssid, password); log_i("ssid: %s password: %s",ssid , password); delay(3000); return "Wlan config changed";} else { WiFi.begin(); delay (3000); return "Wlan config not changed";}
// }


// void loader(String httpstring) {         // Daten via HTTP laden

//         WiFiClientSecure clientsec;
//         messagetoc64 = "";
//         byte command = httpstring[3];
//         httpstring.remove(0,4);
//         swap=3; total=0;
                
//         prefstring1 = getValue(lastinput, 36, 1);  // 36 = $VARIABLE$ - Upper case = C64 !
//         prefstring2 = getValue(lastinput, 36, 3);  // 36 = $
//         prefstring3 = getValue(lastinput, 36, 5);  // 36 = $
//         if (prefstring1.length() > 0) { prefstring1data = preferences.getString(prefstring1.c_str()); }
//         if (prefstring2.length() > 0) { prefstring2data = preferences.getString(prefstring2.c_str()); }
//         if (prefstring3.length() > 0) { prefstring3data = preferences.getString(prefstring3.c_str()); }
//         if (prefstring1.length() > 0 && prefstring1data.length() >0 ) { httpstring.replace("$"+prefstring1+"$", prefstring1data ); }
//         if (prefstring2.length() > 0 && prefstring1data.length() >0 ) { httpstring.replace("$"+prefstring2+"$", prefstring2data ); }
//         if (prefstring3.length() > 0 && prefstring1data.length() >0 ) { httpstring.replace("$"+prefstring3+"$", prefstring3data ); }
        
//         if (command == 15) { httpstring = en_code(httpstring); }     //Strings für Chatserver umkodieren - Hardy
        
//         httpstring.replace("%ser", setserver);  // Im Kommando ein %ser durch Serveradresse ersetzen die vorher mit "sets" Kommando gesetzt wurde (C64 Strings abkürzen -> LazyJones Wunsch
//         if (httpstring.startsWith ("!"))  {httpstring.replace("!", setserver); }  // Wenn ein http request mit ! beginnt wird das ! durch den default server ersetzt -> load "!xxx.prg" = "http://www.wic64.de/prg/xxx.prg"
        
//         if(httpstring.indexOf("%mac") > 0) {
//         sectoken = preferences.getString(sectokenname.c_str());
//         String tempmac = WiFi.macAddress(); tempmac.replace(":",""); httpstring.replace("%mac", tempmac + sectoken); // Im Kommando ein %mac durch die MAC Adresse des ESP ersetzen XX:XX.XX.XX.XX.XX
//         }
        
//         log_i("[HTTP] begin. Counter %i ", crashcounter);
//         if (httpstring.startsWith ("https")) {
//         clientsec.setInsecure();
//         httpInitResult = http.begin(clientsec, httpstring);     //HTTPS init
//         } else {
//         httpInitResult = http.begin(client, httpstring);     //HTTP init
//         }

//         if (httpInitResult == true) {
//         http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);    

//         log_i("HTTP Request: %s", httpstring.c_str());

//         int httpCode = http.GET();          // start connection and send HTTP header
//         log_i("[HTTP] Returncode %d", httpCode);
//         if(httpCode == 201) { log_i("201 detected !"); checkprefs=true; httpCode = 200; }                    // Server sendet Code 201 = Preferences Anweisung !
//         if(httpCode == 200) {
//                 String databuffer = http.getString(); 

//                 if (checkprefs == true) { checkprefs=false; databuffer = setprefsphp(databuffer); } 
                
//                 int dsize = databuffer.length();
//                     if (httpstring.endsWith (".prg")) {   dsize--; dsize--;  log_i("--> PRG load detected "); }  //  Ladeadresse am Anfang soll der C64 nicht mitzählen beim Laden 2 Byte x y Schleife
//                 String datasize;
//                 char c = ((dsize) / 256); datasize += c;
//                      c = ((dsize) % 256); datasize += c;
//                 payload = datasize;
//                 payload += databuffer;
//                 count=0; payloadsize = payload.length();
//                 if (pending == true) { pending = false; handshake_flag2(); delay(200); timeout=millis()+2000;}  transferdata = true; 
//                 if (command == 24) { transferdata = false; } // http nicht via irq übertragen - Sonderfunktion für Firmware-Check 
//                 log_i("Dataload: %d - Payload: %d ",dsize,payloadsize);
                
// #ifdef DEBUG    
//                 Serial.print("Payload message to C64: "); stringtohexdump(payload);
// #endif
                
                
//                           } else { messagetoc64 = "!0"; } 

//         clientsec.stop(); http.end();
//                 } else { messagetoc64 = "!0"; }
//         log_i("Loop counter: %d" , crashcounter);

//  }


// void sendudpmsg(String udpmsg){       // UDP: IP IP IP IP DATAPART

//       udpmsg.remove(0,4);                 // UPDS abschneiden
//       IPAddress remote = {udpmsg[0],udpmsg[1],udpmsg[2],udpmsg[3]};
//       udpmsg.remove(0,4);                 // IP Adresse abschneiden
//       for (int i = 0; i < udpmsg.length(); i++) { buffer[i] = char(udpmsg[i]); }
//       udp.beginPacket(remote, udpport);
//       udp.write(buffer, udpmsg.length());
//       udp.endPacket();
//       memset(buffer, 0, 50);
//   }


// String getudpmsg(){
//      memset(buffer, 0, 50);
//      udpmsgr = ""; messagetoc64 = "";
//      int UdppacketSize = udp.parsePacket();
//      if(udp.read(buffer, 50) > 0){
//      IPAddress remote = udp.remoteIP();
//      int dsize = UdppacketSize + 4;
//      payload = String("") + char(remote[0])+ char(remote[1]) + char(remote[2]) + char(remote[3]);
//      for (int i = 0; i < UdppacketSize; i++) { payload += char(buffer[i]); }

//      payloadsize = payload.length();
//      if (pending == true) { pending = false; handshake_flag2(); delay(200);  timeout=millis()+2000; }  transferdata = true; 
//      return payload;
     
//      } else { 
//       payload = ""; //String("") +char(0)+char(0);
//         return payload;
//      }
// #ifdef DEBUG
//       Serial.print("UDP data received: "); stringtohexdump(payload);
// #endif
//   }

// void startudpport(){
//      unsigned int udpporth = lastinput.charAt(5); unsigned int udpportl = lastinput.charAt(4); udpport = udpporth*256+ udpportl;
//      udp.begin(udpport);                               // UDP Netzwerk aktivieren
//      log_i("udp port %i", udpport);
// }

// void settcpport(){
//      unsigned int tcpporth = lastinput.charAt(5); unsigned int tcpportl = lastinput.charAt(4); tcpport = tcpporth*256+ tcpportl;
//      log_i("tcp port %i", tcpport);
// }

// String getValue(String data, char separator, int index) {         // String auseinanderschneiden nach Seperatorzeichen und Nummer
    
//     int found = 0; int strIndex[] = { 0, -1 }; int maxIndex = data.length() - 1;
//     for (int i = 0; i <= maxIndex && found <= index; i++) {
//         if (data.charAt(i) == separator || i == maxIndex) {
//             found++;
//             strIndex[0] = strIndex[1] + 1;
//             strIndex[1] = (i == maxIndex) ? i+1 : i;
//         }
//     }
//     return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
// }

// void stringtohexdump(String data){
//     Serial.println("");
//     Serial.print("Hexdata: ");
//     for (int i = 0; i < data.length(); i++) { if (i > 79) {break;} if(data[i] <= 15) {Serial.print("0");} Serial.print(data[i],HEX); Serial.print(" "); }
//     Serial.println("");
//     Serial.print("Ascdata: ");
//     for (int i = 0; i < data.length(); i++) { if (i > 79) {break;} if(data[i] <= 30) {Serial.print("$$ "); } else { Serial.print(" "); Serial.print(data[i]); Serial.print(" "); } }
//     Serial.println("");
//     Serial.println("---");
// #ifdef DEBUG    
//       log_i("Free ESP memory: %i",ESP.getFreeHeap() );
//       log_i("---");
// #endif
    

// }



// String getWLAN(){
// WiFi.disconnect(true);
// delay(100);
// int n = WiFi.scanNetworks();
// String wlan;

// if (n>15){n=15;} // Max. Anzahl Wlans in Liste

// if (n == 0) {return "no networks found";}
// else        {for (int i = 0; i < n; ++i) {wlan += i; wlan += sep; wlan += WiFi.SSID(i); wlan += sep; wlan += WiFi.RSSI(i); wlan += sep;}
//              return wlan;}
  
// }

// String setWLAN_list(){                             // !! ACHTUNG !! SENSIBLE DATEN WIE PASSWÖRTER AUF SERIELLER SCHNITTSTELLE

//    lastinput.remove(0,4); // Wegschneiden Befehl
//    char nr = lastinput.charAt(0)-48; Serial.print("Nummer:"); Serial.println(nr,HEX);
//    lastinput.remove(0,1); // Wegschneiden Netzwerknummer - jetzt bleibt nur noch das Kennwort über.
//    passworddata = getValue(lastinput, sep, 1); passworddata = convertspecial(passworddata);
//    ssiddata = WiFi.SSID(nr); Serial.println(ssiddata);
//    log_i("WLAN Config updated. SSID: %s Password: %s", ssiddata, passworddata); 
//    ssiddata.toCharArray(ssid, ssiddata.length()+1 );
//    passworddata.toCharArray(password, passworddata.length()+1 );
//    if (ssiddata != "")  {WiFi.begin(ssid, password); log_i("WLAN Config updated. SSID: %s Password: %s", ssid, password);  delay(3000); return "Wlan config changed"; } else { WiFi.begin(); delay (3000); return "Wlan config not changed";} 
// }


// String en_code(String httpstring) // Hardy begin
// {

// // Wenn im HTTP String etwas codiert werden soll:
//         String httpstring_code = "";
//         int laenge;
//         bool codiert = false;
//         String hexcode;

//         // loop über jedes Zeichen von httpstring und Codierung fals erforderlich
//         for (int i = 0; i < httpstring.length(); i++) 
//         { 
//           //Wenn die Escapesequenz <$ gefunden wird
//           if(httpstring[i] == 60 and httpstring[i+1] == 36) // Codieren einschalten
//           {
//             codiert = true;
//             // ermittlung der nächsten zwei Bytes (Low und High der längenangabe)
//             laenge = httpstring[i+2];
//             laenge = laenge + (256*httpstring[i+3]);

//             // Schleife über die zu codierenden Zeichen  
//             for (int x = 0; x < laenge; x++) 
//             {
//                 hexcode = String(httpstring[i+x+4], HEX);
                
//                 if (httpstring[i+x+4] <= 15)
//                   {
//                     httpstring_code += char(48);
//                     httpstring_code += char(hexcode[0]);
//                   }
//                 else
//                   {
//                     httpstring_code += char(hexcode[0]);
//                     httpstring_code += char(hexcode[1]);
//                   }


//             }
//             // den Schleifenzähler nach der Codierschleife übernehmen in die große Schleife.
//             i=i+laenge+3;
//           }
//           else
//           {
//             // Wenn nicht codiert werden soll die Zeichen einfach so übernehmen.
//             httpstring_code += httpstring[i];
//           }
//         }

//         // Wenn die Codierung abeschlossen ist, das Codierte im HTTP String ersetzten:
//         if (codiert == true)
//         {   
//            httpstring=httpstring_code;
//         }

//         return httpstring;
// }


// String convertspecial(String passworddata) {

// String temp = passworddata;
// for (int i=0; i < 1; ) {

// int textposition =  temp.indexOf("~");
// if (textposition == -1 ) i++;
// String cut = temp;
// cut.remove(0,textposition);
// int cutend = cut.length();
// cut.remove(3,(cutend - 3));  // !33
// String hex = cut;
// hex.remove(0,1);
// char a = hex.charAt(0);
// char b = hex.charAt(1);
// a = toupper(a);
// b = toupper(b);
// int high = (a >= 'A') ? a - 'A' + 10 : a - '0';
// int low = (b >= 'A') ? b - 'A' + 10 : b - '0';
// char outvalue =(high*16+low);
// String outchar(outvalue);
// temp.replace(cut,outchar);
// }
// return (temp);
// }

// void getLocalTime()
// {
//   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
//   struct tm timeinfo;
//   if(!getLocalTime(&timeinfo)){
//     log_i("Failed to obtain time");
//     return;
//   }
//    char timeStringBuff[25];
//    strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S %d-%m-%Y", &timeinfo);
//    acttime = timeStringBuff;
//    log_i("Time successfully obtained");
// }

// void settimezone(){
//      unsigned int timezonel = lastinput.charAt(4); unsigned int timezoneh = lastinput.charAt(5); 
//      unsigned int timezone = timezoneh*10 + timezonel;
//      int long timezones[] = {0,0,3600,7200,7200,10800,12600,14400,18000,19800,21600,25200,28800,32400,34200,36000,39600,43200,
//                              -39600,-36000,-32400,-28800,-25200,-25200,-21600,-18000,-18000,-14400,-12600,-10800,-10800,-3600}; 
     
//      gmtOffset_sec = timezones[timezone];
//      preferences.putLong("gmtoffset", gmtOffset_sec);
//      getLocalTime();
//      log_i("time zone %i", gmtOffset_sec);
// }


// String getprefs(){                         // Seperator $01
       
//         lastinput.remove(0,4);
//         prefname = getValue(lastinput, sep, 0);
//         prefdata = preferences.getString(prefname.c_str());
//         log_i("Reading prefs name: %s value %s:", prefname, prefdata.c_str() ); 
//         return (prefdata);
// }

// String setprefs(){                         // Seperator $01
        
//         lastinput.remove(0,4); 
//         prefname = getValue(lastinput, sep, 0);
//         prefdata = getValue(lastinput, sep, 1); 
//         preferences.putString(prefname.c_str(), prefdata); 
//         log_i("Saving prefs name: %s value %s:", prefname, prefdata.c_str() ); 
//         return ("");
// }
// String setprefsphp(String lastinput){                         // Seperator $01
//         prefname   = getValue(lastinput, sep, 1);
//         prefdata   = getValue(lastinput, sep, 2);
//         prefanswer = getValue(lastinput, sep, 3);  
//         preferences.putString(prefname.c_str(), prefdata);
//         if (prefname == "sectokenname")  { sectokenname = prefdata; }
//         log_i("Saving PHP prefs name: %s value %s:", prefname, prefdata.c_str() ); 
//         return (prefanswer);
// }

// String connecttcp1() {
//         lastinput.remove(0,4);
//         server1 = getValue(lastinput, 58, 0);  // 58 = :
//         port1 = getValue(lastinput, 58, 1).toInt();  lastinput="";
//         Serial.print("Server: ");Serial.println(server1);
//         Serial.print("Port: ");Serial.println(port1);
//         if (client.connect(server1.c_str() , port1, 3000)) { return "0"; } else { return "!E"; }
  
// }

// String gettcp1() {
//         String databuffer;
//         unsigned long gettimeout = millis();
//         while (millis() - gettimeout < 500)  {   // 1 Sekunden laufen lassen

//           if (client.available()) {
//             char c = client.read();
//             databuffer += c;
            
//           }
//         }
        
//         return databuffer;
// }

// String sendtcp1() {
//   lastinput.remove(0,4);
//   lastinput.toCharArray(tcpanswer,lastinput.length() + 1);
//   if (client.write(tcpanswer, lastinput.length() + 1)) { return "0"; } else { return "!E"; }
// }

// String httppost() {
//   messagetoc64="";
//   WiFiClient client;
//    lastinput.remove(0,4);
//    if (lastinput.startsWith ("!"))  {lastinput.remove(0,1); String tempinput = lastinput; lastinput=setsaveserver+tempinput; }
//    stringtohexdump(lastinput);
//    server1      = getValue(lastinput, 58, 0);  // 58 = :
//    port1        = getValue(lastinput, 58, 1).toInt();
//    serverpath1  = getValue(lastinput, 58, 2);  // 58 = :
//    filename     = getValue(lastinput, 58, 3);  // 58 = : 
//    int datacut = lastinput.indexOf(char(1));
//    lastinput.remove(0,datacut+1);  
//    String httpboundary="WiC64"+String (millis(),HEX) + "WiC64";

//   if (client.connect(server1.c_str(), port1)) {
//     String head = "--"+httpboundary+"\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\""+filename+"\"\r\nContent-Type: image/jpeg\r\r\n\r\n";
//     String tail = "\r\n--"+httpboundary+"--\r\n";
//     client.println("POST "+serverpath1+" HTTP/1.1\r\nHost: "+server1 );
//     client.println("Content-Length: "+String(lastinput.length()+head.length()+tail.length() ));
//     client.println("Content-Type: multipart/form-data; boundary="+httpboundary+"\r\n");
//     client.print(head);
//     client.print (lastinput); 
//     client.print(tail);

//     while (client.connected()) {
//       String line = client.readStringUntil('\n');
//       if (line == "\r") {break;}
//      }
//      while (client.available()) {
//       messagetoc64 += char(client.read());
//      }
      
//     client.stop();
//     lastinput="";
//     return messagetoc64;
    
//   } else { return "!E"; }
// } 

// void bigloader(String httpstring) {         // Daten via HTTP laden

//         WiFiClientSecure clientsec;
//         HTTPClient http;
//         WiFiClient client;  
//         messagetoc64 = "";
//         byte command = httpstring[3];
//         httpstring.remove(0,4);
               
//         prefstring1 = getValue(lastinput, 36, 1);  // 36 = $VARIABLE$ - Upper case = C64 !
//         prefstring2 = getValue(lastinput, 36, 3);  // 36 = $
//         prefstring3 = getValue(lastinput, 36, 5);  // 36 = $
//         if (prefstring1.length() > 0) { prefstring1data = preferences.getString(prefstring1.c_str()); }
//         if (prefstring2.length() > 0) { prefstring2data = preferences.getString(prefstring2.c_str()); }
//         if (prefstring3.length() > 0) { prefstring3data = preferences.getString(prefstring3.c_str()); }
//         if (prefstring1.length() > 0 && prefstring1data.length() >0 ) { httpstring.replace("$"+prefstring1+"$", prefstring1data ); }
//         if (prefstring2.length() > 0 && prefstring1data.length() >0 ) { httpstring.replace("$"+prefstring2+"$", prefstring2data ); }
//         if (prefstring3.length() > 0 && prefstring1data.length() >0 ) { httpstring.replace("$"+prefstring3+"$", prefstring3data ); }
       
//         httpstring.replace("%ser", setserver);  // Im Kommando ein %ser durch Serveradresse ersetzen die vorher mit "sets" Kommando gesetzt wurde (C64 Strings abkürzen -> LazyJones Wunsch
//         if (httpstring.startsWith ("!"))  {httpstring.replace("!", setserver); }  // Wenn ein http request mit ! beginnt wird das ! durch den default server ersetzt -> load "!xxx.prg" = "http://www.wic64.de/prg/xxx.prg"
        
//         if(httpstring.indexOf("%mac") > 0) {
//         sectoken = preferences.getString(sectokenname.c_str());
//         String tempmac = WiFi.macAddress(); tempmac.replace(":",""); httpstring.replace("%mac", tempmac + sectoken); // Im Kommando ein %mac durch die MAC Adresse des ESP ersetzen XX:XX.XX.XX.XX.XX
//         }
        
//         log_i("[Large HTTP] begin. Counter %i ", crashcounter);
//         if (httpstring.startsWith ("https")) {
//         clientsec.setInsecure();
//         httpInitResult = http.begin(clientsec, httpstring);     //HTTPS init
//         } else {
//         httpInitResult = http.begin(client, httpstring);     //HTTP init
//         }

//         if (httpInitResult == true) {
//         http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);  

//         log_i("HTTP Request: %s", httpstring.c_str());

//         int httpCode = http.GET();          // start connection and send HTTP header
//         log_i("[HTTP] Returncode %d", httpCode);
//         if(httpCode == 200) {
                

                                
//                 swap=0; total=0;
//                 int len = http.getSize();
//                 buff[65535] = { 0 };   // create buffer for read
//                 String datasize;
//                 char c = ((len) / 16777216); datasize += c;
//                      c = ((len) / 65536); datasize += c;
//                      c = ((len) / 256); datasize += c; 
//                      c = ((len) % 256); datasize += c;

//                 buff[65535] = { 0 };   // create buffer for read
//                       int fr=0;
                
//                 WiFiClient * stream = http.getStreamPtr(); // get tcp stream
//                 while(http.connected() && (len > 0 || len == -1)) {    // read all data from server

//                     int size = stream->available();  // get available data size
//                     if(size) { // read up to 128 byte
//                         int c = stream->readBytes(buff, 50000);
//                         //int c = stream->readBytes(buff, ((size > 65530) ? 65530 : size*2));
//                         if (c>0) {
//                         if (swap==0) { payload = datasize; for (int i = 0; i < c; i++) { payload+=char(buff[i]); } count=0; payloadsize = c+4; transferdata = true; fr=1; }
//                         while (pay2size != 0 ) { delay(10); } // Warten bis der 2. Buffer leer ist
//                         if (swap==1 ||swap==3) { payloadB = ""; for (int i = 0; i < c; i++) { payloadB+=char(buff[i]); } pay2size = c; swap=2; log_i("PayB filled");} // pay2size = payloadB.length();
//                         }
//                         if (fr==1) { fr=2;  swap=1; } 
//                         if(len > 0) { len -= c; }
//                         if(len < 1) { swap=3; log_i("All data received"); } 
//                         if (len > 65535 && fr ==2) { fr=3; log_i("both buffers filled");
//                         if (pending == true) { pending = false; handshake_flag2(); delay(200); timeout=millis()+2000; }  // C64 ggf. aus Senderoutine rausholen
//                         }
//                         if (payloadsize > 0 && pending == false) handshake_flag2(); // C64 Transfer starten
//                         log_i("loop");
//                         }
//                     delay(1);
                    
//                     } 
//                     swap=3;  
//                     log_i("Big dataload ended: %d - Payload: %d ",len,payloadsize);                
                   
                
//                           } else { messagetoc64 = "!0"; } 

//         clientsec.stop(); client.stop(); http.end();
//                 } else { messagetoc64 = "!0"; }
//         log_i("Loop counter: %d" , crashcounter);
// }
  
