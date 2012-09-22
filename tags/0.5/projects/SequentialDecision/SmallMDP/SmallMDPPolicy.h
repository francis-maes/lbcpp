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
# include <lbcpp/Data/Matrix.h>
# include <lbcpp/Function/IterationFunction.h>

namespace lbcpp
{

class SmallMDPPolicy : public Object
{
public:
	// all of these three functions are called from the outside
  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp) {} // beginning of the episode (how many steps to run managed from the main program)
  virtual size_t selectAction(ExecutionContext& context, size_t state) = 0;
  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t newState, double reward) {} // called after transition has been sampled

protected:
	// get best action, and if there is more than one optimal action, choose one at random (hence "sample" best action)
  size_t sampleBestAction(ExecutionContext& context, const DoubleMatrixPtr& qValues, size_t state) const;
  double getBestQValue(const DoubleMatrixPtr& q, size_t state) const;
  double getBestQValueExpectation(const DoubleMatrixPtr& q, const SparseDoubleVectorPtr& stateProbabilities) const;
  DenseDoubleVectorPtr computeStateValuesFromActionValues(const DoubleMatrixPtr& q) const;
 
  // applies Bellman operator (is only used in OptimalSmallMDP)
  DoubleMatrixPtr bellmanOperator(const SmallMDPPtr& mdp, const DoubleMatrixPtr& q, double& differenceSumOfSquares, double& maxDifference) const;
  // computes Qstar (and calls bellmanOperator)
  DoubleMatrixPtr computeOptimalQFunction(ExecutionContext& context, const SmallMDPPtr& mdp) const;
  
  DenseDoubleVectorPtr createVector(size_t numElements, double initialValue = 0.0) const;
  DoubleMatrixPtr createMatrix(size_t numRows, size_t numColumns, double initialValue = 0.0) const;
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
  // IterationFunctionPtr represents a function that goes from positive integers (being the time index t) to real numbers (the eps_t), this specifies how epsilon should vary over time
  QLearningSmallMDPPolicy(IterationFunctionPtr epsilon, double w)
    : epsilon(epsilon), w(w) {}
  QLearningSmallMDPPolicy() {}

  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp)
  {
    // optimistic initialization
    q = createMatrix(mdp->getNumStates(), mdp->getNumActions(), 1.0 / (1.0 - mdp->getDiscount()));
    experienceCounts = createMatrix(mdp->getNumStates(), mdp->getNumActions(), 0.0);
    epoch = 1; // epoch is the current time step t
    discount = mdp->getDiscount();
  }

  virtual size_t selectAction(ExecutionContext& context, size_t state)
  {// eps greedy
    RandomGeneratorPtr random = context.getRandomGenerator();
    if (random->sampleBool(epsilon->computeIterationFunction(epoch)))
      return random->sampleSize(getNumActions()); // exploration step
    else
      return sampleBestAction(context, q, state); // exploitation step
  }

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t newState, double reward)
  {
    double n = experienceCounts->getValue(state, action); //getValue extracts one value from the matrix
    double alpha = pow(n + 1.0, -w);
    q->setValue(state, action, (1.0 - alpha) * q->getValue(state, action) + 
                                       alpha * (reward + discount * getBestQValue(q, newState)));
    ++epoch;
    experienceCounts->setValue(state, action, n + 1); // setValue sets one value in matrix (indices are from 0..nElements-1)
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
    q = createMatrix(numStates, numActions, 1.0 / (1.0 - mdp->getDiscount()));
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
    DoubleMatrixPtr res = createMatrix(ns, na);
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

class RTDPRMaxSmallMDPPolicy : public ModelBasedSmallMDPPolicy
{
public:
  RTDPRMaxSmallMDPPolicy(size_t m = 0)
    : m(m) {}

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t nextState, double reward)
  {
    static const double epsilon = 1e-9;

    if (model->getNumObservations(state, action) < m)
    {
      model->observeTransition(state, action, nextState, reward);
      if (model->getNumObservations(state, action) == m)
      {
        DenseDoubleVectorPtr v = computeStateValuesFromActionValues(q);
 
        double newValue = 0.0;
        double Z;
        SparseDoubleVectorPtr transitions = model->getTransitionProbabilities(state, action, Z);
        for (size_t k = 0; k < transitions->getNumValues(); ++k)
        {
          size_t nextState = transitions->getValue(k).first;
          double transitionProbability = transitions->getValue(k).second / Z;
          double r = model->getRewardExpectation(state, action, nextState);
          newValue += transitionProbability * (r + model->getDiscount() * v->getValue(nextState));
        }
        if (fabs(newValue - q->getValue(state, action)) > epsilon)
          q->setValue(state, action, newValue);
      }
    }
  }

protected:
  friend class RTDPRMaxSmallMDPPolicyClass;

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
    DoubleMatrixPtr res = createMatrix(ns, na);
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
