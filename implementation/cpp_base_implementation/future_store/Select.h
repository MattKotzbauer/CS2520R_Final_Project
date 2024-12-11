#ifndef SELECT_H
#define SELECT_H

#include "Operand.h"
#include "GroupOperand.h"
#include "Filter.h"
#include <memory>
#include <string>
#include <vector>

// Element Select
struct ElementSelect {
    std::shared_ptr<Operand> operand;
    std::string table; // Assuming table refers to CSV file name
    std::shared_ptr<ElementFilter> filter;

    ElementSelect(const std::shared_ptr<Operand>& op, const std::string& tbl)
        : operand(op), table(tbl), filter(std::make_shared<ElementFilter>()) {}
};

// Group Select
struct GroupSelect {
    std::shared_ptr<GroupOperand> group_operand;
    std::string table;
    std::shared_ptr<Operand> group_by_operand;
    std::shared_ptr<ElementFilter> element_filter;
    std::shared_ptr<GroupFilter> group_filter;

    GroupSelect(const std::shared_ptr<GroupOperand>& g_op, const std::string& tbl, const std::shared_ptr<Operand>& group_by)
        : group_operand(g_op), table(tbl), group_by_operand(group_by),
          element_filter(std::make_shared<ElementFilter>()),
          group_filter(std::make_shared<GroupFilter>()) {}
};

#endif // SELECT_H

