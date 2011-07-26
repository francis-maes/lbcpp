/*-----------------------------------------.---------------------------------.
| Filename: BinaryDecisionTreeBatchLearner.h| Extra Tree Batch Learner       |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Function/Predicate.h>
#include <lbcpp/Core/Vector.h>
#include "BinaryDecisionTreeBatchLearner.h"
using namespace lbcpp;

/*
** BinaryDecisionTreeFunction
*/
BinaryDecisionTreeFunction::BinaryDecisionTreeFunction()
  {setBatchLearner(filterUnsupervisedExamplesBatchLearner(new BinaryDecisionTreeBatchLearner(10, 5)));}

/*
** BinaryDecisionTreeBatchLearner
*/
bool BinaryDecisionTreeBatchLearner::train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
{
  const BinaryDecisionTreeFunctionPtr& treeFunction = function.dynamicCast<BinaryDecisionTreeFunction>();

  if (!checkHasAtLeastOneExemples(trainingData))
  {
    context.errorCallback(T("No training examples"));
    return false;
  }

  size_t n = trainingData.size();
  TypePtr inputType = function->getInputsClass()->getMemberVariable(0)->getType();
  size_t numAttributes = inputType->getNumMemberVariables();
  std::vector< std::vector<Variable> > attributes;
  std::vector<Variable> labels;
  attributes.reserve(n);
  labels.reserve(n);
  for (size_t i = 0; i < n; ++i)
  {
    std::vector<Variable> example(numAttributes, Variable());
    for (size_t j = 0; j < numAttributes; ++j)
      example[j] = trainingData[i]->getVariable(0).getObject()->getVariable(j);
    attributes.push_back(example);
    labels.push_back(trainingData[i]->getVariable(1));
  }
  
  std::vector<size_t> indices(n, 0);
  for (size_t i = 1; i < n; ++i)
    indices[i] = i;

  DecisionTreeExampleVector examples(attributes, labels, indices);
  const_cast<BinaryDecisionTreeBatchLearner* >(this)->random = new RandomGenerator(/* FIXME: determistic seed */);

  BinaryDecisionTreePtr tree = sampleTree(context, inputType, function->getOutputType(), examples);
  jassert(tree);

  context.resultCallback(T("Num Attributes"), function->getInputsClass()->getNumMemberVariables()); // FIXME getInputType = anyType (@see BinaryDecisionTreeInference)
  context.resultCallback(T("Num Active Attributes"), numActiveAttributes);
  context.resultCallback(T("K"), numAttributeSamplesPerSplit);
  context.resultCallback(T("Num Examples"), n);
  context.resultCallback(T("Num Nodes"), tree->getNumNodes());

  treeFunction->setTree(tree);

  return true;
}

bool BinaryDecisionTreeBatchLearner::shouldCreateLeaf(ExecutionContext& context,
                                                       const DecisionTreeExampleVector& examples,
                                                       const std::vector<size_t>& variables,
                                                       TypePtr outputType, Variable& leafValue) const
{
  size_t n = examples.getNumExamples();
  jassert(n);

  if (examples.isLabelConstant(leafValue))
    return true;
  
  if (n >= minimumSizeForSplitting)
    return false;
  
  if (n == 1)
  {
    if (outputType->inheritsFrom(enumValueType))
    {
      SparseDoubleVectorPtr res = new SparseDoubleVector(outputType, doubleType);
      res->appendValue(examples.getLabel(0).getInteger(), 1.0);
      leafValue = res;
    }
    else if (outputType->inheritsFrom(booleanType))
      leafValue = Variable(examples.getLabel(0).getDouble(), probabilityType);
    else if (outputType->inheritsFrom(doubleType))
      leafValue = Variable(examples.getLabel(0).getDouble(), doubleType);
    else
    {
      context.errorCallback(T("BinaryDecisionTreeBatchLearner::shouldCreateLeaf"), T("Output type not implemented: ") + outputType->toString().quoted());
      return false;
    }
    return true;
  }
  // there are more than one example
  if (outputType->inheritsFrom(enumValueType))
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(outputType, doubleType);
    for (size_t i = 0; i < n; ++i)
      res->getValueReference(examples.getLabel(i).getInteger()) += 1.0;
    size_t numElements = res->getNumElements();
    for (size_t i = 0; i < numElements; ++i)
      res->getValueReference(i) /= n;
    leafValue = res;
  }
  else if (outputType->inheritsFrom(doubleType))
  {
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += examples.getLabel(i).getDouble();
    res /= n;
    if (outputType->inheritsFrom(sumType(booleanType, probabilityType)))
      leafValue = Variable(res, probabilityType);
    else
      leafValue = Variable(res, doubleType);
  }
  else
  {
    context.errorCallback(T("BinaryDecisionTreeBatchLearner::shouldCreateLeaf"), T("Output type not implemented: ") + outputType->toString().quoted());
    return false;
  }
  return true;
}

void BinaryDecisionTreeBatchLearner::sampleTreeRecursively(ExecutionContext& context,
                                                            const BinaryDecisionTreePtr& tree, size_t nodeIndex,
                                                            TypePtr inputType, TypePtr outputType,
                                                            const DecisionTreeExampleVector& examples,
                                                            const std::vector<size_t>& variables,
                                                            std::vector<Split>& bestSplits,
                                                            size_t& numLeaves, size_t numExamples) const
{
  jassert(examples.getNumExamples());

  // update "non constant variables" set
  std::vector<size_t> nonConstantVariables;
  nonConstantVariables.reserve(variables.size());
  for (size_t i = 0; i < variables.size(); ++i)
  {
    if (!examples.isAttributeConstant(variables[i]))
      nonConstantVariables.push_back(variables[i]);
  }
  
  if (nodeIndex == 0)
    const_cast<BinaryDecisionTreeBatchLearner* >(this)->numActiveAttributes = nonConstantVariables.size();

  Variable leafValue;
  if (shouldCreateLeaf(context, examples, nonConstantVariables, outputType, leafValue))
  {
    tree->createLeaf(nodeIndex, leafValue);
    numLeaves += examples.getNumExamples();
    context.progressCallback(new ProgressionState((double)numLeaves, (double)numExamples, T("Leaves")));
    return;
  }

  // select K split variables
  std::vector<size_t> splitVariables;
  if (numAttributeSamplesPerSplit >= nonConstantVariables.size())
    splitVariables = nonConstantVariables;
  else
    context.getRandomGenerator()->sampleSubset(nonConstantVariables, numAttributeSamplesPerSplit, splitVariables); 
  size_t K = splitVariables.size();

  // generate split predicates, score them, and keep the best one
  double bestSplitScore = -DBL_MAX;
  for (size_t i = 0; i < K; ++i)
  {
    BinaryDecisionTreeSplitterPtr splitter = tree->getSplitter(splitVariables[i]);
    Variable splitArgument = splitter->sampleSplit(examples);
    PredicatePtr splitPredicate = splitter->getSplitPredicate(splitArgument);

    std::vector<size_t> leftExamples, rightExamples;
    double splitScore = splitter->computeSplitScore(context, examples, leftExamples, rightExamples, splitPredicate);

    jassert(leftExamples.size());
    jassert(rightExamples.size());
    jassert(leftExamples.size() + rightExamples.size() == examples.getNumExamples());

    if (splitScore > bestSplitScore)
    {
      bestSplits.clear();
      bestSplitScore = splitScore;
    }
    if (splitScore >= bestSplitScore)
    {
      Split s = {
        splitVariables[i],
        splitArgument,
        leftExamples,
        rightExamples
      };

      bestSplits.push_back(s);
    }
  }
  jassert(bestSplits.size());
  int bestIndex = context.getRandomGenerator()->sampleInt(0, (int)bestSplits.size());
  Split selectedSplit = bestSplits[bestIndex];

  // allocate child nodes
  size_t leftChildIndex = tree->allocateNodes(2);
  // create the node
  tree->createInternalNode(nodeIndex, selectedSplit.variableIndex, selectedSplit.argument, leftChildIndex);
  // call recursively
  sampleTreeRecursively(context, tree, leftChildIndex, inputType, outputType, examples.subset(selectedSplit.left), nonConstantVariables, bestSplits, numLeaves, numExamples);
  sampleTreeRecursively(context, tree, leftChildIndex + 1, inputType, outputType, examples.subset(selectedSplit.right), nonConstantVariables, bestSplits, numLeaves, numExamples);
}

BinaryDecisionTreePtr BinaryDecisionTreeBatchLearner::sampleTree(ExecutionContext& context, TypePtr inputClass, TypePtr outputClass, const DecisionTreeExampleVector& examples) const
{
  size_t n = examples.getNumExamples();
  if (!n)
    return BinaryDecisionTreePtr();

  size_t numInputVariables = inputClass->getNumMemberVariables();
  // we start with all variables
  std::vector<size_t> variables(numInputVariables);
  for (size_t i = 0; i < variables.size(); ++i)
    variables[i] = i;

  // create the initial binary decision tree
  BinaryDecisionTreePtr res = new BinaryDecisionTree(numInputVariables);
  res->reserveNodes(n);
  size_t nodeIndex = res->allocateNodes(1);
  jassert(nodeIndex == 0);

  // create splitters
  for (size_t i = 0; i < numInputVariables; ++i)
    res->setSplitter(i, getBinaryDecisionTreeSplitter(inputClass->getMemberVariableType(i), outputClass, i));

  // sample tree recursively
  std::vector<Split> bestSplits;
  size_t numLeaves = 0;
  size_t numExamples = n;
  sampleTreeRecursively(context, res, nodeIndex, inputClass, outputClass,
                        examples, variables, bestSplits, numLeaves, numExamples);
  return res;
}

BinaryDecisionTreeSplitterPtr BinaryDecisionTreeBatchLearner::getBinaryDecisionTreeSplitter(TypePtr inputType, TypePtr outputType, size_t variableIndex) const
{
  SplitScoringFunctionPtr scoringFunction;
  if (outputType->inheritsFrom(enumValueType))
    scoringFunction = new ClassificationIGSplitScoringFunction();
  else if (outputType->inheritsFrom(booleanType))
    scoringFunction = new BinaryIGSplitScoringFunction();
  else if (outputType->inheritsFrom(doubleType))
    scoringFunction = new RegressionIGSplitScoringFunction();
  else
  {
    jassertfalse;
    return BinaryDecisionTreeSplitterPtr();
  }
  
  if (inputType->inheritsFrom(enumValueType))
    return new EnumerationBinaryDecisionTreeSplitter(scoringFunction, random, variableIndex);
  else if (inputType->inheritsFrom(booleanType))
    return new BooleanBinaryDecisionTreeSplitter(scoringFunction, random, variableIndex);
  else if (inputType->inheritsFrom(integerType))
    return new IntegerBinaryDecisionTreeSplitter(scoringFunction, random, variableIndex);
  else if (inputType->inheritsFrom(doubleType))
    return new DoubleBinaryDecisionTreeSplitter(scoringFunction, random, variableIndex);

  jassertfalse;
  return BinaryDecisionTreeSplitterPtr();
}
