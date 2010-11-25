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
# include "BinaryDecisionTreeSplitter.h"
# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp 
{

// Input: InferenceBatchLearnerInput
// Supervision: None
// Output: BinaryDecisionTree
class SingleExtraTreeInferenceLearner : public Inference
{
public:
  SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting);
  SingleExtraTreeInferenceLearner() : numAttributeSamplesPerSplit(0), minimumSizeForSplitting(0) {}

protected:
  friend class SingleExtraTreeInferenceLearnerClass;

  RandomGeneratorPtr random;

  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
  
  struct Split
  {
    size_t       variableIndex;
    Variable     argument;
    ContainerPtr positive;
    ContainerPtr negative;
  };

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);

  BinaryDecisionTreePtr sampleTree(ExecutionContext& context, TypePtr inputClass, TypePtr outputClass, ContainerPtr trainingData);

  void sampleTreeRecursively(ExecutionContext& context,
                             BinaryDecisionTreePtr tree, size_t nodeIndex,
                             TypePtr inputType, TypePtr outputType,
                             ContainerPtr trainingData, const std::vector<size_t>& variables,
                             std::vector<Split>& bestSplits);

  bool shouldCreateLeaf(ExecutionContext& context, ContainerPtr trainingData, const std::vector<size_t>& variables, TypePtr outputType, Variable& leafValue) const;

  BinaryDecisionTreeSplitterPtr getBinaryDecisionTreeSplitter(TypePtr inputType, TypePtr outputType) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
