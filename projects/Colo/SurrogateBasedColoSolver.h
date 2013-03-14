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
# include <ml/VariableEncoder.h>
# include "ColoProblem.h"
# include "../../lib/ml/Solver/SurrogateBasedSolver.h"

namespace lbcpp
{

class SurrogateBasedMOSolver : public BatchSurrogateBasedSolver
{
public:
  SurrogateBasedMOSolver(SamplerPtr initialSampler, size_t numInitialSamples, SolverPtr surrogateLearner, SolverPtr surrogateSolver, VariableEncoderPtr variableEncoder, SelectionCriterionPtr selectionCriterion, size_t numIterations)
    : BatchSurrogateBasedSolver(samplerToVectorSampler(initialSampler, numInitialSamples), surrogateLearner, surrogateSolver, variableEncoder, selectionCriterion, numIterations) {}
  SurrogateBasedMOSolver() {}

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    std::vector<ObjectPtr> objects;
    if (iter == 0)
    {
      // make random samples
      OVectorPtr samples = initialVectorSampler->sample(context).dynamicCast<OVector>();
      jassert(samples);
      objects.resize(samples->getNumElements());
      for (size_t i = 0; i < samples->getNumElements(); ++i)
        objects[i] = samples->getElement(i);
    }
    else
    {
      // learn surrogate
      if (verbosity >= verbosityDetailed)
        context.enterScope("Learn surrogate");
      ExpressionPtr surrogateModel = getSurrogateModel(context);
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
      addFitnessSample(context, object, fitness);      
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

}; /* namespace lbcpp */

#endif // !COLO_SOLVER_SURROGATE_BASED_H_
