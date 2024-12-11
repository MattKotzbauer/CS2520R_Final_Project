#ifndef BTREE_H
#define BTREE_H

#include <cstdint>
#include <vector>
#include <variant>
#include <string>
#include <memory>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <cstring>

// Enumeration for key types
enum class KeyType {
    INTEGER = 0,
    DOUBLE = 1,
    STRING = 2
};

// Alias for KeyValue using std::variant to support multiple types
using KeyValue = std::variant<int, double, std::string>;

// Forward declaration
class BTreeNode;

// BTreeHeader struct to store metadata about the B-tree
struct BTreeHeader {
    char magic_number[4];    // e.g., "BTRE"
    uint32_t version;        // e.g., 1
    uint32_t order;          // B-tree order 't'
    uint8_t key_type;        // 0: int, 1: double, 2: string
    char column_name[50];    // Name of the indexed column
    uint64_t root_offset;    // Byte offset of the root node

    BTreeHeader() {
        magic_number[0] = 'B';
        magic_number[1] = 'T';
        magic_number[2] = 'R';
        magic_number[3] = 'E';
        version = 1;
        order = 3; // Default B-tree order
        key_type = 0;
        memset(column_name, 0, sizeof(column_name));
        root_offset = 0;
    }
};

// BTreeNode class representing each node in the B-tree
class BTreeNode {
public:
    BTreeNode(bool is_leaf = true, uint64_t offset = 0);
    
    // Insert a key and its associated data pointer into the node
    void insert(const KeyValue& key, uint64_t data_pointer, KeyType key_type, uint32_t order);

    // Search for a key and return associated data pointers
    std::vector<uint64_t> search(const KeyValue& key, KeyType key_type) const;

    // Serialize the node to a binary file at the given offset
    void serialize(std::fstream& file, KeyType key_type, uint32_t order) const;

    // Deserialize the node from a binary file at the given offset
    void deserialize(std::fstream& file, KeyType key_type, uint32_t order);

    // Getters and setters
    bool isLeaf() const { return is_leaf_; }
    uint32_t getNumKeys() const { return num_keys_; }
    const std::vector<KeyValue>& getKeys() const { return keys_; }
    const std::vector<uint64_t>& getChildren() const { return children_; }
    const std::vector<uint64_t>& getDataPointers() const { return data_pointers_; }
    void setOffset(uint64_t offset) { offset_ = offset; }
    uint64_t getOffset() const { return offset_; }

private:
    bool is_leaf_;                             // Indicates if the node is a leaf
    uint32_t num_keys_;                        // Current number of keys
    std::vector<KeyValue> keys_;               // Keys stored in the node
    std::vector<uint64_t> children_;           // Child node offsets
    std::vector<uint64_t> data_pointers_;      // Data pointers (only for leaf nodes)
    uint64_t offset_;                          // Byte offset of the node in the file
};

// BTree class managing the overall B-tree structure
class BTree {
public:
    // Constructor: Initializes the B-tree with the index file and key type
    BTree(const std::string& index_file, KeyType key_type, uint32_t order = 3);

    // Initialize a new B-tree (e.g., create root node)
    void initialize();

    // Insert a key and its associated data pointer into the B-tree
    void insert(const KeyValue& key, uint64_t data_pointer);

    // Search for a key and return associated data pointers
    std::vector<uint64_t> search(const KeyValue& key) const;

    // Serialize the entire B-tree to the index file
    bool save() const;

    // Deserialize the B-tree from the index file
    bool load();

    // Get the key type
    KeyType getKeyType() const { return key_type_; }

private:
    std::string index_file_;                // Path to the B-tree index file
    KeyType key_type_;                      // Type of keys stored
    uint32_t order_;                        // B-tree order 't'
    uint64_t root_offset_;                  // Byte offset of the root node
    std::unique_ptr<BTreeNode> root_;       // Root node

    // Helper methods
    void splitChild(BTreeNode& parent, int index, BTreeNode& child);
    void insertNonFull(BTreeNode& node, const KeyValue& key, uint64_t data_pointer);
    std::unique_ptr<BTreeNode> createNode(bool is_leaf, uint64_t offset);
};

#endif // BTREE_H
