/*-----------------------------------------.---------------------------------.
| Filename: OptimizeExpressionSampler.h    | Optimize Expression Sampler     |
| Author  : Francis Maes                   |                                 |
| Started : 23/10/2012 19:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_OPTIMIZE_EXPRESSION_SAMPLER_H_
# define LBCPP_MCGP_OPTIMIZE_EXPRESSION_SAMPLER_H_

# include <lbcpp-ml/ExpressionDomain.h>
# include <lbcpp-ml/BanditPool.h>
# include <lbcpp/Execution/WorkUnit.h>
# include "MCGPSandBox.h"

namespace lbcpp
{

class OptimizeExpressionSamplerObjective : public Objective
{
public:
  OptimizeExpressionSamplerObjective(SamplerPtr sampler, ProblemPtr targetProblem) 
    : sampler(sampler), targetProblem(targetProblem) {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {targetProblem->getObjective(0)->getObjectiveRange(worst, best);}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    DenseDoubleVectorPtr parameters = object.staticCast<DenseDoubleVector>();
    SamplerPtr sampler = this->sampler->cloneAndCast<Sampler>();
    sampler->setVariable(sampler->getClass()->findMemberVariable("parameters"), parameters);
    sampler->initialize(context, targetProblem->getDomain());
    SolverPtr solver = stepSearchAlgorithm(lookAheadSearchAlgorithm(rolloutSearchAlgorithm(sampler)));
    FitnessPtr bestFitness;
    solver->solve(context, targetProblem, storeBestFitnessSolverCallback(bestFitness));
    return bestFitness;
  }

protected:
  SamplerPtr sampler;
  ProblemPtr targetProblem;
};

class OptimizeExpressionSamplerProblem : public Problem
{
public:
  OptimizeExpressionSamplerProblem(SearchActionCodeGeneratorPtr codeGenerator, ProblemPtr targetProblem)
    : codeGenerator(codeGenerator), targetProblem(targetProblem)
    {initialize(defaultExecutionContext());}
  OptimizeExpressionSamplerProblem() {}
   
  virtual void initialize(ExecutionContext& context)
  {
    size_t numCodes = codeGenerator->getNumCodes(targetProblem->getDomain().staticCast<SearchDomain>()->getInitialState());
    setDomain(new ContinuousDomain(std::vector< std::pair<double, double> >(numCodes, std::make_pair(-5.0, 5.0))));
    addObjective(new OptimizeExpressionSamplerObjective(logLinearActionCodeSearchSampler(codeGenerator), targetProblem));
  }

protected:
  friend class OptimizeExpressionSamplerProblemClass;

  SearchActionCodeGeneratorPtr codeGenerator;
  ProblemPtr targetProblem;
};

class OptimizeExpressionSamplerSandBox : public WorkUnit
{
public:
  OptimizeExpressionSamplerSandBox() : numEvaluations(10000), maxExpressionSize(20) {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    ExpressionDomainPtr domain = problem->getDomain();
    context.informationCallback(domain->toShortString());
    ProblemPtr searchProblem = new ExpressionToSearchProblem(problem, maxExpressionSize, false);

    SearchActionCodeGeneratorPtr codeGenerator = new NGramExpressionSearchActionCodeGenerator(2);
    ProblemPtr samplerProblem = new OptimizeExpressionSamplerProblem(codeGenerator, searchProblem);

    SolverPtr samplerSolver = crossEntropySolver(diagonalGaussianSampler(), 1000, 300, 10, false);

    context.enterScope("Optimizing sampler");
    samplerSolver->setVerbosity(verbosityDetailed);
    ParetoFrontPtr samplerSolutions = new ParetoFront();
    samplerSolver->solve(context, samplerProblem, fillParetoFrontSolverCallback(samplerSolutions));
    DenseDoubleVectorPtr samplerParameters = samplerSolutions->getSolution(0).staticCast<DenseDoubleVector>();
    context.resultCallback("samplerParameters", samplerParameters);
    context.leaveScope();

    testSamplerParameters(context, codeGenerator, new DenseDoubleVector(1, 0.0));
    testSamplerParameters(context, codeGenerator, samplerParameters);
    return ObjectPtr();
  }

  void testSamplerParameters(ExecutionContext& context, SearchActionCodeGeneratorPtr codeGenerator, DenseDoubleVectorPtr parameters)
  {
    ProblemPtr searchProblem = new ExpressionToSearchProblem(problem, maxExpressionSize, false);

    SamplerPtr sampler = logLinearActionCodeSearchSampler(codeGenerator);
    sampler->setVariable(sampler->getClass()->findMemberVariable("parameters"), parameters);
    sampler->initialize(context, searchProblem->getDomain());

    SolverPtr finalSolver = repeatSolver(stepSearchAlgorithm(lookAheadSearchAlgorithm(rolloutSearchAlgorithm(sampler))));
    //SolverPtr finalSolver = randomSolver(sampler, numEvaluations);

    context.enterScope("Testing sampler");
    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("stats");
    for (size_t i = 0; i < 10; ++i)
    {
      FitnessPtr bestFitness;
      finalSolver->solve(context, searchProblem,
        compositeSolverCallback(storeBestFitnessSolverCallback(bestFitness), maxEvaluationsSolverCallback(numEvaluations)));
      double score = bestFitness->getValue(0);
      stats->push(score);
      context.progressCallback(new ProgressionState(i+1, 10, "Runs"));
    }
    context.leaveScope(stats);
  }

protected:
  friend class OptimizeExpressionSamplerSandBoxClass;

  ProblemPtr problem;
  size_t numEvaluations;
  size_t maxExpressionSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_OPTIMIZE_EXPRESSION_SAMPLER_H_
