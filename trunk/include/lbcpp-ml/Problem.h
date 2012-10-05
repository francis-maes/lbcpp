/*-----------------------------------------.---------------------------------.
| Filename: Problem.h                      | Optimization Problem            |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_PROBLEM_H_
# define LBCPP_ML_PROBLEM_H_

# include "Domain.h"
# include "Fitness.h"

namespace lbcpp
{

class Problem : public Object
{
public:
  virtual DomainPtr getDomain() const = 0;
  virtual FitnessLimitsPtr getFitnessLimits() const = 0;

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object) = 0;

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {jassertfalse; return ObjectPtr();}

  virtual bool shouldStop() const
    {return false;}

  size_t getNumObjectives() const
    {return getFitnessLimits()->getNumObjectives();}
};

class ContinuousProblem : public Problem
{
public:
  ContinuousProblem() {}

  virtual DomainPtr getDomain() const
    {return domain;}

  virtual FitnessLimitsPtr getFitnessLimits() const
    {return limits;}

protected:
  ContinuousDomainPtr domain;
  FitnessLimitsPtr limits;
};

class ContinuousDerivableProblem : public ContinuousProblem
{
public:
  virtual void evaluate(ExecutionContext& context, const DenseDoubleVectorPtr& parameters, size_t objectiveNumber, double* value, DoubleVectorPtr* gradient) = 0;

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    DenseDoubleVectorPtr parameters = object.staticCast<DenseDoubleVector>();

    std::vector<double> values(limits->getNumObjectives());
    for (size_t i = 0; i < values.size(); ++i)
      evaluate(context, parameters, i, &values[i], NULL);
    return new Fitness(values, limits);
  }
};

typedef ReferenceCountedObjectPtr<ContinuousDerivableProblem> ContinuousDerivableProblemPtr;

class DecoratorProblem : public Problem
{
public:
  DecoratorProblem(ProblemPtr problem = ProblemPtr());

  virtual DomainPtr getDomain() const;
  virtual FitnessLimitsPtr getFitnessLimits() const;
  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);
  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const;
  virtual bool shouldStop() const;

protected:
  friend class DecoratorProblemClass;

  ProblemPtr problem;
};

class MaxIterationsDecoratorProblem : public DecoratorProblem
{
public:
  MaxIterationsDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations);
  MaxIterationsDecoratorProblem();

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);
  virtual bool shouldStop() const;
  size_t getNumEvaluations() const;

protected:
  friend class MaxIterationsDecoratorProblemClass;

  size_t maxNumEvaluations;
  size_t numEvaluations;
};

typedef ReferenceCountedObjectPtr<MaxIterationsDecoratorProblem> MaxIterationsDecoratorProblemPtr;

class EvaluatorDecoratorProblem : public MaxIterationsDecoratorProblem
{
public:
  EvaluatorDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod);
  EvaluatorDecoratorProblem();

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);

  const std::vector<double>& getCpuTimes() const;
  size_t getEvaluationPeriod() const;

protected:
  friend class EvaluatorDecoratorProblemClass;

  size_t evaluationPeriod;

  double firstEvaluationTime;
  std::vector<double> cpuTimes;

  virtual FitnessPtr decoratedEvaluate(ExecutionContext& context, const ObjectPtr& solution)
    {return MaxIterationsDecoratorProblem::evaluate(context, solution);}

  virtual void evaluate(ExecutionContext& context) {}
};

class SingleObjectiveEvaluatorDecoratorProblem : public EvaluatorDecoratorProblem
{
public:
  SingleObjectiveEvaluatorDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod);
  SingleObjectiveEvaluatorDecoratorProblem() {}
 
  virtual FitnessPtr decoratedEvaluate(ExecutionContext& context, const ObjectPtr& solution);
  virtual void evaluate(ExecutionContext& context);
  const std::vector<double>& getScores() const;

protected:
  std::vector<double> scores;
  FitnessPtr bestFitness;
};

typedef ReferenceCountedObjectPtr<SingleObjectiveEvaluatorDecoratorProblem> SingleObjectiveEvaluatorDecoratorProblemPtr;

class HyperVolumeEvaluatorDecoratorProblem : public EvaluatorDecoratorProblem
{
public:
  HyperVolumeEvaluatorDecoratorProblem(ProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod);
  HyperVolumeEvaluatorDecoratorProblem() {}

  virtual FitnessPtr decoratedEvaluate(ExecutionContext& context, const ObjectPtr& object);
  virtual void evaluate(ExecutionContext& context);
  const std::vector<double>& getHyperVolumes() const;

protected:
  ParetoFrontPtr front;
  std::vector<double> hyperVolumes;
};

typedef ReferenceCountedObjectPtr<HyperVolumeEvaluatorDecoratorProblem> HyperVolumeEvaluatorDecoratorProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_PROBLEM_H_
