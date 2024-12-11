#ifndef QUERY_H
#define QUERY_H

#include "Select.h"
#include "CSVLoader.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <algorithm>
#include <functional>

// Helper function to evaluate an operand for a given row
class QueryExecutor {
public:
    QueryExecutor(const CSVLoader& loader) : loader_(loader) {}

    // Evaluate Operand
    double evaluateOperand(const std::shared_ptr<Operand>& operand, const std::unordered_map<std::string, std::string>& row) const {
        if (std::holds_alternative<std::string>(operand->value)) {
            // Assume the column contains numeric values
            const std::string& col = std::get<std::string>(operand->value);
            auto it = row.find(col);
            if (it != row.end()) {
                return std::stod(it->second);
            } else {
                throw std::runtime_error("Column not found: " + col);
            }
        }
        else if (std::holds_alternative<int>(operand->value)) {
            return static_cast<double>(std::get<int>(operand->value));
        }
        else if (std::holds_alternative<Operand::Expression>(operand->value)) {
            auto expr = std::get<Operand::Expression>(operand->value);
            double left = evaluateOperand(std::get<0>(expr), row);
            OperatorType op = std::get<1>(expr);
            double right = evaluateOperand(std::get<2>(expr), row);
            switch (op) {
                case OperatorType::ADD: return left + right;
                case OperatorType::SUBTRACT: return left - right;
                case OperatorType::MULTIPLY: return left * right;
                case OperatorType::DIVIDE:
                    if (right == 0) throw std::runtime_error("Division by zero");
                    return left / right;
                default: throw std::runtime_error("Unknown operator");
            }
        }
        else {
            throw std::runtime_error("Unsupported operand type");
        }
    }

    // Evaluate GroupOperand (similar to Operand, but tailored for group operations)
    double evaluateGroupOperand(const std::shared_ptr<GroupOperand>& group_operand, const std::unordered_map<std::string, std::string>& row) const {
        if (std::holds_alternative<std::pair<GroupOperator, std::string>>(group_operand->value)) {
            // Group operations like MAX, MIN, AVG are handled separately
            // Here, return the column value for individual rows
            const auto& pair = std::get<std::pair<GroupOperator, std::string>>(group_operand->value);
            const std::string& col = pair.second;
            auto it = row.find(col);
            if (it != row.end()) {
                return std::stod(it->second);
            } else {
                throw std::runtime_error("Column not found: " + col);
            }
        }
        else if (std::holds_alternative<int>(group_operand->value)) {
            return static_cast<double>(std::get<int>(group_operand->value));
        }
        else if (std::holds_alternative<GroupOperand::Expression>(group_operand->value)) {
            auto expr = std::get<GroupOperand::Expression>(group_operand->value);
            double left = evaluateGroupOperand(std::get<0>(expr), row);
            OperatorType op = std::get<1>(expr);
            double right = evaluateGroupOperand(std::get<2>(expr), row);
            switch (op) {
                case OperatorType::ADD: return left + right;
                case OperatorType::SUBTRACT: return left - right;
                case OperatorType::MULTIPLY: return left * right;
                case OperatorType::DIVIDE:
                    if (right == 0) throw std::runtime_error("Division by zero");
                    return left / right;
                default: throw std::runtime_error("Unknown operator");
            }
        }
        else {
            throw std::runtime_error("Unsupported group operand type");
        }
    }

    // Apply WHERE filter
    bool applyWhereFilter(const std::shared_ptr<WhereFilter>& filter, const std::unordered_map<std::string, std::string>& row) const {
        double left = evaluateOperand(filter->left, row);
        double right = evaluateOperand(filter->right, row);
        switch (filter->comparator) {
            case Comparator::EQUAL: return left == right;
            case Comparator::NOT_EQUAL: return left != right;
            case Comparator::GREATER: return left > right;
            case Comparator::LESS: return left < right;
            case Comparator::GREATER_EQUAL: return left >= right;
            case Comparator::LESS_EQUAL: return left <= right;
            case Comparator::IN:
                // For simplicity, assume 'IN' is not implemented
                throw std::runtime_error("'IN' comparator not implemented");
            default: throw std::runtime_error("Unknown comparator");
        }
    }

    // Apply Distinct filter
    bool applyDistinctFilter(const std::shared_ptr<DistinctFilter>& filter, const std::unordered_map<std::string, std::string>& row, std::unordered_map<std::string, bool>& distinct_tracker) const {
        // Implement DISTINCT logic if needed
        // For simplicity, not implemented
        return true;
    }

    // Apply OrderBy filter
    bool applyOrderByFilter(const std::shared_ptr<OrderByFilter>& filter, const std::unordered_map<std::string, std::string>& row) const {
        // Implement OrderBy logic if needed
        // For simplicity, not implemented
        return true;
    }

    // Apply Limit filter
    bool applyLimitFilter(const std::shared_ptr<LimitFilter>& filter, int current_count) const {
        // Implement Limit logic if needed
        // For simplicity, not implemented
        return true;
    }

    // Execute ElementFilter
    bool applyElementFilters(const std::shared_ptr<ElementFilter>& filter, const std::unordered_map<std::string, std::string>& row) const {
        for (const auto& f : filter->filters) {
            if (auto where = std::dynamic_pointer_cast<WhereFilter>(f)) {
                if (!applyWhereFilter(where, row)) return false;
            }
            // Implement other filters like Distinct, OrderBy as needed
            // For simplicity, only WHERE is handled here
        }
        return true;
    }

    // Execute the query
    void execute(const ElementSelect& select) const {
        const auto& data = loader_.getData();
        const auto& headers = loader_.getHeaders();

        // Iterate over data and apply filters
        for (const auto& row : data) {
            if (applyElementFilters(select.filter, row)) {
                double result = evaluateOperand(select.operand, row);
                std::cout << result << std::endl;
            }
        }
    }

    // Execute GroupSelect
    void execute(const GroupSelect& select) const {
        const auto& data = loader_.getData();
        const auto& headers = loader_.getHeaders();

        // Grouping
        std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>> groups;
        for (const auto& row : data) {
            // Evaluate group_by_operand to determine the group key
            double group_key_val = evaluateOperand(select.group_by_operand, row);
            std::string group_key = std::to_string(group_key_val);
            groups[group_key].push_back(row);
        }

        // Apply HAVING and other group filters
        for (const auto& [key, rows] : groups) {
            // For simplicity, HAVING filters are not implemented
            // Implement HAVING filter if needed

            // Apply aggregations
            double aggregation_result = 0.0;
            if (auto group_op = std::dynamic_pointer_cast<GroupOperand>(select.group_operand)) {
                auto pair = std::get<std::pair<GroupOperator, std::string>>(group_op->value);
                const std::string& col = pair.second;
                switch (pair.first) {
                    case GroupOperator::MAX: {
                        double max_val = -std::numeric_limits<double>::infinity();
                        for (const auto& row : rows) {
                            double val = std::stod(row.at(col));
                            if (val > max_val) max_val = val;
                        }
                        aggregation_result = max_val;
                        break;
                    }
                    case GroupOperator::MIN: {
                        double min_val = std::numeric_limits<double>::infinity();
                        for (const auto& row : rows) {
                            double val = std::stod(row.at(col));
                            if (val < min_val) min_val = val;
                        }
                        aggregation_result = min_val;
                        break;
                    }
                    case GroupOperator::AVG: {
                        double sum = 0.0;
                        for (const auto& row : rows) {
                            sum += std::stod(row.at(col));
                        }
                        aggregation_result = sum / rows.size();
                        break;
                    }
                    default:
                        throw std::runtime_error("Unknown group operator");
                }
            }

            std::cout << "Group " << key << ": " << aggregation_result << std::endl;
        }
    }

private:
    const CSVLoader& loader_;
};

#endif // QUERY_H

