/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Object/Predicate.h>
#include <lbcpp/Object/ProbabilityDistribution.h>
#include "ExtraTreeInferenceLearner.h"
using namespace lbcpp;

/*
** SingleExtraTreeInferenceLearner
*/
SingleExtraTreeInferenceLearner::SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : Inference(T("SingleExtraTreeInferenceLearner")),
    numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
    minimumSizeForSplitting(minimumSizeForSplitting) {}

Variable SingleExtraTreeInferenceLearner::run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  BinaryDecisionTreeInferencePtr inference = input[0].getObjectAndCast<BinaryDecisionTreeInference>();
  VariableContainerPtr trainingData = input[1].getObjectAndCast<VariableContainer>();
  jassert(inference && trainingData);
  if (!trainingData->size())
    return Variable();

  Variable firstExample = trainingData->getVariable(0);
  jassert(firstExample);
  TypePtr inputClass = firstExample[0].getType();
  TypePtr outputClass = firstExample[1].getType();
#ifdef JUCE_DEBUG
  for (size_t i = 1; i < trainingData->size(); ++i)
  {
    Variable example = trainingData->getVariable(i);
    jassert(example[0].getType() == inputClass);
    jassert(example[1].getType() == outputClass);
  }
#endif // JUCE_DEBUG

  BinaryDecisionTreePtr tree = sampleTree(inputClass, outputClass, trainingData);
  if (tree)
    inference->setTree(tree);
  return Variable();
}

static bool isVariableConstant(VariableContainerPtr container, const std::vector<size_t>& indices, size_t index1, int index2, Variable& value)
{
  size_t n = indices.size();
  if (n <= 1)
    return true;
  value = container->getVariable(indices[0])[index1];
  if (index2 >= 0)
    value = value[(size_t)index2];
  for (size_t i = 1; i < n; ++i)
  {
    Variable otherValue = container->getVariable(indices[i])[index1];
    if (index2 >= 0)
      otherValue = otherValue[(size_t)index2];

    if (value != otherValue)
      return false;
  }
  return true;
}

static bool isInputVariableConstant(VariableContainerPtr trainingData, const std::vector<size_t>& indices, size_t variableIndex, Variable& value)
  {return isVariableConstant(trainingData, indices, 0, variableIndex, value);}

static bool isOutputConstant(VariableContainerPtr trainingData, const std::vector<size_t>& indices, Variable& value)
  {return isVariableConstant(trainingData, indices, 1, -1, value);}

Variable SingleExtraTreeInferenceLearner::createOutputDistribution(TypePtr outputType, VariableContainerPtr trainingData, const std::vector<size_t>& examples) const
{
  EnumerationPtr enumeration = outputType.dynamicCast<Enumeration>();
  if (enumeration)
  {
    DiscreteProbabilityDistributionPtr res = new DiscreteProbabilityDistribution(enumeration);
    for (size_t i = 0; i < examples.size(); ++i)
      res->increment(trainingData->getVariable(examples[i])[1]);
    return res;
  }
  else
  {
    // Not Implemented
    jassert(false);
    return Variable();
  }
}

bool SingleExtraTreeInferenceLearner::shouldCreateLeaf(VariableContainerPtr trainingData, const std::vector<size_t>& examples, const std::vector<size_t>& variables, TypePtr outputType, Variable& leafValue) const
{
  jassert(examples.size());

  if (examples.size() < numAttributeSamplesPerSplit || variables.empty())
  {
    if (examples.size() == 1)
      leafValue = trainingData->getVariable(examples[0])[1];
    else
      leafValue = createOutputDistribution(outputType, trainingData, examples);
    return true;
  }
  return isOutputConstant(trainingData, examples, leafValue);
}

void SingleExtraTreeInferenceLearner::sampleTreeRecursively(BinaryDecisionTreePtr tree, size_t nodeIndex, TypePtr inputType, TypePtr outputType, VariableContainerPtr trainingData, const std::vector<size_t>& examples, const std::vector<size_t>& variables)
{
  // update "non constant variables" set
  std::vector<size_t> nonConstantVariables;
  nonConstantVariables.reserve(variables.size());
  for (size_t i = 0; i < variables.size(); ++i)
  {
    Variable value;
    if (!isInputVariableConstant(trainingData, examples, variables[i], value))
      nonConstantVariables.push_back(variables[i]);
  }
  
  Variable leafValue;
  if (shouldCreateLeaf(trainingData, examples, nonConstantVariables, outputType, leafValue))
    tree->createLeaf(nodeIndex, leafValue);
  else
  {
    size_t leftChildIndex = tree->getNumNodes() + 1;
    size_t rightChildIndex = leftChildIndex + 1;



    //tree->createInternalNode(nodeIndex, 
    // TODO: createInternalNode
  }
}

BinaryDecisionTreePtr SingleExtraTreeInferenceLearner::sampleTree(TypePtr inputClass, TypePtr outputClass, VariableContainerPtr trainingData)
{
  size_t n = trainingData->size();
  if (!n)
    return BinaryDecisionTreePtr();

  BinaryDecisionTreePtr res = new BinaryDecisionTree();
  res->reserveNodes(n);

  size_t numVariables = inputClass->getNumStaticVariables();
  std::vector<size_t> nonConstantVariables(numVariables);
  for (size_t i = 0; i < numVariables; ++i)
    nonConstantVariables[i] = i;
  
  std::vector<size_t> indices(n);
  for (size_t i = 0; i < n; ++i)
    indices[i] = i;
  sampleTreeRecursively(res, 0, inputClass, outputClass, trainingData, indices, nonConstantVariables);
  return res;
}

//////////////////////////

ExtraTreeInference::ExtraTreeInference(const String& name, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : ParallelVoteInference(name)
{
  InferencePtr baseLearner = new SingleExtraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting);
  subInferences.resize(numTrees);
  for (size_t i = 0; i < numTrees; ++i)
  {
    InferencePtr treeInference = new BinaryDecisionTreeInference(name);
    treeInference->setBatchLearner(baseLearner);
    subInferences.set(i, treeInference);
  }
  setBatchLearner(parallelInferenceLearner());
}
