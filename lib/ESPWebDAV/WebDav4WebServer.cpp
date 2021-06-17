
#include <WebDav4WebServer.h>


#if WEBSERVER_HAS_HOOK
WebServer::HookFunction hookWebDAVForWebserver(const String& davRootDir, ESPWebDAVCore& dav)
{
    return [&dav, davRootDir](const String & method, const String & url, WiFiClient * client, WebServer::ContentTypeFunction contentType)
    {
        if (url.indexOf(davRootDir) != 0)
        {
            DBG_PRINT("CLIENT_REQUEST_CAN_CONTINUE, %s is not seen in %s", davRootDir.c_str(), url.c_str());
            return WebServer::CLIENT_REQUEST_CAN_CONTINUE;
        }
        else
        {
            if (dav.parseRequest(method, url, client, contentType))
            {
                DBG_PRINT("CLIENT_REQUEST_IS_HANDLED");
                return WebServer::CLIENT_REQUEST_IS_HANDLED;
            }
            else
            {
                DBG_PRINT("CLIENT_MUST_STOP");
                return WebServer::CLIENT_MUST_STOP;
            }
        }
    };
}
#endif // WEBSERVER_HAS_HOOK
