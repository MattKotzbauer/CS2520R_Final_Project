#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

// B-tree order
const int ORDER = 3;

// B-tree node structure
struct BTreeNode {
    bool isLeaf;
    std::vector<int> keys;
    std::vector<std::shared_ptr<BTreeNode>> children;

    BTreeNode(bool leaf) : isLeaf(leaf) {}
};

// B-tree class
class BTree {
private:
    std::shared_ptr<BTreeNode> root;

    // Insert key into subtree rooted with node
    void insertNonFull(std::shared_ptr<BTreeNode> node, int key) {
        int i = node->keys.size() - 1;

        if (node->isLeaf) {
            // Insert the new key into the correct position
            node->keys.push_back(0); // Dummy key for expansion
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key;
        } else {
            // Find the child which is going to have the new key
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;
            // Check if the found child is full
            if (node->children[i]->keys.size() == ORDER - 1) {
                splitChild(node, i, node->children[i]);
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key);
        }
    }

    // Split the child y of node at index i
    void splitChild(std::shared_ptr<BTreeNode> parent, int i, std::shared_ptr<BTreeNode> y) {
        // Create a new node which will store (ORDER-1)/2 keys of y
        std::shared_ptr<BTreeNode> z = std::make_shared<BTreeNode>(y->isLeaf);
        int t = ORDER / 2;

        // Copy the last t-1 keys of y to z
        for (int j = 0; j < t - 1; j++) {
            z->keys.push_back(y->keys[j + t]);
        }

        // If y is not a leaf, copy the last t children of y to z
        if (!y->isLeaf) {
            for (int j = 0; j < t; j++) {
                z->children.push_back(y->children[j + t]);
            }
            // Remove the children copied to z
            y->children.resize(t);
        }

        // Reduce the number of keys in y
        y->keys.resize(t - 1);

        // Insert a new child to parent
        parent->children.insert(parent->children.begin() + i + 1, z);

        // Move the middle key of y up to the parent
        parent->keys.insert(parent->keys.begin() + i, y->keys[t - 1]);
    }

    // Serialize the B-tree to binary file using pre-order traversal
    void serializeNode(std::ofstream &out, std::shared_ptr<BTreeNode> node) const {
        // Write if the node is a leaf
        out.write(reinterpret_cast<const char*>(&node->isLeaf), sizeof(bool));

        // Write the number of keys
        int numKeys = node->keys.size();
        out.write(reinterpret_cast<const char*>(&numKeys), sizeof(int));

        // Write the keys
        for (int key : node->keys) {
            out.write(reinterpret_cast<const char*>(&key), sizeof(int));
        }

        // If not leaf, write children recursively
        if (!node->isLeaf) {
            for (auto &child : node->children) {
                serializeNode(out, child);
            }
        }
    }

public:
    BTree() : root(nullptr) {}

    // Insert a key into the B-tree
    void insert(int key) {
        if (!root) {
            // Allocate memory for root
            root = std::make_shared<BTreeNode>(true);
            root->keys.push_back(key);
        } else {
            // If root is full, then tree grows in height
            if (root->keys.size() == ORDER - 1) {
                std::shared_ptr<BTreeNode> s = std::make_shared<BTreeNode>(false);
                s->children.push_back(root);
                splitChild(s, 0, root);
                // New root has two children now. Decide which child to insert the key
                int i = 0;
                if (s->keys[0] < key) {
                    i++;
                }
                insertNonFull(s->children[i], key);
                root = s;
            } else {
                insertNonFull(root, key);
            }
        }
    }

    // Serialize the B-tree to a binary file
    void serialize(const std::string &filename) const {
        if (!root) {
            std::cerr << "B-tree is empty. Nothing to serialize.\n";
            return;
        }
        std::ofstream out(filename, std::ios::binary);
        if (!out) {
            std::cerr << "Failed to open file for writing: " << filename << "\n";
            return;
        }
        serializeNode(out, root);
        out.close();
        std::cout << "B-tree serialized to " << filename << "\n";
    }
};

// Function to trim whitespace from both ends of a string
std::string trim(const std::string &s) {
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

    BTree btree;
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
            int id = std::stoi(trim(item));
            btree.insert(id);
        }
    }

    file.close();

    // Serialize the B-tree to BTree.bin
    std::string binaryFile = "BTree.bin";
    btree.serialize(binaryFile);

    return 0;
}
