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

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    FitnessLimitsPtr limits = getFitnessLimits();
    std::vector<double> o(objectives.size());
    for (size_t i = 0; i < o.size(); ++i)
      o[i] = objectives[i]->evaluate(context, object);
    return new Fitness(o, limits);
  }

  void reinitialize(ExecutionContext& context)
  {
    domain = DomainPtr();
    objectives.clear();
    initialGuess = ObjectPtr();
    initialize(context);
  }
  
  ObjectivePtr getObjective(size_t index) const
    {jassert(index < objectives.size()); return objectives[index];}

  ObjectPtr getInitialGuess() const
    {return initialGuess;}
  
  void setDomain(DomainPtr domain)
    {this->domain = domain;}

  void addObjective(ObjectivePtr objective)
    {objectives.push_back(objective);}

  void setInitialGuess(ObjectPtr initialGuess)
    {this->initialGuess = initialGuess;}

  virtual bool loadFromString(ExecutionContext& context, const String& str);

protected:
  DomainPtr domain;
  std::vector<ObjectivePtr> objectives;
  ObjectPtr initialGuess;

  virtual void initialize(ExecutionContext& context) {}

private:
  FitnessLimitsPtr limits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_PROBLEM_H_
