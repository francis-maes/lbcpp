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
