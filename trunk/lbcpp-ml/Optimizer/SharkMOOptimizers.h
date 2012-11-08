/*-----------------------------------------.---------------------------------.
| Filename: SharkMOOptimizers.h            | Wrapper for Multi-Objective     |
| Author  : Francis Maes                   | Shark Optimizers                |
| Started : 11/09/2012 21:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_SHARK_H_
# define LBCPP_ML_OPTIMIZER_SHARK_H_

# include <lbcpp-ml/Solver.h>
# undef T
# include <MOO-EALib/NSGA2.h>
# include <MOO-EALib/MO-CMA.h>
# define T JUCE_T

namespace lbcpp
{

class SharkObjectiveFunctionFromProblem : public ObjectiveFunctionVS<double> 
{
public:
  SharkObjectiveFunctionFromProblem(ExecutionContext& context, ProblemPtr problem, SolverPtr solver)
    : context(context), problem(problem), solver(solver)
  {
  	m_name = (const char* )problem->toShortString();
    ContinuousDomainPtr domain = problem->getDomain().staticCast<ContinuousDomain>();
    m_dimension = domain->getNumDimensions();
    std::vector<double> lower(domain->getNumDimensions());
    std::vector<double> upper(domain->getNumDimensions());
    for (size_t i = 0; i < lower.size(); ++i)
    {
      lower[i] = domain->getLowerLimit(i);
      upper[i] = domain->getUpperLimit(i);
    }
    constrainthandler = new BoxConstraintHandler(lower, upper); 
  }
  virtual ~SharkObjectiveFunctionFromProblem()
    {delete constrainthandler;}

  virtual unsigned int objectives() const
    {return (int)problem->getNumObjectives();}

  virtual void result(double* const& point, std::vector<double>& value)
  {
    DenseDoubleVectorPtr solution = new DenseDoubleVector((size_t)m_dimension, 0.0);
    memcpy(solution->getValuePointer(0), point, sizeof (double) * m_dimension);
    FitnessPtr fitness = problem->evaluate(context, solution);
    jassert(fitness);
    value = fitness->getValues();
    solver->getCallback()->solutionEvaluated(context, solver, solution, fitness);
	  m_timesCalled++;
  }
  
  virtual bool ProposeStartingPoint(double*& point) const
  {
    DenseDoubleVectorPtr solution = problem->getInitialGuess().staticCast<DenseDoubleVector>();
    if (!solution)
      return false;
    memcpy(point, solution->getValuePointer(0), sizeof (double) * m_dimension);
    return true;
  }

protected:
  ExecutionContext& context;
  ProblemPtr problem;
  SolverPtr solver;
};

class NSGA2MOOptimizer : public PopulationBasedSolver
{
public:
  NSGA2MOOptimizer(size_t populationSize = 100, size_t numGenerations = 0, double mutationDistributionIndex = 20.0, double crossOverDistributionIndex = 20.0, double crossOverProbability = 0.9)
    : PopulationBasedSolver(populationSize, numGenerations), mutationDistributionIndex(mutationDistributionIndex), crossOverDistributionIndex(crossOverDistributionIndex), crossOverProbability(crossOverProbability), objective(NULL), nsga2(NULL) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    PopulationBasedSolver::startSolver(context, problem, callback, startingSolution);
    objective = new SharkObjectiveFunctionFromProblem(context, problem, refCountedPointerFromThis(this));
    nsga2 = new NSGA2Search();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    jassert(nsga2);
    if (iter == 0)
      nsga2->init(*objective, populationSize, mutationDistributionIndex, crossOverDistributionIndex, crossOverProbability);
    else
      nsga2->run();
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
  NSGA2Search* nsga2;
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
    mocma = new MOCMASearch();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    jassert(mocma && objective);
    if (iter == 0)
      mocma->init(*objective, populationSize, numOffsprings);
    else
      mocma->run();
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
  MOCMASearch* mocma;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_SHARK_H_
