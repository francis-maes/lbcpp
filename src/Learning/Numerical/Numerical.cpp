/*-----------------------------------------.---------------------------------.
| Filename: Numerical.cpp                  | Numerical Learning general stuff|
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Learning/Numerical.h>
#include <lbcpp/Learning/LossFunction.h>
using namespace lbcpp;

/*
** StochasticGDParameters
*/
StochasticGDParameters::StochasticGDParameters(IterationFunctionPtr learningRate,
                                                StoppingCriterionPtr stoppingCriterion,
                                                size_t maxIterations,
                                                bool doPerEpisodeUpdates,
                                                bool normalizeLearningRate,
                                                bool restoreBestParameters,
                                                bool randomizeExamples,
                                                bool evaluateAtEachIteration)
  : learningRate(learningRate), stoppingCriterion(stoppingCriterion), maxIterations(maxIterations), 
    doPerEpisodeUpdates(doPerEpisodeUpdates), normalizeLearningRate(normalizeLearningRate), 
    restoreBestParameters(restoreBestParameters), randomizeExamples(randomizeExamples), evaluateAtEachIteration(evaluateAtEachIteration)
{
}

BatchLearnerPtr StochasticGDParameters::createBatchLearner(ExecutionContext& context) const
{
  return stochasticBatchLearner(maxIterations, randomizeExamples);
}

OnlineLearnerPtr StochasticGDParameters::createOnlineLearner(ExecutionContext& context) const
{
  // create gradient descent learner
  std::vector<OnlineLearnerPtr> learners;
  learners.push_back(doPerEpisodeUpdates
    ? perEpisodeGDOnlineLearner(lossFunction, learningRate, normalizeLearningRate)
    : stochasticGDOnlineLearner(lossFunction, learningRate, normalizeLearningRate));

  // create other optionnal online learners
  if (evaluateAtEachIteration)
    learners.push_back(evaluatorOnlineLearner());
  if (stoppingCriterion)
    learners.push_back(stoppingCriterionOnlineLearner(stoppingCriterion));
  if (restoreBestParameters)
    learners.push_back(restoreBestParametersOnlineLearner());

  return learners.size() == 1 ? learners[0] : compositeOnlineLearner(learners);
}

/*
** Conversion stuff
*/
bool lbcpp::convertSupervisionVariableToBoolean(const Variable& supervision, bool& result)
{
  if (!supervision.exists())
    return false;
  if (supervision.isBoolean())
  {
    result = supervision.getBoolean();
    return true;
  }
  if (supervision.getType() == probabilityType)
  {
    result = supervision.getDouble() > 0.5;
    return true;
  }
  return false;
}

