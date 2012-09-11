/*-----------------------------------------.---------------------------------.
| Filename: MOOSharkOptimizer.h            | Wrapper for MOO Shark Optimizers|
| Author  : Francis Maes                   |                                 |
| Started : 11/09/2012 21:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SHARK_OPTIMIZER_H_
# define LBCPP_MOO_SHARK_OPTIMIZER_H_

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
    ContinuousMOODomainPtr domain = problem->getSolutionDomain().staticCast<ContinuousMOODomain>();
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

class SharkWrapperMOOOptimizer : public MOOOptimizer
{
protected:
  template<class SearchAlgorithmClass>
  void fillParetoFront(SearchAlgorithmClass& searchAlgorithm, MOOProblemPtr problem, MOOParetoFrontPtr front)
  {
    ContinuousMOODomainPtr domain = problem->getSolutionDomain().staticCast<ContinuousMOODomain>();
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
      front->insert(sol, new MOOFitness(fitness, problem->getFitnessLimits()));
    }
  }

  template<class SearchAlgorithmClass>
  void runAndFillParetoFront(ExecutionContext& context, SearchAlgorithmClass& searchAlgorithm, MOOProblemPtr problem, MOOParetoFrontPtr front, size_t numGenerations)
  {
    for (size_t i = 0; i < numGenerations; ++i)
    {
      searchAlgorithm.run();
      context.progressCallback(new ProgressionState(i+1, numGenerations, "Generations"));
    }
    fillParetoFront(searchAlgorithm, problem, front);
  }
};

class NSGA2MOOOptimizer : public SharkWrapperMOOOptimizer
{
public:
  NSGA2MOOOptimizer(size_t populationSize = 100, size_t numGenerations = 250, double nm = 20.0, double nc = 20.0, double pc = 0.9)
    : populationSize(populationSize), numGenerations(numGenerations), nm(nm), nc(nc), pc(pc) {}

  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoFront)
  {
    SharkObjectiveFunctionFromMOOProblem sharkObjective(context, problem);
    NSGA2Search nsga2;
    nsga2.init(sharkObjective, populationSize, nm, nc, pc);
    runAndFillParetoFront(context, nsga2, problem, paretoFront, numGenerations);
  }

protected:
  friend class NSGA2MOOOptimizerClass;

  size_t populationSize;
  size_t numGenerations;
  double nm, nc, pc; // todo: replace by understandable names
};

class CMAESMOOOptimizer : public SharkWrapperMOOOptimizer
{
public:
  CMAESMOOOptimizer(size_t numParents = 100, size_t numOffsprings = 100, size_t numGenerations = 100)
    : numParents(numParents), numOffsprings(numOffsprings), numGenerations(numGenerations) {}
  
  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoFront)
  {
    SharkObjectiveFunctionFromMOOProblem sharkObjective(context, problem);
    MOCMASearch mocma;
    mocma.init(sharkObjective, numParents, numOffsprings);
    runAndFillParetoFront(context, mocma, problem, paretoFront, numGenerations);
  }

protected:
  friend class CMAESMOOOptimizerClass;

  size_t numParents;
  size_t numOffsprings;
  size_t numGenerations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SHARK_OPTIMIZER_H_
