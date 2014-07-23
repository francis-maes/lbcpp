/*-----------------------------------------.---------------------------------.
| Filename: Problem.cpp                    | Optimization Problem            |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <ml/Problem.h>
#include <ml/Expression.h>
#include <ml/ExpressionDomain.h>
#include <ml/SolutionContainer.h>
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

FitnessPtr Problem::evaluate(ExecutionContext& context, const ObjectPtr& object) const
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

ProblemPtr Problem::toSupervisedLearningProblem(ExecutionContext& context, size_t numSamples, size_t numValidationSamples, SamplerPtr sampler) const
{
  ExpressionDomainPtr domain = new ExpressionDomain();
  TablePtr supervision = new Table(numSamples);
  TablePtr validation = new Table(numValidationSamples);
    
  for (size_t i = 0; i < getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions(); ++i)
  {
    VariableExpressionPtr x = domain->addInput(doubleClass, "x" + string((int) i));
    supervision->addColumn(x, doubleClass);
    validation->addColumn(x, doubleClass);
  }
  VariableExpressionPtr y;
  if (this->getNumObjectives() > 1)
    {jassertfalse;}
  else
    y = domain->createSupervision(doubleClass, "y");
  supervision->addColumn(y, y->getType());
  validation->addColumn(y, y->getType());

  // fill the tables
  sampler->initialize(context, getDomain());
  for (size_t i = 0; i < numSamples; ++i)
  {
    DenseDoubleVectorPtr sample = sampler->sample(context).staticCast<DenseDoubleVector>();
    FitnessPtr result = evaluate(context, sample);
    for (size_t j = 0; j < sample->getNumValues(); ++j)
      supervision->setElement(i, j, new Double(sample->getValue(j)));
    supervision->setElement(i, sample->getNumValues(), new Double(result->getValue(0)));
  }
  for (size_t i = 0; i < numValidationSamples; ++i)
  {
    DenseDoubleVectorPtr sample = sampler->sample(context).staticCast<DenseDoubleVector>();
    FitnessPtr result = evaluate(context, sample);
    for (size_t j = 0; j < sample->getNumValues(); ++j)
      validation->setElement(i, j, new Double(sample->getValue(j)));
    validation->setElement(i, sample->getNumValues(), new Double(result->getValue(0)));
  }

  ProblemPtr res = new Problem();
  res->setThisClass(getClass());
    
  res->setDomain(domain);
  res->addObjective(rmseRegressionObjective(supervision, y));
  res->addValidationObjective(rrseRegressionObjective(supervision, validation, y));

  return res;
}

std::vector<ProblemPtr> Problem::generateFolds(ExecutionContext& context, size_t numFolds, size_t samplesPerFold, SamplerPtr sampler) const
{
  std::vector<ProblemPtr> res(numFolds);
  std::vector<VariableExpressionPtr> variables;
  size_t numSamples = numFolds * samplesPerFold;
  TablePtr samples = new Table(numSamples);
  ExpressionDomainPtr domain = new ExpressionDomain();

  for (size_t i = 0; i < getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions(); ++i)
  {
    VariableExpressionPtr x = domain->addInput(doubleClass, "x" + string((int) i));
    variables.push_back(x);
    samples->addColumn(x, doubleClass);
  }
  VariableExpressionPtr y;
  if (this->getNumObjectives() > 1)
    {jassertfalse;}
  else
    y = domain->createSupervision(doubleClass, "y");
  samples->addColumn(y, y->getType());

  for (size_t i = 0; i < numSamples; ++i)
  {
    DenseDoubleVectorPtr sample = sampler->sample(context).staticCast<DenseDoubleVector>();
    FitnessPtr result = evaluate(context, sample);
    for (size_t j = 0; j < sample->getNumValues(); ++j)
      samples->setElement(i, j, new Double(sample->getValue(j)));
    samples->setElement(i, sample->getNumValues(), new Double(result->getValue(0)));
  }

  for (size_t i = 0; i < numFolds; ++i)
  {
    TablePtr foldTrain = new Table();
    TablePtr foldTest = new Table();
    for (size_t j = 0; j < variables.size(); ++j)
    {
      foldTrain->addColumn(variables[j], variables[j]->getType());
      foldTest->addColumn(variables[j], variables[j]->getType());
    }
    foldTrain->addColumn(y, y->getType());
    foldTest->addColumn(y, y->getType());
    for (size_t j = 0; j < numSamples; ++j)
    {
      if (((int)j - (int)i) % (int)numFolds == 0)
        foldTest->addRow(samples->getRow(j));
      else
        foldTrain->addRow(samples->getRow(j));
    }
    ProblemPtr fold = new Problem();
    fold->setDomain(domain);
    fold->addObjective(rmseRegressionObjective(foldTrain, y));
    fold->addValidationObjective(rrseRegressionObjective(foldTrain, foldTest, y));
    res[i] = fold;
  }

  return res;
}

/**
 * Create a supervised learning problem from a Table
 * This method assumes the last column of the Table is the supervision
 */
ProblemPtr Problem::fromTable(ExecutionContext& context, const TablePtr& table, double testSetFraction)
{
  ExpressionDomainPtr domain = new ExpressionDomain();
  size_t numTrain = (size_t)(table->getNumRows() * (1 - testSetFraction));
  std::vector<size_t> indices(table->getNumRows());
  for (size_t i = 0; i < table->getNumRows(); ++i)
    indices[i] = i;
  context.getRandomGenerator()->shuffle(indices);
  TablePtr train = new Table();
  TablePtr test = new Table();
  for (size_t i = 0; i < table->getNumColumns() - 1; ++i)
  {
    VariableExpressionPtr x = domain->addInput(table->getType(i), table->getKey(i)->toShortString());
    train->addColumn(x, x->getType());
    test->addColumn(x, x->getType());
  }
  VariableExpressionPtr y = domain->createSupervision(table->getType(table->getNumColumns() - 1), table->getKey(table->getNumColumns() - 1)->toShortString());
  train->addColumn(y, y->getType());
  test->addColumn(y, y->getType());

  for (size_t i = 0; i < numTrain; ++i)
    train->addRow(table->getRow(indices[i]));
  for (size_t i = numTrain; i < indices.size(); ++i)
    test->addRow(table->getRow(indices[i]));

  ProblemPtr result = new Problem();
  result->setDomain(domain);
  result->addObjective(rmseRegressionObjective(train, y));
  result->addValidationObjective(rrseRegressionObjective(train, test, y));

  return result;
}

std::vector<ProblemPtr> Problem::generateFoldsFromTable(ExecutionContext& context, const TablePtr& table, size_t numFolds)
{
  std::vector<VariableExpressionPtr> variables;
  ExpressionDomainPtr domain = new ExpressionDomain();
  size_t numSamples = table->getNumRows();
  std::vector<ProblemPtr> res(numFolds);

  for (size_t i = 0; i < table->getNumColumns() - 1; ++i)
    variables.push_back(domain->addInput(table->getType(i), table->getKey(i)->toShortString()));
  VariableExpressionPtr y = domain->createSupervision(table->getType(table->getNumColumns() - 1), table->getKey(table->getNumColumns() - 1)->toShortString());

  for (size_t i = 0; i < numFolds; ++i)
  {
    TablePtr foldTrain = new Table();
    TablePtr foldTest = new Table();
    for (size_t j = 0; j < variables.size(); ++j)
    {
      foldTrain->addColumn(variables[j], variables[j]->getType());
      foldTest->addColumn(variables[j], variables[j]->getType());
    }
    foldTrain->addColumn(y, y->getType());
    foldTest->addColumn(y, y->getType());
    for (size_t j = 0; j < numSamples; ++j)
    {
      if (((int)j - (int)i) % (int)numFolds == 0)
        foldTest->addRow(table->getRow(j));
      else
        foldTrain->addRow(table->getRow(j));
    }
    ProblemPtr fold = new Problem();
    fold->setDomain(domain);
    fold->addObjective(rmseRegressionObjective(foldTrain, y));
    fold->addValidationObjective(rrseRegressionObjective(foldTrain, foldTest, y));
    res[i] = fold;
  }

  return res;
}

