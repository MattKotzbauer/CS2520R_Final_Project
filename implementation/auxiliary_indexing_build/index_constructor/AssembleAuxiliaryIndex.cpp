#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iterator>

// Function to read a binary file into a vector
bool readFile(const std::string& filename, std::vector<uint8_t>& data) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open " << filename << " for reading.\n";
        return false;
    }
    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);
    std::streampos filesize = file.tellg();
    file.seekg(0, std::ios::beg);
    data.reserve(filesize);
    data.insert(data.begin(),
               std::istream_iterator<uint8_t>(file),
               std::istream_iterator<uint8_t>());
    file.close();
    return true;
}

int main() {
    // Read BTree.bin
    std::vector<uint8_t> btreeData;
    if (!readFile("BTree.bin", btreeData)) {
        return 1;
    }

    // Read BloomFilter.bin
    std::vector<uint8_t> bloomData;
    if (!readFile("BloomFilter.bin", bloomData)) {
        return 1;
    }

    // Calculate the offset where BloomFilter.bin will be stored
    // First 8 bytes for the offset, followed by BTree.bin
    uint64_t bloomOffset = 8 + btreeData.size(); // Offset in bytes

    // Open AuxiliaryIndex.bin for writing
    std::ofstream auxFile("AuxiliaryIndex.bin", std::ios::binary);
    if (!auxFile) {
        std::cerr << "Error: Cannot open AuxiliaryIndex.bin for writing.\n";
        return 1;
    }

    // Write the offset (8 bytes)
    auxFile.write(reinterpret_cast<const char*>(&bloomOffset), sizeof(bloomOffset));
    if (!auxFile) {
        std::cerr << "Error: Failed to write BloomFilter offset to AuxiliaryIndex.bin.\n";
        return 1;
    }

    // Write BTree.bin content
    auxFile.write(reinterpret_cast<const char*>(btreeData.data()), btreeData.size());
    if (!auxFile) {
        std::cerr << "Error: Failed to write BTree.bin content to AuxiliaryIndex.bin.\n";
        return 1;
    }

    // Write the copy of BloomFilter.bin
    auxFile.write(reinterpret_cast<const char*>(bloomData.data()), bloomData.size());
    if (!auxFile) {
        std::cerr << "Error: Failed to write BloomFilter.bin copy to AuxiliaryIndex.bin.\n";
        return 1;
    }

    auxFile.close();
    std::cout << "AuxiliaryIndex.bin created successfully (Version 2).\n";
    std::cout << "Structure:\n";
    std::cout << " - First 8 bytes: Offset to BloomFilter.bin data (" << bloomOffset << " bytes from start).\n";
    std::cout << " - Next bytes: Content of BTree.bin.\n";
    std::cout << " - Final bytes: Copy of BloomFilter.bin.\n";
    return 0;
}
