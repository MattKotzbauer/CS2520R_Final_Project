#include "CSVLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

CSVLoader::CSVLoader(const std::string& filename) : filename_(filename) {}

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
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::unordered_map<std::string, std::string> row;
        size_t idx = 0;
        while (std::getline(ss, cell, ',') && idx < headers_.size()) {
            row[headers_[idx++]] = cell;
        }
        data_.push_back(row);
    }

    file.close();
    return true;
}

const std::vector<std::unordered_map<std::string, std::string>>& CSVLoader::getData() const {
    return data_;
}

const std::vector<std::string>& CSVLoader::getHeaders() const {
    return headers_;
}

