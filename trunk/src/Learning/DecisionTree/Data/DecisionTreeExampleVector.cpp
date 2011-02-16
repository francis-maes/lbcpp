/*-----------------------------------------.---------------------------------.
| Filename: DecisionTreeExampleVector.cpp  | Decision Tree Example Vector    |
| Author  : Julien Becker                  |                                 |
| Started : 18/01/2011 09:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "DecisionTreeExampleVector.h"

using namespace lbcpp;

bool DecisionTreeExampleVector::isAttributeConstant(size_t variableIndex) const
{
  if (getAttribute(0, variableIndex).isDouble())
    return isDoubleAttributeConstant(variableIndex);
  size_t n = indices.size();
  Variable constantValue;
  bool isConstantValueSet = false;
  for (size_t i = 0; i < n; ++i)
  {
    const Variable& value = getAttribute(i, variableIndex);
    jassert(!value.isNil());
    if (value.isMissingValue())
      continue;
    if (!isConstantValueSet)
    {
      constantValue = value;
      isConstantValueSet = true;
    }
    else if (constantValue != value)
      return false;
  }
  return true;
}

bool DecisionTreeExampleVector::isDoubleAttributeConstant(size_t variableIndex) const
{
  size_t n = indices.size();
  double minValue = DBL_MAX;
  double maxValue = -DBL_MAX;

  for (size_t i = 0; i < n; ++i)
  {
    const Variable& value = getAttribute(i, variableIndex);
    jassert(!value.isNil() && value.isDouble());
    if (value.isMissingValue())
      continue;
    double val = value.getDouble();
    if (val < minValue)
      minValue = val;
    if (val > maxValue)
      maxValue = val;
  }
  return maxValue - minValue <= 10e-9;
}

bool DecisionTreeExampleVector::isLabelConstant(Variable& constantValue) const
{
  size_t n = indices.size();
  constantValue = getLabel(0);
  jassert(constantValue.exists());
  for (size_t i = 1; i < n; ++i)
  {
    jassert(getLabel(i).exists());
    if (constantValue != getLabel(i))
      return false;
  }
  return true;
}
