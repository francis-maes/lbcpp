/*-----------------------------------------.---------------------------------.
| Filename: SurrogateBasedColoSolver.h     | Surrogate Based Colo Solver     |
| Author  : Francis Maes                   |                                 |
| Started : 08/01/2013 16:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef COLO_SOLVER_SURROGATE_BASED_H_
# define COLO_SOLVER_SURROGATE_BASED_H_

# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/ExpressionDomain.h>
# include "ColoProblem.h"
# include "../../lib/ml/Solver/SurrogateBasedSolver.h"

namespace lbcpp
{

class SurrogateBasedMOSolver : public SurrogateBasedSolver
{
public:
  SurrogateBasedMOSolver(SamplerPtr initialSampler, size_t numInitialSamples, SolverPtr surrogateLearner, SolverPtr surrogateSolver, size_t numIterations)
    : SurrogateBasedSolver(initialSampler, numInitialSamples, surrogateLearner, surrogateSolver, numIterations) {}
  SurrogateBasedMOSolver() {}

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    std::vector<ObjectPtr> objects;
    if (iter == 0)
    {
      // make random samples
      objects.resize(numInitialSamples);
      for (size_t i = 0; i < numInitialSamples; ++i)
        objects[i] = initialSampler->sample(context);
    }
    else
    {
      // learn surrogate
      if (verbosity >= verbosityDetailed)
        context.enterScope("Learn surrogate");
      ExpressionPtr surrogateModel = learnSurrogateModel(context, surrogateLearningProblem);
      if (verbosity >= verbosityDetailed)
      {
        context.resultCallback("surrogateModel", surrogateModel);
        context.leaveScope();
      }
      
      // optimize surrogate
      if (verbosity >= verbosityDetailed)
        context.enterScope("Optimize surrogate");
      ParetoFrontPtr front = optimizeSurrogate(context, surrogateModel);
      if (verbosity >= verbosityDetailed)
      {
        context.leaveScope();

        context.resultCallback("surrogateOptimizationFront", front);
        
        ParetoFrontPtr trueFront = new ParetoFront(problem->getFitnessLimits());
        double error = 0.0;
        for (size_t i = 0; i < front->getNumSolutions(); ++i)
        {
          ObjectPtr object = front->getSolution(i);
          FitnessPtr predictedFitness = front->getFitness(i);
          FitnessPtr trueFitness = problem->evaluate(context, object);
          for (size_t j = 0; j < problem->getNumObjectives(); ++j)
            error += fabs(predictedFitness->getValue(j) - trueFitness->getValue(j));
          trueFront->insertSolution(object, trueFitness);
        }
        error /= (double)(front->getNumSolutions() * problem->getNumObjectives());
        context.resultCallback("predictionError", error);
        context.resultCallback("predictedFrontHyperVolume", front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
        context.resultCallback("trueFrontHyperVolume", trueFront->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
      }

      // retrieve candidate solutions
      objects.resize(front->getNumSolutions());
      for (size_t i = 0; i < objects.size(); ++i)
        objects[i] = front->getSolution(i);
    }
    
    // evaluate points and add them to training data
    for (size_t i = 0; i < objects.size() && !callback->shouldStop(); ++i)
    {
      ObjectPtr object = objects[i];
      FitnessPtr fitness = evaluate(context, object);
      if (verbosity >= verbosityDetailed)
        context.informationCallback(object->toShortString() + " => " + fitness->toShortString());
      addSurrogateData(context, object, fitness, surrogateData);      
    }
    return true;
  }

protected:
  ParetoFrontPtr optimizeSurrogate(ExecutionContext& context, ExpressionPtr surrogateModel)
  {
    ProblemPtr surrogateProblem = createSurrogateOptimizationProblem(surrogateModel);
    ParetoFrontPtr front = new ParetoFront(surrogateProblem->getFitnessLimits());
    surrogateSolver->solve(context, surrogateProblem, fillParetoFrontSolverCallback(front));
    return front;
  }
};

class ColoSurrogateBasedMOSolver : public SurrogateBasedMOSolver
{
public:
  ColoSurrogateBasedMOSolver(SamplerPtr initialSampler, size_t numInitialSamples, SolverPtr surrogateLearner, SolverPtr surrogateSolver, size_t numIterations)
    : SurrogateBasedMOSolver(initialSampler, numInitialSamples, surrogateLearner, surrogateSolver, numIterations) {}
  ColoSurrogateBasedMOSolver() {}

  virtual void createEncodingVariables(ExecutionContext& context, DomainPtr domain, ExpressionDomainPtr res)
  {
    ColoDomainPtr coloDomain = domain.staticCast<ColoDomain>();
    size_t numFlags = coloDomain->getNumFlags();
    for (size_t i = 0; i < numFlags; ++i)
      res->addInput(positiveIntegerClass, "flag" + string((int)i+1));
    for (size_t i = 0; i < numFlags; ++i)
      for (size_t j = 0; j < numFlags; ++j)
        res->addInput(positiveIntegerClass, "flag" + string((int)i+1) + "before" + string((int)j+1));
  }

  virtual void encodeIntoVariables(ExecutionContext& context, ObjectPtr solution, std::vector<ObjectPtr>& res)
  {
    ColoDomainPtr coloDomain = problem->getDomain().staticCast<ColoDomain>();
    size_t numFlags = coloDomain->getNumFlags();

    ColoObjectPtr coloObject = solution.staticCast<ColoObject>();
    std::vector<size_t> counts(numFlags * (numFlags + 1));
    for (size_t j = 0; j < coloObject->getLength(); ++j)
    {
      size_t flag = coloObject->getFlag(j);
      counts[flag]++;
      for (size_t i = 0; i < j; ++i)
      {
        size_t previousFlag = coloObject->getFlag(i);
        counts[numFlags + previousFlag * numFlags + flag]++;
      }
    }

    res.resize(counts.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = getPositiveInteger(counts[i]);
  }

private:
  std::vector<ObjectPtr> integers; // avoid allocating dozens of PositiveIntegers

  ObjectPtr getPositiveInteger(size_t i)
  {
    while (i >= integers.size())
    {
      ObjectPtr posInt = new PositiveInteger(integers.size());
      integers.push_back(posInt);
    }
    return integers[i];
  }
};

}; /* namespace lbcpp */

#endif // !COLO_SOLVER_SURROGATE_BASED_H_
