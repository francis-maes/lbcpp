/*-----------------------------------------.---------------------------------.
| Filename: SequentialInferenceBatchLea..h | A batch learner that            |
| Author  : Francis Maes                   |  sequentially learns            |
| Started : 26/05/2010 19:08               |  it sub-inferences              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_SEQUENTIAL_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_SEQUENTIAL_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Data/Vector.h>
# include "RunOnSupervisedExamplesInference.h"

namespace lbcpp
{

class SequentialInferenceBatchLearner : public Inference
{
public:
  ReturnCode train(InferenceContextPtr context, InferencePtr inf, ContainerPtr trainingData)
  {
    size_t numTrainingExamples = trainingData->size();

    SequentialInferencePtr inference = inf.dynamicCast<SequentialInference>();
    jassert(inference);

    // prepare inferences
    ReturnCode returnCode = finishedReturnCode;
    std::vector<SequentialInferenceStatePtr> currentStates(numTrainingExamples);
    for (size_t i = 0; i < numTrainingExamples; ++i)
    {
      Variable example = trainingData->getVariable(i);
      jassert(example && example.size() == 2 && example[0]);
      SequentialInferenceStatePtr state = inference->prepareInference(context, example[0], example[1], returnCode);
      if (returnCode != finishedReturnCode)
        return returnCode;
      jassert(state->getSubInput());
      currentStates[i] = state;
    }

    size_t stepNumber = 0;
    while (true)
    {
      // make sub-training data 
      ContainerPtr subTrainingData = new Vector(pairType(), numTrainingExamples);
      for (size_t i = 0; i < numTrainingExamples; ++i)
      {
        Variable subInput = currentStates[i]->getSubInput();
        Variable subSupervision = currentStates[i]->getSubSupervision();
        jassert(subInput);
        subTrainingData->setVariable(i, Variable::pair(subInput, subSupervision));
      }

      // get sub-inference and sub-learner
      InferencePtr subInference = currentStates[0]->getSubInference();
#ifdef JUCE_DEBUG
      for (size_t i = 1; i < currentStates.size(); ++i)
        jassert(currentStates[i]->getSubInference() == subInference);
#endif // JUCE_DEBUG

      // apply sub-learner if it exists
      
      // tmp
      if (!subInference->getBatchLearner())
        subInference->setBatchLearner(simulationInferenceLearner());
      // -

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

protected:
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    SequentialInferencePtr inference = input[0].getObjectAndCast<SequentialInference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    jassert(inference && trainingData);
    returnCode = train(context, inference, trainingData);
    return ObjectPtr();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_SEQUENTIAL_H_
