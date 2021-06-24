#if defined(ESP8266)

#include "HTTPUpdate.h"

void HTTPUpdate::checkForUpdates() {
  // String mac = getMAC();
  // String fwURL = String( updateURL );
  // fwURL.replace( "{{DEVICE_ID}}", mac );
  // String fwVersionURL = fwURL;
  // fwVersionURL.concat( "v" ); // Get Firmware Version

  // Serial.println( "Checking for firmware updates." );
  // Serial.print( "MAC address: " );
  // Serial.println( mac );
  // Serial.print( "Firmware version URL: " );
  // Serial.println( fwVersionURL );

  // HTTPClient httpClient;
  // httpClient.begin( fwVersionURL );
  // uint8_t httpCode = httpClient.GET();
  // if( httpCode == 200 ) {
  //   String newFWVersion = httpClient.getString();

  //   Serial.print( "Current firmware version: " );
  //   Serial.println( FW_VERSION );
  //   Serial.print( "Available firmware version: " );
  //   Serial.println( newFWVersion );

  //   if( strcmp(FW_VERSION, newFWVersion.c_str() ) == 0 ) {
  //     Serial.println( "Preparing to update" );

  //     String fwImageURL = fwURL;
  //     fwImageURL.concat( "i" ); // Get Firmware Image
  //     t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );

  //     switch(ret) {
  //       case HTTP_UPDATE_FAILED:
  //         Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
  //         break;

  //       case HTTP_UPDATE_NO_UPDATES:
  //         Serial.println("HTTP_UPDATE_NO_UPDATES");
  //         break;
  //     }
  //   }
  //   else {
  //     Serial.println( "Already on latest version" );
  //   }
  // }
  // else {
  //   Serial.print( "Firmware version check failed, got HTTP response code " );
  //   Serial.println( httpCode );
  // }
  // httpClient.end();
}

String HTTPUpdate::getMAC()
{
    uint8_t mac[6];
    char result[14];

    snprintf( result, sizeof( result ), "%02x%02x%02x%02x%02x%02x", mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );

    return String( result );
}

#endif