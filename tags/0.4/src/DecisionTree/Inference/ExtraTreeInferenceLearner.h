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
# include <lbcpp/Distribution/DistributionBuilder.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>
# include "../../Inference/BatchLearner/StaticParallelInferenceLearner.h"

namespace lbcpp 
{

extern ClassPtr extraTreeBatchLearnerInputClass;

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

// Input: InferenceBatchLearnerInput
// Supervision: None
// Output: BinaryDecisionTree
class SingleExtraTreeInferenceLearner : public InferenceBatchLearner<Inference>
{
public:
  SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting, DistributionBuilderPtr builder);
  SingleExtraTreeInferenceLearner() : numAttributeSamplesPerSplit(0), minimumSizeForSplitting(0), numActiveAttributes(0) {}

  typedef InferenceBatchLearner<Inference> BaseClass;

  virtual ClassPtr getTargetInferenceClass() const
    {return binaryDecisionTreeInferenceClass;}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    InferenceBatchLearner<Inference>::clone(context, target);
    target.staticCast<SingleExtraTreeInferenceLearner>()->builder = builder->cloneAndCast<DistributionBuilder>(context);
  }

protected:
  friend class SingleExtraTreeInferenceLearnerClass;

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

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const;

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
