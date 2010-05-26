/*-----------------------------------------.---------------------------------.
| Filename: InferenceBatchLearner.h        | Inference Batch Learners        |
| Author  : Francis Maes                   |                                 |
| Started : 26/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_H_

# include "Inference.h"
# include "../ObjectPredeclarations.h"

namespace lbcpp
{

// Input: (input, supervision) object pair container
// Supervision: none
// Output: none
class InferenceBatchLearner : public Inference
{
public:
  InferenceBatchLearner(InferencePtr inference)
    : inference(inference) {}

  virtual void accept(InferenceVisitorPtr visitor);

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr output, ReturnCode& returnCode)
  {
    ObjectContainerPtr trainingData = input.dynamicCast<ObjectContainer>();
    jassert(trainingData);
    returnCode = train(context, trainingData);
    return ObjectPtr();
  }

  InferencePtr getLearnedInference() const
    {return inference;}

protected:
  InferencePtr inference;

  virtual ReturnCode train(InferenceContextPtr context, ObjectContainerPtr trainingData) = 0;
};

extern InferenceBatchLearnerPtr simulationInferenceBatchLearner(InferencePtr inference);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_BATCH_LEARNER_H_
