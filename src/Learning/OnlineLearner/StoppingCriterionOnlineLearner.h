/*-----------------------------------------.---------------------------------.
| Filename: StoppingCriterionOnlineLearner.h| Stopping Online Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 16/02/2011 14:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_ONLINE_LEARNER_STOPPING_CRITERION_H_
# define LBCPP_LEARNING_ONLINE_LEARNER_STOPPING_CRITERION_H_

# include <lbcpp/Learning/OnlineLearner.h>
# include <lbcpp/Function/StoppingCriterion.h>

namespace lbcpp
{

class StoppingCriterionOnlineLearner : public OnlineLearner
{
public:
  StoppingCriterionOnlineLearner(StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr())
    : context(NULL)
  {
    if (stoppingCriterion)
      this->stoppingCriterion = stoppingCriterion->cloneAndCast<StoppingCriterion>(defaultExecutionContext());
  }

  virtual bool startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
    {stoppingCriterion->reset(); this->context = &context; return true;}

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    bool shouldStop = stoppingCriterion->shouldStop(objectiveValueToMinimize);
    if (shouldStop)
      context->informationCallback(T("Stopping criterion: ") + stoppingCriterion->toString());
    return shouldStop;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    OnlineLearner::clone(context, target);
    if (stoppingCriterion)
      target.staticCast<StoppingCriterionOnlineLearner>()->stoppingCriterion = stoppingCriterion->cloneAndCast<StoppingCriterion>();
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class StoppingCriterionOnlineLearnerClass;

  StoppingCriterionPtr stoppingCriterion;
  ExecutionContext* context;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_STOPPING_CRITERION_H_
