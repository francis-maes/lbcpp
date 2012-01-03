/*-----------------------------------------.---------------------------------.
| Filename: BoostingWeakLearner.cpp        | Boosting Weak Learner           |
| Author  : Francis Maes                   |   base classes                  |
| Started : 22/12/2011 14:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeLearner.h>
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
double BoostingWeakLearner::computeWeakObjectiveWithEventualStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, LuapeNodePtr& weakNode, const IndexSetPtr& examples) const
{
  jassert(examples->size());
  if (weakNode->getType() == booleanType)
    return computeWeakObjective(context, structureLearner, weakNode, examples);
  else
  {
    jassert(weakNode->getType()->isConvertibleToDouble());
    double threshold;
    double res = computeWeakObjectiveWithStump(context, structureLearner, weakNode, examples, threshold);
    weakNode = makeStump(structureLearner, weakNode, threshold);
    return res;
  }
}

double BoostingWeakLearner::computeWeakObjective(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& indices) const
{
  LuapeSampleVectorPtr weakPredictions = structureLearner->getTrainingCache()->getSamples(context, weakNode, indices);
  BoostingWeakObjectivePtr edgeCalculator = structureLearner->createWeakObjective();
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  double res = edgeCalculator->compute(weakPredictions);
  const_cast<BoostingWeakLearner* >(this)->observeObjectiveValue(context, structureLearner, weakNode, indices, res);
  return res;
}

double BoostingWeakLearner::computeWeakObjectiveWithStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, const IndexSetPtr& indices, double& bestThreshold) const
{
  jassert(indices->size());
  BoostingWeakObjectivePtr weakObjective = structureLearner->createWeakObjective();
  double bestScore;
  SparseDoubleVectorPtr sortedDoubleValues = structureLearner->getTrainingCache()->getSortedDoubleValues(context, numberNode, indices);
  bestThreshold = weakObjective->findBestThreshold(context, indices, sortedDoubleValues, bestScore, false);
  const_cast<BoostingWeakLearner* >(this)->observeObjectiveValue(context, structureLearner, numberNode, indices, bestScore);
  return bestScore;
}

LuapeNodePtr BoostingWeakLearner::makeStump(const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, double threshold) const
{
  return new LuapeFunctionNode(stumpLuapeFunction(threshold), numberNode); // bypass universe
  //return structureLearner->getUniverse()->makeFunctionNode(stumpLuapeFunction(threshold), numberNode);
}

LuapeNodePtr BoostingWeakLearner::makeContribution(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, double weakObjective, const IndexSetPtr& examples) const
{
  if (!weakNode || weakObjective == -DBL_MAX)
    return LuapeNodePtr();
  const_cast<BoostingWeakLearner* >(this)->observeBestWeakNode(context, structureLearner, weakNode, examples, weakObjective);
  return structureLearner->turnWeakNodeIntoContribution(context, weakNode, weakObjective, examples);
}
