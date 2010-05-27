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

// Input: Inference
// Supervision: (input, supervision) object pair container
// Output: None
class InferenceBatchLearner : public Inference
{
public:
  virtual void accept(InferenceVisitorPtr visitor);

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    InferencePtr inference = input.dynamicCast<Inference>();
    ObjectContainerPtr trainingData = supervision.dynamicCast<ObjectContainer>();
    jassert(inference && trainingData);
    returnCode = train(context, inference, trainingData);
    return ObjectPtr();
  }

protected:
  virtual ReturnCode train(InferenceContextPtr context, InferencePtr inference, ObjectContainerPtr trainingData)
    {return finishedReturnCode;}
};

extern InferenceBatchLearnerPtr simulationInferenceBatchLearner();

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_BATCH_LEARNER_H_
