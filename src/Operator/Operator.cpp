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

bool Operator::initialize(ExecutionContext& context, const std::vector<TypePtr>& inputs)
{
  this->inputTypes = inputs;
  this->outputType = initializeOperator(context);
  return this->outputType != TypePtr();
}

bool Operator::checkNumInputsEquals(ExecutionContext& context, size_t numInputs) const
{
  if (inputTypes.size() != numInputs)
  {
    context.errorCallback(T("Wrong number of inputs"));
    return false;
  }
  return true;
}

bool Operator::checkInputType(ExecutionContext& context, size_t index, TypePtr requestedType) const
{
  if (index >= inputTypes.size())
  {
    context.errorCallback(T("Missing input"));
    return false;
  }

  return context.checkInheritance(inputTypes[index], requestedType);
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

namespace lbcpp
{

OperatorPtr accumulateOperator(TypePtr inputType)
{
  if (inputType->inheritsFrom(containerClass(anyType)))
  {
    TypePtr elementsType = inputType->getTemplateArgument(0); // FIXME: cast into container base class !!!!
    if (elementsType.dynamicCast<Enumeration>())
      return new AccumulateEnumerationContainerOperator();
    else if (elementsType->inheritsFrom(doubleType))
      return new AccumulateDoubleContainerOperator();
    else if (elementsType->inheritsFrom(enumerationDistributionClass(anyType)))
      return new AccumulateEnumerationDistributionContainerOperator();
  }
  return OperatorPtr();
}

OperatorPtr discretizeOperator(TypePtr inputType, bool sampleBest)
{
  if (inputType->inheritsFrom(containerClass(distributionClass(anyType))))
    return new DiscretizeContainerOperator(sampleBest);
  return OperatorPtr();
}

OperatorPtr segmentOperator(TypePtr inputType)
{
  if (inputType->inheritsFrom(containerClass(anyType)))
    return new SegmentContainerOperator();
  return OperatorPtr();
}

}; /* namespace lbcpp */
