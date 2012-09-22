/*-----------------------------------------.---------------------------------.
| Filename: Problem.cpp                    | Optimization Problem            |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Problem.h>
#include <lbcpp-ml/SolutionSet.h>
using namespace lbcpp;

/*
** DecoratorMOOProblem
*/
DecoratorMOOProblem::DecoratorMOOProblem(MOOProblemPtr problem)
  : problem(problem) {}

MOODomainPtr DecoratorMOOProblem::getObjectDomain() const
  {return problem->getObjectDomain();}

MOOFitnessLimitsPtr DecoratorMOOProblem::getFitnessLimits() const
  {return problem->getFitnessLimits();}

MOOFitnessPtr DecoratorMOOProblem::evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {return problem->evaluate(context, solution);}

ObjectPtr DecoratorMOOProblem::proposeStartingSolution(ExecutionContext& context) const
  {return problem->proposeStartingSolution(context);}

bool DecoratorMOOProblem::shouldStop() const
  {return problem->shouldStop();}

/*
** MaxIterationsDecoratorProblem
*/
MaxIterationsDecoratorProblem::MaxIterationsDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations)
  : DecoratorMOOProblem(problem), maxNumEvaluations(maxNumEvaluations), numEvaluations(0)
{
}

MaxIterationsDecoratorProblem::MaxIterationsDecoratorProblem()
  : maxNumEvaluations(0), numEvaluations(0)
{
}

MOOFitnessPtr MaxIterationsDecoratorProblem::evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {jassert(numEvaluations < maxNumEvaluations); ++numEvaluations; return DecoratorMOOProblem::evaluate(context, solution);}

bool MaxIterationsDecoratorProblem::shouldStop() const
  {return numEvaluations >= maxNumEvaluations || DecoratorMOOProblem::shouldStop();}

size_t MaxIterationsDecoratorProblem::getNumEvaluations() const
  {return numEvaluations;}

/*
** EvaluatorDecoratorProblem
*/
EvaluatorDecoratorProblem::EvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
  : MaxIterationsDecoratorProblem(problem, maxNumEvaluations), evaluationPeriod(evaluationPeriod) {}
EvaluatorDecoratorProblem::EvaluatorDecoratorProblem() : evaluationPeriod(0) {}

MOOFitnessPtr EvaluatorDecoratorProblem::evaluate(ExecutionContext& context, const ObjectPtr& solution)
{
  if (numEvaluations == 0)
    firstEvaluationTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
  MOOFitnessPtr res = decoratedEvaluate(context, solution);
  if ((numEvaluations % evaluationPeriod) == 0)
  {
    evaluate(context);
    cpuTimes.push_back(juce::Time::getMillisecondCounterHiRes() / 1000.0 - firstEvaluationTime);
  }
  return res;
}

const std::vector<double>& EvaluatorDecoratorProblem::getCpuTimes() const
  {return cpuTimes;}

size_t EvaluatorDecoratorProblem::getEvaluationPeriod() const
  {return evaluationPeriod;}

/*
** SingleObjectiveEvaluatorDecoratorProblem
*/
SingleObjectiveEvaluatorDecoratorProblem::SingleObjectiveEvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
  : EvaluatorDecoratorProblem(problem, maxNumEvaluations, evaluationPeriod) {jassert(problem->getNumObjectives() == 1);}
 
MOOFitnessPtr SingleObjectiveEvaluatorDecoratorProblem::decoratedEvaluate(ExecutionContext& context, const ObjectPtr& solution)
{
  MOOFitnessPtr res = MaxIterationsDecoratorProblem::evaluate(context, solution);
  if (!bestFitness || res->strictlyDominates(bestFitness))
    bestFitness = res;
  return res;
}

void SingleObjectiveEvaluatorDecoratorProblem::evaluate(ExecutionContext& context)
  {scores.push_back(bestFitness->getValue(0));}

const std::vector<double>& SingleObjectiveEvaluatorDecoratorProblem::getScores() const
  {return scores;}

/*
** HyperVolumeEvaluatorDecoratorProblem
*/
HyperVolumeEvaluatorDecoratorProblem::HyperVolumeEvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
  : EvaluatorDecoratorProblem(problem, maxNumEvaluations, evaluationPeriod)
  {front = new MOOParetoFront(problem->getFitnessLimits());}

MOOFitnessPtr HyperVolumeEvaluatorDecoratorProblem::decoratedEvaluate(ExecutionContext& context, const ObjectPtr& object)
{
  MOOFitnessPtr res = MaxIterationsDecoratorProblem::evaluate(context, object);
  front->addSolutionAndUpdateFront(object, res);
  return res;
}
 
void HyperVolumeEvaluatorDecoratorProblem::evaluate(ExecutionContext& context)
  {hyperVolumes.push_back(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));}

const std::vector<double>& HyperVolumeEvaluatorDecoratorProblem::getHyperVolumes() const
  {return hyperVolumes;}
