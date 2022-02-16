// WS:// - WebSockets
// https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API
// https://en.wikipedia.org/wiki/WebSocket
//

#ifndef MEATFILESYSTEM_SCHEME_WS
#define MEATFILESYSTEM_SCHEME_WS

#include "meat_io.h"
#include "../../include/global_defines.h"
#include <ArduinoWebsockets.h>
#include "string_utils.h"

/********************************************************
 * Streams
 ********************************************************/

class CSIOStream: public MIStream, public MOStream {
public:
    CSIOStream(MFile* file, bool isServer) : m_isServer(isServer) {
        // drop ws:// from url and it's OK!
        address = websockets::WSInterfaceString(mstr::drop(file->url, 5).c_str());
        if(!file->port.empty())
            port = std::stoi(file->port);
    }

    ~CSIOStream() {
        close();
    }

    void close() override {
        client.close();        
    }

    bool open() override {
        if(m_isServer) {
            server.listen(port);
            client = server.accept();
            prepareClientCallbacks();
            m_isOpen = server.available();
        }
        else {
            prepareClientCallbacks();
            m_isOpen = client.connect(address);
            // waring - client->poll() required to keep it working!
        }

        return m_isOpen;
    };

    // MStream methods
    size_t position() override { return 0; };
    size_t available() override { return INT_MAX; };
    bool isOpen() { return m_isOpen; };
    bool seek(size_t pos) { return false; };
    size_t size() { return INT_MAX; };

    size_t read(uint8_t* buf, size_t size) override {
        //auto msg = client.readBlocking(); // we don't want to block. We'll store a message from callback in msg

        if(!m_isOpen)
            return 0;

        if(!lastMsg.isEmpty()) {
            auto maxLen = (size < lastMsg.length()) ? size : lastMsg.length();
            strncpy((char *)buf, lastMsg.c_str(), maxLen);
            Debug_printv("Sending last msg and polling!");
            lastMsg = "";
            client.poll();
            return maxLen;
        }
        else {
            Debug_printv("Polling!");
            client.poll();
            return 0;
        }
    }
    size_t write(const uint8_t *buf, size_t size) override {
        if(!m_isOpen)
            return 0;

        websockets::WSInterfaceString message((char *)buf);
        client.send(message);
        return message.length();
    }

protected:
    websockets::WSInterfaceString address; // format: www.myserver.com:8080
    uint16_t port = 80;
    bool m_isOpen;
    bool m_isServer;
    websockets::WebsocketsClient client;
    websockets::WebsocketsServer server;
    websockets::WSInterfaceString lastMsg;

    void prepareClientCallbacks() {
        client.onMessage([this](websockets::WebsocketsMessage msg){
            //
            // I think this get called only if you do client.poll()
            //
            Debug_printv("Got message: %s", msg.data().c_str());

            if(msg.isText()) {
                //(*msgPtr) = (std::string)msg.data();    
                this->lastMsg = msg.data();
            }
        });

        client.onEvent([this](websockets::WebsocketsEvent event, String data) {
            if(event == websockets::WebsocketsEvent::ConnectionOpened) {
                Debug_printv("Socket Connnection Opened");
                this->m_isOpen = true;
            } else if(event == websockets::WebsocketsEvent::ConnectionClosed) {
                Debug_printv("Socket Connnection Closed");
                this->m_isOpen = false;
            } else if(event == websockets::WebsocketsEvent::GotPing) {
                Debug_printv("Got a Ping!");
            } else if(event == websockets::WebsocketsEvent::GotPong) {
                Debug_printv("Got a Pong!");
            }
        });
    };
};

/********************************************************
 * File implementations
 ********************************************************/

class WSFile: public MFile {

public:
    WSFile(std::string path): MFile(path) {};

    bool isDirectory() override { return false; }
    MIStream* inputStream() override {
        // input stream = SERVER socket
        return new CSIOStream(this, true);
    }; 
    MOStream* outputStream() override {
        // output stream = CLIENT socket
        return new CSIOStream(this, false);
    }; 
    time_t getLastWrite() override { return 0; };
    time_t getCreationTime() override { return 0; };
    bool rewindDirectory() override { return false; };
    MFile* getNextFileInDir() override { return nullptr; };
    bool mkDir() override { return false; };
    bool exists() override { return false; };
    size_t size() override { return 0; };
    bool remove() override { return false; };
    bool rename(std::string dest) { return false; };
    MIStream* createIStream(std::shared_ptr<MIStream> src) {
        return nullptr;
    };

};



/********************************************************
 * FS
 ********************************************************/

class WSFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        return new WSFile(path);
    }

    bool handles(std::string name) {
        std::string pattern = "ws:";
        return mstr::equals(name, pattern, false);
    }
public:
    WSFileSystem(): MFileSystem("ws") {};
};



#endif /* MEATFILESYSTEM_SCHEME_WS */
