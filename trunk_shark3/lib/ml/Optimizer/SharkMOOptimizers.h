/*-----------------------------------------.---------------------------------.
| Filename: SharkMOOptimizers.h            | Wrapper for Multi-Objective     |
| Author  : Francis Maes                   | Shark Optimizers                |
| Started : 11/09/2012 21:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_MO_OPTIMIZER_SHARK_H_
# define ML_MO_OPTIMIZER_SHARK_H_

# include <ml/Solver.h>
# include "SharkSOOptimizers.h"
# undef T
# include <shark/Algorithms/DirectSearch/RealCodedNSGAII.h>
# include <shark/Algorithms/DirectSearch/MOCMA.h>
# define T JUCE_T

namespace lbcpp
{

class NSGA2MOOptimizer : public PopulationBasedSolver
{
public:
  NSGA2MOOptimizer(size_t populationSize = 100, size_t numGenerations = 0, double mutationDistributionIndex = 20.0, double crossOverDistributionIndex = 20.0, double crossOverProbability = 0.9)
    : PopulationBasedSolver(populationSize, numGenerations), mutationDistributionIndex(mutationDistributionIndex), crossOverDistributionIndex(crossOverDistributionIndex), crossOverProbability(crossOverProbability), objective(NULL), nsga2(NULL) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    PopulationBasedSolver::startSolver(context, problem, callback, startingSolution);
    objective = new SharkObjectiveFunctionFromProblem(context, problem, refCountedPointerFromThis(this));
    nsga2 = new shark::RealCodedNSGAII();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    jassert(nsga2);
    if (iter == 0)
      nsga2->init(*objective, populationSize, mutationDistributionIndex, crossOverDistributionIndex, crossOverProbability);
    else
      nsga2->step();
    return true;
  }

  virtual void stopSolver(ExecutionContext& context)
  {
    deleteAndZero(nsga2);
    deleteAndZero(objective);
    PopulationBasedSolver::stopSolver(context);
  }

protected:
  friend class NSGA2MOOptimizerClass;

  double mutationDistributionIndex;
  double crossOverDistributionIndex;
  double crossOverProbability;

  SharkObjectiveFunctionFromProblem* objective;
  shark::detail::RealCodedNSGAII* nsga2;
};

class CMAESMOOptimizer : public PopulationBasedSolver
{
public:
  CMAESMOOptimizer(size_t populationSize = 100, size_t numOffsprings = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), numOffsprings(numOffsprings), objective(NULL), mocma(NULL) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    PopulationBasedSolver::startSolver(context, problem, callback, startingSolution);
    objective = new SharkObjectiveFunctionFromProblem(context, problem, refCountedPointerFromThis(this));
    mocma = new shark::MOCMA();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    jassert(mocma && objective);
    if (iter == 0)
      mocma->init(*objective, populationSize, numOffsprings);
    else
      mocma->step(*objective);
    return true;
  }

  virtual void stopSolver(ExecutionContext& context)
  {
    deleteAndZero(mocma);
    deleteAndZero(objective);
    PopulationBasedSolver::stopSolver(context);
  }

protected:
  friend class CMAESMOOptimizerClass;

  size_t numOffsprings;

  SharkObjectiveFunctionFromProblem* objective;
  shark::detail::MOCMA* mocma;
};

}; /* namespace lbcpp */

#endif // !ML_MO_OPTIMIZER_SHARK_H_
