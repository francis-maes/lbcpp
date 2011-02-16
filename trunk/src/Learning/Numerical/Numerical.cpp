/*-----------------------------------------.---------------------------------.
| Filename: Numerical.cpp                  | Numerical Learning general stuff|
| Author  : Julien Becker                  |                                 |
| Started : 16/02/2011 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Learning/Numerical.h>
using namespace lbcpp;

StochasticGDParameters::StochasticGDParameters(IterationFunctionPtr learningRate,
                                                StoppingCriterionPtr stoppingCriterion,
                                                size_t maxIterations,
                                                EvaluatorPtr evaluator,
                                                bool doPerEpisodeUpdates,
                                                bool normalizeLearningRate,
                                                bool restoreBestParameters,
                                                bool randomizeExamples)
  : learningRate(learningRate), stoppingCriterion(stoppingCriterion), maxIterations(maxIterations), 
    evaluator(evaluator), doPerEpisodeUpdates(doPerEpisodeUpdates), normalizeLearningRate(normalizeLearningRate), 
    restoreBestParameters(restoreBestParameters), randomizeExamples(randomizeExamples)
{
}

BatchLearnerPtr StochasticGDParameters::createBatchLearner() const
{
  return stochasticBatchLearner(maxIterations, randomizeExamples);
}

OnlineLearnerPtr StochasticGDParameters::createOnlineLearner() const
{
  std::vector<OnlineLearnerPtr> learners;

  learners.push_back(doPerEpisodeUpdates ? perEpisodeGDOnlineLearner(learningRate, normalizeLearningRate) : stochasticGDOnlineLearner(learningRate, normalizeLearningRate));
  if (evaluator)
    learners.push_back(evaluatorOnlineLearner(evaluator));
  if (stoppingCriterion)
    learners.push_back(stoppingCriterionOnlineLearner(stoppingCriterion));
  if (restoreBestParameters)
    learners.push_back(restoreBestParametersOnlineLearner());

  return learners.size() == 1 ? learners[0] : compositeOnlineLearner(learners);
}
