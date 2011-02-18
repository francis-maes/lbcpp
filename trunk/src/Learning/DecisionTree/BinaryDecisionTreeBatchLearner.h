/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
# define LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_

# include <lbcpp/Data/RandomGenerator.h>
# include "BinaryDecisionTreeFunction.h"
# include "Data/BinaryDecisionTreeSplitter.h"

namespace lbcpp 
{

class BinaryDecisionTreeBatchLearner : public BatchLearner
{
public:
  BinaryDecisionTreeBatchLearner(size_t numAttributeSamplesPerSplit,
                                 size_t minimumSizeForSplitting)
    : numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
      minimumSizeForSplitting(minimumSizeForSplitting),
      numActiveAttributes(0) {}

  BinaryDecisionTreeBatchLearner()
    : numAttributeSamplesPerSplit(0),
      minimumSizeForSplitting(0),
      numActiveAttributes(0) {}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;

protected:
  friend class BinaryDecisionTreeBatchLearnerClass;

  RandomGeneratorPtr random;

  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
  size_t numActiveAttributes;

  struct Split
  {
    size_t       variableIndex;
    Variable     argument;
    std::vector<size_t> left;
    std::vector<size_t> right;
  };

  BinaryDecisionTreePtr sampleTree(ExecutionContext& context, TypePtr inputClass, TypePtr outputClass, const DecisionTreeExampleVector& examples) const;

  void sampleTreeRecursively(ExecutionContext& context,
                             const BinaryDecisionTreePtr& tree, size_t nodeIndex,
                             TypePtr inputType, TypePtr outputType,
                             const DecisionTreeExampleVector& examples,
                             const std::vector<size_t>& variables,
                             std::vector<Split>& bestSplits,
                             size_t& numLeaves, size_t numExamples) const;

  bool shouldCreateLeaf(ExecutionContext& context,
                        const DecisionTreeExampleVector& examples,
                        const std::vector<size_t>& variables,
                        TypePtr outputType, Variable& leafValue) const;

  BinaryDecisionTreeSplitterPtr getBinaryDecisionTreeSplitter(TypePtr inputType, TypePtr outputType, size_t variableIndex) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
