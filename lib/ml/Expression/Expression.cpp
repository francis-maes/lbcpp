/*-----------------------------------------.---------------------------------.
| Filename: Expression.cpp                 | Expression classes              |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <ml/Expression.h>
#include <oil/Core/Table.h>
#include <algorithm>
using namespace lbcpp;

/*
** DataVector
*/
DataVector::DataVector(Implementation implementation, const IndexSetPtr& indices, const ClassPtr& elementsType)
  : implementation(implementation), indices(indices), elementsType(elementsType), constantRawBoolean(2) {}

DataVector::DataVector(const IndexSetPtr& indices, const VectorPtr& ownedVector)
  : implementation(ownedVectorImpl), indices(indices), elementsType(ownedVector->getElementsType()), constantRawBoolean(2), vector(ownedVector) {}

DataVector::DataVector() : implementation(noImpl), constantRawBoolean(2)
{
}

DataVectorPtr DataVector::createConstant(IndexSetPtr indices, const ObjectPtr& constantValue)
{
  DataVectorPtr res(new DataVector(constantValueImpl, indices, constantValue->getClass()));
  res->constantRawBoolean = constantValue.dynamicCast<Boolean>() ? Boolean::get(constantValue) : 2;
  res->constantRawDouble = constantValue.dynamicCast<Double>() ? Double::get(constantValue) : 0.0;
  res->constantRawObject = constantValue;
  return res;
}

ObjectPtr DataVector::sampleElement(RandomGeneratorPtr random) const
{
  if (implementation == constantValueImpl)
    return constantRawObject;
  else
    return vector->getElement(random->sampleSize(vector->getNumElements()));
}

DataVectorPtr DataVector::createCached(IndexSetPtr indices, const VectorPtr& cachedVector)
{
  DataVectorPtr res(new DataVector(cachedVectorImpl, indices, cachedVector->getElementsType()));
  res->vector = cachedVector;
  return res;
}

/*
** Expression
*/
static size_t makeExpressionAllocationIndex()
{
  static size_t res = 0;
  return res++; // warning: not safe for multi-threading ...
}

Expression::Expression(const ClassPtr& type)
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

static ExpressionPtr getNodeByTreeIndexRec(ExpressionPtr expression, size_t& index)
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

DataVectorPtr Expression::compute(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
{
  IndexSetPtr idx = indices ? indices : new IndexSet(0, data->getNumRows());
  VectorPtr samples = data->getDataByKey(refCountedPointerFromThis(this));
  if (samples)
    return DataVector::createCached(idx, samples);
  else
    return computeSamples(context, data, idx);
}

/*
** VariableExpression
*/
VariableExpression::VariableExpression(const ClassPtr& type, const string& name, size_t inputIndex)
  : Expression(type), name(name), inputIndex(inputIndex)
{
}

VariableExpression::VariableExpression() : inputIndex(0)
{
}

string VariableExpression::toShortString() const
  {return name;}

ObjectPtr VariableExpression::compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
  {return inputs[inputIndex];}

DataVectorPtr VariableExpression::computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
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

string ConstantExpression::toShortString() const
  {return value->toShortString();}

ObjectPtr ConstantExpression::compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
  {return value;}

DataVectorPtr ConstantExpression::computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
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

string FunctionExpression::toShortString() const
  {return function->makeNodeName(arguments);}

void FunctionExpression::initialize()
{
  size_t numInputs = function->getNumInputs();
  jassert(arguments.size() == numInputs);
  ClassPtr* inputTypes = new ClassPtr[numInputs];
  for (size_t i = 0; i < numInputs; ++i)
  {
    jassert(arguments[i]);
    inputTypes[i] = arguments[i]->getType();
    jassert(function->doAcceptInputType(i, inputTypes[i]));
  }

  type = function->initialize(inputTypes);
  delete [] inputTypes;
}

ObjectPtr FunctionExpression::compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
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

DataVectorPtr FunctionExpression::computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
{
  std::vector<DataVectorPtr> inputs(arguments.size());
  for (size_t i = 0; i < inputs.size(); ++i)
    inputs[i] = arguments[i]->compute(context, data, indices);
  return function->compute(context, inputs, type);
}

/*
** AggregatorExpression
*/
AggregatorExpression::AggregatorExpression(AggregatorPtr aggregator, const std::vector<ExpressionPtr>& nodes)
  : aggregator(aggregator), nodes(nodes)
{
  type = aggregator->initialize(nodes[0]->getType());
}

AggregatorExpression::AggregatorExpression(AggregatorPtr aggregator, ClassPtr type)
  : Expression(type), aggregator(aggregator)
{
}

string AggregatorExpression::toShortString() const
{
  string res = getClass()->getShortName() + "\n";
  for (size_t i = 0; i < nodes.size(); ++i)
    res += nodes[i]->toShortString() + T("\n");
  return res;
}

ObjectPtr AggregatorExpression::compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
{
  std::vector<ObjectPtr> inputValues(nodes.size());
  for (size_t i = 0; i < nodes.size(); ++i)
    inputValues[i] = nodes[i]->compute(context, inputs);
  return aggregator->compute(context, inputValues, type);
}

DataVectorPtr AggregatorExpression::computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
{
  std::vector<DataVectorPtr> nodeValues(nodes.size());
  for (size_t i = 0; i < nodeValues.size(); ++i)
    nodeValues[i] = nodes[i]->compute(context, data, indices);
  return aggregator->compute(context, nodeValues, type);
}

/*
** TestExpression
*/
TestExpression::TestExpression(const ExpressionPtr& conditionNode, const ExpressionPtr& failureNode, const ExpressionPtr& successNode, const ExpressionPtr& missingNode)
  : Expression(successNode->getType()), conditionNode(conditionNode), failureNode(failureNode), successNode(successNode), missingNode(missingNode)
{
}

TestExpression::TestExpression(const ExpressionPtr& conditionNode, ClassPtr outputType)
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

string TestExpression::toShortString() const
{
  string res = "(" + conditionNode->toShortString() + " ? " + 
    (successNode ? successNode->toShortString() : T("NULL")) + T(" : ") + 
    (failureNode ? failureNode->toShortString() : T("NULL"));
  if (missingNode && missingNode.isInstanceOf<ConstantExpression>() && missingNode.staticCast<ConstantExpression>()->getValue().exists())
    res += T(" : ") + missingNode->toShortString();
  return res + T(")");
}

ObjectPtr TestExpression::compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs) const
{
  ObjectPtr condition = conditionNode->compute(context, inputs);
  ExpressionPtr subNode = (condition ? (Boolean::get(condition) ? successNode : failureNode) : missingNode);
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

DataVectorPtr TestExpression::computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
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
    
    if (v[0].dynamicCast<Double>() && v[1].dynamicCast<Double>() && v[2].dynamicCast<Double>())
    {
      double dv[3];
      dv[0] = Double::get(v[0]);
      dv[1] = Double::get(v[1]);
      dv[2] = Double::get(v[2]);
      DVectorPtr res = new DVector(type, n, 0.0);
      double* ptr = res->getDataPointer();
      for (DataVector::const_iterator it = conditions->begin(); it != conditions->end(); ++it)
        *ptr++ = dv[it.getRawBoolean()];
      resultVector = res;
    }
    else
    {
      OVectorPtr res = new OVector(type, n);
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
    
    ClassPtr elementsType = subValues[0]->getElementsType();
    jassert(subValues[1]->getElementsType() == elementsType);
    jassert(subValues[2]->getElementsType() == elementsType);

    DataVector::const_iterator it[3];
    it[0] = subValues[0]->begin();
    it[1] = subValues[1]->begin();
    it[2] = subValues[2]->begin();

    if (elementsType == doubleClass)
    {
      DVectorPtr res = new DVector(type, n, 0.0);
      double* ptr = res->getDataPointer();
      for (DataVector::const_iterator conditionIt = conditions->begin(); conditionIt != conditions->end(); ++conditionIt)
      {
        DataVector::const_iterator& currentIt = it[conditionIt.getRawBoolean()];
        *ptr++ = currentIt.getRawDouble();
        ++currentIt;
      }
      resultVector = res;
    }
    else
    {
      jassert(elementsType->inheritsFrom(objectClass));
      OVectorPtr res = new OVector(type, n);
      size_t i = 0;
      for (DataVector::const_iterator conditionIt = conditions->begin(); conditionIt != conditions->end(); ++conditionIt, ++i)
      {
        DataVector::const_iterator& currentIt = it[conditionIt.getRawBoolean()];
        res->set(i, currentIt.getRawObject());
        ++currentIt;
      }
      resultVector = res;
    }
  }
  return new DataVector(indices, resultVector);
}

DataVectorPtr TestExpression::getSubSamples(ExecutionContext& context, const ExpressionPtr& subNode, const TablePtr& data, const IndexSetPtr& subIndices) const
  {return subNode && subIndices->size() ? subNode->compute(context, data, subIndices) : DataVector::createConstant(subIndices, getType()->createObject(context));}

ObjectPtr LinearModelExpression::compute(ExecutionContext &context, const std::vector<ObjectPtr>& inputs) const
{
  if (weights->getNumValues() == 0)
    return new Double(0.0);
  jassert(inputs.size() + 1 == weights->getNumValues());
  double result = weights->getValue(0);
  for (size_t i = 0; i < inputs.size(); ++i)
    result += weights->getValue(i+1) * Double::get(inputs[i]);
  return new Double(result);
}

DataVectorPtr LinearModelExpression::computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
{
  DVectorPtr vector = new DVector(indices->size());
  size_t i = 0;
  for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    vector->set(i++, Double::get(compute(context, data->getRow(*it))));
  return new DataVector(indices, vector);
}

DataVectorPtr PerceptronExpression::computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
{
  DVectorPtr vector = new DVector(indices->size());
  size_t i = 0;
  for (IndexSet::const_iterator it = indices->begin(); it != indices->end(); ++it)
    vector->set(i++, Double::get(compute(context, data->getRow(*it))));
  return new DataVector(indices, vector);
}

void PerceptronExpression::updateStatistics(ExecutionContext& context, const DenseDoubleVectorPtr& inputs)
{
  if (statistics.empty())
  {
    DenseDoubleVectorPtr& weights = model->getWeights();
    // This is the first sample, initialize the statistics
    statistics.resize(inputs->getNumValues());
    normalizedInput.resize(inputs->getNumValues());
    weights = new DenseDoubleVector(inputs->getNumValues() + 1, 0.0);

    for (size_t i = 0; i < statistics.size(); ++i)
    {
      statistics[i] = new ScalarVariableMeanAndVariance();
      normalizedInput[i] = new Double();
    }
    for (size_t i = 0; i < weights->getNumValues(); ++i)
      weights->setValue(i, context.getRandomGenerator()->sampleDouble(-1.0, 1.0));
  }
  for (size_t i = 0; i < inputs->getNumValues(); ++i)
    statistics[i]->push(inputs->getValue(i));
}

/** Calculate the normalization of an input vector based on a training sample
 *  \param sample The training sample input: a vector of length \f$n\f$.
 *  \return A DenseDoubleVectorPtr of \f$n\f$ elements where the elements are
 *          the normalized input vector.
 */
DenseDoubleVectorPtr PerceptronExpression::normalizeInput(const DenseDoubleVectorPtr& sample) const
{
  DenseDoubleVectorPtr result = new DenseDoubleVector(sample->getNumValues(), 1.0);
  for (size_t i = 0; i < result->getNumValues(); ++i)
    result->setValue(i, normalize(sample->getValue(i), statistics[i]->getMean(), statistics[i]->getStandardDeviation()));
  return result;
}


TreeNodePtr HoeffdingTreeNode::findLeaf(const ObjectPtr &input) const
{
  if (isLeaf())
    return refCountedPointerFromThis(this);
  DenseDoubleVectorPtr inputVector = input.staticCast<DenseDoubleVector>();
  if (inputVector->getValue(testVariable) > testThreshold)
    return right->findLeaf(input);
  else
    return left->findLeaf(input);
}

#include <iomanip> // std::setw

void HoeffdingTreeNode::pprint(int indent) const
  {
	  if(isLeaf()){
		if (indent) std::cout << std::setw(indent) << ' ';
		for(unsigned i = 0; i < linearModel->getWeights()->getNumValues(); i++)
			{std::cout << "y= " << linearModel->getWeights()->getValue(i) << "*x" << i << " + ";}
	  }
	  else{
		if (indent) std::cout << std::setw(indent) << ' ';
		HoeffdingTreeNode test;
		std::cout<< "x" << testVariable << "<=" << testThreshold << std::endl;
		std::cout << "left:" << std::endl;
		(((HoeffdingTreeNodePtr)left).staticCast<HoeffdingTreeNode>())->pprint(indent+4);
		std::cout << "right:" << std::endl;
		(((HoeffdingTreeNodePtr)right).staticCast<HoeffdingTreeNode>())->pprint(indent+4);
	  }
}

int HoeffdingTreeNode::getNbOfLeaves() const
  {
	  if(isLeaf())
		  return 1;
	  else
		return (((HoeffdingTreeNodePtr)left).staticCast<HoeffdingTreeNode>())->getNbOfLeaves()+(((HoeffdingTreeNodePtr)right).staticCast<HoeffdingTreeNode>())->getNbOfLeaves();
}

DenseDoubleVectorPtr HoeffdingTreeNode::getSplits() const
{
	if(isLeaf()){
		DoubleVectorPtr none;
		return none;
	}
	else {
		DenseDoubleVectorPtr leftSplits = left.staticCast<HoeffdingTreeNode>()->getSplits();
		DenseDoubleVectorPtr rightSplits = right.staticCast<HoeffdingTreeNode>()->getSplits();
		DenseDoubleVectorPtr splits = new DenseDoubleVector(leftSplits->getNumValues() + rightSplits->getNumValues() + 1, 1.0);
		for (size_t i = 0; i < leftSplits->getNumValues(); ++i)
			splits->setValue(i, leftSplits->getValue(i));
		size_t offset = leftSplits->getNumValues();
		for (size_t i = 0; i < rightSplits->getNumValues(); ++i)
			splits->setValue(offset + i, rightSplits->getValue(i));
		splits->setValue(offset + rightSplits->getNumValues(), testThreshold);
		return splits;
	}
}

void HoeffdingTreeNode::split(ExecutionContext& context, size_t testVariable, double testThreshold)
{
  
}

size_t HoeffdingTreeNode::getNumSamples() const
{
  return 0;
}

ObjectPtr HoeffdingTreeNode::getSampleInput(size_t index) const
{
	return ObjectPtr();
}

ObjectPtr HoeffdingTreeNode::getSamplePrediction(size_t index) const
{
	return ObjectPtr();
}


ObjectPtr HoeffdingTreeNode::compute(ExecutionContext &context, const std::vector<ObjectPtr>& inputs) const
{
  return ObjectPtr();
}

DataVectorPtr HoeffdingTreeNode::computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const
{
  return DataVectorPtr();
}

