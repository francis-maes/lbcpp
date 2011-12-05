/*-----------------------------------------.---------------------------------.
| Filename: LuapeNode.cpp                  | Luape Graph Node                |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "LuapeNode.h"
#include <algorithm>
using namespace lbcpp;

/*
** LuapeNode
*/
static size_t makeLuapeNodeAllocationIndex()
{
  static size_t res = 0;
  return res++; // warning: not safe for multi-threading ...
}

LuapeNode::LuapeNode(const TypePtr& type)
  : type(type), allocationIndex(makeLuapeNodeAllocationIndex())
{
}

/*
** LuapeInputNode
*/
LuapeInputNode::LuapeInputNode(const TypePtr& type, const String& name)
  : LuapeNode(type), name(name) {}

String LuapeInputNode::toShortString() const
  {return name;}

Variable LuapeInputNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  jassert(false); // the value should already have been cached
  return Variable();
}

VectorPtr LuapeInputNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const
{
  jassert(false); // the value should already have been cached
  return VectorPtr();
}

/*
** LuapeConstantNode
*/
LuapeConstantNode::LuapeConstantNode(const Variable& value)
  : LuapeNode(value.getType()), value(value)
{
}

String LuapeConstantNode::toShortString() const
  {return value.toShortString();}

Variable LuapeConstantNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
  {return value;}

VectorPtr LuapeConstantNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const
{
  size_t n = cache->getNumSamples();
  if (type == doubleType)
    return new DenseDoubleVector(n, value.getDouble());
  else
  {
    VectorPtr res = vector(type, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, value);
    return res;
  }
}

/*
** LuapeFunctionNode
*/
LuapeFunctionNode::LuapeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& arguments)
  : function(function), arguments(arguments)
{
  initialize();
}

LuapeFunctionNode::LuapeFunctionNode(const LuapeFunctionPtr& function, LuapeNodePtr argument)
  : function(function), arguments(1, argument)
{
  initialize();
}

String LuapeFunctionNode::toShortString() const
  {return function->toShortString(arguments);}

void LuapeFunctionNode::initialize()
{
  size_t numInputs = function->getNumInputs();
  jassert(arguments.size() == numInputs);
  std::vector<TypePtr> inputTypes(numInputs);
  for (size_t i = 0; i < numInputs; ++i)
  {
    jassert(arguments[i]);
    inputTypes[i] = arguments[i]->getType();
    jassert(function->doAcceptInputType(i, inputTypes[i]));
  }

  type = function->getOutputType(inputTypes);
}

Variable LuapeFunctionNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  std::vector<Variable> inputValues(arguments.size());
  for (size_t i = 0; i < inputValues.size(); ++i)
    inputValues[i] = cache->compute(context, arguments[i]);
  return function->compute(context, &inputValues[0]);
}

VectorPtr LuapeFunctionNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const
{
  std::vector<VectorPtr> inputs(arguments.size());
  for (size_t i = 0; i < inputs.size(); ++i)
    inputs[i] = cache->compute(context, arguments[i]);
  return function->compute(context, inputs, type);
}

/*
** LuapeTestNode
*/
LuapeTestNode::LuapeTestNode(const LuapeNodePtr& conditionNode, const LuapeNodePtr& successNode, const LuapeNodePtr& failureNode, const LuapeNodePtr& missingNode)
  : LuapeNode(successNode->getType()), conditionNode(conditionNode), successNode(successNode), failureNode(failureNode), missingNode(missingNode)
{
}

size_t LuapeTestNode::getNumSubNodes() const
  {return 4;}
  
const LuapeNodePtr& LuapeTestNode::getSubNode(size_t index) const
{
  switch (index)
  {
  case 0: return conditionNode;
  case 1: return successNode;
  case 2: return failureNode;
  case 3: return missingNode;
  };
  jassert(false);
  return conditionNode;
}

String LuapeTestNode::toShortString() const
{
  return "(" + conditionNode->toShortString() + " ? " + 
          successNode->toShortString() + T(" : ") + 
          failureNode->toShortString() + T(" : ") + 
          missingNode->toShortString() + T(")");
}

Variable LuapeTestNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  Variable condition = conditionNode->compute(context, cache);
  if (condition.isMissingValue())
    return missingNode->compute(context, cache);
  else
    return (condition.getBoolean() ? successNode : failureNode)->compute(context, cache);
}

VectorPtr LuapeTestNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const
{
  BooleanVectorPtr conditions = conditionNode->compute(context, cache).staticCast<BooleanVector>();
  size_t n = conditions->getNumElements();
  const unsigned char* conditionsPtr = conditions->getData();

  if (successNode.isInstanceOf<LuapeConstantNode>() && failureNode.isInstanceOf<LuapeConstantNode>() && missingNode.isInstanceOf<LuapeConstantNode>())
  {
    Variable successValue = successNode.staticCast<LuapeConstantNode>()->getValue();
    Variable failureValue = failureNode.staticCast<LuapeConstantNode>()->getValue();
    Variable missingValue = failureNode.staticCast<LuapeConstantNode>()->getValue();

    if (successValue.isDouble() && failureValue.isDouble() && missingValue.isDouble())
    {
      double v[3];
      v[0] = failureValue.getDouble();
      v[1] = successValue.getDouble();
      v[2] = missingValue.getDouble();
      DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
      for (size_t i = 0; i < n; ++i)
        res->setValue(i, v[*conditionsPtr++]);
      return res;
    }
    else if (successValue.isObject() && failureValue.isObject() && missingValue.isObject())
    {
      ObjectPtr o[3];
      o[0] = failureValue.getObject();
      o[1] = successValue.getObject();
      o[2] = missingValue.getObject();      
      ObjectVectorPtr res = new ObjectVector(type, n);
      for (size_t i = 0; i < n; ++i)
        res->set(i, o[*conditionsPtr++]);
      return res;
    }
    else
    {
      Variable v[3];
      v[0] = failureValue;
      v[1] = successValue;
      v[2] = missingValue;
      VectorPtr res = vector(type, n);
      for (size_t i = 0; i < n; ++i)
        res->setElement(i, v[*conditionsPtr++]);
      return res;
    }
  }
  else
  {
    VectorPtr successValues = cache->compute(context, successNode);
    VectorPtr failureValues = cache->compute(context, failureNode);
    VectorPtr missingValues = cache->compute(context, missingNode);
    jassert(successValues->getNumElements() == n && failureValues->getNumElements() == n && missingValues->getNumElements() == n);

    if (successValues.isInstanceOf<DenseDoubleVector>() && failureValues.isInstanceOf<DenseDoubleVector>() && missingValues.isInstanceOf<DenseDoubleVector>())
    {
      DenseDoubleVectorPtr v[3];
      v[0] = failureValues.staticCast<DenseDoubleVector>();
      v[1] = successValues.staticCast<DenseDoubleVector>();
      v[2] = missingValues.staticCast<DenseDoubleVector>();
      DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
      for (size_t i = 0; i < n; ++i)
        res->setValue(i, v[*conditionsPtr++]->getValue(i));
      return res;
    }
    else if (successValues.isInstanceOf<ObjectVector>() && failureValues.isInstanceOf<ObjectVector>() && missingValues.isInstanceOf<ObjectVector>())
    {
      ObjectVectorPtr o[3];
      o[0] = failureValues.staticCast<ObjectVector>();
      o[1] = successValues.staticCast<ObjectVector>();
      o[2] = missingValues.staticCast<ObjectVector>();
      ObjectVectorPtr res = new ObjectVector(type, n);
      for (size_t i = 0; i < n; ++i)
        res->set(i, o[*conditionsPtr++]->get(i));
      return res;
    }
    else
    {
      VectorPtr v[3];
      v[0] = failureValues;
      v[1] = successValues;
      v[2] = missingValues;
      VectorPtr res = vector(type, n);
      for (size_t i = 0; i < n; ++i)
      {
        Variable value = v[*conditionsPtr++]->getElement(i);
        res->setElement(i, value);
      }
      return res;
    }
  }
  return VectorPtr();
}

/*
** LuapeSequenceNode
*/
LuapeSequenceNode::LuapeSequenceNode(TypePtr type, const std::vector<LuapeNodePtr>& nodes)
  : LuapeNode(type), nodes(nodes)
{
}

String LuapeSequenceNode::toShortString() const
{
  String res = getClass()->getShortName() + "\n";
  for (size_t i = 0; i < nodes.size(); ++i)
    res += nodes[i]->toShortString() + T("\n");
  return res;
}

VectorPtr LuapeSequenceNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const
{
  size_t n = cache->getNumSamples();
  VectorPtr outputs = createEmptyOutputs(n);
  for (size_t i = 0; i < nodes.size(); ++i)
    updateOutputs(outputs, cache->compute(context, nodes[i]));
  return outputs;
}

void LuapeSequenceNode::pushNode(const LuapeNodePtr& node, const std::vector<LuapeSamplesCachePtr>& cachesToUpdate)
{
  nodes.push_back(node);

  // update caches
  for (size_t i = 0; i < cachesToUpdate.size(); ++i)
  {
    LuapeSamplesCachePtr cache = cachesToUpdate[i];
    size_t n = cache->getNumSamples();
    VectorPtr outputs = cache->get(this);
    if (!outputs)
    {
      outputs = createEmptyOutputs(n);
      cache->set(this, outputs);
    }
    updateOutputs(outputs, cache->compute(defaultExecutionContext(), node));
  }
}

/*
** LuapeScalarSumNode
*/
LuapeScalarSumNode::LuapeScalarSumNode(const std::vector<LuapeNodePtr>& nodes)
  : LuapeSequenceNode(doubleType, nodes)
{
}

LuapeScalarSumNode::LuapeScalarSumNode() 
  : LuapeSequenceNode(doubleType)
{
}

Variable LuapeScalarSumNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  double res = 0.0;
  for (size_t i = 0; i < nodes.size(); ++i)
    res += cache->compute(context, nodes[i]).getDouble();
  return res;
}

VectorPtr LuapeScalarSumNode::createEmptyOutputs(size_t numSamples) const
  {return new DenseDoubleVector(numSamples, 0.0);}

void LuapeScalarSumNode::updateOutputs(const VectorPtr& outputs, const VectorPtr& newNodeValues) const
{
  const DenseDoubleVectorPtr& a = outputs.staticCast<DenseDoubleVector>();
  const DenseDoubleVectorPtr& b = newNodeValues.staticCast<DenseDoubleVector>();
  b->addTo(a);
}

/*
** LuapeVectorSumNode
*/
LuapeVectorSumNode::LuapeVectorSumNode(EnumerationPtr enumeration, const std::vector<LuapeNodePtr>& nodes)
  : LuapeSequenceNode(denseDoubleVectorClass(enumeration, doubleType), nodes)
{
}

LuapeVectorSumNode::LuapeVectorSumNode(EnumerationPtr enumeration) 
  : LuapeSequenceNode(denseDoubleVectorClass(enumeration, doubleType))
{
}

Variable LuapeVectorSumNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  ClassPtr doubleVectorClass = type;
  DenseDoubleVectorPtr res = new DenseDoubleVector(doubleVectorClass);
  for (size_t i = 0; i < nodes.size(); ++i)
    cache->compute(context, nodes[i]).getObjectAndCast<DenseDoubleVector>()->addTo(res);
  return res;
}

VectorPtr LuapeVectorSumNode::createEmptyOutputs(size_t numSamples) const
{
  ClassPtr doubleVectorClass = type;
  ObjectVectorPtr res = new ObjectVector(doubleVectorClass, numSamples);
  for (size_t i = 0; i < numSamples; ++i)
    res->set(i, new DenseDoubleVector(doubleVectorClass));
  return res;
}
 
void LuapeVectorSumNode::updateOutputs(const VectorPtr& outputs, const VectorPtr& newNodeValues) const
{
  const ObjectVectorPtr& a = outputs.staticCast<ObjectVector>();
  const ObjectVectorPtr& b = newNodeValues.staticCast<ObjectVector>();
  size_t n = a->getNumElements();
  jassert(n == b->getNumElements());
  for (size_t i = 0; i < n; ++i)
  {
    const DenseDoubleVectorPtr& newNodeValue = b->getAndCast<DenseDoubleVector>(i);
    if (newNodeValue)
    {
      DenseDoubleVectorPtr target = a->getAndCast<DenseDoubleVector>(i);
      newNodeValue->addTo(target);
#ifdef JUCE_DEBUG
      for (size_t j = 0; j < target->getNumValues(); ++j)
        jassert(isNumberValid(target->getValue(j)));
#endif // JUCE_DBEUG
    }
  }
}
