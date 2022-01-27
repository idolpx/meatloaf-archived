
#include <WebDav4WebServer.h>


#if WEBSERVER_HAS_HOOK
WebServer::HookFunction hookWebDAVForWebserver(const String& davRootDir, ESPWebDAVCore& dav, const String& fsRootDir)
{
    dav.setDAVRoot(davRootDir);
    dav.setFsRoot(fsRootDir);
    return [&dav](const String & method, const String & url, WiFiClient * client, WebServer::ContentTypeFunction contentType)
    {
        if (dav.isIgnored(url))
        {
            DBG_PRINT("CLIENT_REQUEST_CAN_CONTINUE, '%s' is explicitally ignored", url.c_str());
            return WebServer::CLIENT_REQUEST_CAN_CONTINUE;
        }
        if (url.indexOf(dav.getDAVRoot()) != 0)
        {
            DBG_PRINT("CLIENT_REQUEST_CAN_CONTINUE, '%s' is not seen in '%s'", dav.getDAVRoot().c_str(), url.c_str());
            return WebServer::CLIENT_REQUEST_CAN_CONTINUE;
        }
        if (dav.parseRequest(method, url, client, contentType))
        {
            DBG_PRINT("CLIENT_REQUEST_IS_HANDLED ('%s')", url.c_str());
            return WebServer::CLIENT_REQUEST_IS_HANDLED;
        }
        DBG_PRINT("CLIENT_MUST_STOP");
        return WebServer::CLIENT_MUST_STOP;
    };
}
#endif // WEBSERVER_HAS_HOOK
