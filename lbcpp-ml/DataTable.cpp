/*-----------------------------------------.---------------------------------.
| Filename: DataTable.cpp                  | Data Table                      |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Expression.h>
#include <lbcpp-ml/DataTable.h>
#include <algorithm>
using namespace lbcpp;

DataTable::DataTable(size_t numSamples)
  : allIndices(new IndexSet(0, numSamples)) {}

void DataTable::addColumn(const ExpressionPtr& expression)
{
  Column c;
  c.expression = expression;
  c.samples = vector(expression->getType(), allIndices->size());
  columnMap[expression] = columns.size();
  columns.push_back(c);
}

String DataTable::getColumnName(size_t index) const
{
  jassert(index < columns.size());
  return columns[index].expression->toShortString();
}

void DataTable::setSample(size_t rowIndex, size_t columnIndex, const ObjectPtr& value)
{
  jassert(rowIndex < allIndices->size());
  jassert(columnIndex < columns.size());
  columns[columnIndex].samples->setElement(rowIndex, value);
}

ObjectPtr DataTable::getSample(size_t rowIndex, size_t columnIndex) const
{
  jassert(rowIndex < allIndices->size());
  jassert(columnIndex < columns.size());
  return columns[columnIndex].samples->getElement(rowIndex).getObject();
}

VectorPtr DataTable::getSamplesByExpression(const ExpressionPtr& expression) const
{
  ExpressionMap::const_iterator it = columnMap.find(expression);
  return it == columnMap.end() ? VectorPtr() : columns[it->second].samples;
}

struct OrderContainerFunction
{
  OrderContainerFunction(const DataTablePtr& data, size_t columnIndex, bool increasingOrder)
    : data(data), columnIndex(columnIndex), increasingOrder(increasingOrder) {}

  DataTablePtr data;
  size_t columnIndex;
  bool increasingOrder;
  
  bool operator ()(size_t first, size_t second) const
  {
    ObjectPtr a = data->getSample(first, columnIndex);
    ObjectPtr b = data->getSample(second, columnIndex);
    int c = a->compare(b);
    return increasingOrder ? (c > 0) : (c < 0);
  }
};

void DataTable::makeOrder(size_t columnIndex, bool increasingOrder, std::vector<size_t>& res) const
{
  res = allIndices->getIndices();
  OrderContainerFunction order(refCountedPointerFromThis(this), columnIndex, increasingOrder);
  std::sort(res.begin(), res.end(), order);
}
