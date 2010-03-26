/*-----------------------------------------.---------------------------------.
| Filename: Table.cpp                      | Tables                          |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 16:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Table.h>
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
