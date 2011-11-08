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
  SmallMDP(size_t numStates, size_t numActions, double discount)
    : model(numStates, std::vector<StateActionInfo>(numActions)), discount(discount)
  {
  }
  SmallMDP() {}

  size_t getNumStates() const
    {return model.size();}
    
  size_t getNumActions() const
    {return model[0].size();}
  
  void setInfo(size_t state, size_t action, const SamplerPtr& reward, const SparseDoubleVectorPtr& nextStates)
  {
    jassert(state < getNumStates() && action < getNumActions());
    StateActionInfo& info = model[state][action];
    info.reward = reward;
    info.nextStates = nextStates;
  }
  
  const SparseDoubleVectorPtr& getTransitions(size_t state, size_t action) const
    {return model[state][action].nextStates;}
  
  double getRewardExpectation(size_t state, size_t action, size_t nextState) const
    {return model[state][action].reward->computeExpectation().getDouble();}
  
  size_t sampleTransition(ExecutionContext& context, size_t state, size_t action, double& reward) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    const StateActionInfo& info = model[state][action];
    reward = info.reward->sample(context, random).toDouble();
    
    double p = random->sampleDouble();
    for (size_t i = 0; i < info.nextStates->getNumValues(); ++i)
    {
      std::pair<size_t, double> tr = info.nextStates->getValue(i);
      if (p <= tr.second)
        return tr.first;
      p -= tr.second;
    }
    jassert(false);
    return (size_t)-1;
  }

  double getDiscount() const
    {return discount;}
  
protected:  
  struct StateActionInfo
  {
    SamplerPtr reward;
    SparseDoubleVectorPtr nextStates;
  };

  std::vector< std::vector< StateActionInfo > > model; // s -> a -> model
  double discount;
};

typedef ReferenceCountedObjectPtr<SmallMDP> SmallMDPPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_H_
