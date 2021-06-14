#include "fs_example.h"

#include "meat_io.h"
#include "../../include/make_unique.h"
#include "../../include/global_defines.h"
#include "buffered_io.h"
#include <string>
#include "../EdUrlParser/EdUrlParser.h"
#include "stream_writer.h"
#include "peoples_url_parser.h"

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>



void testReader(MFile* readeTest) {
    // /* Test Line reader */
    std::shared_ptr<MIstream> readerStream(readeTest->inputStream());
    StreamReader reader(readerStream.get());

    std::string line = reader.readLn();

    while(!reader.eof()) {
        Serial.printf("FSTEST: line read:'%s'\n",line.c_str());
        line = reader.readLn();
    };
}

void testFileProperties(MFile* cserverFile) {
    Serial.printf("FSTEST: %s, isDir = %d\n", cserverFile->path.c_str(), cserverFile->isDirectory());
    Serial.printf("URL: [%s]\n", cserverFile->url().c_str());
    //Serial.printf("Root: [%s]\n", cserverFile->root().c_str());
    Serial.printf("Base: [%s]\n", cserverFile->base().c_str());
    Serial.printf("Scheme: [%s]\n", cserverFile->scheme.c_str());
    Serial.printf("Username: [%s]\n", cserverFile->username.c_str());
    Serial.printf("Password: [%s]\n", cserverFile->password.c_str());
    Serial.printf("Host: [%s]\n", cserverFile->hostname.c_str());
    Serial.printf("Port: [%s]\n", cserverFile->port.c_str());
    Serial.printf("Path: [%s]\n", cserverFile->path.c_str());
    Serial.printf("File: [%s]\n", cserverFile->filename.c_str());
    Serial.printf("Extension: [%s]\n", cserverFile->extension.c_str());
    Serial.printf("Query: [%s]\n", cserverFile->query.c_str());
    Serial.printf("Fragment: [%s]\n\n", cserverFile->fragment.c_str());
    Serial.printf("Local path: [%s]\n\n", cserverFile->localPath.c_str());
    Serial.printf("-------------------------------\n");
}

void testDirectory(MFile* dir, bool verbose=false) {
    std::unique_ptr<MFile> entry(dir->getNextFileInDir());

    while(entry != nullptr) {
        if(verbose)
            testFileProperties(entry.get());
        else
            Serial.printf("%s\n", entry->url().c_str());
        entry.reset(dir->getNextFileInDir());
    }
}


void testRecursiveDir(MFile* file, std::string indent) {
    if(file->isDirectory()) {
        std::unique_ptr<MFile> entry(file->getNextFileInDir());

        while(entry != nullptr) {
            if(entry->isDirectory())
            {
                Serial.printf("%s%s <dir>\n", indent.c_str(), entry->filename.c_str());
                testRecursiveDir(entry.get(), indent+"   ");
            }
            else
            {
                Serial.printf("%s%s\n", indent.c_str(), entry->filename.c_str());                
            }

            entry.reset(file->getNextFileInDir());
        }
    }
}

void testCopy(MFile* srcFile, MFile* dstFile) {
    char exampleText[]="Proletariusze wszystkich krajow, laczcie sie!";

    // test 1 - write some string to a plain file in root dir

    // if(!srcFile->exists()) {
    //     Serial.printf("FSTEST: %s doesn't exist!\n", srcFile->path.c_str());
    //     return;
    // }

    if(dstFile->exists()) {
        bool result = dstFile->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", dstFile->path.c_str(), result);
    }

    Serial.println("FSTEST copy: attempt obtain istream");
    std::shared_ptr<MIstream> srcStream(srcFile->inputStream());
    Serial.println("FSTEST copy: attempt obtain ostream");
    std::shared_ptr<MOstream> dstStream(dstFile->outputStream());

    if(!srcStream->isOpen()) {
        Serial.println("FSTEST: couldn't open a stream for reading");
        return;
    }
    else if(!dstStream->isOpen()) {
        Serial.println("FSTEST: couldn't open a stream for writing");
        return;
    }
    else {
        auto br = std::make_unique<BufferedReader>(srcStream.get());
        auto bw = std::make_unique<BufferedWriter>(dstStream.get());

        do {
            auto buffer = br->read();
            
            if(buffer->length() != 0) {
                Serial.printf("FSTEST: Bytes read into buffred reader: %d\n",buffer->length());
                int written = bw->write(buffer);
                Serial.printf("FSTEST: Bytes written into buffred writer: %d\n",written);

            }
        } while (!br->eof());
    }

    srcStream->close(); // not required, closes automagically
    dstStream->close(); // nor required, closes automagically
}

void dumpParts(std::vector<std::string> v) {
    for(auto i = v.begin(); i < v.end(); i++)
        Serial.printf("%s::",(*i).c_str());
}

void testPaths() {
    Serial.printf("pa = %s\n", mstr::drop("dupa",2).c_str());
    Serial.printf("du = %s\n", mstr::dropLast("dupa",2).c_str());


    std::shared_ptr<MFile> testFile(MFSOwner::File("http://somneserver.com/utilities/disk tools/cie.dnp/subdir/CIE+SERIAL"));

    std::shared_ptr<MFile> inDnp(testFile->cd("/inDnp"));
    Serial.printf("*** inDnp root: '%s'\n", inDnp->url().c_str());

    std::shared_ptr<MFile> inFlash(testFile->cd("//iFlash"));
    Serial.printf("*** inFlash root: '%s'\n", inFlash->url().c_str());

    std::shared_ptr<MFile> parallel(testFile->cd("../inSubdir"));
    Serial.printf("*** inSubdir root: '%s'\n", parallel->url().c_str());

    std::shared_ptr<MFile> inCie(testFile->cd("inCie"));
    Serial.printf("*** inCie root: '%s'\n", inCie->url().c_str());


} 


void htmlStream(char *url)
{
    bool success = true;
    size_t i = 0;
    size_t b_len = 1;
	uint8_t b[b_len];
    Debug_printv("Opening '%s'\r\n", url);
    std::shared_ptr<MFile> file(MFSOwner::File(url));

    if (file->exists())
    {
        size_t len = file->size();
        Debug_printv("File exists! size [%d]\r\n", len);

        std::shared_ptr<MIstream> stream(file->inputStream());

		for(i=0;i < len; i++)
		{
			success = stream->read(b, b_len);
			if (success)
			{
                Serial.write(b, b_len);
            }
        }
        stream->close();
        Debug_println("");
        Debug_printv("%d of %d bytes sent\r\n", i, len);
    }
    else
    {
        Debug_printv("File does not exist.\r\n");
    }
}

void httpGet(char *url)
{
    bool success = true;
    size_t i = 0;
    WiFiClient client;
    HTTPClient http;
    size_t b_len = 1;
	uint8_t b[1];

    http.setUserAgent("some agent");
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.setRedirectLimit(10);

    Serial.printf("[HTTP] begin... [%s]\n", url);
    if (http.begin(client, url)) {  // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();
      size_t len = http.getSize();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

            for(i=0;i < len; i++)
            {
                success = client.readBytes(b, b_len);
                if (success)
                {
                    Serial.write(b, b_len);
                }
            }
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      client.stop();
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
}


void runTests() {
    testPaths();
    // this is a directory for verious directories tests
    // std::shared_ptr<MFile> testDir(MFSOwner::File("cs:///utilities"));

    // // this is a file for tests that run on files
    // std::shared_ptr<MFile> testFile(MFSOwner::File("cs:///utilities/disk tools/cie.d64/CIE+SERIAL"));

    // // this is a file for write tests
    // std::shared_ptr<MFile> destFile(MFSOwner::File("/testfile"));

    // std::shared_ptr<MFile> otherFile(MFSOwner::File("/"));

    // Uncomment as needed
    //testReader(testFile.get());
    //testFileProperties(testDir.get());
    //testDirectory(testDir.get());

    // !!!!!!!!!!!!!!!!
    // TODO - this function fucks up littlefs, when run next tests will crash meatloaf
    // si this is probably some bug in dir reading functions of littlefs!
    // !!!!!!!!!!!!!!!!
    //testRecursiveDir(otherFile.get(),"");

    //testFileProperties(destFile.get());

    //testCopy(testFile.get(), destFile.get());

    htmlStream("http://jigsaw.w3.org/HTTP/connection.html");
    //httpGet("http://jigsaw.w3.org/HTTP/connection.html");
}

