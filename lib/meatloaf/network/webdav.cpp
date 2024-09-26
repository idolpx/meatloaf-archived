// #include "webdav.h"

// /********************************************************
//  * File impls
//  ********************************************************/

// bool WebDAVFile::isDirectory() {
//     // hey, why not?
//     return false;
// }

// MStream* WebDAVFile::getSourceStream() {
//     // has to return OPENED stream
//     Debug_printv("[%s]", url.c_str());
//     MStream* istream = new WebDAVIStream(url);
//     istream->open();
//     return istream;
// }

// MStream* WebDAVFile::getDecodedStream(std::shared_ptr<MStream> is) {
//     return is.get(); // we've overriden istreamfunction, so this one won't be used
// }

// time_t WebDAVFile::getLastWrite() {
//     return 0; // might be taken from Last-Modified, probably not worth it
// }

// time_t WebDAVFile::getCreationTime() {
//     return 0; // might be taken from Last-Modified, probably not worth it
// }

// bool WebDAVFile::exists() {
//     Debug_printv("[%s]", url.c_str());
//     // we may try open the stream to check if it exists
//     std::unique_ptr<MStream> test(getSourceStream());
//     // remember that MStream destuctor should close the stream!
//     return test->isOpen();
// }

// uint32_t WebDAVFile::size() {
//     // we may take content-lenght from header if exists
//     std::unique_ptr<MStream> test(getSourceStream());

//     size_t size = 0;

//     if(test->isOpen())
//         size = test->available();

//     test->close();

//     return size;
// }



// // void WebDAVFile::addHeader(const String& name, const String& value, bool first, bool replace) {
// //     //m_http.addHeader
// // }


// /********************************************************
//  * Ostream impls
//  ********************************************************/

// bool WebDAVOStream::seek(uint32_t pos) {
//     if(pos==_position)
//         return true;

//     if(isFriendlySkipper) {
//         char str[40];
//         // Range: bytes=91536-(91536+255)
//         snprintf(str, sizeof str, "bytes=%lu-%lu", (unsigned long)pos, ((unsigned long)pos + 255));
//         m_http.set_header("range",str);
//         int httpCode = m_http.GET(); //Send the request
//         Debug_printv("httpCode[%d] str[%s]", httpCode, str);
//         if(httpCode != 200 || httpCode != 206)
//             return false;

//         Debug_printv("stream opened[%s]", url.c_str());
//         //m_file = m_http.getStream();  //Get the response payload as Stream
//         _position = pos;
//         return true;

//     } else {
//         if(pos<_position) {
//             // skipping backward and range not supported, let's simply reopen the stream...
//             m_http.close();
//             bool op = open();
//             if(!op)
//                 return false;
//         }

//         _position = 0;
//         // ... and then read until we reach pos
//         // while(_position < pos) {
//         //  _position+=m_file.readBytes(buffer, size);  <----------- trurn this on!!!!
//         // }

//         return true;
//     }
// }



// void WebDAVOStream::close() {
//     m_http.close();
// }

// bool WebDAVOStream::open() {
//     // we'll ad a lambda that will allow adding headers
//     // m_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//     mstr::replaceAll(url, "HTTP:", "http:");
// //    m_http.setReuse(true);
//     bool initOk = m_http.begin( url );
//     Debug_printv("[%s] initOk[%d]", url.c_str(), initOk);
//     if(!initOk)
//         return false;

//     //int httpCode = m_http.PUT(); //Send the request
// //Serial.printf("URLSTR: httpCode=%d\r\n", httpCode);
//     // if(httpCode != 200)
//     //     return false;

//     _is_open = true;
//     //m_file = m_http.getStream();  //Get the response payload as Stream
//     return true;
// }

// //uint32_t WebDAVOStream::write(uint8_t) {};
// uint32_t WebDAVOStream::write(const uint8_t *buf, uint32_t size) {
//     return 0; // m_file.write(buf, size);
// }

// bool WebDAVOStream::isOpen() {
//     return _is_open;
// }


// /********************************************************
//  * Istream impls
//  ********************************************************/

// bool WebDAVIStream::seek(uint32_t pos) {
//     if(pos==_position)
//         return true;

//     if(isFriendlySkipper) {
//         char str[40];
//         // Range: bytes=91536-(91536+255)
//         snprintf(str, sizeof str, "bytes=%lu-%lu", (unsigned long)pos, ((unsigned long)pos + 255));
//         m_http.set_header("range",str);
//         int httpCode = m_http.GET(); //Send the request
//         Debug_printv("httpCode[%d] str[%s]", httpCode, str);
//         if(httpCode != 200 || httpCode != 206)
//             return false;

//         Debug_printv("stream opened[%s]", url.c_str());
//         //m_file = m_http.getStream();  //Get the response payload as Stream
//         _position = pos;
//         return true;

//     } else {
//         if(pos<_position) {
//             // skipping backward and range not supported, let's simply reopen the stream...
//             m_http.close();
//             bool op = open();
//             if(!op)
//                 return false;
//         }

//         _position = 0;
//         // ... and then read until we reach pos
//         // while(_position < pos) {
//         //  _position+=m_file.readBytes(buffer, size);  <----------- trurn this on!!!!
//         // }

//         return true;
//     }
// }

// void WebDAVIStream::close() {
//     m_http.close();
// }

// bool WebDAVIStream::open() {
//     //mstr::replaceAll(url, "HTTP:", "http:");
//     bool initOk = m_http.begin( url );
//     Debug_printv("input %s: someRc=%d", url.c_str(), initOk);
//     if(!initOk)
//         return false;

//     // Setup response headers we want to collect
//     const char * headerKeys[] = {"accept-ranges", "content-type", "content-length"};
//     const size_t numberOfHeaders = 2;
//     m_http.collect_headers(headerKeys, numberOfHeaders);

//     //Send the request
//     int httpCode = m_http.GET();
//     Debug_printv("httpCode=%d", httpCode);
//     if(httpCode != 200)
//         return false;

//     // Accept-Ranges: bytes - if we get such header from any request, good!
//     isFriendlySkipper = m_http.get_header("accept-ranges") == "bytes";
//     Debug_printv("isFriendlySkipper[%d]", isFriendlySkipper);
//     _is_open = true;
//     Debug_printv("[%s]", url.c_str());
//     //m_file = m_http.getStream();  //Get the response payload as Stream
//     _size = stoi(m_http.get_header("content-length"));
//     Debug_printv("length=%d", _size);

//     // Is this text?
//     std::string ct = m_http.get_header("content-type").c_str();
//     Debug_printv("content_type[%s]", ct.c_str());
//     isText = mstr::isText(ct);

//     return true;
// };

// uint32_t WebDAVIStream::read(uint8_t* buf, uint32_t size) {
//     auto bytesRead= m_http.read( buf, size );
//     _position+=bytesRead;
//     return bytesRead;
// };

// bool WebDAVIStream::isOpen() {
//     return _is_open;
// };
