/*-----------------------------------------.---------------------------------.
| Filename: ParallelInferenceBatchLearner.h| A batch learner that            |
| Author  : Francis Maes                   |  parallely learns               |
| Started : 26/05/2010 19:08               |  it sub-inferences              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_PARALLEL_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_PARALLEL_H_

# include <lbcpp/Inference/Inference.h>
# include "RunOnSupervisedExamplesInference.h"

namespace lbcpp
{

class ParallelInferenceBatchLearner : public ParallelInference
{
public:
  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    StaticParallelInferencePtr inference = input[0].getObjectAndCast<StaticParallelInference>();
    size_t n = inference->getNumSubInferences();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();

    // Compute sub-inferences for each example
    // Compute input and supervision types for each sub-inference
    std::vector<ParallelInferenceStatePtr> currentStates(trainingData->size());
    std::vector<TypePtr> subInferenceInputTypes(n);
    std::vector<TypePtr> subInferenceSupervisionTypes(n);
    for (size_t i = 0; i < currentStates.size(); ++i)
    {
      Variable example = trainingData->getVariable(i);
      currentStates[i] = inference->prepareInference(context, example[0], example[1], returnCode);
      if (returnCode != Inference::finishedReturnCode)
        return ParallelInferenceStatePtr();
      jassert(currentStates[i]->getNumSubInferences() == n);
      for (size_t j = 0; j < n; ++j)
      {
        Variable subInput = currentStates[i]->getSubInput(j);
        if (!subInferenceInputTypes[j] && !subInput.isNil())
          subInferenceInputTypes[j] = subInput.getType();
        else
          jassert(subInput.isNil() || subInferenceInputTypes[j] == subInput.getType());

        Variable subSupervision = currentStates[i]->getSubSupervision(j);
        if (!subInferenceSupervisionTypes[j] && !subSupervision.isNil())
          subInferenceSupervisionTypes[j] = subSupervision.getType();
        else
          jassert(subSupervision.isNil() || subInferenceSupervisionTypes[j] == subSupervision.getType());
      }
    }

    // Create Learning Parallel Inference
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      InferencePtr subInference = inference->getSubInference(i);
      InferencePtr subInferenceLearner = subInference->getBatchLearner();
      if (subInferenceLearner)
      {
        VectorPtr subTrainingData = new Vector(pairType(subInferenceInputTypes[i], subInferenceSupervisionTypes[i]), currentStates.size());
        for (size_t j = 0; j < currentStates.size(); ++j)
          subTrainingData->setVariable(j, Variable::pair(currentStates[j]->getSubInput(i), currentStates[j]->getSubSupervision(i)));
        res->addSubInference(subInferenceLearner, Variable::pair(subInference, subTrainingData), Variable());
      }
    }
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
    {return Variable();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_PARALLEL_H_
