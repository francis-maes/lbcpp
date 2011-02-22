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

class RegressionErrorScoreObject : public ScoreObject
{
public:
  RegressionErrorScoreObject()
    : absoluteError(new ScalarVariableMean()),
      squaredError(new ScalarVariableMean()) {}
  
  virtual double getScoreToMinimize() const
    {return getRMSE();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
  {
    res.push_back(std::make_pair(T("LS"),   squaredError->getSum()));
    res.push_back(std::make_pair(T("MSE"),  squaredError->getMean()));
    res.push_back(std::make_pair(T("RMSE"), getRMSE()));
    res.push_back(std::make_pair(T("AbsE"), absoluteError->getMean()));
  }

  void addDelta(double delta)
  {
    absoluteError->push(fabs(delta));
    squaredError->push(delta * delta);
  }

  double getRMSE() const
    {return sqrt(squaredError->getMean());}

  virtual String toString() const
  {
    double count = squaredError->getCount();
    if (!count)
      return String::empty;
    return getName()
    + T(": Least squares = ") + String(squaredError->getSum(), 4)
    + T(", MSE = ") + String(squaredError->getMean(), 4)
    + T(", RMSE = ") + String(getRMSE(), 4)
    + T(", ABS = ") + String(absoluteError->getMean(), 4)
    + T(" (") + String((int)count) + T(" examples)");
  }

protected:
  friend class RegressionErrorScoreObjectClass;
  
  ScalarVariableMeanPtr absoluteError;
  ScalarVariableMeanPtr squaredError;
};

class RegressionErrorEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictedElementsType() const
    {return doubleType;}
  
  virtual TypePtr getRequiredSupervisionElementsType() const
    {return doubleType;}
  
protected:  
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {return new RegressionErrorScoreObject();}

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
    {result.staticCast<RegressionErrorScoreObject>()->addDelta(predictedObject.getDouble() - correctObject.getDouble());}
};

class DihedralRegressionErrorEvaluator : public RegressionErrorEvaluator
{
protected:
  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
    {result.staticCast<RegressionErrorScoreObject>()->addDelta(normalizeAngle(predictedObject.getDouble() - correctObject.getDouble()));}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_REGRESSION_ERROR_H_
