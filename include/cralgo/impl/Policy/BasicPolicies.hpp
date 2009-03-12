/*-----------------------------------------.---------------------------------.
| Filename: BasicPolicies.hpp              | Simple common policies          |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_BASIC_POLICIES_H_
# define CRALGO_IMPL_POLICY_BASIC_POLICIES_H_

# include "PolicyStatic.hpp"

namespace cralgo {
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
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_BASIC_POLICIES_H_
