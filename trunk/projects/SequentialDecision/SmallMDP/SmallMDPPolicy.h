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

class ModelBasedSmallMDPPolicy : public SmallMDPPolicy
{
public:
  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp)
  {
    size_t numStates = mdp->getNumStates();
    size_t numActions = mdp->getNumActions();

    model = new EmpiricalSmallMDP(numStates, numActions, mdp->getDiscount());
    q = new DoubleMatrix(numStates, numActions, 1.0 / (1.0 - mdp->getDiscount()));
  }

  virtual size_t selectAction(ExecutionContext& context, size_t state)
    {return sampleBestAction(context, q, state);}

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t nextState, double reward)
    {model->observeTransition(state, action, nextState, reward);}

protected:
  EmpiricalSmallMDPPtr model;
  DoubleMatrixPtr q;
};

typedef ReferenceCountedObjectPtr<ModelBasedSmallMDPPolicy> ModelBasedSmallMDPPolicyPtr;

class RMaxSmallMDPPolicy : public ModelBasedSmallMDPPolicy
{
public:
  RMaxSmallMDPPolicy(size_t m = 0)
    : m(m) {}

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t nextState, double reward)
  {
    if (model->getNumObservations(state, action) < m)
    {
      model->observeTransition(state, action, nextState, reward);
      if (model->getNumObservations(state, action) == m)
      {
        while (true)
        {
          static const double epsilon = 1e-9;
          double residual = updateActionValues();
          if (residual < epsilon)
            break;
        }
      }
    }
  }

  double updateActionValues()
  {
    size_t ns = model->getNumStates();
    size_t na = model->getNumActions();
    
    DenseDoubleVectorPtr v = computeStateValuesFromActionValues(q);
    
    double residual = 0.0;
    DoubleMatrixPtr res(new DoubleMatrix(ns, na));
    for (size_t i = 0; i < ns; ++i)
      for (size_t j = 0; j < na; ++j)
        if (model->getNumObservations(i, j) == m)
        {
          // Q_{t+1}(s,a) = sum_{s'} [ P(s'|s,a) (r(s,a,s') + discount * V(s')) ]
          // with V(s) = max_a Q(s,a)
          double newValue = 0.0;
          double Z;
          SparseDoubleVectorPtr transitions = model->getTransitionProbabilities(i, j, Z);
          for (size_t k = 0; k < transitions->getNumValues(); ++k)
          {
            size_t nextState = transitions->getValue(k).first;
            double transitionProbability = transitions->getValue(k).second / Z;
            double r = model->getRewardExpectation(i, j, nextState);
            newValue += transitionProbability * (r + model->getDiscount() * v->getValue(nextState));
          }
          double previousValue = q->getValue(i, j);
          residual = juce::jmax(residual, fabs(newValue - previousValue));
          res->setValue(i, j, newValue);
        }
        else
          res->setValue(i, j, q->getValue(i, j));

    q = res;
    return residual;
  }

protected:
  friend class RMaxSmallMDPPolicyClass;

  size_t m;
};

class MBIEEBSmallMDPPolicy : public ModelBasedSmallMDPPolicy
{
public:
  MBIEEBSmallMDPPolicy(size_t m = 0, double beta = 0.0)
    : m(m), beta(beta) {}

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t nextState, double reward)
  {
    if (model->getNumObservations(state, action) < m)
    {
      size_t i = 0;
      model->observeTransition(state, action, nextState, reward);
      while (true)
      {
        static const double epsilon = 1e-9;
        double residual = updateActionValues();
        if (residual < epsilon)
          break;
      }
    }
  }

  double updateActionValues()
  {
    size_t ns = model->getNumStates();
    size_t na = model->getNumActions();
    
    DenseDoubleVectorPtr v = computeStateValuesFromActionValues(q);
    
    double residual = 0.0;
    DoubleMatrixPtr res(new DoubleMatrix(ns, na));
    for (size_t i = 0; i < ns; ++i)
      for (size_t j = 0; j < na; ++j)
        if (model->getNumObservations(i, j) > 0)
        {
          // Q_{t+1}(s,a) = sum_{s'} [ P(s'|s,a) (r(s,a,s') + discount * V(s')) ] + beta / sqrt(n(s,a))
          // with V(s) = max_a Q(s,a)
          double newValue = 0.0;
          double Z;
          SparseDoubleVectorPtr transitions = model->getTransitionProbabilities(i, j, Z);
          for (size_t k = 0; k < transitions->getNumValues(); ++k)
          {
            size_t nextState = transitions->getValue(k).first;
            double transitionProbability = transitions->getValue(k).second / Z;
            double r = model->getRewardExpectation(i, j, nextState);
            newValue += transitionProbability * (r + model->getDiscount() * v->getValue(nextState));
          }
          size_t nsa = model->getNumObservations(i, j);
          newValue += beta / sqrt((double)nsa); // FIXME: what should we do when n(s,a) == 0 ??
          double previousValue = q->getValue(i, j);
          residual = juce::jmax(residual, fabs(newValue - previousValue));
          res->setValue(i, j, newValue);
        }
        else
          res->setValue(i, j, q->getValue(i, j));

    q = res;
    return residual;
  }

protected:
  friend class MBIEEBSmallMDPPolicyClass;

  size_t m;
  double beta;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_POLICY_H_
