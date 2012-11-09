/*-----------------------------------------.---------------------------------.
| Filename: Table.cpp                      | Table                           |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Data/Table.h>
#include <algorithm>
using namespace lbcpp;

/*
** Table
*/
Table::Table(size_t numSamples)
  : allIndices(new IndexSet(0, numSamples)) {}

void Table::addColumn(const ObjectPtr& key, const TypePtr& type)
{
  Column c;
  c.key = key;
  c.type = type;
  c.data = vector(type, allIndices->size());
  columnMap[key] = columns.size();
  columns.push_back(c);
}

String Table::getDescription(size_t index) const
{
  jassert(index < columns.size());
  return columns[index].key->toShortString();
}

void Table::setElement(size_t rowIndex, size_t columnIndex, const ObjectPtr& value)
{
  jassert(rowIndex < allIndices->size());
  jassert(columnIndex < columns.size());
  columns[columnIndex].data->setElement(rowIndex, value);
}

ObjectPtr Table::getElement(size_t rowIndex, size_t columnIndex) const
{
  jassert(rowIndex < allIndices->size());
  jassert(columnIndex < columns.size());
  return columns[columnIndex].data->getElement(rowIndex).getObject();
}

VectorPtr Table::getDataByKey(const ObjectPtr& key) const
{
  ColumnMap::const_iterator it = columnMap.find(key);
  return it == columnMap.end() ? VectorPtr() : columns[it->second].data;
}

struct OrderContainerFunction
{
  OrderContainerFunction(const TablePtr& data, size_t columnIndex, bool increasingOrder)
    : data(data), columnIndex(columnIndex), increasingOrder(increasingOrder) {}

  TablePtr data;
  size_t columnIndex;
  bool increasingOrder;
  
  bool operator ()(size_t first, size_t second) const
  {
    ObjectPtr a = data->getElement(first, columnIndex);
    ObjectPtr b = data->getElement(second, columnIndex);
    int c = a->compare(b);
    return increasingOrder ? (c > 0) : (c < 0);
  }
};

void Table::makeOrder(size_t columnIndex, bool increasingOrder, std::vector<size_t>& res) const
{
  res = allIndices->getIndices();
  OrderContainerFunction order(refCountedPointerFromThis(this), columnIndex, increasingOrder);
  std::sort(res.begin(), res.end(), order);
}
