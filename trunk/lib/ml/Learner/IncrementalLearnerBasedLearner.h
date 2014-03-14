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
    return true;
  }

  virtual void stopSolver(ExecutionContext& context)
  {
    evaluate(context, expression);
    IterativeSolver::stopSolver(context);
  }

private:
  friend class IncrementalLearnerBasedLearnerClass;

  IncrementalLearnerPtr learner;

  ExpressionPtr expression;
};

}; /* namespace lbcpp */

#endif // ML_LEARNER_INCREMENTAL_BASED_H_
