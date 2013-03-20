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
# include <shark/ObjectiveFunctions/AbstractMultiObjectiveFunction.h>
# include <shark/Core/SearchSpaces/VectorSpace.h>
# define T JUCE_T

namespace shark
{
class SharkMOObjectiveFunctionFromProblem : public AbstractMultiObjectiveFunction< VectorSpace<double> >
{
public:
SharkMOObjectiveFunctionFromProblem(lbcpp::ExecutionContext& context, lbcpp::ProblemPtr problem, lbcpp::SolverPtr solver)
: context(context), problem(problem), solver(solver)
{
  m_name = problem->toShortString();
  lbcpp::ScalarVectorDomainPtr domain = problem->getDomain().staticCast<lbcpp::ScalarVectorDomain>();
  m_numberOfVariables = domain->getNumDimensions();
  std::vector<double> lower(domain->getNumDimensions());
  std::vector<double> upper(domain->getNumDimensions());
  for (size_t i = 0; i < lower.size(); ++i)
  {
    lower[i] = domain->getLowerLimit(i);
    upper[i] = domain->getUpperLimit(i);
  }
  //constrainthandler = new BoxConstraintHandler(lower, upper); 
}
virtual ~SharkMOObjectiveFunctionFromProblem()
  {/*delete constrainthandler;*/}
  
virtual unsigned int objectives() const
  {return (int)problem->getNumObjectives();}
  
virtual void result(double* const& point, std::vector<double>& value)
{
  lbcpp::DenseDoubleVectorPtr solution = new lbcpp::DenseDoubleVector((size_t)m_numberOfVariables, 0.0);
  memcpy(solution->getValuePointer(0), point, sizeof (double) * m_numberOfVariables);
  lbcpp::FitnessPtr fitness = problem->evaluate(context, solution);
  jassert(fitness);
  value = fitness->getValues();
  solver->addSolution(context, solution, fitness);
  m_evaluationCounter++;
}
  
virtual bool ProposeStartingPoint(double*& point) const
{
  lbcpp::DenseDoubleVectorPtr solution = problem->getInitialGuess().staticCast<lbcpp::DenseDoubleVector>();
  if (!solution)
    return false;
  memcpy(point, solution->getValuePointer(0), sizeof (double) * m_numberOfVariables);
  return true;
}

virtual size_t numberOfVariables() const
  {return m_numberOfVariables;}
  
protected:
  lbcpp::ExecutionContext& context;
  lbcpp::ProblemPtr problem;
  lbcpp::SolverPtr solver;
  size_t m_numberOfVariables;
};

}; /* namespace shark */

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
    objective = new shark::SharkMOObjectiveFunctionFromProblem(context, problem, refCountedPointerFromThis(this));
    nsga2 = new shark::detail::RealCodedNSGAII<shark::HypervolumeIndicator>();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    jassert(nsga2);
    if (iter == 0)
      nsga2->init(populationSize, mutationDistributionIndex, crossOverDistributionIndex, crossOverProbability);
    else
      nsga2->step(*objective);
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

  shark::SharkMOObjectiveFunctionFromProblem* objective;
  shark::detail::RealCodedNSGAII<shark::HypervolumeIndicator>* nsga2;
};

class CMAESMOOptimizer : public PopulationBasedSolver
{
public:
  CMAESMOOptimizer(size_t populationSize = 100, size_t numOffsprings = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), numOffsprings(numOffsprings), objective(NULL), mocma(NULL) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    PopulationBasedSolver::startSolver(context, problem, callback, startingSolution);
    objective = new shark::SharkMOObjectiveFunctionFromProblem(context, problem, refCountedPointerFromThis(this));
    mocma = new shark::detail::MOCMA<shark::HypervolumeIndicator>();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    jassert(mocma && objective);
    if (iter == 0)
      mocma->init(*objective);
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

  shark::SharkMOObjectiveFunctionFromProblem* objective;
  shark::detail::MOCMA<shark::HypervolumeIndicator>* mocma;
};

}; /* namespace lbcpp */

#endif // !ML_MO_OPTIMIZER_SHARK_H_
