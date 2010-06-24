/*-----------------------------------------.---------------------------------.
| Filename: Table.cpp                      | Tables                          |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Object/Table.h>
#include <algorithm>
using namespace lbcpp;

size_t Table::getNumColumns() const
{
  return getHeader()->getNumColumns();
}

String Table::toString(size_t rowNumber, size_t columnNumber) const
{
  TableHeader::Type t = getHeader()->getColumnType(columnNumber);
  switch (t)
  {
  case TableHeader::integerType:
    return lbcpp::toString(getInteger(rowNumber, columnNumber));

  case TableHeader::doubleType:
    return lbcpp::toString(getDouble(rowNumber, columnNumber));

  case TableHeader::stringType:
    return getString(rowNumber, columnNumber);

  default:
    jassert(false);
    return "";
  };
}

class ReorderedTable : public Table
{
public:
  ReorderedTable(TablePtr target, const std::vector<size_t>& order)
    : target(target), order(order) {}

  virtual TableHeaderPtr getHeader() const
    {return target->getHeader();}

  virtual size_t getNumRows() const
    {return target->getNumRows();}
  
  virtual TablePtr sort(size_t columnNumber, bool decreasingOrder) const
    {return target->sort(columnNumber, decreasingOrder);}

  virtual int getInteger(size_t rowNumber, size_t columnNumber) const
    {return target->getInteger(order[rowNumber], columnNumber);}
    
  virtual double getDouble(size_t rowNumber, size_t columnNumber) const
    {return target->getDouble(order[rowNumber], columnNumber);}

  virtual String getString(size_t rowNumber, size_t columnNumber) const
    {return target->getString(order[rowNumber], columnNumber);}

private:
  TablePtr target;
  std::vector<size_t> order;
};

struct TableSortOperator
{
  TableSortOperator(TablePtr table, size_t columnNumber, bool decreasingOrder)
    : table(table), columnNumber(columnNumber), decreasingOrder(decreasingOrder) {}
    
  TablePtr table;
  size_t columnNumber;
  bool decreasingOrder;
  
  bool operator ()(size_t a, size_t b) const
  {
    if (a == b)
      return false;

    if (decreasingOrder)
      {size_t tmp = a; a = b; b = tmp;}
    
    jassert(a < table->getNumRows() && b < table->getNumRows());
    
    switch (table->getHeader()->getColumnType(columnNumber))
    {
    case TableHeader::integerType:
      return table->getInteger(a, columnNumber) < table->getInteger(b, columnNumber);

    case TableHeader::doubleType:
      return table->getDouble(a, columnNumber) < table->getDouble(b, columnNumber);

    case TableHeader::stringType:
      return table->getString(a, columnNumber) < table->getString(b, columnNumber);
    }
    jassert(false);
    return false;
  }
};

TablePtr Table::sort(size_t columnNumber, bool decreasingOrder) const
{
  std::vector<size_t> order(getNumRows());
  for (size_t i = 0; i < order.size(); ++i)
    order[i] = i;
  TablePtr pthis(const_cast<Table* >(this));
  std::sort(order.begin(), order.end(), TableSortOperator(pthis, columnNumber, decreasingOrder));
  return new ReorderedTable(pthis, order);
}
