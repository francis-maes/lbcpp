/*-----------------------------------------.---------------------------------.
| Filename: StochasticBatchLearner.h       | Stochastic Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
# define LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/OnlineLearner.h>
# include <lbcpp/Function/StoppingCriterion.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/RandomGenerator.h>
# include "../OnlineLearner/HierarchicalOnlineLearner.h"

namespace lbcpp
{

class StochasticBatchLearner : public BatchLearner
{
public:
  StochasticBatchLearner(size_t maxIterations, bool randomizeExamples)
    : maxIterations(maxIterations), randomizeExamples(randomizeExamples) {}
  StochasticBatchLearner() {}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    // start learning
    CompositeOnlineLearnerPtr onlineLearner = hierarchicalOnlineLearner();
    bool ok = onlineLearner->startLearning(context, function, maxIterations, trainingData, validationData);

    // perform learning iterations
    double startTime = Time::getMillisecondCounterHiRes();
    bool isLearningFinished = false;
    for (size_t i = 0; !isLearningFinished && (!maxIterations || i < maxIterations); ++i)
    {
      context.enterScope(T("Learning Iteration ") + String((int)i + 1));
      context.resultCallback(T("Iteration"), i + 1);
      isLearningFinished = doLearningIteration(context, i, function, trainingData, onlineLearner);
      context.resultCallback(T("Time"), Variable((Time::getMillisecondCounterHiRes() - startTime) / 1000.0, timeType));
      context.leaveScope(onlineLearner.staticCast<HierarchicalOnlineLearner>()->getLastLearningIterationResult());
      context.progressCallback(new ProgressionState(i + 1, maxIterations, T("Learning Iterations")));
    }

    // finish learning
    onlineLearner->finishLearning();
    return ok;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class StochasticBatchLearnerClass;

  size_t maxIterations;
  bool randomizeExamples;

  void doEpisode(ExecutionContext& context, const FunctionPtr& function, const ObjectPtr& inputs, const OnlineLearnerPtr& onlineLearner) const
  {
    if (!inputs)
      return; // empty episode
    onlineLearner->startEpisode(inputs);
    Variable output = function->computeWithInputsObject(context, inputs);
    onlineLearner->finishEpisode(inputs, output);
  }

  // return true if learning is finished
  bool doLearningIteration(ExecutionContext& context, size_t iteration, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, OnlineLearnerPtr onlineLearner) const
  {
    onlineLearner->startLearningIteration(iteration);

    if (randomizeExamples)
    {
      std::vector<size_t> order;
      RandomGenerator::getInstance()->sampleOrder(trainingData.size(), order);
      for (size_t i = 0; i < order.size(); ++i)
        doEpisode(context, function, trainingData[order[i]], onlineLearner);
    }
    else
      for (size_t i = 0; i < trainingData.size(); ++i)
        doEpisode(context, function, trainingData[i], onlineLearner);

    double objective = 0.0;
    return onlineLearner->finishLearningIteration(iteration, objective);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
