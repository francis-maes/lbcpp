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
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/BanditPool.h>
# include "MCGPSandBox.h"

namespace lbcpp
{

class OptimizeExpressionSamplerProblem : public ContinuousProblem
{
public:
  OptimizeExpressionSamplerProblem(SearchActionCodeGeneratorPtr codeGenerator, ProblemPtr targetProblem)
    : codeGenerator(codeGenerator), targetProblem(targetProblem)
  {
    size_t numCodes = codeGenerator->getNumCodes(targetProblem->getDomain().staticCast<SearchDomain>()->getInitialState());
    domain = new ContinuousDomain(std::vector< std::pair<double, double> >(numCodes, std::make_pair(-5.0, 5.0)));
    limits = targetProblem->getFitnessLimits();
  }
  OptimizeExpressionSamplerProblem() {}
   
  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    DenseDoubleVectorPtr parameters = object.staticCast<DenseDoubleVector>();
    SamplerPtr sampler = logLinearActionCodeSearchSampler(codeGenerator);
    sampler->setVariable(sampler->getClass()->findMemberVariable("parameters"), parameters);
    sampler->initialize(context, targetProblem->getDomain());
    SolverPtr solver = stepSearchAlgorithm(lookAheadSearchAlgorithm(rolloutSearchAlgorithm(sampler)));
    SolutionVectorPtr solutions = solver->optimize(context, targetProblem);
    return solutions->getFitness(0);
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

  virtual Variable run(ExecutionContext& context)
  {
    ExpressionDomainPtr domain = problem->getDomain();
    context.informationCallback(domain->toShortString());
    ProblemPtr searchProblem = new ExpressionToSearchProblem(problem, maxExpressionSize, false);

    SearchActionCodeGeneratorPtr codeGenerator = new NGramExpressionSearchActionCodeGenerator(2);
    ProblemPtr samplerProblem = new OptimizeExpressionSamplerProblem(codeGenerator, searchProblem);

    SolverPtr samplerSolver = crossEntropySolver(diagonalGaussianSampler(), 1000, 300, 10, false);

    context.enterScope("Optimizing sampler");
    SolutionContainerPtr samplerSolutions = samplerSolver->optimize(context, samplerProblem, ObjectPtr(), Solver::verbosityDetailed);
    DenseDoubleVectorPtr samplerParameters = samplerSolutions->getSolution(0).staticCast<DenseDoubleVector>();
    context.resultCallback("samplerParameters", samplerParameters);
    context.leaveScope();

    testSamplerParameters(context, codeGenerator, new DenseDoubleVector(1, 0.0));
    testSamplerParameters(context, codeGenerator, samplerParameters);
    return true;
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
      ProblemPtr decoratedProblem = new MCGPEvaluationDecoratorProblem(searchProblem, numEvaluations);
      SolutionContainerPtr solutions = finalSolver->optimize(context, decoratedProblem);
      double score = solutions->getFitness(0)->getValue(0);
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
