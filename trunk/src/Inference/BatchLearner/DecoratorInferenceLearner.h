/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInferenceLearner.h    | A batch learner to learn a      |
| Author  : Francis Maes                   | DecoratorInference              |
| Started : 14/07/2010 12:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_DECORATOR_LEARNER_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_DECORATOR_LEARNER_H_

# include <lbcpp/Data/Vector.h>
# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class DecoratorInferenceLearner : public InferenceBatchLearner<DecoratorInference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return staticDecoratorInferenceClass;}

  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const StaticDecoratorInferencePtr& targetInference = learnerInput->getTargetInference().staticCast<StaticDecoratorInference>();
    jassert(targetInference);
    
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);

    InferencePtr targetSubInference = targetInference->getSubInference();
    if (targetSubInference && targetSubInference->getBatchLearner())
    {
      InferenceBatchLearnerInputPtr subLearnerInput = createSubLearnerInput(context, learnerInput, returnCode);
      if (returnCode != finishedReturnCode)
        return DecoratorInferenceStatePtr();

      res->setSubInference(targetSubInference->getBatchLearner(), subLearnerInput, Variable());
    }
    return res;
  }

protected:
  virtual InferenceBatchLearnerInputPtr createSubLearnerInput(ExecutionContext& context, const InferenceBatchLearnerInputPtr& input, ReturnCode& returnCode)
  {
    const StaticDecoratorInferencePtr& targetInference = input->getTargetInference().staticCast<StaticDecoratorInference>();
    const InferencePtr& targetSubInference = targetInference->getSubInference();
    InferenceBatchLearnerInputPtr res = new InferenceBatchLearnerInput(targetSubInference, input->getNumTrainingExamples(), input->getNumValidationExamples());

    size_t n = input->getNumExamples();
    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<Variable, Variable>& example = input->getExample(i);
      Inference::ReturnCode returnCode = Inference::finishedReturnCode;
      DecoratorInferenceStatePtr state = targetInference->prepareInference(context, example.first, example.second, returnCode);
      if (returnCode != Inference::finishedReturnCode)
        return InferenceBatchLearnerInputPtr();
      res->setExample(i, state->getSubInput(), state->getSubSupervision());
    }
    return res;
  }
};

class PostProcessInferenceLearner : public DecoratorInferenceLearner
{
protected:
  virtual InferenceBatchLearnerInputPtr createSubLearnerInput(ExecutionContext& context, const InferenceBatchLearnerInputPtr& input, ReturnCode& returnCode)
  {
    const InferencePtr& targetSubInference = input->getTargetInference().staticCast<StaticDecoratorInference>()->getSubInference();
    return new InferenceBatchLearnerInput(targetSubInference, input->getTrainingExamples(), input->getValidationExamples());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_DECORATOR_LEARNER_H_
