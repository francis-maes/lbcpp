/*-----------------------------------------.---------------------------------.
| Filename: PrecomputePerceptionsInfere...h| A decorator learner that        |
| Author  : Francis Maes                   |  precomputes Perceptions        |
| Started : 01/11/2010 15:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_

# include <lbcpp/Data/Pair.h>
# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class PrecomputePerceptionsInferenceLearner : public InferenceBatchLearner<StaticDecoratorInference>
{
public:
  PrecomputePerceptionsInferenceLearner(InferencePtr baseLearner)
    {setSubInference(baseLearner);}
  PrecomputePerceptionsInferenceLearner() {}

  virtual const PerceptionPtr& getPerception(const InferencePtr& targetInference) const = 0;

  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const InferencePtr& targetInference = learnerInput->getTargetInference();
    const PerceptionPtr& perception = getPerception(targetInference);

    InferenceBatchLearnerInputPtr subLearnerInput = new InferenceBatchLearnerInput(targetInference, learnerInput->getNumTrainingExamples(), learnerInput->getNumValidationExamples());
    size_t n = learnerInput->getNumExamples();
    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<Variable, Variable>& example = learnerInput->getExample(i);
      subLearnerInput->setExample(i, perception->computeFunction(context, example.first), example.second);
    }

    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    res->setSubInference(decorated, subLearnerInput, supervision);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_
