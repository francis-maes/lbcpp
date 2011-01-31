/*-----------------------------------------.---------------------------------.
| Filename: Function.cpp                   | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 18:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Core/Function.h>
using namespace lbcpp;

bool Function::checkNumInputsEquals(ExecutionContext& context, size_t numInputs, size_t requestedNumInputs) const
{
  if (requestedNumInputs != numInputs)
  {
    context.errorCallback(T("Wrong number of inputs"));
    return false;
  }
  return true;
}

bool Function::checkType(ExecutionContext& context, const Variable& variable, TypePtr type) const
  {return context.checkInheritance(variable, type);}

bool Function::checkExistence(ExecutionContext& context, const Variable& variable) const
{
  if (!variable.exists())
  {
    context.errorCallback(T("Variable does not exists"));
    return false;
  }
  return true;
}

#include "../Function/Operator/AccumulateContainerOperator.h"
#include "../Function/Operator/DiscretizeContainerOperator.h"

namespace lbcpp
{

FunctionPtr accumulateOperator(TypePtr inputType)
{
  if (inputType->inheritsFrom(containerClass(anyType)))
  {
    TypePtr elementsType = inputType->getTemplateArgument(0); // FIXME: cast into container base class !!!!
    if (elementsType.dynamicCast<Enumeration>())
      return new AccumulateEnumerationContainerOperator(elementsType);
    else if (elementsType->inheritsFrom(doubleType))
      return new AccumulateDoubleContainerOperator();
    else if (elementsType->inheritsFrom(enumerationDistributionClass(anyType)))
    {
      EnumerationPtr enumeration = elementsType->getTemplateArgument(0).dynamicCast<Enumeration>();
      jassert(enumeration);
      return new AccumulateEnumerationDistributionContainerOperator(enumeration);
    }
  }

  return FunctionPtr();
}

FunctionPtr discretizeOperator(TypePtr inputType, bool sampleBest)
{
  if (inputType->inheritsFrom(containerClass(distributionClass(anyType))))
  {
    TypePtr elementsType = inputType->getTemplateArgument(0)->getTemplateArgument(0); // FIXME: cast into container base class !!!!
    return new DiscretizeContainerOperator(elementsType, sampleBest);
  }
  return FunctionPtr();
}

}; /* namespace lbcpp */
