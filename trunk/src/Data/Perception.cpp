/*-----------------------------------------.---------------------------------.
| Filename: Perception.cpp                 | Perception                      |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Perception.h>
#include <lbcpp/Data/Vector.h> // for VariableVector
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

Variable Perception::computeFunction(const Variable& input, ErrorHandler& callback) const
{
  TypePtr outputType = getOutputType();
  ObjectPtr object = Variable::create(outputType).getObject();
  ReferenceCountedObjectPtr<SetInObjectPerceptionCallback> perceptionCallback(new SetInObjectPerceptionCallback(object));
  computePerception(input, perceptionCallback);
  return perceptionCallback->atLeastOneVariable ? object : Variable::missingValue(outputType);
}

/////////////////////////////////////
class DynamicObject : public Object
{
public:
  DynamicObject(TypePtr thisType)
    : Object(thisType) {}
  
  virtual ~DynamicObject()
  {
    for (size_t i = 0; i < variableValues.size(); ++i)
      thisClass->getObjectVariableType(i)->destroy(variableValues[i]);
  }

  VariableValue& operator[](size_t index)
  {
    jassert(index < thisClass->getObjectNumVariables());
    if (variableValues.size() <= index)
    {
      size_t i = variableValues.size();
      variableValues.resize(index + 1);
      while (i < variableValues.size())
      {
        variableValues[i] = thisClass->getObjectVariableType(i)->getMissingValue();
        ++i;
      }
    }
    return variableValues[index];
  }

private:
  std::vector<VariableValue> variableValues;
};

typedef ReferenceCountedObjectPtr<DynamicObject> DynamicObjectPtr;

class DynamicClass : public DefaultClass
{
public:
  DynamicClass(const String& name, TypePtr baseClass = objectClass())
    : DefaultClass(name, baseClass) {}

  virtual VariableValue create() const
    {return new DynamicObject(refCountedPointerFromThis(this));}

  virtual Variable getObjectVariable(const VariableValue& value, size_t index) const
  {
    DynamicObjectPtr object = value.getObjectAndCast<DynamicObject>();
    jassert(object);
    return Variable::copyFrom(getObjectVariableType(index), (*object)[index]);
  }

  virtual void setObjectVariable(const VariableValue& value, size_t index, const Variable& subValue) const
  {
    jassert(subValue.getType()->inheritsFrom(getObjectVariableType(index)));
    DynamicObjectPtr object = value.getObjectAndCast<DynamicObject>();
    jassert(object);
    subValue.copyTo((*object)[index]);
  }
};
//////////////////////////////////

void Perception::ensureTypeIsComputed()
{
  if (outputType)
    return;
  outputType = new DynamicClass(getClassName() + T("Class"));
  size_t n = getNumOutputVariables();
  for (size_t i = 0; i < n; ++i)
    outputType->addVariable(getOutputVariableType(i), getOutputVariableName(i));
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
