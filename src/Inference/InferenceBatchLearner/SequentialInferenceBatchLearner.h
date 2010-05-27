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
  virtual InferenceBatchLearnerPtr getSubLearner(SequentialInferencePtr inference, size_t index) const = 0;
 
  virtual ReturnCode train(InferenceContextPtr context, InferencePtr inf, ObjectContainerPtr trainingData)
  {
    size_t numTrainingExamples = trainingData->size();

    SequentialInferencePtr inference = inf.dynamicCast<SequentialInference>();
    jassert(inference);

    // prepare inferences
    ReturnCode returnCode = finishedReturnCode;
    std::vector<ObjectPtr> currentObjects(numTrainingExamples);
    for (size_t i = 0; i < numTrainingExamples; ++i)
    {
      ObjectPairPtr example = trainingData->getAndCast<ObjectPair>(i);
      jassert(example);
      currentObjects[i] = inference->prepareInference(example->getFirst(), example->getSecond(), returnCode);
      if (returnCode != finishedReturnCode)
        return returnCode;
    }

    for (size_t step = 0; step < inference->getNumSubInferences(); ++step)
    {
      InferencePtr subInference = inference->getSubInference(step);
      InferenceBatchLearnerPtr subLearner = getSubLearner(inference, step);

      // make sub-training data 
      VectorObjectContainerPtr subTrainingData = new VectorObjectContainer();
      subTrainingData->resize(numTrainingExamples);
      for (size_t i = 0; i < numTrainingExamples; ++i)
      {
        ObjectPairPtr example = trainingData->getAndCast<ObjectPair>(i);
        ObjectPairPtr subExample = inference->prepareSubInference(example->getFirst(), example->getSecond(), step, currentObjects[i], returnCode);
        if (returnCode != finishedReturnCode)
          return returnCode;
        subTrainingData->set(i, subExample);
      }

      // apply sub-learner if it exists
      if (subLearner)
      {
        ReturnCode res = context->train(subLearner, subInference, subTrainingData);
        if (res != finishedReturnCode)
          return res;
      }
      
      if (step < inference->getNumSubInferences() - 1)
      {
        // evaluate sub-inference and update currentObjects
        InferencePtr evaluateStepOnSubTrainingData = new RunSequentialInferenceStepOnExamples(inference, step, currentObjects);
        context->runInference(evaluateStepOnSubTrainingData, subTrainingData, ObjectPtr(), returnCode);
      }
    }
    return finishedReturnCode;
  }
};

class SharedSequentialInferenceBatchLearner : public SequentialInferenceBatchLearner
{
public:
  SharedSequentialInferenceBatchLearner(InferenceBatchLearnerPtr subLearner)
    : subLearner(subLearner) {}

  virtual InferenceBatchLearnerPtr getSubLearner(SequentialInferencePtr inference, size_t index) const
    {return subLearner;}

protected:
  InferenceBatchLearnerPtr subLearner;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_SEQUENTIAL_H_
