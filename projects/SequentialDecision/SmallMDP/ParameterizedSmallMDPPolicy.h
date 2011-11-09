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
  ParameterizedQLearningSmallMDPPolicy(size_t numTerms)
    : numTerms(numTerms)
    {initializeParameters(createParametersEnumeration());}
  ParameterizedQLearningSmallMDPPolicy() {}

  virtual ObjectPtr computeGeneratedObject(ExecutionContext& context, const String& variableName)
    {return createParametersEnumeration();}

  EnumerationPtr createParametersEnumeration()
  {
    ExecutionContext& context = defaultExecutionContext();

    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("parameters"));
    for (size_t i = 0; i < 10; ++i)
      parametersEnumeration->addElement(context, "param" + String((int)i));


/*    static const size_t numVariables = 5;
    static const char* names[numVariables] = {"Q^t(s,a)", "r + lambda.V(s')", "r_{emp}(s,a)", "s_{emp}(s,a)", "n(s,a)"};

    for (size_t i = 0; i < numTerms; ++i)
    {
      String prefix = T("term(") + String((int)i) + T(", ");
      parametersEnumeration->addElement(context, prefix + T("weight)"));
      for (size_t j = 0; j < numVariables; ++j)
        parametersEnumeration->addElement(context, prefix + T("pow ") + names[j] + T(")"));
    }*/

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
    
    /*double newValue = 0.0;
    size_t index = 0;
    for (size_t i = 0; i < numTerms; ++i)
    {
      double term = parameters->getValue(index++);
      for (size_t j = 0; j < 3; ++j)
        term *= myPow(variables[j], parameters->getValue(index++));
      newValue += term;
    }
    //for (size_t i = 0; i < variables.size(); ++i)
    //  newValue += parameters->getValue(i*2) * myPow(variables[i], parameters->getValue(i*2+1));
    */

    double alpha = 0.0;
    for (size_t i = 0; i < variables.size(); ++i)
      alpha += parameters->getValue(i*2) * myPow(variables[i], parameters->getValue(i*2+1));
    double newValue = (1.0 - alpha) * q->getValue(state, action) + 
                       alpha * (reward + discount * getBestQValue(q, newState));

    q->setValue(state, action, newValue);
  }

  static double myPow(double a, double b)
    {return a == 0.0 ? 0.0 : pow(a, b);}

protected:
  friend class ParameterizedQLearningSmallMDPPolicyClass;

  size_t numTerms;

  std::vector<std::vector<ScalarVariableStatistics> > rewards;
  DoubleMatrixPtr q;
  size_t epoch;
  double discount;
};


class ParameterizedRTDPRMaxSmallMDPPolicy : public ModelBasedSmallMDPPolicy, public DoubleVectorParameterized
{
public:
  ParameterizedRTDPRMaxSmallMDPPolicy(size_t numTerms)
    : numTerms(numTerms)
    {initializeParameters(createParametersEnumeration());}
  ParameterizedRTDPRMaxSmallMDPPolicy() {}

  virtual ObjectPtr computeGeneratedObject(ExecutionContext& context, const String& variableName)
    {return createParametersEnumeration();}

  EnumerationPtr createParametersEnumeration()
  {
    ExecutionContext& context = defaultExecutionContext();

    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("parameters"));
    for (size_t i = 0; i < 10; ++i)
      parametersEnumeration->addElement(context, "param" + String((int)i));
    return parametersEnumeration;
  }

  virtual void observeTransition(ExecutionContext& context, size_t state, size_t action, size_t nextState, double reward)
  {
    static const double epsilon = 1e-9;
    model->observeTransition(state, action, nextState, reward);
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
    
    std::vector<double> variables(3);
    variables[0] = q->getValue(state, action);
    variables[1] = newValue;
    variables[2] = model->getNumObservations(state, action);

    double newQ = 0.0;
    size_t index = 0;
    for (size_t i = 0; i < numTerms; ++i)
    {
      double term = parameters->getValue(index++);
      for (size_t j = 0; j < variables.size(); ++j)
        term *= pow(variables[j], parameters->getValue(index++));
      newQ += term;
    }
    q->setValue(state, action, newQ);
  }

protected:
  friend class ParameterizedRTDPRMaxSmallMDPPolicyClass;

  size_t numTerms;
};


}; /* namespace lbcpp */

#endif // !LBCPP_SMALL_MDP_PARAMETERIZED_POLICY_H_
