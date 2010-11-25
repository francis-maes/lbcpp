/*-----------------------------------------.---------------------------------.
| Filename: InitializeByCloningInferenc...h| Initialize inference by cloning |
| Author  : Francis Maes                   |  another inference              |
| Started : 31/10/2010 19:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_INITIALIZE_BY_CLONING_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_INITIALIZE_BY_CLONING_H_

# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class InitializeByCloningInferenceLearner : public InferenceBatchLearner<Inference>
{
public:
  InitializeByCloningInferenceLearner(const InferencePtr& inferenceToClone)
    : inferenceToClone(inferenceToClone) {}
  InitializeByCloningInferenceLearner() {}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    context.informationCallback(T("InitializeByCloningInferenceLearner"), T("Cloning Inference ") + inferenceToClone->getName());
    inferenceToClone->clone(context, learnerInput->getTargetInference());
    return Variable();
  }

protected:
  friend class InitializeByCloningInferenceLearnerClass;
  InferencePtr inferenceToClone;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_INITIALIZE_BY_CLONING_H_
