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
#include "BoostingWeakLearner/LuapeGraphBuilderTypeSearchSpace.h"
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

/*
** FiniteBoostingWeakLearner
*/
LuapeNodePtr FiniteBoostingWeakLearner::learn(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const IndexSetPtr& examples, bool verbose, double& weakObjective)
{
  const LuapeInferencePtr& function = structureLearner->getFunction();

  std::vector<LuapeNodePtr> weakNodes;
  if (!getCandidateWeakNodes(context, structureLearner, weakNodes))
  {
    context.errorCallback(T("Could not get finite set of candidate weak nodes"));
    return LuapeNodePtr();
  }

  weakObjective = -DBL_MAX;
  LuapeNodePtr bestWeakNode;
  ScalarVariableMeanAndVariance objectiveStats;
  //std::vector< std::pair<double, double> > objectiveAndThresholds(weakNodes.size());
  for (size_t i = 0; i < weakNodes.size(); ++i)
  {
    LuapeNodePtr weakNode = weakNodes[i];
    double objective = computeWeakObjectiveWithEventualStump(context, structureLearner, weakNode, examples); // side effect of weakNode
    objectiveStats.push(objective);
    /*double threshold = 0.0;
    if (weakNode.isInstanceOf<LuapeFunctionNode>())
    {
      StumpLuapeFunctionPtr stump = weakNode.staticCast<LuapeFunctionNode>()->getFunction().dynamicCast<StumpLuapeFunction>();
      if (stump)
        threshold = stump->getThreshold();
    }
    objectiveAndThresholds[i] = std::make_pair(objective, threshold);*/
    if (objective > weakObjective)
      weakObjective = objective, bestWeakNode = weakNode;
  }

/*  for (size_t i = 0; i < weakNodes.size(); ++i)
  {
    double objective = objectiveAndThresholds[i].first;
    double threshold = objectiveAndThresholds[i].second;
    String name = weakNodes[i]->toShortString();
    context.resultCallback(name, (objective - objectiveStats.getMean()) / objectiveStats.getStandardDeviation());
    context.resultCallback(name + " threshold", threshold);
  }*/
  return makeContribution(context, structureLearner, bestWeakNode, weakObjective, examples);
}

/*
** StochasticFiniteBoostingWeakLearner
*/
bool StochasticFiniteBoostingWeakLearner::getCandidateWeakNodes(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& res) const
{
  size_t numFailuresAllowed = 10 * numWeakNodes;
  size_t numFailures = 0;
  std::set<LuapeNodePtr> weakNodes;
  while (weakNodes.size() < numWeakNodes && numFailures < numFailuresAllowed)
  {
    LuapeNodePtr weakNode = sampleWeakNode(context, structureLearner);
    if (weakNode && weakNodes.find(weakNode) == weakNodes.end())
      weakNodes.insert(weakNode);
    else
      ++numFailures;
  }

  context.resultCallback(T("numSamplingFailures"), numFailures);

  size_t index = res.size();
  res.resize(index + weakNodes.size());
  for (std::set<LuapeNodePtr>::const_iterator it = weakNodes.begin(); it != weakNodes.end(); ++it)
  {
    //context.informationCallback(T("Candidate: ") + (*it)->toShortString());
    res[index++] = *it;
  }
  return true;
}

/*
** SequentialBuilderWeakLearner
*/
SequentialBuilderWeakLearner::SequentialBuilderWeakLearner(size_t numWeakNodes, size_t maxSteps)
  : StochasticFiniteBoostingWeakLearner(numWeakNodes), maxSteps(maxSteps)
{
}

bool SequentialBuilderWeakLearner::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  universe = function->getUniverse();
  typeSearchSpace = new LuapeGraphBuilderTypeSearchSpace(function, maxSteps);
  typeSearchSpace->pruneStates(context);
  typeSearchSpace->assignStateIndices(context);
  return true;
}

LuapeNodePtr SequentialBuilderWeakLearner::sampleWeakNode(ExecutionContext& context, const LuapeLearnerPtr& structureLearner) const
{
  RandomGeneratorPtr random = context.getRandomGenerator();

  std::vector<LuapeNodePtr> stack;
  LuapeGraphBuilderTypeStatePtr typeState;

  for (size_t i = 0; i < maxSteps; ++i)
  {
    // Retrieve type-state index
    LuapeGraphBuilderTypeStatePtr typeState = getTypeState(i, stack);
    jassert(typeState);

    // Sample action
    ObjectPtr action;
    size_t numFailuresAllowed = 100;
    size_t numFailures;
    for (numFailures = 0; numFailures < numFailuresAllowed; ++numFailures)
      if (sampleAction(context, typeState, action) && isActionAvailable(action, stack))
        break;
    const_cast<SequentialBuilderWeakLearner* >(this)->samplingDone(context, numFailures, numFailuresAllowed);
    if (numFailures == numFailuresAllowed)
      return LuapeNodePtr();

    // Execute action
    executeAction(stack, action);
    if (!action)
    {
      //context.informationCallback(T("Candidate: ") + stack[0]->toShortString());
      return stack[0]; // yield action
    }
  }

  context.informationCallback("Failed to sample candidate weak node");
  return LuapeNodePtr();
}

bool SequentialBuilderWeakLearner::isActionAvailable(ObjectPtr action, const std::vector<LuapeNodePtr>& stack)
{
  return !action || !action.isInstanceOf<LuapeFunction>() ||
    action.staticCast<LuapeFunction>()->acceptInputsStack(stack);
}

LuapeGraphBuilderTypeStatePtr SequentialBuilderWeakLearner::getTypeState(size_t stepNumber, const std::vector<LuapeNodePtr>& stack) const
{
  std::vector<TypePtr> typeStack(stack.size());
  for (size_t j = 0; j < typeStack.size(); ++j)
    typeStack[j] = stack[j]->getType();
  return typeSearchSpace->getState(stepNumber, typeStack);
}

void SequentialBuilderWeakLearner::executeAction(std::vector<LuapeNodePtr>& stack, const ObjectPtr& action) const
{
  // Execute action
  if (action)
  {
    if (action.isInstanceOf<LuapeNode>())
      stack.push_back(action);   // push action
    else
    {
      // apply action
      LuapeFunctionPtr function = action.staticCast<LuapeFunction>();
      size_t n = function->getNumInputs();
      jassert(stack.size() >= n && n > 0);
      std::vector<LuapeNodePtr> inputs(n);
      for (size_t i = 0; i < n; ++i)
        inputs[i] = stack[stack.size() - n + i];
      stack.erase(stack.begin() + stack.size() - n, stack.end());
      stack.push_back(universe->makeFunctionNode(function, inputs));
    }
  }
  else
  {
    // yield action
    jassert(stack.size() == 1);
  }
}
