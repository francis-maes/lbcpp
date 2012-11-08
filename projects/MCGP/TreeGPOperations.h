/*-----------------------------------------.---------------------------------.
| Filename: TreeGPOperations.h             | Tree based Genetic Programming  |
| Author  : Francis Maes                   | Implementation based on         |
| Started : 20/10/2012 11:06               |  solution-set operations        |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_TREE_GP_OPERATIONS_H_
# define LBCPP_MCGP_TREE_GP_OPERATIONS_H_

# include <lbcpp-ml/SolutionComparator.h>
# include <lbcpp-ml/ExpressionSampler.h>
# include <lbcpp-ml/Solver.h>

namespace lbcpp
{

/////////////// OPERATORS //////////////////////

class SolutionsOperator : public Object
{
public:
  typedef SolutionContainer::SolutionAndFitness SolutionAndFitness;

  // FIXME: maybe, the problem should directly be attached to the solution container 
  virtual SolutionContainerPtr compute(ExecutionContext& context, const ProblemPtr& problem, const SolutionContainerPtr& solutions) = 0;
};

typedef ReferenceCountedObjectPtr<SolutionsOperator> SolutionsOperatorPtr;

class CompositeSolutionsOperator : public SolutionsOperator
{
public:
  CompositeSolutionsOperator(SolutionsOperatorPtr operator1, SolutionsOperatorPtr operator2, SolutionsOperatorPtr operator3, SolutionsOperatorPtr operator4, SolutionsOperatorPtr operator5)
  {
    if (operator1) operators.push_back(operator1);
    if (operator2) operators.push_back(operator2);
    if (operator3) operators.push_back(operator3);
    if (operator4) operators.push_back(operator4);
    if (operator5) operators.push_back(operator5);
  }
  CompositeSolutionsOperator(const std::vector<SolutionsOperatorPtr>& operators)
    : operators(operators) {}
  CompositeSolutionsOperator() {}

  virtual SolutionContainerPtr compute(ExecutionContext& context, const ProblemPtr& problem, const SolutionContainerPtr& solutions)
  {
    SolutionContainerPtr res = solutions;
    for (size_t i = 0; i < operators.size(); ++i)
      res = operators[i]->compute(context, problem, res);
    return res;
  }

protected:
  friend class CompositeSolutionsOperatorClass;
  std::vector<SolutionsOperatorPtr> operators;
};

class SolutionSelector : public SolutionsOperator
{
public:
  virtual SolutionAndFitness select(ExecutionContext& context, const SolutionContainerPtr& solutions) = 0;

  virtual SolutionContainerPtr compute(ExecutionContext& context, const ProblemPtr& problem, const SolutionContainerPtr& solutions)
  {
    size_t n = solutions->getNumSolutions();
    SolutionVectorPtr res = new SolutionVector(solutions->getFitnessLimits());
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
      res->insertSolution(select(context, solutions));
    return res;
  }
};

typedef ReferenceCountedObjectPtr<SolutionSelector> SolutionSelectorPtr;

class TournamentSolutionSelector : public SolutionSelector
{
public:
  TournamentSolutionSelector(SolutionComparatorPtr comparator, size_t tournamentSize)
    : comparator(comparator), tournamentSize(tournamentSize) {}
  TournamentSolutionSelector() : tournamentSize(0) {}

  virtual SolutionContainerPtr compute(ExecutionContext& context, const ProblemPtr& problem, const SolutionContainerPtr& solutions)
  {
    comparator->initialize(solutions);
    return SolutionSelector::compute(context, problem, solutions);
  }

  virtual SolutionAndFitness select(ExecutionContext& context, const SolutionContainerPtr& solutions)
  {
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
    return solutions->getSolutionAndFitness(best);
  }

protected:
  friend class TournamentSolutionSelectorClass;

  SolutionComparatorPtr comparator;
  size_t tournamentSize;
};

class NBestsSolutionSelector : public SolutionsOperator
{
public:
  NBestsSolutionSelector(SolutionComparatorPtr comparator, size_t numBests)
    : comparator(comparator), numBests(numBests) {}
  NBestsSolutionSelector() : numBests(0) {}
  
  virtual SolutionContainerPtr compute(ExecutionContext& context, const ProblemPtr& problem, const SolutionContainerPtr& solutions)
  {
    SolutionVectorPtr res = solutions.staticCast<SolutionVector>()->selectNBests(comparator, numBests);
    res->duplicateSolutionsUntilReachingSize(solutions->getNumSolutions());
    return res;
  }

protected:
  friend class NBestsSolutionSelectorClass;

  SolutionComparatorPtr comparator;
  size_t numBests;
};

class MutationOperator : public SolutionsOperator
{
public:
  MutationOperator(PerturbatorPtr perturbator, double probability)
    : perturbator(perturbator), probability(probability) {}
  MutationOperator() : probability(0.0) {}

  virtual SolutionContainerPtr compute(ExecutionContext& context, const ProblemPtr& problem, const SolutionContainerPtr& solutions)
  {
    perturbator->initialize(context, problem->getDomain());

    RandomGeneratorPtr random = context.getRandomGenerator();
    size_t n = solutions->getNumSolutions();
    SolutionVectorPtr res = new SolutionVector(solutions->getFitnessLimits());
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      SolutionAndFitness solutionAndFitness = solutions->getSolutionAndFitness(i);
      if (random->sampleBool(probability))
        res->insertSolution(perturbator->sample(context, solutionAndFitness.first), FitnessPtr());
      else
        res->insertSolution(solutionAndFitness);
    }
    return res;
  }

protected:
  friend class MutationOperatorClass;

  PerturbatorPtr perturbator;
  double probability;
};

class CrossOverOperator : public SolutionsOperator
{
public:
  CrossOverOperator(BinaryPerturbatorPtr perturbator, double probability)
    : perturbator(perturbator), probability(probability) {}
  CrossOverOperator() : probability(0.0) {}

  virtual SolutionContainerPtr compute(ExecutionContext& context, const ProblemPtr& problem, const SolutionContainerPtr& solutions)
  {
    // select and shuffle parents
    RandomGeneratorPtr random = context.getRandomGenerator();
    size_t n = solutions->getNumSolutions();
    std::vector<size_t> selected;
    selected.reserve((size_t)(probability * 1.2 * n));
    for (size_t i = 0; i < n; ++i)
      if (random->sampleBool(probability))
        selected.push_back(i);
    random->shuffle(selected);
    if (selected.size() % 2 == 1)
      selected.pop_back();
    if (selected.empty())
      return solutions;

    // copy population
    SolutionVectorPtr res = new SolutionVector(solutions->getFitnessLimits());
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
      res->insertSolution(solutions->getSolutionAndFitness(i));

    // perform cross-overs
    for (size_t i = 0; i < selected.size(); i += 2)
    {
      ObjectPtr object1 = res->getSolution(selected[i]); 
      ObjectPtr object2 = res->getSolution(selected[i+1]);
      std::pair<ObjectPtr, ObjectPtr> newSolutions = perturbator->samplePair(context, object1, object2);
      res->setSolution(selected[i], newSolutions.first);
      res->setSolution(selected[i+1], newSolutions.second);
    }
    return res;
  }

protected:
  friend class CrossOverOperatorClass;

  BinaryPerturbatorPtr perturbator;
  double probability;
};

/////////////// TOP-LEVEL SOLVER ////////////////////////

extern SolutionSelectorPtr tournamentSolutionSelector(SolutionComparatorPtr comparator, size_t tournamentSize);
extern SolutionsOperatorPtr nBestsSolutionSelector(SolutionComparatorPtr comparator, size_t numBests);
extern SolutionsOperatorPtr mutationOperator(PerturbatorPtr perturbator, double probability);
extern SolutionsOperatorPtr crossOverOperator(BinaryPerturbatorPtr perturbator, double probability);
extern SolutionsOperatorPtr compositeSolutionsOperator(const std::vector<SolutionsOperatorPtr>& operators);
extern SolutionsOperatorPtr compositeSolutionsOperator(SolutionsOperatorPtr operator1, SolutionsOperatorPtr operator2, SolutionsOperatorPtr operator3 = SolutionsOperatorPtr(), SolutionsOperatorPtr operator4 = SolutionsOperatorPtr(), SolutionsOperatorPtr operator5 = SolutionsOperatorPtr());
  
class TreeGPOperationsSolver : public PopulationBasedSolver
{
public:
  TreeGPOperationsSolver(SamplerPtr initialSampler, SolutionsOperatorPtr solutionOperator, size_t populationSize = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), initialSampler(initialSampler), solutionOperator(solutionOperator) {}
  TreeGPOperationsSolver() {}

  static SolverPtr createDefault(size_t populationSize, size_t maxGenerations, size_t tournamentSize,
                                double crossOverProbability,
                                double shrinkProbability = 0.0,
                                double swapProbability = 0.0,
                                double insertProbability = 0.0,
                                double kozaProbability = 0.0)
  {
    SamplerPtr initialSampler = binaryMixtureSampler(fullExpressionSampler(2, 5), growExpressionSampler(2, 5), 0.5);
    std::vector<SolutionsOperatorPtr> operators;
    //operators.push_back(nBestsSolutionSelector(objectiveComparator(0), populationSize / 4));
    operators.push_back(tournamentSolutionSelector(objectiveComparator(0), tournamentSize));
    
    if (crossOverProbability > 0.0)
      operators.push_back(crossOverOperator(subTreeCrossOverExpressionPerturbator(0.9), crossOverProbability));
    if (kozaProbability > 0.0)
      operators.push_back(mutationOperator(kozaExpressionPerturbator(growExpressionSampler()), insertProbability));
    if (shrinkProbability > 0.0)
      operators.push_back(mutationOperator(shrinkExpressionPerturbator(), shrinkProbability));
    if (swapProbability > 0.0)
      operators.push_back(mutationOperator(swapExpressionPerturbator(0.9), swapProbability));
    if (insertProbability > 0.0)
      operators.push_back(mutationOperator(insertExpressionPerturbator(), insertProbability));

    return new TreeGPOperationsSolver(initialSampler, compositeSolutionsOperator(operators), populationSize, maxGenerations);
  }

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    initialSampler->initialize(context, problem->getDomain());
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    if (iter == 0)
    {
      population = sampleAndEvaluatePopulation(context, initialSampler, populationSize);
    }
    else
    {
      population = solutionOperator->compute(context, problem, population);
      computeMissingFitnesses(context, population);
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

  virtual void stopSolver(ExecutionContext& context)
  {
    population = SolutionVectorPtr();
  }

protected:
  friend class TreeGPOperationsSolverClass;

  SamplerPtr initialSampler;
  SolutionsOperatorPtr solutionOperator;

  SolutionVectorPtr population;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_TREE_BASED_GENETIC_PROGRAMMING_H_
