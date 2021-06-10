#include "ml.h"


MLFile::~MLFile() {
    // just to be sure to close it if we don't read the directory until the very end
    m_http.end();
}

MFile* MLFile::getNextFileInDir() {
    if(!dirIsOpen) // might be first call, so let's try opening the dir
    {
        Serial.print("\ndirIsOpen = 0, rewinding");
        dirIsOpen = rewindDirectory();
    }

    if(!dirIsOpen)
        return nullptr; // we couldn't open it or whole dir was at this stage - return nullptr, as usual

    // calling this proc will read a single JSON line that will be processed into MFile and returned
    m_lineBuffer = m_file.readStringUntil('\n');
Serial.printf("Buffer read from ml server: %s\n", m_lineBuffer.c_str());
	if(m_lineBuffer.length() > 1)
	{
		// Parse JSON object
		DeserializationError error = deserializeJson(m_jsonHTTP, m_lineBuffer);
		if (error)
		{
			Serial.print(F("\r\ndeserializeJson() failed: "));
			Serial.println(error.c_str());
            dirIsOpen = false;
            m_http.end();
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
            dirIsOpen = true;
            ledToggle(true);

            std::string path = urldecode(m_jsonHTTP["name"]).c_str();
            size_t size = m_jsonHTTP["size"];
            bool dir = m_jsonHTTP["dir"];

            return new MLFile(path, size, dir); // note such path can't be used to do our "magic" stream-in-strea-in-stream, you can use it only to list dir
            //return new MLFile("ml://c64.meatloaf.cc/dummy/file.prg", 123, false);
        }

	} 
    else {
        // no more entries, let's close the stream
                Serial.println("no more entries");

        dirIsOpen = false;
        ledOFF();
        return nullptr;
    }
};


bool MLFile::rewindDirectory() {
    if (!isDirectory()) { 
        dirIsOpen = false;
        return false;
    }
    
Serial.printf("\r\nRequesting JSON dir from PHP: ");

	String user_agent(String(PRODUCT_ID) + " [" + String(FW_VERSION) + "]");
	String url("http://c64.meatloaf.cc/api/");
	//String post_data("p=" + urlencode(m_device.path()) + "&i=" + urlencode(m_device.image()) + "&f=" + urlencode(m_filename));
    String post_data("p=" + urlencode(String(pathInStream.substr(1, pathInStream.find_first_of("/")).c_str()))); // pathInStream will return here /c64.meatloaf.cc/some/directory

	// Connect to HTTP server
	//WiFiClient client; // TODO do we need both clients? This and payload? Maybe we do, just asking....
	url.toLowerCase();
	if (!m_http.begin(m_file, url))
	{
		Serial.printf("\r\nConnection failed");
		dirIsOpen = false;
        return false;
	}
	m_http.addHeader("Content-Type", "application/x-www-form-urlencoded");

	Serial.printf("\r\nConnected!\r\n--------------------\r\n%s\r\n%s\r\n%s\r\n", user_agent.c_str(), url.c_str(), post_data.c_str());

	uint8_t httpCode = m_http.POST(post_data);	 //Send the request
	//payload = http.getStream(); //Get the response payload as Stream

	Serial.printf("HTTP Status: %d\r\n", httpCode); //Print HTTP return code

	if (httpCode != 200) {
        Serial.println("httpCode != 200");
		dirIsOpen = false;
    }
    else
        dirIsOpen = true;

    return dirIsOpen;
};
