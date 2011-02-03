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

bool Function::getContainerElementsType(ExecutionContext& context, TypePtr type, TypePtr& res) const
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Container"));
  if (!dvType)
  {
    context.errorCallback(type->getName() + T(" is not a Container"));
    return false;
  }
  jassert(dvType->getNumTemplateArguments() == 1);
  res = dvType->getTemplateArgument(0);
  return true;
}

bool Function::getDistributionElementsType(ExecutionContext& context, TypePtr type, TypePtr& res) const
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Distribution"));
  if (!dvType)
  {
    context.errorCallback(type->getName() + T(" is not a Distribution"));
    return false;
  }
  jassert(dvType->getNumTemplateArguments() == 1);
  res = dvType->getTemplateArgument(0);
  return true;
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
