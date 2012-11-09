/*-----------------------------------------.---------------------------------.
| Filename: DataTable.h                    | Data Table                      |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 14:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_DATA_TABLE_H_
# define LBCPP_ML_DATA_TABLE_H_

# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Data/IndexSet.h>
# include "predeclarations.h"

namespace lbcpp
{

class DataTable : public Object
{
public:
  DataTable(size_t numSamples);
  DataTable() {}

  size_t getNumSamples() const
    {return allIndices->size();}
  
  size_t getNumColumns() const
    {return columns.size();}

  void addColumn(const ExpressionPtr& expression);

  void setSample(size_t rowIndex, size_t columnIndex, const ObjectPtr& value);
  ObjectPtr getSample(size_t rowIndex, size_t columnIndex) const;
  VectorPtr getSamples(const ExpressionPtr& expression) const;

  IndexSetPtr getAllIndices() const
    {return allIndices;}

private:
  IndexSetPtr allIndices;

  struct Column
  {
    ExpressionPtr expression;
    VectorPtr samples;
    SparseDoubleVectorPtr sortedDoubleValues;
  };
  std::vector<Column> columns;

  typedef std::map<ExpressionPtr, size_t> ExpressionMap;
  ExpressionMap columnMap;
};

typedef ReferenceCountedObjectPtr<DataTable> DataTablePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_DATA_TABLE_H_
