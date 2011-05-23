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
    : randomGenerators(samplers.size()), samplers(samplers)
  {
    size_t n = samplers.size();
    availableActions = vector(positiveIntegerType, n);
    ++seedValue;
    for (size_t i = 0; i < n; ++i)
    {
      randomGenerators[i] = new RandomGenerator(seedValue * i);
      availableActions->setElement(i, i);
    }
  }
  DiscreteBanditState() {}

  virtual TypePtr getActionType() const
    {return positiveIntegerType;}

  virtual ContainerPtr getAvailableActions() const
    {return availableActions;}

  void setSeed(juce::uint32 seedValue)
  {
    ++seedValue;
    for (size_t i = 0; i < randomGenerators.size(); ++i)
      randomGenerators[i] = new RandomGenerator(seedValue * i);
  }

  double getExpectedReward(size_t banditNumber) const
    {return samplers[banditNumber]->computeExpectation().toDouble();}

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward)
  {
    size_t banditNumber = (size_t)action.getInteger();
    jassert(banditNumber < randomGenerators.size());
    reward = samplers[banditNumber]->sample(context, randomGenerators[banditNumber]).toDouble();
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    Object::clone(context, t);
    const DiscreteBanditStatePtr& target = t.staticCast<DiscreteBanditState>();
    target->availableActions = availableActions;
    target->randomGenerators.resize(randomGenerators.size());
    for (size_t i = 0; i < randomGenerators.size(); ++i)
      target->randomGenerators[i] = randomGenerators[i]->cloneAndCast<RandomGenerator>(context);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DiscreteBanditStateClass;

  std::vector<RandomGeneratorPtr> randomGenerators;
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

/*
** Bernouilli Bandits
*
class BernouilliDiscreteBanditState : public DiscreteBanditState
{
public:
  BernouilliDiscreteBanditState(const std::vector<double>& probabilities, juce::uint32 seedValue)
    : DiscreteBanditState(probabilities.size(), seedValue), probabilities(probabilities)
  {
    String name;
    for (size_t i = 0; i < probabilities.size(); ++i)
    {
      if (name.isNotEmpty())
        name += T(" ");
      name += T("p") + String((int)i + 1) + T(" = ") + String(probabilities[i]);
    }
    setName(name);
  }
  BernouilliDiscreteBanditState() {}

  virtual double sampleReward(size_t banditNumber, RandomGeneratorPtr random) const
    {return random->sampleBool(probabilities[banditNumber]) ? 1.0 : 0.0;}

  virtual double getExpectedReward(size_t banditNumber) const
    {jassert(banditNumber < probabilities.size()); return probabilities[banditNumber];}

  virtual size_t getOptimalBandit(double& bestReward, double& secondBestReward) const
  {
    size_t res = 0;
    bestReward = -DBL_MAX;
    for (size_t i = 0; i < probabilities.size(); ++i)
    {
      double p = probabilities[i];
      if (p > bestReward)
      {
        bestReward = p;
        res = i;
      }
    }

    secondBestReward = -DBL_MAX;
    for (size_t i = 0; i < probabilities.size(); ++i)
      if (i != res)
        secondBestReward = juce::jmax(secondBestReward, probabilities[i]);

    jassert(secondBestReward < bestReward);
    return res;
  }

protected:
  friend class BernouilliDiscreteBanditStateClass;

  std::vector<double> probabilities;
};

extern ClassPtr bernouilliDiscreteBanditStateClass;


class BernouilliBanditsInitialStateSampler : public SimpleUnaryFunction
{
public:
  BernouilliBanditsInitialStateSampler(size_t numBandits = 0)
    : SimpleUnaryFunction(randomGeneratorClass, bernouilliDiscreteBanditStateClass), numBandits(numBandits) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    std::vector<SamplerPtr> samplers(numBandit);
    for (size_t i = 0; i < numBandits; ++i)
      samplers[i] = bernouilliSampler(random->sampleDouble(1.0));
    juce::uint32 seed = random->sampleUint32();
    return new DiscreteBanditState(samplers, seed);   
  }

protected:
  friend class BernouilliBanditsInitialStateSamplerClass;

  size_t numBandits;
};

class BernouilliDiscreteBanditDecisionProblem : public DiscreteBanditDecisionProblem
{
public:
  BernouilliDiscreteBanditDecisionProblem(size_t numBandits)
    : DiscreteBanditDecisionProblem(new BernouilliBanditsInitialStateSampler(numBandits), numBandits) {}
  BernouilliDiscreteBanditDecisionProblem() {}
};*/


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_BANDITS_H_
