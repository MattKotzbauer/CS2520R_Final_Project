// BTreeNode.h
#ifndef BTREENODE_H
#define BTREENODE_H

#include <cstdint>
#include <vector>
#include <variant>
#include <string>

struct BTreeNode {
    bool is_leaf;                             // Leaf node indicator
    uint32_t num_keys;                        // Number of keys
    std::vector<std::variant<int, double, std::string>> keys; // Keys
    std::vector<uint64_t> children_offsets;    // Child node offsets (for internal nodes)
    std::vector<uint64_t> data_pointers;       // Data pointers (for leaf nodes)
};

#endif // BTREENODE_H
