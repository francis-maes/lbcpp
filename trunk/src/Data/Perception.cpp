/*-----------------------------------------.---------------------------------.
| Filename: Perception.cpp                 | Perception                      |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Perception.h>
#include <lbcpp/Data/Vector.h> // for DynamicObject
using namespace lbcpp;

void PerceptionCallback::sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
  {sense(variableNumber, subPerception->compute(input));}

TypePtr Perception::getOutputType() const
  {const_cast<Perception* >(this)->ensureTypeIsComputed(); return type;}

class SetInObjectPerceptionCallback : public PerceptionCallback
{
public:
  SetInObjectPerceptionCallback(ObjectPtr target)
    : target(target) {}

  virtual void sense(size_t variableNumber, const Variable& value)
    {target->setVariable(variableNumber, value);}

private:
  ObjectPtr target;
};

Variable Perception::computeFunction(const Variable& input, ErrorHandler& callback) const
{
  const_cast<Perception* >(this)->ensureTypeIsComputed();
  DynamicObjectPtr res = new DynamicObject(type);
  computePerception(input, PerceptionCallbackPtr(new SetInObjectPerceptionCallback(res)));
  return res;
}

void Perception::ensureTypeIsComputed()
{
  if (type)
    return;
  type = new DynamicClass(getClassName() + T("Class"));
  size_t n = getNumOutputVariables();
  for (size_t i = 0; i < n; ++i)
    type->addVariable(getOutputVariableType(i), getOutputVariableName(i));
}

/*
** CompositePerception
*/
size_t CompositePerception::getNumOutputVariables() const
  {return subPerceptions.size();}

TypePtr CompositePerception::getOutputVariableType(size_t index) const
{
  jassert(index < subPerceptions.size());
  return subPerceptions[index].second->getOutputType();
}

String CompositePerception::getOutputVariableName(size_t index) const
{
  jassert(index < subPerceptions.size());
  return subPerceptions[index].first;
}

PerceptionPtr CompositePerception::getOutputVariableGenerator(size_t index) const
{
  jassert(index < subPerceptions.size());
  return subPerceptions[index].second;
}

void CompositePerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  for (size_t i = 0; i < subPerceptions.size(); ++i)
    callback->sense(i, subPerceptions[i].second, input);
}

#include "Perception/PreprocessPerception.h"
#include "Perception/FlattenPerception.h"

PerceptionPtr Perception::addPreprocessor(FunctionPtr preProcessingFunction) const
  {return new PreprocessPerception(preProcessingFunction, PerceptionPtr(const_cast<Perception* >(this)));}

PerceptionPtr Perception::flatten() const
  {return new FlattenPerception(PerceptionPtr(const_cast<Perception* >(this)));}

#include "Perception/IdentityPerception.h"
#include "Perception/WindowPerception.h"

PerceptionPtr lbcpp::identityPerception(TypePtr type)
  {return new IdentityPerception(type);}

PerceptionPtr lbcpp::windowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception)
  {return new WindowPerception(elementsType, windowSize, subPerception);}

void declarePerceptionClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Perception, Function);

    LBCPP_DECLARE_CLASS(IdentityPerception, Perception);
    LBCPP_DECLARE_CLASS(WindowPerception, Perception);

    LBCPP_DECLARE_ABSTRACT_CLASS(CompositePerception, Perception);
  
    LBCPP_DECLARE_ABSTRACT_CLASS(DecoratorPerception, Perception);
      LBCPP_DECLARE_CLASS(PreprocessPerception, DecoratorPerception);
      LBCPP_DECLARE_CLASS(FlattenPerception, DecoratorPerception);
}
