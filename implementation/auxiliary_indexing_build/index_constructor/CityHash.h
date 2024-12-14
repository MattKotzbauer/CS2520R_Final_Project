// CityHash.h
#ifndef CITYHASH_H
#define CITYHASH_H

#include <cstdint>
#include <cstddef>

// Function declarations
uint64_t CityHash64(const char* s, size_t len);
uint64_t CityHash64WithSeed(const char* s, size_t len, uint64_t seed);

#endif // CITYHASH_H
