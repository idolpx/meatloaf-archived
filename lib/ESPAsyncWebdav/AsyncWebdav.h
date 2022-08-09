#include <Arduino.h>

#include <fstream>

enum DavResourceType { DAV_RESOURCE_NONE, DAV_RESOURCE_FILE, DAV_RESOURCE_DIR };
enum DavDepthType { DAV_DEPTH_NONE, DAV_DEPTH_CHILD, DAV_DEPTH_ALL };


class AsyncWebdav: public AsyncWebHandler {
    public:
        AsyncWebdav(const String& url);

        virtual bool canHandle(AsyncWebServerRequest *request) override final;
        virtual void handleRequest(AsyncWebServerRequest *request) override final;
        virtual void handleBody(AsyncWebServerRequest *request, unsigned char *data, size_t len, size_t index, size_t total) override final;
        const char* url() const {
            return _url.c_str();
        }

    private:
        String _url;

        void handlePropfind(const String& path, DavResourceType resource, AsyncWebServerRequest * request);
        void handleGet(const String& path, DavResourceType resource, AsyncWebServerRequest * request);
        void handlePut(const String& path, DavResourceType resource, AsyncWebServerRequest * request, unsigned char *data, size_t len, size_t index, size_t total);
        void handleLock(const String& path, DavResourceType resource, AsyncWebServerRequest * request);
        void handleUnlock(const String& path, DavResourceType resource, AsyncWebServerRequest * request);
        void handleMkcol(const String& path, DavResourceType resource, AsyncWebServerRequest * request);
        void handleMove(const String& path, DavResourceType resource, AsyncWebServerRequest * request);
        void handleDelete(const String& path, DavResourceType resource, AsyncWebServerRequest * request);
        void handleHead(DavResourceType resource, AsyncWebServerRequest * request);
        void handleNotFound(AsyncWebServerRequest * request);
        void sendPropResponse(AsyncResponseStream *response, boolean recursing, File *curFile);
        String urlToUri(String url);

};




class filebufPropResponse : public std::filebuf {
    const size_t BUFFER_SIZE = 256;
    std::shared_ptr<MFile> container;
    char* buffer;
    byte phase = 0; // 0 = headers, 1 = files, 2 = footer

public:
    idirbuf() {
        buffer = new char[BUFFER_SIZE];
    };

    ~idirbuf() {
        if(buffer != nullptr)
            delete[] buffer;
    }

    void open(std::shared_ptr<MFile> c) {
        container = c;
        container->rewindDirectory(); // in case we're in the middle of reading here!
        phase = 0;
    }

    int underflow() override {
        if(phase == 0) {
            // OK, so you are in headers. 
            // First you have to fill this buffer line by line or as whole. Doesn't matter.
            // i.e. like this:
            size_t written = lineToBuffer(69,"some header line");
            // When you're done, call this:
            this->setg(buffer, buffer, buffer + written);      
            
            // of course filling it at once would be easiest, as you just fill it here and set this:
            //phase = 1;
            // if you fill line by line incremet some count and set phase to 1 when all lines written...
        }
        else if(phase == 1) {
            // So now we're in files list phase, let's try and get a file:
            std::unique_ptr<MFile> entry(container->getNextFileInDir());

            if (entry != nullptr) {
                // There's still a file in this dir, so... Same principle:
                // we'll put BASIV V2 into buffer variable using this function:
                auto readCount = fileToBasicV2(entry.get());
                // and set required pointers:
                this->setg(buffer, buffer, buffer + readCount);
            }
            else
                phase = 2; // nope, no more files here. Let's change phase, so we can write footer
                            // in the same pass. Otherwise we'll get an EOF!
        }
        
        if(phase == 2) {
            // ok, so now the footer, again - write it into the buffer and call:
            size_t written = lineToBuffer(69,"bytes free");
            this->setg(buffer, buffer, buffer + written);     
            phase = 3; 
            // next time underflow is called, we'll just return eof
        }
        else if(phase == 3) {
            // we've past the footer, let's send eof
            return std::char_traits<char>::eof();
        }

        return this->gptr() == this->egptr()
            ? std::char_traits<char>::eof()
            : std::char_traits<char>::to_int_type(*this->gptr());
    };

};