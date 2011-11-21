/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.cpp          | Luape Graph Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeLearner.h"
using namespace lbcpp;

/*
** LuapeLearner
*/
bool LuapeLearner::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
{
  this->problem = problem;
  this->function = function;
  if (!function->getVotes())
    function->setVotes(function->createVoteVector(0));
  if (!function->getGraph())
    function->setGraph(problem->createInitialGraph(context));
  this->graph = function->getGraph();
  return !problem->getObjective() || problem->getObjective()->initialize(context, problem, function);
}

bool LuapeLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  graph->clearSamples(isTrainingData, !isTrainingData);
  if (problem->getObjective())
    problem->getObjective()->setExamples(isTrainingData, data);
  return true;
}
