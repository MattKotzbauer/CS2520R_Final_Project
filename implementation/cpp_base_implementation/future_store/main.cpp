#include "CSVLoader.h"
#include "Query.h"
#include <memory>
#include <iostream>

int main() {
    // Load CSV
    CSVLoader loader("data.csv");
    if (!loader.load()) {
        return 1;
    }

    QueryExecutor executor(loader);

    // Example 1: Select the 'salary' column where 'age' > 27
    {
        // Define Operand for 'salary'
        auto salary = std::make_shared<Operand>("salary");

        // Create ElementSelect
        ElementSelect select(salary, "data.csv");

        // Define Operand for 'age'
        auto age = std::make_shared<Operand>("age");

        // Define Operand for 27
        auto value27 = std::make_shared<Operand>(27);

        // Create WhereFilter: age > 27
        auto where_filter = std::make_shared<WhereFilter>(age, Comparator::GREATER, value27);
        select.filter->addFilter(where_filter);

        std::cout << "Salaries where age > 27:" << std::endl;
        executor.execute(select);
    }

    // Example 2: Group by 'age' and compute MAX(salary)
    {
        // Define GroupOperand for MAX(salary)
        auto max_salary = std::make_shared<GroupOperand>(GroupOperator::MAX, "salary");

        // Define Operand for 'age' (group by)
        auto group_by_age = std::make_shared<Operand>("age");

        // Create GroupSelect
        GroupSelect group_select(max_salary, "data.csv", group_by_age);

        std::cout << "\nMAX salary grouped by age:" << std::endl;
        executor.execute(group_select);
    }

    return 0;
}

