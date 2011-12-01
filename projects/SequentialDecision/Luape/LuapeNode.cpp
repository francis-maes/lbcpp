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

#if 0
/*
** LuapeNodeCache
*/
LuapeNodeCache::LuapeNodeCache(TypePtr elementsType) : elementsType(elementsType)
{
}

void LuapeNodeCache::resizeSamples(bool isTrainingSamples, size_t size)
{
  VectorPtr& samples = isTrainingSamples ? trainingSamples : validationSamples;
  if (!samples)
    samples = vector(elementsType, size);
  else
    samples->resize(size);
}

void LuapeNodeCache::resizeSamples(size_t numTrainingSamples, size_t numValidationSamples)
{
  resizeSamples(true, numTrainingSamples);
  resizeSamples(false, numValidationSamples);
}

void LuapeNodeCache::setSample(bool isTrainingSample, size_t index, const Variable& value)
{
  const VectorPtr& samples = isTrainingSample ? trainingSamples : validationSamples;
  jassert(samples);
  samples->setElement(index, value);
}

void LuapeNodeCache::clearSamples(bool isTrainingSamples)
{
  if (isTrainingSamples)
  {
    trainingSamples = VectorPtr();
    sortedDoubleValues = SparseDoubleVectorPtr();
  }
  else
    validationSamples = VectorPtr();
}

struct SortDoubleValuesOperator
{
  static double transformIntoValidNumber(double input)
    {return input;}

  bool operator()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
  {
    double aa = transformIntoValidNumber(a.second);
    double bb = transformIntoValidNumber(b.second);
    return aa == bb ? a.first < b.first : aa < bb;
  }
};

SparseDoubleVectorPtr LuapeNodeCache::getSortedDoubleValues()
{
  jassert(elementsType->isConvertibleToDouble());

  size_t n = trainingSamples->getNumElements();
  if (!sortedDoubleValues)
  {
    sortedDoubleValues = new SparseDoubleVector();
    std::vector< std::pair<size_t, double> >& v = sortedDoubleValues->getValuesVector();
    v.resize(n);
    for (size_t i = 0; i < n; ++i)
      v[i] = std::make_pair(i, trainingSamples->getElement(i).toDouble());
    std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
  }
  return sortedDoubleValues;
}

BinaryKeyPtr LuapeNodeCache::makeKeyFromSamples(bool useTrainingSamples) const
{
  ContainerPtr samples = getSamples(useTrainingSamples);
  size_t n = samples->getNumElements();
  
  BooleanVectorPtr booleanVector = samples.dynamicCast<BooleanVector>();
  if (booleanVector)
  {
    BinaryKeyPtr res = new BinaryKey(1 + n / 8);
    std::vector<bool>::const_iterator it = booleanVector->getElements().begin();
    for (size_t i = 0; i < n; ++i)
      res->pushBit(*it++);
    res->fillBits();
    return res;
  }

  DenseDoubleVectorPtr doubleVector = samples.dynamicCast<DenseDoubleVector>();
  if (doubleVector)
  {
    BinaryKeyPtr res = new BinaryKey(n * 4);
    for (size_t i = 0; i < n; ++i)
      res->push32BitInteger((int)(doubleVector->getValue(i) * 10e6));
    return res;
  }

  ObjectVectorPtr objectVector = samples.dynamicCast<ObjectVector>();
  if (objectVector)
  {
    BinaryKeyPtr res = new BinaryKey(n * sizeof (void* ));
    for (size_t i = 0; i < n; ++i)
      res->pushPointer(objectVector->get(i));
    return res;
  }

  GenericVectorPtr genericVector = samples.dynamicCast<GenericVector>();
  if (genericVector)
  {
    if (genericVector->getElementsType()->inheritsFrom(integerType))
    {
      BinaryKeyPtr res = new BinaryKey(n * sizeof (VariableValue));
      res->pushBytes((unsigned char* )&genericVector->getValues()[0], n * sizeof (VariableValue));
      return res;
    }

    jassert(false);
  }

  jassert(false);
  return BinaryKeyPtr();
}

String LuapeNodeCache::toShortString() const
{
  size_t n = trainingSamples ? trainingSamples->getNumElements() : 0;
  if (n == 0)
    return "<no examples>";

  TypePtr elementsType = trainingSamples->getElementsType();
  if (elementsType == booleanType)
  {
    BooleanVectorPtr booleans = trainingSamples.staticCast<BooleanVector>();
    size_t countOfTrue = 0;
    for (size_t i = 0; i < n; ++i)
      if (booleans->get(i))
        ++countOfTrue;
    return String((int)countOfTrue) + T(" / ") + String((int)n);
  }
  else if (elementsType.isInstanceOf<Enumeration>())
  {
    DenseDoubleVectorPtr probabilities = new DenseDoubleVector(elementsType.staticCast<Enumeration>(), doubleType);
    double invZ = 1.0 / (double)n;
    for (size_t i = 0; i < n; ++i)
    {
      Variable v = trainingSamples->getElement(i);
      if (!v.isMissingValue())
        probabilities->incrementValue(v.getInteger(), invZ);
    }
    return probabilities->toShortString();
  }
  else if (elementsType->isConvertibleToDouble())
  {
    ScalarVariableStatistics stats;
    for (size_t i = 0; i < n; ++i)
      stats.push(trainingSamples->getElement(i).toDouble());
    return stats.toShortString();
  }
  else
    return String((int)n);
}

#endif // 0

/*
** LuapeNode
*/
static size_t makeLuapeNodeAllocationIndex()
{
  static size_t res = 0;
  return res++; // warning: not safe for multi-threading ...
}

LuapeNode::LuapeNode(const TypePtr& type, const String& name)
  : NameableObject(name), type(type), allocationIndex(makeLuapeNodeAllocationIndex())
{
}

LuapeNode::LuapeNode() : allocationIndex(makeLuapeNodeAllocationIndex())
{
}

/*
** LuapeInputNode
*/
LuapeInputNode::LuapeInputNode(const TypePtr& type, const String& name)
  : LuapeNode(type, name) {}

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
    inputTypes[i] = arguments[i]->getType();
    jassert(function->doAcceptInputType(i, inputTypes[i]));
  }

  type = function->getOutputType(inputTypes);
  name = function->toShortString(arguments);
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
LuapeTestNode::LuapeTestNode(const LuapeNodePtr& conditionNode, const LuapeNodePtr& successNode, const LuapeNodePtr& failureNode)
  : LuapeNode(successNode->getType(), "if(" + conditionNode->toShortString() + ")"),
    conditionNode(conditionNode), successNode(successNode), failureNode(failureNode)
{
}

Variable LuapeTestNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  bool test = conditionNode->compute(context, cache).getBoolean();
  return (test ? successNode : failureNode)->compute(context, cache);
}

VectorPtr LuapeTestNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const
{
/*  BooleanVectorPtr tests = conditionNode->compute(context, cache).staticCast<BooleanVector>();
  size_t n = tests->getNumElements();
  std::vector<bool>::const_iterator it = tests->getElements().begin()
  for (size_t i = 0; i < n; ++i)
  {
    if (*it++)
    {
    }
  */
  jassert(false);
  return VectorPtr();
}

/*
** LuapeSequenceNode
*/
LuapeSequenceNode::LuapeSequenceNode(const std::vector<LuapeNodePtr>& nodes)
  : nodes(nodes)
{
}

String LuapeSequenceNode::toShortString() const
{
  String res;
  for (size_t i = 0; i < nodes.size(); ++i)
    res += nodes[i]->toShortString() + T("\n");
  return res;
}

Variable LuapeSequenceNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    if (i < nodes.size() - 1)
      cache->compute(context, nodes[i]);
    else
      return cache->compute(context, nodes[i]);
  }
  return Variable();
}

VectorPtr LuapeSequenceNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const
{
  jassert(false);
  return VectorPtr();
}

void LuapeSequenceNode::pushNode(const LuapeNodePtr& node)
{
  nodes.push_back(node);
  type = node->getType();
}

/*
** LuapeYieldNode
*/
LuapeYieldNode::LuapeYieldNode(const Variable& value)
  : LuapeNode(nilType, T("yield(") + value.toShortString() + T(")")), value(value)
{
}

LuapeYieldNode::LuapeYieldNode()
  : LuapeNode(nilType, String::empty)
{
}

String LuapeYieldNode::toShortString() const
{
  return name;
}

Variable LuapeYieldNode::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  const LuapeGraphCallbackPtr& callback = cache->getCallback();
  if (callback)
    callback->graphYielded(refCountedPointerFromThis(this), value);
  return Variable();
}

VectorPtr LuapeYieldNode::compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache) const
{
  jassert(false);
  return VectorPtr();
}
