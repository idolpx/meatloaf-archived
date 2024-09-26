// Meatloaf - A Commodore 64/128 multi-device emulator
// https://github.com/idolpx/meatloaf
// Copyright(C) 2020 James Johnston
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

#ifndef IEC_H
#define IEC_H

#include <Arduino.h>
#include <memory>

#include "../../include/global_defines.h"
#include "../../include/cbmdefines.h"
#include "../../include/petscii.h"
#include "string_utils.h"

#include "protocol/_protocol.h"
#include "protocol/jiffydos.h"
#ifdef MEATLOAF_MAX
#include "protocol/saucedos.h"
#endif
#ifdef PARALLEL_BUS
#include "protocol/dolphindos.h"
#endif

#define	IEC_CMD_MAX_LENGTH 	100

using namespace Protocol;


// Return values for service:
typedef enum
{
    BUS_OFFLINE = -4,   // Bus is empty
    BUS_RESET = -3,     // The bus is in a reset state (RESET line).
    BUS_ERROR = -2,     // A problem occoured, reset communication
    BUS_RELEASE = -1,   // Clean Up
    BUS_IDLE = 0,       // Nothing recieved of our concern
    BUS_ACTIVE = 1,     // ATN is pulled and a command byte is expected
    BUS_PROCESS = 2,    // A command is ready to be processed
} bus_state_t;

/**
 * @enum bus command
 */
typedef enum
{
    IEC_GLOBAL = 0x00,    // 0x00 + cmd (global command)
    IEC_LISTEN = 0x20,    // 0x20 + device_id (LISTEN) (0-30)
    IEC_UNLISTEN = 0x3F,  // 0x3F (UNLISTEN)
    IEC_TALK = 0x40,      // 0x40 + device_id (TALK) (0-30)
    IEC_UNTALK = 0x5F,    // 0x5F (UNTALK)
    IEC_REOPEN = 0x60,    // 0x60 + channel (OPEN CHANNEL) (0-15)
    IEC_REOPEN_JD = 0x61, // 0x61 + channel (OPEN CHANNEL) (0-15) - JIFFYDOS LOAD
    IEC_CLOSE = 0xE0,     // 0xE0 + channel (CLOSE NAMED CHANNEL) (0-15)
    IEC_OPEN = 0xF0       // 0xF0 + channel (OPEN NAMED CHANNEL) (0-15)
} bus_command_t;

typedef enum
{
    DEVICE_ERROR = -1,
    DEVICE_IDLE = 0,    // Ready and waiting
    DEVICE_ACTIVE = 1,
    DEVICE_LISTEN = 2,  // A command is recieved and data is coming to us
    DEVICE_TALK = 3,    // A command is recieved and we must talk now
    DEVICE_PROCESS = 4, // Execute device command
} device_state_t;

typedef enum {
    PROTOCOL_SERIAL,
    PROTOCOL_FAST_SERIAL,
    PROTOCOL_SAUCEDOS,
    PROTOCOL_JIFFYDOS,
    PROTOCOL_EPYXFASTLOAD,
    PROTOCOL_WARPSPEED,
    PROTOCOL_SPEEDDOS,
    PROTOCOL_DOLPHINDOS,
    PROTOCOL_WIC64,
    PROTOCOL_IEEE488
} bus_protocol_t;


/**
 * @class IECData
 * @brief the IEC command data passed to devices
 */
class IECData
{
public:
    /**
     * @brief the primary command byte
     */
    uint8_t primary = 0;
    /**
     * @brief the primary device number
     */
    uint8_t device = 0;
    /**
     * @brief the secondary command byte
     */
    uint8_t secondary = 0;
    /**
     * @brief the secondary command channel
     */
    uint8_t channel = 0;
    /**
     * @brief the device command
     */
    std::string payload = "";
    /**
     * @brief the raw bytes received for the command
     */
    std::vector<uint8_t> payload_raw;
    /**
     * @brief secondary action description
     */
    std::string action ="";
    /**
     * @brief clear and initialize IEC command data
     */
    void init(void)
    {
        //primary = 0;
        device = 0;
        secondary = 0;
        channel = 0;
        payload = "";
        payload_raw.clear();
        action = "";
    }
};


class systemBus
{
public:
	systemBus();
	~systemBus() {};

	// Initialise iec driver
	bool init();

	IECData data;

	// Checks if CBM is sending an attention message. If this is the case,
	// the message is recieved and stored in IEC->data.
	void service();

	// Checks if CBM is sending a reset (setting the RESET line high). This is typicall
	// when the CBM is reset itself. In this case, we are supposed to reset all states to initial.
//	bool checkRESET();

	// Sends a byte. The communication must be in the correct state: a load command
	// must just have been recieved. If something is not OK, FALSE is returned.
	bool send(byte data);
	bool send(std::string data);

	// Same as IEC_send, but indicating that this is the last byte.
	bool sendEOI(byte data);

	// A special send command that informs file not found condition
	bool sendFNF();

	// Recieves a byte
	int16_t receive(uint8_t device = 0);

	// Enabled Device Bit Mask
	uint32_t enabledDevices;
	bool isDeviceEnabled(const uint8_t deviceNumber);
	void enableDevice(const uint8_t deviceNumber);
	void disableDevice(const uint8_t deviceNumber);

	void debugTiming();

    /**
     * @brief signal to bus that we timed out.
     */
    void senderTimeout();

	//uint8_t state();
    uint16_t flags = CLEAR;

    uint8_t bit = 0;
    uint8_t byte = 0;

//    void init_gpio(uint8_t _pin);
    void pull ( uint8_t _pin );
    void release ( uint8_t _pin );
    bool status ( uint8_t _pin );
    bool status ();

private:
	// IEC Bus Commands
	void deviceListen();
	void deviceTalk();
	bool turnAround();

	void read_command();
	void read_payload();

	void releaseLines(bool wait = true);

    /**
     * @brief The chain of devices on the bus.
     */
    //std::forward_list<virtualDevice *> _daisyChain;

    /**
     * @brief the detected bus protocol
     */
    bus_protocol_t detected_protocol = PROTOCOL_SERIAL;  // default is IEC Serial

    /**
     * @brief Switch to detected bus protocol
     */
    std::shared_ptr<IECProtocol> selectProtocol();

protected:
    /**
     * @brief current bus state
     */
    bus_state_t state;
};
/**
 * @brief Return
 */
extern systemBus IEC;
#endif
