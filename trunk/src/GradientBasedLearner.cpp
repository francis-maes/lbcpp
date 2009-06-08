/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearner.cpp       | Gradient based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 23:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/GradientBasedLearner.h>
#include <lbcpp/Optimizer.h>
#include <lbcpp/impl/impl.h>
#include "GradientBasedLearner/BatchGradientBasedLearner.h"
#include "GradientBasedLearner/NonLearnerGradientBasedLearner.h"
#include "GradientBasedLearner/StochasticGradientDescentLearner.h"
using namespace lbcpp;

GradientBasedLearnerPtr lbcpp::stochasticDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate)
  {return new StochasticGradientDescentLearner(learningRate, normalizeLearningRate);}

GradientBasedLearnerPtr lbcpp::batchLearner(VectorOptimizerPtr optimizer, OptimizerStoppingCriterionPtr stoppingCriterion)
  {return new BatchGradientBasedLearner(optimizer, stoppingCriterion);}

GradientBasedLearnerPtr lbcpp::batchLearner(VectorOptimizerPtr optimizer, size_t maxIterations, double tolerance)
{
  OptimizerStoppingCriterionPtr stoppingCriterion;
  if (maxIterations > 0)
    stoppingCriterion = maxIterationsStoppingCriterion(maxIterations);
  if (tolerance > 0)
  {
    OptimizerStoppingCriterionPtr c = averageImprovementThresholdStoppingCriterion(tolerance);
    stoppingCriterion = stoppingCriterion ? logicalOr(stoppingCriterion, c) : c;
  }
  assert(stoppingCriterion);
  return batchLearner(optimizer, stoppingCriterion);
}

GradientBasedLearnerPtr lbcpp::dummyLearner()
  {return new NonLearnerGradientBasedLearner();}
