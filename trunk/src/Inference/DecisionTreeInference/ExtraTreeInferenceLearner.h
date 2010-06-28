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

  BinaryDecisionTreePtr sampleTree(ClassPtr inputClass, ClassPtr outputClass, VariableContainerPtr trainingData);

  bool isAttributeConstant(size_t attributeNumber, VariableContainerPtr trainingData, const std::set<size_t>& indices) const
  {
    // FIXME
    return false;
  }

  bool shouldCreateLeaf(VariableContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes) const;
  size_t sampleTreeRecursively(BinaryDecisionTreePtr tree, ClassPtr inputClass, ClassPtr outputClass, VariableContainerPtr trainingData, const std::set<size_t>& indices, const std::set<size_t>& nonConstantAttributes);
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
