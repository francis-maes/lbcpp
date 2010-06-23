/*-----------------------------------------.---------------------------------.
| Filename: SequentialInferenceBatchLea..h | A batch learner that            |
| Author  : Francis Maes                   |  sequentially learns            |
| Started : 26/05/2010 19:08               |  it sub-inferences              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_SEQUENTIAL_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_SEQUENTIAL_H_

# include <lbcpp/Inference/InferenceBatchLearner.h>
# include "../Inference/RunOnSupervisedExamplesInference.h"

namespace lbcpp
{

class SequentialInferenceBatchLearner : public InferenceBatchLearner
{
public:
  virtual ReturnCode train(InferenceContextPtr context, InferencePtr inf, ObjectContainerPtr trainingData)
  {
    size_t numTrainingExamples = trainingData->size();

    SequentialInferencePtr inference = inf.dynamicCast<SequentialInference>();
    jassert(inference);

    // prepare inferences
    ReturnCode returnCode = finishedReturnCode;
    std::vector<SequentialInferenceStatePtr> currentStates(numTrainingExamples);
    for (size_t i = 0; i < numTrainingExamples; ++i)
    {
      ObjectPairPtr example = trainingData->getAndCast<ObjectPair>(i);
      jassert(example);
      SequentialInferenceStatePtr state = context->makeSequentialInferenceInitialState(inference, example->getFirst(), example->getSecond(), returnCode);
      if (returnCode != finishedReturnCode)
        return returnCode;
      currentStates[i] = state;
    }

    size_t stepNumber = 0;
    while (true)
    {
      // make sub-training data 
      VectorObjectContainerPtr subTrainingData = new VectorObjectContainer();
      subTrainingData->resize(numTrainingExamples);
      for (size_t i = 0; i < numTrainingExamples; ++i)
      {
        std::pair<ObjectPtr, ObjectPtr> subExample = inference->prepareSubInference(currentStates[i], returnCode);
        if (returnCode != finishedReturnCode)
          return returnCode;
        subTrainingData->set(i, new ObjectPair(subExample.first, subExample.second));
      }

      // get sub-inference and sub-learner
      InferencePtr subInference = currentStates[0]->getCurrentSubInference();
#ifdef JUCE_DEBUG
      for (size_t i = 1; i < currentStates.size(); ++i)
        jassert(currentStates[i]->getCurrentSubInference() == subInference);
#endif // JUCE_DEBUG

      // apply sub-learner if it exists
      if (subInference->getBatchLearner())
      {
        ReturnCode res = context->train(subInference, subTrainingData);
        if (res != finishedReturnCode)
          return res;
      }
      
      // evaluate sub-inference and update currentObjects
      InferencePtr evaluateStepOnSubTrainingData = new RunSequentialInferenceStepOnExamples(inference, currentStates);
      context->runInference(evaluateStepOnSubTrainingData, subTrainingData, ObjectPtr(), returnCode);

      // check if we have reached the final state on all examples
      bool areAllStatesFinal = true;
      for (size_t i = 0; i < currentStates.size(); ++i)
        if (!currentStates[i]->isFinal())
        {
          areAllStatesFinal = false;
          break;
        }
      if (areAllStatesFinal)
        return finishedReturnCode;

      ++stepNumber;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_SEQUENTIAL_H_
