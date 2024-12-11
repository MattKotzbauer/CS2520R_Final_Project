#include "BTree.h"
#include <cstring> // For memcpy
#include <iostream>
#include <fstream>

// Forward declaration of file_size
uint64_t file_size(const std::string& filename);

// Constructor for BTreeNode
BTreeNode::BTreeNode(bool is_leaf, uint64_t offset)
    : is_leaf_(is_leaf), num_keys_(0), offset_(offset) {}

// Insert a key and data pointer into the node
void BTreeNode::insert(const KeyValue& key, uint64_t data_pointer, KeyType key_type, uint32_t order) {
    // Simple insertion for illustration; handle node splits as needed
    if (num_keys_ < 2 * order - 1) {
        // Insert key in sorted order
        auto it = std::upper_bound(keys_.begin(), keys_.end(), key, [&](const KeyValue& a, const KeyValue& b) -> bool {
            if (a.index() != b.index()) {
                throw std::runtime_error("Mismatched key types during insertion.");
            }
            switch (key_type) {
                case KeyType::INTEGER:
                    return std::get<int>(a) < std::get<int>(b);
                case KeyType::DOUBLE:
                    return std::get<double>(a) < std::get<double>(b);
                case KeyType::STRING:
                    return std::get<std::string>(a) < std::get<std::string>(b);
                default:
                    throw std::runtime_error("Unsupported KeyType.");
            }
        });
        size_t index = it - keys_.begin();
        keys_.insert(it, key);
        if (is_leaf_) {
            data_pointers_.insert(data_pointers_.begin() + index, data_pointer);
        } else {
            if (index == num_keys_) {
                children_.insert(children_.begin() + index + 1, data_pointer); // Insert at index + 1 if at the end
            } else {
                children_.insert(children_.begin() + index, data_pointer); // Insert at index otherwise
            }
        }
        num_keys_++;
    } else {
        // Handle node splitting
        throw std::runtime_error("Node splitting not implemented yet.");
    }
}

// Search for a key and return associated data pointers
std::vector<uint64_t> BTreeNode::search(const KeyValue& key, KeyType key_type) const {
    std::vector<uint64_t> results;
    size_t i = 0;
    // Find the first key >= key
    while (i < num_keys_) {
        bool less = false;
        switch (key_type) {
            case KeyType::INTEGER:
                less = std::get<int>(keys_[i]) < std::get<int>(key);
                break;
            case KeyType::DOUBLE:
                less = std::get<double>(keys_[i]) < std::get<double>(key);
                break;
            case KeyType::STRING:
                less = std::get<std::string>(keys_[i]) < std::get<std::string>(key);
                break;
            default:
                throw std::runtime_error("Unsupported KeyType.");
        }
        if (!less) {
            break;
        }
        i++;
    }

    // If the found key matches, collect data pointers
    if (i < num_keys_) {
        bool match = false;
        switch (key_type) {
            case KeyType::INTEGER:
                match = (std::get<int>(keys_[i]) == std::get<int>(key));
                break;
            case KeyType::DOUBLE:
                match = (std::get<double>(keys_[i]) == std::get<double>(key));
                break;
            case KeyType::STRING:
                match = (std::get<std::string>(keys_[i]) == std::get<std::string>(key));
                break;
            default:
                throw std::runtime_error("Unsupported KeyType.");
        }
        if (match) {
            if (is_leaf_) {
                results.push_back(data_pointers_[i]);
            } else {
                // Internal node: recurse into child
                // For simplicity, assuming data_pointers_ holds child offsets
                // Implement recursive search in BTree class
                // Placeholder:
                // BTreeNode child = ...; // Deserialize child node
                // auto child_results = child.search(key, key_type);
                // results.insert(results.end(), child_results.begin(), child_results.end());
            }
        }
    }

    return results;
}

// Serialize the node to a binary file
void BTreeNode::serialize(std::fstream& file, KeyType key_type, uint32_t order) const {
    // Move to the node's offset
    file.seekp(offset_, std::ios::beg);

    // Serialize is_leaf_
    file.write(reinterpret_cast<const char*>(&is_leaf_), sizeof(is_leaf_));

    // Serialize num_keys_
    file.write(reinterpret_cast<const char*>(&num_keys_), sizeof(num_keys_));

    // Serialize keys_
    for (size_t i = 0; i < 2 * order - 1; ++i) {
        if (i < keys_.size()) {
            switch (key_type) {
                case KeyType::INTEGER: {
                    int val = std::get<int>(keys_[i]);
                    file.write(reinterpret_cast<const char*>(&val), sizeof(int));
                    break;
                }
                case KeyType::DOUBLE: {
                    double val = std::get<double>(keys_[i]);
                    file.write(reinterpret_cast<const char*>(&val), sizeof(double));
                    break;
                }
                case KeyType::STRING: {
                    std::string str = std::get<std::string>(keys_[i]);
                    uint32_t len = str.size();
                    file.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
                    file.write(str.c_str(), len);
                    // Pad remaining space if necessary
                    uint32_t padding = 100 - len; // Example fixed length
                    std::string pad(padding, '\0');
                    file.write(pad.c_str(), padding);
                    break;
                }
                default:
                    throw std::runtime_error("Unsupported KeyType during serialization.");
            }
        } else {
            // Write empty/default keys
            if (key_type == KeyType::STRING) {
                uint32_t len = 0;
                file.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
                std::string pad(100, '\0');
                file.write(pad.c_str(), 100);
            } else {
                // For int and double, write zero
                if (key_type == KeyType::INTEGER) {
                    int val = 0;
                    file.write(reinterpret_cast<const char*>(&val), sizeof(int));
                } else if (key_type == KeyType::DOUBLE) {
                    double val = 0.0;
                    file.write(reinterpret_cast<const char*>(&val), sizeof(double));
                }
            }
        }
    }

    // Serialize children_offsets_ or data_pointers_
    if (is_leaf_) {
        for (size_t i = 0; i < 2 * order - 1; ++i) {
            if (i < data_pointers_.size()) {
                file.write(reinterpret_cast<const char*>(&data_pointers_[i]), sizeof(uint64_t));
            } else {
                uint64_t zero = 0;
                file.write(reinterpret_cast<const char*>(&zero), sizeof(uint64_t));
            }
        }
    } else {
        for (size_t i = 0; i < 2 * order; ++i) {
            if (i < children_.size()) {
                file.write(reinterpret_cast<const char*>(&children_[i]), sizeof(uint64_t));
            } else {
                uint64_t zero = 0;
                file.write(reinterpret_cast<const char*>(&zero), sizeof(uint64_t));
            }
        }
    }
}

// Deserialize the node from a binary file
void BTreeNode::deserialize(std::fstream& file, KeyType key_type, uint32_t order) {
    // Move to the node's offset
    file.seekg(offset_, std::ios::beg);

    // Deserialize is_leaf_
    file.read(reinterpret_cast<char*>(&is_leaf_), sizeof(is_leaf_));

    // Deserialize num_keys_
    file.read(reinterpret_cast<char*>(&num_keys_), sizeof(num_keys_));

    // Deserialize keys_
    keys_.clear();
    for (size_t i = 0; i < 2 * order - 1; ++i) {
        if (i < num_keys_) {
            switch (key_type) {
                case KeyType::INTEGER: {
                    int val;
                    file.read(reinterpret_cast<char*>(&val), sizeof(int));
                    keys_.emplace_back(val);
                    break;
                }
                case KeyType::DOUBLE: {
                    double val;
                    file.read(reinterpret_cast<char*>(&val), sizeof(double));
                    keys_.emplace_back(val);
                    break;
                }
                case KeyType::STRING: {
                    uint32_t len;
                    file.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
                    std::string str(len, '\0');
                    file.read(&str[0], len);
                    keys_.emplace_back(str);
                    // Skip padding
                    file.seekg(100 - len, std::ios::cur);
                    break;
                }
                default:
                    throw std::runtime_error("Unsupported KeyType during deserialization.");
            }
        } else {
            // Skip over empty keys
            if (key_type == KeyType::STRING) {
                uint32_t len;
                file.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
                file.seekg(100 - len, std::ios::cur);
            } else {
                if (key_type == KeyType::INTEGER) {
                    int val;
                    file.read(reinterpret_cast<char*>(&val), sizeof(int));
                } else if (key_type == KeyType::DOUBLE) {
                    double val;
                    file.read(reinterpret_cast<char*>(&val), sizeof(double));
                }
            }
        }
    }

    // Deserialize children_offsets_ or data_pointers_
    children_.clear();
    data_pointers_.clear();
    if (is_leaf_) {
        for (size_t i = 0; i < 2 * order - 1; ++i) {
            uint64_t ptr;
            file.read(reinterpret_cast<char*>(&ptr), sizeof(uint64_t));
            if (i < num_keys_) {
                data_pointers_.push_back(ptr);
            }
        }
    } else {
        for (size_t i = 0; i < 2 * order; ++i) {
            uint64_t ptr;
            file.read(reinterpret_cast<char*>(&ptr), sizeof(uint64_t));
            if (i <= num_keys_) { // Number of children is num_keys_ + 1
                children_.push_back(ptr);
            }
        }
    }
}

// Constructor for BTree
BTree::BTree(const std::string& index_file, KeyType key_type, uint32_t order)
    : index_file_(index_file), key_type_(key_type), order_(order), root_offset_(0), root_(nullptr) {}

// Initialize a new B-tree by creating a root node
void BTree::initialize() {
    // Create a new root node
    root_ = std::make_unique<BTreeNode>(true, sizeof(BTreeHeader)); // Root starts after header
    root_->setOffset(sizeof(BTreeHeader));
    root_offset_ = root_->getOffset();

    // Open the file and write the header
    std::fstream file(index_file_, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create index file: " + index_file_);
    }

    // Create and write header
    BTreeHeader header;
    header.order = order_;
    header.key_type = static_cast<uint8_t>(key_type_);
    strncpy(header.column_name, index_file_.c_str(), sizeof(header.column_name) - 1); // Assuming filename includes column name
    header.root_offset = root_offset_;

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Serialize root node
    root_->serialize(file, key_type_, order_);

    file.close();
}

// Insert a key and data pointer into the B-tree
void BTree::insert(const KeyValue& key, uint64_t data_pointer) {
    if (!root_) {
        throw std::runtime_error("B-tree not initialized.");
    }

    if (root_->getNumKeys() == 2 * order_ - 1) {
        // Root is full; need to split
        auto new_root = std::make_unique<BTreeNode>(false, file_size(index_file_)); // Next available offset
        new_root->children_.push_back(root_->getOffset());

        // Split the old root and move one key to the new root
        root_->splitChild(0, *root_, key_type_, order_);

        // Determine which child to insert into
        int i = 0;
        // ...code to compare key with new_root->keys_[0]...
        // Example for INTEGER key_type; generalize as needed
        if (key.index() == 0 && std::get<int>(key) > std::get<int>(new_root->keys_[0])) {
            i++;
        }

        // Load the appropriate child
        BTreeNode child = BTreeNode();
        child.setOffset(new_root->children_[i]);
        child.deserialize(file, key_type_, order_);

        // Insert the key into the non-full child
        child.insertNonFull(key, data_pointer, key_type_, order_);

        // Update parent with the new child
        new_root->children_.insert(new_root->children_.begin() + i + 1, child.getOffset());
        new_root->keys_.insert(new_root->keys_.begin() + i, new_root->keys_[0]); // Example adjustment

        // Update root
        root_ = std::move(new_root);
        root_offset_ = root_->getOffset();
    } else {
        insertNonFull(*root_, key, data_pointer);
    }
}

// Helper method to insert a key into a node that is not full
void BTree::insertNonFull(BTreeNode& node, const KeyValue& key, uint64_t data_pointer) {
    int i = node.getNumKeys() - 1;

    if (node.isLeaf()) {
        // Insert the key into the leaf node in sorted order
        node.insert(key, data_pointer, key_type_, order_);
        // Serialize the node
        std::fstream file(index_file_, std::ios::binary | std::ios::in | std::ios::out);
        node.serialize(file, key_type_, order_);
        file.close();
    } else {
        // Find the child which is going to have the new key
        while (i >= 0) {
            bool less = false;
            switch (key_type_) {
                case KeyType::INTEGER:
                    less = std::get<int>(key) > std::get<int>(node.getKeys()[i]);
                    break;
                case KeyType::DOUBLE:
                    less = std::get<double>(key) > std::get<double>(node.getKeys()[i]);
                    break;
                case KeyType::STRING:
                    less = std::get<std::string>(key) > std::get<std::string>(node.getKeys()[i]);
                    break;
                default:
                    throw std::runtime_error("Unsupported KeyType during insertion.");
            }
            if (less) {
                break;
            }
            i--;
        }
        i++;

        // Deserialize the child node
        BTreeNode child;
        child.deserialize(*new std::fstream(index_file_, std::ios::binary | std::ios::in | std::ios::out),
                          key_type_, order_);

        // If the child is full, split it
        if (child.getNumKeys() == 2 * order_ - 1) {
            splitChild(node, i, child);
            // Decide which of the two children is now the appropriate child
            if (std::get<int>(key) > std::get<int>(node.getKeys()[i])) { // Example for int; generalize as needed
                i++;
            }
        }

        // Insert into the appropriate child
        // For simplicity, assume child is not full after potential split
        // Implement recursive insertion as needed
    }
}

// Split the child node at index
void BTree::splitChild(BTreeNode& parent, int index, BTreeNode& child) {
    // Create a new node to store (order_ - 1) keys of child
    auto new_child = std::make_unique<BTreeNode>(child.isLeaf(), file_size(index_file_));
    new_child->deserialize(*new std::fstream(index_file_, std::ios::binary | std::ios::in | std::ios::out),
                           key_type_, order_);

    // Transfer (order_ - 1) keys from child to new_child
    for (int j = 0; j < order_ - 1; j++) {
        new_child->insert(child.getKeys()[order_ + j], child.getDataPointers()[order_ + j],
                         key_type_, order_);
    }

    // If child is not leaf, transfer the last order_ children to new_child
    if (!child.isLeaf()) {
        for (int j = 0; j < order_; j++) {
            new_child->getChildren().push_back(child.getChildren()[order_ + j]);
        }
        // Remove transferred children from child
        const_cast<std::vector<uint64_t>&>(child.getChildren()).resize(order_);
    }

    // Reduce the number of keys in child
    child.keys_.resize(order_ - 1);
    child.data_pointers_.resize(order_ - 1);
    child.num_keys_ = order_ - 1;

    // Insert a new key into parent
    const_cast<std::vector<KeyValue>&>(parent.getKeys()).insert(parent.getKeys().begin() + index, child.getKeys()[order_ - 1]);
    const_cast<std::vector<uint64_t>&>(parent.getChildren()).insert(parent.getChildren().begin() + index + 1, new_child->getOffset());
    parent.setNumKeys(parent.getNumKeys() + 1);

    // Serialize the updated child, new_child, and parent nodes
    std::fstream file(index_file_, std::ios::binary | std::ios::in | std::ios::out);
    child.serialize(file, key_type_, order_);
    new_child->serialize(file, key_type_, order_);
    parent.serialize(file, key_type_, order_);
    file.close();
}

// Search for a key in the B-tree and return associated data pointers
std::vector<uint64_t> BTree::search(const KeyValue& key) const {
    if (!root_) {
        throw std::runtime_error("B-tree is empty.");
    }

    // Start searching from the root node
    // Implement recursive search or iterative search as needed
    // For simplicity, returning an empty vector
    return {}; // Placeholder
}

// Serialize the entire B-tree to the index file
bool BTree::save() const {
    std::fstream file(index_file_, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Failed to open index file for writing: " << index_file_ << std::endl;
        return false;
    }

    // Write header
    BTreeHeader header;
    header.order = order_;
    header.key_type = static_cast<uint8_t>(key_type_);
    strncpy(header.column_name, index_file_.c_str(), sizeof(header.column_name) - 1);
    header.root_offset = root_offset_;

    file.seekp(0, std::ios::beg);
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // Serialize root node
    root_->serialize(file, key_type_, order_);

    file.close();
    return true;
}

// Deserialize the B-tree from the index file
bool BTree::load() {
    std::fstream file(index_file_, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Failed to open index file for reading: " << index_file_ << std::endl;
        return false;
    }

    // Read header
    BTreeHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    // Validate magic number
    if (header.magic_number[0] != 'B' || header.magic_number[1] != 'T' ||
        header.magic_number[2] != 'R' || header.magic_number[3] != 'E') {
        std::cerr << "Invalid B-tree index file: " << index_file_ << std::endl;
        file.close();
        return false;
    }

    // Validate key type
    key_type_ = static_cast<KeyType>(header.key_type);
    order_ = header.order;
    root_offset_ = header.root_offset;

    // Deserialize root node
    root_ = std::make_unique<BTreeNode>();
    root_->deserialize(file, key_type_, order_);

    file.close();
    return true;
}

// Create a new node and assign it a byte offset in the file
std::unique_ptr<BTreeNode> BTree::createNode(bool is_leaf, uint64_t offset) {
    auto node = std::make_unique<BTreeNode>(is_leaf, offset);
    // Initialize node as needed
    return node;
}

// Placeholder for file size calculation
uint64_t file_size(const std::string& filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    if (!in.is_open()) {
        return 0;
    }
    return in.tellg();
}
