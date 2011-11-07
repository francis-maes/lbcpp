/*-----------------------------------------.---------------------------------.
| Filename: SmallMDPSandBox.h              | Small MDP Sand Box              |
| Author  : Francis Maes                   |                                 |
| Started : 07/11/2011 16:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SMALL_MDP_SANDBOX_H_
# define LBCPP_SMALL_MDP_SANDBOX_H_

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

class SmallMDPSandBox : public WorkUnit
{
public:
  SmallMDPSandBox() : numStates(20), numActions(4), discount(0.9), numSuccessorsPerState(5) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    SmallMDPPtr mdp = sampleMDP(context.getRandomGenerator());
    DoubleMatrixPtr optimalQValues = computeOptimalQFunction(context, mdp);
    context.resultCallback("qstar", optimalQValues);
    
    return true;
  }
  
protected:
  friend class SmallMDPSandBoxClass;
  
  size_t numStates;
  size_t numActions;
  double discount;
  size_t numSuccessorsPerState;
  
  SmallMDPPtr sampleMDP(RandomGeneratorPtr randomGenerator) const
  {
    SmallMDPPtr res(new SmallMDP(numStates, numActions, discount));
    for (size_t i = 0; i < numStates; ++i)
      for (size_t j = 0; j < numActions; ++j)
      {
        SamplerPtr reward = bernoulliSampler(randomGenerator->sampleDouble());
        SparseDoubleVectorPtr transitions = new SparseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType);
        
        std::vector<size_t> order;
        randomGenerator->sampleOrder(numStates, order);
        double Z = 0.0;
        for (size_t k = 0; k < numSuccessorsPerState; ++k)
        {
          double p = randomGenerator->sampleDouble();
          transitions->setElement(order[k], p);
          Z += p;
        }
        transitions->multiplyByScalar(1.0 / Z);
        res->setInfo(i, j, reward, transitions);
      }
    return res;
  }
  
  DoubleMatrixPtr computeOptimalQFunction(ExecutionContext& context, const SmallMDPPtr& mdp) const
  {
    static const double epsilon = 1e-12;
    size_t maxIterations = mdp->getNumStates() * mdp->getNumActions() * 100;
  
    DoubleMatrixPtr res(new DoubleMatrix(mdp->getNumStates(), mdp->getNumActions()));
    for (size_t i = 0; i < maxIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      double differenceSumOfSquares, maxDifference;
      res = bellmanOperator(mdp, res, differenceSumOfSquares, maxDifference);
      context.resultCallback(T("Iteration"), i);
      context.resultCallback(T("differenceSumOfSquares"), differenceSumOfSquares);
      context.resultCallback(T("maxDifference"), maxDifference);
      //context.resultCallback(T("Qfunction"), res->cloneAndCast<DoubleMatrix>());
      context.leaveScope();
      
      if (differenceSumOfSquares < epsilon)
        break;
    }
    return res;
  }
  
  DenseDoubleVectorPtr computeStateValuesFromActionValues(const DoubleMatrixPtr& q) const
  {
    size_t ns = q->getNumRows();
    size_t na = q->getNumColumns();
    
    DenseDoubleVectorPtr res(new DenseDoubleVector(ns, 0.0));
    for (size_t i = 0; i < ns; ++i)
    {
      // V(s) = max_a Q(s,a)
      double bestValue = -DBL_MAX;
      for (size_t j = 0; j < na; ++j)
        bestValue = juce::jmax(bestValue, q->getValue(i, j));
      res->setValue(i, bestValue);
    }
    return res;
  }
  
  DoubleMatrixPtr bellmanOperator(const SmallMDPPtr& mdp, const DoubleMatrixPtr& q, double& differenceSumOfSquares, double& maxDifference) const
  {
    differenceSumOfSquares = 0.0;
    maxDifference = 0.0;
    
    size_t ns = mdp->getNumStates();
    size_t na = mdp->getNumActions();
    
    DenseDoubleVectorPtr v = computeStateValuesFromActionValues(q);
    
    DoubleMatrixPtr res(new DoubleMatrix(ns, na));
    for (size_t i = 0; i < ns; ++i)
      for (size_t j = 0; j < na; ++j)
      {
        // Q_{t+1}(s,a) = sum_{s'} [ P(s'|s,a) (r(s,a,s') + discount * V(s')) ]
        // with V(s) = max_a Q(s,a)
        double newValue = 0.0;
        SparseDoubleVectorPtr transitions = mdp->getTransitions(i, j);
        for (size_t k = 0; k < transitions->getNumValues(); ++k)
        {
          size_t nextState = transitions->getValue(k).first;
          double transitionProbability = transitions->getValue(k).second;
          
          double r = mdp->getRewardExpectation(i, j, nextState);
          newValue += transitionProbability * (r + mdp->getDiscount() * v->getValue(nextState));
        }
        double previousValue = q->getValue(i, j);
        differenceSumOfSquares += (newValue - previousValue) * (newValue - previousValue);
        maxDifference = juce::jmax(maxDifference, fabs(newValue - previousValue));
        res->setValue(i, j, newValue);
      }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_SANDBOX_H_

