// Meatloaf - A Commodore 64/128 multi-device emulator
// https://github.com/idolpx/meatloaf
// Copyright(C) 2020 James Johnston
//
// This file is part of Meatloaf but adapted for use in the FujiNet project
// https://github.com/FujiNetWIFI/fujinet-platformio
// 
// Meatloaf is free software : you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Meatloaf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Meatloaf. If not, see <http://www.gnu.org/licenses/>.

#include <string.h>

#include "disk.h"

#include "../../include/global_defines.h"

using namespace CBM;

iecDisk::iecDisk(void)
{
	reset();
} // ctor


void iecDisk::sendStatus(void)
{
	uint8_t i, readResult;
	
	std::string status("00, OK, 00, 08");
	readResult = status.length();

	Debug_printf("\r\nsendStatus: ");
	// Length does not include the CR, write all but the last one should be with EOI.
	for (i = 0; i < readResult - 2; ++i)
		IEC.send(status[i]);

	// ...and last byte in string as with EOI marker.
	IEC.sendEOI(status[i]);
} // sendStatus


void iecDisk::sendDeviceStatus()
{
	Debug_printf("\r\nsendDeviceStatus:\r\n");

	// Reset basic memory pointer:
	uint16_t basicPtr = C64_BASIC_START;

	// Send load address
	IEC.send(C64_BASIC_START bitand 0xff);
	IEC.send((C64_BASIC_START >> 8) bitand 0xff);
	Debug_println("");

	// Send List HEADER
	sendLine(basicPtr, 0, "\x12 %s V%s ", PRODUCT_ID, FW_VERSION);

	// Current Config
	sendLine(basicPtr, 0, "DEVICE    : %d", _device_id);
	sendLine(basicPtr, 0, "DRIVE     : %d", _drive);
	sendLine(basicPtr, 0, "PARTITION : %d", _partition);
	sendLine(basicPtr, 0, "URL       : %s", _url.c_str());
	sendLine(basicPtr, 0, "PATH      : %s", _path.c_str());
	sendLine(basicPtr, 0, "ARCHIVE   : %s", _archive.c_str());
	sendLine(basicPtr, 0, "IMAGE     : %s", _image.c_str());
	sendLine(basicPtr, 0, "FILENAME  : %s", _filename.c_str());

	// End program with two zeros after last line. Last zero goes out as EOI.
	IEC.send(0);
	IEC.sendEOI(0);

	ledON();
} // sendDeviceStatus



void iecDisk::_process_command(void)
{
	_device_id = IEC.ATN.device_id;
	_filename = IEC.ATN.data;
	util_string_trim(_filename);
	_extension = _filename.substr(_filename.find_last_of(".") + 1);
	util_string_toupper(_extension);
	if (_extension.length() > 4 || _extension.length() == _filename.length())
		_extension = "";

	Dir local_file = m_fileSystem->openDir(std::string(_path + _filename).c_str());

	//Serial.printf("\r\n$IEC: DEVICE[%d] DRIVE[%d] PARTITION[%d] URL[%s] PATH[%s] IMAGE[%s] FILENAME[%s] FILETYPE[%s] COMMAND[%s]\r\n", _device_id, _drive, _partition, _url.c_str(), _path.c_str(), _image.c_str(), _filename.c_str(), _extension.c_str(), IEC.ATN.data);
	if (util_starts_with(_filename, "$"))
	{
		_openState = O_DIR;
	}
	else if (local_file.isDirectory())
	{
		// Enter directory
		Debug_printf("\r\nchangeDir: [%s] >", _filename.c_str());
		_path = _path + _filename.substr(3) + "/";
		_openState = O_DIR;
	}
	else if (std::string(IMAGE_TYPES).find_last_of(_extension) >= 0 && _extension.length() > 0)
	{
		// Mount image file
		Debug_printf("\r\nmount: [%s] >", _filename.c_str());
		_image = _filename;

		_openState = O_DIR;
	}
	else if (util_starts_with(_filename, "HTTP://"))
	{
		// Mount url
		Debug_printf("\r\nmount: [%s] >", _filename.c_str());
		_partition = 0;
		_url = _filename.substr(7);
		_path = "/";
		_image = "";

		_openState = O_DIR;
	}
	else if (util_starts_with(_filename, "CD"))
	{
		if (util_ends_with(_filename, "_"))
		{
			if (_image.length())
			{
				// Unmount image file
				//Debug_printf("\r\nunmount: [%s] <", _image.c_str());
				_image = "";
			}
			else if (_url.length() && _path == "/")
			{
				// Unmount url
				//Debug_printf("\r\nunmount: [%s] <", _url.c_str());
				_url = "";
			}
			else
			{
				// Go back a directory
				//Debug_printf("\r\nchangeDir: [%s] <", _filename.c_str());
				//m_device.path(_path.substr(0, _path.find_last_of("/", _path.length() - 2) + 1));
				_path = _path.substr(0, _path.find_last_of("/", _path.length() - 2) + 1);

				if (!_path.length())
				{
					_path = "/";
				}
			}
		}
		else if (_filename.length() > 3)
		{
			// Switch to root
			if (util_starts_with(_filename, "CD//"))
			{
				_path = "/";
				_image = "";
			}

			if (std::string(IMAGE_TYPES).find_last_of(_extension) >= 0 && _extension.length() > 0)
			{
				// Mount image file
				//Debug_printf("\r\nmount: [%s] >", _filename.c_str());
				_image = _filename.substr(3);
			}
			else
			{
				// Enter directory
				//Debug_printf("\r\nchangeDir: [%s] >", _filename.c_str());
				_path = _path + _filename.substr(3) + "/";
			}
		}

		if (IEC.ATN.channel == 0x00)
		{
			_openState = O_DIR;
		}
	}
	else if (util_starts_with(_filename, "@INFO"))
	{
		_filename = "";
		_openState = O_SYSTEM_INFO;
	}
	else if (util_starts_with(_filename, "@STAT"))
	{
		_filename = "";
		_openState = O_DEVICE_STATUS;
	}
	else
	{
		_openState = O_FILE;
	}

	if (_openState == O_DIR)
	{
		_filename = "$";
		_extension = "";
		IEC.ATN.data[0] = '\0';
	}

	//Debug_printf("\r\nhandleATNCmdCodeOpen: %d (M_OPENSTATE) [%s]", _openState, IEC.ATN.data);
	Debug_printf("\r\n$IEC: DEVICE[%d] DRIVE[%d] PARTITION[%d] URL[%s] PATH[%s] IMAGE[%s] FILENAME[%s] FILETYPE[%s] COMMAND[%s]\r\n", _device_id, _drive, _partition, _url.c_str(), _path.c_str(), _image.c_str(), _filename.c_str(), _extension.c_str(), IEC.ATN.data);

} // _process_command


void iecDisk::_talk_data(uint8_t chan)
{
	// process response into _queuedError.
	// Response: ><code in binary><CR>

	Debug_printf("\r\nhandleATNCmdCodeDataTalk: %d (CHANNEL) %d (M_OPENSTATE)", chan, _openState);

	if (chan == CMD_CHANNEL)
	{
		// Send status message
		sendStatus();
		// go back to OK state, we have dispatched the error to IEC host now.
		_queuedError = ErrOK;
	}
	else
	{

		//Debug_printf("\r\nm_openState: %d", _openState);

		switch (_openState)
		{
			case O_NOTHING:
				// Say file not found
				IEC.sendFNF();
				break;

			case O_INFO:
				// Reset and send SD card info
				reset();
				sendListing();
				break;

			case O_FILE:
				// Send program file
				if (_url.length())
				{
					sendFileHTTP();
				}
				else
				{
					sendFile();
				}
				break;

			case O_DIR:
				// Send listing
				if (_url.length())
				{
					sendListingHTTP();
				}
				else
				{
					sendListing();
				}
				break;


			case O_FILE_ERR:
				// FIXME: interface with Host for error info.
				//sendListing(/*&send_file_err*/);
				IEC.sendFNF();
				break;

			case O_SYSTEM_INFO:
				// Send device info
				sendSystemInfo();
				break;

			case O_DEVICE_STATUS:
				// Send device info
				sendDeviceStatus();
				break;
		}
	}

} // _talk_data


void iecDisk::_listen_data()
{

} // _listen_data


void iecDisk::_close()
{
	Debug_printf("\r\nhandleATNCmdClose: Success!");

	//Serial.printf("\r\nIEC: DEVICE[%d] DRIVE[%d] PARTITION[%d] URL[%s] PATH[%s] IMAGE[%s] FILENAME[%s] FILETYPE[%s]\r\n", _device_id, _drive, _partition, _url.c_str(), _path.c_str(), _image.c_str(), _filename.c_str(), _extension.c_str());
	Debug_printf("\r\n=================================\r\n\r\n");

	_filename = "";
} // handleATNCmdClose


// send single basic line, including heading basic pointer and terminating zero.
uint16_t iecDisk::sendLine(uint16_t &basicPtr, uint16_t blocks, const char* format, ...)
{
	// Format our string
	va_list args;
	va_start(args, format);
	char text[vsnprintf(NULL, 0, format, args) + 1];
	vsnprintf(text, sizeof text, format, args);
	va_end(args);

	return sendLine(basicPtr, blocks, text);
}

uint16_t iecDisk::sendLine(uint16_t &basicPtr, uint16_t blocks, char* text)
{
	uint8_t i;
	uint16_t b_cnt = 0;

	Debug_printf("%d %s ", blocks, text);

	// Get text length
	uint8_t len = strlen(text);

	// Increment next line pointer
	basicPtr += len + 5;

	// Send that pointer
	IEC.send(basicPtr bitand 0xFF);
	IEC.send(basicPtr >> 8);

	// Send blocks
	IEC.send(blocks bitand 0xFF);
	IEC.send(blocks >> 8);

	// Send line contents
	for (i = 0; i < len; i++)
		IEC.send(text[i]);

	// Finish line
	IEC.send(0);

	Debug_println("");

	b_cnt += (len + 5);

	return b_cnt;
} // sendLine


uint16_t iecDisk::sendHeader(uint16_t &basicPtr)
{
	uint16_t byte_count = 0;

	// Send List HEADER
	// "      MEAT LOAF 64      "
	byte space_cnt = (16 - strlen(PRODUCT_ID)) / 2;
	byte_count += sendLine(basicPtr, 0, "\x12\"%*s%s%*s\" %.02d 2A", space_cnt, "", PRODUCT_ID, space_cnt, "", _device_id);

	// Send Extra INFO
	if (_url.length())
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 3, "", 16, "[URL]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 3, "", 16, _url.c_str());
	}
	if (_path.length() > 1)
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 3, "", 16, "[PATH]");
		byte_count += sendLine(basicPtr, 0, "%*s\"%-*s\" NFO", 3, "", 16, _path.c_str());
	}
	if (_url.length() + _path.length() > 1)
	{
		byte_count += sendLine(basicPtr, 0, "%*s\"----------------\" NFO", 3, "");
	}

	return byte_count;
}

void iecDisk::sendListing()
{
	Debug_printf("\r\nsendListing:\r\n");

	uint16_t byte_count = 0;
	String extension = "DIR";

	// Reset basic memory pointer:
	uint16_t basicPtr = C64_BASIC_START;

	// Send load address
	IEC.send(C64_BASIC_START bitand 0xff);
	IEC.send((C64_BASIC_START >> 8) bitand 0xff);
	byte_count += 2;
	Debug_println("");

	byte_count += sendHeader(basicPtr);

	// Send List ITEMS
	//byte_count += sendLine(basicPtr, 1, "\"THIS IS A FILE\"     PRG");
	//byte_count += sendLine(basicPtr, 5, "\"THIS IS A FILE 2\"   PRG");

	Dir dir = m_fileSystem->openDir(_path.c_str());
	while (dir.next())
	{
		uint16_t block_cnt = dir.fileSize() / 256;
		byte block_spc = 3;
		if (block_cnt > 9)
			block_spc--;
		if (block_cnt > 99)
			block_spc--;
		if (block_cnt > 999)
			block_spc--;

		byte space_cnt = 21 - (dir.fileName().length() + 5);
		if (space_cnt > 21)
			space_cnt = 0;

		if (dir.fileSize())
		{
			block_cnt = dir.fileSize() / 256;

			uint8_t ext_pos = dir.fileName().lastIndexOf(".") + 1;
			if (ext_pos && ext_pos != dir.fileName().length())
			{
				extension = dir.fileName().substring(ext_pos);
				extension.toUpperCase();
			}
			else
			{
				extension = "PRG";
			}
		}
		else
		{
			extension = "DIR";
		}

		// Don't show hidden folders or files
		if (!dir.fileName().startsWith("."))
		{
			byte_count += sendLine(basicPtr, block_cnt, "%*s\"%s\"%*s %3s", block_spc, "", dir.fileName().c_str(), space_cnt, "", extension.c_str());
		}

		//Debug_printf(" (%d, %d)\r\n", space_cnt, byte_count);
		toggleLED(true);
	}

	byte_count += sendFooter(basicPtr);

	// End program with two zeros after last line. Last zero goes out as EOI.
	IEC.send(0);
	IEC.sendEOI(0);

	Debug_printf("\r\nsendListing: %d Bytes Sent\r\n", byte_count);

	ledON();
} // sendListing


uint16_t iecDisk::sendFooter(uint16_t &basicPtr)
{
	// Send List FOOTER
	// #if defined(USE_LITTLEFS)
	FSInfo64 fs_info;
	m_fileSystem->info64(fs_info);
	return sendLine(basicPtr, (fs_info.totalBytes - fs_info.usedBytes) / 256, "BLOCKS FREE.");
	// #elif defined(USE_SPIFFS)
	// 	return sendLine(basicPtr, 00, "UNKNOWN BLOCKS FREE.");
	// #endif
	//Debug_println("");
}


void iecDisk::sendFile()
{
	uint16_t i = 0;
	bool success = true;

	uint16_t bi = 0;
	uint16_t load_address = 0;
	char b[1];
	char ba[9];

	ba[8] = '\0';

	// Update device database
	//m_device.save();

	// Find first program
	if (util_ends_with(_filename, "*"))
	{
		_filename = "";

		if (_path == "/" && _image.length() == 0)
		{
			_filename = "FB64";
		}
		else
		{

			Dir dir = m_fileSystem->openDir(_path.c_str());
			while (dir.next() && dir.isDirectory())
			{
				Debug_printf("\r\nsendFile: %s", dir.fileName().c_str());
			}
			if (dir.isFile())
				_filename = dir.fileName().c_str();
		}
	}
	std::string inFile = _path + _filename;

	File file = m_fileSystem->open(inFile.c_str(), "r");

	if (!file.available())
	{
		Debug_printf("\r\nsendFile: %s (File Not Found)\r\n", inFile.c_str());
		IEC.sendFNF();
	}
	else
	{
		size_t len = file.size();

		// Get file load address
		file.readBytes(b, 1);
		success = IEC.send(b[0]);
		load_address = *b & 0x00FF; // low byte
		file.readBytes(b, 1);
		success = IEC.send(b[0]);
		load_address = load_address | *b << 8;  // high byte
		// fseek(file, 0, SEEK_SET);

		Debug_printf("\r\nsendFile: [%s] [$%.4X] (%d bytes)\r\n=================================\r\n", inFile.c_str(), load_address, len);
		for (i = 2; success and i < len; ++i) 
		{
			success = file.readBytes(b, 1);
			if (success)
			{
#ifdef DATA_STREAM
				if (bi == 0)
				{
					Debug_printf(":%.4X ", load_address);
					load_address += 8;
				}
#endif
				if (i == len)
				{
					success = IEC.sendEOI(b[0]); // indicate end of file.
				}
				else
				{
					success = IEC.send(b[0]);
				}

#ifdef DATA_STREAM
				// Show ASCII Data
				if (b[0] < 32 || b[0] >= 127) 
					b[0] = 46;

				ba[bi++] = b[0];

				if(bi == 8)
				{
					size_t t = (i * 100) / len;
					Debug_printf(" %s (%d %d%%)\r\n", ba, i, t);
					bi = 0;
				}
#endif
				// Toggle LED
				if (i % 50 == 0)
				{
					toggleLED(true);
				}
			}
		}
		file.close();
		Debug_println("");
		Debug_printf("%d of %d bytes sent\r\n", i, len);

		ledON();

		if (!success || i != len)
		{
			Debug_println("sendFile: Transfer failed!");
		}
	}
} // sendFile

void iecDisk::saveFile()
{
	std::string outFile = _path + _filename;
	byte b;

	Debug_printf("\r\nsaveFile: %s", outFile.c_str());

	File file = m_fileSystem->open(outFile.c_str(), "w");
	//	noInterrupts();
	if (!file.available())
	{
		Debug_printf("\r\nsaveFile: %s (Error)\r\n", outFile.c_str());
	}
	else
	{
		bool done = false;
		// Recieve bytes until a EOI is detected
		do
		{
			b = IEC.receive();
			done = (IEC.state() bitand IECState::eoiFlag) or (IEC.state() bitand IECState::errorFlag);

			file.write(b);
		} while (not done);
		file.close();
	}
	//	interrupts();
} // saveFile


void iecDisk::sendListingHTTP(void)
{
	Debug_printf("\r\nsendListingHTTP: ");

	uint16_t byte_count = 0;

	std::string user_agent = std::string(PRODUCT_ID) + " [" + std::string(FW_VERSION) + "]";
	std::string url = "http://" + _url + "/api/";
	std::string post_data = "p=" + urlencode(_path.c_str()) + "&i=" + urlencode(_image.c_str()) + "&f=" + urlencode(_filename.c_str());

	// Connect to HTTP server
	HTTPClient client;
	client.setUserAgent(user_agent.c_str());
	// client.setFollowRedirects(true);
	client.setTimeout(10000);
	if (!client.begin(url.c_str()))
	{
		Debug_println(F("\r\nConnection failed"));
		IEC.sendFNF();
		return;
	}
	client.addHeader("Content-Type", "application/x-www-form-urlencoded");

	Debug_printf("\r\nConnected!\r\n--------------------\r\n%s\r\n%s\r\n%s\r\n", user_agent.c_str(), url.c_str(), post_data.c_str());

	uint8_t httpCode = client.POST(post_data.c_str());	 //Send the request
	WiFiClient payload = client.getStream(); //Get the response payload as Stream
	//String payload = client.getString();    //Get the response payload as String

	Debug_printf("HTTP Status: %d\r\n", httpCode); //Print HTTP return code
	if (httpCode != 200)
	{
		Debug_println(F("Error"));
		IEC.sendFNF();
		return;
	}

	//Serial.println(payload);    //Print request response payload
	m_lineBuffer = payload.readStringUntil('\n');

	// Reset basic memory pointer:
	uint16_t basicPtr = C64_BASIC_START;

	// Send load address
	IEC.send(C64_BASIC_START bitand 0xff);
	IEC.send((C64_BASIC_START >> 8) bitand 0xff);
	byte_count += 2;
	Debug_println("");

	do
	{
		// Parse JSON object
		DeserializationError error = deserializeJson(m_jsonHTTP, m_lineBuffer);
		if (error)
		{
			Serial.print(F("\r\ndeserializeJson() failed: "));
			Serial.println(error.c_str());
			break;
		}

		byte_count += sendLine(basicPtr, m_jsonHTTP["blocks"], "%s", urldecode(m_jsonHTTP["line"].as<String>()).c_str());
		toggleLED(true);
		m_lineBuffer = payload.readStringUntil('\n');
		//Serial.printf("\r\nlinebuffer: %d %s", m_lineBuffer.length(), m_lineBuffer.c_str());
	} while (m_lineBuffer.length() > 1);

	//End program with two zeros after last line. Last zero goes out as EOI.
	IEC.send(0);
	IEC.sendEOI(0);

	Debug_printf("\r\nBytes Sent: %d\r\n", byte_count);

	client.end(); //Close connection

	ledON();
} // sendListingHTTP

void iecDisk::sendFileHTTP(void)
{
	uint16_t i = 0;
	bool success = true;

	uint16_t bi = 0;
	uint16_t load_address = 0;
	char b[1];
	byte ba[9];

	ba[8] = '\0';

	// Update device database
	//m_device.save();

	Debug_printf("\r\nsendFileHTTP: ");

	std::string user_agent = std::string(PRODUCT_ID) + " [" + std::string(FW_VERSION) + "]";
	std::string url = "http://" + _url + "/api/";
	std::string post_data = "p=" + urlencode(_path.c_str()).c_str() + "&i=" + urlencode(_image.c_str()).c_str() + "&f=" + urlencode(_filename.c_str()).c_str();

	// Connect to HTTP server
	HTTPClient client;
	client.setUserAgent(user_agent.c_str());
	// client.setFollowRedirects(true);
	client.setTimeout(10000);
	if (!client.begin(url.c_str()))
	{
		Debug_println(F("\r\nConnection failed"));
		IEC.sendFNF();
		return;
	}
	client.addHeader("Content-Type", "application/x-www-form-urlencoded");

	Debug_printf("\r\nConnected!\r\n--------------------\r\n%s\r\n%s\r\n%s\r\n", user_agent.c_str(), url.c_str(), post_data.c_str());

	uint8_t httpCode = client.POST(post_data.c_str()); //Send the request
	WiFiClient file = client.getStream();  //Get the response payload as Stream

	if (!file.available())
	{
		Debug_printf("\r\nsendFileHTTP: %s (File Not Found)\r\n", url.c_str());
		IEC.sendFNF();
	}
	else
	{
		size_t len = client.getSize();

		// Get file load address
		file.readBytes(b, 1);
		success = IEC.send(b[0]);
		load_address = *b & 0x00FF; // low byte
		file.readBytes(b, 1);
		success = IEC.send(b[0]);
		load_address = load_address | *b << 8;  // high byte
		// fseek(file, 0, SEEK_SET);

		Debug_printf("\r\nsendFileHTTP: [%s] [$%.4X] (%d bytes)\r\n=================================\r\n", _filename.c_str(), load_address, len);
		for (i = 2; success and i < len; ++i)
		{ // End if sending to CBM fails.
			success = file.readBytes(b, 1);
			if (success)
			{
#ifdef DATA_STREAM
				if (bi == 0)
				{
					Debug_printf(":%.4X ", load_address);
					load_address += 8;
				}
#endif
				if (i == len)
				{
					success = IEC.sendEOI(b[0]); // indicate end of file.
				}
				else
				{
					success = IEC.send(b[0]);
				}

#ifdef DATA_STREAM
				// Show ASCII Data
				if (b[0] < 32 || b[0] >= 127) 
					b[0] = 46;

				ba[bi++] = b[0];

				if(bi == 8)
				{
					size_t t = (i * 100) / len;
					Debug_printf(" %s (%d %d%%)\r\n", ba, i, t);
					bi = 0;
				}
#endif
				// Toggle LED
				if (i % 50 == 0)
				{
					toggleLED(true);
				}
			}
		}
		client.end();
		Debug_println("");
		Debug_printf("%d of %d bytes sent\r\n", i, len);

		ledON();

		if (!success || i != len)
		{
			Debug_println("Transfer failed!");
		}
	}
}
