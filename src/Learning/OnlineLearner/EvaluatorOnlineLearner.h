/*-----------------------------------------.---------------------------------.
| Filename: EvaluatorOnlineLearner.h       | Evaluator Online Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 16/02/2011 15:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_ONLINE_LEARNER_EVALUATOR_H_
# define LBCPP_LEARNING_ONLINE_LEARNER_EVALUATOR_H_

# include <lbcpp/Learning/OnlineLearner.h>
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class EvaluatorOnlineLearner : public OnlineLearner
{
public:
  EvaluatorOnlineLearner(bool evaluateOnTrainingData = true, bool evaluateOnValidationData = true)
    : evaluateOnTrainingData(evaluateOnTrainingData), evaluateOnValidationData(evaluateOnValidationData),
      context(NULL), trainingData(NULL), validationData(NULL) {}

  virtual bool startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    this->context = &context;
    this->function = function;
    this->trainingData = trainingData.size() ? &trainingData : NULL;
    this->validationData = validationData.size() ? &validationData : NULL;
    return true;
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    if (trainingData && evaluateOnTrainingData)
    {
      ScoreObjectPtr score = evaluate(*trainingData, T("Train"));
      if (score)
        objectiveValueToMinimize = score->getScoreToMinimize();
    }

    if (validationData && evaluateOnValidationData)
    {
      ScoreObjectPtr score = evaluate(*validationData, T("Validation"));
      if (score)
        objectiveValueToMinimize = score->getScoreToMinimize();
    }
    return false;
  }

  virtual void finishLearning()
    {function = FunctionPtr();}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class EvaluatorOnlineLearnerClass;

  bool evaluateOnTrainingData;
  bool evaluateOnValidationData;

  ExecutionContext* context;
  FunctionPtr function;
  const std::vector<ObjectPtr>* trainingData;
  const std::vector<ObjectPtr>* validationData;

  ScoreObjectPtr evaluate(const std::vector<ObjectPtr>& data, const String& name)
  {
    ScoreObjectPtr score = function->evaluate(*context, data, EvaluatorPtr(), name + T(" evaluation"));
    if (score)
      context->resultCallback(name + T(" score"), score->getScoreToMinimize());
    return score;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_EVALUATOR_H_
