/*-----------------------------------------.---------------------------------.
| Filename: SurrogateQuarticSandBox.h      | Surrogate Quartic SandBox       |
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 15:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_SURROGATE_QUARTIC_SANDBOX_H_
# define LBCPP_MCGP_SURROGATE_QUARTIC_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp-ml/Solver.h>

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class SurrogateQuarticSandBox : public WorkUnit
{
public:
  SurrogateQuarticSandBox() : ensembleSize(100) {}
  
  SolverPtr createRegressionExtraTreeLearner()
  {
    SamplerPtr expressionVectorSampler = scalarExpressionVectorSampler();
    SolverPtr conditionLearner = randomSplitConditionLearner(expressionVectorSampler);
    //conditionLearner->setVerbosity((SolverVerbosity)verbosity);
    SolverPtr learner = treeLearner(stddevReductionSplittingCriterion(), conditionLearner); 
    //learner->setVerbosity((SolverVerbosity)verbosity);
    learner = simpleEnsembleLearner(learner, ensembleSize);
    learner->setVerbosity(verbosityDetailed);
    return learner;
  }
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    ProblemPtr problem = makeProblem(context);
    SolverPtr ceSolver = crossEntropySolver(diagonalGaussianSampler(), 100, 30, 50);

    context.enterScope("Solve with Cross Entropy");
    ObjectPtr solution;
    ceSolver->setVerbosity(verbosityDetailed);
    ceSolver->solve(context, problem, storeBestSolutionSolverCallback(solution));
    context.resultCallback("solution", solution);
    context.leaveScope();
    
    context.enterScope("Solve with Surrogate based");
    SolverPtr sbSolver = surrogateBasedSolver(uniformScalarVectorSampler(), 20, createRegressionExtraTreeLearner(), ceSolver, 5000);
    sbSolver->setVerbosity(verbosityDetailed);
    sbSolver->solve(context, problem, storeBestSolutionSolverCallback(solution));
    context.resultCallback("solution2", solution);
    context.leaveScope();
    
    return new Boolean(true);
  }
  
private:
  friend class SurrogateQuarticSandBoxClass;
  
  size_t ensembleSize;
  
  struct Obj : public Objective
  {
    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = 0.0; best = 1.0;}
  
    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
    {
      double err = 0.0;
      for (size_t i = 0; i < 21; ++i)
      {
        double x = -1.0 + i / 10.0;
        double y = x * (x * (x * (x + 1.0) + 1.0) + 1.0);
        double p = object.staticCast<DenseDoubleVector>()->getValue(i);
        err += (y - p) * (y - p);
      }
      err = sqrt(err / 21.0);
      return 1.0 / (1.0 + err);
    }
  };

  ProblemPtr makeProblem(ExecutionContext& context)
  {
    ProblemPtr res = new Problem();
    ScalarVectorDomainPtr domain = new ScalarVectorDomain();
    for (size_t i = 0; i < 21; ++i)
      domain->addDimension(-5.0, 5.0);
    res->setDomain(domain);
    res->addObjective(new Obj());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_MCGP_REGRESSION_SANDBOX_H_
