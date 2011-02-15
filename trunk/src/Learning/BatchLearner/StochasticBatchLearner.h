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

namespace lbcpp
{

class StochasticBatchLearner : public BatchLearner
{
public:
  StochasticBatchLearner(const std::vector<FunctionPtr>& functionsToLearn, EvaluatorPtr evaluator,
                    size_t maxIterations = 1000,
                    StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(),
                    bool randomizeExamples = true,
                    bool restoreBestParametersWhenDone = true)
    : functionsToLearn(functionsToLearn), evaluator(evaluator), maxIterations(maxIterations),
      randomizeExamples(randomizeExamples), restoreBestParametersWhenDone(restoreBestParametersWhenDone) {}

  StochasticBatchLearner() {}

  virtual TypePtr getRequiredExamplesType() const
    {return objectClass;} // frameObjectClass

  virtual FunctionPtr train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    RunningLearnerVector runningLearners;
    startLearning(function, runningLearners);

    if (stoppingCriterion)
      stoppingCriterion->reset();
    double bestMainScore = -DBL_MAX;
    for (size_t i = 0; runningLearners.size() && (!maxIterations || i < maxIterations); ++i)
    {
      context.enterScope(T("Learning Iteration ") + String((int)i + 1));
      context.resultCallback(T("Iteration"), i + 1);

      // Learning iteration
      startLearningIteration(function, i, maxIterations, runningLearners);

      if (randomizeExamples)
      {
        std::vector<size_t> order;
        RandomGenerator::getInstance()->sampleOrder(trainingData.size(), order);
        for (size_t i = 0; i < order.size(); ++i)
          doEpisode(context, function, trainingData[order[i]], runningLearners);
      }
      else
        for (size_t i = 0; i < trainingData.size(); ++i)
          doEpisode(context, function, trainingData[i], runningLearners);

      bool learningFinished = finishLearningIteration(context, runningLearners);

      // Evaluation
      EvaluatorPtr trainEvaluator = evaluator->cloneAndCast<Evaluator>();
      function->evaluate(context, trainingData, trainEvaluator);
      returnEvaluatorResults(context, trainEvaluator, T("Train"));
      EvaluatorPtr validationEvaluator;
      if (validationData.size())
      {
        validationEvaluator = evaluator->cloneAndCast<Evaluator>();
        function->evaluate(context, validationData, validationEvaluator);
        returnEvaluatorResults(context, validationEvaluator, T("Validation"));
      }

      double mainScore = validationEvaluator ? validationEvaluator->getDefaultScore() : trainEvaluator->getDefaultScore();
      
      context.leaveScope(mainScore);
      context.progressCallback(new ProgressionState(i + 1, maxIterations, T("Learning Iterations")));

      if (mainScore > bestMainScore)
      {
        if (restoreBestParametersWhenDone)
          storeParameters(function);
        bestMainScore = mainScore;
        context.informationCallback(T("Best score: ") + String(mainScore));
      }

      if (learningFinished || (stoppingCriterion && stoppingCriterion->shouldStop(mainScore)))
        break;
    }

    finishLearning();
    
    if (restoreBestParametersWhenDone)
      restoreParameters(function);

    return function;
  }

  void returnEvaluatorResults(ExecutionContext& context, EvaluatorPtr evaluator, const String& name) const
  {
    std::vector< std::pair<String, double> > results;
    evaluator->getScores(results);
    for (size_t i = 0; i < results.size(); ++i)
      context.resultCallback(name + T(" ") + results[i].first, results[i].second);
  }

protected:
  typedef std::vector< std::pair<FunctionPtr, OnlineLearnerPtr> > RunningLearnerVector;

  std::vector<FunctionPtr> functionsToLearn;
  EvaluatorPtr evaluator;
  size_t maxIterations;
  StoppingCriterionPtr stoppingCriterion;
  bool randomizeExamples;
  bool restoreBestParametersWhenDone;

  void doEpisode(ExecutionContext& context, const FunctionPtr& function, const ObjectPtr& inputs, const RunningLearnerVector& runningLearners) const
  {
    std::vector<Variable> in(function->getNumInputs());
    for (size_t i = 0; i < in.size(); ++i)
      in[i] = inputs->getVariable(i);

    startEpisode(runningLearners);
    function->compute(context, &in[0]);
    finishEpisode(runningLearners);
  }

  void storeParameters(const FunctionPtr& function) const
  {
    // FIXME
  }

  void restoreParameters(const FunctionPtr& function) const
  {
    // FIXME
  }

private:
  void startLearning(const FunctionPtr& function, RunningLearnerVector& runningLearners) const
  {
    for (size_t i = 0; i < functionsToLearn.size(); ++i)
    {
      OnlineLearnerPtr onlineLearner = functionsToLearn[i]->getOnlineLearner();
      jassert(onlineLearner);
      onlineLearner->startLearning(function);
      runningLearners.push_back(std::make_pair(functionsToLearn[i], onlineLearner));
    }
  }

  void finishLearning() const
  {
    for (int i = functionsToLearn.size() - 1; i >= 0; --i)
      functionsToLearn[i]->getOnlineLearner()->finishLearning();
  }

  static void startLearningIteration(const FunctionPtr& function, size_t iteration, size_t maxIterations, const RunningLearnerVector& runningLearners)
  {
    for (size_t i = 0; i < runningLearners.size(); ++i)
      runningLearners[i].second->startLearningIteration(runningLearners[i].first, iteration, maxIterations);
  }

  static bool finishLearningIteration(ExecutionContext& context, RunningLearnerVector& runningLearners)
  {
    bool learningFinished = true;
    for (int i = runningLearners.size() - 1; i >= 0; --i)
    {
      bool learnerFinished = runningLearners[i].second->finishLearningIteration(context, runningLearners[i].first);
      if (learnerFinished)
        runningLearners.erase(runningLearners.begin() + i);
      else
        learningFinished = false;
    }
    return learningFinished;
  }

  static void startEpisode(const RunningLearnerVector& runningLearners)
  {
    for (size_t i = 0; i < runningLearners.size(); ++i)
      runningLearners[i].second->startEpisode(runningLearners[i].first);
  }

  static void finishEpisode(const RunningLearnerVector& runningLearners)
  {
    for (int i = runningLearners.size() - 1; i >= 0; --i)
      runningLearners[i].second->finishEpisode(runningLearners[i].first);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
