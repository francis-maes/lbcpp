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

struct SetInObjectPerceptionCallback : public PerceptionCallback
{
  SetInObjectPerceptionCallback(ObjectPtr target)
    : target(target), atLeastOneVariable(false) {}

  virtual void sense(size_t variableNumber, const Variable& value)
    {target->setVariable(variableNumber, value); atLeastOneVariable = true;}

  bool atLeastOneVariable;
  ObjectPtr target;
};

Variable Perception::computeFunction(const Variable& input, ErrorHandler& callback) const
{
  const_cast<Perception* >(this)->ensureTypeIsComputed();
  DynamicObjectPtr res = new DynamicObject(type);
  ReferenceCountedObjectPtr<SetInObjectPerceptionCallback> perceptionCallback(new SetInObjectPerceptionCallback(res));
  computePerception(input, perceptionCallback);
  return perceptionCallback->atLeastOneVariable ? res : Variable::missingValue(type);
}

void Perception::ensureTypeIsComputed()
{
  if (type)
    return;
  type = new DynamicClass(getClassName() + T("Class"), dynamicObjectClass());
  size_t n = getNumOutputVariables();
  for (size_t i = 0; i < n; ++i)
    type->addVariable(getOutputVariableType(i), getOutputVariableName(i));
}

/*
** ModifierPerception
*/
ModifierPerception::ModifierPerception(PerceptionPtr decorated)
  : DecoratorPerception(decorated)
  {}

TypePtr ModifierPerception::getOutputVariableType(size_t index) const
{
  PerceptionPtr modifiedPerception = getModifiedPerceptionCached(index);
  return modifiedPerception ? modifiedPerception->getOutputType() : DecoratorPerception::getOutputVariableType(index);
}

struct ModifierPerceptionCallback : public PerceptionCallback
{
  ModifierPerceptionCallback(PerceptionPtr targetRepresentation, PerceptionCallbackPtr targetCallback, const ModifierPerception* owner)
    : targetCallback(targetCallback), owner(owner)
    {}

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    PerceptionPtr modifiedPerception = owner->getModifiedPerceptionCached(variableNumber);
    if (modifiedPerception)
      targetCallback->sense(variableNumber, modifiedPerception, value);
    else
      targetCallback->sense(variableNumber, value);
  }

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
  {
    PerceptionPtr modifiedPerception = owner->getModifiedPerceptionCached(variableNumber);
    targetCallback->sense(variableNumber, modifiedPerception ? modifiedPerception : subPerception, input);
  }

private:
  PerceptionCallbackPtr targetCallback;
  const ModifierPerception* owner;
};

void ModifierPerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  PerceptionCallbackPtr decoratedVisitor(new ModifierPerceptionCallback(decorated, callback, this));
  DecoratorPerception::computePerception(input, decoratedVisitor);
}

PerceptionPtr ModifierPerception::getModifiedPerceptionCached(size_t index) const
{
  if (index < modifiedPerceptions.size())
    return modifiedPerceptions[index];
  ModifierPerception* pthis = const_cast<ModifierPerception* >(this);
  pthis->modifiedPerceptions.resize(index + 1);
  PerceptionPtr decoratedSubPerception = decorated->getOutputVariableGenerator(index);
  if (decoratedSubPerception)
  {
    ModifierPerceptionPtr mp = cloneAndCast<ModifierPerception>();
    mp->decorated = decoratedSubPerception;
    pthis->modifiedPerceptions[index] = mp;
  }
  else
    pthis->modifiedPerceptions[index] = getModifiedPerception(index, decorated->getOutputVariableType(index));
  return modifiedPerceptions[index];
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
