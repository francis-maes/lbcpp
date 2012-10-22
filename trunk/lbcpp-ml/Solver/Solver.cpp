/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Optimizer Base Classes          |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Solver.h>
#include <lbcpp-ml/SolutionContainer.h>
#include <lbcpp-ml/Sampler.h>
using namespace lbcpp;

/*
** Solver
*/
SolutionContainerPtr Solver::createDefaultSolutionContainer(FitnessLimitsPtr limits) const
  {return new ParetoFront(limits);}

void Solver::configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution, Verbosity verbosity)
{
  this->problem = problem;
  this->solutions = solutions;
  this->verbosity = verbosity;
}

void Solver::clear(ExecutionContext& context)
{
  problem = ProblemPtr();
  solutions = SolutionContainerPtr();
  verbosity = verbosityQuiet;
}

SolutionContainerPtr Solver::optimize(ExecutionContext& context, ProblemPtr problem, ObjectPtr initialSolution, Verbosity verbosity)
{
  SolutionContainerPtr res = createDefaultSolutionContainer(problem->getFitnessLimits());
  
  configure(context, problem, res, initialSolution, verbosity);
  optimize(context);
  if (verbosity >= verbosityProgressAndResult)
    context.resultCallback("solutions", res);
  clear(context);
  return res;
}

FitnessPtr Solver::evaluate(ExecutionContext& context, const ObjectPtr& solution)
{
  jassert(problem && solutions);
  FitnessPtr fitness = problem->evaluate(context, solution);
  for (size_t i = 0; i < fitness->getNumValues(); ++i)
    jassert(isNumberValid(fitness->getValue(i)));
  jassert(fitness->getNumValues() == problem->getFitnessLimits()->getNumDimensions());
  solutions->insertSolution(solution, fitness);
  return fitness;
}

/*
** IterativeSolver
*/
void IterativeSolver::optimize(ExecutionContext& context)
{
  bool shouldContinue = true;
  for (size_t i = 0; (!numIterations || i < numIterations) && !problem->shouldStop() && shouldContinue; ++i)
  {
    //Object::displayObjectAllocationInfo(std::cout);

    if (verbosity >= verbosityDetailed)
    {
      context.enterScope("Iteration " + String((int)i + 1));
      context.resultCallback("iteration", i + 1);
    }

    shouldContinue = iteration(context, i);

    if (verbosity >= verbosityDetailed)
    {
      if (solutions->getNumSolutions() > 0)
      {
        if (problem->getNumObjectives() == 1)
        {
          // single-objective
          context.resultCallback("fitness", solutions->getFitness(0)->getValue(0));
          DoubleVectorPtr doubleVector = solutions->getSolution(0).dynamicCast<DoubleVector>();
          if (doubleVector)
          {
            context.resultCallback("l0norm", doubleVector->l0norm());
            context.resultCallback("l1norm", doubleVector->l1norm());
            context.resultCallback("l2norm", doubleVector->l2norm());
          }
        }
        else
        {
          // multi-objective
          FitnessLimitsPtr empiricalLimits = solutions->getEmpiricalFitnessLimits();
          for (size_t i = 0; i < empiricalLimits->getNumDimensions(); ++i)
          {
            context.resultCallback("objective" + String((int)i) + "lower", empiricalLimits->getLowerLimit(i));
            context.resultCallback("objective" + String((int)i) + "upper", empiricalLimits->getUpperLimit(i));
          }
          ParetoFrontPtr front = solutions.dynamicCast<ParetoFront>();
          if (front)
            context.resultCallback("hyperVolume", front->computeHyperVolume());
        }
      }
      context.leaveScope();
    }
    if (verbosity >= verbosityProgressAndResult)
    {
      context.resultCallback("hasConverged", !shouldContinue);
      context.progressCallback(new ProgressionState(i+1, numIterations, "Iterations"));
    }
  }
}

/*
** PopulationBasedSolver
*/
SolutionVectorPtr PopulationBasedSolver::sampleAndEvaluatePopulation(ExecutionContext& context, SamplerPtr sampler, size_t populationSize)
{
  SolutionVectorPtr res = new SolutionVector(problem->getFitnessLimits());
  for (size_t i = 0; i < populationSize && !problem->shouldStop(); ++i)
  {
    ObjectPtr solution = sampler->sample(context);
    FitnessPtr fitness = evaluate(context, solution);
    res->insertSolution(solution, fitness);
  }
  jassert(res->getNumSolutions() <= populationSize);
  return res;
}

void PopulationBasedSolver::computeMissingFitnesses(ExecutionContext& context, const SolutionVectorPtr& population)
{
  size_t n = population->getNumSolutions();
  for (size_t i = 0; i < n && !problem->shouldStop(); ++i)
    if (!population->getFitness(i))
      population->setFitness(i, evaluate(context, population->getSolution(i)));
}

void PopulationBasedSolver::learnSampler(ExecutionContext& context, SolutionVectorPtr solutions, SamplerPtr sampler)
  {sampler->learn(context, solutions->getObjects());}
