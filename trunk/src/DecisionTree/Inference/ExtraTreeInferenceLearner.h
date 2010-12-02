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
# include <lbcpp/Inference/InferenceBatchLearner.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/ProbabilityDistribution/ProbabilityDistributionBuilder.h>

namespace lbcpp 
{

// Input: InferenceBatchLearnerInput
// Supervision: None
// Output: BinaryDecisionTree
class SingleExtraTreeInferenceLearner : public InferenceBatchLearner<Inference>
{
public:
  SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting, ProbabilityDistributionBuilderPtr builder);
  SingleExtraTreeInferenceLearner() : numAttributeSamplesPerSplit(0), minimumSizeForSplitting(0) {}

  typedef InferenceBatchLearner<Inference> BaseClass;

  virtual ClassPtr getTargetInferenceClass() const
    {return binaryDecisionTreeInferenceClass;}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    InferenceBatchLearner<Inference>::clone(context, target);
    target.staticCast<SingleExtraTreeInferenceLearner>()->random = new RandomGenerator(random->sampleInt());
    target.staticCast<SingleExtraTreeInferenceLearner>()->builder = builder->cloneAndCast<ProbabilityDistributionBuilder>(context);
  }

protected:
  friend class SingleExtraTreeInferenceLearnerClass;

  RandomGeneratorPtr random;

  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
  ProbabilityDistributionBuilderPtr builder;
  
  struct Split
  {
    size_t       variableIndex;
    Variable     argument;
    ContainerPtr positive;
    ContainerPtr negative;
  };

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const;

  BinaryDecisionTreePtr sampleTree(ExecutionContext& context, TypePtr inputClass, TypePtr outputClass, ContainerPtr trainingData) const;

  void sampleTreeRecursively(ExecutionContext& context,
                             BinaryDecisionTreePtr tree, size_t nodeIndex,
                             TypePtr inputType, TypePtr outputType,
                             ContainerPtr trainingData, const std::vector<size_t>& variables,
                             std::vector<Split>& bestSplits) const;

  bool shouldCreateLeaf(ExecutionContext& context, ContainerPtr trainingData, const std::vector<size_t>& variables, TypePtr outputType, Variable& leafValue) const;

  BinaryDecisionTreeSplitterPtr getBinaryDecisionTreeSplitter(TypePtr inputType, TypePtr outputType, size_t variableIndex) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
