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
LuapeNodePtr LuapeLearner::learn(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
{
  // create initial node
  LuapeNodePtr node = problem->getRootNode();
  if (!node)
    node = createInitialNode(context, problem);
  if (node)
    problem->setRootNode(context, node);

  // learn
  IndexSetPtr indices = examples ? examples : problem->getTrainingCache()->getAllIndices();
  node = learn(context, node, problem, indices);
  if (!node)
    return LuapeNodePtr();
  problem->setRootNode(context, node);
  return node;
}

void LuapeLearner::evaluatePredictions(ExecutionContext& context, const LuapeInferencePtr& problem, double& trainingScore, double& validationScore)
{
  TimedScope _(context, "evaluate", verbose);
  trainingScore = problem->evaluatePredictions(context, problem->getTrainingPredictions(), problem->getTrainingSupervisions());
  context.resultCallback(T("train error"), trainingScore);

  if (problem->getValidationCache())
  {
    validationScore = problem->evaluatePredictions(context, problem->getValidationPredictions(), problem->getValidationSupervisions());
    context.resultCallback(T("validation error"), validationScore);
  }
  else
    validationScore = 0.0;
}

LuapeNodePtr LuapeLearner::subLearn(ExecutionContext& context, const LuapeLearnerPtr& subLearner, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double* objectiveValue) const
{
  if (!examples->size())
    return LuapeNodePtr();
  if (verbose)
    context.enterScope(T("Learning with ") + String((int)examples->size()) + T(" examples"));
  LuapeNodePtr weakNode = subLearner->learn(context, node, problem, examples);
  double score = subLearner->getBestObjectiveValue();
  if (verbose)
    context.leaveScope(score);
  if (!weakNode || score == -DBL_MAX)
    return LuapeNodePtr();
  if (objectiveValue)
    *objectiveValue = score;
  return weakNode;
}

void LuapeLearner::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  Object::clone(context, t);
  const LuapeLearnerPtr& target = t.staticCast<LuapeLearner>();
  if (objective)
    target->objective = objective->cloneAndCast<LearningObjective>(context);
  target->bestObjectiveValue = bestObjectiveValue;
}

/*
** IterativeLearner
*/
IterativeLearner::IterativeLearner(const LearningObjectivePtr& objective, size_t maxIterations)
  : LuapeLearner(objective), maxIterations(maxIterations), plotOutputStream(NULL)
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

LuapeNodePtr IterativeLearner::learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
{
  LuapeNodePtr res = node;
  if (verbose)
  {
    context.enterScope(T("Learning"));
    context.informationCallback(String((int)problem->getTrainingCache()->getNumSamples()) + T(" training examples"));
    if (problem->getValidationCache())
      context.informationCallback(String((int)problem->getValidationCache()->getNumSamples()) + T(" validation examples"));
  }

  if (plotOutputStream)
  {
    *plotOutputStream << "# " << String((int)problem->getTrainingCache()->getNumSamples()) << " training examples, "
                      << String(problem->getValidationCache() ? (int)problem->getValidationCache()->getNumSamples() : 0) << " validation examples\n";
    *plotOutputStream << "# Learner: " << toShortString() << "\n";
    *plotOutputStream << "# Iterations: " << String((int)maxIterations) << "\n\n";
    plotOutputStream->flush();
  }

  if (!initialize(context, res, problem, examples))
    return LuapeNodePtr();

  LuapeUniversePtr universe = problem->getUniverse();

  ScalarVariableMean lastIterationsValidationScore;
  double bestValidationScore = DBL_MAX;
  double trainingScore, validationScore;
  bool stopped = false;

  for (size_t i = 0; i < maxIterations && !stopped; ++i)
  {
    //learner->getTrainingCache()->displayCacheInformation(context);
    //if (i % 100 == 0)
    //  Object::displayObjectAllocationInfo(std::cout);

    if (verbose)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i+1);
    }
    
    if (!doLearningIteration(context, res, problem, examples, trainingScore, validationScore))
      stopped = true;

    if (verbose)
    {
      context.resultCallback("trainCacheSizeInMb", problem->getTrainingCache()->getCacheSizeInBytes() / (1024.0 * 1024.0));
      if (problem->getValidationCache())
        context.resultCallback("validationCacheSizeInMb", problem->getValidationCache()->getCacheSizeInBytes() / (1024.0 * 1024.0));
      context.resultCallback(T("log10(iteration)"), log10((double)i+1.0));
    }

    if (validationScore < bestValidationScore)
      bestValidationScore = validationScore;
    if (plotOutputStream)
    {
      *plotOutputStream << String((int)i+1) << " " << String(trainingScore) << " " << String(validationScore) << " " << String(bestValidationScore) << "\n";
      plotOutputStream->flush();
    }
    if (i >= 4 * maxIterations / 5)
      lastIterationsValidationScore.push(validationScore);

    if (verbose)
    {
      context.leaveScope();

      //  context.informationCallback(T("Graph: ") + learner->getGraph()->toShortString());
      context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
    
      if (i % 10 == 9)
      {
        context.enterScope(T("Most important nodes"));
        std::map<LuapeNodePtr, double> importances;
        LuapeUniverse::getImportances(problem->getRootNode(), importances);
        LuapeUniverse::displayMostImportantNodes(context, importances);
        context.leaveScope();
      }
    }
  }
  if (verbose)
  {
    context.leaveScope();

    context.enterScope(T("Most important nodes"));
    std::map<LuapeNodePtr, double> importances;
    LuapeUniverse::getImportances(problem->getRootNode(), importances);
    LuapeUniverse::displayMostImportantNodes(context, importances);
    context.leaveScope();

    context.informationCallback(T("Best evaluation: ") + String(bestValidationScore * 100.0, 3) + T("%"));
    context.informationCallback(T("Last 20% iteration evaluation: ") + String(lastIterationsValidationScore.getMean() * 100, 3) + T("%"));
  }
  
  if (plotOutputStream)
  {
    *plotOutputStream << "\n# best evaluation score: " << String(bestValidationScore * 100.0, 3) << "%\n";
    *plotOutputStream << "# last 20% iteration evaluation: " << String(lastIterationsValidationScore.getMean() * 100, 3) << "%\n";
    *plotOutputStream << "# final evaluation: " << String(validationScore * 100, 3) << "%\n";
    plotOutputStream->flush();
  }
    
  if (!finalize(context, res, problem, examples))
    return LuapeNodePtr();

  //Object::displayObjectAllocationInfo(std::cerr);
  //context.resultCallback("votes", function->getVotes());
  return res;
}

/*
** NodeBuilderBasedLearner
*/
NodeBuilderBasedLearner::NodeBuilderBasedLearner(LuapeNodeBuilderPtr nodeBuilder)
  : nodeBuilder(nodeBuilder)
{
}

void NodeBuilderBasedLearner::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  LuapeLearner::clone(context, target);
  if (nodeBuilder)
    target.staticCast<NodeBuilderBasedLearner>()->nodeBuilder = nodeBuilder->cloneAndCast<LuapeNodeBuilder>(context);
}

/*
** DecoratorLearner
*/
DecoratorLearner::DecoratorLearner(LuapeLearnerPtr decorated)
  : decorated(decorated)
{
}

LuapeNodePtr DecoratorLearner::createInitialNode(ExecutionContext& context, const LuapeInferencePtr& problem)
  {return decorated->createInitialNode(context, problem);}

LuapeNodePtr DecoratorLearner::learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
{
  if (!decorated->getObjective())
    decorated->setObjective(objective);
  decorated->setVerbose(verbose);
  LuapeNodePtr res = decorated->learn(context, node, problem, examples);
  bestObjectiveValue = decorated->getBestObjectiveValue();
  return res;
}

void DecoratorLearner::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  LuapeLearner::clone(context, target);
  if (decorated)
    target.staticCast<DecoratorLearner>()->decorated = decorated->cloneAndCast<LuapeLearner>();
}