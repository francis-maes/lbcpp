/*-----------------------------------------.---------------------------------.
| Filename: TreeGPSamplers.h               | Tree based Genetic Programming  |
| Author  : Francis Maes                   | Implementation based on         |
| Started : 20/10/2012 22:33               |  samplers                       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_TREE_GP_SAMPLERS_H_
# define LBCPP_MCGP_TREE_GP_SAMPLERS_H_

# include <lbcpp-ml/SolutionComparator.h>
# include <lbcpp-ml/ExpressionSampler.h>
# include <lbcpp-ml/Solver.h>

namespace lbcpp
{

/*
class EmpiricalSampler : public Sampler
{
public:
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    size_t n = objects.size();
    jassert(n);
    return n ? objects[context.getRandomGenerator()->sampleSize(n)] : ObjectPtr();
  }
  
  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
    {this->objects = objects;}

  virtual void learn(ExecutionContext& context, const SolutionVectorPtr& solutions)
  {
    objects.resize(solutions->getNumSolutions());
    for (size_t i = 0; i < objects.size(); ++i)
      objects[i] = solutions->getSolution(i);
  }

protected:
  friend class EmpiricalSamplerClass;

  std::vector<ObjectPtr> objects;
};
*/

class TournamentSampler : public Sampler
{
public:
  TournamentSampler(SolutionComparatorPtr comparator, size_t tournamentSize)
    : comparator(comparator), tournamentSize(tournamentSize) {}
  TournamentSampler() : tournamentSize(0) {}

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    jassert(solutions);
    RandomGeneratorPtr random = context.getRandomGenerator();
    jassert(tournamentSize > 0);
    size_t n = solutions->getNumSolutions();
    size_t best = random->sampleSize(n);
    for (size_t i = 1; i < tournamentSize; ++i)
    {
      size_t candidate = random->sampleSize(n);
      if (comparator->compare(candidate, best) < 0)
        best = candidate;
    }
    return solutions->getSolution(best);
  }
  
  virtual void learn(ExecutionContext& context, const SolutionVectorPtr& solutions)
  {
    this->solutions = solutions;
    comparator->initialize(solutions);
  }

protected:
  friend class TournamentSamplerClass;

  SolutionComparatorPtr comparator;
  size_t tournamentSize;
  SolutionVectorPtr solutions;
};

class PerturbatorSampler : public DecoratorSampler
{
public:
  PerturbatorSampler(SamplerPtr inputSampler, PerturbatorPtr perturbator, double probability)
    : DecoratorSampler(inputSampler), perturbator(perturbator), probability(probability) {}
  PerturbatorSampler() : probability(0.0) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    DecoratorSampler::initialize(context, domain);
    perturbator->initialize(context, domain);
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    ObjectPtr res = DecoratorSampler::sample(context);
    if (context.getRandomGenerator()->sampleBool(probability))
      res = perturbator->sample(context, res);
    return res;
  }

protected:
  friend class PerturbatorSamplerClass;

  PerturbatorPtr perturbator;
  double probability;
};

class BinaryPerturbatorSampler : public DecoratorSampler
{
public:
  BinaryPerturbatorSampler(SamplerPtr inputSampler, BinaryPerturbatorPtr perturbator, double probability)
    : DecoratorSampler(inputSampler), perturbator(perturbator), probability(probability) {}
  BinaryPerturbatorSampler() : probability(0.0) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    DecoratorSampler::initialize(context, domain);
    perturbator->initialize(context, domain);
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    if (nextObject)
    {
      ObjectPtr res = nextObject;
      const_cast<BinaryPerturbatorSampler* >(this)->nextObject = ObjectPtr();
      return res;
    }
    else
    { 
      ObjectPtr input1 = DecoratorSampler::sample(context);
      ObjectPtr input2 = DecoratorSampler::sample(context);
      std::pair<ObjectPtr, ObjectPtr> pair;
      if (context.getRandomGenerator()->sampleBool(probability))
        pair = perturbator->samplePair(context, input1, input2);
      else
        pair = std::make_pair(input1, input2);
      const_cast<BinaryPerturbatorSampler* >(this)->nextObject = pair.second;
      return pair.first;
    }
  }

protected:
  friend class BinaryPerturbatorSamplerClass;

  BinaryPerturbatorPtr perturbator;
  double probability;

  ObjectPtr nextObject;
};

////////////////////////////////////////////////////////////////////////////

extern SamplerPtr tournamentSampler(SolutionComparatorPtr comparator, size_t tournamentSize);
extern DecoratorSamplerPtr perturbatorSampler(SamplerPtr inputSampler, PerturbatorPtr perturbator, double probability);
extern DecoratorSamplerPtr binaryPerturbatorSampler(SamplerPtr inputSampler, BinaryPerturbatorPtr perturbator, double probability);

class TreeGPSamplersSolver : public PopulationBasedSolver
{
public:
  TreeGPSamplersSolver(SamplerPtr initialSampler, SamplerPtr subsequentSampler, size_t populationSize = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), initialSampler(initialSampler), subsequentSampler(subsequentSampler) {}
  TreeGPSamplersSolver() {}

  static SolverPtr createDefault(size_t populationSize, size_t maxGenerations, size_t tournamentSize,
                                double crossOverProbability,
                                double shrinkProbability = 0.0,
                                double swapProbability = 0.0,
                                double insertProbability = 0.0,
                                double kozaProbability = 0.0)
  {
    SamplerPtr initialSampler = binaryMixtureSampler(fullExpressionSampler(2, 5), growExpressionSampler(2, 5), 0.5);
    SamplerPtr subsequentSampler = tournamentSampler(objectiveComparator(0), tournamentSize);

    if (crossOverProbability > 0.0)
      subsequentSampler = binaryPerturbatorSampler(subsequentSampler, subTreeCrossOverExpressionPerturbator(0.9), crossOverProbability);
    if (kozaProbability > 0.0)
      subsequentSampler = perturbatorSampler(subsequentSampler, kozaExpressionPerturbator(growExpressionSampler()), insertProbability);
    if (shrinkProbability > 0.0)
      subsequentSampler = perturbatorSampler(subsequentSampler, shrinkExpressionPerturbator(), shrinkProbability);
    if (swapProbability > 0.0)
      subsequentSampler = perturbatorSampler(subsequentSampler, swapExpressionPerturbator(0.9), swapProbability);
    if (insertProbability > 0.0)
      subsequentSampler = perturbatorSampler(subsequentSampler, insertExpressionPerturbator(), insertProbability);

    return new TreeGPSamplersSolver(initialSampler, subsequentSampler, populationSize, maxGenerations);
  }

  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution = ObjectPtr(), Verbosity verbosity = verbosityQuiet)
  {
    IterativeSolver::configure(context, problem, solutions, initialSolution, verbosity);
    initialSampler->initialize(context, problem->getDomain());
    subsequentSampler->initialize(context, problem->getDomain());
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    if (iter == 0)
    {
      population = sampleAndEvaluatePopulation(context, initialSampler, populationSize);
    }
    else
    {
      size_t n = population->getNumSolutions();
      subsequentSampler->learn(context, population);

      std::map<ObjectPtr, FitnessPtr> fitnessByObject;
      for (size_t i = 0; i < n; ++i)
        fitnessByObject[population->getSolution(i)] = population->getFitness(i);

      population = new SolutionVector(population->getFitnessLimits());
      population->reserve(n);
      for (size_t i = 0; i < n; ++i)
      {
        ObjectPtr solution = subsequentSampler->sample(context);
        std::map<ObjectPtr, FitnessPtr>::const_iterator it = fitnessByObject.find(solution);
        FitnessPtr fitness;
        if (it == fitnessByObject.end())
          fitness = evaluate(context, solution);
        else
          fitness = it->second;
        population->insertSolution(solution, fitness);
      }
    }

    if (verbosity >= verbosityDetailed)
    {
      ScalarVariableStatisticsPtr treeSize = new ScalarVariableStatistics("treeSize");
      ScalarVariableStatisticsPtr treeDepth = new ScalarVariableStatistics("treeDepth");
      for (size_t i = 0; i < population->getNumSolutions(); ++i)
      {
        ExpressionPtr expression = population->getSolution(i).staticCast<Expression>();
        treeSize->push(expression->getTreeSize());
        treeDepth->push(expression->getDepth());
      }
      context.resultCallback("treeSize", treeSize);
      context.resultCallback("treeDepth", treeDepth);
    }
    return true;
  }

  virtual void clear(ExecutionContext& context)
  {
    population = SolutionVectorPtr();
  }

protected:
  friend class TreeGPSamplersSolverClass;

  SamplerPtr initialSampler;
  SamplerPtr subsequentSampler;

  SolutionVectorPtr population;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_TREE_GP_SAMPLERS_H_
