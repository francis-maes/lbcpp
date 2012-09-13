/*-----------------------------------------.---------------------------------.
| Filename: DecoratorMOOProblems.h         | Decorator MOO Problems          |
| Author  : Francis Maes                   |                                 |
| Started : 12/09/2012 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_PROBLEM_DECORATOR_H_
# define LBCPP_MOO_PROBLEM_DECORATOR_H_

# include "MOOCore.h"

namespace lbcpp
{

class DecoratorMOOProblem : public MOOProblem
{
public:
  DecoratorMOOProblem(MOOProblemPtr problem = MOOProblemPtr())
    : problem(problem) {}

  virtual MOODomainPtr getSolutionDomain() const
    {return problem->getSolutionDomain();}

  virtual MOOFitnessLimitsPtr getFitnessLimits() const
    {return problem->getFitnessLimits();}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
    {return problem->evaluate(context, solution);}

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {return problem->proposeStartingSolution(context);}

  virtual bool shouldStop() const
    {return problem->shouldStop();}

protected:
  friend class DecoratorMOOProblemClass;

  MOOProblemPtr problem;
};

class MaxIterationsDecoratorProblem : public DecoratorMOOProblem
{
public:
  MaxIterationsDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations)
    : DecoratorMOOProblem(problem), maxNumEvaluations(maxNumEvaluations), numEvaluations(0) {}
  MaxIterationsDecoratorProblem() : maxNumEvaluations(0), numEvaluations(0) {}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
    {jassert(numEvaluations < maxNumEvaluations); ++numEvaluations; return DecoratorMOOProblem::evaluate(context, solution);}

  virtual bool shouldStop() const
    {return numEvaluations >= maxNumEvaluations || DecoratorMOOProblem::shouldStop();}

  size_t getNumEvaluations() const
    {return numEvaluations;}

protected:
  friend class MaxIterationsDecoratorProblemClass;

  size_t maxNumEvaluations;

  size_t numEvaluations;
};

typedef ReferenceCountedObjectPtr<MaxIterationsDecoratorProblem> MaxIterationsDecoratorProblemPtr;

class EvaluatorDecoratorProblem : public MaxIterationsDecoratorProblem
{
public:
  EvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
    : MaxIterationsDecoratorProblem(problem, maxNumEvaluations), evaluationPeriod(evaluationPeriod) {}
  EvaluatorDecoratorProblem() : evaluationPeriod(0) {}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    if (numEvaluations == 0)
      firstEvaluationTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    MOOFitnessPtr res = decoratedEvaluate(context, solution);
    if ((numEvaluations % evaluationPeriod) == 0)
    {
      evaluate(context);
      cpuTimes.push_back(juce::Time::getMillisecondCounterHiRes() / 1000.0 - firstEvaluationTime);
    }
    return res;
  }

  const std::vector<double>& getCpuTimes() const
    {return cpuTimes;}

  size_t getEvaluationPeriod() const
    {return evaluationPeriod;}

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
  SingleObjectiveEvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
    : EvaluatorDecoratorProblem(problem, maxNumEvaluations, evaluationPeriod) {jassert(problem->getNumObjectives() == 1);}
  SingleObjectiveEvaluatorDecoratorProblem() {}
 
  virtual MOOFitnessPtr decoratedEvaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    MOOFitnessPtr res = MaxIterationsDecoratorProblem::evaluate(context, solution);
    if (!bestFitness || res->strictlyDominates(bestFitness))
      bestFitness = res;
    return res;
  }

  virtual void evaluate(ExecutionContext& context)
    {scores.push_back(bestFitness->getValue(0));}

  const std::vector<double>& getScores() const
    {return scores;}

protected:
  std::vector<double> scores;
  MOOFitnessPtr bestFitness;
};

typedef ReferenceCountedObjectPtr<SingleObjectiveEvaluatorDecoratorProblem> SingleObjectiveEvaluatorDecoratorProblemPtr;

class HyperVolumeEvaluatorDecoratorProblem : public EvaluatorDecoratorProblem
{
public:
  HyperVolumeEvaluatorDecoratorProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t evaluationPeriod)
    : EvaluatorDecoratorProblem(problem, maxNumEvaluations, evaluationPeriod)
    {front = new MOOParetoFront(problem->getFitnessLimits());}
  HyperVolumeEvaluatorDecoratorProblem() {}

  virtual MOOFitnessPtr decoratedEvaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    MOOFitnessPtr res = MaxIterationsDecoratorProblem::evaluate(context, solution);
    front->insert(solution, res);
    return res;
  }
 
  virtual void evaluate(ExecutionContext& context)
    {hyperVolumes.push_back(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));}

  const std::vector<double>& getHyperVolumes() const
    {return hyperVolumes;}

protected:
  MOOParetoFrontPtr front;
  std::vector<double> hyperVolumes;
};

typedef ReferenceCountedObjectPtr<HyperVolumeEvaluatorDecoratorProblem> HyperVolumeEvaluatorDecoratorProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_PROBLEM_DECORATOR_H_
