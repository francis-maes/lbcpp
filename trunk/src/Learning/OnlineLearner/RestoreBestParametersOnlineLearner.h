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
    isLastFunctionBest = true;
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    if (objectiveValueToMinimize < bestFunctionValue)
    {
      bestFunction = function->cloneAndCast<Function>();
      bestFunctionValue = objectiveValueToMinimize;
      isLastFunctionBest = true;
    }
    else
      isLastFunctionBest = false;
    return false;
  }

  virtual void finishLearning()
  {
    if (!isLastFunctionBest && bestFunction)
      bestFunction->clone(defaultExecutionContext(), function);
    function = FunctionPtr();
    bestFunction = FunctionPtr();
  }

  lbcpp_UseDebuggingNewOperator

protected:
  FunctionPtr function;
  FunctionPtr bestFunction;
  double bestFunctionValue;
  bool isLastFunctionBest;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_RESTORE_BEST_PARAMETERS_H_
