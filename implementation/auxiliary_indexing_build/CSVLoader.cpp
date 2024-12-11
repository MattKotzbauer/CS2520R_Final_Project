// CSVLoader.cpp
#include "CSVLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

CSVLoader::CSVLoader(const std::string& filename) : filename_(filename) {}

bool CSVLoader::createIndex(const std::string& column) {
    if (std::find(headers_.begin(), headers_.end(), column) == headers_.end()) {
        std::cerr << "Column '" << column << "' does not exist in CSV." << std::endl;
        return false;
    }

    // Determine key type based on column data
    // For simplicity, assume string type. Implement dynamic type detection as needed
    KeyType key_type = KeyType::STRING; // Change based on your requirements
    std::string index_filename = column + ".btree";

    // Check if index file already exists
    if (fs::exists(index_filename)) {
        // Load existing index
        auto btree = std::make_shared<BTree>(index_filename, key_type);
        if (!btree->load()) {
            std::cerr << "Failed to load existing B-tree index: " << index_filename << std::endl;
            return false;
        }
        indexes_[column] = btree;
        std::cout << "Loaded existing B-tree index for column: " << column << std::endl;
    } else {
        // Create a new B-tree index
        auto btree = std::make_shared<BTree>(index_filename, key_type);
        btree->initialize(); // Initialize B-tree (create root node, etc.)
        indexes_[column] = btree;
        std::cout << "Initialized new B-tree index for column: " << column << std::endl;
    }

    return true;
}

std::shared_ptr<BTree> CSVLoader::getIndex(const std::string& column) const {
    auto it = indexes_.find(column);
    if (it != indexes_.end()) {
        return it->second;
    }
    return nullptr;
}

bool CSVLoader::load() {
    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename_ << std::endl;
        return false;
    }

    std::string line;
    // Read headers
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            headers_.push_back(cell);
        }
    } else {
        std::cerr << "Empty CSV file: " << filename_ << std::endl;
        return false;
    }

    // Read data
    uint64_t row_num = 0;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::unordered_map<std::string, std::string> row;
        size_t idx = 0;
        uint64_t file_offset = file.tellg(); // Optional: Store file offset if needed
        while (std::getline(ss, cell, ',') && idx < headers_.size()) {
            row[headers_[idx]] = cell;
            // Insert into index if applicable
            if (indexes_.find(headers_[idx]) != indexes_.end()) {
                auto key_str = cell;
                // Convert key_str to appropriate type based on index
                auto btree = indexes_[headers_[idx]];
                OperandValue key_val;
                // Example for string type
                if (btree->getKeyType() == KeyType::STRING) {
                    key_val = key_str;
                }
                // Handle other types (int, double) as needed
                // Insert key and row number or file offset
                btree->insert(key_val, row_num); // Assuming row_num as data pointer
            }
            idx++;
        }
        data_.push_back(row);
        row_num++;
    }

    file.close();

    // Save indexes after building
    for (auto& [column, btree] : indexes_) {
        if (!btree->save()) {
            std::cerr << "Failed to save B-tree index for column: " << column << std::endl;
            // Handle error as needed
        }
    }

    return true;
}
