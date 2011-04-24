/*-----------------------------------------.---------------------------------.
| Filename: BanditsDecisionProblem.h       | Bandits DecisionProblem         |
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
class BanditsState;
typedef ReferenceCountedObjectPtr<BanditsState> BanditsStatePtr;

class BanditsState : public DecisionProblemState
{
public:
  BanditsState(size_t numBandits, long long seedValue)
  {
    randomGenerators.resize(numBandits);
    availableActions = vector(positiveIntegerType, numBandits);
    for (size_t i = 0; i < numBandits; ++i)
    {
      randomGenerators[i] = new RandomGenerator(seedValue);
      availableActions->setElement(i, i);
    }
  }
  BanditsState() {}

  virtual TypePtr getActionType() const
    {return positiveIntegerType;}

  virtual ContainerPtr getAvailableActions() const
    {return availableActions;}

  virtual double sampleReward(size_t banditNumber, RandomGeneratorPtr random) = 0;

  virtual void performTransition(const Variable& action, double& reward)
  {
    size_t banditNumber = (size_t)action.getInteger();
    jassert(banditNumber < randomGenerators.size());
    reward = sampleReward(banditNumber, randomGenerators[banditNumber]);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t)
  {
    const BanditsStatePtr& target = t.staticCast<BanditsState>();
    target->availableActions = availableActions;
    target->randomGenerators.resize(randomGenerators.size());
    for (size_t i = 0; i < randomGenerators.size(); ++i)
      target->randomGenerators[i] = randomGenerators[i]->cloneAndCast<RandomGenerator>(context);
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class BanditsStateClass;

  std::vector<RandomGeneratorPtr> randomGenerators;
  ContainerPtr availableActions;
};

class BanditsDecisionProblem : public DecisionProblem
{
public:
  BanditsDecisionProblem(const FunctionPtr& initialStateSampler, size_t numBandits)
    : DecisionProblem(initialStateSampler, 1.0), numBandits(numBandits) {}
  BanditsDecisionProblem() {}

  virtual TypePtr getActionType() const
    {return positiveIntegerType;}

  virtual size_t getFixedNumberOfActions() const
    {return numBandits;}

protected:
  friend class BanditsDecisionProblemClass;

  size_t numBandits;
};

/*
** Bernouilli Bandits
*/
class BernouilliBanditsState : public BanditsState
{
public:
  BernouilliBanditsState(const std::vector<double>& probabilities, long long seedValue)
    : BanditsState(probabilities.size(), seedValue), probabilities(probabilities) {}
  BernouilliBanditsState() {}

  virtual double sampleReward(size_t banditNumber, RandomGeneratorPtr random)
    {return random->sampleBool(probabilities[banditNumber]) ? 1.0 : 0.0;}

protected:
  friend class BernouilliBanditsStateClass;

  std::vector<double> probabilities;
};

extern ClassPtr bernouilliBanditsStateClass;

class BernouilliBanditsInitialStateSampler : public SimpleUnaryFunction
{
public:
  BernouilliBanditsInitialStateSampler(size_t numBandits = 0)
    : SimpleUnaryFunction(randomGeneratorClass, bernouilliBanditsStateClass), numBandits(numBandits) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const RandomGeneratorPtr& random = input.getObjectAndCast<RandomGenerator>();
    std::vector<double> probabilities(numBandits);
    for (size_t i = 0; i < numBandits; ++i)
      probabilities[i] = random->sampleDouble(1.0);
    long long seed = random->sampleInt();
    return new BernouilliBanditsState(probabilities, seed);   
  }

protected:
  friend class BernouilliBanditsInitialStateSamplerClass;

  size_t numBandits;
};

class BernouilliBanditsDecisionProblem : public BanditsDecisionProblem
{
public:
  BernouilliBanditsDecisionProblem(size_t numBandits)
    : BanditsDecisionProblem(new BernouilliBanditsInitialStateSampler(numBandits), numBandits) {}
  BernouilliBanditsDecisionProblem() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_PROBLEM_BANDITS_H_
