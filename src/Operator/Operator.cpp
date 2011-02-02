/*-----------------------------------------.---------------------------------.
| Filename: Operator.cpp                   | Base class for Operators        |
| Author  : Francis Maes                   |                                 |
| Started : 01/02/2011 16:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Operator/Operator.h>
#include "Container/AccumulateContainerOperator.h"
#include "Container/DiscretizeContainerOperator.h"
#include "Container/SegmentContainerOperator.h"
using namespace lbcpp;

bool Operator::initialize(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables)
{
  this->inputVariables = inputVariables;
  this->outputVariable = initializeOperator(context);
  return this->outputVariable != VariableSignaturePtr();
}

bool Operator::checkNumInputs(ExecutionContext& context, size_t numInputs) const
{
  if (inputVariables.size() != numInputs)
  {
    context.errorCallback(T("Wrong number of inputs"));
    return false;
  }
  return true;
}

bool Operator::checkInputType(ExecutionContext& context, size_t index, TypePtr requestedType) const
{
  if (index >= inputVariables.size())
  {
    context.errorCallback(T("Missing input"));
    return false;
  }

  return context.checkInheritance(inputVariables[index]->getType(), requestedType);
}

TypePtr Operator::getTemplateArgument(ExecutionContext& context, TypePtr type, size_t templateArgumentIndex, TypePtr requestedType) const
{
  if (templateArgumentIndex >= type->getNumTemplateArguments())
  {
    context.errorCallback(T("Missing template argument"));
    return TypePtr();
  }
  TypePtr res = type->getTemplateArgument(templateArgumentIndex);
  if (requestedType && !context.checkInheritance(res, requestedType))
    return TypePtr();
  return res;
}

TypePtr Operator::getContainerElementsType(ExecutionContext& context, TypePtr type) const
{
  TypePtr t = type;
  while (t && (!t->getTemplate() || t->getTemplate()->getName() != T("Container")))
    t = t->getBaseType();
  if (!t)
  {
    context.errorCallback(type->getName() + T(" is not a container"));
    return false;
  }
  return getTemplateArgument(context, t, 0, anyType);
}

TypePtr Operator::getDistributionElementsType(ExecutionContext& context, TypePtr type) const
{
  TypePtr t = type;
  while (t && (!t->getTemplate() || t->getTemplate()->getName() != T("Distribution")))
    t = t->getBaseType();
  if (!t)
  {
    context.errorCallback(type->getName() + T(" is not a distribution"));
    return false;
  }
  return getTemplateArgument(context, t, 0, anyType);
}
