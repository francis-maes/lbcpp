/*-----------------------------------------.---------------------------------.
| Filename: EpsilonGreedyPolicy.h          | Epsilon greedy policy           |
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 21:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_EPSILON_GREEDY_H_
# define LBCPP_POLICY_EPSILON_GREEDY_H_

# include <lbcpp/Policy.h>

namespace lbcpp
{

class EpsilonGreedyPolicy : public DecoratorPolicy
{
public:
  EpsilonGreedyPolicy(PolicyPtr decorated, IterationFunctionPtr epsilon)
    : DecoratorPolicy(decorated), numChooses(0), epsilon(epsilon) {}
  EpsilonGreedyPolicy() : numChooses(0) {}

  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    double eps = epsilon->compute(numChooses);
    ++numChooses;
    VariablePtr choice = DecoratorPolicy::policyChoose(choose);
    return RandomGenerator::getInstance().sampleBool(eps)
      ? choose->sampleRandomChoice()
      : choice;
  }
  
  virtual String toString() const
    {return "epsilonGreedyPolicy(" + decorated->toString() + ", " + epsilon->toString() + ")";}
  
  virtual void save(std::ostream& ostr) const
    {DecoratorPolicy::save(ostr); write(ostr, numChooses); write(ostr, epsilon);}

  virtual bool load(std::istream& istr)
    {return DecoratorPolicy::load(istr) && read(istr, numChooses) && read(istr, epsilon);}
  
private:
  size_t numChooses;
  IterationFunctionPtr epsilon;
};

}; /* namespace impl */

#endif // !LBCPP_POLICY_EPSILON_GREEDY_H_
