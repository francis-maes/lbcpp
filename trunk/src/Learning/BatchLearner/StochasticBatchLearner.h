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
# include "../OnlineLearner/CompositeOnlineLearner.h"

namespace lbcpp
{

class StochasticBatchLearner : public BatchLearner
{
public:
  StochasticBatchLearner(const std::vector<FunctionPtr>& functionsToLearn, size_t maxIterations, bool randomizeExamples)
    : functionsToLearn(functionsToLearn), maxIterations(maxIterations), randomizeExamples(randomizeExamples) {}

  StochasticBatchLearner() {}

  virtual TypePtr getRequiredExamplesType() const
    {return objectClass;}

  virtual FunctionPtr train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    CompositeOnlineLearnerPtr compositeOnlineLearner = new CompositeOnlineLearner();
    for (size_t i = 0; i < functionsToLearn.size(); ++i)
      compositeOnlineLearner->startLearningAndAddLearner(context, functionsToLearn[i], maxIterations, trainingData, validationData);

    for (size_t i = 0; compositeOnlineLearner->getNumLearners() && (!maxIterations || i < maxIterations); ++i)
    {
      context.enterScope(T("Learning Iteration ") + String((int)i + 1));
      context.resultCallback(T("Iteration"), i + 1);
      Variable learningIterationResult = doLearningIteration(context, i, function, trainingData, compositeOnlineLearner);
      context.leaveScope(learningIterationResult);
      context.progressCallback(new ProgressionState(i + 1, maxIterations, T("Learning Iterations")));
    }

    compositeOnlineLearner->finishLearning();
    return function;
  }

protected:
  friend class StochasticBatchLearnerClass;

  std::vector<FunctionPtr> functionsToLearn;
  EvaluatorPtr evaluator;
  size_t maxIterations;
  bool randomizeExamples;
  
  void doEpisode(ExecutionContext& context, const FunctionPtr& function, const ObjectPtr& inputs, const OnlineLearnerPtr& onlineLearner) const
  {
    onlineLearner->startEpisode();
    function->computeWithInputsObject(context, inputs);
    onlineLearner->finishEpisode();
  }

  Variable doLearningIteration(ExecutionContext& context, size_t iteration, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, CompositeOnlineLearnerPtr compositeOnlineLearner) const
  {
    jassert(compositeOnlineLearner->getNumLearners());
    compositeOnlineLearner->startLearningIteration(iteration);

    if (randomizeExamples)
    {
      std::vector<size_t> order;
      RandomGenerator::getInstance()->sampleOrder(trainingData.size(), order);
      for (size_t i = 0; i < order.size(); ++i)
        doEpisode(context, function, trainingData[order[i]], compositeOnlineLearner);
    }
    else
      for (size_t i = 0; i < trainingData.size(); ++i)
        doEpisode(context, function, trainingData[i], compositeOnlineLearner);

    return compositeOnlineLearner->finishLearningIterationAndRemoveFinishedLearners(iteration);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
