/*-----------------------------------------.---------------------------------.
| Filename: SmallMDPPolicy.cpp             | Small MDP Policy                |
| Author  : Francis Maes                   |                                 |
| Started : 08/11/2011 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "SmallMDPPolicy.h"
using namespace lbcpp;

DenseDoubleVectorPtr SmallMDPPolicy::createVector(size_t numElements, double initialValue) const
{
  return new DenseDoubleVector(numElements, initialValue);
}

DoubleMatrixPtr SmallMDPPolicy::createMatrix(size_t numRows, size_t numColumns, double initialValue) const
{
  static ClassPtr matrixClass = doubleMatrixClass(doubleType);
  return new DoubleMatrix(matrixClass, numRows, numColumns, initialValue);
}

size_t SmallMDPPolicy::sampleBestAction(ExecutionContext& context, const DoubleMatrixPtr& qValues, size_t state) const
{
  std::set<size_t> bestActions;
  double bestValue = -DBL_MAX;
  size_t n = qValues->getNumColumns();
  for (size_t i = 0; i < n; ++i)
  {
    double value = qValues->getValue(state, i);
    if (value >= bestValue)
    {
      if (value > bestValue)
      {
        bestValue = value;
        bestActions.clear();
      }
      bestActions.insert(i);
    }
  }
  if (bestActions.empty())
    return context.getRandomGenerator()->sampleSize(n); // random action
  jassert(bestActions.size());
  n = context.getRandomGenerator()->sampleSize(bestActions.size());
  //this just gets element at location index n (set is implemented as a list, so we cannot access elements at random)
  std::set<size_t>::const_iterator it = bestActions.begin();
  for (size_t i = 0; i < n; ++i)
    ++it;
  return *it;
}

DoubleMatrixPtr SmallMDPPolicy::computeOptimalQFunction(ExecutionContext& context, const SmallMDPPtr& mdp) const
{
  static const double epsilon = 1e-9;

  DoubleMatrixPtr res(new DoubleMatrix(mdp->getNumStates(), mdp->getNumActions()));
  while (true)
  {
   // context.enterScope(T("Iteration ") + String((int)i));
    double differenceSumOfSquares, maxDifference;
    res = bellmanOperator(mdp, res, differenceSumOfSquares, maxDifference);
   /* context.resultCallback(T("Iteration"), i);
    context.resultCallback(T("differenceSumOfSquares"), differenceSumOfSquares);
    context.resultCallback(T("maxDifference"), maxDifference);
    //context.resultCallback(T("Qfunction"), res->cloneAndCast<DoubleMatrix>());
    context.leaveScope();*/
    
    if (maxDifference < epsilon)
      break;
  }
  return res;
}

double SmallMDPPolicy::getBestQValue(const DoubleMatrixPtr& q, size_t state) const
{
  size_t numActions = q->getNumColumns();
  double res = -DBL_MAX;
  for (size_t i = 0; i < numActions; ++i)
    res = juce::jmax(res, q->getValue(state, i));
  return res;
}

double SmallMDPPolicy::getBestQValueExpectation(const DoubleMatrixPtr& q, const SparseDoubleVectorPtr& stateProbabilities) const
{
  double res = 0.0;
  for (size_t i = 0; i < stateProbabilities->getNumValues(); ++i)
  {
    size_t state = stateProbabilities->getValue(i).first;
    double probability = stateProbabilities->getValue(i).second;
    res += probability * getBestQValue(q, state);
  }
  return res;
}

DenseDoubleVectorPtr SmallMDPPolicy::computeStateValuesFromActionValues(const DoubleMatrixPtr& q) const
{
  size_t numStates = q->getNumRows();
  DenseDoubleVectorPtr res = createVector(numStates);
  for (size_t i = 0; i < numStates; ++i)
    res->setValue(i, getBestQValue(q, i)); // V(s) = max_a Q(s,a)
  return res;
}

DoubleMatrixPtr SmallMDPPolicy::bellmanOperator(const SmallMDPPtr& mdp, const DoubleMatrixPtr& q, double& differenceSumOfSquares, double& maxDifference) const
{
  differenceSumOfSquares = 0.0;
  maxDifference = 0.0;
  
  size_t ns = mdp->getNumStates();
  size_t na = mdp->getNumActions();
  
  DenseDoubleVectorPtr v = computeStateValuesFromActionValues(q);
  
  DoubleMatrixPtr res = createMatrix(ns, na);
  for (size_t i = 0; i < ns; ++i)
    for (size_t j = 0; j < na; ++j)
    {
      // Q_{t+1}(s,a) = sum_{s'} [ P(s'|s,a) (r(s,a,s') + discount * V(s')) ]
      // with V(s) = max_a Q(s,a)
      double newValue = 0.0;
      double Z;
      SparseDoubleVectorPtr transitions = mdp->getTransitionProbabilities(i, j, Z);
      for (size_t k = 0; k < transitions->getNumValues(); ++k)
      {
        size_t nextState = transitions->getValue(k).first;
        double transitionProbability = transitions->getValue(k).second / Z;
        
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
