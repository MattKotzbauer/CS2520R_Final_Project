// ElementSelect.cpp
#include "ElementSelect.h"

// Constructor
ElementSelect::ElementSelect(const std::vector<std::shared_ptr<Operand>>& operands, const std::string& table)
    : operands_(operands), table_(table), filter_(std::make_shared<CompositeElementFilter>()) {}

// Add a filter to the composite filter
void ElementSelect::addFilter(std::shared_ptr<ElementFilter> filter) {
    std::dynamic_pointer_cast<CompositeElementFilter>(filter_)->addFilter(filter);
}

// Getters
const std::vector<std::shared_ptr<Operand>>& ElementSelect::getOperands() const { 
    return operands_; 
}

const std::string& ElementSelect::getTable() const { 
    return table_; 
}

std::shared_ptr<ElementFilter> ElementSelect::getFilter() const { 
    return filter_; 
}
