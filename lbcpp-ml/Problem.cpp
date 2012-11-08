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
** NewProblem
*/
bool NewProblem::loadFromString(ExecutionContext& context, const String& str)
{
  if (!Problem::loadFromString(context, str))
    return false;
  initialize(context);
  return true;
}


/*
** ContinuousDerivableProblem
*/
FitnessPtr ContinuousDerivableProblem::evaluate(ExecutionContext& context, const ObjectPtr& object)
{
  DenseDoubleVectorPtr parameters = object.staticCast<DenseDoubleVector>();

  std::vector<double> values(limits->getNumObjectives());
  for (size_t i = 0; i < values.size(); ++i)
    evaluate(context, parameters, i, &values[i], NULL);
  return new Fitness(values, limits);
}

bool ContinuousDerivableProblem::testDerivativeWithRandomDirection(ExecutionContext& context, const DenseDoubleVectorPtr& parameters)
{
  DenseDoubleVectorPtr direction = new DenseDoubleVector(parameters->getClass());
  size_t n = parameters->getNumValues();
  direction->ensureSize(n);
  for (size_t i = 0; i < n; ++i)
    direction->setValue(i, context.getRandomGenerator()->sampleDoubleFromGaussian());
  return testDerivative(context, parameters, direction);
}

bool ContinuousDerivableProblem::testDerivative(ExecutionContext& context, const DenseDoubleVectorPtr& parameters, const DoubleVectorPtr& direction)
{
  double dirNorm = direction->l2norm();
  if (!dirNorm)
  {
    context.errorCallback("Empty direction");
    jassert(false);
    return false;
  }
  DoubleVectorPtr gradient;
  evaluate(context, parameters, 0, NULL, &gradient);
  double analyticDerivative = gradient->dotProduct(direction) / dirNorm;

  double eps = 5e-6 / dirNorm;
  double v1, v2;

  DenseDoubleVectorPtr x = parameters->cloneAndCast<DenseDoubleVector>();
  direction->addWeightedTo(x, 0, -eps);
  evaluate(context, x, 0, &v1, NULL);
  direction->addWeightedTo(x, 0, 2 * eps);
  evaluate(context, x, 0, &v2, NULL);
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
** DecoratorProblem
*/
DecoratorProblem::DecoratorProblem(ProblemPtr problem)
  : problem(problem) {}

DomainPtr DecoratorProblem::getDomain() const
  {return problem->getDomain();}

FitnessLimitsPtr DecoratorProblem::getFitnessLimits() const
  {return problem->getFitnessLimits();}

FitnessPtr DecoratorProblem::evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {return problem->evaluate(context, solution);}

ObjectPtr DecoratorProblem::proposeStartingSolution(ExecutionContext& context) const
  {return problem->proposeStartingSolution(context);}

bool DecoratorProblem::shouldStop() const
  {return problem->shouldStop();}

/*
** MaxIterationsDecoratorProblem
*/
MaxIterationsDecoratorProblem::MaxIterationsDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations)
  : DecoratorProblem(problem), maxNumEvaluations(maxNumEvaluations), numEvaluations(0)
{
}

MaxIterationsDecoratorProblem::MaxIterationsDecoratorProblem()
  : maxNumEvaluations(0), numEvaluations(0)
{
}

FitnessPtr MaxIterationsDecoratorProblem::evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {jassert(numEvaluations < maxNumEvaluations); ++numEvaluations; return DecoratorProblem::evaluate(context, solution);}

bool MaxIterationsDecoratorProblem::shouldStop() const
  {return numEvaluations >= maxNumEvaluations || DecoratorProblem::shouldStop();}

size_t MaxIterationsDecoratorProblem::getNumEvaluations() const
  {return numEvaluations;}

/*
** EvaluatorDecoratorProblem
*/
EvaluatorDecoratorProblem::EvaluatorDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
  : MaxIterationsDecoratorProblem(problem, maxNumEvaluations), evaluationPeriod(evaluationPeriod) {}
EvaluatorDecoratorProblem::EvaluatorDecoratorProblem() : evaluationPeriod(0) {}

FitnessPtr EvaluatorDecoratorProblem::evaluate(ExecutionContext& context, const ObjectPtr& solution)
{
  if (numEvaluations == 0)
    firstEvaluationTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
  FitnessPtr res = decoratedEvaluate(context, solution);
  if ((numEvaluations % evaluationPeriod) == 0)
  {
    evaluate(context);
    cpuTimes.push_back(juce::Time::getMillisecondCounterHiRes() / 1000.0 - firstEvaluationTime);
  }
  return res;
}

const std::vector<double>& EvaluatorDecoratorProblem::getCpuTimes() const
  {return cpuTimes;}

size_t EvaluatorDecoratorProblem::getEvaluationPeriod() const
  {return evaluationPeriod;}

/*
** SingleObjectiveEvaluatorDecoratorProblem
*/
SingleObjectiveEvaluatorDecoratorProblem::SingleObjectiveEvaluatorDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
  : EvaluatorDecoratorProblem(problem, maxNumEvaluations, evaluationPeriod) {jassert(problem->getNumObjectives() == 1);}
 
FitnessPtr SingleObjectiveEvaluatorDecoratorProblem::decoratedEvaluate(ExecutionContext& context, const ObjectPtr& solution)
{
  FitnessPtr res = MaxIterationsDecoratorProblem::evaluate(context, solution);
  if (!bestFitness || res->strictlyDominates(bestFitness))
    bestFitness = res;
  return res;
}

void SingleObjectiveEvaluatorDecoratorProblem::evaluate(ExecutionContext& context)
  {scores.push_back(bestFitness->getValue(0));}

const std::vector<double>& SingleObjectiveEvaluatorDecoratorProblem::getScores() const
  {return scores;}

/*
** HyperVolumeEvaluatorDecoratorProblem
*/
HyperVolumeEvaluatorDecoratorProblem::HyperVolumeEvaluatorDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
  : EvaluatorDecoratorProblem(problem, maxNumEvaluations, evaluationPeriod)
  {front = new ParetoFront(problem->getFitnessLimits());}

FitnessPtr HyperVolumeEvaluatorDecoratorProblem::decoratedEvaluate(ExecutionContext& context, const ObjectPtr& object)
{
  FitnessPtr res = MaxIterationsDecoratorProblem::evaluate(context, object);
  front->insertSolution(object, res);
  return res;
}
 
void HyperVolumeEvaluatorDecoratorProblem::evaluate(ExecutionContext& context)
  {hyperVolumes.push_back(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));}

const std::vector<double>& HyperVolumeEvaluatorDecoratorProblem::getHyperVolumes() const
  {return hyperVolumes;}
