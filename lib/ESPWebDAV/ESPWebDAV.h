#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <FS.h>

#include "../../include/global_defines.h"

// constants for WebServer
#define CONTENT_LENGTH_UNKNOWN ((size_t) -1)
#define CONTENT_LENGTH_NOT_SET ((size_t) -2)
#define HTTP_MAX_POST_WAIT 		5000 

enum ResourceType { RESOURCE_NONE, RESOURCE_FILE, RESOURCE_DIR };
enum DepthType { DEPTH_NONE, DEPTH_CHILD, DEPTH_ALL };


class ESPWebDAV	{
public:
	bool init(uint8_t serverPort, FS* fileSystem);
	bool stop(void);
	bool isClientWaiting(void);
	void handleClient(String blank = "");
	void rejectClient(String rejectMessage);

protected:
	typedef void (ESPWebDAV::*THandlerFunction)(String);
	
	void processClient(THandlerFunction handler, String message);
	void handleNotFound();
	void handleReject(String rejectMessage);
	void handleRequest(String blank);
	void handleOptions(ResourceType resource);
	void handleLock(ResourceType resource);
	void handleUnlock(ResourceType resource);
	void handlePropPatch(ResourceType resource);
	void handleProp(ResourceType resource);
	void sendPropResponse(boolean recursing, File *curFile);
	void handleGet(ResourceType resource, bool isGet);
	void handlePut(ResourceType resource);
	void handleWriteError(String message, File *wFile);
	void handleDirectoryCreate(ResourceType resource);
	void handleMove(ResourceType resource);
	void handleDelete(ResourceType resource);

	uint8_t deleteRecursive(String path);

	// Sections are copied from ESP8266Webserver
	String getMimeType(String path);
	String urlDecode(const String& text);
	String urlToUri(String url);
	bool parseRequest();
	void sendHeader(const String& name, const String& value, bool first = false);
	void send(String code, const char* content_type, const String& content);
	void _prepareHeader(String& response, String code, const char* content_type, size_t contentLength);
	void sendContent(const String& content);
	void sendContent_P(PGM_P content);
	void setContentLength(size_t len);
	size_t readBytesWithTimeout(uint8_t *buf, size_t bufSize);
	size_t readBytesWithTimeout(uint8_t *buf, size_t bufSize, size_t numToRead);
	
	
	// variables pertaining to current most HTTP request being serviced
	WiFiServer *server;
	FS *m_fileSystem;
	WiFiClient 	client;
	String 		method;
	String 		uri;
	String 		contentLengthHeader;
	String 		depthHeader;
	String 		hostHeader;
	String		destinationHeader;

	String 		_responseHeaders;
	bool		_chunked;
	int		_contentLength;
};

