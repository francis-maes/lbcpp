/*-----------------------------------------.---------------------------------.
| Filename: RegressionObjectives.h         | Regression Objectives           |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 13:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_REGRESSION_OBJECTIVES_H_
# define LBCPP_ML_EXPRESSION_REGRESSION_OBJECTIVES_H_

# include <lbcpp-ml/Objective.h>

namespace lbcpp
{

class MSERegressionObjective : public SupervisedLearningObjective
{
public:
  MSERegressionObjective(TablePtr data, VariableExpressionPtr supervision)
    {configure(data, supervision);}
  MSERegressionObjective() {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = DBL_MAX; best = 0.0;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
     // retrieve predictions and supervisions
    ExpressionPtr expression = object.staticCast<Expression>();
    DataVectorPtr predictions = computePredictions(context, expression);
    DVectorPtr supervisions = getSupervisions().staticCast<DVector>();
    
    // compute mean absolute error
    double squaredError = 0.0;
    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      double prediction = it.getRawDouble();
      if (prediction == DVector::missingValue || !isNumberValid(prediction))
        prediction = 0.0;
      double delta = supervisions->get(it.getIndex()) - prediction;
      squaredError += delta * delta;
    }
    // mean squared error
    return squaredError / (double)supervisions->getNumElements();
  }
};

class RMSERegressionObjective : public MSERegressionObjective
{
public:
  RMSERegressionObjective(TablePtr data, VariableExpressionPtr supervision)
    : MSERegressionObjective(data, supervision) {}
  RMSERegressionObjective() {}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
    {return sqrt(MSERegressionObjective::evaluate(context, object));}
};

class NormalizedRMSERegressionObjective : public RMSERegressionObjective
{
public:
  NormalizedRMSERegressionObjective(TablePtr data, VariableExpressionPtr supervision)
    : RMSERegressionObjective(data, supervision) {}
  NormalizedRMSERegressionObjective() {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    double rmse = RMSERegressionObjective::evaluate(context, object);
    return 1.0 / (1.0 + rmse);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_REGRESSION_OBJECTIVES_H_
