// QueryExecutor.cpp
#include "QueryExecutor.h"
#include <iomanip> // For formatting output

void QueryExecutor::execute(const ElementSelect& select) const {
    const auto& data = loader_.getData();
    const auto& headers = loader_.getHeaders();
    const auto& operands = select.getOperands();
    const auto& filter = select.getFilter();

    // Display headers
    for (const auto& operand : operands) {
        // For simplicity, assume operand is ColumnOperand or ExpressionOperand
        // Display "Expr" for expressions
        std::shared_ptr<ColumnOperand> colOp = std::dynamic_pointer_cast<ColumnOperand>(operand);
        if (colOp) {
            std::cout << colOp->getColumn() << "\t";
        }
        else {
            std::cout << "Expr\t";
        }
    }
    std::cout << std::endl;

    // Display a separator line
    for (size_t i = 0; i < operands.size(); ++i) {
        std::cout << "----\t";
    }
    std::cout << std::endl;

    // Iterate over data and apply filters
    for (size_t row_num = 0; row_num < data.size(); ++row_num) {
        const auto& row = data[row_num];
        try {
            if (filter->apply(row)) {
                for (const auto& operand : operands) {
                    OperandValue value = operand->evaluate(row);
                    if (std::holds_alternative<int>(value)) {
                        std::cout << std::get<int>(value) << "\t";
                    }
                    else if (std::holds_alternative<double>(value)) {
                        double num = std::get<double>(value);
                        // Display as integer if no fractional part
                        if (num == static_cast<int>(num)) {
                            std::cout << static_cast<int>(num) << "\t";
                        }
                        else {
                            std::cout << num << "\t";
                        }
                    }
                    else if (std::holds_alternative<bool>(value)) {
                        std::cout << (std::get<bool>(value) ? "true" : "false") << "\t";
                    }
                    else if (std::holds_alternative<std::string>(value)) {
                        std::cout << std::get<std::string>(value) << "\t";
                    }
                }
                std::cout << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error processing row " << row_num + 1 << ": " << e.what() << std::endl;
            // Otherwise, continue processing other rows
        }
    }
}

