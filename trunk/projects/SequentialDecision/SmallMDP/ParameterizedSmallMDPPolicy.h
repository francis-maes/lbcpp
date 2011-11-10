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
    q = new DoubleMatrix(numStates, numActions, getParameter(0) / (1.0 - mdp->getDiscount()));
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

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_PARAMETERIZED_POLICY_H_
