/*-----------------------------------------.---------------------------------.
| Filename: SharkMOOptimizers.h            | Wrapper for Multi-Objective     |
| Author  : Francis Maes                   | Shark Optimizers                |
| Started : 11/09/2012 21:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_OPTIMIZER_SHARK_H_
# define LBCPP_MOO_OPTIMIZER_SHARK_H_

# include "MOOCore.h"
# undef T
# include <MOO-EALib/NSGA2.h>
# include <MOO-EALib/MO-CMA.h>
# define T JUCE_T

namespace lbcpp
{

class SharkObjectiveFunctionFromMOOProblem : public ObjectiveFunctionVS<double> 
{
public:
  SharkObjectiveFunctionFromMOOProblem(ExecutionContext& context, MOOProblemPtr problem)
    : context(context), problem(problem)
  {
  	m_name = (const char* )problem->toShortString();
    ContinuousMOODomainPtr domain = problem->getObjectDomain().staticCast<ContinuousMOODomain>();
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
  virtual ~SharkObjectiveFunctionFromMOOProblem()
    {delete constrainthandler;}

  virtual unsigned int objectives() const
    {return (int)problem->getNumObjectives();}

  virtual void result(double* const& point, std::vector<double>& value)
  {
    DenseDoubleVectorPtr solution = new DenseDoubleVector((size_t)m_dimension, 0.0);
    memcpy(solution->getValuePointer(0), point, sizeof (double) * m_dimension);
    MOOFitnessPtr fitness = problem->evaluate(context, solution);
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
  MOOProblemPtr problem;
};

template<class SearchAlgorithmClass>
static void sharkFillParetoFront(SearchAlgorithmClass& searchAlgorithm, MOOProblemPtr problem, MOOParetoFrontPtr front)
{
  ContinuousMOODomainPtr domain = problem->getObjectDomain().staticCast<ContinuousMOODomain>();
  size_t n = domain->getNumDimensions();
  std::vector<double* > solutions;
  Array<double> fitnesses;
  searchAlgorithm.bestSolutions(solutions);
  searchAlgorithm.bestSolutionsFitness(fitnesses);
  for (size_t i = 0; i < solutions.size(); ++i)
  {
    DenseDoubleVectorPtr sol(new DenseDoubleVector(n, 0.0));
    memcpy(sol->getValuePointer(0), solutions[i], sizeof (double) * n);
    std::vector<double> fitness(problem->getNumObjectives());
    for (size_t j = 0; j < fitness.size(); ++j)
      fitness[j] = fitnesses(i, j);
    front->addSolutionAndUpdateFront(sol, new MOOFitness(fitness, problem->getFitnessLimits()));
  }
}

template<class SearchAlgorithmClass>
static void sharkRunAndFillParetoFront(ExecutionContext& context, SearchAlgorithmClass& searchAlgorithm, MOOProblemPtr problem, MOOParetoFrontPtr front, size_t numGenerations, MOOOptimizer::Verbosity verbosity)
{
  for (size_t i = 1; (numGenerations == 0 || i < numGenerations) && !problem->shouldStop(); ++i) // first generation is evaluated during the init()
  {
    searchAlgorithm.run();
    if (verbosity >= MOOOptimizer::verbosityProgressAndResult)
      context.progressCallback(new ProgressionState(i+1, numGenerations, "Generations"));
  }
  sharkFillParetoFront(searchAlgorithm, problem, front);
}

class NSGA2MOOptimizer : public PopulationBasedMOOOptimizer
{
public:
  NSGA2MOOptimizer(size_t populationSize = 100, size_t numGenerations = 0, double mutationDistributionIndex = 20.0, double crossOverDistributionIndex = 20.0, double crossOverProbability = 0.9)
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), mutationDistributionIndex(mutationDistributionIndex), crossOverDistributionIndex(crossOverDistributionIndex), crossOverProbability(crossOverProbability) {}

  virtual void optimize(ExecutionContext& context)
  {
    SharkObjectiveFunctionFromMOOProblem sharkObjective(context, problem);
    NSGA2Search nsga2;
    nsga2.init(sharkObjective, populationSize, mutationDistributionIndex, crossOverDistributionIndex, crossOverProbability);
    sharkRunAndFillParetoFront(context, nsga2, problem, front, numGenerations, verbosity);
  }

protected:
  friend class NSGA2MOOptimizerClass;

  double mutationDistributionIndex;
  double crossOverDistributionIndex;
  double crossOverProbability;
};

class CMAESMOOptimizer : public PopulationBasedMOOOptimizer
{
public:
  CMAESMOOptimizer(size_t populationSize = 100, size_t numOffsprings = 100, size_t numGenerations = 0)
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), numOffsprings(numOffsprings) {}
  
  virtual void optimize(ExecutionContext& context)
  {
    SharkObjectiveFunctionFromMOOProblem sharkObjective(context, problem);
    MOCMASearch mocma;
    mocma.init(sharkObjective, populationSize, numOffsprings);
    sharkRunAndFillParetoFront(context, mocma, problem, front, numGenerations, verbosity);
  }

protected:
  friend class CMAESMOOptimizerClass;

  size_t numOffsprings;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_OPTIMIZER_SHARK_H_
