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

namespace lbcpp
{

// todo: use this in PowerDiscreteBanditPolicy
class DoubleVectorParameterized : public Parameterized
{
public:
  virtual SamplerPtr createParametersSampler() const
  {
    SamplerPtr scalarSampler = gaussianSampler(0.0, 1.0);
    return independentDoubleVectorSampler(parametersEnumeration, scalarSampler);
  }

  virtual void setParameters(const Variable& parameters)
    {this->parameters = parameters.getObjectAndCast<DenseDoubleVector>();}

  virtual Variable getParameters() const
    {return parameters;}

  virtual TypePtr getParametersType() const
    {return getParameters().getType();}

protected:
  EnumerationPtr parametersEnumeration;
  DenseDoubleVectorPtr parameters;

  void initializeParameters(EnumerationPtr parametersEnumeration)
  {
    this->parametersEnumeration = parametersEnumeration;
    parameters = new DenseDoubleVector(parametersEnumeration, doubleType);
  }
};

class ParameterizedQLearningSmallMDPPolicy : public SmallMDPPolicy, public DoubleVectorParameterized
{
public:
  ParameterizedQLearningSmallMDPPolicy(size_t numTerms = 0)
    : numTerms(numTerms)
  {
    initializeParameters(createParametersEnumeration());
  }

  virtual ObjectPtr computeGeneratedObject(ExecutionContext& context, const String& variableName)
    {return createParametersEnumeration();}

  EnumerationPtr createParametersEnumeration()
  {
    ExecutionContext& context = defaultExecutionContext();

    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("parameters"));

    static const size_t numVariables = 5;
    static const char* names[numVariables] = {"Q^t(s,a)", "r + lambda.V(s')", "r_{emp}(s,a)", "s_{emp}(s,a)", "n(s,a)"};

    for (size_t i = 0; i < numTerms; ++i)
    {
      String prefix = T("term(") + String((int)i) + T(", ");
      parametersEnumeration->addElement(context, prefix + T("weight)"));
      for (size_t j = 0; j < numVariables; ++j)
        parametersEnumeration->addElement(context, prefix + T("pow ") + names[j] + T(")"));
    }
    return parametersEnumeration;
  }

  virtual void initialize(ExecutionContext& context, const SmallMDPPtr& mdp)
  {
    size_t numStates = mdp->getNumStates();
    size_t numActions = mdp->getNumActions();

    q = new DoubleMatrix(numStates, numActions, 1.0 / (1.0 - mdp->getDiscount()));
    rewards = std::vector<std::vector<ScalarVariableStatistics> >(numStates, std::vector<ScalarVariableStatistics>(numActions));
    epoch = 1;
    discount = mdp->getDiscount();
  }

  virtual size_t selectAction(ExecutionContext& context, size_t state)
    {return sampleBestAction(context, q, state);}

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t newState, double reward)
  {
    ++epoch;
    rewards[state][action].push(reward);

    std::vector<double> variables(5);
    variables[0] = q->getValue(state, action);
    variables[1] = reward + discount * getBestQValue(q, newState);
    variables[2] = rewards[state][action].getCount();
    variables[3] = rewards[state][action].getMean();
    variables[4] = rewards[state][action].getStandardDeviation();
    // todo: add epoch ?
    
    double newValue = 0.0;
    size_t index = 0;
    for (size_t i = 0; i < numTerms; ++i)
    {
      double term = parameters->getValue(index++);
      for (size_t j = 0; j < variables.size(); ++j)
        term *= pow(variables[j], parameters->getValue(index++));
      newValue += term;
    }
    q->setValue(state, action, newValue);
  }

protected:
  friend class ParameterizedQLearningSmallMDPPolicyClass;

  size_t numTerms;

  std::vector<std::vector<ScalarVariableStatistics> > rewards;
  DoubleMatrixPtr q;
  size_t epoch;
  double discount;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_PARAMETERIZED_POLICY_H_
