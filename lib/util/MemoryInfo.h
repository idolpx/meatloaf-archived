// C++ for Arduino
// What is heap fragmentation?
// https://cpp4arduino.com/

#pragma once

#include <stddef.h> // for size_t

// Returns the number of total bytes in the RAM.
size_t getTotalMemory();

// Returns the number of free bytes in the RAM.
size_t getTotalAvailableMemory();

// Returns the size of the largest allocable block of RAM.
size_t getLargestAvailableBlock();

// Computes the heap fragmentation percentage.
inline float getFragmentation() {
  return 100 - getLargestAvailableBlock() * 100.0 / getTotalAvailableMemory();
}
