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
    : stoppingCriterion(stoppingCriterion) {}

  virtual void startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
    {stoppingCriterion->reset();}

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
    {return stoppingCriterion->shouldStop(objectiveValueToMinimize);}

  virtual void clone(const ObjectPtr& target) const
  {
    if (stoppingCriterion)
      target.staticCast<StoppingCriterionOnlineLearner>()->stoppingCriterion = stoppingCriterion->cloneAndCast<StoppingCriterion>();
  }

protected:
  friend class StoppingCriterionOnlineLearnerClass;

  StoppingCriterionPtr stoppingCriterion;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_STOPPING_CRITERION_H_
