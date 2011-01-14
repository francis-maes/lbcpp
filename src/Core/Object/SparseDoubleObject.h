/*-----------------------------------------.---------------------------------.
| Filename: SparseDoubleObject.h           | Sparse Double Object            |
| Author  : Francis Maes                   |                                 |
| Started : 29/10/2010 19:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_SPARSE_DOUBLE_H_
# define LBCPP_DATA_OBJECT_SPARSE_DOUBLE_H_

# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

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

SparseDoubleObject::SparseDoubleObject(DynamicClassSharedPtr thisClass)
  : Object((Class* )thisClass.get()), thisClass(thisClass), lastIndex(-1)  {}

String SparseDoubleObject::toShortString() const
  {return thisClass->getName() + T(" (actives: ") + String((int)values.size()) + T(" / ") + String((int)thisClass->getObjectNumVariables()) + T(")");}

Variable SparseDoubleObject::getVariable(size_t index) const
{
  jassert(false); // not implemented yet
  return Variable();
}

void SparseDoubleObject::setVariable(ExecutionContext& context, size_t index, const Variable& value)
{
  jassert(value.exists() && value.isDouble());
  if ((int)index > lastIndex)
  {
    appendValue(index, value.getDouble());
    return;
  }
  // not implemented
  jassert(false);
}

void SparseDoubleObject::appendValue(size_t index, double value)
{
  jassert((int)index > lastIndex);
  if (value)
  {
    values.push_back(std::make_pair(index, value));
    lastIndex = (int)index;
  }
}

Object::VariableIterator* SparseDoubleObject::createVariablesIterator() const
  {return new SparseDoubleObjectVariableIterator(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_SPARSE_DOUBLE_H_
