// main.cpp

#include "CSVLoader.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    // Check if the CSV filename is provided as a command-line argument
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <csv_filename>" << endl;
        return 1;
    }

    // Get the CSV file name from the first command-line argument
    string filename = argv[1];

    // Create an instance of CSVLoader with the provided filename
    CSVLoader loader(filename);

    // Load the CSV data
    if (!loader.load()) {
        cerr << "Error: Failed to load the CSV file." << endl;
        return 1;
    }

    // Retrieve headers and data
    const vector<string>& headers = loader.getHeaders();
    const vector<unordered_map<string, string>>& data = loader.getData();

    // Display headers
    for (const auto& header : headers) {
        cout << header << "\t";
    }
    cout << endl;

    // Display a separator line
    for (size_t i = 0; i < headers.size(); ++i) {
        cout << "----\t";
    }
    cout << endl;

    // Display each row of data
    for (const auto& row : data) {
        for (const auto& header : headers) {
            // Access the value using the header name
            // If the key doesn't exist, display "N/A"
            auto it = row.find(header);
            if (it != row.end()) {
                cout << it->second << "\t";
            } else {
                cout << "N/A\t";
            }
        }
        cout << endl;
    }

    return 0;
}
