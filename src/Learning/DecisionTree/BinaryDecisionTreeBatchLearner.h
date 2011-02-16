/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
# define LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_

# include "BinaryDecisionTreeFunction.h"
# include "Data/BinaryDecisionTreeSplitter.h"
# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Distribution/DistributionBuilder.h>
# include "../../Inference/BatchLearner/StaticParallelInferenceLearner.h"

namespace lbcpp 
{
#if 0
/* DecisionTreeSharedExampleVector */
class DecisionTreeSharedExampleVector : public Object
{
public:
  std::vector< std::vector<Variable> > attributes;
  std::vector<Variable> labels;
  std::vector<size_t> indices;
};

typedef ReferenceCountedObjectPtr<DecisionTreeSharedExampleVector> DecisionTreeSharedExampleVectorPtr;

class ExtraTreeBatchLearnerInput : public InferenceBatchLearnerInput
{
public:
  ExtraTreeBatchLearnerInput(const InferencePtr& targetInference,
                             DecisionTreeSharedExampleVectorPtr examples,
                             const TypePtr& inputType,
                             const TypePtr& outputType)
    : InferenceBatchLearnerInput(targetInference, examples->labels.size()),
      examples(examples), randomGeneratorSeed(0), inputType(inputType), outputType(outputType)
    {thisClass = extraTreeBatchLearnerInputClass;}
  ExtraTreeBatchLearnerInput() 
    {thisClass = extraTreeBatchLearnerInputClass;}

  void setRandomGeneratorSeed(size_t seed)
    {randomGeneratorSeed = seed;}
  
  size_t getRandomGeneratorSeed() const
    {return randomGeneratorSeed;}
  
  DecisionTreeExampleVector getExamples()
    {return DecisionTreeExampleVector(examples->attributes, examples->labels, examples->indices);}

  TypePtr getInputType() const
    {return inputType;}

  TypePtr getOutputType() const
    {return outputType;}

protected:
  friend class ExtraTreeBatchLearnerInputClass;
  
  DecisionTreeSharedExampleVectorPtr examples;
  size_t randomGeneratorSeed;
  TypePtr inputType;
  TypePtr outputType;
};

typedef ReferenceCountedObjectPtr<ExtraTreeBatchLearnerInput> ExtraTreeBatchLearnerInputPtr;

class ExtraTreeInferenceLearner : public ParallelVoteInferenceLearner
{
  virtual InferenceBatchLearnerInputPtr createBatchLearnerSubInputModel(ExecutionContext& context,
                                                                        const InferencePtr& targetInference,
                                                                        const InferenceExampleVectorPtr& trainingExamples,
                                                                        const InferenceExampleVectorPtr& validationExamples) const;

  virtual InferenceBatchLearnerInputPtr duplicateBatchLearnerSubInput(ExecutionContext& context,
                                                                      const InferenceBatchLearnerInputPtr& learnerInputModel,
                                                                      const InferencePtr& targetInference, 
                                                                      size_t subInferenceIndex) const;
};
#endif
// Input: InferenceBatchLearnerInput
// Supervision: None
// Output: BinaryDecisionTree
class BinaryDecisionTreeBatchLearner : public BatchLearner
{
public:
  BinaryDecisionTreeBatchLearner(size_t numAttributeSamplesPerSplit,
                                 size_t minimumSizeForSplitting,
                                 DistributionBuilderPtr builder)
    : numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
      minimumSizeForSplitting(minimumSizeForSplitting),
      builder(builder),
      numActiveAttributes(0) {}

  BinaryDecisionTreeBatchLearner()
    : numAttributeSamplesPerSplit(0),
      minimumSizeForSplitting(0),
      numActiveAttributes(0) {}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;

  /* Object */
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    BatchLearner::clone(context, target);
    target.staticCast<BinaryDecisionTreeBatchLearner>()->builder = builder->cloneAndCast<DistributionBuilder>(context);
  }

protected:
  friend class BinaryDecisionTreeBatchLearnerClass;

  RandomGeneratorPtr random;

  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;
  DistributionBuilderPtr builder;

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
