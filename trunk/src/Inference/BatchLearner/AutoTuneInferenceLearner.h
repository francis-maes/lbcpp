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
# include <lbcpp/Optimizer/Optimizer.h>

namespace lbcpp
{

class EvaluateStochasticLearnerObjectiveFunction : public EvaluateBatchLearnerObjectiveFunction
{
public:
  virtual String toString() const
    {return T("Stochastic Learner");}

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
  AutoTuneInferenceLearner(const OptimizerPtr& optimizer, const OptimizerInputPtr& optimizerInput)
    : optimizer(optimizer), optimizerInput(optimizerInput)
  {
    setPushIntoStackFlag(true);
  }
  AutoTuneInferenceLearner() {}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>();

    // clone the objective and set the batch learner input
    EvaluateBatchLearnerObjectiveFunctionPtr objective = optimizerInput->getObjective()->cloneAndCast<EvaluateBatchLearnerObjectiveFunction>(context);
    objective->setLearnerInput(learnerInput);
    OptimizerInputPtr optimizerInput(new OptimizerInput(objective, this->optimizerInput->getAprioriDistribution(), this->optimizerInput->getInitialGuess()));
    
    // optimize the parameters
    Variable optimizedParameters = optimizer->computeFunction(context, optimizerInput);
    if (optimizedParameters.isNil())
      return Variable();
    
    // learn with optimized parameters
    InferencePtr targetInference = learnerInput->getTargetInference();
    InferencePtr baseLearner = objective->createLearner(context, optimizedParameters, targetInference);
    baseLearner->run(context, learnerInput, Variable());
    return Variable();
  }

  virtual String getDescription(const Variable& input) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>();
    return T("Auto-tune ") + learnerInput->getTargetInference()->getName();
  }

protected:
  friend class AutoTuneInferenceLearnerClass;

  OptimizerPtr optimizer;
  OptimizerInputPtr optimizerInput;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_AUTO_TUNE_H_
