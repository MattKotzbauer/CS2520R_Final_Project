#ifndef CSVLOADER_H
#define CSVLOADER_H

#include <string>
#include <vector>
#include <unordered_map>

class CSVLoader {
public:
    CSVLoader(const std::string& filename);
    bool load();
    const std::vector<std::unordered_map<std::string, std::string>>& getData() const;
    const std::vector<std::string>& getHeaders() const;

private:
    std::string filename_;
    std::vector<std::unordered_map<std::string, std::string>> data_;
    std::vector<std::string> headers_;
};

#endif // CSVLOADER_H

