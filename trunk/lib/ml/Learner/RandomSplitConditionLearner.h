/*-----------------------------------------.---------------------------------.
| Filename: RandomSplitConditionLearner.h  | Random Split Condition Learner  |
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 14:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_LEARNER_RANDOM_SPLIT_CONDITION_H_
# define LBCPP_ML_LEARNER_RANDOM_SPLIT_CONDITION_H_

# include <ml/Solver.h>
# include <ml/Expression.h>
# include <ml/Sampler.h>
# include <ml/SplittingCriterion.h>
# include <algorithm>

namespace lbcpp
{

class RandomSplitConditionLearner : public Solver
{
public:
  RandomSplitConditionLearner(SamplerPtr expressionsSampler)
    : expressionsSampler(expressionsSampler) {}
  RandomSplitConditionLearner() {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    Solver::startSolver(context, problem, callback, startingSolution);
    expressionsSampler->initialize(context, vectorDomain(problem->getDomain()));
  }

  virtual void runSolver(ExecutionContext& context)
  {
    OVectorPtr expressions = expressionsSampler->sample(context).staticCast<OVector>();
    for (size_t i = 0; i < expressions->getNumElements(); ++i)
      evaluate(context, makeBooleanExpression(context, expressions->getElement(i).staticCast<Expression>()));
  }

protected:
  friend class RandomSplitConditionLearnerClass;

  SamplerPtr expressionsSampler;
  
  ExpressionPtr makeBooleanExpression(ExecutionContext& context, ExpressionPtr booleanOrScalar)
  {
    if (booleanOrScalar->getType() == booleanClass)
      return booleanOrScalar;
    else
    {
      jassert(booleanOrScalar->getType()->isConvertibleToDouble());
      LearningObjectivePtr objective = problem->getObjective(0).staticCast<LearningObjective>();
      DataVectorPtr values = objective->computePredictions(context, booleanOrScalar);
      double minValue = DBL_MAX;
      double maxValue = -DBL_MAX;
      for (DataVector::const_iterator it = values->begin(); it != values->end(); ++it)
      {
        double value = it.getRawDouble();
        if (value < minValue)
          minValue = value;
        if (value > maxValue)
          maxValue = value;
      }
      double threshold = context.getRandomGenerator()->sampleDouble(minValue, maxValue);
      return new FunctionExpression(stumpFunction(threshold), booleanOrScalar);
    }
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_ML_LEARNER_RANDOM_SPLIT_CONDITION_H_
