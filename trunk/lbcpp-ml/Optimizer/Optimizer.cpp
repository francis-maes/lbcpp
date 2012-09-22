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
** MOOOptimizer
*/
void MOOOptimizer::configure(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr front, Verbosity verbosity)
{
  this->front = front;
  this->problem = problem;
  this->verbosity = verbosity;
}

void MOOOptimizer::clear(ExecutionContext& context)
{
  front = MOOParetoFrontPtr();
  problem = MOOProblemPtr();
  verbosity = verbosityQuiet;
}

MOOParetoFrontPtr MOOOptimizer::optimize(ExecutionContext& context, MOOProblemPtr problem, Verbosity verbosity)
{
  MOOParetoFrontPtr res = new MOOParetoFront(problem->getFitnessLimits());
  
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

double MOOOptimizer::computeHyperVolume() const
  {return front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());}

MOOFitnessPtr MOOOptimizer::evaluate(ExecutionContext& context, const ObjectPtr& object)
{
  jassert(problem && front);
  MOOFitnessPtr fitness = problem->evaluate(context, object);
  for (size_t i = 0; i < fitness->getNumValues(); ++i)
    jassert(isNumberValid(fitness->getValue(i)));
  front->addSolutionAndUpdateFront(object, fitness);
  return fitness;
}

MOOFitnessPtr MOOOptimizer::evaluateAndSave(ExecutionContext& context, const ObjectPtr& object, MOOSolutionSetPtr solutions)
{
  MOOFitnessPtr fitness = evaluate(context, object);
  solutions->addSolution(object, fitness);
  return fitness;
}

ObjectPtr MOOOptimizer::sampleSolution(ExecutionContext& context, MOOSamplerPtr sampler)
  {return problem->getObjectDomain()->projectIntoDomain(sampler->sample(context));}
 
MOOFitnessPtr MOOOptimizer::sampleAndEvaluateSolution(ExecutionContext& context, MOOSamplerPtr sampler, MOOSolutionSetPtr population)
{
  ObjectPtr solution = sampleSolution(context, sampler); 
  return population ? evaluateAndSave(context, solution, population) : evaluate(context, solution);
}

MOOSolutionSetPtr MOOOptimizer::sampleAndEvaluatePopulation(ExecutionContext& context, MOOSamplerPtr sampler, size_t populationSize)
{
  MOOSolutionSetPtr res = new MOOSolutionSet(problem->getFitnessLimits());
  for (size_t i = 0; i < populationSize; ++i)
    sampleAndEvaluateSolution(context, sampler, res);
  jassert(res->getNumSolutions() == populationSize);
  return res;
}

void MOOOptimizer::learnSampler(ExecutionContext& context, MOOSolutionSetPtr solutions, MOOSamplerPtr sampler)
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
