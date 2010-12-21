/*-----------------------------------------.---------------------------------.
| Filename: AutoTuneInferenceLearner.h     | Auto Tune Inference             |
| Author  : Francis Maes                   |  Batch Learner                  |
| Started : 21/12/2010 22:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_AUTO_TUNE_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_AUTO_TUNE_H_

# include <lbcpp/Inference/InferenceBatchLearner.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Function/ObjectiveFunction.h>

namespace lbcpp
{

class EvaluateStochasticLearnerObjectiveFunction : public EvaluateBatchLearnerObjectiveFunction
{
public:
  virtual TypePtr getInputType() const
    {return inferenceOnlineLearnerParametersClass;}

  virtual InferencePtr createLearner(ExecutionContext& context, const Variable& parameters, const InferencePtr& targetInference) const
  {
    const InferenceOnlineLearnerParametersPtr& params = parameters.getObjectAndCast<InferenceOnlineLearnerParameters>();
    jassert(params);
    targetInference->setOnlineLearner(params->createLearner());
    return stochasticInferenceLearner(true);
  }

  virtual double getObjective(ExecutionContext& context, const InferencePtr& targetInference) const
    {return targetInference->getOnlineLearner()->getLastLearner()->getDefaultScore();}
};

class AutoTuneInferenceLearner : public InferenceBatchLearner<Inference>
{
public:
  AutoTuneInferenceLearner(const InferencePtr& optimizer, const EvaluateBatchLearnerObjectiveFunctionPtr& objective)
    : optimizer(optimizer), objective(objective) {}
  AutoTuneInferenceLearner() {}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>();

    // optimize parameters
    EvaluateBatchLearnerObjectiveFunctionPtr objective = this->objective->cloneAndCast<EvaluateBatchLearnerObjectiveFunction>(context);
    objective->setLearnerInput(learnerInput);
    Variable optimizedParameters;
    if (!optimizer->run(context, objective, Variable(), &optimizedParameters))
      return Variable();
    
    // learn with optimized parameters
    InferencePtr targetInference = learnerInput->getTargetInference();
    InferencePtr baseLearner = objective->createLearner(context, optimizedParameters, targetInference);
    baseLearner->run(context, learnerInput, Variable());
    return Variable();
  }

protected:
  friend class AutoTuneInferenceLearnerClass;

  InferencePtr optimizer;
  EvaluateBatchLearnerObjectiveFunctionPtr objective;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_AUTO_TUNE_H_
