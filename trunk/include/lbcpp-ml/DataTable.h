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
  
/*
** DataVector
*/
class DataVector;
typedef ReferenceCountedObjectPtr<DataVector> DataVectorPtr;

class DataVector : public Object
{
public:
  enum Implementation
  {
    constantValueImpl = 0,
    ownedVectorImpl,
    cachedVectorImpl,
    noImpl
  };

  DataVector(Implementation implementation, const IndexSetPtr& indices, const TypePtr& elementsType);
  DataVector(const IndexSetPtr& indices, const VectorPtr& ownedVector);
  DataVector();

  static DataVectorPtr createConstant(IndexSetPtr indices, const ObjectPtr& constantValue);
  static DataVectorPtr createCached(IndexSetPtr indices, const VectorPtr& cachedVector);
  
  const TypePtr& getElementsType() const
    {return elementsType;}

  struct const_iterator
  {
    typedef std::vector<size_t>::const_iterator index_iterator;

    const_iterator(const DataVector* owner, size_t position, index_iterator it)
      : owner(owner), position(position), it(it) {}
    const_iterator(const const_iterator& other)
      : owner(other.owner), position(other.position), it(other.it) {}
    const_iterator() : owner(NULL), position(0) {}

    const_iterator& operator =(const const_iterator& other)
      {owner = other.owner; position = other.position; it = other.it; return *this;}

    const_iterator& operator ++()
    {
      ++position;
      ++it;
      return *this;
    }

    inline Variable operator *() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawObject;
      case ownedVectorImpl: return owner->vector->getElement(position);
      case cachedVectorImpl: return owner->vector->getElement(*it);
      default: jassert(false); return Variable();
      }
    }

    inline unsigned char getRawBoolean() const
    {
      if (owner->implementation == constantValueImpl)
        return owner->constantRawBoolean;
      size_t index = (owner->implementation == ownedVectorImpl ? position : *it);
      if (owner->elementsType == booleanType)
        return owner->vector.staticCast<BooleanVector>()->getData()[index];
      else if (owner->elementsType == probabilityType)
      {
        double value = owner->vector.staticCast<DenseDoubleVector>()->getValue(index);
        return value == doubleMissingValue ? 2 : (value > 0.5 ? 1 : 0);
      }
      else
      {
        jassert(owner->elementsType == doubleType);
        double value = owner->vector.staticCast<DenseDoubleVector>()->getValue(index);
        return value == doubleMissingValue ? 2 : (value > 0 ? 1 : 0);
      }
    }

    inline int getRawInteger() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return (int)NewInteger::get(owner->constantRawObject);
      case ownedVectorImpl: return owner->vector->getElement(position).getInteger();
      case cachedVectorImpl: return owner->vector->getElement(*it).getInteger();
      default: jassert(false); return 0;
      }
    }

    inline double getRawDouble() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawDouble;
      case ownedVectorImpl: return owner->vector.staticCast<DenseDoubleVector>()->getValue(position);
      case cachedVectorImpl: return owner->vector.staticCast<DenseDoubleVector>()->getValue(*it);
      default: jassert(false); return 0.0;
      }
    }

    inline const ObjectPtr& getRawObject() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawObject;
      case ownedVectorImpl: return owner->vector.staticCast<ObjectVector>()->get(position);
      case cachedVectorImpl: return owner->vector.staticCast<ObjectVector>()->get(*it);
      default: jassert(false); static ObjectPtr empty; return empty;
      }
    }

    bool operator ==(const const_iterator& other) const
      {return owner == other.owner && position == other.position;}

    bool operator !=(const const_iterator& other) const
      {return owner != other.owner || position != other.position;}

    size_t getIndex() const
      {return *it;}

  private:
    friend class DataVector;

    const DataVector* owner;
    size_t position;
    index_iterator it;
  };

  const_iterator begin() const
    {return const_iterator(this, 0, indices->begin());}

  const_iterator end() const
    {return const_iterator(this, indices->size(), indices->end());}

  size_t size() const
    {return indices->size();}

  const IndexSetPtr& getIndices() const
    {return indices;}

  Implementation getImplementation() const
    {return implementation;}

  const VectorPtr& getVector() const
    {return vector;}

  Variable sampleElement(RandomGeneratorPtr random) const;

protected:
  Implementation implementation;
  IndexSetPtr indices;
  TypePtr elementsType;

  unsigned char constantRawBoolean;
  double constantRawDouble;
  ObjectPtr constantRawObject;

  VectorPtr vector;       // ownedVectorImpl and cachedVectorImpl
};

typedef ReferenceCountedObjectPtr<DataVector> DataVectorPtr;

/*
** DataTable
*/
class DataTable : public Object
{
public:
  DataTable(size_t numSamples);
  DataTable() {}

  size_t getNumSamples() const
    {return allIndices->size();}
  
  size_t getNumColumns() const
    {return columns.size();}

  String getColumnName(size_t index) const;

  ExpressionPtr getExpression(size_t index) const
    {jassert(index < columns.size()); return columns[index].expression;}

  VectorPtr getSamples(size_t index) const
    {jassert(index < columns.size()); return columns[index].samples;}

  VectorPtr getSamplesByExpression(const ExpressionPtr& expression) const;

  void addColumn(const ExpressionPtr& expression);

  void setSample(size_t rowIndex, size_t columnIndex, const ObjectPtr& value);
  ObjectPtr getSample(size_t rowIndex, size_t columnIndex) const;

  IndexSetPtr getAllIndices() const
    {return allIndices;}

  void makeOrder(size_t columnIndex, bool increasingOrder, std::vector<size_t>& res) const;

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
