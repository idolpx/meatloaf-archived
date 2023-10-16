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

            std::string fname ="ml://" + host + "/" + urldecode(m_jsonHTTP["name"]).c_str();
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

bool MLFile::isDirectory() {
    //String url("http://c64.meatloaf.cc/api/");
    //String ml_url = std::string("http://" + host + "/api/").c_str();
    std::string ml_url = "http://" + host + "/api/";
    std::string post_data = "a=checkp=" + mstr::urlEncode(path);

	// Connect to HTTP server
	Serial.printf("\r\nConnecting!\r\n--------------------\r\n%s\r\n%s\r\n", ml_url.c_str(), post_data.c_str());
	if (!m_http.begin(m_file, ml_url.c_str()))
	{
		Serial.printf("\r\nConnection failed");
        return false;
	}
	m_http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Setup response headers we want to collect
    const char * headerKeys[] = {"ml_media_dir"} ;
    const size_t numberOfHeaders = 1;
    m_http.collectHeaders(headerKeys, numberOfHeaders);

    // Send the request
	uint8_t httpCode = m_http.POST(post_data.c_str());

	Serial.printf("HTTP Status: %d\r\n", httpCode); //Print HTTP return code

	if (httpCode == 200) {
        if (m_http.header("ml_media_dir") == "1")
            return true;
    }

    return false;
};


bool MLFile::rewindDirectory() {
    if (!isDirectory()) {
        dirIsOpen = false;
        return false;
    }

    Debug_printv("Requesting JSON dir from PHP: ");

	//String url("http://c64.meatloaf.cc/api/");
    //String ml_url = std::string("http://" + host + "/api/").c_str();
    std::string ml_url = "http://" + host + "/api/";
	//String post_data("p=" + urlencode(m_device.path()) + "&i=" + urlencode(m_device.image()) + "&f=" + urlencode(m_filename));
    //String post_data = std::string("p=" + mstr::urlEncode(path)).c_str(); // pathInStream will return here /c64.meatloaf.cc/some/directory
    std::string post_data = "p=" + mstr::urlEncode(path);

	// Connect to HTTP server
	Serial.printf("\r\nConnecting!\r\n--------------------\r\n%s\r\n%s\r\n", ml_url.c_str(), post_data.c_str());
	if (!m_http.begin(m_file, ml_url.c_str()))
	{
		Serial.printf("\r\nConnection failed");
		dirIsOpen = false;
        return false;
	}
	m_http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Setup response headers we want to collect
    const char * headerKeys[] = {"accept-ranges", "content-type", "ml_media_header", "ml_media_id", "ml_media_blocks_free", "ml_media_block_size"} ;
    const size_t numberOfHeaders = 6;
    m_http.collectHeaders(headerKeys, numberOfHeaders);

    // Send the request
	uint8_t httpCode = m_http.POST(post_data.c_str());

	Serial.printf("HTTP Status: %d\r\n", httpCode); //Print HTTP return code

	if (httpCode != 200) {
        Serial.println(m_http.errorToString(httpCode));
		dirIsOpen = false;

        // // Show HTTP Headers
        // Serial.println("HEADERS--------------");
        // int i = 0;
        // for (i=0; i < m_http.headers(); i++)
        // {
        //     Serial.println(m_http.header(i));
        // }
        // Serial.println("DATA-----------------");
        // Serial.println(m_http.getString());
        // Serial.println("---------------------");

    }
    else
    {
        dirIsOpen = true;
        media_header = m_http.header("ml_media_header").c_str();
        media_id = m_http.header("ml_media_id").c_str();
        media_block_size = m_http.header("ml_media_block_size").toInt();
        media_blocks_free = m_http.header("ml_media_blocks_free").toInt();
    }

    return dirIsOpen;
};

MIStream* MLFile::inputStream() {
    // has to return OPENED stream
    Debug_printv("[%s]", url.c_str());
    MIStream* istream = new MLIStream(url);
    istream->open();   
    return istream;
}; 


bool MLIStream::open() {
    PeoplesUrlParser urlParser;
    urlParser.parseUrl(url);

    //String ml_url = std::string("http://" + urlParser.host + "/api/").c_str();
	//String post_data("p=" + urlencode(m_device.path()) + "&i=" + urlencode(m_device.image()) + "&f=" + urlencode(m_filename));
    //String post_data = std::string("p=" + mstr::urlEncode(urlParser.path)).c_str(); // pathInStream will return here /c64.meatloaf.cc/some/directory
    std::string ml_url = "http://" + urlParser.host + "/api";
    std::string post_data = "p=" + urlParser.path;

    m_http.setReuse(true);
    bool initOk = m_http.begin(m_file, ml_url.c_str());
    Debug_printv("input %s: someRc=%d, post[%s]", ml_url.c_str(), initOk, post_data.c_str());
    if(!initOk)
        return false;

    // Send the request
	uint8_t httpCode = m_http.POST(post_data.c_str());
    Debug_printv("httpCode=%d", httpCode);
    if(httpCode != 200)
        return false;

    // Accept-Ranges: bytes - if we get such header from any request, good!
    isFriendlySkipper = m_http.header("accept-ranges") == "bytes";
    m_isOpen = true;
    Debug_printv("[%s]", ml_url.c_str());
    m_length = m_http.getSize();
    Debug_printv("length=%d", m_length);
    m_bytesAvailable = m_length;
    return true;
};