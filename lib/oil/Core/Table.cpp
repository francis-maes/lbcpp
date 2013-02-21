/*-----------------------------------------.---------------------------------.
| Filename: Table.cpp                      | Table                           |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <oil/Core/String.h>
#include <oil/Core/Table.h>
#include <oil/Execution/ExecutionContext.h>
#include <algorithm>
using namespace lbcpp;

/*
** Table
*/
Table::Table(size_t numRows)
  : numRows(numRows) {}

void Table::addColumn(const ObjectPtr& key, const ClassPtr& type)
{
  Column c;
  c.key = key;
  c.type = type;
  c.data = vector(type, getNumRows());
  columnMap[key] = columns.size();
  columns.push_back(c);
}

void Table::addColumn(const string& name, const ClassPtr& type)
{
  addColumn(new String(name), type);
}

void Table::addRow(const std::vector<ObjectPtr>& elements)
{
  ++numRows;
  jassert(elements.size() == columns.size());
  for (size_t i = 0; i < elements.size(); ++i)
  {
    const ObjectPtr& element = elements[i];
    jassert(!element || element->getClass()->inheritsFrom(columns[i].type));
    columns[i].data->appendElement(element);
  }
}

void Table::resize(size_t numRows)
{
  for (size_t i = 0; i < columns.size(); ++i)
    columns[i].data->resize(numRows);
  this->numRows = numRows;
}

string Table::getDescription(size_t index) const
{
  jassert(index < columns.size());
  return columns[index].key->toShortString();
}

void Table::setElement(size_t rowIndex, size_t columnIndex, const ObjectPtr& value)
{
  jassert(rowIndex < numRows);
  jassert(columnIndex < columns.size());
  columns[columnIndex].data->setElement(rowIndex, value);
}

ObjectPtr Table::getElement(size_t rowIndex, size_t columnIndex) const
{
  jassert(rowIndex < numRows);
  jassert(columnIndex < columns.size());
  return columns[columnIndex].data->getElement(rowIndex);
}

int Table::findColumnByKey(const ObjectPtr& key) const
{
  ColumnMap::const_iterator it = columnMap.find(key);
  return it == columnMap.end() ? -1 : (int)it->second;
}

VectorPtr Table::getDataByKey(const ObjectPtr& key) const
{
  ColumnMap::const_iterator it = columnMap.find(key);
  return it == columnMap.end() ? VectorPtr() : columns[it->second].data;
}

struct OrderContainerFunction
{
  OrderContainerFunction(const TablePtr& data, size_t columnIndex, bool increasingOrder, bool convertToDoubles)
    : data(data), columnIndex(columnIndex), increasingOrder(increasingOrder), convertToDoubles(convertToDoubles) {}

  TablePtr data;
  size_t columnIndex;
  bool increasingOrder;
  bool convertToDoubles;
  
  bool operator ()(size_t first, size_t second) const
  {
    ObjectPtr a = data->getElement(first, columnIndex);
    ObjectPtr b = data->getElement(second, columnIndex);
    if (!a && !b)
      return first < second;
    if (!a && b)
      return !increasingOrder;
    if (a && !b)
      return increasingOrder;

    if (convertToDoubles)
    {
      double da = a->toDouble();
      double db = b->toDouble();
      if (da == db)
        return first < second;
      return increasingOrder ? (da > db) : (da < db);
    }
    else
    {
      int c = a->compare(b);
      if (c == 0)
        return first < second;
      return increasingOrder ? (c > 0) : (c < 0);
    }
  }
};

void Table::makeOrder(size_t columnIndex, bool increasingOrder, std::vector<size_t>& res) const
{
  res.resize(numRows);
  for (size_t i = 0; i < numRows; ++i)
    res[i] = i;
  OrderContainerFunction order(refCountedPointerFromThis(this), columnIndex, increasingOrder, getType(columnIndex)->isConvertibleToDouble());
  std::sort(res.begin(), res.end(), order);
}

void Table::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const TablePtr& target = t.staticCast<Table>();
  target->numRows = numRows;
  target->columnMap = columnMap;
  target->columns = columns;
}

TablePtr Table::randomize(ExecutionContext& context) const
{
  size_t nc = columns.size();
  TablePtr res = cloneAndCast<Table>();
  std::vector<size_t> order;
  context.getRandomGenerator()->sampleOrder(numRows, order);
  for (size_t i = 0; i < numRows; ++i)
    res->columns[i].data = vector(res->columns[i].type, numRows);
  for (size_t i = 0; i < order.size(); ++i)
    for (size_t j = 0; j < nc; ++j)
      res->setElement(i, j, getElement(order[i], j));
  return res;
}

TablePtr Table::range(size_t begin, size_t end) const
{
  jassert(end >= begin);
  size_t numColumns = getNumColumns();
  TablePtr res = cloneAndCast<Table>();
  res->numRows = end - begin;
  for (size_t i = 0; i < numColumns; ++i)
  {
    VectorPtr data = vector(columns[i].type, res->numRows);
    for (size_t j = 0; j < res->numRows; ++j)
      data->setElement(j, columns[i].data->getElement(begin + j));
    res->columns[i].data = data;
  }
  return res;
}

TablePtr Table::invRange(size_t begin, size_t end) const
{
  jassert(end >= begin);
  size_t numColumns = getNumColumns();
  TablePtr res = cloneAndCast<Table>();
  res->numRows = getNumRows() - (end - begin);
  for (size_t i = 0; i < numColumns; ++i)
  {
    VectorPtr data = vector(columns[i].type, res->numRows);
    size_t idx = 0;
    for (size_t j = 0; j < begin; ++j)
      data->setElement(idx++, columns[i].data->getElement(j));
    for (size_t j = end; j < res->numRows; ++j)
      data->setElement(idx++, columns[i].data->getElement(j));
    res->columns[i].data = data;
  }
  return res;
}
