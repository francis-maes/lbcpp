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

VectorPtr DataTable::getSamples(const ExpressionPtr& expression) const
{
  ExpressionMap::const_iterator it = columnMap.find(expression);
  return it == columnMap.end() ? VectorPtr() : columns[it->second].samples;
}
