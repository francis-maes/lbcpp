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

#include "../Function/Function/AccumulateContainerFunction.h"

namespace lbcpp
{

FunctionPtr accumulateFunction(TypePtr inputType)
{
  if (inputType->inheritsFrom(containerClass(anyType)))
  {
    TypePtr elementsType = inputType->getTemplateArgument(0); // FIXME: cast into container base class !!!!
    if (elementsType.dynamicCast<Enumeration>())
      return new AccumulateEnumerationContainerFunction(elementsType);
    else if (elementsType->inheritsFrom(doubleType))
      return new AccumulateDoubleContainerFunction();
    else if (elementsType->inheritsFrom(enumerationDistributionClass(anyType)))
    {
      EnumerationPtr enumeration = elementsType->getTemplateArgument(0).dynamicCast<Enumeration>();
      jassert(enumeration);
      return new AccumulateEnumerationDistributionContainerFunction(enumeration);
    }
  }

  return FunctionPtr();
}

}; /* namespace lbcpp */