/*-----------------------------------------.---------------------------------.
| Filename: TreeBasedGeneticProgramming.h  | Tree based Genetic Programming  |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 11:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_TREE_BASED_GENETIC_PROGRAMMING_H_
# define LBCPP_MCGP_TREE_BASED_GENETIC_PROGRAMMING_H_

# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SolutionComparator.h>
# include <lbcpp-ml/ExpressionSampler.h>
# include <lbcpp-ml/Expression.h>
# include <lbcpp-ml/Solver.h>

namespace lbcpp
{

/////////////// PERTURBATORS ////////////////////////

class Perturbator : public Object
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain) = 0;

  virtual ObjectPtr sample(ExecutionContext& context, const ObjectPtr& object) const = 0;
};

typedef ReferenceCountedObjectPtr<Perturbator> PerturbatorPtr;

class BinaryPerturbator : public Perturbator
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain) {}

  virtual ObjectPtr sample(ExecutionContext& context, const ObjectPtr& object) const
  {
    PairPtr pair = object.staticCast<Pair>();
    std::pair<ObjectPtr, ObjectPtr> res = samplePair(context, pair->getFirst().getObject(), pair->getSecond().getObject());
    return new Pair(res.first, res.second);
  }

  virtual std::pair<ObjectPtr, ObjectPtr> samplePair(ExecutionContext& context, const ObjectPtr& object1, const ObjectPtr& object2) const = 0;
};

typedef ReferenceCountedObjectPtr<BinaryPerturbator> BinaryPerturbatorPtr;

class ExpressionPerturbator : public Perturbator
{
public:
  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const = 0;

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    this->domain = domain.staticCast<ExpressionDomain>();
  }

  virtual ObjectPtr sample(ExecutionContext& context, const ObjectPtr& object) const
  {
    const ExpressionPtr& expression = object.staticCast<Expression>();
    if (!expression->getNumSubNodes())
      return expression; // do not apply mutations on leaf nodes
    for (size_t attempt = 0; attempt < 2; ++attempt)
    {
      ExpressionPtr res = sampleExpression(context, expression);
      if (res)
        return res;
    }
    return expression;
  }

protected:
  ExpressionDomainPtr domain;
};

typedef ReferenceCountedObjectPtr<ExpressionPerturbator> ExpressionPerturbatorPtr;

// replaces a randomly chosen internal node by one of its child nodes (also randomly chosen)
class ShrinkExpressionPerturbator : public ExpressionPerturbator
{
public:
  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const
  {
    RandomGeneratorPtr random  = context.getRandomGenerator();
    ExpressionPtr selected = expression->sampleNode(random, 1.0);
    ExpressionPtr res = expression->cloneAndSubstitute(selected, selected->sampleSubNode(random));
    jassert(res->getTreeSize() < expression->getTreeSize());
    return res;
  }
};

// swaps a randomly chosen node symbol by another symbol of same arity
class SwapExpressionPerturbator : public ExpressionPerturbator
{
public:
  SwapExpressionPerturbator(double functionSelectionProbability = 0.0)
    : functionSelectionProbability(functionSelectionProbability) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionPerturbator::initialize(context, domain);
    terminals = this->domain->getTerminals();
    size_t maxArity = this->domain->getMaxFunctionArity();
    functionsByArity.resize(maxArity + 1);
    for (size_t i = 0; i < this->domain->getNumFunctions(); ++i)
    {
      FunctionPtr function = this->domain->getFunction(i);
      functionsByArity[function->getNumInputs()].push_back(function);
    }
  }

  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const
  {
    RandomGeneratorPtr random  = context.getRandomGenerator();
    ExpressionPtr node = expression->sampleNode(random, functionSelectionProbability);
    FunctionExpressionPtr functionNode = node.dynamicCast<FunctionExpression>();
    ExpressionPtr newNode;
    if (functionNode)
    {
      const std::vector<FunctionPtr>& functions = functionsByArity[functionNode->getNumSubNodes()];
      FunctionPtr function = functions[random->sampleSize(functions.size())];
      newNode = new FunctionExpression(function, functionNode->getArguments());
    }
    else
      newNode = terminals[random->sampleSize(terminals.size())];
    ExpressionPtr res = expression->cloneAndSubstitute(node, newNode);
    jassert(res->getTreeSize() == expression->getTreeSize());
    return res;
  }

protected:
  friend class SwapExpressionPerturbatorClass;

  double functionSelectionProbability;

  std::vector<ExpressionPtr> terminals;
  std::vector< std::vector<FunctionPtr> > functionsByArity;
};


// This operator mutate a GP tree by inserting a new branch at a random position in a tree,
// using the original subtree at this position as one argument, and if necessary randomly
// selecting terminal primitives to complete the arguments of the inserted node.

class InsertExpressionPerturbator : public ExpressionPerturbator
{
public:
  InsertExpressionPerturbator(size_t maxDepth = 17)
    : maxDepth(maxDepth) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionPerturbator::initialize(context, domain);
    terminals = this->domain->getTerminals();
  }
  
  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    ExpressionPtr node = expression->sampleNode(random);
    FunctionPtr function = domain->getFunction(random->sampleSize(domain->getNumFunctions()));
    std::vector<ExpressionPtr> arguments(function->getNumInputs());
    arguments[random->sampleSize(arguments.size())] = node;
    for (size_t i = 0; i < arguments.size(); ++i)
      if (!arguments[i])
        arguments[i] = terminals[random->sampleSize(terminals.size())];
    ExpressionPtr res = expression->cloneAndSubstitute(node, new FunctionExpression(function, arguments));
    jassert(res->getTreeSize() > expression->getTreeSize());
    return res->getDepth() <= maxDepth ? res : ExpressionPtr();
  }

protected:
  friend class InsertExpressionPerturbatorClass;

  size_t maxDepth;

  std::vector<ExpressionPtr> terminals;
};

// replaces a randomly chosen node by a subtree generated by a given sampler
class StandardExpressionPerturbator : public ExpressionPerturbator
{
public:
  StandardExpressionPerturbator(DepthControlledExpressionSamplerPtr sampler, size_t maxRegenerationDepth, size_t maxDepth)
    : sampler(sampler), maxRegenerationDepth(maxRegenerationDepth), maxDepth(maxDepth) {}
  StandardExpressionPerturbator() : maxRegenerationDepth(0), maxDepth(0) {}
    
  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    ExpressionPtr selectedNode = expression->sampleNode(random);
    size_t selectedNodeDepth = expression->getNodeDepth(selectedNode);
    if (selectedNodeDepth > maxDepth)
      return ExpressionPtr();
    size_t maxMutationDepth = maxDepth - selectedNodeDepth + 1;
    if (maxMutationDepth > maxRegenerationDepth)
      maxMutationDepth = maxRegenerationDepth;
    jassert(maxMutationDepth >= 1);
    ExpressionPtr newNode = sampler->sampleTree(random, 1, maxMutationDepth);
    return expression->cloneAndSubstitute(selectedNode, newNode);
  }

protected:
  friend class StandardExpressionPerturbatorClass;

  DepthControlledExpressionSamplerPtr sampler;
  size_t maxRegenerationDepth;
  size_t maxDepth;
};

class SubTreeBinaryPerturbator : public BinaryPerturbator
{
public:
  SubTreeBinaryPerturbator(double functionSelectionProbability = 0.0, size_t maxDepth = 17)
    : functionSelectionProbability(functionSelectionProbability), maxDepth(maxDepth) {}

  virtual std::pair<ObjectPtr, ObjectPtr> samplePair(ExecutionContext& context, const ObjectPtr& object1, const ObjectPtr& object2) const
  {
    for (size_t attempt = 0; attempt < 2; ++attempt)
    {
      const ExpressionPtr& expression1 = object1.staticCast<Expression>();
      const ExpressionPtr& expression2 = object2.staticCast<Expression>();
      ExpressionPtr node1 = expression1->sampleNode(context.getRandomGenerator(), functionSelectionProbability);
      ExpressionPtr node2 = expression2->sampleNode(context.getRandomGenerator(), functionSelectionProbability);
      ExpressionPtr newExpression1 = expression1->cloneAndSubstitute(node1, node2);
      ExpressionPtr newExpression2 = expression2->cloneAndSubstitute(node2, node1);
      if (newExpression1->getDepth() <= maxDepth && newExpression2->getDepth() <= maxDepth)
        return std::make_pair(newExpression1, newExpression2);
    }
    return std::make_pair(object1, object2);
  }

protected:
  friend class SubTreeBinaryPerturbatorClass;

  double functionSelectionProbability;
  size_t maxDepth;

  ExpressionPtr sampleNode(ExecutionContext& context, ExpressionPtr root) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    std::vector<ExpressionPtr> nodes;
    if (random->sampleBool(functionSelectionProbability))
      root->getInternalNodes(nodes);
    else
    {
      if (root->getNumSubNodes() == 0)
        return root;
      else
        root->getLeafNodes(nodes);
    }
    jassert(nodes.size() == 0);
    return nodes[random->sampleSize(nodes.size())];
  }
};

/////////////// SOLUTIONS OPERATORS //////////////////////

class SolutionsOperator : public Object
{
public:
  typedef SolutionContainer::SolutionAndFitness SolutionAndFitness;

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

// mutations
extern PerturbatorPtr shrinkExpressionPerturbator();
extern PerturbatorPtr swapExpressionPerturbator(double functionSelectionProbability);
extern PerturbatorPtr insertExpressionPerturbator(size_t maxDepth = 17);
extern PerturbatorPtr standardExpressionPerturbator(DepthControlledExpressionSamplerPtr sampler, size_t maxRegenerationDepth = 5, size_t maxDepth = 17);
// cross-over
extern BinaryPerturbatorPtr subTreeBinaryPerturbator(double functionSelectionProbability, size_t maxDepth = 17);

// operators
extern SolutionSelectorPtr tournamentSolutionSelector(SolutionComparatorPtr comparator, size_t tournamentSize);
extern SolutionsOperatorPtr nBestsSolutionSelector(SolutionComparatorPtr comparator, size_t numBests);
extern SolutionsOperatorPtr mutationOperator(PerturbatorPtr perturbator, double probability);
extern SolutionsOperatorPtr crossOverOperator(BinaryPerturbatorPtr perturbator, double probability);

extern SolutionsOperatorPtr compositeSolutionsOperator(const std::vector<SolutionsOperatorPtr>& operators);
extern SolutionsOperatorPtr compositeSolutionsOperator(SolutionsOperatorPtr operator1, SolutionsOperatorPtr operator2, SolutionsOperatorPtr operator3 = SolutionsOperatorPtr(), SolutionsOperatorPtr operator4 = SolutionsOperatorPtr(), SolutionsOperatorPtr operator5 = SolutionsOperatorPtr());
  
class TreeBasedGeneticProgrammingSolver : public PopulationBasedSolver
{
public:
  TreeBasedGeneticProgrammingSolver(SamplerPtr initialSampler, SolutionsOperatorPtr solutionOperator, size_t populationSize = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), initialSampler(initialSampler), solutionOperator(solutionOperator) {}
  TreeBasedGeneticProgrammingSolver() {}

  static SolverPtr createDefault(size_t populationSize, size_t maxGenerations, size_t tournamentSize,
                                double crossOverProbability,
                                double shrinkProbability = 0.0,
                                double swapProbability = 0.0,
                                double insertProbability = 0.0,
                                double standardProbability = 0.0)
  {
    SamplerPtr initialSampler = binaryMixtureSampler(fullExpressionSampler(2, 5), growExpressionSampler(2, 5), 0.5);
    std::vector<SolutionsOperatorPtr> operators;
    operators.push_back(nBestsSolutionSelector(objectiveComparator(0), populationSize / 4));

    //operators.push_back(tournamentSolutionSelector(objectiveComparator(0), tournamentSize));
    
    if (crossOverProbability > 0.0)
      operators.push_back(crossOverOperator(subTreeBinaryPerturbator(0.9), crossOverProbability));
    if (standardProbability > 0.0)
      operators.push_back(mutationOperator(standardExpressionPerturbator(growExpressionSampler()), insertProbability));
    if (shrinkProbability > 0.0)
      operators.push_back(mutationOperator(shrinkExpressionPerturbator(), shrinkProbability));
    if (swapProbability > 0.0)
      operators.push_back(mutationOperator(swapExpressionPerturbator(0.9), swapProbability));
    if (insertProbability > 0.0)
      operators.push_back(mutationOperator(insertExpressionPerturbator(), insertProbability));

    return new TreeBasedGeneticProgrammingSolver(initialSampler, compositeSolutionsOperator(operators), populationSize, maxGenerations);
  }

  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution = ObjectPtr(), Verbosity verbosity = verbosityQuiet)
  {
    IterativeSolver::configure(context, problem, solutions, initialSolution, verbosity);
    initialSampler->initialize(context, problem->getDomain());
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
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

  virtual void clear(ExecutionContext& context)
  {
    population = SolutionVectorPtr();
  }

protected:
  friend class TreeBasedGeneticProgrammingSolverClass;

  SamplerPtr initialSampler;
  SolutionsOperatorPtr solutionOperator;

  SolutionVectorPtr population;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_TREE_BASED_GENETIC_PROGRAMMING_H_
