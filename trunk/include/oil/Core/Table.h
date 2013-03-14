/*-----------------------------------------.---------------------------------.
| Filename: Table.h                        | Table                           |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 20:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_CORE_TABLE_H_
# define OIL_CORE_TABLE_H_

# include "Vector.h"

namespace lbcpp
{
  
class Table : public Object
{
public:
  Table(size_t numRows = 0);

  void addColumn(const ObjectPtr& key, const ClassPtr& type);
  void addColumn(const string& name, const ClassPtr& type);
  void addRow(const std::vector<ObjectPtr>& elements);
  void resize(size_t numRows);

  size_t getNumColumns() const
    {return columns.size();}

  size_t getNumRows() const
    {return numRows;}

  std::vector<ObjectPtr> getRow(size_t index) const;

  string getDescription(size_t index) const;

  ObjectPtr getKey(size_t index) const
    {jassert(index < columns.size()); return columns[index].key;}

  ClassPtr getType(size_t index) const
    {jassert(index < columns.size()); return columns[index].type;}

  VectorPtr getData(size_t index) const
    {jassert(index < columns.size()); return columns[index].data;}

  int findColumnByKey(const ObjectPtr& key) const;
  VectorPtr getDataByKey(const ObjectPtr& key) const;

  ObjectPtr getElement(size_t rowIndex, size_t columnIndex) const;
  void setElement(size_t rowIndex, size_t columnIndex, const ObjectPtr& value);

  void makeOrder(size_t columnIndex, bool increasingOrder, std::vector<size_t>& res) const;

  TablePtr randomize(ExecutionContext& context) const;
  TablePtr range(size_t begin, size_t end) const;
  TablePtr invRange(size_t begin, size_t end) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

private:
  size_t numRows;

  struct Column
  {
    ObjectPtr key;
    ClassPtr type;
    VectorPtr data;
  };
  std::vector<Column> columns;

  typedef std::map<ObjectPtr, size_t> ColumnMap;
  ColumnMap columnMap;
};

typedef ReferenceCountedObjectPtr<Table> TablePtr;
extern ClassPtr tableClass;

}; /* namespace lbcpp */

#endif // !OIL_CORE_TABLE_H_
