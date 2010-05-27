/*-----------------------------------------.---------------------------------.
| Filename: SimulationInferenceBatchLea..h | A batch learner that relies on  |
| Author  : Francis Maes                   | entire-inference simulation     |
| Started : 26/05/2010 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_SIMULATION_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_SIMULATION_H_

# include <lbcpp/Inference/InferenceBatchLearner.h>
# include <lbcpp/Object/ObjectPair.h>
# include "../InferenceCallback/OnlineLearningInferenceCallback.h"

namespace lbcpp
{

class SimulationInferenceBatchLearner : public InferenceBatchLearner
{
public:
  virtual ReturnCode train(InferenceContextPtr context, InferencePtr inference, ObjectContainerPtr trainingData)
  {
    ReturnCode returnCode = Inference::finishedReturnCode;
    
    InferencePtr learningIteration = runOnSupervisedExamplesInference(inference);

    OnlineLearningInferenceCallbackPtr onlineLearningCallback = new OnlineLearningInferenceCallback(InferencePtr(this));
    context->appendCallback(onlineLearningCallback);
    while (!onlineLearningCallback->isLearningStopped() && returnCode == Inference::finishedReturnCode)
      context->runInference(learningIteration, inference, trainingData, returnCode);
    context->removeCallback(onlineLearningCallback);
    return returnCode;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_SIMULATION_H_
