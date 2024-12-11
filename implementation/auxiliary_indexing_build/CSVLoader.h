// CSVLoader.h
#ifndef CSVLOADER_H
#define CSVLOADER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "BTree.h"

class CSVLoader {
public:
    CSVLoader(const std::string& filename);
    bool load();
    const std::vector<std::unordered_map<std::string, std::string>>& getData() const;
    const std::vector<std::string>& getHeaders() const;
    
    // New methods
    bool createIndex(const std::string& column);
    std::shared_ptr<BTree> getIndex(const std::string& column) const;
    
private:
    std::string filename_;
    std::vector<std::unordered_map<std::string, std::string>> data_;
    std::vector<std::string> headers_;
    
    // Map of column name to B-tree index
    std::unordered_map<std::string, std::shared_ptr<BTree>> indexes_;
};

#endif // CSVLOADER_H
