import pandas as pd
from dataclasses import dataclass
from typing import List, Union, Any
from enum import Enum

# Enums and Classes as defined by you
class Comparator(Enum):
    EQUAL = '='
    NOT_EQUAL = '<>'
    GREATER_THAN = '>'
    LESS_THAN = '<'
    GREATER_EQUAL = '>='
    LESS_EQUAL = '<='
    IN = 'IN'

class Operator(Enum):
    EQUAL = '='
    NOT_EQUAL = '<>'
    GREATER_THAN = '>'
    LESS_THAN = '<'
    GREATER_EQUAL = '>='
    LESS_EQUAL = '<='
    IN = 'IN'
    PLUS = '+'
    MINUS = '-'
    MULTIPLY = '*'
    DIVIDE = '/'

class GroupOperator(Enum):
    MAX = 'MAX'
    MIN = 'MIN'
    AVG = 'AVG'

# Operand definitions
class Operand:
    pass

@dataclass
class ColumnOperand(Operand):
    column_id: str

@dataclass
class ValueOperand(Operand):
    value: Any  # Could be int, str, etc.

@dataclass
class BinaryOperand(Operand):
    left: Operand
    operator: Operator
    right: Operand

@dataclass
class ListOperand(Operand):
    operands: List[Operand]

# GroupOperand definitions
class GroupOperand:
    pass

@dataclass
class GroupFunctionOperand(GroupOperand):
    group_operator: GroupOperator
    column_id: str

@dataclass
class GroupValueOperand(GroupOperand):
    value: Any

@dataclass
class GroupBinaryOperand(GroupOperand):
    left: GroupOperand
    operator: Operator
    right: GroupOperand

@dataclass
class GroupListOperand(GroupOperand):
    operands: List[GroupOperand]

# ElementFilter definitions
class ElementFilter:
    pass

@dataclass
class EmptyElementFilter(ElementFilter):
    pass

@dataclass
class WhereElementFilter(ElementFilter):
    previous_filter: ElementFilter
    left_operand: Operand
    comparator: Comparator
    right_operand: Operand

@dataclass
class DistinctElementFilter(ElementFilter):
    previous_filter: ElementFilter

@dataclass
class OrderByElementFilter(ElementFilter):
    previous_filter: ElementFilter
    operand: Operand

@dataclass
class LimitElementFilter(ElementFilter):
    previous_filter: ElementFilter
    limit: int

# GroupFilter definitions
class GroupFilter:
    pass

@dataclass
class EmptyGroupFilter(GroupFilter):
    pass

@dataclass
class HavingGroupFilter(GroupFilter):
    previous_filter: GroupFilter
    left_operand: GroupOperand
    comparator: Comparator
    right_operand: GroupOperand

@dataclass
class OrderByGroupFilter(GroupFilter):
    previous_filter: GroupFilter
    operand: GroupOperand

@dataclass
class LimitGroupFilter(GroupFilter):
    previous_filter: GroupFilter
    limit: int

# ElementSelect definitions
class ElementSelect:
    pass

@dataclass
class BaseElementSelect(ElementSelect):
    operand: Operand
    table: str

@dataclass
class FilteredElementSelect(ElementSelect):
    element_select: ElementSelect
    element_filter: ElementFilter

# GroupSelect definitions
class GroupSelect:
    pass

@dataclass
class BaseGroupSelect(GroupSelect):
    group_operand: GroupOperand
    table: str
    group_by_operand: Operand

@dataclass
class FilteredGroupSelect(GroupSelect):
    group_select: GroupSelect
    element_filter: ElementFilter  # Can be EmptyElementFilter()
    group_filter: GroupFilter      # Can be EmptyGroupFilter()

# Evaluator Implementation
class Evaluator:
    def __init__(self, csv_path: str):
        self.df = pd.read_csv(csv_path)
        print(f"Loaded CSV with columns: {self.df.columns.tolist()}")

    def evaluate_operand(self, operand: Operand) -> Any:
        if isinstance(operand, ColumnOperand):
            return self.df[operand.column_id]
        elif isinstance(operand, ValueOperand):
            return operand.value
        elif isinstance(operand, BinaryOperand):
            left = self.evaluate_operand(operand.left)
            right = self.evaluate_operand(operand.right)
            return self.apply_operator(left, operand.operator, right)
        elif isinstance(operand, ListOperand):
            return [self.evaluate_operand(op) for op in operand.operands]
        else:
            raise ValueError(f"Unsupported Operand type: {type(operand)}")

    def apply_operator(self, left, operator: Operator, right):
        if operator == Operator.PLUS:
            return left + right
        elif operator == Operator.MINUS:
            return left - right
        elif operator == Operator.MULTIPLY:
            return left * right
        elif operator == Operator.DIVIDE:
            return left / right
        elif operator == Operator.EQUAL:
            return left == right
        elif operator == Operator.NOT_EQUAL:
            return left != right
        elif operator == Operator.GREATER_THAN:
            return left > right
        elif operator == Operator.LESS_THAN:
            return left < right
        elif operator == Operator.GREATER_EQUAL:
            return left >= right
        elif operator == Operator.LESS_EQUAL:
            return left <= right
        elif operator == Operator.IN:
            return left.isin(right) if isinstance(right, list) else left == right
        else:
            raise ValueError(f"Unsupported Operator: {operator}")

    def evaluate_group_operand(self, group_operand: GroupOperand) -> Any:
        if isinstance(group_operand, GroupFunctionOperand):
            if group_operand.group_operator == GroupOperator.MAX:
                return self.df[group_operand.column_id].max()
            elif group_operand.group_operator == GroupOperator.MIN:
                return self.df[group_operand.column_id].min()
            elif group_operand.group_operator == GroupOperator.AVG:
                return self.df[group_operand.column_id].mean()
            else:
                raise ValueError(f"Unsupported GroupOperator: {group_operand.group_operator}")
        elif isinstance(group_operand, GroupValueOperand):
            return group_operand.value
        elif isinstance(group_operand, GroupBinaryOperand):
            left = self.evaluate_group_operand(group_operand.left)
            right = self.evaluate_group_operand(group_operand.right)
            return self.apply_operator(left, group_operand.operator, right)
        elif isinstance(group_operand, GroupListOperand):
            return [self.evaluate_group_operand(op) for op in group_operand.operands]
        else:
            raise ValueError(f"Unsupported GroupOperand type: {type(group_operand)}")

    def evaluate_element_filter(self, filter_obj: ElementFilter) -> pd.DataFrame:
        if isinstance(filter_obj, EmptyElementFilter):
            return self.df
        elif isinstance(filter_obj, WhereElementFilter):
            previous_df = self.evaluate_element_filter(filter_obj.previous_filter)
            left = self.evaluate_operand(filter_obj.left_operand)
            right = self.evaluate_operand(filter_obj.right_operand)
            comparator = filter_obj.comparator

            if comparator == Comparator.EQUAL:
                condition = left == right
            elif comparator == Comparator.NOT_EQUAL:
                condition = left != right
            elif comparator == Comparator.GREATER_THAN:
                condition = left > right
            elif comparator == Comparator.LESS_THAN:
                condition = left < right
            elif comparator == Comparator.GREATER_EQUAL:
                condition = left >= right
            elif comparator == Comparator.LESS_EQUAL:
                condition = left <= right
            elif comparator == Comparator.IN:
                condition = left.isin(right) if isinstance(right, list) else left == right
            else:
                raise ValueError(f"Unsupported Comparator: {comparator}")

            filtered_df = previous_df[condition]
            print(f"Applied WHERE filter: {comparator} on operands, resulting in {len(filtered_df)} rows")
            return filtered_df

        elif isinstance(filter_obj, DistinctElementFilter):
            previous_df = self.evaluate_element_filter(filter_obj.previous_filter)
            distinct_df = previous_df.drop_duplicates()
            print(f"Applied DISTINCT filter, resulting in {len(distinct_df)} unique rows")
            return distinct_df

        elif isinstance(filter_obj, OrderByElementFilter):
            previous_df = self.evaluate_element_filter(filter_obj.previous_filter)
            operand = filter_obj.operand
            if isinstance(operand, ColumnOperand):
                sorted_df = previous_df.sort_values(by=operand.column_id)
            else:
                raise ValueError("OrderBy currently supports only ColumnOperand")
            print(f"Applied ORDER BY on {operand.column_id}")
            return sorted_df

        elif isinstance(filter_obj, LimitElementFilter):
            previous_df = self.evaluate_element_filter(filter_obj.previous_filter)
            limited_df = previous_df.head(filter_obj.limit)
            print(f"Applied LIMIT of {filter_obj.limit}, resulting in {len(limited_df)} rows")
            return limited_df

        else:
            raise ValueError(f"Unsupported ElementFilter type: {type(filter_obj)}")

    def evaluate_group_filter(self, group_filter_obj: GroupFilter, grouped_df: pd.core.groupby.DataFrameGroupBy) -> pd.core.groupby.DataFrameGroupBy:
        if isinstance(group_filter_obj, EmptyGroupFilter):
            return grouped_df
        elif isinstance(group_filter_obj, HavingGroupFilter):
            previous_grouped = self.evaluate_group_filter(group_filter_obj.previous_filter, grouped_df)
            # Apply HAVING condition
            left = self.evaluate_group_operand(group_filter_obj.left_operand)
            right = self.evaluate_group_operand(group_filter_obj.right_operand)
            comparator = group_filter_obj.comparator

            if comparator == Comparator.EQUAL:
                condition = left == right
            elif comparator == Comparator.NOT_EQUAL:
                condition = left != right
            elif comparator == Comparator.GREATER_THAN:
                condition = left > right
            elif comparator == Comparator.LESS_THAN:
                condition = left < right
            elif comparator == Comparator.GREATER_EQUAL:
                condition = left >= right
            elif comparator == Comparator.LESS_EQUAL:
                condition = left <= right
            elif comparator == Comparator.IN:
                condition = left.isin(right) if isinstance(right, list) else left == right
            else:
                raise ValueError(f"Unsupported Comparator: {comparator}")

            # Since pandas doesn't directly support HAVING in groupby,
            # we need to filter the groups based on the condition
            filtered_groups = grouped_df.filter(lambda x: condition)
            print(f"Applied HAVING filter with comparator {comparator}")
            return filtered_groups
        elif isinstance(group_filter_obj, OrderByGroupFilter):
            # Pandas doesn't support ordering within groupby directly.
            # This would typically be handled post aggregation.
            print("ORDER BY in GROUP FILTER is not directly supported in this evaluator.")
            return grouped_df
        elif isinstance(group_filter_obj, LimitGroupFilter):
            # Similarly, LIMIT in group filter would need custom handling
            print("LIMIT in GROUP FILTER is not directly supported in this evaluator.")
            return grouped_df
        else:
            raise ValueError(f"Unsupported GroupFilter type: {type(group_filter_obj)}")

    def execute_element_select(self, select: ElementSelect) -> pd.DataFrame:
        if isinstance(select, BaseElementSelect):
            selected = self.evaluate_operand(select.operand)
            if isinstance(selected, pd.Series):
                result = selected.to_frame(select.operand.column_id)
            else:
                result = pd.DataFrame(selected)
            print(f"Selected columns: {result.columns.tolist()}")
            return result
        elif isinstance(select, FilteredElementSelect):
            base_df = self.execute_element_select(select.element_select)
            filter_df = self.evaluate_element_filter(select.element_filter)
            # Merge the base selection with the filter
            # Assuming base_df is a subset of self.df
            merged_df = base_df.loc[filter_df.index]
            print(f"Filtered selection to {len(merged_df)} rows")
            return merged_df
        else:
            raise ValueError(f"Unsupported ElementSelect type: {type(select)}")

    def execute_group_select(self, select: GroupSelect) -> pd.DataFrame:
        if isinstance(select, BaseGroupSelect):
            group_by = self.evaluate_operand(select.group_by_operand)
            if isinstance(group_by, pd.Series):
                group_by = group_by
            elif isinstance(group_by, list):
                group_by = self.df[group_by]
            else:
                raise ValueError("Unsupported group_by_operand type")

            grouped = self.df.groupby(group_by)
            aggregation = {}
            if isinstance(select.group_operand, GroupFunctionOperand):
                func = select.group_operand.group_operator.value.lower()
                aggregation[select.group_operand.column_id] = func
            else:
                raise ValueError("Unsupported group_operand in BaseGroupSelect")

            aggregated_df = grouped.agg(aggregation).reset_index()
            print(f"Performed GROUP BY on {select.group_by_operand} with aggregation {aggregation}")
            return aggregated_df
        elif isinstance(select, FilteredGroupSelect):
            base_group_df = self.execute_group_select(select.group_select)
            # Apply element_filter and group_filter
            # For simplicity, assuming filters are applied on the aggregated data
            if isinstance(select.element_filter, EmptyElementFilter) and isinstance(select.group_filter, EmptyGroupFilter):
                return base_group_df
            # Implementing filters requires more detailed handling
            # Here, we'll just return the base_group_df
            print("FilteredGroupSelect with non-empty filters is not fully implemented.")
            return base_group_df
        else:
            raise ValueError(f"Unsupported GroupSelect type: {type(select)}")

# Example Usage
if __name__ == "__main__":
    # Initialize Evaluator with sample.csv
    evaluator = Evaluator("sample.csv")

    # Example 1: Select 'name' where 'age' > 30
    select_operand = ColumnOperand(column_id="name")
    base_select = BaseElementSelect(operand=select_operand, table="sample")
    where_filter = WhereElementFilter(
        previous_filter=EmptyElementFilter(),
        left_operand=ColumnOperand(column_id="age"),
        comparator=Comparator.GREATER_THAN,
        right_operand=ValueOperand(value=30)
    )
    filtered_select = FilteredElementSelect(
        element_select=base_select,
        element_filter=where_filter
    )

    result_df = evaluator.execute_element_select(filtered_select)
    print("Query 1 Result:")
    print(result_df)

    # Example 2: Select distinct 'age' ordered by 'age' ascending
    select_operand = ColumnOperand(column_id="age")
    base_select = BaseElementSelect(operand=select_operand, table="sample")
    distinct_filter = DistinctElementFilter(previous_filter=EmptyElementFilter())
    order_filter = OrderByElementFilter(
        previous_filter=distinct_filter,
        operand=ColumnOperand(column_id="age")
    )
    filtered_select = FilteredElementSelect(
        element_select=base_select,
        element_filter=order_filter
    )

    result_df = evaluator.execute_element_select(filtered_select)
    print("\nQuery 2 Result:")
    print(result_df)

    # Example 3: Group by 'age' and get the maximum 'id' in each group
    group_operand = GroupFunctionOperand(group_operator=GroupOperator.MAX, column_id="id")
    group_by_operand = ColumnOperand(column_id="age")
    base_group_select = BaseGroupSelect(
        group_operand=group_operand,
        table="sample",
        group_by_operand=group_by_operand
    )

    result_df = evaluator.execute_group_select(base_group_select)
    print("\nQuery 3 Result:")
    print(result_df)
