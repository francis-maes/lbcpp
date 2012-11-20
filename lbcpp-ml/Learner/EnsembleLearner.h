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
    std::pair<AggregatorPtr, ClassPtr> aggregatorAndOutputType = createAggregator(supervisionType);
    AggregatorExpressionPtr res = new AggregatorExpression(aggregatorAndOutputType.first, aggregatorAndOutputType.second);
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
        res->pushNode(expression);

      if (verbosity >= verbosityDetailed)
      {
        // FIXME: incremental calculation of these quantities
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

  std::pair<AggregatorPtr, ClassPtr> createAggregator(ClassPtr supervisionType)
  {
    if (supervisionType.isInstanceOf<Enumeration>())
      return std::make_pair(meanDoubleVectorAggregator(), denseDoubleVectorClass(supervisionType.staticCast<Enumeration>(), doubleClass));
    else
    {
      jassertfalse; // not implemented yet
      return std::make_pair(AggregatorPtr(), ClassPtr());
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
