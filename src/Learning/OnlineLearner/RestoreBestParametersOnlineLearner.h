/*-----------------------------------------.---------------------------------.
| Filename: RestoreBestParametersOnlineLearner.h | Restore the best          |
| Author  : Francis Maes                   | parameters when learning is     |
| Started : 16/02/2011 14:43               | finished.                       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_ONLINE_LEARNER_RESTORE_BEST_PARAMETERS_H_
# define LBCPP_LEARNING_ONLINE_LEARNER_RESTORE_BEST_PARAMETERS_H_

# include <lbcpp/Learning/OnlineLearner.h>

namespace lbcpp
{

class RestoreBestParametersOnlineLearner : public OnlineLearner
{
public:
  virtual void startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    this->function = function;
    bestFunction = FunctionPtr();
    bestFunctionValue = DBL_MAX;
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    if (objectiveValueToMinimize < bestFunctionValue)
    {
      bestFunction = function->cloneAndCast<Function>();
      bestFunctionValue = objectiveValueToMinimize;
    }
    return false;
  }

  virtual void finishLearning()
  {
    if (bestFunction)
      bestFunction->clone(defaultExecutionContext(), function);
  }

protected:
  FunctionPtr function;
  FunctionPtr bestFunction;
  double bestFunctionValue;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_RESTORE_BEST_PARAMETERS_H_
