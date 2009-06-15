/*-----------------------------------------.---------------------------------.
| Filename: Table.h                        | Base class for tables           |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 16:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_TABLE_H_
# define LBCPP_TABLE_H_

# include "ObjectPredeclarations.h"

namespace lbcpp
{

class TableHeader : public Object
{
public:
  enum Type
  {
    integerType,
    doubleType,
    stringType
  };

  size_t getNumColumns() const
    {return columns.size();}
    
  std::string getColumnName(size_t index) const
    {assert(index < columns.size()); return columns[index].first;}

  Type getColumnType(size_t index) const
    {assert(index < columns.size()); return columns[index].second;}

  void addColumn(const std::string& name, const Type& type)
    {columns.push_back(std::make_pair(name, type));}

private:
  std::vector<std::pair<std::string, Type> > columns;
};

class Table : public Object
{
public:
  virtual TableHeaderPtr getHeader() const = 0;
  virtual size_t getNumRows() const = 0;
  size_t getNumColumns() const;
  
  virtual int getInteger(size_t rowNumber, size_t columnNumber) const
    {assert(false); return 0;}
  virtual double getDouble(size_t rowNumber, size_t columnNumber) const
    {assert(false); return 0.0;}
  virtual std::string getString(size_t rowNumber, size_t columnNumber) const
    {assert(false); return "";}
    
  std::string toString(size_t rowNumber, size_t columnNumber) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_TABLE_H_
