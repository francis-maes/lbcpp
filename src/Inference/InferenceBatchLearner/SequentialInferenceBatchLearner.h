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

namespace lbcpp
{

class SequentialInferenceBatchLearner : public InferenceBatchLearner
{
public:
  virtual InferenceBatchLearnerPtr getSubLearner(SequentialInferencePtr inference, size_t index) const = 0;

  virtual ReturnCode train(InferenceContextPtr context, InferencePtr inference, ObjectContainerPtr trainingData)
  {
    SequentialInferencePtr inferenceSequence = inference.dynamicCast<SequentialInference>();
    jassert(inferenceSequence);
    for (size_t i = 0; i < inferenceSequence->getNumSubInferences(); ++i)
    {
      InferenceBatchLearnerPtr subLearner = getSubLearner(inferenceSequence, i);



      // FIXME: Extend the SequentialInference base class
      // FIXME: compute sub-training data
      ReturnCode res = context->train(subLearner, inferenceSequence->getSubInference(i), trainingData);
      if (res != finishedReturnCode)
        return res;
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
