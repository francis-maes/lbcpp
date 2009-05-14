/*-----------------------------------------.---------------------------------.
| Filename: VerbosePolicy.hpp              | A policy that displays          |
| Author  : Francis Maes                   | information on an output stream |
| Started : 13/03/2009 13:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_POLICY_VERBOSE_H_
# define LBCPP_CORE_IMPL_POLICY_VERBOSE_H_

# include "PolicyStatic.hpp"

namespace lbcpp {
namespace impl {

template<class DecoratedType>
struct VerbosePolicy
  : public DecoratorPolicy<VerbosePolicy<DecoratedType> , DecoratedType>
{
  typedef DecoratorPolicy<VerbosePolicy, DecoratedType> BaseClass;
  
  /*
  ** Verbosity
  */
  // 0: nothing
  // 1: one '.' per episode
  // 2: one '.' per action
  // 3: chosen action descriptions
  // 4: state descriptions
  // 5: ActionSet descriptions
    
  VerbosePolicy(const DecoratedType& decorated, size_t verbosity, std::ostream& ostr)
    : BaseClass(decorated), ostr(ostr), verbosity(verbosity), inclusionLevel(0), stepNumber(0)
  {
  }
  
  VariablePtr policyChoose(ChoosePtr choose)
  {
    VariablePtr res = BaseClass::policyChoose(choose);
    if (verbosity == 2)
      ostr << "." << std::flush;
    else if (verbosity >= 4)
    {
      ostr << std::endl << "==== Step " << ++stepNumber << " ====" << std::endl;
      if (verbosity == 4)
        ostr << choose->getCRAlgorithm()->toString();
      else if (verbosity == 5)
        ostr << choose->toString() << std::endl;
    }
    if (verbosity >= 3)
      ostr << "  => Choice: " << res->toString() << std::endl;
    return res;
  }

  void policyReward(double reward)
  {
    if (verbosity >= 3)
      ostr << "  => Reward: " << reward << std::endl;
    episodeReward += reward;
    BaseClass::policyReward(reward);
  }
  
  void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    if (verbosity >= 3)
      ostr << "policyEnter(" << crAlgorithm->getName() << ")" << std::endl;
    BaseClass::policyEnter(crAlgorithm);
    stepNumber = 0;
    ++inclusionLevel;
  }
    
  void policyLeave()
  {
    if (verbosity >= 3)
      ostr << "policyLeave()" << std::endl;
    BaseClass::policyLeave();
    --inclusionLevel;
    if (inclusionLevel == 0)
    {
      if (verbosity == 1)
        ostr << "." << std::flush;
      else if (verbosity == 2)
        ostr << " -> " << episodeReward << std::endl;
      else if (verbosity >= 3)
      {
        if (verbosity >= 3)
          ostr << std::endl << "==================" << std::endl;
        ostr << "Episode Reward: " << episodeReward << std::endl << std::endl;
      }
    }
  }

private:
  std::ostream& ostr;
  size_t verbosity;
  size_t inclusionLevel;
  size_t stepNumber;
  double episodeReward;
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_POLICY_CONSOLE_LOGGER_H_
