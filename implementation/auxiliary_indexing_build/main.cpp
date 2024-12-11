// main.cpp

#include "CSVLoader.h"
#include "Operand.h"
#include "ElementFilter.h"
#include "ElementSelect.h"
#include "QueryExecutor.h"
#include <iostream>
#include <memory>

using namespace std;

int main(int argc, char* argv[]) {
    // Check if the CSV filename is provided as a command-line argument
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <csv_filename>" << endl;
        return 1;
    }

    // Get the CSV file name from the first command-line argument
    string filename = argv[1];

    // Create an instance of CSVLoader with the provided filename
    CSVLoader loader(filename);

    // Load the CSV data
    if (!loader.load()) {
        cerr << "Error: Failed to load the CSV file." << endl;
        return 1;
    }

    // Define operands to select: 'name', 'salary', and an expression 'salary + 5000'
    vector<shared_ptr<Operand>> operands;
    operands.push_back(make_shared<ColumnOperand>("name"));
    operands.push_back(make_shared<ColumnOperand>("salary"));

    // Create an expression operand: salary + 5000
    shared_ptr<Operand> salaryOperand = make_shared<ColumnOperand>("salary");
    shared_ptr<Operand> value5000 = make_shared<IntegerOperand>(5000);
    shared_ptr<Operand> salaryPlus5000 = make_shared<ExpressionOperand>(salaryOperand, OperatorType::ADD, value5000);
    operands.push_back(salaryPlus5000);

    // Debug: Verify operand types
    // cout << "Defined Operands:" << endl;
    // for (const auto& operand : operands) {
    //     if (dynamic_pointer_cast<ColumnOperand>(operand)) {
    //         cout << "- ColumnOperand" << endl;
    //     }
    //     else if (dynamic_pointer_cast<IntegerOperand>(operand)) {
    //         cout << "- IntegerOperand" << endl;
    //     }
    //     else if (dynamic_pointer_cast<ExpressionOperand>(operand)) {
    //         cout << "- ExpressionOperand" << endl;
    //     }
    //     else if (dynamic_pointer_cast<BooleanOperand>(operand)) {
    //         cout << "- BooleanOperand" << endl;
    //     }
    //     else {
    //         cout << "- Unknown Operand Type" << endl;
    //     }
    // }
    // cout << endl;

    // Create ElementSelect
    ElementSelect select(operands, filename);

    // WHERE filter: age > 27
    shared_ptr<Operand> ageOperand = make_shared<ColumnOperand>("age");
    shared_ptr<Operand> value27 = make_shared<IntegerOperand>(27);
    shared_ptr<WhereFilter> whereFilter1 = make_shared<WhereFilter>(ageOperand, Comparator::GREATER, value27);
    select.addFilter(whereFilter1);

    // WHERE filter: salary >= 60000
    shared_ptr<Operand> salaryOperandFilter = make_shared<ColumnOperand>("salary");
    shared_ptr<Operand> value60000 = make_shared<IntegerOperand>(60000);
    shared_ptr<WhereFilter> whereFilter2 = make_shared<WhereFilter>(salaryOperandFilter, Comparator::GREATER_EQUAL, value60000);
    select.addFilter(whereFilter2);

    // DISTINCT filter
    shared_ptr<ElementFilter> distinctFilter = make_shared<DistinctFilter>(operands);
    select.addFilter(distinctFilter);

    // Optionally, add a LIMIT filter (e.g., limit to 2 results)
    // shared_ptr<ElementFilter> limitFilter = make_shared<LimitFilter>(2);
    // select.addFilter(limitFilter);

    // Create QueryExecutor
    QueryExecutor executor(loader);

    // Execute the query
    executor.execute(select);

    return 0;
}

