/*-----------------------------------------.---------------------------------.
| Filename: MCGPSandBox.h                  | Monte Carlo Genetic Programming |
| Author  : Francis Maes                   | SandBox                         |
| Started : 03/10/2012 19:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_SANDBOX_H_
# define LBCPP_MCGP_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Luape/LuapeCache.h>
# include <lbcpp/Luape/ExpressionBuilder.h>
# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/Sampler.h>
# include <lbcpp-ml/SolutionContainer.h>
# include <lbcpp-ml/ExpressionDomain.h>
# include <lbcpp-ml/ExpressionSampler.h>
# include <lbcpp-ml/Search.h>
# include "TreeGPOperations.h"
# include "TreeGPSamplers.h"
# include <algorithm>

namespace lbcpp
{

class PrunedSearchState : public SearchState
{
public:
  PrunedSearchState(SearchNodePtr node = SearchNodePtr())
    : node(node) {}

  virtual DomainPtr getActionDomain() const
    {return node->getPrunedActionDomain();}

  virtual void performTransition(ExecutionContext& context, const ObjectPtr& action, Variable* stateBackup = NULL)
  {
    jassert(node);
    if (stateBackup)
      *stateBackup = node;
    node = node->getSuccessor(context, action);
  }

  virtual void undoTransition(ExecutionContext& context, const Variable& stateBackup)
    {node = stateBackup.getObjectAndCast<SearchNode>();}

  virtual bool isFinalState() const
    {return node->isFinalState();}

  virtual int compare(const ObjectPtr& otherObject) const
    {return (int)((juce::int64)node.get() - (juce::int64)otherObject.staticCast<PrunedSearchState>()->node.get());}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
    {target.staticCast<PrunedSearchState>()->node = node;}
  
  virtual ObjectPtr getConstructedObject() const
    {return node->getState()->getConstructedObject();}

  virtual String toShortString() const
    {return node->getState()->toShortString();}

  const SearchNodePtr& getNode() const
    {return node;}

protected:
  friend class PrunedSearchStateClass;

  SearchNodePtr node;
};

/////////////////////////
class ExpressionToSearchProblem : public DecoratorProblem
{
public:
  ExpressionToSearchProblem(ProblemPtr expressionProblem, size_t maxSize, bool usePostfixNotation)
    : DecoratorProblem(expressionProblem)
  {
    ExpressionDomainPtr expressionDomain = expressionProblem->getDomain().staticCast<ExpressionDomain>();
    SearchStatePtr initialState;
    if (usePostfixNotation)
      initialState = postfixExpressionState(expressionDomain, maxSize);
    else
      initialState = prefixExpressionState(expressionDomain, maxSize);
    
    //if (pruneActions)
    //  initialState = new PrunedSearchState(new SearchNode(NULL, initialState));

    domain = new SearchDomain(initialState);
  }

  ExpressionToSearchProblem() {}

  virtual DomainPtr getDomain() const
    {return domain;}
  
  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    SearchTrajectoryPtr trajectory = object.staticCast<SearchTrajectory>();
    ExpressionPtr expression = trajectory->getFinalState()->getConstructedObject().staticCast<Expression>();
    return problem->evaluate(context, expression);
  }

  ExpressionProblemPtr getExpressionProblem() const
    {return problem.staticCast<ExpressionProblem>();}
  
protected:
  SearchDomainPtr domain;
};

//////////////////////////////////////

class MABSamplerExpressionSolver : public IterativeSolver
{
public:
  MABSamplerExpressionSolver(SearchActionCodeGeneratorPtr codeGenerator, size_t numArms, size_t maxExpressionSize, bool usePostfixNotation)
    : codeGenerator(codeGenerator), numArms(numArms), maxExpressionSize(maxExpressionSize), usePostfixNotation(usePostfixNotation) {}
  MABSamplerExpressionSolver() : numArms(0) {}

  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution, Verbosity verbosity)
  {
    IterativeSolver::configure(context, problem, solutions, initialSolution, verbosity);

    SearchStatePtr initialState;
    if (usePostfixNotation)
      initialState = postfixExpressionState(problem->getDomain(), maxExpressionSize);
    else
      initialState = prefixExpressionState(problem->getDomain(), maxExpressionSize);
    size_t numParameters = codeGenerator->getNumCodes(initialState);
    
    pool = new BanditPool(new BanditObjective(this), 0.1);
    pool->reserveArms(numArms);
    for (size_t i = 0; i < numArms; ++i)
    {
      DenseDoubleVectorPtr parameters = new DenseDoubleVector(numParameters, 0.0);
      for (size_t j = 0; j < numParameters; ++j)
        parameters->setValue(j, context.getRandomGenerator()->sampleDoubleFromGaussian(0.0, 2.0));
      pool->createArm(parameters);
    }
  }

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    pool->play(context, 1, false);
    if ((iter + 1) % numArms == 0)
    {
      context.enterScope("Iter " + String((int)iter));
      pool->displayInformation(context);
      context.leaveScope();
    }
    return true;
  }

  virtual void clear(ExecutionContext& context)
  {
    context.enterScope("All arms");
    pool->displayAllArms(context);
    context.leaveScope();
    IterativeSolver::clear(context);
    pool = BanditPoolPtr();
  }

  struct BanditObjective : public BanditPoolObjective
  {
    BanditObjective(MABSamplerExpressionSolver* owner)
      : owner(owner) {}

    virtual void getObjectiveRange(double& worst, double& best) const
    {
      // TODO: check !
      worst = owner->problem->getFitnessLimits()->getLowerLimit(0);
      best = owner->problem->getFitnessLimits()->getUpperLimit(0);
    }

    virtual double computeObjective(ExecutionContext& context, const Variable& parameter, size_t instanceIndex)
    {
      ProblemPtr searchProblem = new ExpressionToSearchProblem(owner->problem, owner->maxExpressionSize, owner->usePostfixNotation);
      SamplerPtr sampler = logLinearActionCodeSearchSampler(owner->codeGenerator);
      sampler->setVariable(sampler->getClass()->findMemberVariable("parameters"), parameter.getObjectAndCast<DenseDoubleVector>());
      SolverPtr solver = stepSearchAlgorithm(lookAheadSearchAlgorithm(rolloutSearchAlgorithm(sampler)));
      SolutionContainerPtr solutions = solver->optimize(context, searchProblem);
      owner->solutions->insertSolution(solutions->getSolution(0), solutions->getFitness(0));
      return solutions->getFitness(0)->getValue(0);
    }

  private:
    MABSamplerExpressionSolver* owner;
  };

protected:
  friend class MABSamplerExpressionSolverClass;

  SearchActionCodeGeneratorPtr codeGenerator;
  size_t numArms;
  size_t maxExpressionSize;
  bool usePostfixNotation;

  BanditPoolPtr pool;
};

///////////////////////////////////////////////////////////

class SimpleExpressionSearchActionCodeGenerator : public SearchActionCodeGenerator
{
public:
  virtual size_t getCode(const SearchStatePtr& state, const ObjectPtr& action)
  {
    const ExpressionStatePtr& expressionState = state.staticCast<ExpressionState>();
    size_t symbol = expressionState->getDomain()->getSymbolIndex(action);
    return symbol * expressionState->getMaxSize() + expressionState->getTrajectoryLength();
  }
};

class NGramExpressionSearchActionCodeGenerator : public SearchActionCodeGenerator
{
public:
  NGramExpressionSearchActionCodeGenerator(size_t n = 0)
    : n(n) {}

  virtual size_t getCode(const SearchStatePtr& state, const ObjectPtr& action)
  {
    const ExpressionStatePtr& expressionState = state.staticCast<ExpressionState>();
    const ExpressionDomainPtr& domain = expressionState->getDomain();
    jassert(n >= 1);
    size_t res = domain->getSymbolIndex(action);
    size_t numSymbols = domain->getNumSymbols();
    const std::vector<ObjectPtr>& previousSymbols = expressionState->getTrajectory();
    for (size_t i = 0; i < n - 1; ++i)
    {
      res *= numSymbols;
      res += domain->getSymbolIndex(i < previousSymbols.size() ? previousSymbols[previousSymbols.size() - 1 - i] : ObjectPtr());
    }
    return res;
  }
  
  virtual size_t getNumCodes(const SearchStatePtr& initialState) const
  {
    size_t numSymbols = initialState.staticCast<ExpressionState>()->getDomain()->getNumSymbols();
    size_t res = 1;
    for (size_t i = 0; i < n; ++i)
      res *= numSymbols;
    return res;
  }

protected:
  friend class NGramExpressionSearchActionCodeGeneratorClass;

  size_t n; // n = 1: one code per symbol, n = 2: one code per bigram, n = 3: one code per trigram, ...
};

///////////////////////////////////////////////////////////

class MinimalisticIncrementalEvolver : public IterativeSolver
{
public:
  MinimalisticIncrementalEvolver(SamplerPtr sampler, size_t tournamentSize)
    : sampler(sampler), tournamentSize(tournamentSize) {}
  MinimalisticIncrementalEvolver() : tournamentSize(0) {}

  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution = ObjectPtr(), Verbosity verbosity = verbosityQuiet)
  {
    currentSampler = sampler->cloneAndCast<Sampler>();
    currentSampler->initialize(context, problem->getDomain());
    currentSolutions = new SolutionVector(problem->getFitnessLimits());
    IterativeSolver::configure(context, problem, solutions, initialSolution, verbosity);
  }
  
  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    ObjectPtr solution = currentSampler->sample(context);
    FitnessPtr fitness = evaluate(context, solution);
    currentSolutions->insertSolution(solution, fitness);
    if (currentSolutions->getNumSolutions() == tournamentSize)
    {
      performTournament(context, currentSampler, currentSolutions);
      currentSolutions->clear();
    }
    return true;
  }

  virtual void clear(ExecutionContext& context)
  {
    IterativeSolver::clear(context);
    currentSolutions = SolutionVectorPtr();
    currentSampler = SamplerPtr();
  }

protected:
  friend class MinimalisticIncrementalEvolverClass;

  SamplerPtr sampler;
  size_t tournamentSize;
  
  SamplerPtr currentSampler;
  SolutionVectorPtr currentSolutions;

  void performTournament(ExecutionContext& context, SamplerPtr sampler, SolutionVectorPtr solutions)
  {
    std::vector< std::pair<size_t, size_t> > mapping;
    std::vector<ParetoFrontPtr> fronts = solutions->nonDominatedSort(&mapping);
    jassert(fronts.size());
    if (fronts.size() == 1)
      return; // all solutions are comparable
    size_t numPositives = fronts[0]->getNumSolutions();
    size_t numNegatives = mapping.size() - numPositives;
    jassert(numPositives && numNegatives);
    double positiveWeight = 1.0 / numPositives;
    double negativeWeight = 1.0 / numNegatives;

    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(solutions->getNumSolutions(), order);
    for (size_t i = 0; i < order.size(); ++i)
    {
      size_t index = order[i];
      bool isPositive = (mapping[index].first == 0); // was mapped to the optimal front
      sampler->reinforce(context, solutions->getSolution(index), isPositive ? positiveWeight : negativeWeight);
    }
  }
};

///////////////////////////////////////////////////////////

class RandomWithActiveSubTreesSampler;
typedef ReferenceCountedObjectPtr<RandomWithActiveSubTreesSampler> RandomWithActiveSubTreesSamplerPtr;

class RandomWithActiveSubTreesSampler : public SearchSampler
{
public:
  void setSubTrees(const std::vector< std::pair<ExpressionPtr, double> >& subTrees)
  {
    this->subTreeSet.clear();
    this->subTrees.resize(subTrees.size());
    for (size_t i = 0; i < subTrees.size(); ++i)
    {
      this->subTrees[i] = subTrees[i].first;
      subTreeSet.insert(subTrees[i].first);
    }
  }

  virtual ObjectPtr sampleAction(ExecutionContext& context, SearchStatePtr state) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    DiscreteDomainPtr actions = state->getActionDomain();
    size_t numActions = actions->getNumElements();
    size_t numTerminalActions = getNumTerminalActions(actions);
    if (subTrees.empty() || numTerminalActions == 0)
      return actions->getElement(random->sampleSize(numActions));
    else
    {
      size_t r = random->sampleSize(numActions + subTrees.size());
      if (r < numActions)
        return actions->getElement(r);
      else
        return subTrees[r - numActions];
    }
  }
  
  bool selectOneNewSubTree(const ExpressionPtr& expression)
  {
    size_t n = expression->getNumSubNodes();
    if (!n)
      return false;
    for (size_t i = 0; i < n; ++i)
      if (selectOneNewSubTree(expression->getSubNode(i)))
        return true;
    if (expression->getTreeSize() > 5)
      return false;
    if (subTreeSet.find(expression) == subTreeSet.end())
    {
      addSubTree(expression);
      return true;
    }
    else
      return false;
  }

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object, double weight)
  {
    //std::cout << "REINFORCE " << object->toShortString() << std::endl;
    jassert(weight == 1.0);
    ExpressionPtr expression = object.staticCast<SearchTrajectory>()->getFinalState()->getConstructedObject().staticCast<Expression>();
    selectOneNewSubTree(expression);
  }
  
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    ObjectPtr res = SearchSampler::sample(context);
    //std::cout << res->toShortString() << std::endl;
    //std::cout << res.staticCast<SearchTrajectory>()->getFinalState()->getConstructedObject().staticCast<Expression>()->getTreeSize() << " ";
    return res;
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    RandomWithActiveSubTreesSamplerPtr t = target.staticCast<RandomWithActiveSubTreesSampler>();
    t->subTrees = subTrees;
    t->subTreeSet = subTreeSet;
    t->domain = domain;
  }

private:
  friend class RandomWithActiveSubTreesSamplerClass;

  std::vector<ExpressionPtr> subTrees;
  std::set<ExpressionPtr> subTreeSet;

  void addSubTree(const ExpressionPtr& subTree)
  {
    //std::cout << "NEW SUB-TREE: " << subTree->toShortString() << std::endl;
    subTrees.push_back(subTree);
    subTreeSet.insert(subTree);
  }

  size_t getNumTerminalActions(DiscreteDomainPtr actions) const
  {
    size_t res = 0;
    for (size_t i = 0; i < actions->getNumElements(); ++i)
    {
      ObjectPtr action = actions->getElement(i);
      if (action && action.isInstanceOf<Expression>())
        ++res;
    }
    return res;
  }
};

///////////////////////////////////////////////////////////

class RandomWithActiveSubTreesSolver : public PopulationBasedSolver
{
public:
  RandomWithActiveSubTreesSolver(size_t populationSize = 100, size_t numGenerations = 0, size_t numBests = 30, size_t numActiveSubTrees = 10)
    : PopulationBasedSolver(populationSize, numGenerations), numBests(numBests), numActiveSubTrees(numActiveSubTrees) {}

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    RandomWithActiveSubTreesSamplerPtr sampler = new RandomWithActiveSubTreesSampler();
    sampler->setSubTrees(subTrees);
    sampler->initialize(context, problem->getDomain());
    
    SolutionVectorPtr population = sampleAndEvaluatePopulation(context, sampler, populationSize);
    makePopulationStatistics(context, population);
    SolutionVectorPtr selected = population->sort(objectiveComparator(0));//population->selectNBests(objectiveComparator(0), numBests);
    subTrees = getMostFrequentSubTrees(selected);
    context.enterScope("SubTrees at Iter " + String((int)iter));
    ExpressionDomainPtr expressionDomain = problem->getDomain().staticCast<SearchDomain>()->getInitialState().staticCast<ExpressionState>()->getDomain();
    for (size_t i = 0; i < subTrees.size(); ++i)
      context.informationCallback(subTrees[i].first->toShortString() + " [" + String(subTrees[i].second) + "]");
    context.leaveScope();

    return true;
  }
  
  virtual void clear(ExecutionContext& context)
    {subTrees.clear();}

  void makePopulationStatistics(ExecutionContext& context, SolutionVectorPtr population)
  {
    ScalarVariableStatisticsPtr treeSize = new ScalarVariableStatistics("treeSize");
    ScalarVariableStatisticsPtr treeDepth = new ScalarVariableStatistics("treeDepth");
    for (size_t i = 0; i < population->getNumSolutions(); ++i)
    {
      ExpressionPtr expression = population->getSolution(i).staticCast<SearchTrajectory>()->getFinalState()->getConstructedObject().staticCast<Expression>();
      treeSize->push(expression->getTreeSize());
      treeDepth->push(expression->getDepth());
    }
    context.resultCallback("treeSize", treeSize);
    context.resultCallback("treeDepth", treeDepth);
  }

  typedef std::map<String, std::pair<ExpressionPtr, double> > SubTreeMap;

  static void addWeightToSubTree(ExpressionPtr expression, double weight, SubTreeMap& res)
  {
    String str = expression->toShortString();
    SubTreeMap::iterator it = res.find(str);
    if (it == res.end())
      res[str] = std::make_pair(expression, weight);
    else
      it->second.second += weight;
  }

  static void fillSubTreeMap(ExpressionPtr expression, double weight, SubTreeMap& res)
  {
    if (expression->getNumSubNodes() > 0)
      addWeightToSubTree(expression, weight, res);
    size_t n = expression->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      fillSubTreeMap(expression->getSubNode(i), weight, res);
  }

  struct CompareExpressions
  {
    bool operator()(const std::pair<ExpressionPtr, double>& a, const std::pair<ExpressionPtr, double>& b) const
    {
      if (a.second != b.second)
        return a.second > b.second;
      size_t sa = a.first->getTreeSize();
      size_t sb = b.first->getTreeSize();
      if (sa != sb)
        return sa < sb;
      size_t da = a.first->getDepth();
      size_t db = b.first->getDepth();
      if( da != db)
        return da < db;
      return a.first.get() < b.first.get();
    }
  };

  std::vector< std::pair<ExpressionPtr, double> > getMostFrequentSubTrees(const SolutionVectorPtr& population) const
  {
    SubTreeMap subTreeWeights;
    for (size_t i = 0; i < population->getNumSolutions(); ++i)
    {
      double k = 1 - 2.0 * i / (population->getNumSolutions() - 1.0);
      ExpressionPtr expression = population->getSolution(i).staticCast<SearchTrajectory>()->getFinalState()->getConstructedObject().staticCast<Expression>();
      fillSubTreeMap(expression, k / (double)expression->getTreeSize(), subTreeWeights);
    }
    std::vector< std::pair<ExpressionPtr, double> > expressions;
    expressions.reserve(subTreeWeights.size());
    for (SubTreeMap::const_iterator it = subTreeWeights.begin(); it != subTreeWeights.end(); ++it)
      expressions.push_back(it->second);
    std::sort(expressions.begin(), expressions.end(), CompareExpressions());

    if (numActiveSubTrees < expressions.size())
      expressions.resize(numActiveSubTrees);
    return expressions;
  }


protected:
  friend class RandomWithActiveSubTreesSolverClass;

  size_t numBests;
  size_t numActiveSubTrees;
  
  std::vector< std::pair<ExpressionPtr, double> > subTrees;
};

///////////////////////////////////////////////////////////

class MCGPEvaluationDecoratorProblem : public MaxIterationsDecoratorProblem
{
public:
  MCGPEvaluationDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations)
    : MaxIterationsDecoratorProblem(problem, maxNumEvaluations)
  {
    nextEvaluationCount = 1.5;
    startingTime = Time::getMillisecondCounterHiRes() / 1000.0;
    nextEvaluationDeltaTime = 0.001;
  }

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    FitnessPtr res = MaxIterationsDecoratorProblem::evaluate(context, solution);

    if (!bestFitness || res->strictlyDominates(bestFitness))
      bestFitness = res;

    while (numEvaluations >= (size_t)nextEvaluationCount)
    {
      fitnessPerEvaluationCount.push_back(bestFitness->getValue(0));
      nextEvaluationCount = nextEvaluationCount * 1.5;
    }

    double deltaTime = Time::getMillisecondCounterHiRes() / 1000.0 - startingTime;
    while (deltaTime >= nextEvaluationDeltaTime)
    {
      fitnessPerCpuTime.push_back(bestFitness->getValue(0));
      nextEvaluationDeltaTime *= 1.5;
    }

    return res;
  }
  
  const std::vector<double>& getFitnessPerEvaluationCount() const
    {return fitnessPerEvaluationCount;}

  const std::vector<double>& getFitnessPerCpuTime() const
    {return fitnessPerCpuTime;}

protected:
  std::vector<double> fitnessPerEvaluationCount;  
  std::vector<double> fitnessPerCpuTime;
  double nextEvaluationCount;
  double startingTime;
  double nextEvaluationDeltaTime;

  FitnessPtr bestFitness;
};

typedef ReferenceCountedObjectPtr<MCGPEvaluationDecoratorProblem> MCGPEvaluationDecoratorProblemPtr;

class MCGPSandBox : public WorkUnit
{
public:
  MCGPSandBox() : numEvaluations(1000), numRuns(100), maxExpressionSize(10), verbose(false) {}

  virtual Variable run(ExecutionContext& context)
  {
    std::vector< std::pair<SolverPtr, String> > solvers;

    //SamplerPtr sampler = Sampler::createFromFile(context, context.getFile("samplers/parity_prefix.sampler"));
    //context.resultCallback("postfixSampler", sampler);

    SolverPtr treeGP1, treeGP2, treeGP3;
    
    if (problem->getClassName() == T("BooleanParityProblem"))
    {
      treeGP1 = TreeGPOperationsSolver::createDefault(4000, juce::jmax(1, numEvaluations / 4000), 7, 0.9, 0, 0, 0);
      treeGP2 = TreeGPSamplersSolver::createDefault(4000, juce::jmax(1, numEvaluations / 4000), 7, 0.9, 0, 0, 0);
    }
    else if (problem->getClassName() == T("BooleanMultiplexerProblem"))
    {
      treeGP1 = TreeGPOperationsSolver::createDefault(4000, juce::jmax(1, numEvaluations / 4000), 7, 0.8, 0.05, 0.05, 0.05);
      treeGP2 = TreeGPSamplersSolver::createDefault(4000, juce::jmax(1, numEvaluations / 4000), 7, 0.8, 0.05, 0.05, 0.05);
    }
    else if (problem->getClassName() == T("SantaFeTrailProblem"))
    {
      treeGP1 = TreeGPOperationsSolver::createDefault(500, juce::jmax(1, numEvaluations / 500), 7, 0.8, 0.05, 0.05, 0.05);
      treeGP2 = TreeGPSamplersSolver::createDefault(500, juce::jmax(1, numEvaluations / 500), 7, 0.8, 0.05, 0.05, 0.05);
    }
    else if (problem->getClassName() == T("QuarticSymbolicRegressionProblem"))
    {
      treeGP1 = TreeGPOperationsSolver::createDefault(100, juce::jmax(1, numEvaluations / 100), 7, 0.8, 0.05, 0.05, 0.5);
      treeGP2 = TreeGPSamplersSolver::createDefault(100, juce::jmax(1, numEvaluations / 100), 7, 0.8, 0.05, 0.05, 0.5);
    }
    else
      jassertfalse;

    SamplerPtr randomSampler = randomSearchSampler();
    //solvers.push_back(std::make_pair(new RandomWithActiveSubTreesSolver(1000, numEvaluations / 1000, 300, 50), "random-ast10"));
    solvers.push_back(std::make_pair(randomSolver(randomSampler, numEvaluations), "random"));
    /*
    SamplerPtr learnableSampler = new RandomWithActiveSubTreesSampler();
    for (size_t level = 1; level <= 3; ++level)
    {
      size_t iterationsPerLevel = (size_t)pow((double)numEvaluations, 1.0 / level);
      solvers.push_back(std::make_pair(repeatSolver(nrpaSolver(learnableSampler, level, iterationsPerLevel)), "nrpa" + String((int)level)));
    }*/
    solvers.push_back(std::make_pair(nmcSolver(0, randomSampler), "nmc0"));
    solvers.push_back(std::make_pair(nmcSolver(1, randomSampler), "nmc1"));
    solvers.push_back(std::make_pair(nmcSolver(2, randomSampler), "nmc2"));
    solvers.push_back(std::make_pair(nmcSolver(3, randomSampler), "nmc3"));

    std::vector< std::pair<SearchActionCodeGeneratorPtr, String> > codeGenerators;
    codeGenerators.push_back(std::make_pair(new SimpleExpressionSearchActionCodeGenerator(), "posandsymb"));
    //codeGenerators.push_back(std::make_pair(new NGramExpressionSearchActionCodeGenerator(1), "unigram"));
    codeGenerators.push_back(std::make_pair(new NGramExpressionSearchActionCodeGenerator(2), "bigram"));
    //codeGenerators.push_back(std::make_pair(new NGramExpressionSearchActionCodeGenerator(3), "trigram"));
    //codeGenerators.push_back(std::make_pair(new NGramExpressionSearchActionCodeGenerator(4), "4gram"));
    //codeGenerators.push_back(std::make_pair(new NGramExpressionSearchActionCodeGenerator(5), "5gram"));

    for (size_t level = 1; level <= 5; ++level)
    {
      for (size_t i = 0; i < codeGenerators.size(); ++i)
      {
        //solvers.push_back(std::make_pair(new MABSamplerExpressionSolver(codeGenerators[i].first, 100, maxExpressionSize, true), "mab-nmc1-" + codeGenerators[i].second));
        SamplerPtr learnableSampler = logLinearActionCodeSearchSampler(codeGenerators[i].first, 0.1, 1.0);
        String postfix = String((int)level) + "-" + codeGenerators[i].second;
        //solvers.push_back(std::make_pair(ceSolver(1000, 300, false, learnableSampler), "NON-E CE" + postfix));
        //solvers.push_back(std::make_pair(ceSolver(1000, 300, true, learnableSampler), "CE" + postfix));

        size_t iterationsPerLevel = (size_t)pow((double)numEvaluations, 1.0 / level);
        solvers.push_back(std::make_pair(repeatSolver(nrpaSolver(learnableSampler, level, iterationsPerLevel)), "nrpa" + postfix));
        //iterationsPerLevel = (size_t)pow((double)numEvaluations / 2.0, 1.0 / level);
        //solvers.push_back(std::make_pair(repeatSolver(beamNRPASolver(learnableSampler, level, iterationsPerLevel, 2, 1)), "bnrpa2-" + postfix));
        //iterationsPerLevel = (size_t)pow((double)numEvaluations / 4.0, 1.0 / level);
        //solvers.push_back(std::make_pair(repeatSolver(beamNRPASolver(learnableSampler, level, iterationsPerLevel, 4, 1)), "bnrpa4-" + postfix));
        iterationsPerLevel = (size_t)pow((double)numEvaluations / 8.0, 1.0 / level);
        solvers.push_back(std::make_pair(repeatSolver(beamNRPASolver(learnableSampler, level, iterationsPerLevel, 8, 1)), "bnrpa8-" + postfix));
        //iterationsPerLevel = (size_t)pow((double)numEvaluations / 16.0, 1.0 / level);
        //solvers.push_back(std::make_pair(repeatSolver(beamNRPASolver(learnableSampler, level, iterationsPerLevel, 16, 1)), "bnrpa16-" + postfix));
        //iterationsPerLevel = (size_t)pow((double)numEvaluations / 32.0, 1.0 / level);
        //solvers.push_back(std::make_pair(repeatSolver(beamNRPASolver(learnableSampler, level, iterationsPerLevel, 32, 1)), "bnrpa32-" + postfix));
      }
    }

#if 0
      for (double learningRate = 0.000001; learningRate <= 10.0; learningRate *= 10.0)
      {
        SamplerPtr learnableSampler = logLinearActionCodeSearchSampler(codeGenerators[i].first, 0.1, learningRate);
      
        //solvers.push_back(std::make_pair(createNrpaSolver(1, learnableSampler), "nrpa1" + postfix));
        //solvers.push_back(std::make_pair(createNrpaSolver(2, learnableSampler), "nrpa2" + postfix));
        //solvers.push_back(std::make_pair(createNrpaSolver(3, learnableSampler), "nrpa3" + postfix));
        SolverPtr solver = new MinimalisticIncrementalEvolver(learnableSampler, 7);
        solvers.push_back(std::make_pair(solver, "mini" + postfix));
      }
#endif // 0

      
    
    solvers.push_back(std::make_pair(treeGP1, "treegp"));
    solvers.push_back(std::make_pair(treeGP2, "treegp-samplers"));


   
    //solvers.push_back(std::make_pair(ceSolver(100, 30, false, 0.1), "ce(100, 30, false, 0.1"));
    //solvers.push_back(std::make_pair(ceSolver(100, 30, true, 0.1), "ce(100, 30, true, 0.1"));

    //solvers.push_back(std::make_pair(stepLaSolver(1, 3), "step(1)la(3)"));
    //solvers.push_back(std::make_pair(stepLaSolver(2, 3), "step(2)la(3)"));
    
    std::vector<SolverInfo> infos;
    context.enterScope("Running");
    for (size_t i = 0; i < solvers.size(); ++i)
    {
      String name = solvers[i].second;
      infos.push_back(runSolver(context, solvers[i].first, name + "-prefix", false)); // polish
      infos.push_back(runSolver(context, solvers[i].first, name + "-postfix", true)); // reverse polish
      //infos.push_back(runSolver(context, solvers[i].first, name, true)); // reverse polish
    }
    context.leaveScope();

    context.enterScope("Results vs. evaluations");
    displayResults(context, infos, false);
    context.leaveScope();

    context.enterScope("Results vs. time");
    displayResults(context, infos, true);
    context.leaveScope();
    return true;
  }

protected:
  friend class MCGPSandBoxClass;

  ExpressionProblemPtr problem;
  size_t numEvaluations;
  size_t numRuns;
  size_t maxExpressionSize;
  File outputDirectory;
  bool verbose;

  struct SolverInfo
  {
    String name;
    std::vector<double> fitnessPerEvaluationCount;
    std::vector<double> fitnessPerCpuTime;

    const std::vector<double>& getResults(bool inFunctionOfCpuTime) const
      {return inFunctionOfCpuTime ? fitnessPerCpuTime : fitnessPerEvaluationCount;}

    double getResult(bool inFunctionOfCpuTime, size_t index) const
    {
      const std::vector<double>& results = getResults(inFunctionOfCpuTime);
      return index < results.size() ? results[index] : results.back();
    }
  };

  SolverPtr nmcSolver(size_t level, SamplerPtr sampler) const
  {
    SolverPtr res = rolloutSearchAlgorithm(sampler);
    for (size_t i = 0; i < level; ++i)
      res = stepSearchAlgorithm(lookAheadSearchAlgorithm(res));
    return repeatSolver(res);
  }

  SolverPtr stepLaSolver(size_t numSteps, size_t numLookAheads, SamplerPtr sampler) const
  {
    SolverPtr res = rolloutSearchAlgorithm(sampler);
    for (size_t i = 0; i < numLookAheads; ++i)
      res = lookAheadSearchAlgorithm(res);
    for (size_t i = 0; i < numSteps; ++i)
      res = stepSearchAlgorithm(res);
    return repeatSolver(res);
  }

  SolverPtr createNrpaSolver(size_t level, SamplerPtr sampler) const
    {return repeatSolver(nrpaSolver(sampler, level, (size_t)pow((double)numEvaluations, 1.0 / level)));}

  SolverPtr ceSolver(size_t populationSize, size_t numTrainingSamples, bool elitist, SamplerPtr sampler) const
    {return crossEntropySolver(sampler, populationSize, numTrainingSamples, numEvaluations / populationSize, elitist);}

  SolverInfo runSolver(ExecutionContext& context, SolverPtr solver, const String& description, bool usePostfixNotation)
  {
    OutputStream* evalsOutput = NULL;
    OutputStream* timesOutput = NULL;

    if (outputDirectory != File::nonexistent)
    {
      outputDirectory.createDirectory();
      File evalsFile = outputDirectory.getChildFile(description + "-evals.txt");
      if (evalsFile.exists()) evalsFile.deleteFile();
      evalsOutput = evalsFile.createOutputStream();

      File timesFile = outputDirectory.getChildFile(description + "-times.txt");
      if (timesFile.exists()) timesFile.deleteFile();
      timesOutput = timesFile.createOutputStream();
    }

    context.enterScope(description);
    context.resultCallback("solver", solver);
	  std::vector<SolverInfo> runInfos(numRuns);
    size_t shortest1 = (size_t)-1;
    size_t shortest2 = (size_t)-1;
    for (size_t i = 0; i < numRuns; ++i)
    {
      SolverInfo& info = runInfos[i];
      if (verbose)
        context.enterScope("Run " + String((int)i));
      double res = runSolverOnce(context, solver, info, usePostfixNotation);
      writeToOutput(evalsOutput, info.fitnessPerEvaluationCount);
      writeToOutput(timesOutput, info.fitnessPerCpuTime);
      if (info.fitnessPerEvaluationCount.size() < shortest1)
        shortest1 = info.fitnessPerEvaluationCount.size();
      if (info.fitnessPerCpuTime.size() < shortest2)
        shortest2 = info.fitnessPerCpuTime.size();
      if (verbose)
        context.leaveScope(res);
      context.progressCallback(new ProgressionState(i+1, numRuns, "Runs"));
    }


    if (evalsOutput)
      deleteAndZero(evalsOutput);
    if (timesOutput)
      deleteAndZero(timesOutput);

    SolverInfo res;
    res.name = description;
    res.fitnessPerEvaluationCount.resize(shortest1);
    mergeResults(res.fitnessPerEvaluationCount, runInfos, false);
    res.fitnessPerCpuTime.resize(shortest2);
    mergeResults(res.fitnessPerCpuTime, runInfos, true);
    context.leaveScope(res.fitnessPerEvaluationCount.back());
    return res;
  }
  
  void writeToOutput(OutputStream* ostr, const std::vector<double>& values)
  {
    if (ostr)
    {
      String line;
      for (size_t i = 0; i < values.size(); ++i)
        line += String(values[i]) + " ";
      line += "\n";
      *ostr << line;
      ostr->flush();
    }
  }

  double runSolverOnce(ExecutionContext& context, SolverPtr solver, SolverInfo& info, bool usePostfixNotation)
  {
    problem->initialize(context); // reinitialize problem (necessary because some problems such as koza symbolic regression are indeed distributions over problems)
    ProblemPtr problem = this->problem;
    if (!solver.isInstanceOf<TreeGPOperationsSolver>() && !solver.isInstanceOf<TreeGPSamplersSolver>() && !solver.isInstanceOf<TestSolver>() && !solver.isInstanceOf<MABSamplerExpressionSolver>())
      problem = new ExpressionToSearchProblem(problem, maxExpressionSize, usePostfixNotation);
    MCGPEvaluationDecoratorProblemPtr decoratedProblem = new MCGPEvaluationDecoratorProblem(problem, numEvaluations);
    SolutionContainerPtr solutions = solver->optimize(context, decoratedProblem, ObjectPtr(), verbose ? Solver::verbosityDetailed : Solver::verbosityQuiet);
    info.fitnessPerEvaluationCount = decoratedProblem->getFitnessPerEvaluationCount();
    info.fitnessPerCpuTime = decoratedProblem->getFitnessPerCpuTime();
    return solutions->getFitness(0)->getValue(0);
  }

  void mergeResults(std::vector<double>& res, const std::vector<SolverInfo>& infos, bool inFunctionOfCpuTime)
  {
    for (size_t i = 0; i < res.size(); ++i)
    {
      ScalarVariableMean mean;
      for (size_t j = 0; j < infos.size(); ++j)
        mean.push(infos[j].getResult(inFunctionOfCpuTime, i));
      res[i] = mean.getMean();
    }
  }

  void displayResults(ExecutionContext& context, const std::vector<SolverInfo>& infos, bool inFunctionOfCpuTime)
  {
    size_t shortestLength = (size_t)-1;
    for (size_t i = 0; i < infos.size(); ++i)
    {
      size_t length = infos[i].getResults(inFunctionOfCpuTime).size();
      if (length < shortestLength)
        shortestLength = length;
    }
    double x = inFunctionOfCpuTime ? 0.001 : 1.5;
    for (size_t i = 2; i < shortestLength; ++i)
    {
      context.enterScope(String(x));

      context.resultCallback("log10(x)", log10(x));
      for (size_t j = 0; j < infos.size(); ++j)
        context.resultCallback(infos[j].name, infos[j].getResult(inFunctionOfCpuTime, i));
      context.leaveScope();
      x *= 1.5;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_SANDBOX_H_
