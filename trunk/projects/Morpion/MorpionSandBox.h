/*-----------------------------------------.---------------------------------.
| Filename: MorpionSandBox.h               | Morpion Solitaire SandBox       |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2012 12:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MORPION_SANDBOX_H_
# define LBCPP_MORPION_SANDBOX_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/SolutionContainer.h>
# include <ml/RandomVariable.h>
# include "MorpionProblem.h"

namespace lbcpp
{

class CompareSolversWorkUnit : public WorkUnit
{
public:
  CompareSolversWorkUnit() : numEvaluations(1000), numRuns(10), verbosity(1) {}

  virtual ProblemPtr createProblem(ExecutionContext& context) = 0;
  virtual void initializeSolvers(ExecutionContext& context, ProblemPtr problem) = 0;

  virtual ObjectPtr run(ExecutionContext& context)
  {
    solvers.clear();
    problem = createProblem(context);
    initializeSolvers(context, problem);
    for (size_t i = 0; i < solvers.size(); ++i)
    {
      context.enterScope(solvers[i].first);
      ScalarVariableStatisticsPtr stats = runSolver(context, problem, solvers[i].second);
      context.leaveScope(stats);
    }
    return ObjectPtr();
  }

protected:
  friend class CompareSolversWorkUnitClass;

  size_t numEvaluations;
  size_t numRuns;
  size_t verbosity;

  ProblemPtr problem;
  std::vector< std::pair<string, SolverPtr> > solvers;

  void addSolver(const string& name, const SolverPtr& solver)
    {solvers.push_back(std::make_pair(name, solver));}

  ScalarVariableStatisticsPtr runSolver(ExecutionContext& context, ProblemPtr problem, SolverPtr solver)
  {
    ScalarVariableStatisticsPtr results = new ScalarVariableStatistics("results");
    for (size_t run = 0; run < numRuns; ++run)
    {
      context.enterScope("Run " + string((int)run+1));
      ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
      SolverCallbackPtr callback = compositeSolverCallback(fillParetoFrontSolverCallback(front), maxEvaluationsSolverCallback(numEvaluations));
      solver->setVerbosity((SolverVerbosity)verbosity);
      solver->solve(context, problem, callback);
      context.resultCallback("front", front);
      double result = front->getFitness(0)->getValue(0);
      results->push(result);
      context.leaveScope(result);
      if (numRuns > 1)
        context.progressCallback(new ProgressionState(run+1, numRuns, "Runs"));
    }
    return results;
  }
};

class MorpionSandBox : public CompareSolversWorkUnit
{
public:
  virtual ProblemPtr createProblem(ExecutionContext& context)
    {return new MorpionProblem(5, true);}

  virtual void initializeSolvers(ExecutionContext& context, ProblemPtr problem)
  {
    SearchActionCodeGeneratorPtr codeGenerator = new MorpionActionCodeGenerator();
    SamplerPtr sampler = logLinearActionCodeSearchSampler(codeGenerator, 0.1, 1.0);
    
    addSolver("random", randomSolver(sampler, numEvaluations));
    //addSolver("nrpa1", repeatSolver(nrpaSolver(sampler, 1, 20)));
    addSolver("nrpa2", repeatSolver(nrpaSolver(sampler, 2, 20)));
    //addSolver("nrpa3", repeatSolver(nrpaSolver(sampler, 3, 20)));
    //addSolver("bnrpa1-8", repeatSolver(beamNRPASolver(sampler, 1, 20, 8, 1)));
    addSolver("bnrpa2-2", repeatSolver(beamNRPASolver(sampler, 2, 20, 2, 1)));
    addSolver("bnrpa2-4", repeatSolver(beamNRPASolver(sampler, 2, 20, 4, 1)));
    addSolver("bnrpa2-8", repeatSolver(beamNRPASolver(sampler, 2, 20, 8, 1)));
    addSolver("bnrpa2-16", repeatSolver(beamNRPASolver(sampler, 2, 20, 16, 1)));
    //addSolver("bnrpa3-8", repeatSolver(beamNRPASolver(sampler, 3, 20, 8, 1)));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MORPION_SANDBOX_H_
