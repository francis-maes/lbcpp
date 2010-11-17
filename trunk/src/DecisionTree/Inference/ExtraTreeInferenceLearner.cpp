/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Function/Predicate.h>
#include <lbcpp/Data/ProbabilityDistribution.h>
#include <lbcpp/Data/Vector.h>
#include "ExtraTreeInferenceLearner.h"
using namespace lbcpp;

/*
** SingleExtraTreeInferenceLearner
*/
SingleExtraTreeInferenceLearner::SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : Inference(T("SingleExtraTreeInferenceLearner")), random(new RandomGenerator),
    numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
    minimumSizeForSplitting(minimumSizeForSplitting) {}

Variable SingleExtraTreeInferenceLearner::run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>();
  jassert(learnerInput);
  const BinaryDecisionTreeInferencePtr& inference = learnerInput->getTargetInference();
  jassert(inference);

  if (!learnerInput->getNumTrainingExamples())
    return Variable();

  TypePtr outputType = learnerInput->getTrainingExample(0).second.getType();
  PerceptionPtr perception = inference->getPerception();
  VectorPtr newTrainingData = vector(pairClass(perception->getOutputType(), outputType), learnerInput->getNumTrainingExamples());
  for (size_t i = 0; i < newTrainingData->getNumElements(); ++i)
  {
    const std::pair<Variable, Variable>& example = learnerInput->getTrainingExample(i);
    newTrainingData->setElement(i, Variable::pair(perception->compute(example.first), example.second));
  }

  BinaryDecisionTreePtr tree = sampleTree(perception->getOutputType(), outputType, newTrainingData);
  if (tree)
  {
    MessageCallback::info(T("Tree: numAttributes = ") + String((int)perception->getNumOutputVariables()) +
          T(" k = ") + String((int)numAttributeSamplesPerSplit) +
          T(" numExamples = ") + String((int)learnerInput->getNumTrainingExamples()) +
          T(" numNodes = ") + String((int)tree->getNumNodes()));
    inference->setTree(tree);
  }
  return Variable();
}

static Variable getInputVariableFromExample(const Variable& example, size_t variableIndex)
  {return example[0].getObject()->getVariable(variableIndex);}

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

bool SingleExtraTreeInferenceLearner::shouldCreateLeaf(ContainerPtr trainingData, const std::vector<size_t>& variables, TypePtr outputType, Variable& leafValue) const
{
  size_t n = trainingData->getNumElements();
  jassert(n);
  
  if (n == 1)
  {
    leafValue = trainingData->getElement(0)[1];
    return true;
  }

  if (n >= minimumSizeForSplitting && variables.size())
    return isOutputConstant(trainingData, leafValue);

  // FIXME: create distribution instead of the most represented output 
  if (outputType->inheritsFrom(doubleType))
  {
    double sum = 0;
    size_t count = 0;
    for (size_t i = 0; i < n; ++i)
    {
      Variable vote = trainingData->getElement(i)[1];
      if (vote.exists())
      {
        ++count;
        sum += vote.getDouble();
      }
    }
    leafValue = count ? Variable(sum / (double)count, doubleType) : Variable::missingValue(doubleType);
  }
  else if (outputType->inheritsFrom(enumValueType))
  {
    EnumerationPtr enumeration = outputType.dynamicCast<Enumeration>();
    std::vector<size_t> vote(enumeration->getNumElements(), 0);
    for (size_t i = 0; i < n; ++i)
    {
      Variable output = trainingData->getElement(i)[1];
      if (output.isMissingValue())
        continue;
      ++vote[output.getInteger()];
    }
    
    int bestClass = -1;
    int bestVote = -1;
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
    {
      if ((int)vote[i] > bestVote)
      {
        bestVote = vote[i];
        bestClass = i;
      }
    }
    leafValue = Variable(bestClass, outputType);
  }
  else
  {
    MessageCallback::error(T("SingleExtraTreeInferenceLearner::shouldCreateLeaf"), T("Type ") + outputType->getClassName().quoted() + (" not yet implemented"));
    leafValue = trainingData->getElement(0)[1];
  }
  /*
  jassert(n > 1);
  double weight = 1.0 / (double)n;
  for (size_t i = 0; i < n; ++i)
    leafValue.addWeighted(trainingData->getElement(i)[1], weight);*/
  
  return true;
}
///////////////////////////////////// Split Predicate Sampling functions /////////////////////
Variable sampleNumericalIntegerSplit(RandomGeneratorPtr random, ContainerPtr trainingData, size_t variableIndex)
{
  Variable dbg;
  jassert(!isInputVariableConstant(trainingData, variableIndex, dbg));
  int minValue = 0x7FFFFFFF;
  int maxValue = -minValue;
  size_t n = trainingData->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    Variable variable = getInputVariableFromExample(trainingData->getElement(i), variableIndex);
    if (!variable.exists())
      continue;
    int value = variable.getInteger();
    if (value < minValue)
      minValue = value;
    if (value > maxValue)
      maxValue = value;
  }
  jassert(minValue != 0x7FFFFFFF && maxValue != -0x7FFFFFFF);
  jassert(maxValue > minValue);
  return random->sampleInt(minValue, maxValue);
}

Variable sampleNumericalSplit(RandomGeneratorPtr random, ContainerPtr trainingData, size_t variableIndex)
{
  double minValue = DBL_MAX, maxValue = -DBL_MAX;
  size_t n = trainingData->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    Variable variable = getInputVariableFromExample(trainingData->getElement(i), variableIndex);
    if (variable.isNil())
      continue;
    double value = variable.getDouble();
    if (value < minValue)
      minValue = value;
    if (value > maxValue)
      maxValue = value;
  }
  jassert(minValue != DBL_MAX && maxValue != -DBL_MAX);
  double res = random->sampleDouble(minValue, maxValue);
  jassert(res >= minValue && res < maxValue);
  return Variable(res);
}

Variable sampleBooleanSplit(RandomGeneratorPtr random, ContainerPtr trainingData, size_t variableIndex)
{
  return Variable(random->sampleBool(), booleanType);
}

Variable sampleEnumerationSplit(RandomGeneratorPtr random, EnumerationPtr enumeration, ContainerPtr trainingData, size_t variableIndex)
{
  size_t n = enumeration->getNumElements();

  // enumerate possible values
  std::set<size_t> possibleValues;
  for (size_t i = 0; i < trainingData->getNumElements(); ++i)
  {
    Variable value = getInputVariableFromExample(trainingData->getElement(i), variableIndex);
    possibleValues.insert((size_t)value.getInteger());
  }
  jassert(possibleValues.size() >= 2);

  // convert from std::set to std::vector
  std::vector<size_t> possibleValuesVector;
  possibleValuesVector.reserve(possibleValues.size());
  for (std::set<size_t>::const_iterator it = possibleValues.begin(); it != possibleValues.end(); ++it)
    possibleValuesVector.push_back(*it);

  // sample selected values
  std::set<size_t> selectedValues;
  random->sampleSubset(possibleValuesVector, possibleValues.size() / 2, selectedValues);

  // create mask
  BooleanVectorPtr mask = new BooleanVector(n + 1);
  for (size_t i = 0; i < mask->getNumElements(); ++i)
  {
    bool bitValue;
    if (possibleValues.find(i) == possibleValues.end())
      bitValue = random->sampleBool(); // 50% probability for values that do not appear in the training data
    else
      bitValue = (selectedValues.find(i) != selectedValues.end()); // true for selected values
    mask->set(i, bitValue);
  }
  return mask;
}

PredicatePtr sampleSplit(RandomGeneratorPtr random, ContainerPtr trainingData, TypePtr inputType, size_t variableIndex, Variable& splitArgument)
{
  TypePtr variableType = inputType->getObjectVariableType(variableIndex);
  if (variableType->inheritsFrom(doubleType))
    splitArgument = sampleNumericalSplit(random, trainingData, variableIndex);
  else if (variableType->inheritsFrom(integerType))
    splitArgument = sampleNumericalIntegerSplit(random, trainingData, variableIndex);
  else if (variableType->inheritsFrom(enumValueType))
  {
    EnumerationPtr enumeration = variableType.dynamicCast<Enumeration>();
    splitArgument = sampleEnumerationSplit(random, enumeration, trainingData, variableIndex);
  }
  else if (variableType->inheritsFrom(booleanType))
    splitArgument = sampleBooleanSplit(random, trainingData, variableIndex);
  else if (variableType->inheritsFrom(discreteProbabilityDistributionClass(topLevelType)))
  {
    jassert(false);
  }
  else
  {
    MessageCallback::error(T("sampleSplit"), T("Type ") + variableType->getClassName().quoted() + (" not yet implemented"));
    return PredicatePtr();
  }
  
  PredicatePtr predicate = BinaryDecisionTree::getSplitPredicate(splitArgument);

#ifdef JUCE_DEBUG
  size_t numPos = 0, numNeg = 0;
  for (size_t i = 0; i < trainingData->getNumElements(); ++i)
  {
    Variable inputOutputPair = trainingData->getElement(i);
    if (predicate->computePredicate(getInputVariableFromExample(inputOutputPair, variableIndex)))
      ++numPos;
    else
      ++numNeg;
  }
  if (!numPos || !numNeg)
  {
    std::cout << "Predicate: " << predicate->toString() << " " << trainingData->getNumElements() << " => " << numPos << " + " << numNeg << std::endl;
    jassert(false);
  }
#endif // JUCE_DEBUG

  return predicate;
}
///////////////////////////////////// Split Scoring  /////////////////////


DiscreteProbabilityDistributionPtr computeDiscreteOutputDistribution(ContainerPtr examples)
{
  if (!examples->getNumElements())
    return DiscreteProbabilityDistributionPtr();
  DiscreteProbabilityDistributionPtr res = new DiscreteProbabilityDistribution(examples->getElementsType()->getTemplateArgument(1));
  size_t n = examples->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    Variable output = examples->getElement(i)[1];
    jassert(output.exists());
    res->increment(output);
  }
  return res;
}

static double computeClassificationSplitScore(ContainerPtr examples, ContainerPtr negativeExamples, ContainerPtr positiveExamples)
{
  DiscreteProbabilityDistributionPtr priorDistribution = computeDiscreteOutputDistribution(examples);
  DiscreteProbabilityDistributionPtr negativeDistribution = computeDiscreteOutputDistribution(negativeExamples);
  DiscreteProbabilityDistributionPtr positiveDistribution = computeDiscreteOutputDistribution(positiveExamples);

  BernoulliDistributionPtr splitDistribution = new BernoulliDistribution(positiveExamples->getNumElements() / (double)examples->getNumElements());

  double classificationEntropy = priorDistribution->computeEntropy();
  
  double informationGain = classificationEntropy
    - splitDistribution->getProbabilityOfTrue() * positiveDistribution->computeEntropy() 
    - splitDistribution->getProbabilityOfFalse() * negativeDistribution->computeEntropy(); 

  double splitEntropy = splitDistribution->computeEntropy();

  jassert(splitEntropy + classificationEntropy != 0);
  return 2.0 * informationGain / (splitEntropy + classificationEntropy);
}

static double computeLeastSquardDeviation(ContainerPtr examples)
{
  size_t n = examples->getNumElements();
  jassert(n);
  /* compute mean */
  double sum = 0;
  for (size_t i = 0; i < n; ++i)
    sum += examples->getElement(i)[1].getDouble();
  double mean = sum / (double)n;
  /* compute least square */
  double leastSquare = 0;
  for (size_t i = 0; i < n; ++i)
  {
    double delta = examples->getElement(i)[1].getDouble() - mean;
    leastSquare += delta * delta;
  }
  
  return leastSquare;
}

static double computeRegressionSplitScore(ContainerPtr examples, ContainerPtr negativeExamples, ContainerPtr positiveExamples)
{
  double partOfNegative = (double)negativeExamples->getNumElements() / examples->getNumElements();
  double partOfPositive = 1 - partOfNegative;

  return -(computeLeastSquardDeviation(negativeExamples)
  + computeLeastSquardDeviation(positiveExamples));
}

double computeSplitScore(ContainerPtr examples, size_t variableIndex, PredicatePtr predicate, ContainerPtr& negativeExamples, ContainerPtr& positiveExamples)
{
  VectorPtr neg = vector(examples->getElementsType());
  VectorPtr pos = vector(examples->getElementsType());
  for (size_t i = 0; i < examples->getNumElements(); ++i)
  {
    Variable inputOutputPair = examples->getElement(i);
    if (predicate->computePredicate(getInputVariableFromExample(inputOutputPair, variableIndex)))
      pos->append(inputOutputPair);
    else
      neg->append(inputOutputPair);
  }
  jassert(pos->getNumElements() && neg->getNumElements());
  negativeExamples = neg;
  positiveExamples = pos;

  TypePtr outputType = examples->getElementsType()->getTemplateArgument(1);
  if (outputType->inheritsFrom(enumValueType))
    return computeClassificationSplitScore(examples, negativeExamples, positiveExamples);
  if (outputType->inheritsFrom(doubleType))
    return computeRegressionSplitScore(examples, negativeExamples, positiveExamples);

  jassert(false);
  return 0.0;
}

///////////////////////////////////// 

void SingleExtraTreeInferenceLearner::sampleTreeRecursively(BinaryDecisionTreePtr tree, size_t nodeIndex, TypePtr inputType, TypePtr outputType, ContainerPtr trainingData, const std::vector<size_t>& variables)
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
  if (shouldCreateLeaf(trainingData, nonConstantVariables, outputType, leafValue))
  {
    tree->createLeaf(nodeIndex, leafValue);
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
  std::vector<size_t>       bestSplitVariable;
  std::vector<Variable>     bestSplitArgument;
  std::vector<ContainerPtr> bestPositiveExamples;
  std::vector<ContainerPtr> bestNegativeExamples;

  for (size_t i = 0; i < K; ++i)
  {
    Variable splitArgument;
    PredicatePtr splitPredicate = sampleSplit(random, trainingData, inputType, splitVariables[i], splitArgument);
    ContainerPtr negativeExamples, positiveExamples;
    double splitScore = computeSplitScore(trainingData, splitVariables[i], splitPredicate, negativeExamples, positiveExamples);
    jassert(negativeExamples->getNumElements() + positiveExamples->getNumElements() == trainingData->getNumElements());
 /*   std::cout << "Predicate: " << splitPredicate->toString()
              << "\tx = " << splitVariables[i]
              << "\t=> score = " << splitScore
              << "\tnbPos: " << positiveExamples->getNumElements()
              << "\tnbNeg: " << negativeExamples->getNumElements()
              << std::endl;*/
    // FIXME: I think that we must take a random bestSplitScore when there are many bestSplit
    // I constat a significant difference between 'take the first bestScore' and 'take the last best score'
    if (splitScore > bestSplitScore)
    {
      //std::cout << "Predicate: " << splitPredicate->toString() << " => score = " << splitScore << std::endl;
      bestSplitVariable.clear();
      bestSplitArgument.clear();
      bestPositiveExamples.clear();
      bestNegativeExamples.clear();
      
      bestSplitScore = splitScore;
    }
    if (splitScore >= bestSplitScore)
    {
      bestSplitVariable.push_back(splitVariables[i]);
      bestSplitArgument.push_back(splitArgument);
      bestPositiveExamples.push_back(positiveExamples);
      bestNegativeExamples.push_back(negativeExamples);
    }
  }
  jassert(bestSplitVariable.size());
  int bestIndex = RandomGenerator::getInstance()->sampleInt(0, (int)bestSplitVariable.size());
/*  std::cout << "Best: " << bestSplitArgument[bestIndex].toString()
  << "\tx = " << bestSplitVariable[bestIndex]
  << "\t=> score = " << bestSplitScore
  << "\tnbPos: " << bestPositiveExamples[bestIndex]->getNumElements()
  << "\tnbNeg: " << bestNegativeExamples[bestIndex]->getNumElements()
  << std::endl;*/

  // allocate child nodes
  size_t leftChildIndex = tree->allocateNodes(2);

  // create the node
  tree->createInternalNode(nodeIndex, bestSplitVariable[bestIndex], bestSplitArgument[bestIndex], leftChildIndex);
  
  // call recursively
  sampleTreeRecursively(tree, leftChildIndex, inputType, outputType, bestNegativeExamples[bestIndex], nonConstantVariables);
  sampleTreeRecursively(tree, leftChildIndex + 1, inputType, outputType, bestPositiveExamples[bestIndex], nonConstantVariables);
}

BinaryDecisionTreePtr SingleExtraTreeInferenceLearner::sampleTree(TypePtr inputClass, TypePtr outputClass, ContainerPtr trainingData)
{
  size_t n = trainingData->getNumElements();
  if (!n)
    return BinaryDecisionTreePtr();

  // we start with all variables
  std::vector<size_t> variables(inputClass->getObjectNumVariables());
  for (size_t i = 0; i < variables.size(); ++i)
    variables[i] = i;

  // create the initial binary decision tree
  BinaryDecisionTreePtr res = new BinaryDecisionTree();
  res->reserveNodes(n);
  size_t nodeIndex = res->allocateNodes(1);
  jassert(nodeIndex == 0);

  // sample tree recursively
  sampleTreeRecursively(res, nodeIndex, inputClass, outputClass, trainingData, variables);
  return res;
}
