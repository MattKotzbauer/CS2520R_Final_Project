// QueryExecutor.h
#ifndef QUERYEXECUTOR_H
#define QUERYEXECUTOR_H

#include "CSVLoader.h"
#include "ElementSelect.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <algorithm>

class QueryExecutor {
public:
    QueryExecutor(const CSVLoader& loader) : loader_(loader) {}
    
    void execute(const ElementSelect& select) const;
    
private:
    const CSVLoader& loader_;
};

#endif // QUERYEXECUTOR_H

