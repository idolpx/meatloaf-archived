// C++ for Arduino
// What is heap fragmentation?
// https://cpp4arduino.com/

// This source file captures the platform dependent code.
// This version was tested with ESP8266 core for Arduino version 2.4.2

#include "MemoryInfo.h"

#if defined(ESP8266)
#include <umm_malloc/umm_malloc.h>
#endif

// const size_t block_size = 8;

size_t getTotalMemory() {
#if defined(ESP32)
  return 0;
#elif defined(ESP8266)
  umm_info(0, 0);
  //return ummHeapInfo.totalBlocks * block_size;
  return umm_usage_metric();
#endif
}

size_t getTotalAvailableMemory() {
#if defined(ESP32)
  return 0;
#elif defined(ESP8266)
  umm_info(0, 0);
  //return ummHeapInfo.freeBlocks * block_size;
  return umm_free_heap_size();
#endif
}

size_t getLargestAvailableBlock() {
#if defined(ESP32)
  return 0;
#elif defined(ESP8266)
  umm_info(0, 0);
  //return ummHeapInfo.maxFreeContiguousBlocks * block_size;
  return umm_max_block_size();
#endif
}
