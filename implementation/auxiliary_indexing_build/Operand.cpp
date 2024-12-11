// Operand.cpp
#include "Operand.h"
#include <iostream>
#include <sstream>
#include <cctype>

// Helper function to convert string to lowercase
static std::string to_lower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}

// Implement ColumnOperand::evaluate
OperandValue ColumnOperand::evaluate(const std::unordered_map<std::string, std::string>& row) const {
    auto it = row.find(column_);
    if (it != row.end()) {
        const std::string& value_str = it->second;
        std::cout << "Evaluating ColumnOperand: " << column_ << " = " << value_str << std::endl;

        // Attempt to parse as int
        try {
            size_t pos;
            int int_val = std::stoi(value_str, &pos);
            if (pos == value_str.length()) {
                std::cout << "Parsed as int: " << int_val << std::endl;
                return int_val;
            }
        } catch (...) { /* Ignore and try next type */ }

        // Attempt to parse as double
        try {
            size_t pos;
            double double_val = std::stod(value_str, &pos);
            if (pos == value_str.length()) {
                std::cout << "Parsed as double: " << double_val << std::endl;
                return double_val;
            }
        } catch (...) { /* Ignore and try next type */ }

        // Attempt to parse as bool
        std::string lower_val = to_lower(value_str);
        if (lower_val == "true" || lower_val == "1") {
            std::cout << "Parsed as bool: true" << std::endl;
            return true;
        }
        if (lower_val == "false" || lower_val == "0") {
            std::cout << "Parsed as bool: false" << std::endl;
            return false;
        }

        // If all parsing attempts fail, return as string
        std::cout << "Returning as string: " << value_str << std::endl;
        return value_str;
    }
    else {
        throw std::runtime_error("Column '" + column_ + "' not found.");
    }
}

// Implement IntegerOperand::evaluate
OperandValue IntegerOperand::evaluate(const std::unordered_map<std::string, std::string>& row) const {
    return value_;
}

// Implement BooleanOperand::evaluate
OperandValue BooleanOperand::evaluate(const std::unordered_map<std::string, std::string>& row) const {
    return value_;
}

// Implement ExpressionOperand::evaluate
OperandValue ExpressionOperand::evaluate(const std::unordered_map<std::string, std::string>& row) const {
    OperandValue left_val = left_->evaluate(row);
    OperandValue right_val = right_->evaluate(row);
    
    // Ensure both operands are numeric (int or double)
    if ((std::holds_alternative<int>(left_val) || std::holds_alternative<double>(left_val)) &&
        (std::holds_alternative<int>(right_val) || std::holds_alternative<double>(right_val))) {
        
        double left = std::holds_alternative<int>(left_val) ? static_cast<double>(std::get<int>(left_val)) : std::get<double>(left_val);
        double right = std::holds_alternative<int>(right_val) ? static_cast<double>(std::get<int>(right_val)) : std::get<double>(right_val);
        
        switch (op_) {
            case OperatorType::ADD:
                return left + right;
            case OperatorType::SUBTRACT:
                return left - right;
            case OperatorType::MULTIPLY:
                return left * right;
            case OperatorType::DIVIDE:
                if (right == 0) throw std::runtime_error("Division by zero in expression.");
                return left / right;
            default:
                throw std::runtime_error("Unknown operator in expression.");
        }
    }
    else {
        throw std::runtime_error("Operands must be numeric (int or double) for expressions.");
    }
}

