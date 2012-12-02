/*-----------------------------------------.---------------------------------.
| Filename: Problem.cpp                    | Optimization Problem            |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/Problem.h>
#include <lbcpp-ml/Expression.h>
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
    context.errorCallback("Inconsistent gradient: eps = " + string(eps) + " Numeric: " + string(numericDerivative) + " Analytic: " + string(analyticDerivative));
    jassertfalse;
  }
  return res;
}

DataVectorPtr LearningObjective::computePredictions(ExecutionContext& context, ExpressionPtr expression) const
  {return expression->compute(context, data, indices);}

void SupervisedLearningObjective::configure(const TablePtr& data, const VariableExpressionPtr& supervision, const DenseDoubleVectorPtr& weights, const IndexSetPtr& indices)
{
  this->data = data;
  this->supervision = supervision;
  this->weights = weights;
  this->indices = indices ? indices : new IndexSet(0, data->getNumRows());
}
  
VectorPtr SupervisedLearningObjective::getSupervisions() const
  {return data->getDataByKey(supervision);}


/*
** Problem
*/
FitnessLimitsPtr Problem::getFitnessLimits() const
{
  if (!limits)
  {
    std::vector<std::pair<double, double> > l(objectives.size());
    for (size_t i = 0; i < l.size(); ++i)
      objectives[i]->getObjectiveRange(l[i].first, l[i].second);
    const_cast<Problem* >(this)->limits = new FitnessLimits(l);
  }
  return limits;
}

FitnessPtr Problem::evaluate(ExecutionContext& context, const ObjectPtr& object)
{
  FitnessLimitsPtr limits = getFitnessLimits();
  std::vector<double> o(objectives.size());
  for (size_t i = 0; i < o.size(); ++i)
    o[i] = objectives[i]->evaluate(context, object);
  return new Fitness(o, limits);
}

void Problem::reinitialize(ExecutionContext& context)
{
  domain = DomainPtr();
  objectives.clear();
  initialGuess = ObjectPtr();
  initialize(context);
}

bool Problem::loadFromString(ExecutionContext& context, const string& str)
{
  if (!Object::loadFromString(context, str))
    return false;
  initialize(context);
  return true;
}
