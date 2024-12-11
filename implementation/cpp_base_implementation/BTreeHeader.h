// BTreeHeader.h
#ifndef BTREEHEADER_H
#define BTREEHEADER_H

#include <cstdint>
#include <string>

struct BTreeHeader {
    char magic_number[4];    // e.g., "BTRE"
    uint32_t version;        // e.g., 1
    uint32_t order;          // B-tree order 't'
    uint8_t key_type;        // 0: int, 1: double, 2: string
    char column_name[50];    // Name of the indexed column
    uint64_t root_offset;    // Byte offset of the root node
};

#endif // BTREEHEADER_H
