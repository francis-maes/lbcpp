/*-----------------------------------------.---------------------------------.
| Filename: Perception.cpp                 | Perception                      |
| Author  : Francis Maes                   |                                 |
| Started : 12/07/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Perception/Perception.h>
using namespace lbcpp;

/*
** PerceptionCallback
*/
struct SetInObjectPerceptionCallback : public PerceptionCallback
{
  SetInObjectPerceptionCallback(ObjectPtr target)
    : target(target), atLeastOneVariable(false) {}

  virtual void sense(size_t variableNumber, const Variable& value)
    {jassert(value.exists()); target->setVariable(variableNumber, value); atLeastOneVariable = true;}

  ObjectPtr target;
  bool atLeastOneVariable;
};

void PerceptionCallback::sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
{
  jassert(subPerception);
  Variable variable = subPerception->compute(input);
  if (variable.exists())
    sense(variableNumber, variable.getObject());
}

/*
** Perception
*/
String Perception::classNameToOutputClassName(const String& className)
{
  String res;
  for (int i = 0; i < className.length(); ++i)
  {
    if (i > 0 && juce::CharacterFunctions::isLowerCase(className[i-1]) && juce::CharacterFunctions::isUpperCase(className[i]))
      res += ' ';
    res += juce::CharacterFunctions::toLowerCase(className[i]);
  }
  return res;
}

String Perception::toString() const
  {return classNameToOutputClassName(getClassName());}

TypePtr Perception::getOutputType() const
{
  if (!outputType)
  {
    const_cast<Perception* >(this)->computeOutputType();
    jassert(outputType);
  }
  return outputType.get();
}

void Perception::clearOutputVariables()
{
  outputVariables.clear();
  if (outputType)
  {
    outputType->clearVariables();
  }
}

void Perception::addOutputVariable(TypePtr type, const String& name, PerceptionPtr subPerception)
{
  OutputVariable v;
  v.type = type;
  v.name = name;
  v.subPerception = subPerception;
  outputVariables.push_back(v);
  if (outputType)
    outputType->addVariable(type, name);
}

Variable Perception::computeFunction(const Variable& input, MessageCallback& callback) const
{
  TypePtr outputType = getOutputType();
  
  // create empty sparse or dense object
  ObjectPtr res;
  if (isSparse())
  {
    DynamicClassPtr dynamicClassOutputType = outputType.dynamicCast<DynamicClass>();
    if (dynamicClassOutputType)
      res = dynamicClassOutputType->createSparseObject();
  }
  if (!res)
  {
    res = Object::create(outputType);
    jassert(res);
  }

  // compute perception
  SetInObjectPerceptionCallback perceptionCallback(res);
  computePerception(input, &perceptionCallback);
  return perceptionCallback.atLeastOneVariable ? Variable(res) : Variable::missingValue(outputType);
}

void Perception::computeOutputType()
{
  if (!outputType)
    outputType = new UnnamedDynamicClass();
  if (!outputType->getBaseType())
  {
    outputType->setName(toString());
    outputType->setBaseType(objectClass);
    size_t n = outputVariables.size();
    for (size_t i = 0; i < n; ++i)
    {
      const OutputVariable& v = outputVariables[i];
      outputType->addVariable(v.type, v.name);
    }
    outputType->initialize(MessageCallback::getInstance());
  }
}

bool Perception::loadFromXml(XmlImporter& importer)
{
  if (!Function::loadFromXml(importer))
    return false;
  outputVariables.clear();
  computeOutputType();
  //DBG("Perception::loadFromXml: " + toString() + T(", num outputs = ") + String(outputVariables.size()));
  jassert(!outputVariables.size() || getOutputType()->getObjectNumVariables());
  return true;
}

/*
** CompositePerception
*/
CompositePerception::CompositePerception(TypePtr inputType, const String& stringDescription)
  : inputType(inputType), stringDescription(stringDescription),
    subPerceptions(vector(pairClass(stringType, perceptionClass)))
{
}

size_t CompositePerception::getNumPerceptions() const
  {return outputVariables.size();}

String CompositePerception::getPerceptionName(size_t index) const
  {return outputVariables[index].name;}

PerceptionPtr CompositePerception::getPerception(size_t index) const
  {return outputVariables[index].subPerception;}

void CompositePerception::addPerception(const String& name, PerceptionPtr subPerception)
{
  if (checkInheritance(getInputType(), subPerception->getInputType()))
  {
    addOutputVariable(name, subPerception);
    subPerceptions->append(Variable::pair(name, subPerception));
  }
}

void CompositePerception::computePerception(const Variable& input, PerceptionCallbackPtr callback) const
{
  for (size_t i = 0; i < outputVariables.size(); ++i)
    callback->sense(i, outputVariables[i].subPerception, input);
}

void CompositePerception::computeOutputType()
{
  outputVariables.resize(subPerceptions->getNumElements());
  for (size_t i = 0; i < outputVariables.size(); ++i)
  {
    Variable nameAndSubPerception = subPerceptions->getElement(i);
    OutputVariable& v = outputVariables[i];
    v.name = nameAndSubPerception[0].getString();
    v.subPerception = nameAndSubPerception[1].getObjectAndCast<Perception>();
    v.type = v.subPerception->getOutputType();
  }
  Perception::computeOutputType();
}

/*
** Constructor functions
*/
namespace lbcpp
{
  extern PerceptionPtr productWithVariablePerception(FunctionPtr multiplyFunction, PerceptionPtr perception, TypePtr variableType, bool swapVariables);
};

PerceptionPtr lbcpp::identityPerception()
{
  static PerceptionPtr identity = identityPerception(anyType);
  return identity;
}

PerceptionPtr lbcpp::productPerception(FunctionPtr multiplyFunction, PerceptionPtr perception1, TypePtr type2)
  {return productWithVariablePerception(multiplyFunction, perception1, type2, false);}

PerceptionPtr lbcpp::productPerception(FunctionPtr multiplyFunction, TypePtr type1, PerceptionPtr perception2)
  {return productWithVariablePerception(multiplyFunction, perception2, type1, true);}

