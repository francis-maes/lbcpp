/*-----------------------------------------.---------------------------------.
| Filename: EnsembleLearner.h              | Ensemble Learner                |
| Author  : Francis Maes                   |                                 |
| Started : 02/01/2012 17:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_LEARNER_ENSEMBLE_H_
# define LBCPP_ML_LEARNER_ENSEMBLE_H_

# include <lbcpp-ml/Solver.h>

namespace lbcpp
{

class EnsembleLearner : public Solver
{
public:
  EnsembleLearner(const SolverPtr& baseLearner, size_t ensembleSize)
    : baseLearner(baseLearner), ensembleSize(ensembleSize) {}
  EnsembleLearner() : ensembleSize(0) {}

  virtual void runSolver(ExecutionContext& context)
  {
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    ClassPtr supervisionType = objective->getSupervision()->getType();
    SequenceExpressionPtr res = createSequenceExpression(supervisionType); 
    res->reserveNodes(ensembleSize);

    for (size_t i = 0; i < ensembleSize; ++i)
    {
      if (verbosity >= verbosityDetailed)
      {
        context.enterScope(T("Iteration ") + string((int)i));
        context.resultCallback(T("iteration"), i);
      }
      IndexSetPtr indices = getSubIndices(context, i, objective->getIndices());

      ExpressionPtr expression;
      baseLearner->solve(context, problem, storeBestSolutionSolverCallback(*(ObjectPtr* )&expression));
      if (expression)
      {
        // FIXME: caches to update
        res->pushNode(context, expression);
      }

      if (verbosity >= verbosityDetailed)
      {
        double trainingScore = objective->evaluate(context, res);
        context.resultCallback("trainingScore", trainingScore);
        if (problem->getNumValidationObjectives())
          context.resultCallback("validationScore", problem->getValidationObjective(0)->evaluate(context, res));
        context.leaveScope();
      }
      if (verbosity >= verbosityProgressAndResult)
        context.progressCallback(new ProgressionState(i + 1, ensembleSize, T(" base models")));
    }

    evaluate(context, res);
  }

protected:
  friend class EnsembleLearnerClass;

  SolverPtr baseLearner;
  size_t ensembleSize;

  virtual IndexSetPtr getSubIndices(ExecutionContext& context, size_t modelIndex, const IndexSetPtr& indices) const
    {return indices;}

  SequenceExpressionPtr createSequenceExpression(ClassPtr supervisionType)
  {
    if (supervisionType.isInstanceOf<Enumeration>())
      return new VectorSumExpression(supervisionType.staticCast<Enumeration>(), false);
    else
    {
      jassertfalse; // not implemented yet
      return SequenceExpressionPtr();
    }
  }
};

class BaggingLearner : public EnsembleLearner
{
public:
  BaggingLearner(const SolverPtr& baseLearner, size_t ensembleSize)
    : EnsembleLearner(baseLearner, ensembleSize) {}
  BaggingLearner() {}

  virtual IndexSetPtr getSubIndices(ExecutionContext& context, size_t modelIndex, const IndexSetPtr& indices) const
    {return indices->sampleBootStrap(context.getRandomGenerator());}
};

}; /* namespace lbcpp */

#endif // LBCPP_ML_LEARNER_ENSEMBLE_H_
