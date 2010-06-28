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
  ObjectPairPtr inferenceAndTrainingData = input.dynamicCast<ObjectPair>();
  jassert(inferenceAndTrainingData);
  ExtraTreeInferencePtr inference = inferenceAndTrainingData->getFirst().dynamicCast<ExtraTreeInference>();
  ObjectContainerPtr trainingData = inferenceAndTrainingData->getSecond().dynamicCast<ObjectContainer>();
  jassert(inference && trainingData);

  ObjectPairPtr firstExample = trainingData->getAndCast<ObjectPair>(0);
  jassert(firstExample);
  ClassPtr inputClass = firstExample->getFirst()->getClass();
  ClassPtr outputClass = firstExample->getSecond()->getClass();
#ifdef JUCE_DEBUG
  for (size_t i = 1; i < trainingData->size(); ++i)
  {
    ObjectPairPtr example = trainingData->getAndCast<ObjectPair>(i);
    jassert(example->getFirst()->getClass() == inputClass);
    jassert(example->getSecond()->getClass() == outputClass);
  }
#endif // JUCE_DEBUG

  return sampleTree(inference, inputClass, outputClass, trainingData);
}

BinaryDecisionTreePtr SingleExtraTreeInferenceLearner::sampleTree(ExtraTreeInferencePtr inference, ClassPtr inputClass, ClassPtr outputClass, ObjectContainerPtr trainingData)
{
  size_t n = trainingData->size();
  if (!n)
    return BinaryDecisionTreePtr();

  BinaryDecisionTreePtr res = new BinaryDecisionTree();
  res->reserveNodes(n);
  std::set<size_t> indices;
  
  std::set<size_t> nonConstantAttributes;
  
  for (size_t i = 0; i < n; ++i)
  {
    indices.insert(i);
  }
  
  // todo: fill nonConstantAttributes
  sampleTreeRecursively(inference, res, inputClass, outputClass, trainingData, indices, nonConstantAttributes);
  return res;
}

bool SingleExtraTreeInferenceLearner::shouldCreateLeaf(ExtraTreeInferencePtr inference, ObjectContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes) const
{
  if (indices.empty() || indices.size() < numAttributeSamplesPerSplit || nonConstantAttributes.empty())
    return true;
  std::set<size_t>::const_iterator it = indices.begin();
  ObjectPtr firstOutput = trainingData->getAndCast<ObjectPair>(*it)->getSecond();
  for (++it; it != indices.end(); ++it)
  {
    ObjectPtr output = trainingData->getAndCast<ObjectPair>(*it)->getSecond();
    // FIXME
    //if (!inference->areOutputObjectsEqual(firstOutput, output))
    //  return false;
  }
  return true;
}

size_t SingleExtraTreeInferenceLearner::sampleTreeRecursively(ExtraTreeInferencePtr inference, BinaryDecisionTreePtr tree, ClassPtr inputClass, ClassPtr outputClass, ObjectContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes)
{
  if (shouldCreateLeaf(inference, trainingData, indices, nonConstantAttributes))
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
    subInferences[i] = new SingleExtraTreeInference(name);
    subInferences[i]->setBatchLearner(baseLearner);
  }
  setBatchLearner(parallelInferenceLearner());
}
