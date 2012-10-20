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

/////////////// INITIALIZATION ////////////////////////

class DepthControlledExpressionSampler : public ExpressionSampler
{
public:
  DepthControlledExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5)
    : minDepth(minDepth), maxDepth(maxDepth) {}
  
  virtual ExpressionPtr sampleTree(RandomGeneratorPtr random, size_t minDepth, size_t maxDepth) const = 0;

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionSampler::initialize(context, domain);
    terminals = this->domain->getTerminals();
  }
  
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    ExpressionPtr res = sampleTree(random, minDepth, random->sampleSize(minDepth, maxDepth + 1));
    jassert(res->getDepth() >= minDepth && res->getDepth() <= maxDepth);
    return res;
  }

protected:
  friend class DepthControlledExpressionSamplerClass;

  size_t minDepth;
  size_t maxDepth;
  std::vector<ExpressionPtr> terminals;
};

class FullExpressionSampler : public DepthControlledExpressionSampler
{
public:
  FullExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5)
    : DepthControlledExpressionSampler(minDepth, maxDepth) {}
  
  // returns a tree of depth maxDepth (argument minDepth is ignored)
  virtual ExpressionPtr sampleTree(RandomGeneratorPtr random, size_t minDepth, size_t maxDepth) const
  {
    jassert(maxDepth > 0);
    if (maxDepth == 1)
      return terminals[random->sampleSize(terminals.size())];
    else
    {
      FunctionPtr function = domain->getFunction(random->sampleSize(domain->getNumFunctions()));
      std::vector<ExpressionPtr> expressions(function->getNumInputs());
      for (size_t i = 0; i < expressions.size(); ++i)
        expressions[i] = sampleTree(random, minDepth, maxDepth - 1);
      return new FunctionExpression(function, expressions);
    }
  }
};

class GrowExpressionSampler : public DepthControlledExpressionSampler
{
public:
  GrowExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5)
    : DepthControlledExpressionSampler(minDepth, maxDepth) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    DepthControlledExpressionSampler::initialize(context, domain);
    functions = this->domain->getFunctions();
    terminalsAndFunctions = this->domain->getTerminalsAndFunctions();
  }

  virtual ExpressionPtr sampleTree(RandomGeneratorPtr random, size_t minDepth, size_t maxDepth) const
  {
    jassert(minDepth >= 1 && maxDepth >= 1 && minDepth <= maxDepth);

    ObjectPtr action;
    if (minDepth > 1) // must be an internal node
      action = functions[random->sampleSize(functions.size())];
    else if (maxDepth == 1) // must be a leaf node
      action = terminals[random->sampleSize(terminals.size())];
    else // any kind of node
      action = terminalsAndFunctions[random->sampleSize(terminalsAndFunctions.size())];

    FunctionPtr function = action.dynamicCast<Function>();
    if (function)
    {
      jassert(maxDepth > 1);
      size_t newMinDepth = (minDepth > 1 ? minDepth - 1 : 1);
      size_t newMaxDepth = (maxDepth - 1);
      std::vector<ExpressionPtr> expressions(function->getNumInputs());
      for (size_t i = 0; i < expressions.size(); ++i)
      {
        expressions[i] = sampleTree(random, newMinDepth, newMaxDepth);
        jassert(expressions[i]);
      }
      ExpressionPtr res = new FunctionExpression(function, expressions);
#ifdef JUCE_DEBUG
      size_t depth = res->getDepth();
      jassert(depth >= minDepth && depth <= maxDepth);
#endif // JUCE_DEBUG
      return res;
    }
    else
    {
      jassert(1 >= minDepth && 1 <= maxDepth);
      return action.staticCast<Expression>();
    }
  }

protected:
  std::vector<FunctionPtr> functions;
  std::vector<ObjectPtr> terminalsAndFunctions;
};

class BinaryMixtureSampler : public Sampler
{
public:
  BinaryMixtureSampler(SamplerPtr sampler1, SamplerPtr sampler2, double probability)
    : sampler1(sampler1), sampler2(sampler2), probability(probability) {}
  BinaryMixtureSampler() : probability(0.5) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    sampler1->initialize(context, domain);
    sampler2->initialize(context, domain);
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return (context.getRandomGenerator()->sampleBool(probability) ? sampler2 : sampler1)->sample(context);}

protected:
  friend class BinaryMixtureSamplerClass;

  SamplerPtr sampler1;
  SamplerPtr sampler2;
  double probability;
};

/////////////// SELECTION ////////////////////////

class SolutionsOperator : public Object
{
public:
  typedef SolutionContainer::SolutionAndFitness SolutionAndFitness;

  virtual SolutionContainerPtr compute(ExecutionContext& context, const SolutionContainerPtr& solutions) = 0;
};

typedef ReferenceCountedObjectPtr<SolutionsOperator> SolutionsOperatorPtr;

class SolutionSelector : public SolutionsOperator
{
public:
  virtual SolutionAndFitness select(ExecutionContext& context, const SolutionContainerPtr& solutions) = 0;

  virtual SolutionContainerPtr compute(ExecutionContext& context, const SolutionContainerPtr& solutions)
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

  virtual SolutionContainerPtr compute(ExecutionContext& context, const SolutionContainerPtr& solutions)
  {
    comparator->initialize(solutions);
    return SolutionSelector::compute(context, solutions);
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

/////////////// GENETIC OPERATIONS ////////////////////////

class CrossOverOperator : public SolutionsOperator
{
public:
  CrossOverOperator(double crossOverProbability = 0.0)
    : crossOverProbability(crossOverProbability) {}

  typedef SolutionContainer::SolutionAndFitness SolutionAndFitness;

  virtual void crossOver(ExecutionContext& context, ObjectPtr& object1, ObjectPtr& object2) = 0;

  virtual SolutionContainerPtr compute(ExecutionContext& context, const SolutionContainerPtr& solutions)
  {
    // select and shuffle parents
    RandomGeneratorPtr random = context.getRandomGenerator();
    size_t n = solutions->getNumSolutions();
    std::vector<size_t> selected;
    selected.reserve((size_t)(crossOverProbability * 1.2 * n));
    for (size_t i = 0; i < n; ++i)
      if (random->sampleBool(crossOverProbability))
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
      crossOver(context, object1, object2);
      res->setSolution(selected[i], object1);
      res->setSolution(selected[i+1], object2);
    }
    return res;
  }

protected:
  friend class CrossOverOperatorClass;

  double crossOverProbability;
};

class SubTreeCrossOverOperator : public CrossOverOperator
{
public:
  SubTreeCrossOverOperator(double crossOverProbability, double probabilityOfInternalNode)
    : CrossOverOperator(crossOverProbability), probabilityOfInternalNode(probabilityOfInternalNode) {}
  SubTreeCrossOverOperator() : probabilityOfInternalNode(0.0) {}

  virtual void crossOver(ExecutionContext& context, ObjectPtr& object1, ObjectPtr& object2)
  {
    ExpressionPtr expression1 = *(ExpressionPtr* )&object1;
    ExpressionPtr expression2 = *(ExpressionPtr* )&object2;
    ExpressionPtr node1 = sampleNode(context, expression1);
    ExpressionPtr node2 = sampleNode(context, expression2);

    //std::cout << "A: " << expression1->toShortString() << " B: " << expression2->toShortString() << std::endl;

    object1 = copyAndSubstitute(expression1, node1, node2);
    object2 = copyAndSubstitute(expression2, node2, node1);

    //std::cout << "A': " << object1->toShortString() << " B': " << object2->toShortString() << std::endl;
  }

protected:
  friend class SubTreeCrossOverOperatorClass;

  double probabilityOfInternalNode;

  ExpressionPtr sampleNode(ExecutionContext& context, ExpressionPtr root)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    std::vector<ExpressionPtr> nodes;
    getNodes(root, random->sampleBool(probabilityOfInternalNode), nodes);
    if (nodes.empty())
    {
      jassert(!root.isInstanceOf<FunctionExpression>()); // only case where this can happen: this node is a leaf and we have tried to sample an internal node
      return root;
    }
    return nodes[random->sampleSize(nodes.size())];
  }

  void getNodes(ExpressionPtr node, bool internalNodes, std::vector<ExpressionPtr>& res)
  {
    FunctionExpressionPtr function = node.dynamicCast<FunctionExpression>();
    if (function)
    {
      if (internalNodes)
        res.push_back(node);
      for (size_t i = 0; i < node->getNumSubNodes(); ++i)
        getNodes(node->getSubNode(i), internalNodes, res);
    }
    else
    {
      if (!internalNodes)
        res.push_back(node);
    }
  }
  
  ExpressionPtr copyAndSubstitute(ExpressionPtr node, ExpressionPtr sourceNode, ExpressionPtr targetNode)
  {
    if (node == sourceNode)
      return targetNode;
    FunctionExpressionPtr functionNode = node.dynamicCast<FunctionExpression>();
    if (!functionNode)
      return node;
    std::vector<ExpressionPtr> arguments(functionNode->getNumArguments());
    for (size_t i = 0; i < arguments.size(); ++i)
      arguments[i] = copyAndSubstitute(functionNode->getArgument(i), sourceNode, targetNode);
    return new FunctionExpression(functionNode->getFunction(), arguments);
  }

};

/////////////// TOP-LEVEL SOLVER ////////////////////////

extern ExpressionSamplerPtr fullExpressionSampler(size_t minDepth, size_t maxDepth);
extern ExpressionSamplerPtr growExpressionSampler(size_t minDepth, size_t maxDepth);
extern SamplerPtr binaryMixtureSampler(SamplerPtr sampler1, SamplerPtr sampler2, double probability = 0.5);

extern SolutionSelectorPtr tournamentSolutionSelector(SolutionComparatorPtr comparator, size_t tournamentSize);

extern SolutionsOperatorPtr subTreeCrossOverOperator(double crossOverProbability, double probabilityOfInternalNode);

class TreeBasedGeneticProgrammingSolver : public PopulationBasedSolver
{
public:
  TreeBasedGeneticProgrammingSolver(SamplerPtr initialSampler, SolutionSelectorPtr selector, SolutionsOperatorPtr generator, size_t populationSize = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), initialSampler(initialSampler), selector(selector), generator(generator) {}
  TreeBasedGeneticProgrammingSolver() {}

  static SolverPtr createDefault(size_t populationSize = 4000)
  {
    SamplerPtr initialSampler = binaryMixtureSampler(fullExpressionSampler(2, 5), growExpressionSampler(2, 5), 0.5);
    SolutionSelectorPtr selector = tournamentSolutionSelector(objectiveComparator(0), 7);
    SolutionsOperatorPtr generator = subTreeCrossOverOperator(0.9, 0.9);
    return new TreeBasedGeneticProgrammingSolver(initialSampler, selector, generator, populationSize, 0);
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
      SolutionVectorPtr selectedPopulation = selector->compute(context, population);
      SolutionVectorPtr newPopulation = generator->compute(context, selectedPopulation);
      computeMissingFitnesses(context, newPopulation);
      population = newPopulation;
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
  SolutionSelectorPtr selector;
  SolutionsOperatorPtr generator;

  SolutionVectorPtr population;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_TREE_BASED_GENETIC_PROGRAMMING_H_
