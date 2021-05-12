#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include "../../include/global_defines.h"

class HTTPUpdate {
public:
    const char* updateURL;

    void checkForUpdates();
    String getMAC();
};
#endif