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
# include "SparseVectorHelper.h"

namespace lbcpp
{

class SparseDoubleObjectVariableIterator : public Object::VariableIterator
{
public:
  SparseDoubleObjectVariableIterator(SparseDoubleObjectPtr object)
    : object(object), objectClass(object->getClass()), currentIndex(0) {}

  virtual bool exists() const
    {return currentIndex < object->values.size();}
  
  virtual Variable getCurrentVariable(size_t& index) const
  {
    std::pair<size_t, double>& entry = object->values[currentIndex];
    index = entry.first;
    return Variable(entry.second, objectClass->getMemberVariableType(entry.first));
  }

  virtual void next()
    {++currentIndex;}

private:
  ClassPtr objectClass;
  SparseDoubleObjectPtr object;
  size_t currentIndex;
};

SparseDoubleObject::SparseDoubleObject(DynamicClassPtr thisClass)
  : Object(thisClass), lastIndex(-1)  {}

String SparseDoubleObject::toShortString() const
  {return thisClass->getName() + T(" (actives: ") + String((int)values.size()) + T(" / ") + String((int)thisClass->getNumMemberVariables()) + T(")");}

Variable SparseDoubleObject::getVariable(size_t index) const
{
  TypePtr type = thisClass->getMemberVariableType(index); 
  const double* value = SparseDoubleVectorHelper::get(values, index);
  return value ? Variable(*value, type) : Variable::missingValue(type);
}

void SparseDoubleObject::setVariable(ExecutionContext& context, size_t index, const Variable& value)
{
  jassert(value.exists() && value.isDouble());
  if ((int)index > lastIndex)
  {
    appendValue(index, value.getDouble());
    return;
  }
  else
    SparseDoubleVectorHelper::set(values, index, value.getDouble());
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

void SparseDoubleObject::appendValuesWithShift(const std::vector<std::pair<size_t, double> >& subValues, size_t shift)
{
  values.reserve(values.size() + subValues.size());
  for (size_t i = 0; i < subValues.size(); ++i)
    appendValue(subValues[i].first + shift, subValues[i].second);
}

Object::VariableIterator* SparseDoubleObject::createVariablesIterator() const
  {return new SparseDoubleObjectVariableIterator(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_SPARSE_DOUBLE_H_
