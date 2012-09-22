/*-----------------------------------------.---------------------------------.
| Filename: SharkMOOptimizers.h            | Wrapper for Multi-Objective     |
| Author  : Francis Maes                   | Shark Optimizers                |
| Started : 11/09/2012 21:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_SHARK_H_
# define LBCPP_ML_OPTIMIZER_SHARK_H_

# include <lbcpp-ml/Optimizer.h>
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
    ContinuousDomainPtr domain = problem->getObjectDomain().staticCast<ContinuousDomain>();
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
  ContinuousDomainPtr domain = problem->getObjectDomain().staticCast<ContinuousDomain>();
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

class NSGA2MOOptimizer : public PopulationBasedMOOOptimizer
{
public:
  NSGA2MOOptimizer(size_t populationSize = 100, size_t numGenerations = 0, double mutationDistributionIndex = 20.0, double crossOverDistributionIndex = 20.0, double crossOverProbability = 0.9)
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), mutationDistributionIndex(mutationDistributionIndex), crossOverDistributionIndex(crossOverDistributionIndex), crossOverProbability(crossOverProbability), objective(NULL), nsga2(NULL) {}

  virtual void configure(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr front, Verbosity verbosity)
  {
    PopulationBasedMOOOptimizer::configure(context, problem, front, verbosity);
    objective = new SharkObjectiveFunctionFromMOOProblem(context, problem);
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
    sharkFillParetoFront(*nsga2, problem, front);
    deleteAndZero(nsga2);
    deleteAndZero(objective);
    PopulationBasedMOOOptimizer::clear(context);
  }

protected:
  friend class NSGA2MOOptimizerClass;

  double mutationDistributionIndex;
  double crossOverDistributionIndex;
  double crossOverProbability;

  SharkObjectiveFunctionFromMOOProblem* objective;
  NSGA2Search* nsga2;
};

class CMAESMOOptimizer : public PopulationBasedMOOOptimizer
{
public:
  CMAESMOOptimizer(size_t populationSize = 100, size_t numOffsprings = 100, size_t numGenerations = 0)
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), numOffsprings(numOffsprings), objective(NULL), mocma(NULL) {}

  virtual void configure(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr front, Verbosity verbosity)
  {
    PopulationBasedMOOOptimizer::configure(context, problem, front, verbosity);
    objective = new SharkObjectiveFunctionFromMOOProblem(context, problem);
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
    sharkFillParetoFront(*mocma, problem, front);
    deleteAndZero(mocma);
    deleteAndZero(objective);
    PopulationBasedMOOOptimizer::clear(context);
  }

protected:
  friend class CMAESMOOptimizerClass;

  size_t numOffsprings;

  SharkObjectiveFunctionFromMOOProblem* objective;
  MOCMASearch* mocma;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_SHARK_H_
