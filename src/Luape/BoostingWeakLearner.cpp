/*-----------------------------------------.---------------------------------.
| Filename: BoostingWeakLearner.cpp        | Boosting Weak Learner           |
| Author  : Francis Maes                   |   base classes                  |
| Started : 22/12/2011 14:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/BoostingWeakLearner.h>
#include <lbcpp/Luape/LuapeCache.h>
#include "NodeBuilder/NodeBuilderTypeSearchSpace.h"
#include "Function/SpecialLuapeFunctions.h" // for StumpLuapeFunction
using namespace lbcpp;

/*
** BoostingWeakObjective
*/
double BoostingWeakObjective::compute(const LuapeSampleVectorPtr& predictions)
{
  setPredictions(predictions);
  return computeObjective();
}

double BoostingWeakObjective::findBestThreshold(ExecutionContext& context, const IndexSetPtr& indices, const SparseDoubleVectorPtr& sortedDoubleValues, double& bestScore, bool verbose)
{
  setPredictions(LuapeSampleVector::createConstant(indices, Variable(false, booleanType)));

  if (sortedDoubleValues->getNumValues() == 0)
  {
    bestScore = computeObjective();
    return 0.0;
  }

  bestScore = -DBL_MAX;
  double res = 0.0;

  if (verbose)
    context.enterScope("Find best threshold for node");

  size_t n = sortedDoubleValues->getNumValues();
  double previousThreshold = sortedDoubleValues->getValue(n - 1).second;
  for (int i = (int)n - 1; i >= 0; --i)
  {
    size_t index = sortedDoubleValues->getValue(i).first;
    double threshold = sortedDoubleValues->getValue(i).second;

    jassert(threshold <= previousThreshold);
    if (threshold < previousThreshold)
    {
      double e = computeObjective();

      if (verbose)
      {
        context.enterScope("Iteration " + String((int)i));
        context.resultCallback("threshold", (threshold + previousThreshold) / 2.0);
        context.resultCallback("edge", e);
        context.leaveScope();
      }

      if (e >= bestScore)
        bestScore = e, res = (threshold + previousThreshold) / 2.0;
      previousThreshold = threshold;
    }
    flipPrediction(index);
  }

  if (verbose)
    context.leaveScope();
  return res;
}

/*
** BoostingWeakLearner
*/
double BoostingWeakLearner::computeWeakObjectiveWithEventualStump(ExecutionContext& context, const LuapeInferencePtr& problem, LuapeNodePtr& weakNode, const IndexSetPtr& examples) const
{
  jassert(examples->size());
  if (weakNode->getType() == booleanType)
    return computeWeakObjective(context, problem, weakNode, examples);
  else
  {
    jassert(weakNode->getType()->isConvertibleToDouble());
    double threshold;
    double res = computeWeakObjectiveWithStump(context, problem, weakNode, examples, threshold);
    weakNode = new LuapeFunctionNode(stumpLuapeFunction(threshold), weakNode);
    return res;
  }
}

double BoostingWeakLearner::computeWeakObjective(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& weakNode, const IndexSetPtr& indices) const
{
  LuapeSampleVectorPtr weakPredictions = problem->getTrainingCache()->getSamples(context, weakNode, indices);
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  return weakObjective->compute(weakPredictions);
}

double BoostingWeakLearner::computeWeakObjectiveWithStump(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& numberNode, const IndexSetPtr& indices, double& bestThreshold) const
{
  jassert(indices->size());
  double bestScore;
  SparseDoubleVectorPtr sortedDoubleValues = problem->getTrainingCache()->getSortedDoubleValues(context, numberNode, indices);
  bestThreshold = weakObjective->findBestThreshold(context, indices, sortedDoubleValues, bestScore, false);
  return bestScore;
}
