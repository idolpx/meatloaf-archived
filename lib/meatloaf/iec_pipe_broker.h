#ifndef MEATLOAF_PIPE_BROKER
#define MEATLOAF_PIPE_BROKER

#include <unordered_map>
#include <string>
#include <tuple>
#include <memory>
#include "iec_pipe.h"

/*

So this keeps pipes for you. When a new channel is requested, you call addPipe on this class, you will get
the new pipe in return value. Then you can call readFile() or writeFile() on it. These functions will
either do the whole work, or get interrupted by ATN.

If you get interrupted by ATN, you can call readFile() or writeFile() again, and it will resume from where it was interrupted.

When readFile() or writeFile() reaches EOF, it will be marked for removal by removeInactivePipes() automatically, but
if you need to close the pipe manually (i.e. because it was closed by C64), you can call removePipeByDeviceAndChannel(device, channel)
and it will be removed for you (writing last byte!)

*/

class iecPipeBroker {
    // Key is a tuple of device number and channel number
    //using DeviceChannelKey = std::tuple<int, int>;

    // Maps for storing and retrieving iecPipe instances
    std::unordered_map<int, std::unique_ptr<iecPipe>> deviceChannelMap;

public:
    // Adds a new iecPipe instance to the broker and returns a pointer to it
    iecPipe* addPipe(int device, int channel, const std::string& filename, std::ios_base::openmode mode, systemBus* bus) {
        int key = (device * 100) + channel;
        auto pipe = std::make_unique<iecPipe>();
        if (pipe->establish(filename, mode, bus)) {
            iecPipe* pipePtr = pipe.get();
            deviceChannelMap[key] = std::move(pipe);
            return pipePtr;
        } else {
            // Handle error if establishing the pipe fails
            return nullptr;
        }
    }

    // Retrieves an iecPipe by device and channel number
    iecPipe* getPipeByDeviceAndChannel(int device, int channel) {
        int key = (device * 100) + channel;
        auto it = deviceChannelMap.find(key);
        if (it != deviceChannelMap.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // Retrieves an iecPipe by filename
    iecPipe* getPipeByFilename(const std::string& filename) {
        for (auto& entry : deviceChannelMap) {
            if (entry.second->getFilename() == filename) {
                return entry.second.get();
            }
        }
        return nullptr;
    }

    // Removes an iecPipe by device and channel number
    void removePipeByDeviceAndChannel(int device, int channel) {
        int key = (device * 100) + channel;
        auto it = deviceChannelMap.find(key);
        if (it != deviceChannelMap.end()) {
            it->second->finish(); // Call finish method before removing
            deviceChannelMap.erase(it);
        }
    }

    // Removes an iecPipe by filename
    void removePipeByFilename(const std::string& filename) {
        for (auto it = deviceChannelMap.begin(); it != deviceChannelMap.end(); ++it) {
            if (it->second->getFilename() == filename) { // Assuming getFilename() method exists
                it->second->finish(); // Call finish method before removing
                deviceChannelMap.erase(it);
                break;
            }
        }
    }

    // Removes all inactive iecPipe instances
    void removeInactivePipes() {
        for (auto it = deviceChannelMap.begin(); it != deviceChannelMap.end(); ) {
            if (!it->second->isActive()) { 
                it->second->finish(); // Call finish method before removing
                it = deviceChannelMap.erase(it); // Erase and move to the next element
            } else {
                ++it;
            }
        }
    }

    // Returns a reference to the deviceChannelMap
    const std::unordered_map<int, std::unique_ptr<iecPipe>>& getDeviceChannelMap() {
        removeInactivePipes();
        return deviceChannelMap;
    }
};


#endif