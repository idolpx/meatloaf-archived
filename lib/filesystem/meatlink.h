#ifndef MEATLINK_DEFINES_H
#define MEATLINK_DEFINES_H

#include "../../include/global_defines.h"
#include "FS.h"
#include "meat_file.h"

#define RECORD_SIZE 256

class MeatLink {
    MeatLink(FS* fileSystem, String linkFileName) 
    {
        m_fileSystem = fileSystem;
        m_fileName = linkFileName;

        if( m_fileSystem->exists(m_fileName) )
        {
            #if defined(ESP32)
            uint8_t buffer[RECORD_SIZE] = { 0 };
            #elif defined(ESP8266)
            char buffer[RECORD_SIZE] = { 0 };
            #endif

            Serial.printf("Getting meatlink [%s]", linkFileName.c_str());
            File f_link = m_fileSystem->open(m_fileName, "r+");
            if(!f_link) {
                size_t readCount = f_link.readBytes(buffer, RECORD_SIZE);
                m_url = String(buffer); // is this enoug? Or do I have to set size somehow?
                f_link.close();
            }
        }
    }

    String get() 
    {
        return m_url;
    }

    void put(String url) 
    {
        File f_linkFile = m_fileSystem->open(m_fileName, "w+");

        if(f_linkFile)
        {
#if defined(ESP32)
            uint8_t buffer[RECORD_SIZE] = { 0 };
#elif defined(ESP8266)
            const char buffer[RECORD_SIZE] = { 0 };
#endif
            sprintf( (char *)buffer, "%s", url.c_str() );
            f_linkFile.write(buffer, RECORD_SIZE);

            Serial.printf("Writing meatlink file %s: %s\r\n", m_fileName.c_str(), m_url.c_str());                    

            f_linkFile.close();
        }
    }

    private:
        FS* m_fileSystem;
        String m_fileName;
        String m_url;
};

#endif