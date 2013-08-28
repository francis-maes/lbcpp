/*-----------------------------------------.---------------------------------.
 | Filename: SharkSOOptimizers.h            | Wrapper for Single-Objective    |
 | Author  : Denny Verbeeck                 | Shark Optimizers                |
 | Started : 12/03/2013 21:03               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_SO_OPTIMIZER_SHARK_H_
# define ML_SO_OPTIMIZER_SHARK_H_

# include <ml/Solver.h>
# undef T
# include <EALib/CMA.h>
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
    ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
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
    for (size_t i = 0; i < value.size(); ++i) // turn everything into minimization
      if (problem->getObjective(i)->isMaximization())
        value[i] = -value[i];
    solver->addSolution(context, solution, fitness);
    m_timesCalled++;
  }
  
  virtual bool ProposeStartingPoint(double*& point) const
  {
    DenseDoubleVectorPtr solution = problem->getDomain()->sampleUniformly(context.getRandomGenerator()).staticCast<DenseDoubleVector>();
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

class CMAESSOOptimizer : public PopulationBasedSolver
{
public:
  CMAESSOOptimizer(size_t populationSize = 100, size_t mu = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), mu(mu), objective(NULL), cma(NULL) {}
  
  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    objective = new SharkObjectiveFunctionFromProblem(context, problem, refCountedPointerFromThis(this));
    cma = new CMASearch();
    if (populationSize == 0)
      cma->init(*objective); // cma decides mu and populationsize
    else
    {
      if (mu == 0)
        mu = populationSize / 3;
      cma->init(*objective, mu, populationSize);
    }
    populationSize = (size_t) cma->lambda();
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
  }
  
  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    jassert(cma && objective);
    if (iter > 0)
      cma->run();
    DenseDoubleVectorPtr bestSolutionObject = new DenseDoubleVector(objective->dimension(), 0.0);
    double* bestSolutionVector = cma->bestSolution();
    for (size_t i = 0; i < objective->dimension(); ++i)
      bestSolutionObject->setElement(i, new Double(bestSolutionVector[i]));
    FitnessPtr solution = evaluate(context, bestSolutionObject);
    if (verbosity >= verbosityDetailed)
      context.resultCallback("fitness", cma->bestSolutionFitness());
    return true;
  }
  
  virtual void stopSolver(ExecutionContext& context)
  {
    deleteAndZero(cma);
    deleteAndZero(objective);
    IterativeSolver::stopSolver(context);
  }
  
protected:
  friend class CMAESSOOptimizerClass;
  size_t mu;
  
  SharkObjectiveFunctionFromProblem* objective;
  CMASearch* cma;
};

}; /* namespace lbcpp */

#endif // !ML_SO_OPTIMIZER_SHARK_H_