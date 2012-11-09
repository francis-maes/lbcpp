/*-----------------------------------------.---------------------------------.
| Filename: Expression.cpp                 | Expression classes              |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp-ml/Expression.h>
#include <lbcpp-ml/DataTable.h>
#include <algorithm>
using namespace lbcpp;

/*
** Expression
*/
static size_t makeExpressionAllocationIndex()
{
  static size_t res = 0;
  return res++; // warning: not safe for multi-threading ...
}

Expression::Expression(const TypePtr& type)
  : type(type), allocationIndex(makeExpressionAllocationIndex()), importance(0.0)
{
}

void Expression::addImportance(double delta)
{
  jassert(isNumberValid(delta));
  importance += delta;
  size_t n = getNumSubNodes();
  for (size_t i = 0; i < n; ++i)
    getSubNode(i)->addImportance(delta);
}

size_t Expression::getDepth() const // returns 1 for a leaf node 
{
  size_t res = 0;
  for (size_t i = 0; i < getNumSubNodes(); ++i)
  {
    ExpressionPtr subNode = getSubNode(i);
    if (subNode)
    {
      size_t d = subNode->getDepth();
      if (d > res)
        res = d;
    }
  }
  return res + 1;
}

size_t Expression::getNodeDepth(ExpressionPtr node) const // returns 1 if (node == this)
{
  if (node.get() == this)
    return 1;
  for (size_t i = 0; i < getNumSubNodes(); ++i)
  {
    ExpressionPtr subNode = getSubNode(i);
    if (subNode)
    {
      size_t res = subNode->getNodeDepth(node);
      if (res != (size_t)-1)
        return res + 1;
    }
  }
  return (size_t)-1;
}

size_t Expression::getTreeSize() const
{
  size_t res = 1;
  for (size_t i = 0; i < getNumSubNodes(); ++i)
  {
    ExpressionPtr subNode = getSubNode(i);
    if (subNode)
      res += subNode->getTreeSize();
  }
  return res;
}

inline ExpressionPtr getNodeByTreeIndexRec(ExpressionPtr expression, size_t& index)
{
  if (index == 0)
    return expression;
  index -= 1;
  for (size_t i = 0; i < expression->getNumSubNodes(); ++i)
  {
    ExpressionPtr subNode = expression->getSubNode(i);
    if (subNode)
    {
      ExpressionPtr res = getNodeByTreeIndexRec(subNode, index);
      if (res)
        return res;
    }
  }
  return ExpressionPtr();
}

ExpressionPtr Expression::getNodeByTreeIndex(size_t index) const
  {return getNodeByTreeIndexRec(refCountedPointerFromThis(this), index);}

ExpressionPtr Expression::cloneAndSubstitute(ExpressionPtr sourceNode, ExpressionPtr targetNode) const
{
  if (this == sourceNode.get())
    return targetNode;
  ExpressionPtr pthis = refCountedPointerFromThis(this);
  FunctionExpressionPtr functionNode = pthis.dynamicCast<FunctionExpression>();
  if (!functionNode)
    return pthis;
  std::vector<ExpressionPtr> arguments(functionNode->getNumArguments());
  for (size_t i = 0; i < arguments.size(); ++i)
    arguments[i] = functionNode->getArgument(i)->cloneAndSubstitute(sourceNode, targetNode);
  return new FunctionExpression(functionNode->getFunction(), arguments);
}

void Expression::getInternalNodes(std::vector<ExpressionPtr>& res) const
{
  size_t n = getNumSubNodes();
  if (n > 0)
  {
    res.push_back(refCountedPointerFromThis(this));
    for (size_t i = 0; i < n; ++i)
      getSubNode(i)->getInternalNodes(res);
  }
}

void Expression::getLeafNodes(std::vector<ExpressionPtr>& res) const
{
  size_t n = getNumSubNodes();
  if (n > 0)
  {
    for (size_t i = 0; i < n; ++i)
      getSubNode(i)->getLeafNodes(res);
  }
  else
    res.push_back(refCountedPointerFromThis(this));
}

ExpressionPtr Expression::sampleNode(RandomGeneratorPtr random) const
  {return getNodeByTreeIndex(random->sampleSize(getTreeSize()));}

ExpressionPtr Expression::sampleNode(RandomGeneratorPtr random, double functionSelectionProbability) const
{
  if (getNumSubNodes() == 0)
    return refCountedPointerFromThis(this);

  std::vector<ExpressionPtr> nodes;
  if (random->sampleBool(functionSelectionProbability))
    getInternalNodes(nodes);
  else
    getLeafNodes(nodes);
  jassert(nodes.size() > 0);
  return nodes[random->sampleSize(nodes.size())];
}

ExpressionPtr Expression::sampleSubNode(RandomGeneratorPtr random) const
{
  size_t n = getNumSubNodes();
  jassert(n > 0);
  return getSubNode(random->sampleSize(n));
}

DataVectorPtr Expression::compute(ExecutionContext& context, const DataTablePtr& data, const IndexSetPtr& indices) const
{
  IndexSetPtr idx = indices ? indices : data->getAllIndices();
  VectorPtr samples = data->getSamplesByExpression(refCountedPointerFromThis(this));
  if (samples)
    return DataVector::createCached(idx, samples);
  else
    return computeSamples(context, data, idx);
}

/*
** VariableExpression
*/
VariableExpression::VariableExpression(const TypePtr& type, const String& name, size_t inputIndex)
  : Expression(type), name(name), inputIndex(inputIndex)
{
}

VariableExpression::VariableExpression() : inputIndex(0)
{
}

String VariableExpression::toShortString() const
  {return name;}

ObjectPtr VariableExpression::compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {return inputs[inputIndex];}

DataVectorPtr VariableExpression::computeSamples(ExecutionContext& context, const DataTablePtr& data, const IndexSetPtr& indices) const
{
  jassertfalse; // if we reach this point, then the data for this variable is missing in the data table
  return DataVectorPtr();
}

/*
** ConstantExpression
*/
ConstantExpression::ConstantExpression(const ObjectPtr& value)
  : Expression(value->getClass()), value(value)
{
}

String ConstantExpression::toShortString() const
  {return value->toShortString();}

ObjectPtr ConstantExpression::compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {return value;}

DataVectorPtr ConstantExpression::computeSamples(ExecutionContext& context, const DataTablePtr& data, const IndexSetPtr& indices) const
  {return DataVector::createConstant(indices, value);}

/*
** FunctionExpression
*/
FunctionExpression::FunctionExpression(const FunctionPtr& function, const std::vector<ExpressionPtr>& arguments)
  : function(function), arguments(arguments)
{
  initialize();
}

FunctionExpression::FunctionExpression(const FunctionPtr& function, const ExpressionPtr& argument1, const ExpressionPtr& argument2)
  : function(function), arguments(2)
{
  arguments[0] = argument1;
  arguments[1] = argument2;
  initialize();
}

FunctionExpression::FunctionExpression(const FunctionPtr& function, const ExpressionPtr& argument)
  : function(function), arguments(1, argument)
{
  initialize();
}

String FunctionExpression::toShortString() const
  {return function->makeNodeName(arguments);}

void FunctionExpression::initialize()
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

ObjectPtr FunctionExpression::compute(ExecutionContext& context, const ObjectPtr* inputs) const
{
  size_t n = arguments.size();
  if (n == 0)
    return function->compute(context, NULL);
  else if (n == 1)
  {
    ObjectPtr v = arguments[0]->compute(context, inputs);
    return function->compute(context, &v);
  }
  else if (n == 2)
  {
    ObjectPtr v[2];
    v[0] = arguments[0]->compute(context, inputs);
    v[1] = arguments[1]->compute(context, inputs);
    return function->compute(context, v);
  }
  else
  {
    std::vector<ObjectPtr> inputValues(arguments.size());
    for (size_t i = 0; i < arguments.size(); ++i)
      inputValues[i] = arguments[i]->compute(context, inputs);
    return function->compute(context, &inputValues[0]);
  }
}

DataVectorPtr FunctionExpression::computeSamples(ExecutionContext& context, const DataTablePtr& data, const IndexSetPtr& indices) const
{
  std::vector<DataVectorPtr> inputs(arguments.size());
  for (size_t i = 0; i < inputs.size(); ++i)
    inputs[i] = arguments[i]->compute(context, data, indices);

  //double startTime = Time::getMillisecondCounterHiRes();
  DataVectorPtr res = function->compute(context, inputs, type);
  //double endTime = Time::getMillisecondCounterHiRes();
  //cache->observeNodeComputingTime(refCountedPointerFromThis(this), indices->size(), endTime - startTime);
  return res;
}

/*
** TestExpression
*/
TestExpression::TestExpression(const ExpressionPtr& conditionNode, const ExpressionPtr& failureNode, const ExpressionPtr& successNode, const ExpressionPtr& missingNode)
  : Expression(successNode->getType()), conditionNode(conditionNode), failureNode(failureNode), successNode(successNode), missingNode(missingNode)
{
}

TestExpression::TestExpression(const ExpressionPtr& conditionNode, TypePtr outputType)
  : Expression(outputType), conditionNode(conditionNode)
{
}

size_t TestExpression::getNumSubNodes() const
  {return 4;}
  
const ExpressionPtr& TestExpression::getSubNode(size_t index) const
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

String TestExpression::toShortString() const
{
  String res = "(" + conditionNode->toShortString() + " ? " + 
    (successNode ? successNode->toShortString() : T("NULL")) + T(" : ") + 
    (failureNode ? failureNode->toShortString() : T("NULL"));
  if (missingNode && missingNode.isInstanceOf<ConstantExpression>() && missingNode.staticCast<ConstantExpression>()->getValue().exists())
    res += T(" : ") + missingNode->toShortString();
  return res + T(")");
}

ObjectPtr TestExpression::compute(ExecutionContext& context, const ObjectPtr* inputs) const
{
  ObjectPtr condition = conditionNode->compute(context, inputs);
  ExpressionPtr subNode = (condition ? (NewBoolean::get(condition) ? successNode : failureNode) : missingNode);
  return subNode ? subNode->compute(context, inputs) : ObjectPtr();
}

void TestExpression::dispatchIndices(const DataVectorPtr& conditionValues, IndexSetPtr& failureIndices, IndexSetPtr& successIndices, IndexSetPtr& missingIndices)
{
  failureIndices = new IndexSet();
  failureIndices->reserve(conditionValues->size() / 4);
  successIndices = new IndexSet();
  successIndices->reserve(conditionValues->size() / 4);
  missingIndices = new IndexSet();
  for (DataVector::const_iterator it = conditionValues->begin(); it != conditionValues->end(); ++it)
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

DataVectorPtr TestExpression::computeSamples(ExecutionContext& context, const DataTablePtr& data, const IndexSetPtr& indices) const
{
  jassert(indices->size());
  DataVectorPtr conditions = conditionNode->compute(context, data, indices);
  size_t n = conditions->size();

  VectorPtr resultVector;

  if (successNode.isInstanceOf<ConstantExpression>() && failureNode.isInstanceOf<ConstantExpression>() && missingNode.isInstanceOf<ConstantExpression>())
  {
    ObjectPtr v[3];
    v[0] = failureNode.staticCast<ConstantExpression>()->getValue();
    v[1] = successNode.staticCast<ConstantExpression>()->getValue();
    v[2] = missingNode.staticCast<ConstantExpression>()->getValue();
    
    if (v[0].dynamicCast<NewDouble>() && v[1].dynamicCast<NewDouble>() && v[2].dynamicCast<NewDouble>())
    {
      double dv[3];
      dv[0] = NewDouble::get(v[0]);
      dv[1] = NewDouble::get(v[1]);
      dv[2] = NewDouble::get(v[2]);
      DenseDoubleVectorPtr res = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, type, n, 0.0);
      double* ptr = res->getValuePointer(0);
      for (DataVector::const_iterator it = conditions->begin(); it != conditions->end(); ++it)
        *ptr++ = dv[it.getRawBoolean()];
      resultVector = res;
    }
    else
    {
      ObjectVectorPtr res = new ObjectVector(type, n);
      size_t i = 0;
      for (DataVector::const_iterator it = conditions->begin(); it != conditions->end(); ++it, ++i)
        res->set(i, v[it.getRawBoolean()]);
      resultVector = res;
    }
  }
  else
  {
    IndexSetPtr failureIndices, successIndices, missingIndices;
    dispatchIndices(conditions, failureIndices, successIndices, missingIndices);

    DataVectorPtr subValues[3];
    subValues[0] = getSubSamples(context, failureNode, data, failureIndices);
    subValues[1] = getSubSamples(context, successNode, data, successIndices);
    subValues[2] = getSubSamples(context, missingNode, data, missingIndices);
    
    TypePtr elementsType = subValues[0]->getElementsType();
    jassert(subValues[1]->getElementsType() == elementsType);
    jassert(subValues[2]->getElementsType() == elementsType);

    DataVector::const_iterator it[3];
    it[0] = subValues[0]->begin();
    it[1] = subValues[1]->begin();
    it[2] = subValues[2]->begin();

    if (elementsType == doubleType)
    {
      DenseDoubleVectorPtr res = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, type, n, 0.0);
      double* ptr = res->getValuePointer(0);
      for (DataVector::const_iterator conditionIt = conditions->begin(); conditionIt != conditions->end(); ++conditionIt)
      {
        DataVector::const_iterator& currentIt = it[conditionIt.getRawBoolean()];
        *ptr++ = currentIt.getRawDouble();
        ++currentIt;
      }
      resultVector = res;
    }
    else if (elementsType->inheritsFrom(objectClass))
    {
      ObjectVectorPtr res = new ObjectVector(type, n);
      size_t i = 0;
      for (DataVector::const_iterator conditionIt = conditions->begin(); conditionIt != conditions->end(); ++conditionIt, ++i)
      {
        DataVector::const_iterator& currentIt = it[conditionIt.getRawBoolean()];
        res->set(i, currentIt.getRawObject());
        ++currentIt;
      }
      resultVector = res;
    }
    else
    {
      VectorPtr res = vector(type, n);
      size_t i = 0;
      for (DataVector::const_iterator conditionIt = conditions->begin(); conditionIt != conditions->end(); ++conditionIt, ++i)
      {
        DataVector::const_iterator& currentIt = it[conditionIt.getRawBoolean()];
        res->setElement(i, *currentIt);
        ++currentIt;
      }
      resultVector = res;
    }
  }
  return new DataVector(indices, resultVector);
}

DataVectorPtr TestExpression::getSubSamples(ExecutionContext& context, const ExpressionPtr& subNode, const DataTablePtr& data, const IndexSetPtr& subIndices) const
  {return subNode ? subNode->compute(context, data, subIndices) : DataVector::createConstant(subIndices, ObjectPtr());}

/*
** SequenceExpression
*/
SequenceExpression::SequenceExpression(TypePtr type, const std::vector<ExpressionPtr>& nodes)
  : Expression(type), nodes(nodes)
{
}

String SequenceExpression::toShortString() const
{
  String res = getClass()->getShortName() + "\n";
  for (size_t i = 0; i < nodes.size(); ++i)
    res += nodes[i]->toShortString() + T("\n");
  return res;
}

DataVectorPtr SequenceExpression::computeSamples(ExecutionContext& context, const DataTablePtr& data, const IndexSetPtr& indices) const
{
  std::vector<DataVectorPtr> nodeValues(nodes.size());
  for (size_t i = 0; i < nodeValues.size(); ++i)
    nodeValues[i] = nodes[i]->compute(context, data, indices);

  VectorPtr outputs = createEmptyOutputs(indices->size());
  for (size_t i = 0; i < nodeValues.size(); ++i)
    updateOutputs(outputs, nodeValues[i], i);

  return new DataVector(indices, outputs);
}

void SequenceExpression::pushNode(ExecutionContext& context, const ExpressionPtr& node, const std::vector<DataTablePtr>& cachesToUpdate)
{
  size_t index = nodes.size();
  jassert(node);
  nodes.push_back(node);

  jassertfalse; // broken
#if 0
  // update caches
  for (size_t i = 0; i < cachesToUpdate.size(); ++i)
  {
    DataTablePtr cache = cachesToUpdate[i];
    //size_t n = cache->getNumSamples();
    VectorPtr outputs = cache->getNodeCache(this);
    jassert(outputs);
    updateOutputs(outputs, cache->getSamples(context, node), index);
  }
#endif // 0
}

/*
** ScalarSumExpression
*/
ScalarSumExpression::ScalarSumExpression(const std::vector<ExpressionPtr>& nodes, bool convertToProbabilities, bool computeAverage)
  : SequenceExpression(convertToProbabilities ? probabilityType : doubleType, nodes),
  convertToProbabilities(convertToProbabilities), computeAverage(computeAverage)
{
}

ScalarSumExpression::ScalarSumExpression(bool convertToProbabilities, bool computeAverage) 
  : SequenceExpression(convertToProbabilities ? probabilityType : doubleType), convertToProbabilities(convertToProbabilities), computeAverage(computeAverage)
{
}

ObjectPtr ScalarSumExpression::compute(ExecutionContext& context, const ObjectPtr* inputs) const
{
  double res = 0.0;
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    ObjectPtr value = nodes[i]->compute(context, inputs);
    if (value)
      res += NewDouble::get(value);
  }
  if (computeAverage)
    res /= (double)nodes.size();
  return new NewDouble(res);
}

VectorPtr ScalarSumExpression::createEmptyOutputs(size_t numSamples) const
  {return new DenseDoubleVector(positiveIntegerEnumerationEnumeration, type, numSamples, 0.0);}

void ScalarSumExpression::updateOutputs(const VectorPtr& outputs, const DataVectorPtr& newNodeValues, size_t newNodeIndex) const
{
  const DenseDoubleVectorPtr& a = outputs.staticCast<DenseDoubleVector>();
  double* dest = a->getValuePointer(0);
  for (DataVector::const_iterator it = newNodeValues->begin(); it != newNodeValues->end(); ++it)
  {
    double value = it.getRawDouble();
    if (value == doubleMissingValue)
      value = 0.0;
    if (computeAverage && newNodeIndex > 0)
      *dest++ = (*dest * (newNodeIndex - 1) + value) / (double)newNodeIndex;
    else
      *dest++ += value;
  }
}

/*
** VectorSumExpression
*/
VectorSumExpression::VectorSumExpression(EnumerationPtr enumeration, bool convertToProbabilities) 
  : SequenceExpression(denseDoubleVectorClass(enumeration, doubleType)), convertToProbabilities(convertToProbabilities)
{
}

ObjectPtr VectorSumExpression::compute(ExecutionContext& context, const ObjectPtr* inputs) const
{
  ClassPtr doubleVectorClass = type;
  DenseDoubleVectorPtr res = new DenseDoubleVector(doubleVectorClass);
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    DenseDoubleVectorPtr value = nodes[i]->compute(context, inputs).staticCast<DenseDoubleVector>();
    if (value)
      value->addTo(res);
  }
  return convertToProbabilities ? convertToProbabilitiesUsingSigmoid(res) : res;
}

DataVectorPtr VectorSumExpression::computeSamples(ExecutionContext& context, const DataTablePtr& data, const IndexSetPtr& indices) const
{
  DataVectorPtr res = SequenceExpression::computeSamples(context, data, indices);
  if (convertToProbabilities)
  {
    //jassert(false); // FIXME
    std::cerr << "Warning: Probabilities not implemented yet" << std::endl;
    return res;
  }
  return res;
}

VectorPtr VectorSumExpression::createEmptyOutputs(size_t numSamples) const
{
  ClassPtr doubleVectorClass = type;
  ObjectVectorPtr res = new ObjectVector(doubleVectorClass, numSamples);
  for (size_t i = 0; i < numSamples; ++i)
    res->set(i, new DenseDoubleVector(doubleVectorClass));
  return res;
}
 
void VectorSumExpression::updateOutputs(const VectorPtr& outputs, const DataVectorPtr& newNodeValues, size_t newNodeIndex) const
{
  const ObjectVectorPtr& a = outputs.staticCast<ObjectVector>();
  jassert(newNodeValues->size() == a->getNumElements());
  jassert(newNodeValues->getElementsType()->inheritsFrom(denseDoubleVectorClass()));
  size_t i = 0;
  for (DataVector::const_iterator it = newNodeValues->begin(); it != newNodeValues->end(); ++it, ++i)
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

DenseDoubleVectorPtr VectorSumExpression::convertToProbabilitiesUsingSigmoid(const DenseDoubleVectorPtr& activations) const
{
  DenseDoubleVectorPtr probabilities = new DenseDoubleVector(DoubleVector::getElementsEnumeration(type), probabilityType);
  size_t n = activations->getNumElements();
  double Z = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    double prob = 1.0 / (1.0 + exp(-activations->getValue(i)));
    Z += prob;
    probabilities->setValue(i, prob);
  }
  if (Z)
    probabilities->multiplyByScalar(1.0 / Z);
  return probabilities;
}

/*
** CreateSparseVectorExpression
*/
CreateSparseVectorExpression::CreateSparseVectorExpression(const std::vector<ExpressionPtr>& nodes)
  : SequenceExpression(sparseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType), nodes) {}

CreateSparseVectorExpression::CreateSparseVectorExpression()
  : SequenceExpression(sparseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType)) {}

ObjectPtr CreateSparseVectorExpression::compute(ExecutionContext& context, const ObjectPtr* inputs) const
{
  SparseDoubleVectorPtr res = new SparseDoubleVector((ClassPtr)getType());
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    ObjectPtr v = nodes[i]->compute(context, inputs);
    if (v)
      res->incrementValue((size_t)NewInteger::get(v), 1.0);
  }
  return res;
}

VectorPtr CreateSparseVectorExpression::createEmptyOutputs(size_t numSamples) const
{
  ClassPtr sparseVectorClass = type;
  ObjectVectorPtr res = new ObjectVector(sparseVectorClass, numSamples);
  for (size_t i = 0; i < numSamples; ++i)
    res->set(i, new SparseDoubleVector(sparseVectorClass));
  return res;
}

void CreateSparseVectorExpression::updateOutputs(const VectorPtr& outputs, const DataVectorPtr& newNodeValues, size_t newNodeIndex) const
{ 
  const ObjectVectorPtr& a = outputs.staticCast<ObjectVector>();
  jassert(newNodeValues->size() == a->getNumElements());
  jassert(newNodeValues->getElementsType() == positiveIntegerType);
  size_t i = 0;
  for (DataVector::const_iterator it = newNodeValues->begin(); it != newNodeValues->end(); ++it, ++i)
  {
    int newNodeValue = it.getRawInteger();
    if (newNodeValue >= 0)
      a->getAndCast<SparseDoubleVector>(i)->incrementValue((size_t)newNodeValue, 1.0);
  }
}
