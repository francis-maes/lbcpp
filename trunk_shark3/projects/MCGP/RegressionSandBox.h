/*-----------------------------------------.---------------------------------.
| Filename: RegressionSandBox.h            | Regression SandBox              |
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 11:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef MCGP_REGRESSION_SANDBOX_H_
# define MCGP_REGRESSION_SANDBOX_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class RegressionSandBox : public WorkUnit
{
public:
  RegressionSandBox() {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);

    ProblemPtr problem = makeProblem(context);

    // make learner
    SamplerPtr expressionVectorSampler = scalarExpressionVectorSampler();
    SolverPtr conditionLearner = randomSplitConditionLearner(expressionVectorSampler);
    //conditionLearner->setVerbosity((SolverVerbosity)verbosity);
    SolverPtr learner = treeLearner(stddevReductionSplittingCriterion(), conditionLearner); 
    //learner->setVerbosity((SolverVerbosity)verbosity);
    learner = simpleEnsembleLearner(learner, 100);
    learner->setVerbosity(verbosityDetailed);

    // learn
    ExpressionPtr model;
    FitnessPtr fitness;
    context.enterScope("Learning");
    learner->solve(context, problem, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
    context.leaveScope();
    context.resultCallback("model", model);
    context.resultCallback("fitness", fitness);

    // evaluate
    double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
    context.resultCallback("testingScore", testingScore);

    // make curve
    context.enterScope("test");
    for (double x = -1.0; x <= 1.0; x += 0.01)
    {
      context.enterScope(string(x));
      context.resultCallback("x", x);
      context.resultCallback("supervision", x * x * x + x * x + x);
      
      std::vector<ObjectPtr> input(1, ObjectPtr(new Double(x)));
      ObjectPtr prediction = model->compute(context, input);
      
      context.resultCallback("prediction", prediction);
      context.leaveScope();
    }
    context.leaveScope();
    

    return new Boolean(true);
  }
  
private:  
  ProblemPtr makeProblem(ExecutionContext& context)
  {
    ProblemPtr res = new Problem();
    ExpressionDomainPtr domain = new ExpressionDomain();
    VariableExpressionPtr x = domain->addInput(doubleClass, "x");
    VariableExpressionPtr y = domain->createSupervision(doubleClass, "y");
    res->setDomain(domain);
    res->addObjective(normalizedRMSERegressionObjective(makeTable(21, x, y), y));
    res->addValidationObjective(normalizedRMSERegressionObjective(makeTable(101, x, y), y));
    return res;
  }
  
  TablePtr makeTable(size_t count, VariableExpressionPtr x, VariableExpressionPtr y)
  {
    TablePtr res = new Table(count);
    res->addColumn(x, doubleClass);
    res->addColumn(y, doubleClass);
    for (size_t i = 0; i < count; ++i)
    {
      double x = -1.0 + i * 2.0 / (count - 1);
      res->setElement(i, 0, new Double(x));
      double y = x * x * x + x * x + x;
      res->setElement(i, 1, new Double(y));
    }
    return res;
  }  
};

}; /* namespace lbcpp */

#endif // MCGP_REGRESSION_SANDBOX_H_
