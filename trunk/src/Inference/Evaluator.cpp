/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.cpp                  | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Evaluator.h>

#include "Evaluator/ObjectContainerEvaluator.h"
#include "Evaluator/ClassificationAccuracyEvaluator.h"
#include "Evaluator/BinaryClassificationConfusionEvaluator.h"
#include "Evaluator/RegressionErrorEvaluator.h"
using namespace lbcpp;

EvaluatorPtr lbcpp::classificationAccuracyEvaluator(const String& name)
  {return new ClassificationAccuracyEvaluator(name);}

EvaluatorPtr lbcpp::binaryClassificationConfusionEvaluator(const String& name)
  {return new BinaryClassificationConfusionEvaluator(name);}

EvaluatorPtr lbcpp::regressionErrorEvaluator(const String& name)
  {return new RegressionErrorEvaluator(name);}

EvaluatorPtr lbcpp::dihedralRegressionErrorEvaluator(const String& name)
  {return new DihedralAngleRegressionErrorEvaluator(name);}

EvaluatorPtr lbcpp::objectContainerEvaluator(const String& name, EvaluatorPtr objectEvaluator)
  {return new ObjectContainerEvaluator(name, objectEvaluator);}


/*
** RegressionErrorEvaluator
*/
RegressionErrorEvaluator::RegressionErrorEvaluator(const String& name)
  : Evaluator(name), absoluteError(new ScalarVariableMean()), squaredError(new ScalarVariableMean())
{
}
  
void RegressionErrorEvaluator::addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
{
  ScalarPtr predicted = predictedObject.dynamicCast<Scalar>();
  ScalarPtr correct = correctObject.dynamicCast<Scalar>();
  if (predicted && correct)
    addDelta(predicted->getValue() - correct->getValue());
}

void RegressionErrorEvaluator::addDelta(double delta)
{
  absoluteError->push(fabs(delta));
  squaredError->push(delta * delta);
}

String RegressionErrorEvaluator::toString() const
{
  double count = squaredError->getCount();
  if (!count)
    return String::empty;
  return getName() + T(": rmse = ") + String(getRMSE(), 4)
      + T(" abs = ") + String(absoluteError->getMean(), 4)
      + T(" (") + lbcpp::toString(count) + T(" examples)");
}

double RegressionErrorEvaluator::getRMSE() const
{
  return sqrt(squaredError->getMean());
}
