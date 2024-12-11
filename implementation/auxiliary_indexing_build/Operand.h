// Operand.h
#ifndef OPERAND_H
#define OPERAND_H

#include <string>
#include <memory>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include <algorithm>

// Enumeration for operator types
enum class OperatorType {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE
};

// Define OperandValue as a variant of int, double, bool, and string
using OperandValue = std::variant<int, double, bool, std::string>;

// Operand base class
class Operand {
public:
    virtual ~Operand() = default;
    virtual OperandValue evaluate(const std::unordered_map<std::string, std::string>& row) const = 0;
};

// Operand representing a column
class ColumnOperand : public Operand {
public:
    ColumnOperand(const std::string& column) : column_(column) {}
    OperandValue evaluate(const std::unordered_map<std::string, std::string>& row) const override;
    const std::string& getColumn() const { return column_; }
private:
    std::string column_;
};

// Operand representing an integer
class IntegerOperand : public Operand {
public:
    IntegerOperand(int value) : value_(value) {}
    OperandValue evaluate(const std::unordered_map<std::string, std::string>& row) const override;
private:
    int value_;
};

// Operand representing a boolean
class BooleanOperand : public Operand {
public:
    BooleanOperand(bool value) : value_(value) {}
    OperandValue evaluate(const std::unordered_map<std::string, std::string>& row) const override;
private:
    bool value_;
};

// Operand representing an expression (operand operator operand)
class ExpressionOperand : public Operand {
public:
    ExpressionOperand(std::shared_ptr<Operand> left, OperatorType op, std::shared_ptr<Operand> right)
        : left_(left), op_(op), right_(right) {}
    OperandValue evaluate(const std::unordered_map<std::string, std::string>& row) const override;
private:
    std::shared_ptr<Operand> left_;
    OperatorType op_;
    std::shared_ptr<Operand> right_;
};

#endif // OPERAND_H

