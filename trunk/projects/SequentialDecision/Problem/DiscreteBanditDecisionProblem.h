/*-----------------------------------------.---------------------------------.
| Filename: DiscreteBanditDecisionProblem.h| Bandits DecisionProblem         |
| Author  : Francis Maes                   |                                 |
| Started : 24/04/2011 14:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_PROBLEM_BANDITS_H_
# define LBCPP_SEQUENTIAL_DECISION_PROBLEM_BANDITS_H_

# include "../Core/DecisionProblem.h"

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
  DiscreteBanditState(const std::vector<SamplerPtr>& samplers, juce::uint32 seedValue)
    : random(new RandomGenerator(seedValue)), samplers(samplers)
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

  void setRandomGenerator(const RandomGeneratorPtr& random)
    {this->random = random;}

  void setSeed(juce::uint32 seedValue)
    {random->setSeed(seedValue);}

  double getExpectedReward(size_t banditNumber) const
    {return samplers[banditNumber]->computeExpectation().toDouble();}

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward)
  {
    size_t banditNumber = (size_t)action.getInteger();
    jassert(banditNumber < samplers.size());
    reward = samplers[banditNumber]->sample(context, random).toDouble();
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
    target->random = random;//->cloneAndCast<RandomGenerator>(context);
    target->name = name;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DiscreteBanditStateClass;

  RandomGeneratorPtr random;
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
    return new DiscreteBanditState(samplers, random->sampleUint32());
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

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_BANDITS_H_
