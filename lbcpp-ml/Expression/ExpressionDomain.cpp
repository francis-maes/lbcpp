/*-----------------------------------------.---------------------------------.
| Filename: ExpressionDomain.cpp           | Expression Domain               |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/ExpressionDomain.h>
#include "../../lbcpp-core/Luape/NodeBuilder/NodeBuilderDecisionProblem.h"
using namespace lbcpp;

ExpressionDomain::ExpressionDomain(ExpressionUniversePtr universe)
  : universe(universe)
{
  if (!universe)
    this->universe = new ExpressionUniverse();
}

VariableExpressionPtr ExpressionDomain::addInput(const TypePtr& type, const String& name)
{
  size_t index = inputs.size();
  VariableExpressionPtr res(new VariableExpression(type, name, index));
  inputs.push_back(res);
  return res;
}

VariableExpressionPtr ExpressionDomain::createSupervision(const TypePtr& type, const String& name)
{
  supervision = new VariableExpression(type, name, inputs.size());
  return supervision;
}

ExpressionPtr ExpressionDomain::getActiveVariable(size_t index) const
{
  jassert(index < activeVariables.size());
  std::set<ExpressionPtr>::const_iterator it = activeVariables.begin();
  for (size_t i = 0; i < index; ++i)
    ++it;
  return *it;
}

bool ExpressionDomain::isTargetTypeAccepted(TypePtr type)
{
  if (!targetTypes.size())
  {
    // old definition for Luape weak learners
    return type->inheritsFrom(booleanType) || type->inheritsFrom(doubleType) ||
      (!type.isInstanceOf<Enumeration>() && type->inheritsFrom(integerType));
  }

  for (std::set<TypePtr>::const_iterator it = targetTypes.begin(); it != targetTypes.end(); ++it)
    if (type->inheritsFrom(*it))
      return true;
  return false;
}

ExpressionRPNTypeSpacePtr ExpressionDomain::getSearchSpace(ExecutionContext& context, size_t complexity, bool verbose) const
{
  ScopedLock _(typeSearchSpacesLock);

  ExpressionDomain* pthis = const_cast<ExpressionDomain* >(this);

  if (complexity >= typeSearchSpaces.size())
    pthis->typeSearchSpaces.resize(complexity + 1);
  if (typeSearchSpaces[complexity])
    return typeSearchSpaces[complexity];

  return (pthis->typeSearchSpaces[complexity] = createTypeSearchSpace(context, std::vector<TypePtr>(), complexity, verbose));
}

ExpressionRPNTypeSpacePtr ExpressionDomain::createTypeSearchSpace(ExecutionContext& context, const std::vector<TypePtr>& initialState, size_t complexity, bool verbose) const
{
  ExpressionRPNTypeSpacePtr res = new ExpressionRPNTypeSpace(refCountedPointerFromThis(this), initialState, complexity);
  res->pruneStates(context, verbose);
  res->assignStateIndices(context);
  return res;
}

static void enumerateExhaustively(ExecutionContext& context, ExpressionBuilderStatePtr state, std::vector<ExpressionPtr>& res, bool verbose)
{
  if (state->isFinalState() && state->getStackSize() == 1)
  {
    ExpressionPtr node = state->getStackElement(0);
    res.push_back(node);
    //if (verbose)
    //  context.informationCallback(node->toShortString());
  }
  else
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable stateBackup;
      Variable action = actions->getElement(i);
      double reward;
      state->performTransition(context, action, reward, &stateBackup);
      enumerateExhaustively(context, state, res, verbose);
      state->undoTransition(context, stateBackup);
    }
  }
}

void ExpressionDomain::enumerateNodesExhaustively(ExecutionContext& context, size_t complexity, std::vector<ExpressionPtr>& res, bool verbose, const ExpressionRPNSequencePtr& subSequence) const
{
  ExpressionRPNTypeSpacePtr typeSearchSpace;
  if (subSequence)
    typeSearchSpace = createTypeSearchSpace(context, subSequence->computeTypeState(), complexity, verbose); // create on-demand
  else
    typeSearchSpace = getSearchSpace(context, complexity, verbose); // use cached version

  ExpressionBuilderStatePtr state = new ExpressionBuilderState(refCountedPointerFromThis(this), typeSearchSpace, subSequence);
  enumerateExhaustively(context, state, res, verbose);
}

LuapeSamplesCachePtr ExpressionDomain::createCache(size_t size, size_t maxCacheSizeInMb) const
    {return new LuapeSamplesCache(universe, inputs, size, maxCacheSizeInMb);}

LuapeSamplesCachePtr ExpressionDomain::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const
{
  size_t n = data.size();
  LuapeSamplesCachePtr res = createCache(n, 512); // default: 512 Mb cache
  VectorPtr supervisionValues = vector(data[0]->getVariableType(1), n);
  for (size_t i = 0; i < n; ++i)
  {
    res->setInputObject(inputs, i, data[i]->getVariable(0).getObject());
    supervisionValues->setElement(i, data[i]->getVariable(1));
  }
  res->cacheNode(context, supervision, supervisionValues, T("Supervision"), false);
  res->recomputeCacheSize();
  return res;
}

void ExpressionDomain::setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
{
  createSupervision(trainingData[0]->getVariableType(1), T("supervision"));
  trainingCache = createSamplesCache(context, trainingData);
  if (validationData.size())
    validationCache = createSamplesCache(context, validationData);
}

std::vector<LuapeSamplesCachePtr> ExpressionDomain::getSamplesCaches() const
{
  std::vector<LuapeSamplesCachePtr> res;
  res.push_back(trainingCache);
  if (validationCache)
    res.push_back(validationCache);
  return res;
}

VectorPtr ExpressionDomain::getTrainingPredictions() const
{
  jassert(trainingCache->isNodeDefinitivelyCached(node));
  return trainingCache->getNodeCache(node);
}

VectorPtr ExpressionDomain::getTrainingSupervisions() const
{
  jassert(trainingCache->isNodeDefinitivelyCached(supervision));
  return trainingCache->getNodeCache(supervision);
}

VectorPtr ExpressionDomain::getValidationPredictions() const
{
  if (!validationCache)
    return VectorPtr();
  jassert(validationCache->isNodeDefinitivelyCached(node));
  return validationCache->getNodeCache(node);
}

VectorPtr ExpressionDomain::getValidationSupervisions() const
{
  if (!validationCache)
    return VectorPtr();
  jassert(validationCache->isNodeDefinitivelyCached(supervision));
  return validationCache->getNodeCache(supervision);
}

/*
** Deprecated
*/

Variable ExpressionDomain::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  return computeNode(context, inputs[0].getObject());
}

Variable ExpressionDomain::computeNode(ExecutionContext& context, const ObjectPtr& inputObject) const
{
  LuapeInstanceCachePtr cache = new LuapeInstanceCache();
  cache->setInputObject(inputs, inputObject);
  return cache->compute(context, node);
}

/*void ExpressionDomain::setLearner(const LuapeLearnerPtr& learner, bool verbose)
{
  learner->setVerbose(verbose);
  //setBatchLearner(new LuapeBatchLearner(learner));
}*/

void ExpressionDomain::setRootNode(ExecutionContext& context, const ExpressionPtr& node)
{
  if (node != this->node)
  {
    if (this->node)
    {
      if (trainingCache)
        trainingCache->uncacheNode(context, this->node);
      if (validationCache)
        validationCache->uncacheNode(context, this->node);
    }
    this->node = node;
    if (this->node)
    {
      if (trainingCache)
        trainingCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
      if (validationCache)
        validationCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
    }
  }
}

void ExpressionDomain::clearRootNode(ExecutionContext& context)
  {if (node) setRootNode(context, ExpressionPtr());}
