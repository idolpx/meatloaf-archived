#ifndef MEATLOAF_STREAM_BROKER
#define MEATLOAF_STREAM_BROKER

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include "meatloaf.h"
#include "iec_pipe_broker.h"
#include "../device/disk.h"

// Custom hash function for std::pair<std::string, std::ios_base::openmode>
struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        std::size_t h1 = std::hash<T1>{}(p.first);
        std::size_t h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1); // Combine the two hash values
    }
};

class StreamBroker {
private:
    // Define the key type for the cache
    using CacheKey = std::pair<std::string, std::ios_base::openmode>;

    // Define the type for storing cached MStream instances
    std::unordered_map<CacheKey, std::shared_ptr<MStream>, PairHash> streamCache;

public:
    // Method to retrieve MStream instance from cache or create a new one if not found
    std::shared_ptr<MStream> getSourceStream(MFile* streamFile, std::ios_base::openmode mode) {
        CacheKey key = std::make_pair(streamFile->path, mode);
        auto it = streamCache.find(key);
        if (it != streamCache.end()) {
            return it->second; // Found in cache, return the cached instance
        } else {
            // Not found, create a new instance and cache it
            std::shared_ptr<MStream> newStream(streamFile->getSourceStream(mode));
            streamCache[key] = newStream;
            return newStream;
        }
    }

    // Function to remove streamCache entries without matching iecPipe instances
    void flushInactiveStreams(iecPipeBroker& broker) {
        const auto& deviceChannelMap = broker.getDeviceChannelMap();

        for (auto keyStreamPair = streamCache.begin(); keyStreamPair != streamCache.end();) {
            bool inUse = false;
            for (const auto& keyPipePair : deviceChannelMap) {
                if (keyPipePair.second->getFilename().find(keyStreamPair->first.first) != std::string::npos) {
                    inUse = true;
                    break;
                }
            }

            if (!inUse) {
                keyStreamPair->second->close(); // Close the stream before removing
                keyStreamPair = streamCache.erase(keyStreamPair); // Remove the entry if no matching iecPipe found
            } else {
                ++keyStreamPair;
            }
        }
    }
};

#endif /* MEATLOAF_STREAM_BROKER */
