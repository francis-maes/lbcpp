/*-----------------------------------------.---------------------------------.
| Filename: RegressionErrorEvaluator.h     | Regression Error Evaluator      |
| Author  : Julien Becker                  |                                 |
| Started : 22/02/2011 11:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_REGRESSION_ERROR_H_
# define LBCPP_FUNCTION_EVALUATOR_REGRESSION_ERROR_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class RegressionScoreObject : public ScoreObject
{
public:
  RegressionScoreObject()
    : absoluteError(new ScalarVariableMean()),
      squaredError(new ScalarVariableMean()),
      leastSquares(0.0),
      meanSquareError(0.0),
      rootMeanSquareError(0.0),
      absolute(0.0){}
  
  virtual double getScoreToMinimize() const
    {return rootMeanSquareError;}

  virtual void finalize()
  {
    leastSquares = squaredError->getSum();
    meanSquareError = squaredError->getMean();
    rootMeanSquareError = sqrt(squaredError->getMean());
    absolute = absoluteError->getMean();
  }

  void addDelta(double delta)
  {
    absoluteError->push(fabs(delta));
    squaredError->push(delta * delta);
  }

  virtual String toString() const
  {
    double count = squaredError->getCount();
    if (!count)
      return String::empty;
    return getName()
    + T(": Least squares = ") + String(leastSquares, 4)
    + T(", MSE = ") + String(meanSquareError, 4)
    + T(", RMSE = ") + String(rootMeanSquareError, 4)
    + T(", ABS = ") + String(absolute, 4)
    + T(" (") + String((int)count) + T(" examples)");
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    ScoreObject::clone(context, target);
    ReferenceCountedObjectPtr<RegressionScoreObject> res = target.staticCast<RegressionScoreObject>();
    if (absoluteError)
      res->absoluteError = absoluteError->cloneAndCast<ScalarVariableMean>(context);
    if (squaredError)
      res->squaredError = squaredError->cloneAndCast<ScalarVariableMean>(context);
  }

protected:
  friend class RegressionScoreObjectClass;
  
  ScalarVariableMeanPtr absoluteError;
  ScalarVariableMeanPtr squaredError;
  
  double leastSquares;
  double meanSquareError;
  double rootMeanSquareError;
  double absolute;
};

class RegressionEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return doubleType;}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return doubleType;}
  
protected:  
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {return new RegressionScoreObject();}
  
  virtual void finalizeScoreObject(ScoreObjectPtr& score) const
    {score.staticCast<RegressionScoreObject>()->finalize();}


  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
    {result.staticCast<RegressionScoreObject>()->addDelta(predictedObject.getDouble() - correctObject.getDouble());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_REGRESSION_ERROR_H_
