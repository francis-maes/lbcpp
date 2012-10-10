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
    jassert(!state->isFinalState());
    jassert(actions);
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

  virtual size_t getActionCode(const ObjectPtr& action) const
    {return node->getState()->getActionCode(action);}

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
    {return (int)node - (int)otherObject.staticCast<PrunedSearchState>()->node;}

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
  F8SymbolicRegressionProblem(size_t functionIndex)
    : functionIndex(functionIndex)
  {
    initialize();
  }
  F8SymbolicRegressionProblem() {}

  void initialize()
  {
    jassert(functionIndex >= 0 && functionIndex < 8);
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
      if (prediction == doubleMissingValue || !isNumberValid(prediction))
        prediction = 0.0;
      res.push(fabs(supervisions->getValue(it.getIndex()) - prediction));
    }

    // construct the Fitness
    std::vector<double> fitness(1);
    fitness[0] = res.getMean();
    //fitness[1] = expression->getTreeSize();
    return new Fitness(fitness, limits);
  }
  
  virtual bool loadFromString(ExecutionContext& context, const String& str)
  {
    if (!ExpressionProblem::loadFromString(context, str))
      return false;
    initialize();
    return true;
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
		if (functionIndex == 6)
			lowerLimit = 0.0, upperLimit = 2.0;
		if (functionIndex == 7)
			lowerLimit = 0.0, upperLimit = 4.0;
  }

  double getWorstError() const
    {return 1.0;}

	double computeFunction(double x) const
	{
		double x2 = x * x;
		switch (functionIndex)
		{
		case 0: return x * x2 + x2 + x;
		case 1: return x2 * x2 + x * x2 + x2 + x;
		case 2: return x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 3: return x2 * x2 * x2 + x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
		case 4: return sin(x2) * cos(x) - 1.0;
		case 5: return sin(x) + sin(x + x2);
		case 6: return log(x + 1) + log(x2 + 1);
		case 7: return sqrt(x);

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
    ExpressionDomainPtr expressionDomain = expressionProblem->getDomain().staticCast<ExpressionDomain>();
    SearchStatePtr rpnState = expressionRPNSearchState(expressionDomain, expressionDomain->getSearchSpace(defaultExecutionContext(), expressionSize));
    SearchNodePtr rootNode = new SearchNode(NULL, rpnState);
    domain = new SearchDomain(new PrunedSearchState(rootNode));
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

class MCGPEvaluationDecoratorProblem : public MaxIterationsDecoratorProblem
{
public:
  MCGPEvaluationDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations)
    : MaxIterationsDecoratorProblem(problem, maxNumEvaluations)
  {
    nextEvaluationCount = 1;
    startingTime = Time::getMillisecondCounterHiRes() / 1000.0;
    nextEvaluationDeltaTime = 0.001;
  }

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    FitnessPtr res = MaxIterationsDecoratorProblem::evaluate(context, solution);

    if (!bestFitness || res->strictlyDominates(bestFitness))
      bestFitness = res;

    if (numEvaluations == nextEvaluationCount)
    {
      fitnessPerEvaluationCount.push_back(bestFitness->getValue(0));
      nextEvaluationCount *= 2;
    }

    double deltaTime = Time::getMillisecondCounterHiRes() / 1000.0 - startingTime;
    while (deltaTime >= nextEvaluationDeltaTime)
    {
      fitnessPerCpuTime.push_back(bestFitness->getValue(0));
      nextEvaluationDeltaTime *= 2.0;
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
  size_t nextEvaluationCount;
  double startingTime;
  double nextEvaluationDeltaTime;

  FitnessPtr bestFitness;
};

typedef ReferenceCountedObjectPtr<MCGPEvaluationDecoratorProblem> MCGPEvaluationDecoratorProblemPtr;

class MCGPSandBox : public WorkUnit
{
public:
  MCGPSandBox() : numEvaluations(1000), numRuns(100), maxExpressionSize(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    std::vector< std::pair<SolverPtr, String> > solvers;
    //solvers.push_back(std::make_pair(new RepeatSolver(rolloutSearchAlgorithm(), numEvaluations), "repeat(rollout)"));
    solvers.push_back(std::make_pair(stepLaSolver(1, 3), "step(1)la(3)"));
    solvers.push_back(std::make_pair(stepLaSolver(2, 3), "step(2)la(3)"));

    solvers.push_back(std::make_pair(nmcSolver(1), "nmc(1)"));
    solvers.push_back(std::make_pair(nmcSolver(2), "nmc(2)"));
    solvers.push_back(std::make_pair(nmcSolver(3), "nmc(3)"));

    solvers.push_back(std::make_pair(nrpaOptimizer(logLinearActionCodeSearchSampler(0.1, 1.0), 1, numEvaluations), "nrpa(1)"));
    solvers.push_back(std::make_pair(nrpaOptimizer(logLinearActionCodeSearchSampler(0.1, 1.0), 2, (size_t)pow((double)numEvaluations, 1.0/2.0)), "nrpa(2)"));
    solvers.push_back(std::make_pair(nrpaOptimizer(logLinearActionCodeSearchSampler(0.1, 1.0), 3, (size_t)pow((double)numEvaluations, 1.0/3.0)), "nrpa(3)"));
    //solvers.push_back(std::make_pair(nrpaOptimizer(logLinearActionCodeSearchSampler(0.1, 1.0), 4, (size_t)pow((double)numEvaluations, 1.0/4.0)), "nrpa(4)"));
    
    std::vector<SolverInfo> infos(solvers.size());
    context.enterScope("Running");
    for (size_t i = 0; i < solvers.size(); ++i)
    {
      context.enterScope(solvers[i].second);
      infos[i].name = solvers[i].second;
      runSolver(context, solvers[i].first, infos[i]);
      context.leaveScope(infos[i].fitnessPerEvaluationCount.back());
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

  ProblemPtr problem;
  size_t numEvaluations;
  size_t numRuns;
  size_t maxExpressionSize;
  
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

  SolverPtr nmcSolver(size_t level) const
  {
    jassert(level >= 1);
    return new RepeatSolver(nmcSolverRec(level));
  }

  SolverPtr nmcSolverRec(size_t level) const
  {
    if (level == 1)
      return rolloutSearchAlgorithm();
    else
      return stepSearchAlgorithm(lookAheadSearchAlgorithm(nmcSolverRec(level - 1)));
  }

  SolverPtr stepLaSolver(size_t numSteps, size_t numLookAheads) const
  {
    SolverPtr res = rolloutSearchAlgorithm();
    for (size_t i = 0; i < numLookAheads; ++i)
      res = lookAheadSearchAlgorithm(res);
    for (size_t i = 0; i < numSteps; ++i)
      res = stepSearchAlgorithm(res);
    return new RepeatSolver(res);
  }

  void runSolver(ExecutionContext& context, SolverPtr solver, SolverInfo& info)
  {
    std::vector<SolverInfo> runInfos(numRuns);
    size_t longest1 = 0, longest2 = 0;
    for (size_t i = 0; i < numRuns; ++i)
    {
      runSolverOnce(context, solver, runInfos[i]);
      if (runInfos[i].fitnessPerEvaluationCount.size() > longest1)
        longest1 = runInfos[i].fitnessPerEvaluationCount.size();
      if (runInfos[i].fitnessPerCpuTime.size() > longest2)
        longest2 = runInfos[i].fitnessPerCpuTime.size();
      context.progressCallback(new ProgressionState(i+1, numRuns, "Runs"));
    }

    info.fitnessPerEvaluationCount.resize(longest1);
    mergeResults(info.fitnessPerEvaluationCount, runInfos, false);
    info.fitnessPerCpuTime.resize(longest2);
    mergeResults(info.fitnessPerCpuTime, runInfos, true);
  }
  
  void runSolverOnce(ExecutionContext& context, SolverPtr solver, SolverInfo& info)
  {
    MCGPEvaluationDecoratorProblemPtr decoratedProblem = new MCGPEvaluationDecoratorProblem(new ExpressionToExpressionRPNProblem(problem, maxExpressionSize), numEvaluations);
    solver->optimize(context, decoratedProblem);
    info.fitnessPerEvaluationCount = decoratedProblem->getFitnessPerEvaluationCount();
    info.fitnessPerCpuTime = decoratedProblem->getFitnessPerCpuTime();
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
    size_t longestLength = 0;
    for (size_t i = 0; i < infos.size(); ++i)
    {
      size_t length = infos[i].getResults(inFunctionOfCpuTime).size();
      if (length > longestLength)
        longestLength = length;
    }
    size_t x = 4;
    for (size_t i = 2; i < longestLength; ++i)
    {
      context.enterScope(String((int)x));
      context.resultCallback("log2(x)", i);
      for (size_t j = 0; j < infos.size(); ++j)
        context.resultCallback(infos[j].name, infos[j].getResult(inFunctionOfCpuTime, i));
      context.leaveScope();
      x *= 2;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_SANDBOX_H_
