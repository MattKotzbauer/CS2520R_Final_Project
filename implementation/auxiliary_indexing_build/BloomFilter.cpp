
#include "BloomFilter.h"

BloomFilter::BloomFilter(size_t size, size_t num_hashes)
    : size_(size), num_hashes_(num_hashes), bit_array_(size, false) {}

void BloomFilter::insert(const std::string& element) {
    auto hashes = getHashes(element);
    for (const auto& hash : hashes) {
        bit_array_[hash % size_] = true;
    }
}

bool BloomFilter::contains(const std::string& element) const {
    auto hashes = getHashes(element);
    for (const auto& hash : hashes) {
        if (!bit_array_[hash % size_]) {
            return false;
        }
    }
    return true;
}

bool BloomFilter::serialize(std::fstream& file) const {
    if (!file.is_open()) {
        return false;
    }

    // Write size and number of hash functions
    file.write(reinterpret_cast<const char*>(&size_), sizeof(size_));
    file.write(reinterpret_cast<const char*>(&num_hashes_), sizeof(num_hashes_));

    // Write bit array
    for (const auto& bit : bit_array_) {
        char byte = bit ? 1 : 0;
        file.write(&byte, sizeof(char));
    }

    return true;
}

bool BloomFilter::deserialize(std::fstream& file) {
    if (!file.is_open()) {
        return false;
    }

    // Read size and number of hash functions
    file.read(reinterpret_cast<char*>(&size_), sizeof(size_));
    file.read(reinterpret_cast<char*>(&num_hashes_), sizeof(num_hashes_));

    // Resize bit array
    bit_array_.resize(size_);

    // Read bit array
    for (size_t i = 0; i < size_; ++i) {
        char byte;
        file.read(&byte, sizeof(char));
        bit_array_[i] = (byte != 0);
    }

    return true;
}

std::vector<size_t> BloomFilter::getHashes(const std::string& element) const {
    std::vector<size_t> hashes;
    std::hash<std::string> hash_fn1;
    std::hash<std::string> hash_fn2;

    size_t hash1 = hash_fn1(element);
    size_t hash2 = hash_fn2(element);

    for (size_t i = 0; i < num_hashes_; ++i) {
        hashes.push_back(hash1 + i * hash2);
    }

    return hashes;
}
        char byte = bit ? 1 : 0;
        file.write(&byte, sizeof(char));
    }

    return true;

    for (size_t i = 0; i < num_hashes_; ++i) {
        hashes.push_back(hash1 + i * hash2);
    }

    return hashes;
}

    size_t hash1 = hash_fn1(element);
    size_t hash2 = hash_fn2(element);
}
        bit_array_[i] = (byte != 0);
    }
    std::hash<std::string> hash_fn1;
    std::hash<std::string> hash_fn2;

std::vector<size_t> BloomFilter::getHashes(const std::string& element) const {
    std::vector<size_t> hashes;

    return true;
}
    for (size_t i = 0; i < size_; ++i) {
        char byte;
        file.read(&byte, sizeof(char));
    bit_array_.resize(size_);

    // Read bit array

bool BloomFilter::deserialize(std::fstream& file) {
    if (!file.is_open()) {
        return false;
    }

    // Read size and number of hash functions
    file.read(reinterpret_cast<char*>(&size_), sizeof(size_));
    file.read(reinterpret_cast<char*>(&num_hashes_), sizeof(num_hashes_));

    // Resize bit array

    // Write bit array
    for (const auto& bit : bit_array_) {
    file.write(reinterpret_cast<const char*>(&size_), sizeof(size_));
    file.write(reinterpret_cast<const char*>(&num_hashes_), sizeof(num_hashes_));
    if (!file.is_open()) {
        return false;
    }

    // Write size and number of hash functions
    return true;
}

bool BloomFilter::serialize(std::fstream& file) const {
    for (const auto& hash : hashes) {
        if (!bit_array_[hash % size_]) {
            return false;
        }
    }

bool BloomFilter::contains(const std::string& element) const {
    auto hashes = getHashes(element);
    for (const auto& hash : hashes) {
        bit_array_[hash % size_] = true;
    }
}

void BloomFilter::insert(const std::string& element) {
    auto hashes = getHashes(element);
    : size_(size), num_hashes_(num_hashes), bit_array_(size, false) {}
#include "BloomFilter.h"

BloomFilter::BloomFilter(size_t size, size_t num_hashes)