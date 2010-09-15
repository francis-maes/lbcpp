/*-----------------------------------------.---------------------------------.
| Filename: Perception.cpp                 | Perception                      |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/Perception.h>
using namespace lbcpp;

void PerceptionCallback::sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
{
  Variable variable = subPerception->compute(input);
  if (variable)
    sense(variableNumber, variable);
}

TypePtr Perception::getOutputType() const
  {const_cast<Perception* >(this)->ensureTypeIsComputed(); return outputType;}

struct SetInObjectPerceptionCallback : public PerceptionCallback
{
  SetInObjectPerceptionCallback(ObjectPtr target)
    : target(target), atLeastOneVariable(false) {}

  virtual void sense(size_t variableNumber, const Variable& value)
    {jassert(value); target->setVariable(variableNumber, value); atLeastOneVariable = true;}

  ObjectPtr target;
  bool atLeastOneVariable;
};

Variable Perception::computeFunction(const Variable& input, MessageCallback& callback) const
{
  TypePtr outputType = getOutputType();
  Variable res = Variable::create(outputType);
  ReferenceCountedObjectPtr<SetInObjectPerceptionCallback> perceptionCallback(new SetInObjectPerceptionCallback(res.getObject()));
  computePerception(input, perceptionCallback);
  return perceptionCallback->atLeastOneVariable ? res : Variable::missingValue(outputType);
}

void Perception::ensureTypeIsComputed()
{
  if (outputType)
    return;
  DefaultClassPtr outputType = new DynamicClass(getPreferedOutputClassName());
  size_t n = getNumOutputVariables();
  for (size_t i = 0; i < n; ++i)
    outputType->addVariable(getOutputVariableType(i), getOutputVariableName(i));
  outputType->initialize(MessageCallback::getInstance());
  this->outputType = outputType;
}

PerceptionPtr Perception::addPreprocessor(FunctionPtr preProcessingFunction) const
  {return preprocessPerception(preProcessingFunction, refCountedPointerFromThis(this));}

PerceptionPtr Perception::flatten() const
  {return flattenPerception(refCountedPointerFromThis(this));}

/*
** CompositePerception
*/
CompositePerception::CompositePerception()
  : subPerceptions(vector(pairType(stringType(), perceptionClass())))
{
}

size_t CompositePerception::getNumPerceptions() const
  {return subPerceptions->getNumElements();}

String CompositePerception::getPerceptionName(size_t index) const
  {return subPerceptions->getElement(index)[0].getString();}

PerceptionPtr CompositePerception::getPerception(size_t index) const
  {return subPerceptions->getElement(index)[1].getObjectAndCast<Perception>();}

void CompositePerception::addPerception(const String& name, PerceptionPtr subPerception)
  {subPerceptions->append(Variable::pair(name, subPerception));}

void CompositePerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  for (size_t i = 0; i < getNumPerceptions(); ++i)
    callback->sense(i, getPerception(i), input);
}

PerceptionPtr lbcpp::identityPerception()
{
  static PerceptionPtr identity = identityPerception(anyType());
  return identity;
}