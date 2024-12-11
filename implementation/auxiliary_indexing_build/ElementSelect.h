// ElementSelect.h
#ifndef ELEMENTSELECT_H
#define ELEMENTSELECT_H

#include "Operand.h"
#include "ElementFilter.h"
#include <memory>
#include <vector>
#include <string>

// ElementSelect class
class ElementSelect {
public:
    // Constructor with operands and table name
    ElementSelect(const std::vector<std::shared_ptr<Operand>>& operands, const std::string& table)
        : operands_(operands), table_(table), filter_(std::make_shared<CompositeElementFilter>()) {}
    
    // Add filter
    void addFilter(std::shared_ptr<ElementFilter> filter) {
        std::dynamic_pointer_cast<CompositeElementFilter>(filter_)->addFilter(filter);
    }
    
    // Getters
    const std::vector<std::shared_ptr<Operand>>& getOperands() const { return operands_; }
    const std::string& getTable() const { return table_; }
    std::shared_ptr<ElementFilter> getFilter() const { return filter_; }
    
private:
    std::vector<std::shared_ptr<Operand>> operands_;
    std::string table_;
    std::shared_ptr<ElementFilter> filter_;
};

#endif // ELEMENTSELECT_H

