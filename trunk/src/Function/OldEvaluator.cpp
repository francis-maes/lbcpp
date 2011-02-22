/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.cpp                  | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/OldEvaluator.h>
#include <lbcpp/Distribution/ContinuousDistribution.h>
#include <lbcpp/NumericalLearning/LossFunctions.h>
using namespace lbcpp;

/*
** OldRegressionErrorEvaluator
*/
OldRegressionErrorEvaluator::OldRegressionErrorEvaluator(const String& name)
  : OldEvaluator(name), absoluteError(new ScalarVariableMean()), squaredError(new ScalarVariableMean())
{
}
  
void OldRegressionErrorEvaluator::addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct)
{
  if (!predicted.exists() || !correct.exists())
    return;
  
  double predictedValue;
  if (predicted.isDouble())
    predictedValue = predicted.getDouble();
  else if (predicted.isObject())
  {
    GaussianDistributionPtr distribution = predicted.dynamicCast<GaussianDistribution>();
    if (distribution)
      predictedValue = distribution->getMean();
    else
      jassert(false);
  }

  double correctValue;
  if (correct.isDouble())
    correctValue = correct.getDouble();
  else
  {
   // jassert(correct.dynamicCast<ScalarFunction>());
   // correct.getObjectAndCast<ScalarFunction>()->
    // CRACK ! : il faut des RegressionLoss pour pouvoir faire ça
  }

  addDelta(predictedValue - correct.getDouble());
}

void OldRegressionErrorEvaluator::addDelta(double delta)
{
  absoluteError->push(fabs(delta));
  squaredError->push(delta * delta);
}

String OldRegressionErrorEvaluator::toString() const
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

void OldRegressionErrorEvaluator::getScores(std::vector< std::pair<String, double> >& res) const
{
  res.push_back(std::make_pair(T("LS"),   -squaredError->getSum()));
  res.push_back(std::make_pair(T("MSE"),  -squaredError->getMean()));
  res.push_back(std::make_pair(T("RMSE"), -getRMSE()));
  res.push_back(std::make_pair(T("AbsE"), -absoluteError->getMean()));
}

double OldRegressionErrorEvaluator::getRMSE() const
{
  return sqrt(squaredError->getMean());
}

void OldRegressionErrorEvaluator::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  const OldRegressionErrorEvaluatorPtr& t = target.staticCast<OldRegressionErrorEvaluator>();
  t->absoluteError = new ScalarVariableMean();
  t->squaredError = new ScalarVariableMean();
}
