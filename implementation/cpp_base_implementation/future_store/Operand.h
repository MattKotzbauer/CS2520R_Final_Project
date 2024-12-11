#ifndef OPERAND_H
#define OPERAND_H

#include <string>
#include <variant>
#include <memory>
#include <vector>

// Forward declaration for Operand
struct Operand;

// Comparator and Operator Enumerations
enum class Comparator {
    EQUAL,
    NOT_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    IN
};

enum class OperatorType {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE
};

// Operand Structure
struct Operand {
    // An operand can be:
    // - A column ID (string)
    // - An integer value
    // - An expression (left Operand, Operator, right Operand)
    // - A list of operands
    using Expression = std::tuple<std::shared_ptr<Operand>, OperatorType, std::shared_ptr<Operand>>;
    using OperandList = std::vector<std::shared_ptr<Operand>>;

    std::variant<
        std::string,                  // Column ID
        int,                          // Static integer value
        Expression,                   // Expression
        OperandList                   // List of operands
    > value;

    // Constructors for different types of operands
    Operand(const std::string& column) : value(column) {}
    Operand(int val) : value(val) {}
    Operand(const std::shared_ptr<Operand>& left, OperatorType op, const std::shared_ptr<Operand>& right)
        : value(std::make_tuple(left, op, right)) {}
    Operand(const OperandList& list) : value(list) {}
};

#endif // OPERAND_H

