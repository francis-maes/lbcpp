/*-----------------------------------------.---------------------------------.
| Filename: CrossEntropySolver.h           | Cross-Entropy Solver            |
| Author  : Francis Maes                   |                                 |
| Started : 13/09/2012 10:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLVER_CROSS_ENTROPY_H_
# define ML_SOLVER_CROSS_ENTROPY_H_

# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/SolutionContainer.h>
# include <ml/SolutionComparator.h>

namespace lbcpp
{

class CrossEntropySolver : public PopulationBasedSolver
{
public:
  CrossEntropySolver(SamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0, bool elitist = false, SolutionComparatorPtr comparator = SolutionComparatorPtr())
    : PopulationBasedSolver(populationSize, numGenerations), sampler(sampler), numTrainingSamples(numTrainingSamples), elitist(elitist), comparator(comparator) {}
  CrossEntropySolver() : elitist(false) {}
  
  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    currentSampler = this->sampler;
    currentSampler->initialize(context, problem->getDomain());
    currentParents = SolutionVectorPtr();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    if (verbosity >= verbosityDetailed)
      context.resultCallback("currentSampler", currentSampler->cloneAndCast<Sampler>());

    SolutionVectorPtr population = sampleAndEvaluatePopulation(context, currentSampler, populationSize);
    if (currentParents)
      population->insertSolutions(currentParents);
    SolutionVectorPtr selectedPopulation = select(population, numTrainingSamples);
    if (verbosity >= verbosityDetailed && selectedPopulation->getNumSolutions())
    {
      ObjectPtr bestSolution = selectedPopulation->getSolution(0);
      context.resultCallback("bestSolution", bestSolution);
      for (size_t i = 0; i < problem->getNumObjectives(); ++i)
        context.resultCallback("bestFitness" + string((int)i), selectedPopulation->getFitness(0)->getValue(i));
      for (size_t i = 0; i < problem->getNumValidationObjectives(); ++i)
        context.resultCallback("bestSolutionValidation" + string((int)i), problem->getValidationObjective(i)->evaluate(context, bestSolution));

      if (problem->getNumObjectives() > 1)
      {
        ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
        for (size_t i = 0; i < population->getNumSolutions(); ++i)
          front->insertSolution(population->getSolution(i), population->getFitness(i));
        context.resultCallback("hyperVolume", front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
      }
    }

    currentSampler = currentSampler->cloneAndCast<Sampler>();
    learnSampler(context, selectedPopulation, currentSampler);
    
    if (elitist)
      currentParents = selectedPopulation;

    return !currentSampler->isDeterministic();
  }

  virtual void stopSolver(ExecutionContext& context)
  {
    if (verbosity >= verbosityProgressAndResult)
      context.resultCallback("sampler", currentSampler);
    Solver::stopSolver(context);
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

#endif // !ML_SOLVER_CROSS_ENTROPY_H_
