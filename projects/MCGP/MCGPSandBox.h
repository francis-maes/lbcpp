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

namespace lbcpp
{

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

class ExpressionToSearchProblem : public DecoratorProblem
{
public:
  ExpressionToSearchProblem(ExpressionProblemPtr expressionProblem, size_t maxSize, bool usePostfixNotation, size_t actionCodeGenerator)
    : DecoratorProblem(expressionProblem)
  {
    ExpressionDomainPtr expressionDomain = expressionProblem->getDomain().staticCast<ExpressionDomain>();
    SearchStatePtr initialState;
    if (usePostfixNotation)
      initialState = postfixExpressionState(expressionDomain, maxSize, new ExpressionActionCodeGenerator(actionCodeGenerator));
    else
      initialState = prefixExpressionState(expressionDomain, maxSize, new ExpressionActionCodeGenerator(actionCodeGenerator));
    
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
  
protected:
  SearchDomainPtr domain;
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
  MCGPSandBox() : numEvaluations(1000), numRuns(100), maxExpressionSize(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    std::vector< std::pair<SolverPtr, String> > solvers;

    solvers.push_back(std::make_pair(nmcSolver(0), "random"));
    solvers.push_back(std::make_pair(nmcSolver(1), "nmc1"));
    solvers.push_back(std::make_pair(nmcSolver(2), "nmc2"));
    solvers.push_back(std::make_pair(nmcSolver(3), "nmc3"));

    solvers.push_back(std::make_pair(nrpaSolver(1), "nrpa1"));
    solvers.push_back(std::make_pair(nrpaSolver(2), "nrpa2"));
    solvers.push_back(std::make_pair(nrpaSolver(3), "nrpa3"));
    
    //solvers.push_back(std::make_pair(ceSolver(100, 30, false, 0.1), "ce(100, 30, false, 0.1"));
    //solvers.push_back(std::make_pair(ceSolver(100, 30, true, 0.1), "ce(100, 30, true, 0.1"));

    //solvers.push_back(std::make_pair(stepLaSolver(1, 3), "step(1)la(3)"));
    //solvers.push_back(std::make_pair(stepLaSolver(2, 3), "step(2)la(3)"));
    
    std::vector<SolverInfo> infos;
    context.enterScope("Running");
    for (size_t i = 0; i < solvers.size(); ++i)
    {
      //size_t numCodeGenerators = (solvers[i].second.startsWith(T("nrpa")) || solvers[i].second.startsWith(T("ce"))) ? 4 : 1;
      //for (size_t j = 0; j < numCodeGenerators; ++j)
      {
        String name = solvers[i].second;
        //if (numCodeGenerators > 1)
        //  name += "-code" + String((int)j);
        infos.push_back(runSolver(context, solvers[i].first, name + "-prefix", false, 1)); // polish
        infos.push_back(runSolver(context, solvers[i].first, name + "-postfix", true, 1)); // reverse polish
      }
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
    SolverPtr res = rolloutSearchAlgorithm();
    for (size_t i = 0; i < level; ++i)
      res = stepSearchAlgorithm(lookAheadSearchAlgorithm(res));
    return new RepeatSolver(res);
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

  SolverPtr nrpaSolver(size_t level) const
    {return new RepeatSolver(nrpaOptimizer(logLinearActionCodeSearchSampler(0.1, 1.0), level, (size_t)pow((double)numEvaluations, 1.0 / level)));}

  SolverPtr ceSolver(size_t populationSize, size_t numTrainingSamples, bool elitist, double regularizer) const
    {return crossEntropyOptimizer(logLinearActionCodeSearchSampler(regularizer), populationSize, numTrainingSamples, numEvaluations / populationSize, elitist);}

  SolverInfo runSolver(ExecutionContext& context, SolverPtr solver, const String& description, bool usePostfixNotation, size_t actionCodeGenerator)
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
	  std::vector<SolverInfo> runInfos(numRuns);
    size_t shortest1 = (size_t)-1;
    size_t shortest2 = (size_t)-1;
    for (size_t i = 0; i < numRuns; ++i)
    {
      SolverInfo& info = runInfos[i];
      runSolverOnce(context, solver, info, usePostfixNotation, actionCodeGenerator);
      writeToOutput(evalsOutput, info.fitnessPerEvaluationCount);
      writeToOutput(timesOutput, info.fitnessPerCpuTime);
      if (info.fitnessPerEvaluationCount.size() < shortest1)
        shortest1 = info.fitnessPerEvaluationCount.size();
      if (info.fitnessPerCpuTime.size() < shortest2)
        shortest2 = info.fitnessPerCpuTime.size();
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

  void runSolverOnce(ExecutionContext& context, SolverPtr solver, SolverInfo& info, bool usePostfixNotation, size_t actionCodeGenerator)
  {
    problem->initialize(context); // reinitialize problem (necessary because some problems such as koza symbolic regression are indeed distributions over problems)
    MCGPEvaluationDecoratorProblemPtr decoratedProblem = new MCGPEvaluationDecoratorProblem(new ExpressionToSearchProblem(problem, maxExpressionSize, usePostfixNotation, actionCodeGenerator), numEvaluations);
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
