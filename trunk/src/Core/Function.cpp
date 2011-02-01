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
