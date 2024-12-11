#ifndef GROUPOPERAND_H
#define GROUPOPERAND_H

#include <string>
#include <variant>
#include <memory>
#include <vector>
#include "Operand.h"

// Forward declaration for GroupOperand
struct GroupOperand;

// Group Operator Enumeration
enum class GroupOperator {
    MAX,
    MIN,
    AVG
};

// GroupOperand Structure
struct GroupOperand {
    // A group operand can be:
    // - A group operation (GroupOperator, column ID)
    // - An integer value
    // - An expression (left GroupOperand, Operator, right GroupOperand)
    // - A list of group operands
    using Expression = std::tuple<std::shared_ptr<GroupOperand>, OperatorType, std::shared_ptr<GroupOperand>>;
    using GroupOperandList = std::vector<std::shared_ptr<GroupOperand>>;

    std::variant<
        std::pair<GroupOperator, std::string>, // (GroupOperator, column ID)
        int,                                   // Static integer value
        Expression,                            // Expression
        GroupOperandList                       // List of group operands
    > value;

    // Constructors for different types of group operands
    GroupOperand(GroupOperator op, const std::string& column)
        : value(std::make_pair(op, column)) {}
    GroupOperand(int val) : value(val) {}
    GroupOperand(const std::shared_ptr<GroupOperand>& left, OperatorType op, const std::shared_ptr<GroupOperand>& right)
        : value(std::make_tuple(left, op, right)) {}
    GroupOperand(const GroupOperandList& list) : value(list) {}
};

#endif // GROUPOPERAND_H

