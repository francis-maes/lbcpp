/*-----------------------------------------.---------------------------------.
| Filename: ParameterizedSmallMDPPolicy.h  | Parameterized Small MDP Policy  |
| Author  : Francis Maes                   |                                 |
| Started : 08/11/2011 16:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SMALL_MDP_PARAMETERIZED_POLICY_H_
# define LBCPP_SMALL_MDP_PARAMETERIZED_POLICY_H_

# include "SmallMDPPolicy.h"
# include "../Bandits/DiscreteBanditPolicy.h" // for Parameterized
# include "../GP/GPExpression.h"

namespace lbcpp
{

class GPExpressionSmallMDPPolicy : public ModelBasedSmallMDPPolicy
{
public:
  GPExpressionSmallMDPPolicy(GPExpressionPtr expression, bool useModel)
    : expression(expression), useModel(useModel) {}
  GPExpressionSmallMDPPolicy() {}

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t nextState, double reward)
  {
    model->observeTransition(state, action, nextState, reward);
    
    double variables[4];
    variables[0] = q->getValue(state, action);
    if (useModel)
    {
      variables[1] = model->getRewardExpectation(state, action, nextState);
      double Z;
      SparseDoubleVectorPtr transitions = model->getTransitionProbabilities(state, action, Z);
      variables[2] = model->getDiscount() * getBestQValueExpectation(q, transitions) / Z;
    }
    else
    {
      variables[1] = reward;
      variables[2] = model->getDiscount() * getBestQValue(q, nextState);
    }
    variables[3] = model->getNumObservations(state, action);

    q->setValue(state, action, expression->compute(variables));
  }

protected:
  friend class GPExpressionSmallMDPPolicyClass;

  GPExpressionPtr expression;
  bool useModel;
};

class ParameterizedSmallMDPPolicy : public ModelBasedSmallMDPPolicy, public DoubleVectorParameterized
{
public:
  ParameterizedSmallMDPPolicy(size_t numParameters, bool useModel)
    : numParameters(numParameters), useModel(useModel)
  {
    initializeParameters(createParametersEnumeration());
  }
  ParameterizedSmallMDPPolicy() {}

  virtual ObjectPtr computeGeneratedObject(ExecutionContext& context, const String& variableName)
    {return createParametersEnumeration();}

  EnumerationPtr createParametersEnumeration()
  {
    ExecutionContext& context = defaultExecutionContext();

    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("parameters"));
    parametersEnumeration->addElement(context, "init");
    parametersEnumeration->addElement(context, "offset");
    parametersEnumeration->addElement(context, "r_weight");
    parametersEnumeration->addElement(context, "r_pow");
    parametersEnumeration->addElement(context, "next_weight");
    parametersEnumeration->addElement(context, "next_pow");
    parametersEnumeration->addElement(context, "qt_weight");
    parametersEnumeration->addElement(context, "qt_pow");
    parametersEnumeration->addElement(context, "n_weight");
    parametersEnumeration->addElement(context, "n_pow");
    return parametersEnumeration;
  }

  double getParameter(size_t index) const
    {return index < numParameters ? parameters->getValue(index) : 0.0;}

  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp)
  {
    size_t numStates = mdp->getNumStates();
    size_t numActions = mdp->getNumActions();

    model = new EmpiricalSmallMDP(numStates, numActions, mdp->getDiscount());
    q = createMatrix(numStates, numActions, getParameter(0) / (1.0 - mdp->getDiscount()));
  }

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t nextState, double reward)
  {
    model->observeTransition(state, action, nextState, reward);
    
    double variables[4];
    if (useModel)
    {
      variables[0] = model->getRewardExpectation(state, action, nextState);
      double Z;
      SparseDoubleVectorPtr transitions = model->getTransitionProbabilities(state, action, Z);
      variables[1] = model->getDiscount() * getBestQValueExpectation(q, transitions) / Z;
    }
    else
    {
      variables[0] = reward;
      variables[1] = model->getDiscount() * getBestQValue(q, nextState);
    }
    variables[2] = q->getValue(state, action);
    variables[3] = model->getNumObservations(state, action);

    double newValue = getParameter(1);
    for (size_t i = 0; i < 4; ++i)
    {
      double weight = getParameter(i*2+2);
      if (weight)
        newValue += weight * myPow(variables[i], getParameter(i*2+3));
    }

    q->setValue(state, action, newValue);
  }

  static double myPow(double a, double b)
    {return a == 0.0 ? 0.0 : pow(a, b);}

protected:
  friend class ParameterizedSmallMDPPolicyClass;

  size_t numParameters;
  bool useModel;
};


class ParameterizedModelBasedSmallMDPPolicy : public ModelBasedSmallMDPPolicy, public DoubleVectorParameterized
{
public:
  ParameterizedModelBasedSmallMDPPolicy(size_t numParameters) : numParameters(numParameters)
  {
    initializeParameters(createParametersEnumeration());
  }
  ParameterizedModelBasedSmallMDPPolicy() {}

  virtual ObjectPtr computeGeneratedObject(ExecutionContext& context, const String& variableName)
    {return createParametersEnumeration();}

  EnumerationPtr createParametersEnumeration()
  {
    ExecutionContext& context = defaultExecutionContext();
    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("parameters"));
    for (size_t i = 0; i < numParameters; ++i)
      parametersEnumeration->addElement(context, "param" + String((int)i+1));
    return parametersEnumeration;
  }

  double computeActionScore(size_t state, size_t action) const
  {
    size_t tk = model->getNumObservations(state, action);
    double qScore = q->getValue(state, action);
    double reward = model->getRewardExpectation(state, action, (size_t)-1);

    double res = 0.0;
    if (numParameters >= 1)
      res += pow((double)tk, parameters->getValue(0));
    if (numParameters >= 2)
    {
      res *= parameters->getValue(1);
      if (numParameters >= 3)
        res += parameters->getValue(2) * reward;
    }
    if (numParameters >= 4)
      res += pow(qScore, parameters->getValue(3));
    else
      res += qScore;
    return res;
  }

  virtual size_t selectAction(ExecutionContext& context, size_t state)
  {
    double bestScore = -DBL_MAX;
    size_t bestAction = 0;
    for (size_t i = 0; i < model->getNumActions(); ++i)
    {
      double score = computeActionScore(state, i);
      if (score > bestScore)
        bestScore = score, bestAction = i;

    }
    return bestAction;
  }

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t nextState, double reward)
  {
    model->observeTransition(state, action, nextState, reward);
    while (true)
    {
      static const double epsilon = 1e-6;
      double residual = updateActionValues();
      if (residual < epsilon)
        break;
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
        if (model->getNumObservations(i, j))
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
  friend class ParameterizedModelBasedSmallMDPPolicyClass;

  size_t numParameters;
};


}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_PARAMETERIZED_POLICY_H_
