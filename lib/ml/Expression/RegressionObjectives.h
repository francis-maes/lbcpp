/*-----------------------------------------.---------------------------------.
| Filename: RegressionObjectives.h         | Regression Objectives           |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 13:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_REGRESSION_OBJECTIVES_H_
# define ML_EXPRESSION_REGRESSION_OBJECTIVES_H_

# include <ml/Objective.h>

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

  virtual double evaluatePredictions(ExecutionContext& context, DataVectorPtr predictions) const
  {
    DVectorPtr supervisions = getSupervisions().staticCast<DVector>();
    
    // compute mean absolute error
    double squaredError = 0.0;
    bool areDoubles = predictions->getElementsType()->inheritsFrom(doubleClass);
    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      double prediction =  (areDoubles ? it.getRawDouble() : it.getRawObject()->toDouble());
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

  virtual double evaluatePredictions(ExecutionContext& context, DataVectorPtr predictions) const
    {return sqrt(MSERegressionObjective::evaluatePredictions(context, predictions));}
};

class NormalizedRMSERegressionObjective : public RMSERegressionObjective
{
public:
  NormalizedRMSERegressionObjective(TablePtr data, VariableExpressionPtr supervision)
    : RMSERegressionObjective(data, supervision) {}
  NormalizedRMSERegressionObjective() {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}

  virtual double evaluatePredictions(ExecutionContext& context, DataVectorPtr predictions) const
  {
    double rmse = RMSERegressionObjective::evaluatePredictions(context, predictions);
    return 1.0 / (1.0 + rmse);
  }
};

/** The Root Relative Squared Error
 */
class RRSERegressionObjective : public SupervisedLearningObjective
{
public:
  RRSERegressionObjective(TablePtr data, VariableExpressionPtr supervision)
    {configure(data, supervision);}

  RRSERegressionObjective() {}

  virtual double evaluatePredictions(ExecutionContext& context, DataVectorPtr predictions) const
  {
    double sumSquaredError = 0.0;
    double meanTarget = 0.0;
    double sumSquaredErrorsFromMean = 0.0;
    
    DVectorPtr supervisions = getSupervisions().staticCast<DVector>();
    
    // compute mean absolute error
    bool areDoubles = predictions->getElementsType()->inheritsFrom(doubleClass);
    
    for (size_t i = 0; i < supervisions->getNumElements(); ++i)
      meanTarget += supervisions->get(i);
    meanTarget /= (double)supervisions->getNumElements();

    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      double prediction =  (areDoubles ? it.getRawDouble() : it.getRawObject()->toDouble());
      if (prediction == DVector::missingValue || !isNumberValid(prediction))
        prediction = 0.0;
      double delta = supervisions->get(it.getIndex()) - prediction;
      sumSquaredError += delta * delta;
      delta = supervisions->get(it.getIndex()) - meanTarget;
      sumSquaredErrorsFromMean += delta * delta;
    }
    
    return sqrt(sumSquaredError / sumSquaredErrorsFromMean);
  }
  
  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = DBL_MAX; best = 0.0;}
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_REGRESSION_OBJECTIVES_H_
