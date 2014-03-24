/*-----------------------------------------.---------------------------------.
| Filename: IncrementalLearnerBasedLearner.h | Transforms an incremental     |
| Author  : Francis Maes                   | learner into a batch one        |
| Started : 14/03/2013 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef ML_LEARNER_INCREMENTAL_BASED_H_
# define ML_LEARNER_INCREMENTAL_BASED_H_

# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>

namespace lbcpp
{

class IncrementalLearnerBasedLearner : public IterativeSolver
{
public:
  IncrementalLearnerBasedLearner(IncrementalLearnerPtr learner = IncrementalLearnerPtr())
    : learner(learner) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    SupervisedLearningObjectivePtr learningObjective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    numIterations = learningObjective->getIndices()->size();
    expression = learner->createExpression(context, learningObjective->getSupervision()->getType());
    learner->setVerbosity(verbosity);
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    SupervisedLearningObjectivePtr learningObjective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    TablePtr data = learningObjective->getData();
    size_t index = learningObjective->getIndices()->getIndices()[iter];
    learner->addTrainingSample(context, data->getRow(index), expression);
    if (verbosity >= verbosityDetailed)
      context.resultCallback("testing", problem->getValidationObjective(0)->evaluate(context, expression));
    if (verbosity >= verbosityAll)
      makeCurve(context, baseProblem, expression);
    return true;
  }

  virtual void stopSolver(ExecutionContext& context)
  {
    evaluate(context, expression);
    IterativeSolver::stopSolver(context);
  }
  
  ProblemPtr baseProblem;

private:
  friend class IncrementalLearnerBasedLearnerClass;

  IncrementalLearnerPtr learner;
  
  ExpressionPtr expression;
  
  void makeCurve(ExecutionContext& context, ProblemPtr baseProblem, ExpressionPtr expression)
  {
    //context.enterScope("Curve");
    //if (baseProblem->getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions() > 1) return;
    double x = 0.0;
    size_t curveSize = 200;
    std::vector<ObjectPtr> input = std::vector<ObjectPtr>(1);
    input[0] = new Double(0.0);
    DenseDoubleVectorPtr problemInput = new DenseDoubleVector(1, 0.0);
    //ScalarVectorDomainPtr domain = baseProblem->getDomain().staticCast<ScalarVectorDomain>();
    double range = 1.0;
    double offset = 0.0;
    for (size_t i = 0; i < curveSize; ++i)
    {
      x = offset + range * i / curveSize;
      context.enterScope(string(x));
      context.resultCallback("x", x);
      input[0].staticCast<Double>()->set(x);
      problemInput->setValue(0, x);
      context.resultCallback("supervision", baseProblem->evaluate(context, problemInput)->getValue(0));
      context.resultCallback("prediction", expression->compute(context, input));
      context.leaveScope();
    }
    //context.leaveScope();
  }
};

}; /* namespace lbcpp */

#endif // ML_LEARNER_INCREMENTAL_BASED_H_
