/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/ObjectPair.h>
#include <lbcpp/Inference/Inference.h>
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

  Variable firstExample = trainingData->getVariable(0);
  jassert(firstExample);
  ClassPtr inputClass = firstExample[0].getType();
  ClassPtr outputClass = firstExample[1].getType();
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

bool isVariableConstant(size_t index1, size_t index2, VariableContainerPtr trainingData)
{
  size_t n = trainingData->size();
  if (n <= 1)
    return true;
  Variable refValue = trainingData->getVariable(0)[index1][index2];
  for (size_t i = 1; i < n; ++i)
  {
    Variable otherValue = trainingData->getVariable(i)[index1][index2];
    if (refValue != otherValue)
      return false;
  }
  return true;
}

BinaryDecisionTreePtr SingleExtraTreeInferenceLearner::sampleTree(ClassPtr inputClass, ClassPtr outputClass, VariableContainerPtr trainingData)
{
  size_t n = trainingData->size();
  if (!n)
    return BinaryDecisionTreePtr();

  BinaryDecisionTreePtr res = new BinaryDecisionTree();
  res->reserveNodes(n);

  std::set<size_t> nonConstantVariables;
  size_t numVariables = inputClass->getNumStaticVariables();
  for (size_t i = 0; i < numVariables; ++i)
    if (!isVariableConstant(0, i, trainingData))
      nonConstantVariables.insert(i);
  
  std::set<size_t> indices;
  for (size_t i = 0; i < n; ++i)
    indices.insert(i);
  sampleTreeRecursively(res, inputClass, outputClass, trainingData, indices, nonConstantVariables);
  return res;
}

bool SingleExtraTreeInferenceLearner::shouldCreateLeaf(VariableContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantVariables) const
{
  if (indices.empty() || indices.size() < numAttributeSamplesPerSplit || nonConstantVariables.empty())
    return true;
  std::set<size_t>::const_iterator it = indices.begin();
  Variable firstOutput = trainingData->getVariable(*it)[1];
  for (++it; it != indices.end(); ++it)
  {
    Variable output = trainingData->getVariable(*it)[1];
    if (output != firstOutput)
      return false;
  }
  return true;
}

size_t SingleExtraTreeInferenceLearner::sampleTreeRecursively(BinaryDecisionTreePtr tree, ClassPtr inputClass, ClassPtr outputClass, VariableContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes)
{
  if (shouldCreateLeaf(trainingData, indices, nonConstantAttributes))
  {
  }
  else
  {
  }
  return 0; // FIXME    
}

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
