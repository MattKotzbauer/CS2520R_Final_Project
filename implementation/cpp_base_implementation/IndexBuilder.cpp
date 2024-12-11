// IndexBuilder.cpp
#include "BTree.h"
#include "CSVLoader.h"
#include <iostream>
#include <fstream>
#include <memory>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: IndexBuilder <csv_file> <column_name>" << std::endl;
        return 1;
    }

    std::string csv_filename = argv[1];
    std::string column_name = argv[2];

    // Initialize CSVLoader
    CSVLoader loader(csv_filename);

    // Specify which column to index
    if (!loader.createIndex(column_name)) {
        std::cerr << "Failed to create index for column: " << column_name << std::endl;
        return 1;
    }

    // Load CSV data and build B-tree
    if (!loader.load()) {
        std::cerr << "Failed to load CSV file: " << csv_filename << std::endl;
        return 1;
    }

    // Serialize the B-tree index
    auto btree = loader.getIndex(column_name);
    if (btree && !btree->save()) {
        std::cerr << "Failed to save B-tree index for column: " << column_name << std::endl;
        return 1;
    }

    std::cout << "B-tree index built and saved successfully for column: " << column_name << std::endl;
    return 0;
}
