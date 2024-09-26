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

#include "iec.h"

#include <memory>

using namespace Protocol;

systemBus IEC;

systemBus::systemBus()
{
	init();
} // ctor

// Set all IEC_signal lines in the correct mode
//
bool systemBus::init()
{
	// make sure the output states are initially LOW
	release(PIN_IEC_ATN);
	release(PIN_IEC_CLK_IN);
	release(PIN_IEC_DATA_IN);
	release(PIN_IEC_SRQ);

	// initial pin modes in GPIO
	pinMode(PIN_IEC_ATN, INPUT);
	pinMode(PIN_IEC_CLK_IN, INPUT);
	pinMode(PIN_IEC_DATA_IN, INPUT);
	pinMode(PIN_IEC_SRQ, OUTPUT);
	pinMode(PIN_IEC_RESET, INPUT);

#ifdef SPLIT_LINES
	pinMode(IEC_PIN_CLK_OUT, OUTPUT);
	pinMode(IEC_PIN_DATA_OUT, OUTPUT);
#endif

	flags = CLEAR;

	return true;
} // init


// void systemBus::init_gpio(gpio_num_t _pin)
// {
//     PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[_pin], PIN_FUNC_GPIO);
//     gpio_set_direction(_pin, GPIO_MODE_INPUT);
//     gpio_pullup_en(_pin);
//     gpio_set_pull_mode(_pin, GPIO_PULLUP_ONLY);
//     gpio_set_level(_pin, LOW);
//     return;
// }

// true => PULL => LOW
void IRAM_ATTR systemBus::pull ( uint8_t _pin )
{
	GPF(_pin) = GPFFS(GPFFS_GPIO(_pin)); // Set mode to GPIO
	GPC(_pin) = (GPC(_pin) & (0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
	GPES = (1 << _pin); // Enable
    // GPIO_MODE_OUTPUT
}

// false => RELEASE => HIGH
void IRAM_ATTR systemBus::release ( uint8_t _pin )
{
	GPF(_pin) = GPFFS(GPFFS_GPIO(_pin)); // Set mode to GPIO
	GPEC = (1 << _pin); // Disable
	GPC(_pin) = (GPC(_pin) & (0xF << GPCI)) | (1 << GPCD); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
	// GPIO_MODE_INPUT
}

bool IRAM_ATTR systemBus::status ( uint8_t _pin )
{
    return GPIP(_pin) ? RELEASED : PULLED;
}

bool IRAM_ATTR systemBus::status ()
{
    // uint64_t pin_states;
    // pin_states = REG_READ(GPIO_IN1_REG);
    // pin_states << 32 & REG_READ(GPIO_IN_REG);
    // pin_atn = pin_states & BIT(PIN_IEC_ATN);
    // pin_clk = pin_states & BIT(PIN_IEC_CLK_IN);
    // pin_data = pin_states & BIT(PIN_IEC_DATA_IN);
    // pin_srq = pin_states & BIT(PIN_IEC_SRQ);
    // pin_reset = pin_states & BIT(PIN_IEC_RESET);
    return true;
};


/******************************************************************************
 *                                                                             *
 *                               Public functions                              *
 *                                                                             *
 ******************************************************************************/

// This function checks and deals with atn signal commands
//
// If a command is recieved, the data.string is saved in data. Only commands
// for *this* device are dealt with.
//
/** from Derogee's "IEC Disected"
 * ATN SEQUENCES
 * When ATN is PULLED true, everybody stops what they are doing. The processor will quickly protocol.pull the
 * Clock line true (it's going to send soon), so it may be hard to notice that all other devices protocol.release the
 * Clock line. At the same time, the processor protocol.releases the Data line to false, but all other devices are
 * getting ready to listen and will each protocol.pull Data to true. They had better do this within one
 * millisecond (1000 microseconds), since the processor is watching and may sound an alarm ("device
 * not available") if it doesn't see this take place. Under normal circumstances, transmission now
 * takes place as previously described. The computer is sending commands rather than data, but the
 * characters are exchanged with exactly the same timing and handshakes as before. All devices
 * receive the commands, but only the specified device acts upon it. This results in a curious
 * situation: you can send a command to a nonexistent device (try "OPEN 6,6") - and the computer
 * will not know that there is a problem, since it receives valid handshakes from the other devices.
 * The computer will notice a problem when you try to send or receive data from the nonexistent
 * device, since the unselected devices will have dropped off when ATN ceased, leaving you with
 * nobody to talk to.
 */
// Return value, see systemBus::BusState definition.

void IRAM_ATTR systemBus::service()
{
    // pull( PIN_IEC_SRQ );

    // Disable Interrupt
    // gpio_intr_disable((gpio_num_t)PIN_IEC_ATN);

    if (state < BUS_ACTIVE)
    {
        // debugTiming();

        // // Handle SRQ for devices
        // for (auto devicep : _daisyChain)
        // {
        //     for (unsigned char i=0;i<16;i++)
        //         devicep->poll_interrupt(i);
        // }

        return;
    }

#ifdef IEC_HAS_RESET
    // Check if CBM is sending a reset (setting the RESET line high). This is typically
    // when the CBM is reset itself. In this case, we are supposed to reset all states to initial.
    bool pin_reset = status(PIN_IEC_RESET);
    if (pin_reset == PULLED)
    {
        if (status(PIN_IEC_ATN) == PULLED)
        {
            // If RESET & ATN are both PULLED then CBM is off
            state = BUS_OFFLINE;
            // gpio_intr_enable((gpio_num_t)PIN_IEC_ATN);
            return;
        }

        //Debug_printf("IEC Reset! reset[%d]\r\n", pin_reset);
        data.init(); // Clear bus data
        releaseLines();
        state = BUS_IDLE;
        //Debug_printv("bus init");

        // Reset virtual devices
        reset_all_our_devices();
        // gpio_intr_enable((gpio_num_t)PIN_IEC_ATN);
        return;
    }
#endif

    // Command or Data Mode
    do
    {
        if (state == BUS_ACTIVE)
        {
            //pull ( PIN_IEC_SRQ );

            // *** IMPORTANT! This helps keep us in sync!
            // Sometimes the C64 pulls ATN but doesn't pull CLOCK right away
            protocol->timeoutWait ( PIN_IEC_CLK_IN, PULLED, TIMEOUT_ATNCLK, false );

            // Read bus command bytes
            //Debug_printv("command");
            read_command();

            //release ( PIN_IEC_SRQ );
        }

        if (state == BUS_PROCESS)
        {
            // Reset bit/byte
            bit = 0;
            byte = 0;

            //Debug_printv("data");
            //pull ( PIN_IEC_SRQ );

            // Data Mode - Get Command or Data
            if (data.primary == IEC_LISTEN)
            {
                //Debug_printv("calling deviceListen()\r\n");
                //pull ( PIN_IEC_SRQ );
                deviceListen();
                //release ( PIN_IEC_SRQ );
            }
            else if (data.primary == IEC_TALK)
            {
                //Debug_printv("calling deviceTalk()\r\n");
                //pull ( PIN_IEC_SRQ );
                deviceTalk();
                //release ( PIN_IEC_SRQ );
                Debug_printf(" (%.2X %s  %.2d CHANNEL)\r\n", data.secondary, data.action.c_str(), data.channel);
            }
            else if (data.primary == IEC_UNLISTEN)
            {
                state = BUS_RELEASE;
            }

            // Switch to detected protocol
            if (data.secondary == IEC_OPEN || data.secondary == IEC_REOPEN)
            {
                //pull ( PIN_IEC_SRQ );
                protocol = selectProtocol();
                //release ( PIN_IEC_SRQ );
            }

            // Queue control codes and command in specified device
            //pull ( PIN_IEC_SRQ );
            auto d = deviceById(data.device);
            if (d != nullptr)
            {
                device_state_t device_state = d->queue_command(data);

                //fnLedManager.set(eLed::LED_BUS, true);

                //Debug_printv("bus[%d] device[%d]", state, device_state);
                // for (auto devicep : _daisyChain)
                // {
                    device_state = d->process();
                    if ( device_state < DEVICE_ACTIVE || device_state == DEVICE_TALK )
                    {
                        state = BUS_RELEASE;
                    }
                // }
            }

            //Debug_printv("bus[%d] device[%d] flags[%d]", state, device_state, flags);

            // Switch back to standard serial
            detected_protocol = PROTOCOL_SERIAL;
            protocol = selectProtocol();
            //release ( PIN_IEC_SRQ );

            if ( status ( PIN_IEC_ATN ) )
            {
                state = BUS_ACTIVE;
            }
        }

    } while( state > BUS_IDLE );

     // Clean Up
    if ( state == BUS_RELEASE )
    {
        releaseLines();
        data.init();
    }

    //Debug_printv ( "primary[%.2X] secondary[%.2X] bus[%d] flags[%d]", data.primary, data.secondary, state, flags );
    //Debug_printv ( "device[%d] channel[%d]", data.device, data.channel);

    Debug_printv("bus[%d] flags[%d]", state, flags);
//    Debug_printf("Heap: %lu\r\n",esp_get_free_internal_heap_size());

    //release( PIN_IEC_SRQ );
    //fnLedStrip.stopRainbow();
    //fnLedManager.set(eLed::LED_BUS, false);
}

void systemBus::read_command()
{
    //pull( PIN_IEC_SRQ );
    uint8_t c = receive();
    //release( PIN_IEC_SRQ );

    // Check for error
    if ( flags & ERROR )
    {
        Debug_printv("Error reading command. flags[%d] c[%X]", flags, c);
        state = BUS_ERROR;
        
        return;
    }
    else if ( flags & EMPTY_STREAM)
    {
        state = BUS_RELEASE;
    }
    else
    {
        if ( flags & JIFFYDOS_ACTIVE )
        {
            Debug_printf("   IEC: [JD][%.2X]", c);
            detected_protocol = PROTOCOL_JIFFYDOS;
        }
        else
        {
            Debug_printf("   IEC: [%.2X]", c);
        }

        // Decode command byte
        uint8_t command = c & 0x60;
        if (c == IEC_UNLISTEN)
            command = IEC_UNLISTEN;
        if (c == IEC_UNTALK)
            command = IEC_UNTALK;

        //Debug_printv ( "device[%d] channel[%d]", data.device, data.channel);
        //Debug_printv ("command[%.2X]", command);

        switch (command)
        {
        // case IEC_GLOBAL:
        //     data.primary = IEC_GLOBAL;
        //     data.device = c ^ IEC_GLOBAL;
        //     state = BUS_IDLE;
        //     Debug_printf(" (00 GLOBAL %.2d COMMAND)\r\n", data.device);
        //     break;

        case IEC_LISTEN:
            data.primary = IEC_LISTEN;
            data.device = c ^ IEC_LISTEN;
            data.secondary = IEC_REOPEN; // Default secondary command
            data.channel = CHANNEL_COMMAND;  // Default channel
            data.payload = "";
            state = BUS_ACTIVE;
            Debug_printf(" (20 LISTEN %.2d DEVICE)\r\n", data.device);
            break;

        case IEC_UNLISTEN:
            data.primary = IEC_UNLISTEN;
            state = BUS_PROCESS;
            Debug_printf(" (3F UNLISTEN)\r\n");
            break;

        case IEC_TALK:
            data.primary = IEC_TALK;
            data.device = c ^ IEC_TALK;
            data.secondary = IEC_REOPEN; // Default secondary command
            data.channel = CHANNEL_COMMAND;  // Default channel
            state = BUS_ACTIVE;
            Debug_printf(" (40 TALK  %.2d DEVICE)\r\n", data.device);
            break;

        case IEC_UNTALK:
            data.primary = IEC_UNTALK;
            data.secondary = 0x00;
            state = BUS_RELEASE;
            Debug_printf(" (5F UNTALK)\r\n");
            break;

        default:

            //pull ( PIN_IEC_SRQ );
            std::string secondary;
            state = BUS_PROCESS;

            command = c & 0xF0;
            switch ( command )
            {
            case IEC_OPEN:
                data.secondary = IEC_OPEN;
                data.channel = c ^ IEC_OPEN;
                data.action = "OPEN";
                break;

            case IEC_REOPEN:
                data.secondary = IEC_REOPEN;
                data.channel = c ^ IEC_REOPEN;
                data.action = "DATA";
                break;

            case IEC_CLOSE:
                data.secondary = IEC_CLOSE;
                data.channel = c ^ IEC_CLOSE;
                data.action = "CLOSE";
                break;

            default:
                state = BUS_IDLE;
            }

            if ( data.primary != IEC_TALK )
                Debug_printf(" (%.2X %s  %.2d CHANNEL)\r\n", data.secondary, data.action.c_str(), data.channel);
        }
    }

    //if ( state == BUS_ACTIVE )  // Normal behaviour is to ignore everything if it's not for us
    if ( state == BUS_PROCESS )  // Use this to sniff the secondary commands
    {
        if ( !isDeviceEnabled( data.device ) )
        {
            // Is this command for us?
            state = BUS_RELEASE; // NOPE!
        }
    }
    else if ( state == BUS_PROCESS )
    {
        // *** IMPORTANT! This helps keep us in sync!
        // Sometimes ATN isn't released immediately. Wait for ATN to be
        // released before trying to process the command. 
        // Long ATN delay (>1.5ms) seems to occur more frequently with VIC-20.
        //pull ( PIN_IEC_SRQ );
        protocol->timeoutWait ( PIN_IEC_ATN, RELEASED, TIMEOUT_DEFAULT, false );

        // Delay after ATN is RELEASED
        //protocol->wait( TIMING_Ttk, false );
        //release ( PIN_IEC_SRQ );
    }

    //Debug_printv ( "code[%.2X] primary[%.2X] secondary[%.2X] bus[%d] flags[%d]", c, data.primary, data.secondary, state, flags );
    //Debug_printv ( "device[%d] channel[%d]", data.device, data.channel);

    //release( PIN_IEC_SRQ );
}

void systemBus::read_payload()
{
    // Record the command string until ATN is PULLED
    // NOTE: string is just a container, it may contain arbitrary bytes but a LOT of code treats payload as a string
    std::string listen_command = "";

    // ATN might get pulled right away if there is no command string to send
    //pull ( PIN_IEC_SRQ );
    while (status(PIN_IEC_ATN) != PULLED)
    {
        //pull ( PIN_IEC_SRQ );
        int16_t c = receive();
        //Debug_printv("c[%2X]", c);
        //release ( PIN_IEC_SRQ );

        if (flags & EMPTY_STREAM || flags & ERROR)
        {
            Debug_printv("flags[%02X]", flags);
            state = BUS_ERROR;
            //release ( PIN_IEC_SRQ );
            return;
        }

        //if (c != 0xFFFFFFFF ) // && c != 0x0D) // Leave 0x0D to be stripped later
        //{
            listen_command += (uint8_t)c;
        //}

        if (flags & EOI_RECVD)
            break;
    }
    data.payload = listen_command;

    state = BUS_IDLE;
    //release ( PIN_IEC_SRQ );
}


void IRAM_ATTR systemBus::deviceListen()
{
    // If the command is SECONDARY and it is not to expect just a small command on the command channel, then
    // we're into something more heavy. Otherwise read it all out right here until UNLISTEN is received.
    if (data.secondary == IEC_REOPEN && data.channel != CHANNEL_COMMAND)
    {
        // A heapload of data might come now, too big for this context to handle so the caller handles this, we're done here.
        // Debug_printf(" (%.2X SECONDARY) (%.2X CHANNEL)\r\n", data.primary, data.channel);
        Debug_printf("REOPEN on non-command channel.\r\n");
        state = BUS_ACTIVE;
    }

    // OPEN or DATA
    else if (data.secondary == IEC_OPEN || data.secondary == IEC_REOPEN)
    {
        read_payload();
        std::string s = mstr::toHex(data.payload);
        Serial.printf("Device #%02d:%02d {%s} [%s]\r\n", data.device, data.channel, data.payload.c_str(), s.c_str());
    }

    // CLOSE Named Channel
    else if (data.secondary == IEC_CLOSE)
    {
        // Debug_printf(" (E0 CLOSE) (%d CHANNEL)\r\n", data.channel);
        state = BUS_PROCESS;
    }

    // Unknown
    else
    {
        Debug_printf(" OTHER (%.2X COMMAND) (%.2X CHANNEL) ", data.secondary, data.channel);
        state = BUS_ERROR;
    }
}


void IRAM_ATTR systemBus::deviceTalk()
{
    // Now do bus turnaround
    //pull(PIN_IEC_SRQ);
    if (!turnAround())
    {
        Debug_printv("error flags[%d]", flags);
        state = BUS_ERROR;
        return;
    }
    //release(PIN_IEC_SRQ);

    // We have recieved a CMD and we should talk now:
    state = BUS_PROCESS;
}


void IRAM_ATTR systemBus::releaseLines(bool wait)
{
    //pull ( PIN_IEC_SRQ );

    // Release lines
    release(PIN_IEC_CLK_OUT);
    release(PIN_IEC_DATA_OUT);

    // Wait for ATN to release and quit
    if (wait)
    {
        Debug_printv("Waiting for ATN to release");
        protocol->timeoutWait ( PIN_IEC_ATN, RELEASED, TIMEOUT_DEFAULT, false );
    }

    //release ( PIN_IEC_SRQ );
}

void IRAM_ATTR systemBus::senderTimeout()
{
    releaseLines();
    this->state = BUS_ERROR;

    protocol->wait( TIMING_EMPTY );
    pull( PIN_IEC_DATA_OUT );
} // senderTimeout



std::shared_ptr<IECProtocol> systemBus::selectProtocol() 
{
    //Debug_printv("protocol[%d]", detected_protocol);
    
    switch(detected_protocol)
    {
#ifdef MEATLOAF_MAX
        case PROTOCOL_SAUCEDOS:
        {
            auto p = std::make_shared<SauceDOS>();
            return std::static_pointer_cast<IECProtocol>(p);
        }
#endif
        case PROTOCOL_JIFFYDOS:
        {
            auto p = std::make_shared<JiffyDOS>();
            return std::static_pointer_cast<IECProtocol>(p);
        }
#ifdef PARALLEL_BUS
        case PROTOCOL_DOLPHINDOS:
        {
            auto p = std::make_shared<DolphinDOS>();
            return std::static_pointer_cast<IECProtocol>(p);
        }
#endif
        default:
        {
#ifdef PARALLEL_BUS
            PARALLEL.state = PBUS_IDLE;
#endif
            auto p = std::make_shared<CPBStandardSerial>();
            return std::static_pointer_cast<IECProtocol>(p);
        }
    }
}


// IEC_receive receives a byte
//
int16_t systemBus::receive(uint8_t device)
{
	int16_t data;
	data = protocol.receiveByte(); // Standard CBM Timing
#ifdef DATA_STREAM
	Debug_printf("%.2X ", data);
#endif
	// if(data < 0)
	// 	protocol.flags = errorFlag;

	return data;
} // receive


// IEC_send sends a byte
//
bool systemBus::send(uint8_t data)
{
#ifdef DATA_STREAM
	Debug_printf("%.2X ", data);
#endif
	return protocol.sendByte(data, false); // Standard CBM Timing
} // send

bool systemBus::send(std::string data)
{
	for (size_t i = 0; i < data.length(); ++i)
		send(data[i]);

	return true;
}


// Same as IEC_send, but indicating that this is the last byte.
//
bool systemBus::sendEOI(uint8_t data)
{
#ifdef DATA_STREAM
	Debug_printf("%.2X ", data);
#endif
	Debug_println("\r\nEOI Sent!");
	if(!protocol.sendByte(data, true))
		return false;

	return true;
} // sendEOI


// A special send command that informs file not found condition
//
bool systemBus::sendFNF()
{
	// Message file not found by just releasing lines
	release(PIN_IEC_DATA_OUT);
	release(PIN_IEC_CLK_OUT);

	// BETWEEN BYTES TIME
	delayMicroseconds(TIMING_Tbb);

	Debug_println("\r\nFNF Sent!");
	return true;
} // sendFNF


bool systemBus::isDeviceEnabled(const uint8_t deviceNumber)
{
	return (enabledDevices & (1<<deviceNumber));
} // isDeviceEnabled

void systemBus::enableDevice(const uint8_t deviceNumber)
{
	enabledDevices |= 1UL << deviceNumber;
} // enableDevice

void systemBus::disableDevice(const uint8_t deviceNumber)
{
	enabledDevices &= ~(1UL << deviceNumber);
} // disableDevice



void systemBus::debugTiming()
{
	int pin = PIN_IEC_ATN;
	pull(pin);
	delayMicroseconds(1000); // 1000
	release(pin);
	delayMicroseconds(1000);

	pin = PIN_IEC_CLK_OUT;
	pull(pin);
	delayMicroseconds(20); // 20
	release(pin);
	delayMicroseconds(1);

	pin = PIN_IEC_DATA_OUT;
	pull(pin);
	delayMicroseconds(50); // 50
	release(pin);
	delayMicroseconds(1);

	pin = PIN_IEC_SRQ;
	pull(pin);
	delayMicroseconds(60); // 60
	release(pin);
	delayMicroseconds(1);

	pin = PIN_IEC_ATN;
	pull(pin);
	delayMicroseconds(100); // 100
	release(pin);
	delayMicroseconds(1);

	pin = PIN_IEC_CLK_OUT;
	pull(pin);
	delayMicroseconds(200); // 200
	release(pin);
	delayMicroseconds(1);
}