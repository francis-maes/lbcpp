/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Optimizer Base Classes          |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Optimizer.h>
#include <lbcpp-ml/SolutionSet.h>
#include <lbcpp-ml/Sampler.h>
using namespace lbcpp;

/*
** Optimizer
*/
void Optimizer::configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, Verbosity verbosity)
{
  this->front = front;
  this->problem = problem;
  this->verbosity = verbosity;
}

void Optimizer::clear(ExecutionContext& context)
{
  front = ParetoFrontPtr();
  problem = ProblemPtr();
  verbosity = verbosityQuiet;
}

ParetoFrontPtr Optimizer::optimize(ExecutionContext& context, ProblemPtr problem, Verbosity verbosity)
{
  ParetoFrontPtr res = new ParetoFront(problem->getFitnessLimits());
  
  configure(context, problem, res, verbosity);
  optimize(context);
  if (verbosity >= verbosityProgressAndResult)
  {
    context.resultCallback("front", res);
    context.resultCallback("hyperVolume", computeHyperVolume());
  }
  clear(context);
  return res;
}

double Optimizer::computeHyperVolume() const
  {return front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());}

FitnessPtr Optimizer::evaluate(ExecutionContext& context, const ObjectPtr& object)
{
  jassert(problem && front);
  FitnessPtr fitness = problem->evaluate(context, object);
  for (size_t i = 0; i < fitness->getNumValues(); ++i)
    jassert(isNumberValid(fitness->getValue(i)));
  front->addSolutionAndUpdateFront(object, fitness);
  return fitness;
}

FitnessPtr Optimizer::evaluateAndSave(ExecutionContext& context, const ObjectPtr& object, SolutionSetPtr solutions)
{
  FitnessPtr fitness = evaluate(context, object);
  solutions->addSolution(object, fitness);
  return fitness;
}

ObjectPtr Optimizer::sampleSolution(ExecutionContext& context, SamplerPtr sampler)
  {return problem->getDomain()->projectIntoDomain(sampler->sample(context));}
 
FitnessPtr Optimizer::sampleAndEvaluateSolution(ExecutionContext& context, SamplerPtr sampler, SolutionSetPtr population)
{
  ObjectPtr solution = sampleSolution(context, sampler); 
  return population ? evaluateAndSave(context, solution, population) : evaluate(context, solution);
}

SolutionSetPtr Optimizer::sampleAndEvaluatePopulation(ExecutionContext& context, SamplerPtr sampler, size_t populationSize)
{
  SolutionSetPtr res = new SolutionSet(problem->getFitnessLimits());
  for (size_t i = 0; i < populationSize; ++i)
    sampleAndEvaluateSolution(context, sampler, res);
  jassert(res->getNumSolutions() == populationSize);
  return res;
}

void Optimizer::learnSampler(ExecutionContext& context, SolutionSetPtr solutions, SamplerPtr sampler)
  {sampler->learn(context, solutions->getObjects());}

/*
** IterativeOptimizer
*/
void IterativeOptimizer::optimize(ExecutionContext& context)
{
  bool shouldContinue = true;
  for (size_t i = 0; (!numIterations || i < numIterations) && !problem->shouldStop() && shouldContinue; ++i)
  {
    if (verbosity >= verbosityDetailed)
    {
      context.enterScope("Iteration " + String((int)i + 1));
      context.resultCallback("iteration", i + 1);
    }

    shouldContinue = iteration(context, i);

    if (verbosity >= verbosityDetailed)
    {
      context.resultCallback("hyperVolume", computeHyperVolume());
      context.leaveScope();
    }
    if (verbosity >= verbosityProgressAndResult)
    {
      context.resultCallback("hasConverged", !shouldContinue);
      context.progressCallback(new ProgressionState(i+1, numIterations, "Iterations"));
    }
  }
}
