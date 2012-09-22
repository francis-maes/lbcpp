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

class MOOProblem : public Object
{
public:
  virtual MOODomainPtr getObjectDomain() const = 0;
  virtual MOOFitnessLimitsPtr getFitnessLimits() const = 0;

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object) = 0;

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {jassertfalse; return ObjectPtr();}

  virtual bool shouldStop() const
    {return false;}

  size_t getNumObjectives() const
    {return getFitnessLimits()->getNumObjectives();}
};

class DecoratorMOOProblem : public MOOProblem
{
public:
  DecoratorMOOProblem(MOOProblemPtr problem = MOOProblemPtr());

  virtual MOODomainPtr getObjectDomain() const;
  virtual MOOFitnessLimitsPtr getFitnessLimits() const;
  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);
  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const;
  virtual bool shouldStop() const;

protected:
  friend class DecoratorMOOProblemClass;

  MOOProblemPtr problem;
};

class MaxIterationsDecoratorProblem : public DecoratorMOOProblem
{
public:
  MaxIterationsDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations);
  MaxIterationsDecoratorProblem();

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);
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
  EvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod);
  EvaluatorDecoratorProblem();

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);

  const std::vector<double>& getCpuTimes() const;
  size_t getEvaluationPeriod() const;

protected:
  friend class EvaluatorDecoratorProblemClass;

  size_t evaluationPeriod;

  double firstEvaluationTime;
  std::vector<double> cpuTimes;

  virtual MOOFitnessPtr decoratedEvaluate(ExecutionContext& context, const ObjectPtr& solution)
    {return MaxIterationsDecoratorProblem::evaluate(context, solution);}

  virtual void evaluate(ExecutionContext& context) {}
};

class SingleObjectiveEvaluatorDecoratorProblem : public EvaluatorDecoratorProblem
{
public:
  SingleObjectiveEvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod);
  SingleObjectiveEvaluatorDecoratorProblem() {}
 
  virtual MOOFitnessPtr decoratedEvaluate(ExecutionContext& context, const ObjectPtr& solution);
  virtual void evaluate(ExecutionContext& context);
  const std::vector<double>& getScores() const;

protected:
  std::vector<double> scores;
  MOOFitnessPtr bestFitness;
};

typedef ReferenceCountedObjectPtr<SingleObjectiveEvaluatorDecoratorProblem> SingleObjectiveEvaluatorDecoratorProblemPtr;

class HyperVolumeEvaluatorDecoratorProblem : public EvaluatorDecoratorProblem
{
public:
  HyperVolumeEvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod);
  HyperVolumeEvaluatorDecoratorProblem() {}

  virtual MOOFitnessPtr decoratedEvaluate(ExecutionContext& context, const ObjectPtr& object);
  virtual void evaluate(ExecutionContext& context);
  const std::vector<double>& getHyperVolumes() const;

protected:
  MOOParetoFrontPtr front;
  std::vector<double> hyperVolumes;
};

typedef ReferenceCountedObjectPtr<HyperVolumeEvaluatorDecoratorProblem> HyperVolumeEvaluatorDecoratorProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_PROBLEM_H_
