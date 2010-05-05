/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearner.cpp       | Gradient based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 23:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/GradientBasedLearner.h>
#include <lbcpp/FeatureGenerator/Optimizer.h>
#include <lbcpp/impl/impl.h>
#include "GradientBasedLearner/BatchGradientBasedLearner.h"
#include "GradientBasedLearner/NonLearnerGradientBasedLearner.h"
#include "GradientBasedLearner/StochasticGradientDescentLearner.h"
#include "GradientBasedLearner/MiniBatchGradientDescentLearner.h"
#include "GradientBasedLearner/StochasticToBatchGradientLearner.h"
using namespace lbcpp;

GradientBasedLearnerPtr lbcpp::stochasticDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate)
  {return new StochasticGradientDescentLearner(learningRate, normalizeLearningRate);}

GradientBasedLearnerPtr lbcpp::miniBatchDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate, size_t batchSize)
  {return new MiniBatchGradientDescentLearner(learningRate, normalizeLearningRate, batchSize);}

GradientBasedLearnerPtr lbcpp::batchLearner(VectorOptimizerPtr optimizer, StoppingCriterionPtr stoppingCriterion)
  {return new BatchGradientBasedLearner(optimizer, stoppingCriterion);}

GradientBasedLearnerPtr lbcpp::batchLearner(VectorOptimizerPtr optimizer, size_t maxIterations, double tolerance)
{
  StoppingCriterionPtr stoppingCriterion;
  if (maxIterations > 0)
    stoppingCriterion = maxIterationsStoppingCriterion(maxIterations);
  if (tolerance > 0)
  {
    StoppingCriterionPtr c = averageImprovementStoppingCriterion(tolerance);
    stoppingCriterion = stoppingCriterion ? logicalOr(stoppingCriterion, c) : c;
  }
  jassert(stoppingCriterion);
  return batchLearner(optimizer, stoppingCriterion);
}

GradientBasedLearnerPtr lbcpp::dummyLearner()
  {return new NonLearnerGradientBasedLearner();}

GradientBasedLearnerPtr lbcpp::stochasticToBatchLearner(GradientBasedLearnerPtr stochasticLearner, StoppingCriterionPtr stoppingCriterion, bool randomizeExamples)
  {return GradientBasedLearnerPtr(new StochasticToBatchGradientLearner(stochasticLearner, stoppingCriterion, randomizeExamples));}

GradientBasedLearnerPtr lbcpp::stochasticToBatchLearner(GradientBasedLearnerPtr stochasticLearner, size_t maxIterationsWithoutImprovement, size_t maxIterations, bool randomizeExamples)
{
  StoppingCriterionPtr criterion = logicalOr(
    maxIterationsWithoutImprovementStoppingCriterion(maxIterationsWithoutImprovement),
    maxIterationsStoppingCriterion(maxIterations));
  return stochasticToBatchLearner(stochasticLearner, criterion, randomizeExamples);
}

/*
** Serializable classes declaration
*/
void declareGradientBasedLearners()
{
  LBCPP_DECLARE_CLASS(StochasticGradientDescentLearner);
  LBCPP_DECLARE_CLASS(MiniBatchGradientDescentLearner);
  LBCPP_DECLARE_CLASS(BatchGradientBasedLearner);
  LBCPP_DECLARE_CLASS(NonLearnerGradientBasedLearner);
  LBCPP_DECLARE_CLASS(StochasticToBatchGradientLearner);
  
}
