/*-----------------------------------------.---------------------------------.
| Filename: Problem.cpp                    | Optimization Problem            |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Problem.h>
#include <lbcpp-ml/SolutionContainer.h>
using namespace lbcpp;

/*
** Objective
*/
bool DifferentiableObjective::testDerivativeWithRandomDirection(ExecutionContext& context, const DenseDoubleVectorPtr& parameters)
{
  DenseDoubleVectorPtr direction = new DenseDoubleVector(parameters->getClass());
  size_t n = parameters->getNumValues();
  direction->ensureSize(n);
  for (size_t i = 0; i < n; ++i)
    direction->setValue(i, context.getRandomGenerator()->sampleDoubleFromGaussian());
  return testDerivative(context, parameters, direction);
}

bool DifferentiableObjective::testDerivative(ExecutionContext& context, const DenseDoubleVectorPtr& parameters, const DoubleVectorPtr& direction)
{
  double dirNorm = direction->l2norm();
  if (!dirNorm)
  {
    context.errorCallback("Empty direction");
    jassert(false);
    return false;
  }
  DoubleVectorPtr gradient;
  evaluate(context, parameters, NULL, &gradient);
  double analyticDerivative = gradient->dotProduct(direction) / dirNorm;

  double eps = 5e-6 / dirNorm;
  double v1, v2;

  DenseDoubleVectorPtr x = parameters->cloneAndCast<DenseDoubleVector>();
  direction->addWeightedTo(x, 0, -eps);
  evaluate(context, x, &v1, NULL);
  direction->addWeightedTo(x, 0, 2 * eps);
  evaluate(context, x, &v2, NULL);
  double numericDerivative = (v2 - v1) / (2.0 * eps * dirNorm);
    
  bool res = fabs(numericDerivative - analyticDerivative) < 0.0001;
  if (!res)
  {
    context.errorCallback("Inconsistent gradient: eps = " + String(eps) + " Numeric: " + String(numericDerivative) + " Analytic: " + String(analyticDerivative));
    jassertfalse;
  }
  return res;
}

/*
** NewProblem
*/
bool NewProblem::loadFromString(ExecutionContext& context, const String& str)
{
  if (!Problem::loadFromString(context, str))
    return false;
  initialize(context);
  return true;
}
