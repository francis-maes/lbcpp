/*-----------------------------------------.---------------------------------.
| Filename: CrossEntropySolver.h           | Cross-Entropy Solver            |
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 10:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_CROSS_ENTROPY_H_
# define LBCPP_ML_SOLVER_CROSS_ENTROPY_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SolutionContainer.h>
# include <lbcpp-ml/SolutionComparator.h>

namespace lbcpp
{

class CrossEntropySolver : public PopulationBasedSolver
{
public:
  CrossEntropySolver(SamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, SolutionComparatorPtr comparator = SolutionComparatorPtr())
    : PopulationBasedSolver(populationSize, numGenerations), sampler(sampler), numTrainingSamples(numTrainingSamples), elitist(elitist), comparator(comparator) {}
  CrossEntropySolver() : elitist(false) {}
  
  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution, Verbosity verbosity)
  {
    IterativeSolver::configure(context, problem, solutions, initialSolution, verbosity);
    currentSampler = this->sampler;
    currentSampler->initialize(context, problem->getDomain());
    currentParents = SolutionVectorPtr();
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    if (verbosity >= verbosityDetailed)
      context.resultCallback("currentSampler", currentSampler->cloneAndCast<Sampler>());

    SolutionVectorPtr population = sampleAndEvaluatePopulation(context, currentSampler, populationSize);
    if (currentParents)
      population->insertSolutions(currentParents);
    SolutionVectorPtr selectedPopulation = select(population, numTrainingSamples);

    currentSampler = currentSampler->cloneAndCast<Sampler>();
    learnSampler(context, selectedPopulation, currentSampler);
    
    if (elitist)
      currentParents = selectedPopulation;

    return !currentSampler->isDeterministic();
  }

  virtual void clear(ExecutionContext& context)
  {
    if (verbosity >= verbosityProgressAndResult)
      context.resultCallback("sampler", currentSampler);
    Solver::clear(context);
  }

 protected:
  friend class CrossEntropySolverClass;

  SamplerPtr sampler;
  size_t numTrainingSamples;
  bool elitist;
  SolutionComparatorPtr comparator;

  SamplerPtr currentSampler;
  SolutionVectorPtr currentParents;

  SolutionComparatorPtr createDefaultComparator() const
  {
    if (problem->getNumObjectives() == 1)
      return objectiveComparator(0);  // single-objective
    else
      return paretoRankAndCrowdingDistanceComparator(); // multi-objective
  }
  
  SolutionVectorPtr select(const SolutionVectorPtr& population, size_t count) const
    {return population->selectNBests(comparator ? comparator : createDefaultComparator(), count);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLVER_CROSS_ENTROPY_H_
