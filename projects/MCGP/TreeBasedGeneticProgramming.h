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

/////////////// TOP-LEVEL SOLVER ////////////////////////

extern ExpressionSamplerPtr fullExpressionSampler(size_t minDepth, size_t maxDepth);
extern ExpressionSamplerPtr growExpressionSampler(size_t minDepth, size_t maxDepth);
extern SamplerPtr binaryMixtureSampler(SamplerPtr sampler1, SamplerPtr sampler2, double probability = 0.5);

class TreeBasedGeneticProgrammingSolver : public PopulationBasedSolver
{
public:
  TreeBasedGeneticProgrammingSolver(SamplerPtr initialSampler, size_t populationSize = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), initialSampler(initialSampler) {}
  TreeBasedGeneticProgrammingSolver() {}

  static SolverPtr createDefault()
  {
    SamplerPtr initialSampler = binaryMixtureSampler(fullExpressionSampler(2, 5), growExpressionSampler(2, 5), 0.5);
    return new TreeBasedGeneticProgrammingSolver(initialSampler, 4000, 20);
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
      // FIXME
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
  SolutionVectorPtr population;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_TREE_BASED_GENETIC_PROGRAMMING_H_
