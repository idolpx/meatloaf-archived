/*
   WiFi SIXFOUR - A virtual WiFi modem based on the ESP 8266 chipset
   Copyright (C) 2016 Paul Rickards <rickards@gmail.com>
   Added EEPROM read/write, status/help pages, busy answering of incoming calls
   uses the readily available Sparkfun ESP8266 WiFi Shield which has 5v level
   shifters and 3.3v voltage regulation present-- easily connect to a C64
   https://www.sparkfun.com/products/13287

   based on
   ESP8266 based virtual modem
   Copyright (C) 2016 Jussi Salin <salinjus@gmail.com>

   https://github.com/jsalin/esp8266_modem

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ESPModem.h"

uint16_t tcpServerPort = LISTEN_PORT;
WiFiClient tcpClient;
WiFiServer tcpServer(tcpServerPort);

String ESPModem::formatBytes ( size_t bytes )
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

String ESPModem::connectTimeString() {
  unsigned long now = millis();
  int secs = (now - connectTime) / 1000;
  int mins = secs / 60;
  int hours = mins / 60;
  String out = "";
  if (hours < 10) out.concat("0");
  out.concat(String(hours));
  out.concat(":");
  if (mins % 60 < 10) out.concat("0");
  out.concat(String(mins % 60));
  out.concat(":");
  if (secs % 60 < 10) out.concat("0");
  out.concat(String(secs % 60));
  return out;
}

void ESPModem::writeSettings() {
  setEEPROM(ssid, SSID_ADDRESS, SSID_LEN);
  setEEPROM(password, PASS_ADDRESS, PASS_LEN);
  setEEPROM(busyMsg, BUSY_MSG_ADDRESS, BUSY_MSG_LEN);

  EEPROM.write(BAUD_ADDRESS, serialspeed);
  EEPROM.write(ECHO_ADDRESS, byte(echo));
  EEPROM.write(AUTO_ANSWEResult_ADDRESS, byte(autoAnswer));
  EEPROM.write(SERVEResult_PORT_ADDRESS, highByte(tcpServerPort));
  EEPROM.write(SERVEResult_PORT_ADDRESS + 1, lowByte(tcpServerPort));
  EEPROM.write(TELNET_ADDRESS, byte(telnet));
  EEPROM.write(VERBOSE_ADDRESS, byte(verboseResults));
  EEPROM.write(PET_TRANSLATE_ADDRESS, byte(petTranslate));
  EEPROM.write(FLOW_CONTROL_ADDRESS, byte(flowControl));
  EEPROM.write(PIN_POLARITY_ADDRESS, byte(pinPolarity));

  for (int i = 0; i < 10; i++) {
    setEEPROM(speedDials[i], speedDialAddresses[i], 50);
  }
  EEPROM.commit();
}

void ESPModem::readSettings() {
  echo = EEPROM.read(ECHO_ADDRESS);
  autoAnswer = EEPROM.read(AUTO_ANSWEResult_ADDRESS);
  // serialspeed = EEPROM.read(BAUD_ADDRESS);

  ssid = getEEPROM(SSID_ADDRESS, SSID_LEN).c_str();
  password = getEEPROM(PASS_ADDRESS, PASS_LEN).c_str();
  busyMsg = getEEPROM(BUSY_MSG_ADDRESS, BUSY_MSG_LEN).c_str();
  tcpServerPort = word(EEPROM.read(SERVEResult_PORT_ADDRESS), EEPROM.read(SERVEResult_PORT_ADDRESS + 1));
  telnet = EEPROM.read(TELNET_ADDRESS);
  verboseResults = EEPROM.read(VERBOSE_ADDRESS);
  petTranslate = EEPROM.read(PET_TRANSLATE_ADDRESS);
  flowControl = EEPROM.read(FLOW_CONTROL_ADDRESS);
  pinPolarity = EEPROM.read(PIN_POLARITY_ADDRESS);

  for (int i = 0; i < 10; i++) {
    speedDials[i] = getEEPROM(speedDialAddresses[i], 50);
  }
}

void ESPModem::defaultEEPROM() {
  EEPROM.write(VERSION_ADDRESS, VERSIONA);
  EEPROM.write(VERSION_ADDRESS + 1, VERSIONB);

  setEEPROM("", SSID_ADDRESS, SSID_LEN);
  setEEPROM("", PASS_ADDRESS, PASS_LEN);
  setEEPROM("d", IP_TYPE_ADDRESS, 1);
  EEPROM.write(SERVEResult_PORT_ADDRESS, highByte(LISTEN_PORT));
  EEPROM.write(SERVEResult_PORT_ADDRESS + 1, lowByte(LISTEN_PORT));

  EEPROM.write(BAUD_ADDRESS, 0x02); // 2400
  EEPROM.write(ECHO_ADDRESS, 0x01);
  EEPROM.write(AUTO_ANSWEResult_ADDRESS, 0x01);
  EEPROM.write(TELNET_ADDRESS, 0x00);
  EEPROM.write(VERBOSE_ADDRESS, 0x01);
  EEPROM.write(PET_TRANSLATE_ADDRESS, 0x00);
  EEPROM.write(FLOW_CONTROL_ADDRESS, 0x00);
  EEPROM.write(PIN_POLARITY_ADDRESS, 0x01);

  setEEPROM("bbs.fozztexx.com:23", speedDialAddresses[0], 50);
  setEEPROM("cottonwoodbbs.dyndns.org:6502", speedDialAddresses[1], 50);
  setEEPROM("borderlinebbs.dyndns.org:6400", speedDialAddresses[2], 50);
  setEEPROM("particlesbbs.dyndns.org:6400", speedDialAddresses[3], 50);
  setEEPROM("reflections.servebbs.com:23", speedDialAddresses[4], 50);
  setEEPROM("heatwavebbs.com:9640", speedDialAddresses[5], 50);

  for (int i = 5; i < 10; i++) {
    setEEPROM("", speedDialAddresses[i], 50);
  }

  setEEPROM("SORRY, SYSTEM IS CURRENTLY BUSY. PLEASE TRY AGAIN LATER.", BUSY_MSG_ADDRESS, BUSY_MSG_LEN);
  EEPROM.commit();
}

String ESPModem::getEEPROM(int startAddress, int len) {
  String myString;

  for (int i = startAddress; i < startAddress + len; i++) {
    if (EEPROM.read(i) == 0x00) {
      break;
    }
    myString += char(EEPROM.read(i));
    //Serial.print(char(EEPROM.read(i)));
  }
  //Serial.println();
  return myString;
}

void ESPModem::setEEPROM(String inString, unsigned int startAddress, unsigned int maxLen) {
  for (unsigned int i = startAddress; i < inString.length() + startAddress; i++) {
    EEPROM.write(i, inString[i - startAddress]);
    //Serial.print(i, DEC); Serial.print(": "); Serial.println(inString[i - startAddress]);
    //if (EEPROM.read(i) != inString[i - startAddress]) { Serial.print(" (!)"); }
    //Serial.println();
  }
  // null pad the remainder of the memory space
  for (unsigned int i = inString.length() + startAddress; i < maxLen + startAddress; i++) {
    EEPROM.write(i, 0x00);
    //Serial.print(i, DEC); Serial.println(": 0x00");
  }
}

void ESPModem::sendResult(int resultCode) {
  Serial.print("\r\n");
  if (verboseResults == 0) {
    Serial.println(resultCode);
    return;
  }
  if (resultCode == Result_CONNECT) {
    Serial.print(String(resultCodes[Result_CONNECT]) + " " + String(bauds[serialspeed]));
  } else if (resultCode == Result_NOCARRIER) {
    Serial.print(String(resultCodes[Result_NOCARRIER]) + " (" + connectTimeString() + ")");
  } else {
    Serial.print(String(resultCodes[resultCode]));
  }
  Serial.print("\r\n");
}

void ESPModem::sendString(String msg) {
  Serial.print("\r\n");
  Serial.print(msg);
  Serial.print("\r\n");
}

// Hold for 5 seconds to switch to 300 baud
// Slow flash: keep holding
// Fast flash: let go
// int ESPModem::checkButton() {
//   long time = millis();
//   while (digitalRead(SWITCH_PIN) == LOW && millis() - time < 5000) {
//     delay(250);
//     digitalWrite(LED_PIN, !digitalRead(LED_PIN));
//     yield();
//   }
//   if (millis() - time > 5000) {
//     Serial.flush();
//     Serial.end();
//     serialspeed = 2; // 2400
//     delay(100);
//     Serial.begin(bauds[serialspeed]);
//     sendResult(Result_OK);
//     while (digitalRead(SWITCH_PIN) == LOW) {
//       delay(50);
//       digitalWrite(LED_PIN, !digitalRead(LED_PIN));
//       yield();
//     }
//     return 1;
//   } else {
//     return 0;
//   }
// }

void ESPModem::connectWiFi() {
  if (ssid == "" || password == "") {
    Serial.println("CONFIGURE SSID AND PASSWORD. TYPE AT? FOR HELP.");
    return;
  }
#if defined(ESP32)
  WiFi.begin(ssid.c_str(), password.c_str());
#elif defined(ESP8266)
  WiFi.begin(ssid, password);
#endif
  Serial.print("\nCONNECTING TO SSID "); Serial.print(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) {
    digitalWrite(LED_PIN, LOW);
    delay(250);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (i == 21) {
    Serial.print("COULD NOT CONNECT TO "); Serial.println(ssid);
    WiFi.disconnect();
    updateLed();
  } else {
    Serial.print("CONNECTED TO "); Serial.println(WiFi.SSID());
    Serial.print("IP ADDRESS: "); Serial.println(WiFi.localIP());
    updateLed();
  }
}

void ESPModem::updateLed() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_PIN, LOW);  // on
  } else {
    digitalWrite(LED_PIN, HIGH); //off
  }
}

void ESPModem::disconnectWiFi() {
  WiFi.disconnect();
  updateLed();
}

void ESPModem::setBaudRate(int inSpeed) {
  if (inSpeed == 0) {
    sendResult(Result_ERROR);
    return;
  }
  int foundBaud = -1;
  for (unsigned long i = 0; i < sizeof(bauds); i++) {
    if (inSpeed == bauds[i]) {
      foundBaud = i;
      break;
    }
  }
  // requested baud rate not found, return error
  if (foundBaud == -1) {
    sendResult(Result_ERROR);
    return;
  }
  if (foundBaud == serialspeed) {
    sendResult(Result_OK);
    return;
  }
  Serial.print("SWITCHING SERIAL PORT TO ");
  Serial.print(inSpeed);
  Serial.println(" IN 5 SECONDS");
  delay(5000);
  Serial.end();
  delay(200);
  Serial.begin(bauds[foundBaud]);
  serialspeed = foundBaud;
  delay(200);
  sendResult(Result_OK);
}

void ESPModem::setCarrier(byte carrier) {
  if (pinPolarity == P_NORMAL) carrier = !carrier;
  digitalWrite(DCD_PIN, carrier);
}

void ESPModem::displayNetworkStatus() {
  Serial.print("WIFI STATUS: ");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("CONNECTED");
  }
  if (WiFi.status() == WL_IDLE_STATUS) {
    Serial.println("OFFLINE");
  }
  if (WiFi.status() == WL_CONNECT_FAILED) {
    Serial.println("CONNECT FAILED");
  }
  if (WiFi.status() == WL_NO_SSID_AVAIL) {
    Serial.println("SSID UNAVAILABLE");
  }
  if (WiFi.status() == WL_CONNECTION_LOST) {
    Serial.println("CONNECTION LOST");
  }
  if (WiFi.status() == WL_DISCONNECTED) {
    Serial.println("DISCONNECTED");
  }
  if (WiFi.status() == WL_SCAN_COMPLETED) {
    Serial.println("SCAN COMPLETED");
  }
  yield();

  Serial.print("SSID.......: "); Serial.println(WiFi.SSID());

  //  Serial.print("ENCRYPTION: ");
  //  switch(WiFi.encryptionType()) {
  //    case 2:
  //      Serial.println("TKIP (WPA)");
  //      break;
  //    case 5:
  //      Serial.println("WEP");
  //      break;
  //    case 4:
  //      Serial.println("CCMP (WPA)");
  //      break;
  //    case 7:
  //      Serial.println("NONE");
  //      break;
  //    case 8:
  //      Serial.println("AUTO");
  //      break;
  //    default:
  //      Serial.println("UNKNOWN");
  //      break;
  //  }

  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC ADDRESS: ");
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);
  yield();

  Serial.print("IP ADDRESS.: "); Serial.println(WiFi.localIP()); yield();
  Serial.print("GATEWAY....: "); Serial.println(WiFi.gatewayIP()); yield();
  Serial.print("SUBNET MASK: "); Serial.println(WiFi.subnetMask()); yield();
  Serial.print("SERVER PORT: "); Serial.println(tcpServerPort); yield();
  Serial.print("WEB CONFIG.: HTTP://"); Serial.println(WiFi.localIP()); yield();
  Serial.print("CALL STATUS: "); yield();
  if (callConnected) {
    Serial.print("CONNECTED TO "); Serial.println(ipToString(tcpClient.remoteIP())); yield();
    Serial.print("CALL LENGTH: "); Serial.println(connectTimeString()); yield();
  } else {
    Serial.println("NOT CONNECTED");
  }
}

void ESPModem::displayCurrentSettings() {
  Serial.println("ACTIVE PROFILE:"); yield();
  Serial.print("BAUD: "); Serial.println(bauds[serialspeed]); yield();
  Serial.print("SSID: "); Serial.println(ssid); yield();
  Serial.print("PASS: "); Serial.println(password); yield();
  //Serial.print("SERVER TCP PORT: "); Serial.println(tcpServerPort); yield();
  Serial.print("BUSY MSG: "); Serial.println(busyMsg); yield();
  Serial.print("E"); Serial.print(echo); Serial.print(" "); yield();
  Serial.print("V"); Serial.print(verboseResults); Serial.print(" "); yield();
  Serial.print("&K"); Serial.print(flowControl); Serial.print(" "); yield();
  Serial.print("&P"); Serial.print(pinPolarity); Serial.print(" "); yield();
  Serial.print("NET"); Serial.print(telnet); Serial.print(" "); yield();
  Serial.print("PET"); Serial.print(petTranslate); Serial.print(" "); yield();
  Serial.print("S0:"); Serial.print(autoAnswer); Serial.print(" "); yield();
  Serial.println(); yield();

  Serial.println("SPEED DIAL:");
  for (int i = 0; i < 10; i++) {
    Serial.print(i); Serial.print(": "); Serial.println(speedDials[i]);
    yield();
  }
  Serial.println();
}

void ESPModem::displayStoredSettings() {
  Serial.println("STORED PROFILE:"); yield();
  Serial.print("BAUD: "); Serial.println(bauds[EEPROM.read(BAUD_ADDRESS)]); yield();
  Serial.print("SSID: "); Serial.println(getEEPROM(SSID_ADDRESS, SSID_LEN)); yield();
  Serial.print("PASS: "); Serial.println(getEEPROM(PASS_ADDRESS, PASS_LEN)); yield();
  //Serial.print("SERVER TCP PORT: "); Serial.println(word(EEPROM.read(SERVEResult_PORT_ADDRESS), EEPROM.read(SERVEResult_PORT_ADDRESS+1))); yield();
  Serial.print("BUSY MSG: "); Serial.println(getEEPROM(BUSY_MSG_ADDRESS, BUSY_MSG_LEN)); yield();
  Serial.print("E"); Serial.print(EEPROM.read(ECHO_ADDRESS)); Serial.print(" "); yield();
  Serial.print("V"); Serial.print(EEPROM.read(VERBOSE_ADDRESS)); Serial.print(" "); yield();
  Serial.print("&K"); Serial.print(EEPROM.read(FLOW_CONTROL_ADDRESS)); Serial.print(" "); yield();
  Serial.print("&P"); Serial.print(EEPROM.read(PIN_POLARITY_ADDRESS)); Serial.print(" "); yield();
  Serial.print("NET"); Serial.print(EEPROM.read(TELNET_ADDRESS)); Serial.print(" "); yield();
  Serial.print("PET"); Serial.print(EEPROM.read(PET_TRANSLATE_ADDRESS)); Serial.print(" "); yield();
  Serial.print("S0:"); Serial.print(EEPROM.read(AUTO_ANSWEResult_ADDRESS)); Serial.print(" "); yield();
  Serial.println(); yield();

  Serial.println("STORED SPEED DIAL:");
  for (int i = 0; i < 10; i++) {
    Serial.print(i); Serial.print(": "); Serial.println(getEEPROM(speedDialAddresses[i], 50));
    yield();
  }
  Serial.println();
}

void ESPModem::waitForSpace() {
  Serial.print("PRESS SPACE");
  char c = 0;
  while (c != 0x20) {
    if (Serial.available() > 0) {
      c = Serial.read();
      if (petTranslate == true){
        if (c > 127) c-= 128;
      }
    }
  }
  Serial.print("\r");
}

void ESPModem::displayHelp() {
  //welcome();
  Serial.println("AT COMMAND SUMMARY:"); yield();
  Serial.println("DIAL HOST.....: ATDTHOST:PORT"); yield();
  Serial.println("SPEED DIAL....: ATDSN (N=0-9)"); yield();
  Serial.println("SET SPEED DIAL: AT&ZN=HOST:PORT (N=0-9)"); yield();
  Serial.println("HANDLE TELNET.: ATNETN (N=0,1)"); yield();
  Serial.println("PET MCTERM TR.: ATPETN (N=0,1)"); yield();
  Serial.println("NETWORK INFO..: ATI"); yield();
  Serial.println("HTTP GET......: ATGET<URL>"); yield();
  //Serial.println("SERVER PORT...: AT$SP=N (N=1-65535)"); yield();
  Serial.println("AUTO ANSWER...: ATS0=N (N=0,1)"); yield();
  Serial.println("SET BUSY MSG..: AT$BM=YOUR BUSY MESSAGE"); yield();
  Serial.println("LOAD NVRAM....: ATZ"); yield();
  Serial.println("SAVE TO NVRAM.: AT&W"); yield();
  Serial.println("SHOW SETTINGS.: AT&V"); yield();
  Serial.println("FACT. DEFAULTS: AT&F"); yield();
  Serial.println("PIN POLARITY..: AT&PN (N=0/INV,1/NORM)"); yield();
  Serial.println("ECHO OFF/ON...: ATE0 / ATE1"); yield();
  Serial.println("VERBOSE OFF/ON: ATV0 / ATV1"); yield();
  Serial.println("SET SSID......: AT$SSID=WIFISSID"); yield();
  Serial.println("SET PASSWORD..: AT$PASS=WIFIPASSWORD"); yield();
  Serial.println("SET BAUD RATE.: AT$SB=N (3,12,24,48,96"); yield();
  Serial.println("                192,384,576,1152)*100"); yield();
  waitForSpace();
  Serial.println("FLOW CONTROL..: AT&KN (N=0/N,1/HW,2/SW)"); yield();
  Serial.println("WIFI OFF/ON...: ATC0 / ATC1"); yield();
  Serial.println("HANGUP........: ATH"); yield();
  Serial.println("ENTER CMD MODE: +++"); yield();
  Serial.println("EXIT CMD MODE.: ATO"); yield();
  Serial.println("QUERY MOST COMMANDS FOLLOWED BY '?'"); yield();
}

void ESPModem::storeSpeedDial(byte num, String location) {
  //if (num < 0 || num > 9) { return; }
  speedDials[num] = location;
  //Serial.print("STORED "); Serial.print(num); Serial.print(": "); Serial.println(location);
}

/**
   Arduino main init function
*/
void ESPModem::setup() 
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // off
//  pinMode(SWITCH_PIN, INPUT);
//  digitalWrite(SWITCH_PIN, HIGH);
  pinMode(DCD_PIN, OUTPUT);
  pinMode(RTS_PIN, OUTPUT);
  digitalWrite(RTS_PIN, HIGH); // ready to receive data
  pinMode(CTS_PIN, INPUT);
  //digitalWrite(CTS_PIN, HIGH); // pull up
  setCarrier(false);

  EEPROM.begin(LAST_ADDRESS + 1);
  delay(10);

  if (EEPROM.read(VERSION_ADDRESS) != VERSIONA || EEPROM.read(VERSION_ADDRESS + 1) != VERSIONB) {
    defaultEEPROM();
  }

  readSettings();
  // Fetch baud rate from EEPROM
  //serialspeed = 11; 
  serialspeed = EEPROM.read(BAUD_ADDRESS);
  // Check if it's out of bounds-- we have to be able to talk
  if (serialspeed < 0 || serialspeed > sizeof(bauds)) {
    serialspeed = 2; // 2400
  }

  Serial.begin(bauds[serialspeed]);
}

void ESPModem::start() 
{
	Serial.print(F("Starting Virtual Modem @ "));
  Serial.print(bauds[serialspeed]);
  Serial.println(F("bps"));

//   char c;
//   //unsigned long startMillis = millis();
//   //while (c != 8 && c != 127 && c!= 20) { // Check for the backspace key to begin
//   //while (c != 32) { // Check for space to begin
//   while (c != 0x0a && c != 0x0d) {
//     if (Serial.available() > 0) {
//       c = Serial.read();
//       if (petTranslate == true){
//         if (c > 127) c-= 128;
//       }
//     }
// //    if (checkButton() == 1) {
// //      break; // button pressed, we're setting to 300 baud and moving on
// //    }
//     //if (millis() - startMillis > 2000) {
//       //digitalWrite(LED_PIN, !digitalRead(LED_PIN));
//       //startMillis = millis();
//     //}
//     yield();
//   }

  WiFi.mode(WIFI_STA);
  connectWiFi();

  if (tcpServerPort > 0)
    tcpServer.begin();

  //sendResult(Result_OK);
  //tcpServer(tcpServerPort); // can't start tcpServer inside a function-- must live outside
}

String ESPModem::ipToString(IPAddress ip) {
  char s[16];
  sprintf(s, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  return s;
}

void ESPModem::hangUp() {
  tcpClient.stop();
  callConnected = false;
  setCarrier(callConnected);
  sendResult(Result_NOCARRIER);
  connectTime = 0;
}

/*
void handleWebHangUp() {
  String t = "NO CARRIER (" + connectTimeString() + ")";
  hangUp();
  webServer.send(200, "text/plain", t);
}

void handleRoot() {
  String page = "WIFI STATUS: ";
  if (WiFi.status() == WL_CONNECTED) {
    page.concat("CONNECTED");
  }
  if (WiFi.status() == WL_IDLE_STATUS) {
    page.concat("OFFLINE");
  }
  if (WiFi.status() == WL_CONNECT_FAILED) {
    page.concat("CONNECT FAILED");
  }
  if (WiFi.status() == WL_NO_SSID_AVAIL) {
    page.concat("SSID UNAVAILABLE");
  }
  if (WiFi.status() == WL_CONNECTION_LOST) {
    page.concat("CONNECTION LOST");
  }
  if (WiFi.status() == WL_DISCONNECTED) {
    page.concat("DISCONNECTED");
  }
  if (WiFi.status() == WL_SCAN_COMPLETED) {
    page.concat("SCAN COMPLETED");
  }
  yield();
  page.concat("\nSSID.......: " + WiFi.SSID());

  byte mac[6];
  WiFi.macAddress(mac);
  page.concat("\nMAC ADDRESS: ");
  page.concat(String(mac[0], HEX));
  page.concat(":");
  page.concat(String(mac[1], HEX));
  page.concat(":");
  page.concat(String(mac[2], HEX));
  page.concat(":");
  page.concat(String(mac[3], HEX));
  page.concat(":");
  page.concat(String(mac[4], HEX));
  page.concat(":");
  page.concat(String(mac[5], HEX));
  yield();

  page.concat("\nIP ADDRESS.: "); page.concat(ipToString(WiFi.localIP()));
  page.concat("\nGATEWAY....: "); page.concat(ipToString(WiFi.gatewayIP()));
  yield();

  page.concat("\nSUBNET MASK: "); page.concat(ipToString(WiFi.subnetMask()));
  yield();
  page.concat("\nSERVER PORT: "); page.concat(tcpServerPort);
  page.concat("\nCALL STATUS: ");
  if (callConnected) {
    page.concat("CONNECTED TO ");
    page.concat(ipToString(tcpClient.remoteIP()));
    page.concat("\nCALL LENGTH: "); page.concat(connectTimeString()); yield();
  } else {
    page.concat("NOT CONNECTED");
  }
  page.concat("\n");
  webServer.send(200, "text/plain", page);
  delay(100);
}
*/

void ESPModem::answerCall() {
  tcpClient = tcpServer.available();
  tcpClient.setNoDelay(true); // try to disable naggle
  //tcpServer.stop();
  sendResult(Result_CONNECT);
  connectTime = millis();
  cmdMode = false;
  callConnected = true;
  setCarrier(callConnected);
  Serial.flush();
}

void ESPModem::handleIncomingConnection() {
  if (callConnected == 1 || (autoAnswer == false && ringCount > 3)) {
    // We're in a call already or didn't answer the call after three rings
    // We didn't answer the call. Notify our party we're busy and disconnect
    ringCount = lastRingMs = 0;
    WiFiClient anotherClient = tcpServer.available();
    anotherClient.print(busyMsg);
    anotherClient.print("\r\n");
    anotherClient.print("CURRENT CALL LENGTH: ");
    anotherClient.print(connectTimeString());
    anotherClient.print("\r\n");
    anotherClient.print("\r\n");
    anotherClient.flush();
    anotherClient.stop();
    return;
  }

  if (autoAnswer == false) {
    if (millis() - lastRingMs > 6000 || lastRingMs == 0) {
      lastRingMs = millis();
      sendResult(Result_RING);
      ringCount++;
    }
    return;
  }

  if (autoAnswer == true) {
    WiFiClient tempClient = tcpServer.available(); // this is the key to keeping the connection open
    tcpClient = tempClient; // hand over the new connection to the global client
    tempClient.stop();   // stop the temporary one
    sendString(String("RING ") + ipToString(tcpClient.remoteIP()));
    delay(1000);
    sendResult(Result_CONNECT);
    connectTime = millis();
    cmdMode = false;
    tcpClient.flush();
    callConnected = true;
    setCarrier(callConnected);
  }
}

void ESPModem::dialOut(String upCmd) {
  // Can't place a call while in a call
  if (callConnected) {
    sendResult(Result_ERROR);
    return;
  }
  String host, port;
  int portIndex;
  // Dialing a stored number
  if (upCmd.indexOf("ATDS") == 0) {
    byte speedNum = upCmd.substring(4, 5).toInt();
    portIndex = speedDials[speedNum].indexOf(':');
    if (portIndex != -1) {
      host = speedDials[speedNum].substring(0, portIndex);
      port = speedDials[speedNum].substring(portIndex + 1);
    } else {
      port = "23";
    }
  } else {
    // Dialing an ad-hoc number
    int portIndex = cmd.indexOf(":");
    if (portIndex != -1)
    {
      host = cmd.substring(4, portIndex);
      port = cmd.substring(portIndex + 1, cmd.length());
    }
    else
    {
      host = cmd.substring(4, cmd.length());
      port = "23"; // Telnet default
    }
  }
  host.trim(); // remove leading or trailing spaces
  port.trim();
  Serial.print("DIALING "); Serial.print(host); Serial.print(":"); Serial.println(port);
  char *hostChr = new char[host.length() + 1];
  host.toCharArray(hostChr, host.length() + 1);
  int portInt = port.toInt();
  tcpClient.setNoDelay(true); // Try to disable naggle
  if (tcpClient.connect(hostChr, portInt))
  {
    tcpClient.setNoDelay(true); // Try to disable naggle
    sendResult(Result_CONNECT);
    connectTime = millis();
    cmdMode = false;
    Serial.flush();
    callConnected = true;
    setCarrier(callConnected);
    //if (tcpServerPort > 0) tcpServer.stop();
  }
  else
  {
    sendResult(Result_NOANSWER);
    callConnected = false;
    setCarrier(callConnected);
  }
  delete hostChr;
}

/**
   Perform a command given in command mode
*/
void ESPModem::command()
{
  cmd.trim();
  if (cmd == "") return;
  Serial.println();
  String upCmd = cmd;
  upCmd.toUpperCase();

  /**** Just AT ****/
  if (upCmd == "AT") sendResult(Result_OK);

  /**** Dial to host ****/
  else if ((upCmd.indexOf("ATDT") == 0) || (upCmd.indexOf("ATDP") == 0) || (upCmd.indexOf("ATDI") == 0) || (upCmd.indexOf("ATDS") == 0))
  {
    dialOut(upCmd);
  }

  /**** Change telnet mode ****/
  else if (upCmd == "ATNET0")
  {
    telnet = false;
    sendResult(Result_OK);
  }
  else if (upCmd == "ATNET1")
  {
    telnet = true;
    sendResult(Result_OK);
  }

  else if (upCmd == "ATNET?") {
    Serial.println(String(telnet));
    sendResult(Result_OK);
  }

  /**** Answer to incoming connection ****/
  else if ((upCmd == "ATA") && tcpServer.hasClient()) {
    answerCall();
  }

  /**** Display Help ****/
  else if (upCmd == "AT?" || upCmd == "ATHELP") {
    displayHelp();
    sendResult(Result_OK);
  }

  /**** Reset, reload settings from EEPROM ****/
  else if (upCmd == "ATZ") {
    readSettings();
    sendResult(Result_OK);
  }

  /**** Disconnect WiFi ****/
  else if (upCmd == "ATC0") {
    disconnectWiFi();
    sendResult(Result_OK);
  }

  /**** Connect WiFi ****/
  else if (upCmd == "ATC1") {
    connectWiFi();
    sendResult(Result_OK);
  }

  /**** Control local echo in command mode ****/
  else if (upCmd.indexOf("ATE") == 0) {
    if (upCmd.substring(3, 4) == "?") {
      sendString(String(echo));
      sendResult(Result_OK);
    }
    else if (upCmd.substring(3, 4) == "0") {
      echo = 0;
      sendResult(Result_OK);
    }
    else if (upCmd.substring(3, 4) == "1") {
      echo = 1;
      sendResult(Result_OK);
    }
    else {
      sendResult(Result_ERROR);
    }
  }

  /**** Control verbosity ****/
  else if (upCmd.indexOf("ATV") == 0) {
    if (upCmd.substring(3, 4) == "?") {
      sendString(String(verboseResults));
      sendResult(Result_OK);
    }
    else if (upCmd.substring(3, 4) == "0") {
      verboseResults = 0;
      sendResult(Result_OK);
    }
    else if (upCmd.substring(3, 4) == "1") {
      verboseResults = 1;
      sendResult(Result_OK);
    }
    else {
      sendResult(Result_ERROR);
    }
  }

  /**** Control pin polarity of CTS, RTS, DCD ****/
  else if (upCmd.indexOf("AT&P") == 0) {
    if (upCmd.substring(4, 5) == "?") {
      sendString(String(pinPolarity));
      sendResult(Result_OK);
    }
    else if (upCmd.substring(4, 5) == "0") {
      pinPolarity = P_INVERTED;
      sendResult(Result_OK);
      setCarrier(callConnected);
    }
    else if (upCmd.substring(4, 5) == "1") {
      pinPolarity = P_NORMAL;
      sendResult(Result_OK);
      setCarrier(callConnected);
    }
    else {
      sendResult(Result_ERROR);
    }
  }

  /**** Control Flow Control ****/
  else if (upCmd.indexOf("AT&K") == 0) {
    if (upCmd.substring(4, 5) == "?") {
      sendString(String(flowControl));
      sendResult(Result_OK);
    }
    else if (upCmd.substring(4, 5) == "0") {
      flowControl = 0;
      sendResult(Result_OK);
    }
    else if (upCmd.substring(4, 5) == "1") {
      flowControl = 1;
      sendResult(Result_OK);
    }
    else if (upCmd.substring(4, 5) == "2") {
      flowControl = 2;
      sendResult(Result_OK);
    }
    else {
      sendResult(Result_ERROR);
    }
  }

  /**** Set current baud rate ****/
  else if (upCmd.indexOf("AT$SB=") == 0) {
    setBaudRate(upCmd.substring(6).toInt());
  }

  /**** Display current baud rate ****/
  else if (upCmd.indexOf("AT$SB?") == 0) {
    sendString(String(bauds[serialspeed]));;
  }

  /**** Set busy message ****/
  else if (upCmd.indexOf("AT$BM=") == 0) {
    busyMsg = cmd.substring(6);
    sendResult(Result_OK);
  }

  /**** Display busy message ****/
  else if (upCmd.indexOf("AT$BM?") == 0) {
    sendString(busyMsg);
    sendResult(Result_OK);
  }

  /**** Display Network settings ****/
  else if (upCmd == "ATI") {
    displayNetworkStatus();
    sendResult(Result_OK);
  }

  /**** Display profile settings ****/
  else if (upCmd == "AT&V") {
    displayCurrentSettings();
    waitForSpace();
    displayStoredSettings();
    sendResult(Result_OK);
  }

  /**** Save (write) current settings to EEPROM ****/
  else if (upCmd == "AT&W") {
    writeSettings();
    sendResult(Result_OK);
  }

  /**** Set or display a speed dial number ****/
  else if (upCmd.indexOf("AT&Z") == 0) {
    byte speedNum = upCmd.substring(4, 5).toInt();
    if (speedNum >= 0 && speedNum <= 9) {
      if (upCmd.substring(5, 6) == "=") {
        String speedDial = cmd;
        storeSpeedDial(speedNum, speedDial.substring(6));
        sendResult(Result_OK);
      }
      if (upCmd.substring(5, 6) == "?") {
        sendString(speedDials[speedNum]);
        sendResult(Result_OK);
      }
    } else {
      sendResult(Result_ERROR);
    }
  }

  /**** Set WiFi SSID ****/
  else if (upCmd.indexOf("AT$SSID=") == 0) {
    ssid = cmd.substring(8);
    sendResult(Result_OK);
  }

  /**** Display WiFi SSID ****/
  else if (upCmd == "AT$SSID?") {
    sendString(ssid);
    sendResult(Result_OK);
  }

  /**** Set WiFi Password ****/
  else if (upCmd.indexOf("AT$PASS=") == 0) {
    password = cmd.substring(8);
    sendResult(Result_OK);
  }

  /**** Display WiFi Password ****/
  else if (upCmd == "AT$PASS?") {
    sendString(password);
    sendResult(Result_OK);
  }

  /**** Reset EEPROM and current settings to factory defaults ****/
  else if (upCmd == "AT&F") {
    defaultEEPROM();
    readSettings();
    sendResult(Result_OK);
  }

  /**** Set auto answer off ****/
  else if (upCmd == "ATS0=0") {
    autoAnswer = false;
    sendResult(Result_OK);
  }

  /**** Set auto answer on ****/
  else if (upCmd == "ATS0=1") {
    autoAnswer = true;
    sendResult(Result_OK);
  }

  /**** Display auto answer setting ****/
  else if (upCmd == "ATS0?") {
    sendString(String(autoAnswer));
    sendResult(Result_OK);
  }

  /**** Set PET MCTerm Translate On ****/
  else if (upCmd == "ATPET=1") {
    petTranslate = true;
    sendResult(Result_OK);
  }

  /**** Set PET MCTerm Translate Off ****/
  else if (upCmd == "ATPET=0") {
    petTranslate = false;
    sendResult(Result_OK);
  }

  /**** Display PET MCTerm Translate Setting ****/
  else if (upCmd == "ATPET?") {
    sendString(String(petTranslate));
    sendResult(Result_OK);
  }

  /**** Set HEX Translate On ****/
  else if (upCmd == "ATHEX=1") {
    hex = true;
    sendResult(Result_OK);
  }

  /**** Set HEX Translate Off ****/
  else if (upCmd == "ATHEX=0") {
    hex = false;
    sendResult(Result_OK);
  }

  /**** Hang up a call ****/
  else if (upCmd.indexOf("ATH") == 0) {
    hangUp();
  }

  /**** Hang up a call ****/
  else if (upCmd.indexOf("AT$RB") == 0) {
    sendResult(Result_OK);
    Serial.flush();
    delay(500);
#if defined(ESP32)
    ESP.restart();
#elif defined(ESP8266)
    ESP.reset();
#endif
  }

  /**** Exit modem command mode, go online ****/
  else if (upCmd == "ATO") {
    if (callConnected == 1) {
      sendResult(Result_CONNECT);
      cmdMode = false;
    } else {
      sendResult(Result_ERROR);
    }
  }

  /**** Set incoming TCP server port ****/
  else if (upCmd.indexOf("AT$SP=") == 0) {
    tcpServerPort = upCmd.substring(6).toInt();
    sendString("CHANGES REQUIRES NV SAVE (AT&W) AND RESTART");
    sendResult(Result_OK);
  }

  /**** Display icoming TCP server port ****/
  else if (upCmd == "AT$SP?") {
    sendString(String(tcpServerPort));
    sendResult(Result_OK);
  }

  /**** See my IP address ****/
  else if (upCmd == "ATIP?")
  {
    Serial.println(WiFi.localIP());
    sendResult(Result_OK);
  }

#if defined(ESP8266)
  /**** Connect to WiFi via WPS ****/
  else if (upCmd == "ATWPS")
  {
    Serial.println("Please press WPS button on your router.\r\n");
    waitForSpace();
    if(!startWPSConnect()) {
       Serial.println("Failed to connect with WPS :-(");  
    }
  }

  /**** See my IP address ****/
  else if (upCmd == "ATFW1")
  {
    updateFirmware();
  }
#endif

  /**** HTTP GET request ****/
  else if (upCmd.indexOf("ATGET") == 0)
  {
    // From the URL, aquire required variables
    // (12 = "ATGEThttp://")
    int portIndex = cmd.indexOf(":", 12); // Index where port number might begin
    int pathIndex = cmd.indexOf("/", 12); // Index first host name and possible port ends and path begins
    int port;
    String path, host;
    if (pathIndex < 0)
    {
      pathIndex = cmd.length();
    }
    if (portIndex < 0)
    {
      port = 80;
      portIndex = pathIndex;
    }
    else
    {
      port = cmd.substring(portIndex + 1, pathIndex).toInt();
    }
    host = cmd.substring(12, portIndex);
    path = cmd.substring(pathIndex, cmd.length());
    if (path == "") path = "/";
    char *hostChr = new char[host.length() + 1];
    host.toCharArray(hostChr, host.length() + 1);

    // Establish connection
    if (!tcpClient.connect(hostChr, port))
    {
      sendResult(Result_NOCARRIER);
      connectTime = 0;
      callConnected = false;
      setCarrier(callConnected);
    }
    else
    {
      sendResult(Result_CONNECT);
      connectTime = millis();
      cmdMode = false;
      callConnected = true;
      setCarrier(callConnected);

      // Send a HTTP request before continuing the connection as usual
      String request = "GET ";
      request += path;
      request += " HTTP/1.1\r\nHost: ";
      request += host;
      request += "\r\nConnection: close\r\n\r\n";
      tcpClient.print(request);
    }
    delete hostChr;
  }

  /**** Unknown command ****/
  else sendResult(Result_ERROR);

  cmd = "";
}

// RTS/CTS protocol is a method of handshaking which uses one wire in each direction to allow each
// device to indicate to the other whether or not it is ready to receive data at any given moment.
// One device sends on RTS and listens on CTS; the other does the reverse. A device should drive
// its handshake-output wire low when it is ready to receive data, and high when it is not. A device
// that wishes to send data should not start sending any bytes while the handshake-input wire is low;
// if it sees the handshake wire go high, it should finish transmitting the current byte and then wait
// for the handshake wire to go low before transmitting any more.
// http://electronics.stackexchange.com/questions/38022/what-is-rts-and-cts-flow-control
void ESPModem::handleFlowControl() {
  if (flowControl == F_NONE) return;
  if (flowControl == F_HARDWARE) {
    if (digitalRead(CTS_PIN) == pinPolarity) txPaused = true;
    else txPaused = false;
  }
  if (flowControl == F_SOFTWARE) {
    
  }
}

/**
   Main service loop function
*/
void ESPModem::service()
{
  // Check flow control
  handleFlowControl();
    
  // Service the Web server
//  webServer.handleClient();

  // Check to see if user is requesting rate change to 300 baud
//  checkButton();

  // New unanswered incoming connection on server listen socket
  if (tcpServer.hasClient()) {
    handleIncomingConnection();
  }

  /**** AT command mode ****/
  if (cmdMode == true)
  {

    // In command mode - don't exchange with TCP but gather characters to a string
    if (Serial.available())
    {
      char chr = Serial.read();

      if (petTranslate == true)
      {
        // Fix PET MCTerm 1.26C Pet->ASCII encoding to actual ASCII
        if (chr > 127) chr-= 128;        
      }
      else
      {
        // Convert uppercase PETSCII to lowercase ASCII (C64) in command mode only
        if ((chr >= 193) && (chr <= 218)) chr-= 96;        
      }

      // Return, enter, new line, carriage return.. anything goes to end the command
      if ((chr == '\n') || (chr == '\r'))
      {
        command();
      }
      // Backspace or delete deletes previous character
      else if ((chr == 8) || (chr == 127) || (chr == 20))
      {
        cmd.remove(cmd.length() - 1);
        if (echo == true) {
          Serial.write(chr);
        }
      }
      else
      {
        if (cmd.length() < MAX_CMD_LENGTH) cmd.concat(chr);
        if (echo == true) {
          Serial.write(chr);
        }
        if (hex) {
          Serial.print(chr, HEX);
        }
      }
    }
  }
  /**** Connected mode ****/
  else
  {
    // Transmit from terminal to TCP
    if (Serial.available())
    {
//      ledOn();

      // In telnet in worst case we have to escape every byte
      // so leave half of the buffer always free
      int max_buf_size;
      if (telnet == true)
        max_buf_size = TX_BUF_SIZE / 2;
      else
        max_buf_size = TX_BUF_SIZE;

      // Read from serial, the amount available up to
      // maximum size of the buffer
      size_t len = std::min(Serial.available(), max_buf_size);
      Serial.readBytes(&txBuf[0], len);

      // Enter command mode with "+++" sequence
      for (int i = 0; i < (int)len; i++)
      {
        if (txBuf[i] == '+') plusCount++; else plusCount = 0;
        if (plusCount >= 3)
        {
          plusTime = millis();
        }
        if (txBuf[i] != '+')
        {
          plusCount = 0;
        }
      }

      // Double (escape) every 0xff for telnet, shifting the following bytes
      // towards the end of the buffer from that point
      if (telnet == true)
      {
        for (int i = len - 1; i >= 0; i--)
        {
          if (txBuf[i] == 0xff)
          {
            for (int j = TX_BUF_SIZE - 1; j > i; j--)
            {
              txBuf[j] = txBuf[j - 1];
            }
            len++;
          }
        }
      }
      // Fix PET MCTerm 1.26C Pet->ASCII encoding to actual ASCII
      if (petTranslate == true) {
        for (int i = len - 1; i >= 0; i--) {
          if (txBuf[i] > 127) txBuf[i]-= 128;
        }
      }
      // Write the buffer to TCP finally
      tcpClient.write(&txBuf[0], len);
      yield();
    }

    // Transmit from TCP to terminal
    while (tcpClient.available() && txPaused == false)
    {
//      ledOn();
      uint8_t rxByte = tcpClient.read();

      // Is a telnet control code starting?
      if ((telnet == true) && (rxByte == 0xff))
      {
#ifdef DEBUG
        Serial.print("<t>");
#endif
        rxByte = tcpClient.read();
        if (rxByte == 0xff)
        {
          // 2 times 0xff is just an escaped real 0xff
          Serial.write(0xff); Serial.flush();
        }
        else
        {
          // rxByte has now the first byte of the actual non-escaped control code
#ifdef DEBUG
          Serial.print(rxByte);
          Serial.print(",");
#endif
          uint8_t cmdByte1 = rxByte;
          rxByte = tcpClient.read();
          uint8_t cmdByte2 = rxByte;
          // rxByte has now the second byte of the actual non-escaped control code
#ifdef DEBUG
          Serial.print(rxByte); Serial.flush();
#endif
          // We are asked to do some option, respond we won't
          if (cmdByte1 == DO)
          {
            tcpClient.write((uint8_t)255); tcpClient.write((uint8_t)WONT); tcpClient.write(cmdByte2);
          }
          // Server wants to do any option, allow it
          else if (cmdByte1 == WILL)
          {
            tcpClient.write((uint8_t)255); tcpClient.write((uint8_t)DO); tcpClient.write(cmdByte2);
          }
        }
#ifdef DEBUG
        Serial.print("</t>");
#endif
      }
      else
      {
        // Non-control codes pass through freely
        Serial.write(rxByte); yield(); Serial.flush(); yield();
      }
      handleFlowControl();
    }
  }

  // If we have received "+++" as last bytes from serial port and there
  // has been over a second without any more bytes
  if (plusCount >= 3)
  {
    if (millis() - plusTime > 1000)
    {
      //tcpClient.stop();
      cmdMode = true;
      sendResult(Result_OK);
      plusCount = 0;
    }
  }

  // Go to command mode if TCP disconnected and not in command mode
  if ((!tcpClient.connected()) && (cmdMode == false) && callConnected == true)
  {
    cmdMode = true;
    sendResult(Result_NOCARRIER);
    connectTime = 0;
    callConnected = false;
    setCarrier(callConnected);
    //if (tcpServerPort > 0) tcpServer.begin();
  }

  // Turn off tx/rx led if it has been lit long enough to be visible
  if (millis() - ledTime > LED_TIME) digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // toggle LED state
}

#if defined(ESP8266)
bool ESPModem::startWPSConnect() {
  Serial.println("WPS config start");
  bool wpsSuccess = WiFi.beginWPSConfig();
  if(wpsSuccess) {
      // Well this means not always success :-/ in case of a timeout we have an empty ssid
      String newSSID = WiFi.SSID();
      if(newSSID.length() > 0) {
        // WPSConfig has already connected in STA mode successfully to the new station. 
        Serial.printf("WPS finished. Connected successfull to SSID '%s'\n", newSSID.c_str());
      } else {
        wpsSuccess = false;
      }
  }
  return wpsSuccess; 
}
#endif


#if defined(ESP8266)
bool updateFirmware() {
  Serial.println("Attempting to bake Meat Loaf 64!\r\n");
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

  ESPhttpUpdate.onEnd(updateEnd);
  ESPhttpUpdate.onProgress(updateProgress);
  t_httpUpdate_return ret = ESPhttpUpdate.update( UPDATE_URL );
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
  }
}

void updateProgress(int cur, int total) {
  Serial.printf("Downloading %d of %d\r", cur, total);
}

void updateEnd() {
  Serial.printf("Update Complete! Restarting!\r\n\r\n");
}
#endif