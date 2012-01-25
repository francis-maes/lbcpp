/*-----------------------------------------.---------------------------------.
| Filename: DiscreteBanditDecisionProblem.h| Bandits DecisionProblem         |
| Author  : Francis Maes                   |                                 |
| Started : 24/04/2011 14:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_DECISION_PROBLEM_H_
# define LBCPP_BANDITS_DISCRETE_DECISION_PROBLEM_H_

# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{

/*
** Bandits base classes
*/
class DiscreteBanditState;
typedef ReferenceCountedObjectPtr<DiscreteBanditState> DiscreteBanditStatePtr;

class DiscreteBanditState : public DecisionProblemState
{
public:
  DiscreteBanditState(const std::vector<SamplerPtr>& samplers) : samplers(samplers)
  {
    size_t n = samplers.size();
    availableActions = vector(positiveIntegerType, n);
    for (size_t i = 0; i < n; ++i)
      availableActions->setElement(i, i);
  }
  DiscreteBanditState() {}

  virtual TypePtr getActionType() const
    {return positiveIntegerType;}

  virtual ContainerPtr getAvailableActions() const
    {return availableActions;}

  double getExpectedReward(size_t banditNumber) const
    {return samplers[banditNumber]->computeExpectation().toDouble();}

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL)
  {
    size_t banditNumber = (size_t)action.getInteger();
    jassert(banditNumber < samplers.size());
    reward = samplers[banditNumber]->sample(context, context.getRandomGenerator()).toDouble();
  }

  virtual ObjectPtr clone(ExecutionContext& context) const
  {
    ObjectPtr res = new DiscreteBanditState();
    clone(context, res);
    return res;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const DiscreteBanditStatePtr& target = t.staticCast<DiscreteBanditState>();
    target->availableActions = availableActions;
    target->samplers = samplers;
    target->name = name;
  }
  
  size_t getNumArms() const
    {return samplers.size();}

  const std::vector<SamplerPtr>& getArms() const
    {return samplers;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DiscreteBanditStateClass;

  std::vector<SamplerPtr> samplers;
  ContainerPtr availableActions;
};

class DiscreteBanditInitialStateSampler : public CompositeSampler
{
public:
  DiscreteBanditInitialStateSampler(SamplerPtr banditSamplerSampler, size_t numBandits)
    : CompositeSampler(1), numBandits(numBandits) {samplers[0] = banditSamplerSampler;}
  DiscreteBanditInitialStateSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    std::vector<SamplerPtr> samplers(numBandits);
    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i] = this->samplers[0]->sample(context, random, inputs).getObjectAndCast<Sampler>();
    return new DiscreteBanditState(samplers);
  }

protected:
  friend class DiscreteBanditInitialStateSamplerClass;
  size_t numBandits;
};

class DiscreteBanditDecisionProblem : public DecisionProblem
{
public:
  DiscreteBanditDecisionProblem(const FunctionPtr& initialStateSampler, size_t numBandits)
    : DecisionProblem(initialStateSampler, 1.0), numBandits(numBandits) {}
  DiscreteBanditDecisionProblem() {}

  virtual TypePtr getActionType() const
    {return positiveIntegerType;}

  virtual size_t getFixedNumberOfActions() const
    {return numBandits;}

protected:
  friend class DiscreteBanditDecisionProblemClass;

  size_t numBandits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_DECISION_PROBLEM_H_
