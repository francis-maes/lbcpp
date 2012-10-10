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
# include <lbcpp-ml/ExpressionRPN.h>
# include <lbcpp-ml/Search.h>

namespace lbcpp
{

///////////////////////////////////////////////////////////

class SearchNode;
typedef ReferenceCountedObjectPtr<SearchNode> SearchNodePtr;

class SearchNode : public Object
{
public:
  SearchNode(SearchNode* parent, const SearchStatePtr& state)
    : parent(parent), state(state), actions(state->getActionDomain().staticCast<DiscreteDomain>())
  {
    if (actions && actions->getNumElements())
    {
      successors.resize(actions->getNumElements(), NULL);
      fullyVisited = false;
    }
    else
      fullyVisited = true;
  }
  SearchNode() : parent(NULL) {}

  DiscreteDomainPtr getPrunedActionDomain() const
  {
    DiscreteDomainPtr res = new DiscreteDomain();
    std::vector<size_t> candidates;
    for (size_t i = 0; i < successors.size(); ++i)
      if (!successors[i] || !successors[i]->fullyVisited)
        res->addElement(actions->getElement(i));
    return res;
  }

  SearchNodePtr getSuccessor(ExecutionContext& context, const ObjectPtr& action)
  {
    for (size_t i = 0; i < actions->getNumElements(); ++i)
      if (isSameAction(actions->getElement(i), action))
      {
        SearchNodePtr& succ = successors[i];
        if (!succ)
        {
          SearchStatePtr nextState = state->cloneAndCast<SearchState>();
          nextState->performTransition(context, action);
          succ = new SearchNode(this, nextState);
          updateIsFullyVisited();
        }
        return succ;
      }

    jassertfalse;
    return NULL;
  }

  const SearchStatePtr& getState() const
    {return state;}

  bool isFinalState() const
    {return state->isFinalState();}

  lbcpp_UseDebuggingNewOperator

private:
  friend class SearchNodeClass;

  SearchNode* parent;
  SearchStatePtr state;
  std::vector<SearchNodePtr> successors;
  DiscreteDomainPtr actions;
  bool fullyVisited;

  bool isSameAction(const ObjectPtr& action1, const ObjectPtr& action2) const
    {return Variable(action1) == Variable(action2);}

  void updateIsFullyVisited()
  {
    bool previousValue = fullyVisited;
    fullyVisited = true;
    for (size_t i = 0; i < successors.size(); ++i)
      if (!successors[i] || !successors[i]->fullyVisited)
      {
        fullyVisited = false;
        break;
      }
    if (parent && previousValue != fullyVisited)
      parent->updateIsFullyVisited();
  }
};

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

class PrunedSearchDomain : public SearchDomain
{
public:
  PrunedSearchDomain(SearchDomainPtr domain) : domain(domain)
    {rootNode = new SearchNode(NULL, domain->createInitialState());}

  virtual SearchStatePtr createInitialState() const
    {return new PrunedSearchState(rootNode);}

  virtual size_t getActionCode(const SearchStatePtr& state, const ObjectPtr& action) const
    {return domain->getActionCode(state.staticCast<PrunedSearchState>()->getNode()->getState(), action);}

  virtual DoubleVectorPtr getActionFeatures(const SearchStatePtr& state, const ObjectPtr& action) const
    {return domain->getActionFeatures(state.staticCast<PrunedSearchState>()->getNode()->getState(), action);}

protected:
  SearchDomainPtr domain;
  SearchNodePtr rootNode;
};

///////////////////////////////////////////////////////////

class ExpressionProblem : public Problem
{
public:
  ExpressionProblem()
  {
    domain = new ExpressionDomain();

    std::vector< std::pair<double, double> > limits(1);
    limits[0] = std::make_pair(-DBL_MAX, DBL_MAX);
    //limits[1] = std::make_pair(DBL_MAX, 0); // expression size: should be minimized
    this->limits = new FitnessLimits(limits);
  }

  virtual DomainPtr getDomain() const
    {return domain;}

  virtual FitnessLimitsPtr getFitnessLimits() const
    {return limits;}

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {jassertfalse; return ExpressionPtr();}

protected:
  ExpressionDomainPtr domain;
  FitnessLimitsPtr limits;
};

typedef ReferenceCountedObjectPtr<ExpressionProblem> ExpressionProblemPtr;

class F8SymbolicRegressionProblem : public ExpressionProblem
{
public:
  F8SymbolicRegressionProblem(size_t functionIndex = 0)
    : functionIndex(functionIndex)
  {
    jassert(functionIndex >= 1 && functionIndex <= 8);
    // domain
		input = domain->addInput(doubleType, "x");

		domain->addConstant(1.0);

		domain->addFunction(logDoubleFunction());
		domain->addFunction(expDoubleFunction());
		domain->addFunction(sinDoubleFunction());
		domain->addFunction(cosDoubleFunction());

		domain->addFunction(addDoubleFunction());
		domain->addFunction(subDoubleFunction());
		domain->addFunction(mulDoubleFunction());
		domain->addFunction(divDoubleFunction());

    output = domain->createSupervision(doubleType, "y");
    
    // fitness limits
    limits->setLimits(0, getWorstError(), 0.0); // absolute error: should be minimized

    // data
    const size_t numSamples = 20;
    cache = domain->createCache(numSamples);
    DenseDoubleVectorPtr supervisionValues = new DenseDoubleVector(numSamples, 0.0);
    double lowerLimit, upperLimit;
    getInputDomain(lowerLimit, upperLimit);
		for (size_t i = 0; i < numSamples; ++i)
		{
			double x = lowerLimit + (upperLimit - lowerLimit) * i / (numSamples - 1.0);// random->sampleDouble(lowerLimit, upperLimit);
      double y = computeFunction(x);

      cache->setInputObject(domain->getInputs(), i, new DenseDoubleVector(1, x));
			supervisionValues->setValue(i, y);
		}
    cache->cacheNode(defaultExecutionContext(), output, supervisionValues, T("Supervision"), false);
    cache->recomputeCacheSize();
  }

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    // retrieve predictions and supervisions
    ExpressionPtr expression = object.staticCast<Expression>();
    LuapeSampleVectorPtr predictions = cache->getSamples(context, expression);
    DenseDoubleVectorPtr supervisions = cache->getNodeCache(output);
    
    // compute mean absolute error
    ScalarVariableMean res;
    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      double prediction = it.getRawDouble();
      if (prediction == doubleMissingValue)
        prediction = 0.0;
      res.push(fabs(supervisions->getValue(it.getIndex()) - prediction));
    }

    // construct the Fitness
    std::vector<double> fitness(1);
    fitness[0] = res.getMean();
    //fitness[1] = expression->getTreeSize();
    return new Fitness(fitness, limits);
  }

protected:
  friend class F8SymbolicRegressionProblemClass;

  size_t functionIndex;

  LuapeSamplesCachePtr cache;
  VariableExpressionPtr input;
  VariableExpressionPtr output;

  void getInputDomain(double& lowerLimit, double& upperLimit) const
  {
    lowerLimit = -1.0;
		upperLimit = 1.0;
		if (functionIndex == 7)
			lowerLimit = 0.0, upperLimit = 2.0;
		if (functionIndex == 8)
			lowerLimit = 0.0, upperLimit = 4.0;
  }

  double getWorstError() const
    {return 1.0;}

	double computeFunction(double x) const
	{
		double x2 = x * x;
		switch (functionIndex)
		{
		case 1: return x * x2 + x2 + x;
		case 2: return x2 * x2 + x * x2 + x2 + x;
		case 3: return x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 4: return x2 * x2 * x2 + x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 5: return sin(x2) * cos(x) - 1.0;
		case 6: return sin(x) + sin(x + x2);
		case 7: return log(x + 1) + log(x2 + 1);
		case 8: return sqrt(x);

/*
                case 1: return x*x2-x2-x;
                case 2: return x2*x2-x2*x-x2-x;
                case 3: return x2*x2 + sin(x);
                case 4: return cos(x2*x)+sin(x+1);
                case 5: return sqrt(x)+x2;
                case 6: return x2*x2*x2 +1;
                case 7: return sin(x2*x+x2);
                case 8: return log(x2*x+1)+x;
*/
		default: jassert(false); return 0.0;
		};
	}
};

/////////////////////////////////////////

class ExpressionToExpressionRPNProblem : public DecoratorProblem
{
public:
  ExpressionToExpressionRPNProblem(ExpressionProblemPtr expressionProblem = ProblemPtr(), size_t expressionSize = 10)
    : DecoratorProblem(expressionProblem)
  {
    domain = new PrunedSearchDomain(new ExpressionRPNSearchDomain(expressionProblem->getDomain().staticCast<ExpressionDomain>(), expressionSize));
  }

  virtual DomainPtr getDomain() const
    {return domain;}
  
  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    SearchTrajectoryPtr trajectory = object.staticCast<SearchTrajectory>();
    ExpressionPtr expression = trajectory->getFinalState()->getConstructedObject().staticCast<Expression>();
    return problem->evaluate(context, expression);
  }

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {jassertfalse; return ExpressionPtr();} // FIXME if required

protected:
  SearchDomainPtr domain;
};

///////////////////

class RepeatSolver : public IterativeSolver
{
public:
  RepeatSolver(SolverPtr solver, size_t numIterations = 0)
    : IterativeSolver(numIterations), solver(solver) {}
  RepeatSolver() {}
  
  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    SolutionContainerPtr solutions = solver->optimize(context, problem, ObjectPtr(), verbosity > verbosityQuiet ? (Verbosity)(verbosity - 1) : verbosityQuiet);
    this->solutions->insertSolutions(solutions);
    return true;
  }

protected:
  friend class RepeatOptimizerClass;

  SolverPtr solver;
};

//////////////////

class SearchAlgorithm : public Solver
{
public:
  virtual void configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, ObjectPtr initialSolution, Verbosity verbosity)
  {
    Solver::configure(context, problem, front, initialSolution, verbosity);
    domain = problem->getDomain().staticCast<SearchDomain>();
    trajectory = initialSolution.staticCast<SearchTrajectory>();
    if (!trajectory)
    {
      trajectory = new SearchTrajectory();
      trajectory->setFinalState(domain->createInitialState());
    }
  }

  virtual void clear(ExecutionContext& context)
  {
    Solver::clear(context);
    domain = SearchDomainPtr();
    trajectory = SearchTrajectoryPtr();
  }

protected:
  SearchDomainPtr domain;
  SearchTrajectoryPtr trajectory;
};

typedef ReferenceCountedObjectPtr<SearchAlgorithm> SearchAlgorithmPtr;

class RolloutSearchAlgorithm : public SearchAlgorithm
{
public:
  virtual void optimize(ExecutionContext& context)
  {
    SearchTrajectoryPtr trajectory = this->trajectory->cloneAndCast<SearchTrajectory>();
    SearchStatePtr state = trajectory->getFinalState();
    while (!state->isFinalState())
    {
      if (problem->shouldStop())
        return;
      DiscreteDomainPtr availableActions = state->getActionDomain().staticCast<DiscreteDomain>();
      size_t n = availableActions->getNumElements();
      ObjectPtr action = availableActions->getElement(context.getRandomGenerator()->sampleSize(n));
      trajectory->append(action);
      state->performTransition(context, action);
    }
    trajectory->setFinalState(state);
    evaluate(context, trajectory);
  }
};

class DecoratorSearchAlgorithm : public SearchAlgorithm
{
public:
  DecoratorSearchAlgorithm(SearchAlgorithmPtr algorithm = SearchAlgorithmPtr())
    : algorithm(algorithm) {}
   
protected:
  friend class DecoratorSearchAlgorithmClass;

  SearchAlgorithmPtr algorithm;

  /*double subSearch(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr state, std::vector<Variable>& actions, DecisionProblemStatePtr& finalState)
  {
    if (state->isFinalState())
      return submitFinalState(context, objective, actions, state);
    else
    {
      double res = algorithm->search(context, objective, state, actions, finalState);
      if (res != -DBL_MAX)
        submitFinalState(context, objective, actions, finalState, res);
      return res;
    }
  }*/
};


class LookAheadSearchAlgorithm : public DecoratorSearchAlgorithm
{
public:
  LookAheadSearchAlgorithm(SearchAlgorithmPtr algorithm, double numActions = 1.0)
    : DecoratorSearchAlgorithm(algorithm), numActions(numActions) {}
  LookAheadSearchAlgorithm() : numActions(0.0) {}

protected:
  friend class LookAheadSearchAlgorithmClass;

  double numActions;

  virtual void optimize(ExecutionContext& context)
  {
    SearchTrajectoryPtr trajectory = this->trajectory->cloneAndCast<SearchTrajectory>();
    SearchStatePtr state = trajectory->getFinalState();
    DiscreteDomainPtr actions = state->getActionDomain();
    size_t n = actions->getNumElements();

    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(n, order);
    std::vector<ObjectPtr> selectedActions((size_t)(juce::jmax(1.0, n * numActions)));
    for (size_t i = 0; i < selectedActions.size(); ++i)
      selectedActions[i] = actions->getElement(order[i]);

    for (size_t i = 0; i < selectedActions.size(); ++i)
    {
      if (problem->shouldStop())
        break;
      ObjectPtr action = selectedActions[i];
      Variable stateBackup;
      state->performTransition(context, action, &stateBackup);

      trajectory->append(action);

      //DecisionProblemStatePtr finalState;
      //subSearch(context, objective, state, actions, finalState);

      trajectory->pop();      

      state->undoTransition(context, stateBackup);
    }
  }
};

// todo: finish look-ahead
// todo: step 

//////////////////

class MCGPSandBox : public WorkUnit
{
public:
  MCGPSandBox() : numEvaluations(1000), verbosity(1) {}

  virtual Variable run(ExecutionContext& context)
  {
    {
      context.enterScope("random-1");
      ScalarVariableMean scores;
      for (size_t j=0;j<5;++j)
      for (size_t i = 1; i <= 8; ++i)
      {
        ProblemPtr problem = new F8SymbolicRegressionProblem(i);
        SamplerPtr sampler = new RandomRPNExpressionSampler(10);
        SolverPtr optimizer = randomOptimizer(sampler, numEvaluations);
        ParetoFrontPtr pareto = optimizer->optimize(context, problem, ObjectPtr(), (Solver::Verbosity)verbosity);
        scores.push(pareto->getFitness(0)->getValue(0));
      }
      context.leaveScope(scores.getMean());
    }

    {
      context.enterScope("random-2");
      ScalarVariableMean scores;
      for (size_t j=0;j<5;++j)
      for (size_t i = 1; i <= 8; ++i)
      {
        ProblemPtr problem = new F8SymbolicRegressionProblem(i);
        SamplerPtr sampler = randomSearchSampler();
        ProblemPtr decoratedProblem = new ExpressionToExpressionRPNProblem(problem, 10);
        SolverPtr optimizer = randomOptimizer(sampler, numEvaluations);
        ParetoFrontPtr pareto = optimizer->optimize(context, decoratedProblem, ObjectPtr(), (Solver::Verbosity)verbosity);
        scores.push(pareto->getFitness(0)->getValue(0));
      }
      context.leaveScope(scores.getMean());
    }

    for (size_t level = 1; level < 4; ++level)
      for (size_t iterPerLevel = 10; iterPerLevel <= 320; iterPerLevel *= 2)
      {
        context.enterScope("nrpa-"+String((int)level)+"-"+String((int)iterPerLevel));
        ScalarVariableMean scores;
        for (size_t j=0;j<5;++j)
        for (size_t i = 1; i <= 8; ++i)
        {
          ProblemPtr problem = new F8SymbolicRegressionProblem(i);
          SamplerPtr sampler = logLinearActionCodeSearchSampler(0.1, 1.0);
          ProblemPtr decoratedProblem = new MaxIterationsDecoratorProblem(new ExpressionToExpressionRPNProblem(problem, 10), numEvaluations);

          SolverPtr optimizer = nrpaOptimizer(sampler, level, iterPerLevel);
          ParetoFrontPtr pareto = optimizer->optimize(context, decoratedProblem, ObjectPtr(), (Solver::Verbosity)verbosity);
          scores.push(pareto->getFitness(0)->getValue(0));
        }
        context.leaveScope(scores.getMean());
      }
    
    return true;
  }

protected:
  friend class MCGPSandBoxClass;

  size_t numEvaluations;
  size_t verbosity;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_SANDBOX_H_
