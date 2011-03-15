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
  RestoreBestParametersOnlineLearner() : context(NULL) {}
  
  virtual bool startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    this->context = &context;
    this->function = function;
    bestFunction = FunctionPtr();
    bestFunctionValue = DBL_MAX;
    isLastFunctionBest = true;
    iteration = 0;
    bestFunctionIteration = 0;
    return true;
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    ++iteration;
    if (objectiveValueToMinimize < bestFunctionValue)
    {
      bestFunction = function->cloneAndCast<Function>();
      bestFunctionValue = objectiveValueToMinimize;
      bestFunctionIteration = iteration;
      isLastFunctionBest = true;
    }
    else
      isLastFunctionBest = false;
    return false;
  }

  virtual void finishLearning()
  {
    if (!isLastFunctionBest && bestFunction)
    {
      context->informationCallback(T("Restoring parameters that gave score ") + String(bestFunctionValue) +
          T(" at iteration ") + String((int)bestFunctionIteration)); 
      bestFunction->clone(defaultExecutionContext(), function);
    }
    function = FunctionPtr();
    bestFunction = FunctionPtr();
  }

  lbcpp_UseDebuggingNewOperator

protected:
  ExecutionContext* context;
  FunctionPtr function;
  FunctionPtr bestFunction;
  double bestFunctionValue;
  bool isLastFunctionBest;
  size_t iteration;
  size_t bestFunctionIteration;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_RESTORE_BEST_PARAMETERS_H_
