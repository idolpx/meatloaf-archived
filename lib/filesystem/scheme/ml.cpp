#include "ml.h"

MFile* MLFile::getNextFileInDir() {
    if(!dirIsOpen) // might be first call, so let's try opening the dir
    {
        dirIsOpen = rewindDirectory();
    }

    if(!dirIsOpen)
        return nullptr; // we couldn't open it or whole dir was at this stage - return nullptr, as usual

    // calling this proc will read a single JSON line that will be processed into MFile and returned
    m_lineBuffer = payload.readStringUntil('\n');

	if(m_lineBuffer.length() > 1)
	{
		// Parse JSON object
		DeserializationError error = deserializeJson(m_jsonHTTP, m_lineBuffer);
		if (error)
		{
			Serial.print(F("\r\ndeserializeJson() failed: "));
			Serial.println(error.c_str());
            dirIsOpen = false;
            http.end();
            ledOFF();
            return nullptr;
		}
        else {
            /*
            Right now you're returning this:

            {"blocks":0,"line":"%12%22MEATLOAF+64+ARCHIVE%22+ID+99","type":"NFO"} 
            {"blocks":0,"line":"%22%5BURL%5D++++++++++++++%22+NFO","type":"NFO"} 

            So let's say now you return instead this:

            {"path":"ml://servername.com/full/file/path/demos","isDir":true,"size":0} 

            */
            ledToggle(true);

            //return new MLFile(m_jsonHTTP["path"], m_jsonHTTP["size"], m_jsonHTTP["isDir"]); // note such path can't be used to do our "magic" stream-in-strea-in-stream, you can use it only to list dir
            return new MLFile("ml://c64.meatloaf.cc/dummy/file.prg", 123, false);
        }

	} 
    else {
        // no more entries, let's close the stream
        dirIsOpen = false;
        http.end(); //Close connection
        ledOFF();
        return nullptr;
    }
};

MLFile::~MLFile() {
    // just to be sure to close it if we don't read the directory until the very end
    http.end();
}

bool MLFile::rewindDirectory() {
    Debug_printf("\r\nRequesting JSON dir from PHP: ");

	String user_agent(String(PRODUCT_ID) + " [" + String(FW_VERSION) + "]");
	String url("/get server base (?) url somehow from the path?/api/");
	//String post_data("p=" + urlencode(m_device.path()) + "&i=" + urlencode(m_device.image()) + "&f=" + urlencode(m_filename));
    String post_data("p=" + urlencode(String(pathInStream.c_str()))); // pathInStream will return here /c64.meatloaf.cc/some/directory

	// Connect to HTTP server
	WiFiClient client; // TODO do we need both clients? This and payload? Maybe we do, just asking....
	http.setUserAgent(user_agent);
	// http.setFollowRedirects(true);
	http.setTimeout(10000);
	url.toLowerCase();
	if (!http.begin(client, url))
	{
		Debug_println(F("\r\nConnection failed"));
		return false;
	}
	http.addHeader("Content-Type", "application/x-www-form-urlencoded");

	Debug_printf("\r\nConnected!\r\n--------------------\r\n%s\r\n%s\r\n%s\r\n", user_agent.c_str(), url.c_str(), post_data.c_str());

	uint8_t httpCode = http.POST(post_data);	 //Send the request
	payload = http.getStream(); //Get the response payload as Stream

	Debug_printf("HTTP Status: %d\r\n", httpCode); //Print HTTP return code
	if (httpCode != 200) {
        http.end(); //Close connection
		return false;
    }
    else    
        return true;
};
