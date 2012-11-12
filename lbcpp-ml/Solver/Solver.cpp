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
void Solver::startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
{
  this->problem = problem;
  this->callback = callback;
}

void Solver::stopSolver(ExecutionContext& context)
{
  problem = ProblemPtr();
  callback = SolverCallbackPtr();
}

void Solver::solve(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
{
  startSolver(context, problem, callback, startingSolution);
  callback->solverStarted(context, refCountedPointerFromThis(this));
  runSolver(context);
  callback->solverStopped(context, refCountedPointerFromThis(this));
  stopSolver(context);
}

FitnessPtr Solver::evaluate(ExecutionContext& context, const ObjectPtr& object)
{
  jassert(problem && callback);
  FitnessPtr fitness = problem->evaluate(context, object);
  for (size_t i = 0; i < fitness->getNumValues(); ++i)
    jassert(isNumberValid(fitness->getValue(i)));
  jassert(fitness->getNumValues() == problem->getFitnessLimits()->getNumDimensions());
  callback->solutionEvaluated(context, refCountedPointerFromThis(this), object, fitness);
  return fitness;
}

/*
** IterativeSolver
*/
void IterativeSolver::runSolver(ExecutionContext& context)
{
  bool shouldContinue = true;
  for (size_t i = 0; (!numIterations || i < numIterations) && !callback->shouldStop() && shouldContinue; ++i)
  {
    //Object::displayObjectAllocationInfo(std::cout);

    if (verbosity >= verbosityDetailed)
    {
      context.enterScope("Iteration " + string((int)i + 1));
      context.resultCallback("iteration", i + 1);
    }

    shouldContinue = iterateSolver(context, i);

    if (verbosity >= verbosityDetailed)
    {
#if 0
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
            context.resultCallback("objective" + string((int)i) + "lower", empiricalLimits->getLowerLimit(i));
            context.resultCallback("objective" + string((int)i) + "upper", empiricalLimits->getUpperLimit(i));
          }
          ParetoFrontPtr front = solutions.dynamicCast<ParetoFront>();
          if (front)
            context.resultCallback("hyperVolume", front->computeHyperVolume());
        }
      }
#endif // 0
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
  for (size_t i = 0; i < populationSize && !callback->shouldStop(); ++i)
  {
    ObjectPtr solution = problem->getDomain()->projectIntoDomain(sampler->sample(context));
    FitnessPtr fitness = evaluate(context, solution);
    res->insertSolution(solution, fitness);
  }
  jassert(res->getNumSolutions() <= populationSize);
  return res;
}

void PopulationBasedSolver::computeMissingFitnesses(ExecutionContext& context, const SolutionVectorPtr& population)
{
  size_t n = population->getNumSolutions();
  for (size_t i = 0; i < n && !callback->shouldStop(); ++i)
    if (!population->getFitness(i))
      population->setFitness(i, evaluate(context, population->getSolution(i)));
}

void PopulationBasedSolver::learnSampler(ExecutionContext& context, SolutionVectorPtr solutions, SamplerPtr sampler)
  {sampler->learn(context, solutions->getObjects());}
