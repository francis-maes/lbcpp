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
  SharkObjectiveFunctionFromProblem(ExecutionContext& context, ProblemPtr problem)
    : context(context), problem(problem)
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
	  m_timesCalled++;
  }
  
  virtual bool ProposeStartingPoint(double*& point) const
  {
    DenseDoubleVectorPtr solution = problem->proposeStartingSolution(context).staticCast<DenseDoubleVector>();
    if (!solution)
      return false;
    memcpy(point, solution->getValuePointer(0), sizeof (double) * m_dimension);
    return true;
  }

protected:
  ExecutionContext& context;
  ProblemPtr problem;
};

template<class SearchAlgorithmClass>
static void sharkFillSolutions(SearchAlgorithmClass& searchAlgorithm, ProblemPtr problem, SolutionContainerPtr solutions)
{
  ContinuousDomainPtr domain = problem->getDomain().staticCast<ContinuousDomain>();
  size_t n = domain->getNumDimensions();
  std::vector<double* > sharkSolutions;
  Array<double> sharkFitnesses;
  searchAlgorithm.bestSolutions(sharkSolutions);
  searchAlgorithm.bestSolutionsFitness(sharkFitnesses);
  for (size_t i = 0; i < sharkSolutions.size(); ++i)
  {
    DenseDoubleVectorPtr sol(new DenseDoubleVector(n, 0.0));
    memcpy(sol->getValuePointer(0), sharkSolutions[i], sizeof (double) * n);
    std::vector<double> fitness(problem->getNumObjectives());
    for (size_t j = 0; j < fitness.size(); ++j)
      fitness[j] = sharkFitnesses(i, j);
    solutions->insertSolution(sol, new Fitness(fitness, problem->getFitnessLimits()));
  }
}

class NSGA2MOOptimizer : public PopulationBasedSolver
{
public:
  NSGA2MOOptimizer(size_t populationSize = 100, size_t numGenerations = 0, double mutationDistributionIndex = 20.0, double crossOverDistributionIndex = 20.0, double crossOverProbability = 0.9)
    : PopulationBasedSolver(populationSize, numGenerations), mutationDistributionIndex(mutationDistributionIndex), crossOverDistributionIndex(crossOverDistributionIndex), crossOverProbability(crossOverProbability), objective(NULL), nsga2(NULL) {}

  virtual void configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, ObjectPtr initialSolution, Verbosity verbosity)
  {
    PopulationBasedSolver::configure(context, problem, front, initialSolution, verbosity);
    objective = new SharkObjectiveFunctionFromProblem(context, problem);
    nsga2 = new NSGA2Search();
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    jassert(nsga2);
    if (iter == 0)
      nsga2->init(*objective, populationSize, mutationDistributionIndex, crossOverDistributionIndex, crossOverProbability);
    else
      nsga2->run();
    return true;
  }

  virtual void clear(ExecutionContext& context)
  {
    sharkFillSolutions(*nsga2, problem, solutions);
    deleteAndZero(nsga2);
    deleteAndZero(objective);
    PopulationBasedSolver::clear(context);
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

  virtual void configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, ObjectPtr initialSolution, Verbosity verbosity)
  {
    PopulationBasedSolver::configure(context, problem, front, initialSolution, verbosity);
    objective = new SharkObjectiveFunctionFromProblem(context, problem);
    mocma = new MOCMASearch();
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    jassert(mocma && objective);
    if (iter == 0)
      mocma->init(*objective, populationSize, numOffsprings);
    else
      mocma->run();
    return true;
  }

  virtual void clear(ExecutionContext& context)
  {
    sharkFillSolutions(*mocma, problem, solutions);
    deleteAndZero(mocma);
    deleteAndZero(objective);
    PopulationBasedSolver::clear(context);
  }

protected:
  friend class CMAESMOOptimizerClass;

  size_t numOffsprings;

  SharkObjectiveFunctionFromProblem* objective;
  MOCMASearch* mocma;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_SHARK_H_
