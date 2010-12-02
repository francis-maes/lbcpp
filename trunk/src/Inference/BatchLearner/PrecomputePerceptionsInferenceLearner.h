/*-----------------------------------------.---------------------------------.
| Filename: PrecomputePerceptionsInfere...h| A decorator learner that        |
| Author  : Francis Maes                   |  precomputes Perceptions        |
| Started : 01/11/2010 15:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_

# include <lbcpp/Core/Pair.h>
# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class PrecomputePerceptionsInferenceLearner : public InferenceBatchLearner<StaticDecoratorInference>
{
public:
  PrecomputePerceptionsInferenceLearner(InferencePtr baseLearner)
    {setSubInference(baseLearner);}
  PrecomputePerceptionsInferenceLearner() {}

  virtual const PerceptionPtr& getPerception(const InferencePtr& targetInference) const = 0;

  struct PrecomputePerceptionWorkUnit : public WorkUnit
  {
    PrecomputePerceptionWorkUnit(const PerceptionPtr& perception, const std::pair<Variable, Variable>& example, const InferenceBatchLearnerInputPtr& subLearnerInput, size_t index)
      : WorkUnit(T("Precompute Perception ") + String((int)index)), perception(perception), example(example), subLearnerInput(subLearnerInput), index(index) {}

    virtual bool run(ExecutionContext& context)
      {subLearnerInput->setExample(index, perception->computeFunction(context, example.first), example.second); return true;}

  protected:
    PerceptionPtr perception;
    std::pair<Variable, Variable> example;
    InferenceBatchLearnerInputPtr subLearnerInput;
    size_t index;
  };

  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const InferencePtr& targetInference = learnerInput->getTargetInference();
    const PerceptionPtr& perception = getPerception(targetInference);

    InferenceBatchLearnerInputPtr subLearnerInput = new InferenceBatchLearnerInput(targetInference, learnerInput->getNumTrainingExamples(), learnerInput->getNumValidationExamples());
    size_t n = learnerInput->getNumExamples();

    if (context.isMultiThread())
    {
      CompositeWorkUnitPtr workUnits(new CompositeWorkUnit(T("Precompute perceptions"), n));
      for (size_t i = 0; i < n; ++i)
        workUnits->setWorkUnit(i, new PrecomputePerceptionWorkUnit(perception, learnerInput->getExample(i), subLearnerInput, i));
      workUnits->setProgressionUnit(perception->getInputType()->getName() + T("s"));
      context.run(workUnits);
    }
    else
    {
      for (size_t i = 0; i < n; ++i)
      {
        const std::pair<Variable, Variable>& example = learnerInput->getExample(i);
        subLearnerInput->setExample(i, perception->computeFunction(context, example.first), example.second);
      }
    }

    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    res->setSubInference(decorated, subLearnerInput, supervision);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_
