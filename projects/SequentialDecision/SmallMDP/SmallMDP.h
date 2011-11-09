/*-----------------------------------------.---------------------------------.
| Filename: SmallMDP.h                     | Finite Markov Decision Process  |
| Author  : Francis Maes                   |                                 |
| Started : 08/11/2011 16:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SMALL_MDP_H_
# define LBCPP_SMALL_MDP_H_

# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class SmallMDP : public Object
{
public:
  virtual size_t getNumStates() const = 0;
  virtual size_t getNumActions() const = 0;
  virtual double getDiscount() const = 0;

  virtual size_t getInitialState() const
    {return 0;}

  virtual SparseDoubleVectorPtr getTransitionProbabilities(size_t state, size_t action, double& Z) const = 0; 

  virtual double sampleReward(ExecutionContext& context, size_t state, size_t action, size_t newState) const = 0;
  virtual double getRewardExpectation(size_t state, size_t action, size_t nextState) const = 0;

  size_t sampleTransition(ExecutionContext& context, size_t state, size_t action, double& reward) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    double Z;
    SparseDoubleVectorPtr transitions = getTransitionProbabilities(state, action, Z);
    
    size_t newState = (size_t)-1;
    double p = random->sampleDouble(Z);
    for (size_t i = 0; i < transitions->getNumValues(); ++i)
    {
      std::pair<size_t, double> tr = transitions->getValue(i);
      if (p <= tr.second)
      {
        newState = tr.first;
        break;
      }
      p -= tr.second;
    }
    jassert(newState != (size_t)-1);
    reward = sampleReward(context, state, action, newState);
    return newState;
  }
};

typedef ReferenceCountedObjectPtr<SmallMDP> SmallMDPPtr;

class EmpiricalSmallMDP : public SmallMDP
{
public:
  EmpiricalSmallMDP(size_t numStates, size_t numActions, double discount)
    : model(numStates, std::vector<StateActionInfo>(numActions)), discount(discount)
  {
    for (size_t i = 0; i < numStates; ++i)
      for (size_t j = 0; j < numActions; ++j)
        model[i][j].initialize();
  }
  EmpiricalSmallMDP() {}

  virtual size_t getNumStates() const
    {return model.size();}
    
  virtual size_t getNumActions() const
    {return model[0].size();}
  
  virtual double getDiscount() const
    {return discount;}
  
  virtual SparseDoubleVectorPtr getTransitionProbabilities(size_t state, size_t action, double& Z) const
  {
    const StateActionInfo& info = model[state][action];
    Z = (double)info.getNumObservations();
    return info.nextStates;
  }
  
  virtual double getRewardExpectation(size_t state, size_t action, size_t nextState) const
    {return model[state][action].rewards->getMean();}
  
  virtual double sampleReward(ExecutionContext& context, size_t state, size_t action, size_t nextState) const
    {jassert(false); return 0.0;}

  size_t getNumObservations(size_t state, size_t action) const
    {return model[state][action].getNumObservations();}

  void observeTransition(size_t state, size_t action, size_t nextState, double reward)
    {model[state][action].observe(nextState, reward);}

protected:
  struct StateActionInfo
  {
    void initialize()
    {
      rewards = new ScalarVariableStatistics("rewards");
      nextStates = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
    }

    void observe(size_t nextState, double reward)
    {
      rewards->push(reward);
      nextStates->incrementValue(nextState, 1.0);
    }

    size_t getNumObservations() const
      {return (size_t)rewards->getCount();}

    ScalarVariableStatisticsPtr rewards;
    SparseDoubleVectorPtr nextStates;
  };

  std::vector< std::vector< StateActionInfo > > model; // s -> a -> model
  double discount;
};

typedef ReferenceCountedObjectPtr<EmpiricalSmallMDP> EmpiricalSmallMDPPtr;

class SimpleSmallMDP : public SmallMDP
{
public:
  SimpleSmallMDP(size_t numStates, size_t numActions, double discount)
    : model(numStates, std::vector<StateActionInfo>(numActions)), discount(discount)
  {
  }
  SimpleSmallMDP() {}

  void setInfo(size_t state, size_t action, const SamplerPtr& reward, const SparseDoubleVectorPtr& nextStates)
  {
    jassert(state < getNumStates() && action < getNumActions());
    StateActionInfo& info = model[state][action];
    info.reward = reward;
    info.nextStates = nextStates;
  }

  virtual size_t getNumStates() const
    {return model.size();}
    
  virtual size_t getNumActions() const
    {return model[0].size();}
  
  virtual double getDiscount() const
    {return discount;}
  
  virtual SparseDoubleVectorPtr getTransitionProbabilities(size_t state, size_t action, double& Z) const
    {Z = 1.0; return model[state][action].nextStates;}
  
  virtual double getRewardExpectation(size_t state, size_t action, size_t nextState) const
    {return model[state][action].reward->computeExpectation().getDouble();}
  
  virtual double sampleReward(ExecutionContext& context, size_t state, size_t action, size_t newState) const
    {return model[state][action].reward->sample(context, context.getRandomGenerator()).toDouble();}

protected:  
  struct StateActionInfo
  {
    SamplerPtr reward;
    SparseDoubleVectorPtr nextStates;
  };

  std::vector< std::vector< StateActionInfo > > model; // s -> a -> model
  double discount;
};

class GeneratedSparseSmallMDP : public SimpleSmallMDP
{
public:
  GeneratedSparseSmallMDP(RandomGeneratorPtr random, size_t numStates, size_t numActions, double discount, size_t numSuccessorsPerState, double nonNullRewardProbability)
    : SimpleSmallMDP(numStates, numActions, discount)
  {
    for (size_t i = 0; i < numStates; ++i)
      for (size_t j = 0; j < numActions; ++j)
      {
        SamplerPtr reward = bernoulliSampler(random->sampleBool(nonNullRewardProbability) ? random->sampleDouble() : 0.0);
        SparseDoubleVectorPtr transitions = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
        
        std::vector<size_t> order;
        random->sampleOrder(numStates, order);
        double Z = 0.0;
        for (size_t k = 0; k < numSuccessorsPerState; ++k)
        {
          double p = random->sampleDouble();
          transitions->setElement(order[k], p);
          Z += p;
        }
        transitions->multiplyByScalar(1.0 / Z);
        setInfo(i, j, reward, transitions);
      }
  }
  GeneratedSparseSmallMDP() {}
};

class HallwaysMDP : public SmallMDP
{
public:
  virtual size_t getNumStates() const
    {return 47;}

  virtual size_t getNumActions() const
    {return 2;}

  virtual double getDiscount() const
    {return 0.95;}

  virtual SparseDoubleVectorPtr getTransitionProbabilities(size_t state, size_t action, double& Z) const
  {
    SparseDoubleVectorPtr res = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
    Z = 1.0;

    if (state <= 2) // initial states
      res->setElement(state * 2 + action + 1, 1.0);
    else if (state <= 42) // inside hallways
      res->setElement(action == 0 ? state + 4 : state, 1.0);
    else if (state <= 46) // terminal states
      res->setElement(action == 0 ? 0 : state, 1.0);
    else
      jassert(false);
    return res;
  }

  virtual double sampleReward(ExecutionContext& context, size_t state, size_t action, size_t nextState) const
  {
    if (action == 0 && (state >= 43 && state <= 46))
    {
      size_t i = state - 42;
      if (context.getRandomGenerator()->sampleBool(1.0 / i))
        return pow(1.5, i + 5.0);
    }
    return 0.0;
  }

  virtual double getRewardExpectation(size_t state, size_t action, size_t nextState) const
  {
    if (action == 0 && (state >= 43 && state <= 46))
    {
      size_t i = state - 42;
      return pow(1.5, i + 5.0) * (1.0 / i);
    }
    else
      return 0.0;
  }
};

class BanditMDP : public SmallMDP
{
public:
  virtual size_t getNumStates() const
    {return 7;}

  virtual size_t getNumActions() const
    {return 6;}

  virtual double getDiscount() const
    {return 0.95;}

  virtual SparseDoubleVectorPtr getTransitionProbabilities(size_t state, size_t action, double& Z) const
  {
    SparseDoubleVectorPtr res = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
    Z = 1.0;

    if (state == 0)
    {
      size_t j = action + 1;
      res->setElement(j, 1.0 / j);
      res->setElement(0, 1.0 - 1.0 / j);
    }
    else
      res->setElement(0, 1.0);
    return res;
  }

  virtual double sampleReward(ExecutionContext& context, size_t state, size_t action, size_t nextState) const
    {return getRewardExpectation(state, action, nextState);}

  virtual double getRewardExpectation(size_t state, size_t action, size_t nextState) const
    {return action == 0 && state > 0 ? pow(1.5, (double)state) : 0.0;}
};

class LongChainMDP : public SmallMDP
{
public:
  virtual size_t getNumStates() const
    {return 20;}

  virtual size_t getNumActions() const
    {return 2;}

  virtual double getDiscount() const
    {return pow(0.5, 1.0 / (getNumStates() - 2));}

  virtual size_t getInitialState() const
    {return 1;}

  virtual SparseDoubleVectorPtr getTransitionProbabilities(size_t state, size_t action, double& Z) const
  {
    SparseDoubleVectorPtr res = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
    Z = 1.0;

    if (state >= 1 && state < getNumStates() - 1)
    {
      if (action == 0)
        res->setElement(state + 1, 1.0);
      else
        res->setElement(0, 1.0);
    }
    else if (state == getNumStates() - 1)
      res->setElement(0, 1.0);
    else if (state == 0)
    {
      res->setElement(getInitialState(), 0.025);
      res->setElement(state, 0.975);
    }
    return res;
  }

  virtual double sampleReward(ExecutionContext& context, size_t state, size_t action, size_t nextState) const
  {
    if (state >= 1 && state < getNumStates() - 1)
    {
      if (action == 0)
        return 0.0;
      else
        return 0.25;
    }
    else if (state == getNumStates() - 1)
      return context.getRandomGenerator()->sampleBool(0.75) ? 1.0 : 0.0;
    else if (state == 0)
      return 0.0;
    else
      return 0.0;
  }

  virtual double getRewardExpectation(size_t state, size_t action, size_t nextState) const
  {
    if (state >= 1 && state < getNumStates() - 1)
    {
      if (action == 0)
        return 0.0;
      else
        return 0.25;
    }
    else if (state == getNumStates() - 1)
      return 0.75;
    else if (state == 0)
      return 0.0;
    else
      return 0.0;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_H_
