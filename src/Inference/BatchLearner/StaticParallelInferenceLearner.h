/*-----------------------------------------.---------------------------------.
| Filename: StaticParallelInferenceLearner.h| A batch learner that           |
| Author  : Francis Maes                   |  parallely learns               |
| Started : 26/05/2010 19:08               |  it sub-inferences              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_STATIC_PARALLEL_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_STATIC_PARALLEL_H_

# include <lbcpp/Core/Vector.h>
# include <lbcpp/Core/Pair.h>
# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class StaticParallelInferenceLearner : public InferenceBatchLearner<ParallelInference>
{
public:
  StaticParallelInferenceLearner()
  {
    setPushIntoStackFlag(true);
    setPushChildrenIntoStackFlag(true);
  }

  virtual ClassPtr getTargetInferenceClass() const
    {return staticParallelInferenceClass;}

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const StaticParallelInferencePtr& targetInference = learnerInput->getTargetInference().staticCast<StaticParallelInference>();

    size_t numSubInferences = targetInference->getNumSubInferences();
    size_t n = learnerInput->getNumExamples();

    // Compute sub-inferences for each example
    // Compute input and supervision types for each sub-inference
    std::vector<ParallelInferenceStatePtr> currentStates(n);
    for (size_t i = 0; i < currentStates.size(); ++i)
    {
      const std::pair<Variable, Variable>& example = learnerInput->getExample(i);
      currentStates[i] = targetInference->prepareInference(context, example.first, example.second);
      if (!currentStates[i])
        return ParallelInferenceStatePtr();
      jassert(currentStates[i]->getNumSubInferences() == numSubInferences);
    }

    // Create Learning Parallel Inference
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(numSubInferences);
    for (size_t i = 0; i < numSubInferences; ++i)
    {
      InferencePtr subInference = targetInference->getSubInference(i);
      InferencePtr subInferenceLearner = subInference->getBatchLearner();
      if (subInferenceLearner)
      {
        InferenceBatchLearnerInputPtr subLearnerInput = new InferenceBatchLearnerInput(subInference, learnerInput->getNumTrainingExamples(), learnerInput->getNumValidationExamples());
        for (size_t j = 0; j < n; ++j)
          subLearnerInput->setExample(j, currentStates[j]->getSubInput(i), currentStates[j]->getSubSupervision(i));
        res->addSubInference(subInferenceLearner, subLearnerInput, Variable());
      }
    }
    return res;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
    {return Variable();}
};

class ParallelVoteInferenceLearner : public StaticParallelInferenceLearner 
{
public:
  virtual InferenceBatchLearnerInputPtr createBatchLearnerSubInputModel(ExecutionContext& context,
                                                                        const InferencePtr& targetInference,
                                                                        const InferenceExampleVectorPtr& trainingExamples,
                                                                        const InferenceExampleVectorPtr& validationExamples) const
    {return new InferenceBatchLearnerInput(targetInference, trainingExamples, validationExamples);}

  virtual InferenceBatchLearnerInputPtr duplicateBatchLearnerSubInput(ExecutionContext& context,
                                                                      const InferenceBatchLearnerInputPtr& learnerInputModel,
                                                                      const InferencePtr& targetInference, 
                                                                      size_t subInferenceIndex) const
  {
    InferenceBatchLearnerInputPtr res = learnerInputModel->cloneAndCast<InferenceBatchLearnerInput>(context);
    res->setTargetInference(targetInference);
    return res;
  }

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const StaticParallelInferencePtr& targetInference = learnerInput->getTargetInference().staticCast<StaticParallelInference>();

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    size_t numSubInferences = targetInference->getNumSubInferences();
    res->reserve(numSubInferences);
    InferenceBatchLearnerInputPtr subLearnerInput;
    for (size_t i = 0; i < numSubInferences; ++i)
    {
      InferencePtr subInference = targetInference->getSubInference(i);
      subLearnerInput = (!subLearnerInput) ? createBatchLearnerSubInputModel(context, subInference, learnerInput->getTrainingExamples(), learnerInput->getValidationExamples())
                                           : duplicateBatchLearnerSubInput(context, subLearnerInput, subInference, i);
      InferencePtr subInferenceLearner = subInference->getBatchLearner();
      if (subInferenceLearner)
        res->addSubInference(subInferenceLearner, subLearnerInput, Variable());
    }
    return res;
  }

  virtual String getProgressionUnit() const
    {return T("Voters");}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_STATIC_PARALLEL_H_
