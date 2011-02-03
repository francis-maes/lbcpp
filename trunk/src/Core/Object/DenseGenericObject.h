/*-----------------------------------------.---------------------------------.
| Filename: DenseGenericObject.h           | Dense Generic Object            |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 23:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_DENSE_GENERIC_H_
# define LBCPP_DATA_OBJECT_DENSE_GENERIC_H_

# include <lbcpp/Core/DynamicObject.h>

namespace lbcpp
{

class DenseGenericObjectVariableIterator : public Object::VariableIterator
{
public:
  DenseGenericObjectVariableIterator(DenseGenericObjectPtr object)
    : object(object), current(0), n(object->variableValues.size()) {moveToNextActiveVariable();}

  virtual bool exists() const
    {return current < n;}
  
  virtual Variable getCurrentVariable(size_t& index) const
    {jassert(current < n); index = current; return currentValue;}

  virtual void next()
  {
    jassert(current < n);
    ++current;
    moveToNextActiveVariable();
  }

private:
  DenseGenericObjectPtr object;
  size_t current;
  Variable currentValue;
  size_t n;

  void moveToNextActiveVariable()
  {
    while (current < n)
    {
      TypePtr type = object->thisClass->getMemberVariableType(current);
      if (!type->isMissingValue(object->variableValues[current]))
      {
        currentValue = Variable::copyFrom(type, object->variableValues[current]);
        break;
      }
      ++current;
    }
  }
};

DenseGenericObject::DenseGenericObject(DynamicClassPtr thisClass)
  : Object(thisClass) {jassert(thisClass);}

DenseGenericObject::~DenseGenericObject()
{
  for (size_t i = 0; i < variableValues.size(); ++i)
    thisClass->getMemberVariableType(i)->destroy(variableValues[i]);
}

VariableValue& DenseGenericObject::getVariableValueReference(size_t index)
{
  jassert(index < thisClass->getNumMemberVariables());
  if (variableValues.size() <= index)
  {
    size_t i = variableValues.size();
    variableValues.resize(index + 1);
    while (i < variableValues.size())
    {
      variableValues[i] = thisClass->getMemberVariableType(i)->getMissingValue();
      ++i;
    }
  }
  return variableValues[index];
}

Variable DenseGenericObject::getVariable(size_t index) const
{
  TypePtr type = thisClass->getMemberVariableType(index);
  if (index < variableValues.size())
    return Variable::copyFrom(type, variableValues[index]);
  else
    return Variable::missingValue(type);
}

void DenseGenericObject::setVariable(size_t index, const Variable& value)
  {value.copyTo(getVariableValueReference(index));}

Object::VariableIterator* DenseGenericObject::createVariablesIterator() const
  {return new DenseGenericObjectVariableIterator(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_DENSE_GENERIC_H_
