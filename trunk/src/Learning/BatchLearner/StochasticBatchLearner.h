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
    CompositeOnlineLearnerPtr compositeOnlineLearner = new CompositeOnlineLearner();
    for (size_t i = 0; i < functionsToLearn.size(); ++i)
      compositeOnlineLearner->startLearningAndAddLearner(context, functionsToLearn[i], maxIterations);

    if (stoppingCriterion)
      stoppingCriterion->reset();
    double bestMainScore = -DBL_MAX;
    for (size_t i = 0; !maxIterations || i < maxIterations; ++i)
    {
      context.enterScope(T("Learning Iteration ") + String((int)i + 1));
      context.resultCallback(T("Iteration"), i + 1);

      // Learning iteration
      jassert(compositeOnlineLearner->getNumLearners());
      compositeOnlineLearner->startLearningIteration(i);

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

      compositeOnlineLearner->finishLearningIterationAndRemoveFinishedLearners(i);
      bool learningFinished = (compositeOnlineLearner->getNumLearners() == 0);

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
        //context.informationCallback(T("Best score: ") + String(mainScore));
      }

      if (learningFinished || (stoppingCriterion && stoppingCriterion->shouldStop(mainScore)))
        break;
    }

    compositeOnlineLearner->finishLearning();
    
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
  std::vector<FunctionPtr> functionsToLearn;
  EvaluatorPtr evaluator;
  size_t maxIterations;
  StoppingCriterionPtr stoppingCriterion;
  bool randomizeExamples;
  bool restoreBestParametersWhenDone;

  void doEpisode(ExecutionContext& context, const FunctionPtr& function, const ObjectPtr& inputs, const OnlineLearnerPtr& onlineLearner) const
  {
    onlineLearner->startEpisode();
    function->computeWithInputsObject(context, inputs);
    onlineLearner->finishEpisode();
  }

  void storeParameters(const FunctionPtr& function) const
  {
    // FIXME
  }

  void restoreParameters(const FunctionPtr& function) const
  {
    // FIXME
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
