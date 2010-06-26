/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
# define LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_

# include "ExtraTreeInference.h"
# include <lbcpp/Inference/ParallelInference.h>

namespace lbcpp 
{

// Input: (Inference, training data ObjectContainer) pair
// Supervision: None
// Output: BinaryDecisionTree
class SingleExtraTreeInferenceLearner : public Inference
{
public:
  SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting);

protected:
  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);

  BinaryDecisionTreePtr sampleTree(ExtraTreeInferencePtr inference, ClassPtr inputClass, ClassPtr outputClass, ObjectContainerPtr trainingData);

  bool isAttributeConstant(size_t attributeNumber, ObjectContainerPtr trainingData, const std::set<size_t>& indices) const
  {
    // FIXME
    return false;
  }

  bool shouldCreateLeaf(ExtraTreeInferencePtr inference, ObjectContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes) const;
  size_t sampleTreeRecursively(ExtraTreeInferencePtr inference, BinaryDecisionTreePtr tree, ClassPtr inputClass, ClassPtr outputClass, ObjectContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes);
};

class ExtraTreeInferenceLearner : public SharedParallelInference
{
public:
  ExtraTreeInferenceLearner(size_t numTrees = 100, size_t numAttributeSamplesPerSplit = 10, size_t minimumSizeForSplitting = 0);

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode);

private:
  size_t numTrees;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
