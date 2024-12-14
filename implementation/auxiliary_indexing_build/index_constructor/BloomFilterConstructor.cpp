// BloomFilterConstructor.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <cstring>
#include "CityHash.h"  // Include the CityHash header

// MurmurHash3 Implementation
uint32_t murmurhash3_32(const std::string& key, uint32_t seed = 0) {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(key.data());
    int len = key.length();
    const int nblocks = len / 4;

    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    // Body
    const uint32_t* blocks = reinterpret_cast<const uint32_t*>(data + nblocks * 4);
    for (int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> (32 - 15));
        k1 *= c2;

        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> (32 - 13));
        h1 = h1 * 5 + 0xe6546b64;
    }

    // Tail
    const uint8_t* tail = reinterpret_cast<const uint8_t*>(data + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3) {
    case 3:
        k1 ^= tail[2] << 16;
    case 2:
        k1 ^= tail[1] << 8;
    case 1:
        k1 ^= tail[0];
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> (32 - 15));
        k1 *= c2;
        h1 ^= k1;
    }

    // Finalization
    h1 ^= len;

    // fmix (final mix)
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}

// Bloom Filter Class
class BloomFilter {
private:
    static const int FILTER_SIZE_BYTES = 64; // 64 bytes = 512 bits
    uint8_t filter[FILTER_SIZE_BYTES];

public:
    BloomFilter() {
        memset(filter, 0, sizeof(filter));
    }

    void add(const std::string& item) {
        // Hash the item using MurmurHash3 and CityHash64
        uint32_t hash1 = murmurhash3_32(item);
        uint64_t hash2 = CityHash64(item.c_str(), item.length());

        // Derive bit positions
        // Ensure the hash values are within [0, 511]
        uint32_t pos1 = hash1 % 512;
        uint32_t pos2 = hash2 % 512;

        setBit(pos1);
        setBit(pos2);
    }

    void setBit(uint32_t pos) {
        if (pos >= 512) return; // Out of range
        uint32_t byteIndex = pos / 8;
        uint32_t bitIndex = pos % 8;
        filter[byteIndex] |= (1 << bitIndex);
    }

    void serialize(const std::string& filename) const {
        std::ofstream out(filename, std::ios::binary);
        if (!out) {
            std::cerr << "Failed to open file for writing: " << filename << "\n";
            return;
        }
        out.write(reinterpret_cast<const char*>(filter), sizeof(filter));
        out.close();
        std::cout << "Bloom filter serialized to " << filename << "\n";
    }

    // Deserialize the Bloom filter from a binary file
    bool deserialize(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) {
            std::cerr << "Failed to open file for reading: " << filename << "\n";
            return false;
        }
        in.read(reinterpret_cast<char*>(filter), sizeof(filter));
        if (!in) {
            std::cerr << "Failed to read Bloom filter from file: " << filename << "\n";
            return false;
        }
        in.close();
        std::cout << "Bloom filter deserialized from " << filename << "\n";
        return true;
    }

    // Query if an item is possibly in the Bloom filter
    bool query(const std::string& item) const {
        // Hash the item using MurmurHash3 and CityHash64
        uint32_t hash1 = murmurhash3_32(item);
        uint64_t hash2 = CityHash64(item.c_str(), item.length());

        // Derive bit positions
        uint32_t pos1 = hash1 % 512;
        uint32_t pos2 = hash2 % 512;

        return getBit(pos1) && getBit(pos2);
    }

    // Helper function to get the value of a bit
    bool getBit(uint32_t pos) const {
        if (pos >= 512) return false; // Out of range
        uint32_t byteIndex = pos / 8;
        uint32_t bitIndex = pos % 8;
        return (filter[byteIndex] & (1 << bitIndex)) != 0;
    }

    // Optional: Function to display the Bloom filter in hex
    void display() const {
        std::cout << "Bloom Filter (Hex): ";
        for (int i = 0; i < FILTER_SIZE_BYTES; ++i) {
            printf("%02x", filter[i]);
        }
        std::cout << "\n";
    }
};

// Function to trim whitespace from both ends of a string
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

int main() {
    std::string csvFile = "data.csv";
    std::ifstream file(csvFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << csvFile << "\n";
        return 1;
    }

    BloomFilter bloomFilter;
    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (isHeader) {
            // Skip the header line
            isHeader = false;
            continue;
        }

        std::stringstream ss(line);
        std::string item;
        // Extract the first column (id)
        if (std::getline(ss, item, ',')) {
            std::string id = trim(item);
            bloomFilter.add(id);
        }
    }

    file.close();

    // Serialize the Bloom filter to BloomFilter.bin
    std::string binaryFile = "BloomFilter.bin";
    bloomFilter.serialize(binaryFile);

    // Optional: Deserialize and Query
    BloomFilter loadedBloomFilter;
    if (loadedBloomFilter.deserialize(binaryFile)) {
        // Query some IDs
        std::vector<std::string> testIDs = { "1", "2", "3", "4", "5" };
        for (const auto& id : testIDs) {
            bool exists = loadedBloomFilter.query(id);
            std::cout << "ID " << id << (exists ? " is possibly in the set.\n" : " is definitely not in the set.\n");
        }
    }

    return 0;
}
