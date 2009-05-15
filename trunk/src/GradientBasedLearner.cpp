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

GradientBasedLearnerPtr GradientBasedLearner::createStochasticDescent(IterationFunctionPtr learningRate, bool normalizeLearningRate)
  {return GradientBasedLearnerPtr(new StochasticGradientDescentLearner(learningRate, normalizeLearningRate));}

GradientBasedLearnerPtr GradientBasedLearner::createBatch(VectorOptimizerPtr optimizer, OptimizerStoppingCriterionPtr stoppingCriterion)
  {return GradientBasedLearnerPtr(new BatchGradientBasedLearner(optimizer, stoppingCriterion));}

GradientBasedLearnerPtr GradientBasedLearner::createBatch(VectorOptimizerPtr optimizer, size_t maxIterations, double tolerance)
{
  OptimizerStoppingCriterionPtr stoppingCriterion;
  if (maxIterations > 0)
    stoppingCriterion = OptimizerStoppingCriterion::createMaxIterations(maxIterations);
  if (tolerance > 0)
  {
    OptimizerStoppingCriterionPtr c = OptimizerStoppingCriterion::createAverageImprovementThreshold(tolerance);
    stoppingCriterion = stoppingCriterion ? OptimizerStoppingCriterion::createOr(stoppingCriterion, c) : c;
  }
  assert(stoppingCriterion);
  return createBatch(optimizer, stoppingCriterion);
}

GradientBasedLearnerPtr GradientBasedLearner::createNonLearner()
  {return new NonLearnerGradientBasedLearner();}
