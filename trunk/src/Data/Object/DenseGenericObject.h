/*-----------------------------------------.---------------------------------.
| Filename: DenseGenericObject.h           | Dense Generic Object            |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 23:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_DENSE_GENERIC_H_
# define LBCPP_DATA_OBJECT_DENSE_GENERIC_H_

# include <lbcpp/Data/DynamicObject.h>

namespace lbcpp
{

class DenseGenericObject : public DynamicObject
{
public:
  DenseGenericObject(TypePtr thisType)
    : DynamicObject(thisType) {}
  virtual ~DenseGenericObject()
  {
    for (size_t i = 0; i < variableValues.size(); ++i)
      thisClass->getObjectVariableType(i)->destroy(variableValues[i]);
  }

  VariableValue& getVariableValueReference(size_t index)
  {
    jassert(index < thisClass->getObjectNumVariables());
    if (variableValues.size() <= index)
    {
      size_t i = variableValues.size();
      variableValues.resize(index + 1);
      while (i < variableValues.size())
      {
        variableValues[i] = thisClass->getObjectVariableType(i)->getMissingValue();
        ++i;
      }
    }
    return variableValues[index];
  }

  virtual Variable getVariableImpl(size_t index) const
  {
    TypePtr type = thisClass->getObjectVariableType(index);
    if (index < variableValues.size())
      return Variable::copyFrom(type, variableValues[index]);
    else
      return Variable::missingValue(type);
  }

  virtual void setVariableImpl(size_t index, const Variable& value)
    {value.copyTo(getVariableValueReference(index));}

private:
  std::vector<VariableValue> variableValues;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_DENSE_GENERIC_H_
