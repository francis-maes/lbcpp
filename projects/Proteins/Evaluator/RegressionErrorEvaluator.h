/*-----------------------------------------.---------------------------------.
| Filename: RegressionErrorEvaluator.h     | Regression Error Evaluator      |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_REGRESSION_ERROR_H_
# define LBCPP_EVALUATOR_REGRESSION_ERROR_H_

# include "Evaluator.h"
# include "../Geometry/DihedralAngle.h"

namespace lbcpp
{

class RegressionErrorEvaluator : public Evaluator
{
public:
  RegressionErrorEvaluator(const String& name) : Evaluator(name),
    absoluteError(new ScalarVariableMean()), squaredError(new ScalarVariableMean()) {}
  RegressionErrorEvaluator() {}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
  {
    ScalarPtr predicted = predictedObject.dynamicCast<Scalar>();
    ScalarPtr correct = correctObject.dynamicCast<Scalar>();
    if (predicted && correct)
      addDelta(predicted->getValue() - correct->getValue());
  }

  virtual void addDelta(double delta)
  {
    absoluteError->push(fabs(delta));
    squaredError->push(delta * delta);
  }

  virtual String toString() const
  {
    double count = squaredError->getCount();
    if (!count)
      return String::empty;
    return getName() + T(": rmse = ") + String(getRMSE(), 4)
        + T(" abs = ") + String(absoluteError->getMean(), 4)
        + T(" (") + lbcpp::toString(count) + T(" examples)");
  }

  virtual double getDefaultScore() const
    {return -getRMSE();}

  double getRMSE() const
    {return sqrt(squaredError->getMean());}

protected:
  ScalarVariableMeanPtr absoluteError;
  ScalarVariableMeanPtr squaredError;
};

typedef ReferenceCountedObjectPtr<RegressionErrorEvaluator> RegressionErrorEvaluatorPtr;

class DihedralAngleRegressionErrorEvaluator : public RegressionErrorEvaluator
{
public:
  DihedralAngleRegressionErrorEvaluator(const String& name) : RegressionErrorEvaluator(name) {}

  virtual void addDelta(double delta)
    {RegressionErrorEvaluator::addDelta(DihedralAngle::normalize(delta));}
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_REGRESSION_ERROR_H_
