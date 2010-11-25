/*-----------------------------------------.---------------------------------.
| Filename: StaticParallelInferenceLearner.h| A batch learner that           |
| Author  : Francis Maes                   |  parallely learns               |
| Started : 26/05/2010 19:08               |  it sub-inferences              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_STATIC_PARALLEL_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_STATIC_PARALLEL_H_

# include <lbcpp/Data/Vector.h>
# include <lbcpp/Data/Pair.h>
# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class StaticParallelInferenceLearner : public InferenceBatchLearner<ParallelInference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return staticParallelInferenceClass;}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
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
      currentStates[i] = targetInference->prepareInference(context, example.first, example.second, returnCode);
      if (returnCode != Inference::finishedReturnCode)
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

  virtual Variable finalizeInference(InferenceContext& context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
    {return Variable();}
};

class ParallelVoteInferenceLearner : public StaticParallelInferenceLearner 
{
public:
  virtual ParallelInferenceStatePtr prepareInference(InferenceContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const StaticParallelInferencePtr& targetInference = learnerInput->getTargetInference().staticCast<StaticParallelInference>();

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    size_t numSubInferences = targetInference->getNumSubInferences();
    res->reserve(numSubInferences);
    for (size_t i = 0; i < numSubInferences; ++i)
    {
      InferencePtr subInference = targetInference->getSubInference(i);
      // FIXME: ref counting on examples vectors
      InferenceBatchLearnerInputPtr subLearnerInput = new InferenceBatchLearnerInput(subInference, learnerInput->getTrainingExamples(), learnerInput->getValidationExamples());
      InferencePtr subInferenceLearner = subInference->getBatchLearner();
      if (subInferenceLearner)
        res->addSubInference(subInferenceLearner, subLearnerInput, Variable());
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_STATIC_PARALLEL_H_
