/*-----------------------------------------.---------------------------------.
| Filename: GlobalSimulationInferenceLearner.h | A Learner based on global   |
| Author  : Francis Maes                   | simulation                      |
| Started : 16/04/2010 18:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_LEARNER_GLOBAL_SIMULATION_H_
# define LBCPP_INFERENCE_LEARNER_GLOBAL_SIMULATION_H_

# include "InferenceLearner.h"
# include "../InferenceContext/ExamplesCreatorCallback.h"

namespace lbcpp
{

class GlobalSimulationInferenceLearner : public InferenceLearner
{
public:
  GlobalSimulationInferenceLearner(InferenceLearnerCallbackPtr callback)
    : InferenceLearner(callback) {}

  virtual void train(InferenceStepPtr inference, ObjectContainerPtr trainingData)
  {
    ExamplesCreatorCallbackPtr learningCallback = new ExamplesCreatorCallback(callback, false);
    InferenceContextPtr trainingContext = callback->createContext(true);
    trainingContext->appendCallback(learningCallback);

    for (size_t i = 0; true; ++i)
    {
      callback->preLearningIterationCallback(i);
      trainingContext->runWithSupervisedExamples(inference, trainingData->randomize());
      learningCallback->trainAndFlushExamples();
      if (!callback->postLearningIterationCallback(inference, i))
        break;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_LEARNER_GLOBAL_SIMULATION_H_
