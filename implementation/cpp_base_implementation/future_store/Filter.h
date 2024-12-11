#ifndef FILTER_H
#define FILTER_H

#include "Operand.h"
#include "GroupOperand.h"
#include <memory>
#include <vector>
#include <functional>

// Base Filter Class
struct Filter {
    virtual ~Filter() = default;
};

// Where Filter
struct WhereFilter : public Filter {
    std::shared_ptr<Operand> left;
    Comparator comparator;
    std::shared_ptr<Operand> right;

    WhereFilter(const std::shared_ptr<Operand>& l, Comparator comp, const std::shared_ptr<Operand>& r)
        : left(l), comparator(comp), right(r) {}
};

// Distinct Filter
struct DistinctFilter : public Filter {
    // No additional members; signifies DISTINCT operation
};

// Order By Filter
struct OrderByFilter : public Filter {
    std::shared_ptr<Operand> operand;
    bool ascending;

    OrderByFilter(const std::shared_ptr<Operand>& op, bool asc = true)
        : operand(op), ascending(asc) {}
};

// Limit Filter
struct LimitFilter : public Filter {
    int limit;
    int offset;

    LimitFilter(int lim, int off = 0) : limit(lim), offset(off) {}
};

// Composite ElementFilter
struct ElementFilter {
    std::vector<std::shared_ptr<Filter>> filters;

    void addFilter(const std::shared_ptr<Filter>& filter) {
        filters.push_back(filter);
    }
};

// Having Filter
struct HavingFilter : public Filter {
    std::shared_ptr<GroupOperand> left;
    Comparator comparator;
    std::shared_ptr<GroupOperand> right;

    HavingFilter(const std::shared_ptr<GroupOperand>& l, Comparator comp, const std::shared_ptr<GroupOperand>& r)
        : left(l), comparator(comp), right(r) {}
};

// Composite GroupFilter
struct GroupFilter {
    std::vector<std::shared_ptr<Filter>> filters;

    void addFilter(const std::shared_ptr<Filter>& filter) {
        filters.push_back(filter);
    }
};

#endif // FILTER_H

