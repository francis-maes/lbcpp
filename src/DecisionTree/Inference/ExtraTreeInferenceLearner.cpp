/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Function/Predicate.h>
#include <lbcpp/Distribution/Distribution.h>
#include <lbcpp/Core/Vector.h>
#include "ExtraTreeInferenceLearner.h"

using namespace lbcpp;

/*
** SingleExtraTreeInferenceLearner
*/
SingleExtraTreeInferenceLearner::SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting, DistributionBuilderPtr builder)
  : random(new RandomGenerator()),
    numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
    minimumSizeForSplitting(minimumSizeForSplitting), builder(builder) {}

Variable SingleExtraTreeInferenceLearner::computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
{
  const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
  jassert(learnerInput);
  const BinaryDecisionTreeInferencePtr& inference = learnerInput->getTargetInference();
  jassert(inference);

  if (!learnerInput->getNumTrainingExamples())
    return Variable();

  TypePtr outputType = learnerInput->getTrainingExample(0).second.getType();
  PerceptionPtr perception = inference->getPerception();
  TypePtr pairType = pairClass(perception->getOutputType(), outputType);
  VectorPtr newTrainingData = vector(pairType, learnerInput->getNumTrainingExamples());
  for (size_t i = 0; i < newTrainingData->getNumElements(); ++i)
  {
    const std::pair<Variable, Variable>& example = learnerInput->getTrainingExample(i);
    newTrainingData->setElement(i, Variable::pair(perception->computeFunction(context, example.first), example.second, pairType));
  }

  BinaryDecisionTreePtr tree = sampleTree(context, perception->getOutputType(), outputType, newTrainingData);
  if (tree)
  {
    context.resultCallback(T("Num Attributes"), perception->getNumOutputVariables());
    context.resultCallback(T("K"), numAttributeSamplesPerSplit);
    context.resultCallback(T("Num Examples"), learnerInput->getNumTrainingExamples());
    context.resultCallback(T("Num Nodes"), tree->getNumNodes());
/*
    context.informationCallback(T("Tree: numAttributes = ") + String((int)perception->getNumOutputVariables()) +
          T(" k = ") + String((int)numAttributeSamplesPerSplit) +
          T(" numExamples = ") + String((int)learnerInput->getNumTrainingExamples()) +
          T(" numNodes = ") + String((int)tree->getNumNodes()));*/
    inference->setTree(tree);
  }
  return Variable();
}

static bool isVariableConstant(ContainerPtr container, size_t index1, int index2, Variable& constantValue)
{
  size_t n = container->getNumElements();
  if (n <= 1)
    return true;
  constantValue = Variable();
  for (size_t i = 0; i < n; ++i)
  {
    Variable value = container->getElement(i)[index1];
    if (index2 >= 0)
      value = value.getObject()->getVariable((size_t)index2);
    if (!value.exists())
      continue;
    if (!constantValue.exists())
      constantValue = value;
    else if (constantValue != value)
      return false;
  }
  return true;
}

static bool isInputVariableConstant(ContainerPtr trainingData, size_t variableIndex, Variable& value)
  {return isVariableConstant(trainingData, 0, (int)variableIndex, value);}

static bool isOutputConstant(ContainerPtr trainingData, Variable& value)
  {return isVariableConstant(trainingData, 1, -1, value);}

bool SingleExtraTreeInferenceLearner::shouldCreateLeaf(ExecutionContext& context, ContainerPtr trainingData, const std::vector<size_t>& variables, TypePtr outputType, Variable& leafValue) const
{
  size_t n = trainingData->getNumElements();
  jassert(n);

  if (n >= minimumSizeForSplitting && variables.size())
  {
    if (isOutputConstant(trainingData, leafValue))
    {
      builder->clear();
      builder->addElement(leafValue);
      leafValue = builder->build(context);
      jassert(leafValue.exists());
      return true;
    }
    return false;
  }

  builder->clear();
  for (size_t i = 0; i < n; ++i)
    builder->addElement(trainingData->getElement(i)[1]);
  leafValue = builder->build(context);
  jassert(leafValue.exists());
  return true;
}

void SingleExtraTreeInferenceLearner::sampleTreeRecursively(ExecutionContext& context,
                                                            BinaryDecisionTreePtr tree, size_t nodeIndex,
                                                            TypePtr inputType, TypePtr outputType,
                                                            ContainerPtr trainingData, const std::vector<size_t>& variables,
                                                            std::vector<Split>& bestSplits,
                                                            size_t& numLeaves, size_t numExamples) const
{
  jassert(trainingData->getNumElements());

  // update "non constant variables" set
  std::vector<size_t> nonConstantVariables;
  nonConstantVariables.reserve(variables.size());
  for (size_t i = 0; i < variables.size(); ++i)
  {
    Variable value;
    if (!isInputVariableConstant(trainingData, variables[i], value))
      nonConstantVariables.push_back(variables[i]);
  }

  Variable leafValue;
  if (shouldCreateLeaf(context, trainingData, nonConstantVariables, outputType, leafValue))
  {
    tree->createLeaf(nodeIndex, leafValue);
    ++numLeaves;
    context.progressCallback((double)numLeaves, (double)numExamples, T("Leaves"));
    return;
  }

  // select K split variables
  std::vector<size_t> splitVariables;
  if (numAttributeSamplesPerSplit >= nonConstantVariables.size())
    splitVariables = nonConstantVariables;
  else
    RandomGenerator::getInstance()->sampleSubset(nonConstantVariables, numAttributeSamplesPerSplit, splitVariables); 
  size_t K = splitVariables.size();

  // generate split predicates, score them, and keep the best one
  double bestSplitScore = -DBL_MAX;
  for (size_t i = 0; i < K; ++i)
  {
    BinaryDecisionTreeSplitterPtr splitter = tree->getSplitter(splitVariables[i]);
    Variable splitArgument = splitter->sampleSplit(trainingData);
    PredicatePtr splitPredicate = splitter->getSplitPredicate(splitArgument);

    ContainerPtr negativeExamples, positiveExamples;
    double splitScore = splitter->computeSplitScore(context, trainingData, negativeExamples, positiveExamples, splitPredicate);

    jassert(negativeExamples->getNumElements() + positiveExamples->getNumElements() == trainingData->getNumElements());

/*    std::cout << splitPredicate->toString() << "\t score: " << splitScore;
    std::cout << "   nbPos: " << positiveExamples->getNumElements();
    std::cout << "   nbNeg: " << negativeExamples->getNumElements() << std::endl;
*/
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
        positiveExamples,
        negativeExamples
      };

      bestSplits.push_back(s);
    }
  }
  jassert(bestSplits.size());
  int bestIndex = RandomGenerator::getInstance()->sampleInt(0, (int)bestSplits.size());
  Split selectedSplit = bestSplits[bestIndex];
  //std::cout << "Best: " << selectedSplit.argument.toString() << std::endl;
  // allocate child nodes
  size_t leftChildIndex = tree->allocateNodes(2);

  // create the node
  tree->createInternalNode(nodeIndex, selectedSplit.variableIndex, selectedSplit.argument, leftChildIndex);

  // call recursively
  sampleTreeRecursively(context, tree, leftChildIndex, inputType, outputType, selectedSplit.negative, nonConstantVariables, bestSplits, numLeaves, numExamples);
  sampleTreeRecursively(context, tree, leftChildIndex + 1, inputType, outputType, selectedSplit.positive, nonConstantVariables, bestSplits, numLeaves, numExamples);
}

BinaryDecisionTreePtr SingleExtraTreeInferenceLearner::sampleTree(ExecutionContext& context, TypePtr inputClass, TypePtr outputClass, ContainerPtr trainingData) const
{
  size_t n = trainingData->getNumElements();
  if (!n)
    return BinaryDecisionTreePtr();

  size_t numInputVariables = inputClass->getObjectNumVariables();
  // we start with all variables
  std::vector<size_t> variables(numInputVariables);
  for (size_t i = 0; i < variables.size(); ++i)
    variables[i] = i;

  // create the initial binary decision tree
  BinaryDecisionTreePtr res = new BinaryDecisionTree(numInputVariables);
  res->reserveNodes(n);
  size_t nodeIndex = res->allocateNodes(1);
  jassert(nodeIndex == 0);

  for (size_t i = 0; i < numInputVariables; ++i)
    res->setSplitter(i, getBinaryDecisionTreeSplitter(inputClass->getObjectVariableType(i), outputClass, i));

  // sample tree recursively
  std::vector<Split> bestSplits;
  size_t numLeaves = 0;
  size_t numExamples = n;
  sampleTreeRecursively(context, res, nodeIndex, inputClass, outputClass, trainingData, variables, bestSplits, numLeaves, numExamples);

  return res;
}

BinaryDecisionTreeSplitterPtr SingleExtraTreeInferenceLearner::getBinaryDecisionTreeSplitter(TypePtr inputType, TypePtr outputType, size_t variableIndex) const
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
    return new IntegereBinaryDecisionTreeSplitter(scoringFunction, random, variableIndex);
  else if (inputType->inheritsFrom(doubleType))
    return new DoubleBinaryDecisionTreeSplitter(scoringFunction, random, variableIndex);

  jassertfalse;
  return BinaryDecisionTreeSplitterPtr();
}
