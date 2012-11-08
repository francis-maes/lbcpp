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
# include "Objective.h"
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

class NewProblem : public Problem
{
public:
  virtual DomainPtr getDomain() const
    {return domain;}

  virtual FitnessLimitsPtr getFitnessLimits() const
  {
    if (!limits)
    {
      std::vector<std::pair<double, double> > l(objectives.size());
      for (size_t i = 0; i < l.size(); ++i)
        objectives[i]->getObjectiveRange(l[i].first, l[i].second);
      const_cast<NewProblem* >(this)->limits = new FitnessLimits(l);
    }
    return limits;
  }

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {return initialGuess;}
  
  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    FitnessLimitsPtr limits = getFitnessLimits();
    std::vector<double> o(objectives.size());
    for (size_t i = 0; i < o.size(); ++i)
      o[i] = objectives[i]->evaluate(context, object);
    return new Fitness(o, limits);
  }

  virtual bool loadFromString(ExecutionContext& context, const String& str);

  void reinitialize(ExecutionContext& context)
  {
    domain = DomainPtr();
    objectives.clear();
    initialGuess = ObjectPtr();
    initialize(context);
  }

protected:
  DomainPtr domain;
  std::vector<ObjectivePtr> objectives;
  ObjectPtr initialGuess;

  virtual void initialize(ExecutionContext& context) {}

  void setDomain(DomainPtr domain)
    {this->domain = domain;}

  void addObjective(ObjectivePtr objective)
    {objectives.push_back(objective);}

  void setInitialGuess(ObjectPtr initialGuess)
    {this->initialGuess = initialGuess;}

private:
  FitnessLimitsPtr limits;
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

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object);

  bool testDerivativeWithRandomDirection(ExecutionContext& context, const DenseDoubleVectorPtr& parameters);
  bool testDerivative(ExecutionContext& context, const DenseDoubleVectorPtr& parameters, const DoubleVectorPtr& direction);
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
