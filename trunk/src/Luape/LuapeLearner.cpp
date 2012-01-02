/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.cpp               | Luape Graph Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeLearner.h>
#include <lbcpp/Luape/LuapeCache.h>
using namespace lbcpp;

/*
** LuapeLearner
*/
bool LuapeLearner::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  this->function = function;
  return true;
}

bool LuapeLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  if (isTrainingData)
  {
    trainingData = data;
    trainingCache = function->createSamplesCache(context, data);
    trainingCache->cacheNode(context, function->getRootNode(), VectorPtr(), "Prediction node", false);
  }
  else
  {
    validationData = data;
    validationCache = function->createSamplesCache(context, data);
    validationCache->cacheNode(context, function->getRootNode(), VectorPtr(), "Prediction node", false);
  }
  return true;
}

VectorPtr LuapeLearner::getTrainingPredictions() const
{
  LuapeSampleVectorPtr samples = trainingCache->getSamples(defaultExecutionContext(), function->getRootNode(), trainingCache->getAllIndices());
  jassert(samples->getImplementation() == LuapeSampleVector::cachedVectorImpl);
  return samples->getVector();
}

VectorPtr LuapeLearner::getValidationPredictions() const
{
  if (validationCache)
  {
    LuapeSampleVectorPtr samples = validationCache->getSamples(defaultExecutionContext(), function->getRootNode(), validationCache->getAllIndices());
    jassert(samples->getImplementation() == LuapeSampleVector::cachedVectorImpl);
    return samples->getVector();
  }
  else
    return VectorPtr();
}

void LuapeLearner::evaluatePredictions(ExecutionContext& context, double& trainingScore, double& validationScore)
{
  TimedScope _(context, "evaluate", verbose);
  VectorPtr trainingPredictions = getTrainingPredictions();
  trainingScore = function->evaluatePredictions(context, trainingPredictions, trainingData);
  context.resultCallback(T("train error"), trainingScore);

  VectorPtr validationPredictions = getValidationPredictions();
  if (validationPredictions)
  {
    validationScore = function->evaluatePredictions(context, validationPredictions, validationData);
    context.resultCallback(T("validation error"), validationScore);
  }
  else
    validationScore = 0.0;
}

/*
** IterativeLearner
*/
IterativeLearner::IterativeLearner(size_t maxIterations)
  : maxIterations(maxIterations), plotOutputStream(NULL)
{
}

IterativeLearner::~IterativeLearner()
{
  if (plotOutputStream)
    delete plotOutputStream;
}

void IterativeLearner::setPlotFile(ExecutionContext& context, const File& plotFile)
{
  jassert(!plotOutputStream);
  if (plotFile.existsAsFile())
    plotFile.deleteFile();
  plotOutputStream = plotFile.createOutputStream();
  if (!plotOutputStream)
    context.warningCallback(T("Could not create file ") + plotFile.getFullPathName());
}

bool IterativeLearner::learn(ExecutionContext& context)
{
  context.enterScope(T("Learning"));

  if (plotOutputStream)
  {
    *plotOutputStream << "# " << String((int)trainingData.size()) << " training examples, " << String((int)validationData.size()) << " validation examples\n";
    *plotOutputStream << "# Learner: " << toShortString() << "\n";
    *plotOutputStream << "# Iterations: " << String((int)maxIterations) << "\n\n";
    plotOutputStream->flush();
  }

  context.informationCallback(String((int)trainingData.size()) + T(" training examples"));
  if (validationData.size())
    context.informationCallback(String((int)validationData.size()) + T(" validation examples"));

  LuapeUniversePtr universe = function->getUniverse();

  ScalarVariableMean lastIterationsValidationScore;
  double bestValidationScore = DBL_MAX;
  for (size_t i = 0; i < maxIterations; ++i)
  {
    //learner->getTrainingCache()->displayCacheInformation(context);
    //Object::displayObjectAllocationInfo(std::cout);

    context.enterScope(T("Iteration ") + String((int)i + 1));
    context.resultCallback(T("iteration"), i+1);
    
    double trainingScore, validationScore;
    doLearningIteration(context, trainingScore, validationScore);
    if (verbose)
    {
      context.resultCallback("trainCacheSizeInMb", trainingCache->getCacheSizeInBytes() / (1024.0 * 1024.0));
      if (validationCache)
        context.resultCallback("validationCacheSizeInMb", validationCache->getCacheSizeInBytes() / (1024.0 * 1024.0));
    }
    context.resultCallback(T("log10(iteration)"), log10((double)i+1.0));

    if (validationScore < bestValidationScore)
      bestValidationScore = validationScore;
    if (plotOutputStream)
    {
      *plotOutputStream << String((int)i+1) << " " << String(trainingScore) << " " << String(validationScore) << " " << String(bestValidationScore) << "\n";
      plotOutputStream->flush();
    }
    if (i >= 4 * maxIterations / 5)
      lastIterationsValidationScore.push(validationScore);

    context.leaveScope();

    //  context.informationCallback(T("Graph: ") + learner->getGraph()->toShortString());
    context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
  
    if (verbose)
      context.enterScope(T("Most important nodes"));
    displayMostImportantNodes(context, function, verbose);
    if (verbose)
      context.leaveScope();
  }
  context.leaveScope();

  context.enterScope(T("Most important nodes"));
  displayMostImportantNodes(context, function, true);
  context.leaveScope();

  context.informationCallback(T("Best evaluation: ") + String(bestValidationScore * 100.0, 3) + T("%"));
  context.informationCallback(T("Last 20% iteration evaluation: ") + String(lastIterationsValidationScore.getMean() * 100, 3) + T("%"));
  if (plotOutputStream)
  {
    *plotOutputStream << "\n# best evaluation score: " << String(bestValidationScore * 100.0, 3) << "%\n";
    *plotOutputStream << "# last 20% iteration evaluation: " << String(lastIterationsValidationScore.getMean() * 100, 3) << "%\n\n";
    plotOutputStream->flush();
  }
    
  //Object::displayObjectAllocationInfo(std::cerr);
  //context.resultCallback("votes", function->getVotes());
  return true;
}

void IterativeLearner::getImportances(const LuapeNodePtr& node, std::map<LuapeNodePtr, double>& res) const
{
  if (node && res.find(node) == res.end())
  {
    double importance = node->getImportance();
    jassert(isNumberValid(importance));
    if (importance > 0)
    //if (!node.isInstanceOf<LuapeFunctionNode>() || node.staticCast<LuapeFunctionNode>()->getFunction()->getClassName() != T("StumpLuapeFunction"))
      res[node] = importance;
    //node->setImportance(0.0);
    size_t n = node->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      getImportances(node->getSubNode(i), res);
  }
}

void IterativeLearner::displayMostImportantNodes(ExecutionContext& context, const LuapeInferencePtr& function, bool verbose) const
{
  const LuapeNodePtr& rootNode = function->getRootNode();

  // get importance values
  std::map<LuapeNodePtr, double> importances;
  getImportances(rootNode, importances);

  // create probabilities and nodes vectors
  double Z = 0.0;
  std::vector<double> probabilities(importances.size());
  std::vector<LuapeNodePtr> nodes(importances.size());
  size_t index = 0;
  for (std::map<LuapeNodePtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it, ++index)
  {
    Z += it->second;
    probabilities[index] = it->second;
    nodes[index] = it->first;
  }

  // display most important nodes
  if (verbose)
  {
    std::multimap<double, LuapeNodePtr> nodeImportanceMap;
    for (std::map<LuapeNodePtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it)
      nodeImportanceMap.insert(std::make_pair(it->second, it->first));
    size_t i = 0;
    for (std::multimap<double, LuapeNodePtr>::reverse_iterator it = nodeImportanceMap.rbegin(); it != nodeImportanceMap.rend() && i < 20; ++it, ++i)
    {
      if (it->first <= 0.0)
        break;
      const LuapeNodePtr& node = it->second;
      context.informationCallback(T("# ") + String((int)i + 1) + T(": ") + node->toShortString() + T(" [") + String(it->first * 100.0 / Z, 2) + T("%]"));
    }
  }

  // sample new active variables
  function->clearActiveVariables();
  while (function->getNumActiveVariables() < 10 && Z > 1e-12)
  {
    jassert(isNumberValid(Z));
    size_t index = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);
    LuapeNodePtr node = nodes[index];
    if (!node.isInstanceOf<LuapeInputNode>())
    {
      context.informationCallback(T("Active variable: ") + node->toShortString());
      function->addActiveVariable(node);
    }
    Z -= probabilities[index];
    probabilities[index] = 0.0;
  }
}

/*
** BoostingLearner
*/
BoostingLearner::BoostingLearner(BoostingWeakLearnerPtr weakLearner, size_t maxIterations)
  : IterativeLearner(maxIterations), weakLearner(weakLearner) {}

bool BoostingLearner::initialize(ExecutionContext& context, const LuapeInferencePtr& function)
{
  if (!LuapeLearner::initialize(context, function))
    return false;
  jassert(weakLearner);
  return weakLearner->initialize(context, function);
}

LuapeNodePtr BoostingLearner::turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, double weakObjective, const IndexSetPtr& indices) const
{
  jassert(weakNode);
  Variable successVote, failureVote, missingVote;
  if (!computeVotes(context, weakNode, indices, successVote, failureVote, missingVote))
    return LuapeNodePtr();

  weakNode->addImportance(weakObjective);

  LuapeNodePtr res;
  jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
  if (weakNode.isInstanceOf<LuapeConstantNode>())
  {
    LuapeConstantNodePtr constantNode = weakNode.staticCast<LuapeConstantNode>();
    Variable constantValue = constantNode->getValue();
    jassert(constantValue.isBoolean());
    if (constantValue.isMissingValue())
      res = new LuapeConstantNode(missingVote);
    else if (constantValue.getBoolean())
      res = new LuapeConstantNode(successVote);
    else
      res = new LuapeConstantNode(failureVote);
  }
  else
    res = new LuapeTestNode(weakNode, new LuapeConstantNode(successVote), new LuapeConstantNode(failureVote), new LuapeConstantNode(missingVote));
  return res;
}

bool BoostingLearner::doLearningIteration(ExecutionContext& context, double& trainingScore, double& validationScore)
{
  LuapeNodePtr contribution;
  double weakObjective;
 
  // do weak learning
  {
    TimedScope _(context, "weak learning", verbose);
    contribution = weakLearner->learn(context, refCountedPointerFromThis(this), trainingCache->getAllIndices(), verbose, weakObjective);
    if (!contribution)
    {
      context.errorCallback(T("Failed to find a weak learner"));
      return false;
    }
    context.resultCallback(T("edge"), weakObjective);
  }

  // add into node and caches
  {
    TimedScope _(context, "add into node", verbose);
    std::vector<LuapeSamplesCachePtr> caches;
    caches.push_back(trainingCache);
    if (validationCache)
      caches.push_back(validationCache);
    function->getRootNode().staticCast<LuapeSequenceNode>()->pushNode(context, contribution, caches);
  }

  // evaluate
  evaluatePredictions(context, trainingScore, validationScore);

  // trainingCache->checkCacheIsCorrect(context, function->getRootNode());
  if (verbose)
    context.resultCallback(T("contribution"), verbose ? Variable(contribution) : Variable(contribution->toShortString()));
  return true;
}
