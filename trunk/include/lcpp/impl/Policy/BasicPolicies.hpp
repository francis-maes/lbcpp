/*-----------------------------------------.---------------------------------.
| Filename: BasicPolicies.hpp              | Simple common policies          |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_POLICY_BASIC_POLICIES_H_
# define LCPP_CORE_IMPL_POLICY_BASIC_POLICIES_H_

# include "PolicyStatic.hpp"

namespace lcpp {
namespace impl {

struct RandomPolicy : public Policy<RandomPolicy>
{
  VariablePtr policyChoose(ChoosePtr choose)
    {return choose->sampleRandomChoice();}
};

struct GreedyPolicy : public Policy<GreedyPolicy>
{
  GreedyPolicy(ActionValueFunctionPtr actionValue)
    : actionValue(actionValue) {}
    
  ActionValueFunctionPtr actionValue;

  VariablePtr policyChoose(ChoosePtr choose)
    {return choose->sampleBestChoice(actionValue);}
  
  void save(std::ostream& ostr) const
    {write(ostr, actionValue);}

  bool load(std::istream& istr)
    {return read(istr, actionValue);}
};

struct NonDeterministicPolicy : public Policy<NonDeterministicPolicy>
{
  NonDeterministicPolicy(ActionValueFunctionPtr actionProbabilities)
    : actionProbabilities(actionProbabilities) {}
    
  ActionValueFunctionPtr actionProbabilities;

  VariablePtr policyChoose(ChoosePtr choose)
  {
    std::vector<double> probabilities;
    choose->computeActionValues(probabilities, actionProbabilities);
#ifdef NDEBUG
    double sum = 0.0;
    for (size_t i = 0; i < probabilities.size(); ++i)
      sum += probabilities[i];
    assert(fabs(sum - 1.0) < 0.000001);
#endif
    return choose->sampleChoiceWithProbabilities(probabilities, 1.0);
  }
  
  void save(std::ostream& ostr) const
    {write(ostr, actionProbabilities);}

  bool load(std::istream& istr)
    {return read(istr, actionProbabilities);}
};

struct GibbsGreedyPolicy : public Policy<GibbsGreedyPolicy>
{
  GibbsGreedyPolicy(ActionValueFunctionPtr actionValue, IterationFunctionPtr temperature)
    : actionValue(actionValue), temperature(temperature) {}

  // P[a | s] = exp(Q(s,a) / T)
  VariablePtr policyChoose(ChoosePtr choose)
  {
    double T = temperature->compute(numChooses);
    assert(T);
    ++numChooses;

    std::vector<double> actionValues;
    choose->computeActionValues(actionValues, actionValue);
    double sum = 0;
    for (size_t i = 0; i < actionValues.size(); ++i)
    {
      double p = exp(actionValues[i] / T);
      sum += p;
      actionValues[i] = p;
    }
    return choose->sampleChoiceWithProbabilities(actionValues, sum);
  }
  
  void save(std::ostream& ostr) const
    {write(ostr, actionValue);}

  bool load(std::istream& istr)
    {return read(istr, actionValue);}
    
private:
  size_t numChooses;
  ActionValueFunctionPtr actionValue;
  IterationFunctionPtr temperature;
};

template<class DecoratedType>
struct EpsilonGreedyPolicy
  : public DecoratorPolicy<EpsilonGreedyPolicy<DecoratedType>, DecoratedType>
{
  typedef DecoratorPolicy<EpsilonGreedyPolicy, DecoratedType> BaseClass;
  
  EpsilonGreedyPolicy(const DecoratedType& decorated, IterationFunctionPtr epsilon)
    : BaseClass(decorated), numChooses(0), epsilon(epsilon) {}

  VariablePtr policyChoose(ChoosePtr choose)
  {
    double eps = epsilon->compute(numChooses);
    ++numChooses;
    VariablePtr choice = BaseClass::policyChoose(choose);
    return Random::getInstance().sampleBool(eps)
      ? choose->sampleRandomChoice()
      : choice;
  }
  
private:
  size_t numChooses;
  IterationFunctionPtr epsilon;
};

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_CORE_IMPL_POLICY_BASIC_POLICIES_H_
