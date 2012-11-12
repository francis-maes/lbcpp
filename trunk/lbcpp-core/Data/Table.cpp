/*-----------------------------------------.---------------------------------.
| Filename: Table.cpp                      | Table                           |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 15:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/String.h>
#include <lbcpp/Data/Table.h>
#include <algorithm>
using namespace lbcpp;

/*
** Table
*/
Table::Table(size_t numSamples)
  : allIndices(new IndexSet(0, numSamples)) {}

void Table::addColumn(const ObjectPtr& key, const ClassPtr& type)
{
  Column c;
  c.key = key;
  c.type = type;
  c.data = vector(type, getNumRows());
  columnMap[key] = columns.size();
  columns.push_back(c);
}

void Table::addColumn(const String& name, const ClassPtr& type)
{
  addColumn(new NewString(name), type);
}

void Table::addRow(const std::vector<ObjectPtr>& elements)
{
  if (allIndices)
    allIndices->append(allIndices->size());
  else
    allIndices = new IndexSet(0, 1);
  jassert(elements.size() == columns.size());
  for (size_t i = 0; i < elements.size(); ++i)
  {
    const ObjectPtr& element = elements[i];
    jassert(!element || element->getClass()->inheritsFrom(columns[i].type));
    columns[i].data->append(element);
  }
}

void Table::resize(size_t numRows)
{
  for (size_t i = 0; i < columns.size(); ++i)
    columns[i].data->resize(numRows);
  allIndices = new IndexSet(0, numRows);
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
  res = allIndices->getIndices();
  OrderContainerFunction order(refCountedPointerFromThis(this), columnIndex, increasingOrder, getType(columnIndex)->isConvertibleToDouble());
  std::sort(res.begin(), res.end(), order);
}
