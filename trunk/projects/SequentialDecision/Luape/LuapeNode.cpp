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
** LuapeFunction
*/
String LuapeFunction::toShortString(const std::vector<LuapeNodePtr>& inputs) const
{
  String res = getClass()->getShortName() + T("(");
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    res += inputs[i]->toShortString();
    if (i < inputs.size() - 1)
      res += T(", ");
  }
  return res + T(")");
}

Variable LuapeFunction::compute(ExecutionContext& context, const std::vector<LuapeNodePtr>& inputs, const std::vector<Variable>& state) const
{
  std::vector<Variable> inputValues(inputs.size());
  for (size_t i = 0; i < inputValues.size(); ++i)
    inputValues[i] = state[inputs[i]->getIndexInGraph()];
  return compute(context, &inputValues[0]);
}

VectorPtr LuapeFunction::compute(ExecutionContext& context, const std::vector<VectorPtr>& inputs, TypePtr outputType) const
{
  jassert(inputs.size() == getNumInputs());
  jassert(inputs.size());

  size_t n = inputs[0]->getNumElements();
  VectorPtr res = vector(outputType, n);
  for (size_t i = 0; i < n; ++i)
  {
    std::vector<Variable> inputValues(inputs.size());
    for (size_t j = 0; j < inputValues.size(); ++j)
      inputValues[j] = inputs[j]->getElement(i);
    res->setElement(i, compute(context, &inputValues[0]));
  }
  return res;
}

bool LuapeFunction::acceptInputsStack(const std::vector<LuapeNodePtr>& stack) const
{
  size_t n = getNumInputs();
  if (n > stack.size())
    return false;
  size_t stackFirstIndex = stack.size() - n;
  for (size_t i = 0; i < n; ++i)
    if (!doAcceptInputType(i, stack[stackFirstIndex + i]->getType()))
      return false;

  if (n)
  {
    if (hasFlags(commutativeFlag))
    {
      LuapeNodePtr node = stack[stackFirstIndex];
      for (size_t i = 1; i < n; ++i)
      {
        LuapeNodePtr newNode = stack[stackFirstIndex + i];
        if (newNode < node)
          return false;
        node = newNode;
      }
    }
    if (hasFlags(allSameArgIrrelevantFlag))
    {
      bool ok = false;
      for (size_t i = 1; i < n; ++i)
        if (stack[stackFirstIndex + i] != stack[stackFirstIndex])
        {
          ok = true;
          break;
        }
      if (!ok)
        return false;
    }
  }
  return true;
}

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

void LuapeNodeCache::clearSamples(bool clearTrainingSamples, bool clearValidationSamples)
{
  if (clearTrainingSamples)
  {
    trainingSamples = VectorPtr();
    sortedDoubleValues = SparseDoubleVectorPtr();
  }
  if (clearValidationSamples)
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
  size_t n = trainingSamples->getNumElements();
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

/*
** LuapeNode
*/
LuapeNode::LuapeNode(const TypePtr& type, const String& name)
  : NameableObject(name), type(type), indexInGraph((size_t)-1)
{
  if (type != nilType)
    cache = new LuapeNodeCache(type);
}

void LuapeNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const LuapeNodePtr& target = t.staticCast<LuapeNode>();
  target->name = name;
  target->type = type;
  target->cache = cache;
}

/*
** LuapeInputNode
*/
LuapeInputNode::LuapeInputNode(const TypePtr& type, const String& name, size_t inputIndex)
  : LuapeNode(type, name), inputIndex(inputIndex) {}

String LuapeInputNode::toShortString() const
  {return getName();}

Variable LuapeInputNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {jassert(false); return Variable();}

void LuapeInputNode::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  target.staticCast<LuapeInputNode>()->inputIndex = inputIndex;
  LuapeNode::clone(context, target);
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
  cache = new LuapeNodeCache(type);
}

Variable LuapeFunctionNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {return function->compute(context, arguments, state);}

void LuapeFunctionNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  LuapeNode::clone(context, t);
  const LuapeFunctionNodePtr& target = t.staticCast<LuapeNode>();
  target->function = function;
  target->arguments = arguments;
}

size_t LuapeFunctionNode::getDepth() const
{
  size_t maxInputDepth = 0;
  for (size_t i = 0; i < arguments.size(); ++i)
  {
    size_t d = arguments[i]->getDepth();
    if (d > maxInputDepth)
      maxInputDepth = d;
  }
  return maxInputDepth + 1;
}

/*
** LuapeYieldNode
*/
LuapeYieldNode::LuapeYieldNode(const LuapeNodePtr& argument)
  : LuapeNode(nilType, argument->getName()), argument(argument)
{
}

String LuapeYieldNode::toShortString() const
{
  return T("yield(") + argument->toShortString() + T(") [") + argument->getCache()->toShortString() + T("]");
}

Variable LuapeYieldNode::compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
{
  const Variable& res = state[argument->getIndexInGraph()];
  if (callback)
    callback->valueYielded(res);
  return res;
}

size_t LuapeYieldNode::getDepth() const
  {return argument->getDepth() + 1;}

void LuapeYieldNode::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  LuapeNode::clone(context, t);
  t.staticCast<LuapeYieldNode>()->argument = argument;
}
