// ElementFilter.cpp
#include "ElementFilter.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>

// Implement WhereFilter::apply
bool WhereFilter::apply(const std::unordered_map<std::string, std::string>& row) const {
    OperandValue left_val = left_->evaluate(row);
    OperandValue right_val = right_->evaluate(row);
    
    // Handle comparison based on the type of left_val and right_val
    if (std::holds_alternative<int>(left_val) && std::holds_alternative<int>(right_val)) {
        int left = std::get<int>(left_val);
        int right = std::get<int>(right_val);
        
        switch (comparator_) {
            case Comparator::EQUAL:
                return left == right;
            case Comparator::NOT_EQUAL:
                return left != right;
            case Comparator::GREATER:
                return left > right;
            case Comparator::LESS:
                return left < right;
            case Comparator::GREATER_EQUAL:
                return left >= right;
            case Comparator::LESS_EQUAL:
                return left <= right;
            case Comparator::IN:
                // Implement 'IN' for integers
                if (std::holds_alternative<std::vector<int>>(right_val)) {
                    const auto& vec = std::get<std::vector<int>>(right_val);
                    return std::find(vec.begin(), vec.end(), left) != vec.end();
                }
                else {
                    throw std::runtime_error("'IN' comparator requires a vector<int> as the right operand for integers.");
                }
            default:
                throw std::runtime_error("Unknown comparator in WhereFilter.");
        }
    }
    else if (std::holds_alternative<double>(left_val) && std::holds_alternative<double>(right_val)) {
        double left = std::get<double>(left_val);
        double right = std::get<double>(right_val);
        
        switch (comparator_) {
            case Comparator::EQUAL:
                return left == right;
            case Comparator::NOT_EQUAL:
                return left != right;
            case Comparator::GREATER:
                return left > right;
            case Comparator::LESS:
                return left < right;
            case Comparator::GREATER_EQUAL:
                return left >= right;
            case Comparator::LESS_EQUAL:
                return left <= right;
            case Comparator::IN:
                // Implement 'IN' for doubles
                if (std::holds_alternative<std::vector<double>>(right_val)) {
                    const auto& vec = std::get<std::vector<double>>(right_val);
                    return std::find(vec.begin(), vec.end(), left) != vec.end();
                }
                else {
                    throw std::runtime_error("'IN' comparator requires a vector<double> as the right operand for doubles.");
                }
            default:
                throw std::runtime_error("Unknown comparator in WhereFilter.");
        }
    }
    else if (std::holds_alternative<bool>(left_val) && std::holds_alternative<bool>(right_val)) {
        bool left = std::get<bool>(left_val);
        bool right = std::get<bool>(right_val);
        
        switch (comparator_) {
            case Comparator::EQUAL:
                return left == right;
            case Comparator::NOT_EQUAL:
                return left != right;
            default:
                throw std::runtime_error("Unsupported comparator for bool operands.");
        }
    }
    else if (std::holds_alternative<std::string>(left_val) && std::holds_alternative<std::string>(right_val)) {
        const std::string& left = std::get<std::string>(left_val);
        const std::string& right = std::get<std::string>(right_val);
        
        switch (comparator_) {
            case Comparator::EQUAL:
                return left == right;
            case Comparator::NOT_EQUAL:
                return left != right;
            case Comparator::IN:
                // Implement 'IN' for strings
                {
                    std::vector<std::string> values;
                    std::stringstream ss(right);
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        values.push_back(item);
                    }
                    return std::find(values.begin(), values.end(), left) != values.end();
                }
            default:
                throw std::runtime_error("Unsupported comparator for string operands.");
        }
    }
    else {
        throw std::runtime_error("Type mismatch between operands in WhereFilter.");
    }
}

// Implement DistinctFilter::apply
bool DistinctFilter::apply(const std::unordered_map<std::string, std::string>& row) const {
    std::stringstream ss;
    for (const auto& operand : operands_) {
        try {
            OperandValue value = operand->evaluate(row);
            if (std::holds_alternative<int>(value)) {
                ss << std::get<int>(value) << "|";
            }
            else if (std::holds_alternative<double>(value)) {
                ss << std::get<double>(value) << "|";
            }
            else if (std::holds_alternative<bool>(value)) {
                ss << (std::get<bool>(value) ? "true" : "false") << "|";
            }
            else if (std::holds_alternative<std::string>(value)) {
                ss << std::get<std::string>(value) << "|";
            }
        }
        catch (...) {
            ss << "N/A|";
        }
    }
    std::string key = ss.str();
    if (seen_.find(key) != seen_.end()) {
        return false; // Duplicate row
    }
    seen_.insert(key);
    return true; // Unique row
}

// Implement OrderByFilter::apply
bool OrderByFilter::apply(const std::unordered_map<std::string, std::string>& row) const {
    // ORDER BY is typically handled after filtering.
    // Here, we'll skip implementation or handle it externally.
    // Alternatively, you can collect rows and sort them in the QueryExecutor.
    return true;
}

// Implement LimitFilter::apply
bool LimitFilter::apply(const std::unordered_map<std::string, std::string>& row) const {
    if (count_ < offset_) {
        count_++;
        return false;
    }
    if (count_ < (offset_ + limit_)) {
        count_++;
        return true;
    }
    return false;
}

// Implement CompositeElementFilter::apply
bool CompositeElementFilter::apply(const std::unordered_map<std::string, std::string>& row) const {
    for (const auto& filter : filters_) {
        if (!filter->apply(row)) {
            return false;
        }
    }
    return true;
}
