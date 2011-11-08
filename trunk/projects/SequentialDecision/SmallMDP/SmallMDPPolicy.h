/*-----------------------------------------.---------------------------------.
| Filename: SmallMDPPolicy.h               | Small MDP Policy                |
| Author  : Francis Maes                   |                                 |
| Started : 08/11/2011 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SMALL_MDP_POLICY_H_
# define LBCPP_SMALL_MDP_POLICY_H_

# include "SmallMDP.h"

namespace lbcpp
{

class SmallMDPPolicy : public Object
{
public:
  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp) {}
  virtual size_t selectAction(ExecutionContext& context, size_t state) = 0;
  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t newState, double reward) {}

protected:
  size_t sampleBestAction(ExecutionContext& context, const DoubleMatrixPtr& qValues, size_t state) const;
  DoubleMatrixPtr computeOptimalQFunction(ExecutionContext& context, const SmallMDPPtr& mdp) const;
  double getBestQValue(const DoubleMatrixPtr& q, size_t state) const;
  DenseDoubleVectorPtr computeStateValuesFromActionValues(const DoubleMatrixPtr& q) const;
  DoubleMatrixPtr bellmanOperator(const SmallMDPPtr& mdp, const DoubleMatrixPtr& q, double& differenceSumOfSquares, double& maxDifference) const;
};

typedef ReferenceCountedObjectPtr<SmallMDPPolicy> SmallMDPPolicyPtr;

class OptimalSmallMDPPolicy : public SmallMDPPolicy
{
public:
  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp)
  {
    qStar = computeOptimalQFunction(context, mdp);
    context.resultCallback(T("qstar"), qStar);
  }

  virtual size_t selectAction(ExecutionContext& context, size_t state)
    {return sampleBestAction(context, qStar, state);}

protected:
  DoubleMatrixPtr qStar;
};

class RandomSmallMDPPolicy : public SmallMDPPolicy
{
public:
  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp)
    {numActions = mdp->getNumActions();}

  virtual size_t selectAction(ExecutionContext& context, size_t state)
    {return context.getRandomGenerator()->sampleSize(numActions);}

protected:
  size_t numActions;
};

class QLearningSmallMDPPolicy : public SmallMDPPolicy
{
public:
  QLearningSmallMDPPolicy(IterationFunctionPtr epsilon, double w)
    : epsilon(epsilon), w(w) {}
  QLearningSmallMDPPolicy() {}

  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp)
  {
    // optimistic initialization
    q = new DoubleMatrix(mdp->getNumStates(), mdp->getNumActions(), 1.0 / (1.0 - mdp->getDiscount()));
    experienceCounts = new DoubleMatrix(mdp->getNumStates(), mdp->getNumActions(), 0.0);
    epoch = 1;
    discount = mdp->getDiscount();
  }

  virtual size_t selectAction(ExecutionContext& context, size_t state)
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    if (random->sampleBool(epsilon->computeIterationFunction(epoch)))
      return random->sampleSize(getNumActions()); // exploration step
    else
      return sampleBestAction(context, q, state); // exploitation step
  }

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t newState, double reward)
  {
    double n = experienceCounts->getValue(state, action);
    double alpha = pow(n + 1.0, -w);
    q->setValue(state, action, (1.0 - alpha) * q->getValue(state, action) + 
                                       alpha * (reward + discount * getBestQValue(q, newState)));
    ++epoch;
    experienceCounts->setValue(state, action, n + 1);
  }

protected:
  friend class QLearningSmallMDPPolicyClass;

  IterationFunctionPtr epsilon;
  double w;

  DoubleMatrixPtr experienceCounts;
  DoubleMatrixPtr q;
  size_t epoch;
  double discount;

  size_t getNumActions() const
    {return q->getNumColumns();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_POLICY_H_
