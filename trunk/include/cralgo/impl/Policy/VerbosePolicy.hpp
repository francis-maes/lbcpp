/*-----------------------------------------.---------------------------------.
| Filename: VerbosePolicy.hpp              | A policy that displays          |
| Author  : Francis Maes                   | information on an output stream |
| Started : 13/03/2009 13:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_VERBOSE_H_
# define CRALGO_IMPL_POLICY_VERBOSE_H_

# include "PolicyStatic.hpp"

namespace cralgo {
namespace impl {

template<class DecoratedType>
struct VerbosePolicy
  : public DecoratorPolicy<VerbosePolicy<DecoratedType> , DecoratedType>
{
  typedef DecoratorPolicy<VerbosePolicy, DecoratedType> BaseClass;
  
  /*
  ** Verbosity
  */
  // 0: one '.' per episode
  // 1: one '.' per action
  // 2: chosen action descriptions
  // 3: state descriptions
  // 4: ActionSet descriptions
    
  VerbosePolicy(const DecoratedType& decorated, std::ostream& ostr, size_t verbosity)
    : BaseClass(decorated), ostr(ostr), verbosity(verbosity), inclusionLevel(0), stepNumber(0)
  {
  }
  
  VariablePtr policyChoose(ChoosePtr choose)
  {
    VariablePtr res = BaseClass::policyChoose(choose);
    if (verbosity == 1)
      ostr << "." << std::flush;
    else if (verbosity >= 3)
    {
      ostr << std::endl << "==== Step " << ++stepNumber << " ====" << std::endl;
      if (verbosity == 3)
        ostr << choose->getCRAlgorithm()->toString();
      else if (verbosity == 4)
        ostr << choose->toString() << std::endl;
    }
    if (verbosity >= 2)
      ostr << "  => Choice: " << res->toString() << std::endl;
    return res;
  }

  void policyReward(double reward)
  {
    if (verbosity >= 2)
      ostr << "  => Reward: " << reward << std::endl;
    episodeReward += reward;
    BaseClass::policyReward(reward);
  }
  
  void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    if (verbosity >= 2)
      ostr << "policyEnter(" << crAlgorithm->getName() << ")" << std::endl;
    BaseClass::policyEnter(crAlgorithm);
    stepNumber = 0;
    ++inclusionLevel;
  }
    
  void policyLeave()
  {
    if (verbosity >= 2)
      ostr << "policyLeave()" << std::endl;
    BaseClass::policyLeave();
    --inclusionLevel;
    if (inclusionLevel == 0)
    {
      if (verbosity == 0)
        ostr << "." << std::flush;
      else if (verbosity == 1)
        ostr << " -> " << episodeReward << std::endl;
      else if (verbosity >= 2)
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
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_CONSOLE_LOGGER_H_
