/*-----------------------------------------.---------------------------------.
| Filename: Function.cpp                   | Base class for Functions        |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 18:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Function/Function.h>
using namespace lbcpp;

bool Function::initialize(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables)
{
  this->inputVariables = inputVariables;
  this->outputVariable = initializeFunction(context);
  jassert(!this->outputVariable || getOutputType());
  return this->outputVariable != VariableSignaturePtr();
}

bool Function::checkNumInputs(ExecutionContext& context, size_t numInputs) const
{
  if (inputVariables.size() != numInputs)
  {
    context.errorCallback(T("Wrong number of inputs"));
    return false;
  }
  return true;
}

bool Function::checkInputType(ExecutionContext& context, size_t index, TypePtr requestedType) const
{
  if (index >= inputVariables.size())
  {
    context.errorCallback(T("Missing input"));
    return false;
  }

  return context.checkInheritance(inputVariables[index]->getType(), requestedType);
}

TypePtr Function::getTemplateArgument(ExecutionContext& context, TypePtr type, size_t templateArgumentIndex, TypePtr requestedType) const
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

TypePtr Function::getContainerElementsType(ExecutionContext& context, TypePtr type) const
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

TypePtr Function::getDistributionElementsType(ExecutionContext& context, TypePtr type) const
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

bool Function::checkExistence(ExecutionContext& context, const Variable& variable) const
{
  if (!variable.exists())
  {
    context.errorCallback(T("Variable does not exists"));
    return false;
  }
  return true;
}
