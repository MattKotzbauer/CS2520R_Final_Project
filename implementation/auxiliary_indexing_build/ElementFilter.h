// ElementFilter.h
#ifndef ELEMENTFILTER_H
#define ELEMENTFILTER_H

#include "Operand.h"
#include "CSVLoader.h"
#include <memory>
#include <vector>
#include <unordered_set>


// Enumeration for comparators
enum class Comparator {
    EQUAL,
    NOT_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    IN
};

// Base class for filters
class ElementFilter {
public:
    virtual ~ElementFilter() = default;
    virtual bool apply(const std::unordered_map<std::string, std::string>& row) const = 0;
};

// Where filter
class WhereFilter : public ElementFilter {
public:
    WhereFilter(std::shared_ptr<Operand> left, Comparator comp, std::shared_ptr<Operand> right)
        : left_(left), comparator_(comp), right_(right) {}
    bool apply(const std::unordered_map<std::string, std::string>& row) const override;
private:
    std::shared_ptr<Operand> left_;
    Comparator comparator_;
    std::shared_ptr<Operand> right_;
};

// Distinct filter
class DistinctFilter : public ElementFilter {
public:
    DistinctFilter(const std::vector<std::shared_ptr<Operand>>& operands)
        : operands_(operands) {}
    bool apply(const std::unordered_map<std::string, std::string>& row) const override;
private:
    std::vector<std::shared_ptr<Operand>> operands_;
    mutable std::unordered_set<std::string> seen_;
};

// OrderBy filter
class OrderByFilter : public ElementFilter {
public:
    OrderByFilter(std::shared_ptr<Operand> operand, bool ascending = true)
        : operand_(operand), ascending_(ascending) {}
    bool apply(const std::unordered_map<std::string, std::string>& row) const override;
    // Implement ORDER BY logic as needed
private:
    std::shared_ptr<Operand> operand_;
    bool ascending_;
};

// Limit filter
class LimitFilter : public ElementFilter {
public:
    LimitFilter(int limit, int offset = 0)
        : limit_(limit), offset_(offset), count_(0) {}
    bool apply(const std::unordered_map<std::string, std::string>& row) const override;
private:
    int limit_;
    int offset_;
    mutable int count_;
};

// Composite filter (for combining multiple filters)
class CompositeElementFilter : public ElementFilter {
public:
    void addFilter(std::shared_ptr<ElementFilter> filter) {
        filters_.push_back(filter);
    }
    bool apply(const std::unordered_map<std::string, std::string>& row) const override;
private:
    std::vector<std::shared_ptr<ElementFilter>> filters_;
};

class WhereFilter : public ElementFilter {
public:
    WhereFilter(std::shared_ptr<Operand> left, Comparator comp, std::shared_ptr<Operand> right, CSVLoader& loader)
        : left_(left), comparator_(comp), right_(right), loader_(loader) {}

    // Modify apply to optionally use index
    bool apply(const std::unordered_map<std::string, std::string>& row) const override;

    // Retrieve matching row indices using B-tree
    std::unordered_set<size_t> getMatchingRows() const;

private:
    std::shared_ptr<Operand> left_;
    Comparator comparator_;
    std::shared_ptr<Operand> right_;
    CSVLoader& loader_;
};


#endif // ELEMENTFILTER_H

