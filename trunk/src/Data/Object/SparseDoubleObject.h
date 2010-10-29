/*-----------------------------------------.---------------------------------.
| Filename: SparseDoubleObject.h           | Sparse Double Object            |
| Author  : Francis Maes                   |                                 |
| Started : 29/10/2010 19:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_SPARSE_DOUBLE_H_
# define LBCPP_DATA_OBJECT_SPARSE_DOUBLE_H_

# include <lbcpp/Data/DynamicObject.h>
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class SparseDoubleObject : public Object
{
public:
  SparseDoubleObject(DynamicClassSharedPtr thisClass)
    : Object((Class* )thisClass.get()), thisClass(thisClass)  {}
  
  virtual Variable getVariable(size_t index) const
  {
    jassert(false); // not implemented yet
    return Variable();
  }

  virtual void setVariable(size_t index, const Variable& value)
  {
    if (value.exists() && (values.empty() || index > values.back().first))
    {
      double d = value.getDouble();
      if (d)
        values.push_back(std::make_pair(index, value.getDouble()));
      return;
    }
    // not implemented
    jassert(false);
  }

  VariableIterator* createVariablesIterator() const;

  std::vector< std::pair<size_t, double> >& getValues()
    {return values;}

  const std::vector< std::pair<size_t, double> >& getValues() const
    {return values;}

private:
  friend class SparseDoubleObjectVariableIterator;

  DynamicClassSharedPtr thisClass;
  std::vector< std::pair<size_t, double> > values;
};

typedef ReferenceCountedObjectPtr<SparseDoubleObject> SparseDoubleObjectPtr;

class SparseDoubleObjectVariableIterator : public Object::VariableIterator
{
public:
  SparseDoubleObjectVariableIterator(SparseDoubleObjectPtr object)
    : object(object), currentIndex(0) {}

  virtual bool exists() const
    {return currentIndex < object->values.size();}
  
  virtual Variable getCurrentVariable(size_t& index) const
  {
    std::pair<size_t, double>& entry = object->values[currentIndex];
    index = entry.first;
    return entry.second;
  }

  virtual void next()
    {++currentIndex;}

private:
  SparseDoubleObjectPtr object;
  size_t currentIndex;
};

inline Object::VariableIterator* SparseDoubleObject::createVariablesIterator() const
  {return new SparseDoubleObjectVariableIterator(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_SPARSE_DOUBLE_H_
