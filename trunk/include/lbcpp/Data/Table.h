/*-----------------------------------------.---------------------------------.
| Filename: Table.h                        | Table                           |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 20:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_TABLE_H_
# define LBCPP_DATA_TABLE_H_

# include "../Core/Vector.h"
# include "IndexSet.h"

namespace lbcpp
{
  
class Table : public Object
{
public:
  Table(size_t numSamples);
  Table() {}

  void addColumn(const ObjectPtr& key, const TypePtr& type);
  void resize(size_t numRows);

  size_t getNumColumns() const
    {return columns.size();}

  size_t getNumRows() const
    {return allIndices ? allIndices->size() : 0;}

  String getDescription(size_t index) const;

  ObjectPtr getKey(size_t index) const
    {jassert(index < columns.size()); return columns[index].key;}

  TypePtr getType(size_t index) const
    {jassert(index < columns.size()); return columns[index].type;}

  VectorPtr getData(size_t index) const
    {jassert(index < columns.size()); return columns[index].data;}

  int findColumnByKey(const ObjectPtr& key) const;
  VectorPtr getDataByKey(const ObjectPtr& key) const;

  ObjectPtr getElement(size_t rowIndex, size_t columnIndex) const;
  void setElement(size_t rowIndex, size_t columnIndex, const ObjectPtr& value);

  IndexSetPtr getAllIndices() const
    {return allIndices;}

  void makeOrder(size_t columnIndex, bool increasingOrder, std::vector<size_t>& res) const;

private:
  IndexSetPtr allIndices;

  struct Column
  {
    ObjectPtr key;
    TypePtr type;
    VectorPtr data;
    SparseDoubleVectorPtr sortedDoubleValues;
  };
  std::vector<Column> columns;

  typedef std::map<ObjectPtr, size_t> ColumnMap;
  ColumnMap columnMap;
};

typedef ReferenceCountedObjectPtr<Table> TablePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_TABLE_H_
