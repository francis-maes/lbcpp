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
CompositePerception::CompositePerception()
  : subPerceptions(new Vector(pairType(stringType(), perceptionClass())))
{
}

size_t CompositePerception::getNumPerceptions() const
  {return subPerceptions->size();}

String CompositePerception::getPerceptionName(size_t index) const
  {return subPerceptions->getVariable(index)[0].getString();}

PerceptionPtr CompositePerception::getPerception(size_t index) const
  {return subPerceptions->getVariable(index)[1].getObjectAndCast<Perception>();}

void CompositePerception::addPerception(const String& name, PerceptionPtr subPerception)
  {subPerceptions->append(Variable::pair(name, subPerception));}

void CompositePerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  for (size_t i = 0; i < getNumPerceptions(); ++i)
    callback->sense(i, getPerception(i), input);
}

/*
** Class constructors
*/
#include "Perception/PreprocessPerception.h"
#include "Perception/FlattenPerception.h"

PerceptionPtr Perception::addPreprocessor(FunctionPtr preProcessingFunction) const
  {return new PreprocessPerception(preProcessingFunction, PerceptionPtr(const_cast<Perception* >(this)));}

PerceptionPtr Perception::flatten() const
  {return new FlattenPerception(PerceptionPtr(const_cast<Perception* >(this)));}

#include "Perception/IdentityPerception.h"
#include "Perception/WindowPerception.h"
#include "Perception/FunctionBasedPerception.h"

PerceptionPtr lbcpp::identityPerception(TypePtr type)
  {return new IdentityPerception(type);}

PerceptionPtr lbcpp::windowPerception(TypePtr elementsType, size_t windowSize, PerceptionPtr subPerception)
  {return new WindowPerception(elementsType, windowSize, subPerception);}

PerceptionPtr lbcpp::functionBasedPerception(FunctionPtr function)
  {return new FunctionBasedPerception(function);}

/*
** Class declarations
*/
ClassPtr lbcpp::perceptionClass()
  {static TypeCache cache(T("Perception")); return cache();}

namespace lbcpp
{
  class CompositePerceptionClass : public DynamicClass
  {
  public:
    CompositePerceptionClass() : DynamicClass(T("CompositePerception"), perceptionClass())
    {
      addVariable(vectorClass(pairType(stringType(), perceptionClass())), T("subPerceptions"));
    }

    LBCPP_DECLARE_VARIABLE_BEGIN(CompositePerception)
      LBCPP_DECLARE_VARIABLE(subPerceptions);
    LBCPP_DECLARE_VARIABLE_END()
  };

  class DecoratorPerceptionClass : public DynamicClass
  {
  public:
    DecoratorPerceptionClass() : DynamicClass(T("DecoratorPerception"), perceptionClass())
    {
      addVariable(perceptionClass(), T("decorated"));
    }

    LBCPP_DECLARE_VARIABLE_BEGIN(DecoratorPerception)
      LBCPP_DECLARE_VARIABLE(decorated);
    LBCPP_DECLARE_VARIABLE_END()
  };

};

ClassPtr lbcpp::compositePerceptionClass()
  {static TypeCache cache(T("CompositePerception")); return cache();}

ClassPtr lbcpp::decoratorPerceptionClass()
  {static TypeCache cache(T("DecoratorPerception")); return cache();}

void declarePerceptionClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Perception, Function);

    Class::declare(new IdentityPerceptionClass());
    Class::declare(new FunctionBasedPerceptionClass());

    Class::declare(new CompositePerceptionClass());
  
    Class::declare(new DecoratorPerceptionClass());
      Class::declare(new PreprocessPerceptionClass());
      LBCPP_DECLARE_CLASS(FlattenPerception, DecoratorPerception);
      Class::declare(new WindowPerceptionClass());
}
