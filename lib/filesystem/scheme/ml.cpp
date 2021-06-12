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
//Serial.printf("Buffer read from ml server: %s\n", m_lineBuffer.c_str());
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

            std::string fname = base() + urldecode(m_jsonHTTP["name"]).c_str();
            size_t size = m_jsonHTTP["size"];
            bool dir = m_jsonHTTP["dir"];

            return new MLFile(fname, size, dir); // note such path can't be used to do our "magic" stream-in-strea-in-stream, you can use it only to list dir
            //return new MLFile("ml://c64.meatloaf.cc/dummy/file.prg", 123, false);
        }

	} 
    else {
        // no more entries, let's close the stream
        //Serial.println("no more entries");

        dirIsOpen = false;
        return nullptr;
    }
};


bool MLFile::rewindDirectory() {
    if (!isDirectory()) { 
        dirIsOpen = false;
        return false;
    }
    
//Serial.printf("\r\nRequesting JSON dir from PHP: ");

	//String url("http://c64.meatloaf.cc/api/");
    String url = std::string("http://" + hostname + "/api/").c_str();
	//String post_data("p=" + urlencode(m_device.path()) + "&i=" + urlencode(m_device.image()) + "&f=" + urlencode(m_filename));
    String post_data = std::string("p=" + pathX).c_str(); // pathInStream will return here /c64.meatloaf.cc/some/directory

	// Connect to HTTP server
	Serial.printf("\r\nConnecting!\r\n--------------------\r\n%s\r\n%s\r\n", url.c_str(), post_data.c_str());
	if (!m_http.begin(m_file, url))
	{
		Serial.printf("\r\nConnection failed");
		dirIsOpen = false;
        return false;
	}
	m_http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Setup response headers we want to collect
    const char * headerKeys[] = {"ml_media_root", "ml_media_header", "ml_media_id", "ml_media_blocks_free", "ml_media_block_size"} ;
    const size_t numberOfHeaders = 5;
    m_http.collectHeaders(headerKeys, numberOfHeaders);

    // Send the request
	uint8_t httpCode = m_http.POST(post_data.c_str());

	Serial.printf("HTTP Status: %d\r\n", httpCode); //Print HTTP return code

	if (httpCode != 200) {
        Serial.println(m_http.errorToString(httpCode));
		dirIsOpen = false;

        // Show HTTP Headers
        Serial.println("HEADERS--------------");
        size_t i = 0;
        for (i=0; i < m_http.headers(); i++)
        {
            Serial.println(m_http.header(i));
        }
        Serial.println("DATA-----------------");
        Serial.println(m_http.getString());
        Serial.println("---------------------");

    }
    else
    {
        dirIsOpen = true;
        media_root = m_http.header("ml_media_root").c_str();
        media_header = m_http.header("ml_media_header").c_str();
        media_id = m_http.header("ml_media_id").c_str();
        media_block_size = m_http.header("ml_media_block_size").toInt();
        media_blocks_free = m_http.header("ml_media_blocks_free").toInt();
    }

    return dirIsOpen;
};
