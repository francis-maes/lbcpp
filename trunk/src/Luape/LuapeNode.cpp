/*-----------------------------------------.---------------------------------.
| Filename: LuapeNode.cpp                  | Luape Graph Node                |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Luape/LuapeNode.h>
#include <lbcpp/Luape/LuapeCache.h>
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
  : type(type), allocationIndex(makeLuapeNodeAllocationIndex()), importance(0.0)
{
}

void LuapeNode::addImportance(double delta)
{
  jassert(isNumberValid(delta));
  importance += delta;
  size_t n = getNumSubNodes();
  for (size_t i = 0; i < n; ++i)
    getSubNode(i)->addImportance(delta);
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

LuapeSampleVectorPtr LuapeInputNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const
{
  jassert(false); // the value should already have been cached
  return LuapeSampleVectorPtr();
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

LuapeSampleVectorPtr LuapeConstantNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const
  {return LuapeSampleVector::createConstant(indices, value);}

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
  TypePtr* inputTypes = new TypePtr[numInputs];
  for (size_t i = 0; i < numInputs; ++i)
  {
    jassert(arguments[i]);
    inputTypes[i] = arguments[i]->getType();
    jassert(function->doAcceptInputType(i, inputTypes[i]));
  }

  type = function->initialize(inputTypes);
  delete [] inputTypes;
}

Variable LuapeFunctionNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  std::vector<Variable> inputValues(arguments.size());
  for (size_t i = 0; i < inputValues.size(); ++i)
    inputValues[i] = cache->compute(context, arguments[i]);
  return function->compute(context, &inputValues[0]);
}

LuapeSampleVectorPtr LuapeFunctionNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const
{
  std::vector<LuapeSampleVectorPtr> inputs(arguments.size());
  for (size_t i = 0; i < inputs.size(); ++i)
    inputs[i] = cache->getSamples(context, arguments[i], indices);

  double startTime = Time::getMillisecondCounterHiRes();
  LuapeSampleVectorPtr res = function->compute(context, inputs, type);
  double endTime = Time::getMillisecondCounterHiRes();
  cache->observeNodeComputingTime(refCountedPointerFromThis(this), indices->size(), endTime - startTime);
  return res;
}

/*
** LuapeTestNode
*/
LuapeTestNode::LuapeTestNode(const LuapeNodePtr& conditionNode, const LuapeNodePtr& failureNode, const LuapeNodePtr& successNode, const LuapeNodePtr& missingNode)
  : LuapeNode(successNode->getType()), conditionNode(conditionNode), failureNode(failureNode), successNode(successNode), missingNode(missingNode)
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
  String res = "(" + conditionNode->toShortString() + " ? " + 
          successNode->toShortString() + T(" : ") + failureNode->toShortString();
  if (missingNode && missingNode.isInstanceOf<LuapeConstantNode>() && missingNode.staticCast<LuapeConstantNode>()->getValue().exists())
    res += T(" : ") + missingNode->toShortString();
  return res + T(")");
}

Variable LuapeTestNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  Variable condition = conditionNode->compute(context, cache);
  if (condition.isMissingValue())
    return missingNode ? missingNode->compute(context, cache) : Variable::missingValue(type);
  else
    return (condition.getBoolean() ? successNode : failureNode)->compute(context, cache);
}

void LuapeTestNode::dispatchIndices(const LuapeSampleVectorPtr& conditionValues, IndexSetPtr& failureIndices, IndexSetPtr& successIndices, IndexSetPtr& missingIndices)
{
  failureIndices = new IndexSet();
  failureIndices->reserve(conditionValues->size() / 4);
  successIndices = new IndexSet();
  successIndices->reserve(conditionValues->size() / 4);
  missingIndices = new IndexSet();
  for (LuapeSampleVector::const_iterator it = conditionValues->begin(); it != conditionValues->end(); ++it)
  {
    switch (it.getRawBoolean())
    {
    case 0: failureIndices->append(it.getIndex()); break;
    case 1: successIndices->append(it.getIndex()); break;
    case 2: missingIndices->append(it.getIndex()); break;
    default: jassert(false);
    }
  }
}

LuapeSampleVectorPtr LuapeTestNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const
{
  jassert(indices->size());
  LuapeSampleVectorPtr conditions = cache->getSamples(context, conditionNode, indices);
  size_t n = conditions->size();

  double startTime;
  VectorPtr resultVector;

  if (successNode.isInstanceOf<LuapeConstantNode>() && failureNode.isInstanceOf<LuapeConstantNode>() && missingNode.isInstanceOf<LuapeConstantNode>())
  {
    Variable v[3];
    v[0] = failureNode.staticCast<LuapeConstantNode>()->getValue();
    v[1] = successNode.staticCast<LuapeConstantNode>()->getValue();
    v[2] = missingNode.staticCast<LuapeConstantNode>()->getValue();
    startTime = Time::getMillisecondCounterHiRes();

    if (v[0].isDouble() && v[1].isDouble() && v[2].isDouble())
    {
      double dv[3];
      dv[0] = v[0].getDouble();
      dv[1] = v[1].getDouble();
      dv[2] = v[2].getDouble();
      DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
      double* ptr = res->getValuePointer(0);
      for (LuapeSampleVector::const_iterator it = conditions->begin(); it != conditions->end(); ++it)
        *ptr++ = dv[it.getRawBoolean()];
      resultVector = res;
    }
    else if (v[0].isObject() && v[1].isObject() && v[2].isObject())
    {
      ObjectPtr ov[3];
      ov[0] = v[0].getObject();
      ov[1] = v[1].getObject();
      ov[2] = v[2].getObject();      
      ObjectVectorPtr res = new ObjectVector(type, n);
      size_t i = 0;
      for (LuapeSampleVector::const_iterator it = conditions->begin(); it != conditions->end(); ++it, ++i)
        res->set(i, ov[it.getRawBoolean()]);
      resultVector = res;
    }
    else
    {
      VectorPtr res = vector(type, n);
      size_t i = 0;
      for (LuapeSampleVector::const_iterator it = conditions->begin(); it != conditions->end(); ++it, ++i)
      {
        unsigned char b = it.getRawBoolean();
        jassert(b < 3);
        res->setElement(i, v[b]);
      }
      resultVector = res;
    }
  }
  else
  {
    IndexSetPtr failureIndices, successIndices, missingIndices;
    dispatchIndices(conditions, failureIndices, successIndices, missingIndices);

    LuapeSampleVectorPtr subValues[3];
    subValues[0] = cache->getSamples(context, failureNode, failureIndices);
    subValues[1] = cache->getSamples(context, successNode, successIndices);
    subValues[2] = cache->getSamples(context, missingNode, missingIndices);
    startTime = Time::getMillisecondCounterHiRes();

    TypePtr elementsType = subValues[0]->getElementsType();
    jassert(subValues[1]->getElementsType() == elementsType);
    jassert(subValues[2]->getElementsType() == elementsType);

    LuapeSampleVector::const_iterator it[3];
    it[0] = subValues[0]->begin();
    it[1] = subValues[1]->begin();
    it[2] = subValues[2]->begin();

    if (elementsType == doubleType)
    {
      DenseDoubleVectorPtr res = new DenseDoubleVector(n, 0.0);
      double* ptr = res->getValuePointer(0);
      for (LuapeSampleVector::const_iterator conditionIt = conditions->begin(); conditionIt != conditions->end(); ++conditionIt)
      {
        LuapeSampleVector::const_iterator& currentIt = it[conditionIt.getRawBoolean()];
        *ptr++ = currentIt.getRawDouble();
        ++currentIt;
      }
      resultVector = res;
    }
    else if (elementsType->inheritsFrom(objectClass))
    {
      ObjectVectorPtr res = new ObjectVector(type, n);
      size_t i = 0;
      for (LuapeSampleVector::const_iterator conditionIt = conditions->begin(); conditionIt != conditions->end(); ++conditionIt, ++i)
      {
        LuapeSampleVector::const_iterator& currentIt = it[conditionIt.getRawBoolean()];
        res->set(i, currentIt.getRawObject());
        ++currentIt;
      }
      resultVector = res;
    }
    else
    {
      VectorPtr res = vector(type, n);
      size_t i = 0;
      for (LuapeSampleVector::const_iterator conditionIt = conditions->begin(); conditionIt != conditions->end(); ++conditionIt, ++i)
      {
        LuapeSampleVector::const_iterator& currentIt = it[conditionIt.getRawBoolean()];
        res->setElement(i, *currentIt);
        ++currentIt;
      }
      resultVector = res;
    }
  }
  double endTime = Time::getMillisecondCounterHiRes();
  cache->observeNodeComputingTime(refCountedPointerFromThis(this), indices->size(), endTime - startTime);
  return new LuapeSampleVector(indices, resultVector);
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

LuapeSampleVectorPtr LuapeSequenceNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const
{
  std::vector<LuapeSampleVectorPtr> nodeValues(nodes.size());
  for (size_t i = 0; i < nodeValues.size(); ++i)
    nodeValues[i] = cache->getSamples(context, nodes[i], indices);

  double startTime = Time::getMillisecondCounterHiRes();
  VectorPtr outputs = createEmptyOutputs(indices->size());
  for (size_t i = 0; i < nodeValues.size(); ++i)
    updateOutputs(outputs, nodeValues[i]);
  double endTime = Time::getMillisecondCounterHiRes();
  cache->observeNodeComputingTime(refCountedPointerFromThis(this), indices->size(), endTime - startTime);

  return new LuapeSampleVector(indices, outputs);
}

void LuapeSequenceNode::pushNode(ExecutionContext& context, const LuapeNodePtr& node, const std::vector<LuapeSamplesCachePtr>& cachesToUpdate)
{
  nodes.push_back(node);

  // update caches
  for (size_t i = 0; i < cachesToUpdate.size(); ++i)
  {
    LuapeSamplesCachePtr cache = cachesToUpdate[i];
    //size_t n = cache->getNumSamples();
    VectorPtr outputs = cache->getNodeCache(this);
    jassert(outputs);
    updateOutputs(outputs, cache->getSamples(context, node, cache->getAllIndices()));
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

void LuapeScalarSumNode::updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues) const
{
  const DenseDoubleVectorPtr& a = outputs.staticCast<DenseDoubleVector>();
  double* dest = a->getValuePointer(0);
  for (LuapeSampleVector::const_iterator it = newNodeValues->begin(); it != newNodeValues->end(); ++it)
  {
    double value = it.getRawDouble();
    *dest++ += (value == doubleMissingValue ? 0.0 : value);
  }
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
  {
    DenseDoubleVectorPtr value = cache->compute(context, nodes[i]).getObjectAndCast<DenseDoubleVector>();
    if (value)
      value->addTo(res);
  }
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
 
void LuapeVectorSumNode::updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues) const
{
  const ObjectVectorPtr& a = outputs.staticCast<ObjectVector>();
  jassert(newNodeValues->size() == a->getNumElements());
  jassert(newNodeValues->getElementsType()->inheritsFrom(denseDoubleVectorClass()));
  size_t i = 0;
  for (LuapeSampleVector::const_iterator it = newNodeValues->begin(); it != newNodeValues->end(); ++it, ++i)
  {
    const DenseDoubleVectorPtr& newNodeValue = it.getRawObject().staticCast<DenseDoubleVector>();
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

/*
** LuapeCreateSparseVectorNode
*/
LuapeCreateSparseVectorNode::LuapeCreateSparseVectorNode(const std::vector<LuapeNodePtr>& nodes)
  : LuapeSequenceNode(sparseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType), nodes) {}

LuapeCreateSparseVectorNode::LuapeCreateSparseVectorNode()
  : LuapeSequenceNode(sparseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType)) {}

Variable LuapeCreateSparseVectorNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  SparseDoubleVectorPtr res = new SparseDoubleVector((ClassPtr)getType());
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    Variable v = cache->compute(context, nodes[i]);
    if (v.exists())
      res->incrementValue((size_t)v.getInteger(), 1.0);
  }
  return res;
}

VectorPtr LuapeCreateSparseVectorNode::createEmptyOutputs(size_t numSamples) const
{
  ClassPtr sparseVectorClass = type;
  ObjectVectorPtr res = new ObjectVector(sparseVectorClass, numSamples);
  for (size_t i = 0; i < numSamples; ++i)
    res->set(i, new SparseDoubleVector(sparseVectorClass));
  return res;
}

void LuapeCreateSparseVectorNode::updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues) const
{ 
  const ObjectVectorPtr& a = outputs.staticCast<ObjectVector>();
  jassert(newNodeValues->size() == a->getNumElements());
  jassert(newNodeValues->getElementsType() == positiveIntegerType);
  size_t i = 0;
  for (LuapeSampleVector::const_iterator it = newNodeValues->begin(); it != newNodeValues->end(); ++it, ++i)
  {
    int newNodeValue = it.getRawInteger();
    if (newNodeValue >= 0)
      a->getAndCast<SparseDoubleVector>(i)->incrementValue((size_t)newNodeValue, 1.0);
  }
}
