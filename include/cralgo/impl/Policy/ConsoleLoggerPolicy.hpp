/*-----------------------------------------.---------------------------------.
| Filename: ConsoleLoggerPolicy.hpp        | A policy that displays          |
| Author  : Francis Maes                   |   information on the console    |
| Started : 13/03/2009 13:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_CONSOLE_LOGGER_H_
# define CRALGO_IMPL_POLICY_CONSOLE_LOGGER_H_

# include "PolicyStatic.hpp"

namespace cralgo {
namespace impl {

template<class DecoratedType>
struct ConsoleLoggerPolicy
  : public DecoratorPolicy<ConsoleLoggerPolicy<DecoratedType> , DecoratedType>
{
  typedef DecoratorPolicy<ConsoleLoggerPolicy, DecoratedType> BaseClass;
  
  /*
  ** Verbosity
  */
  // 0: one '.' per episode
  // 1: one '.' per action
  // 2: chosen action descriptions
  // 3: state descriptions
  // 4: ActionSet descriptions
    
  ConsoleLoggerPolicy(const DecoratedType& decorated, std::ostream& ostr, size_t verbosity)
    : BaseClass(decorated), ostr(ostr), verbosity(verbosity), inclusionLevel(0)
  {
  }

  //////////
  void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    BaseClass::policyEnter(crAlgorithm);
    if (inclusionLevel == 0)
      beginEpisode(crAlgorithm);
    ++inclusionLevel;
  }
  
  void policyLeave()
  {
    BaseClass::policyLeave();    
    assert(inclusionLevel > 0);
    --inclusionLevel;
    if (inclusionLevel == 0)
      finishEpisode();
  }
  /////////////
  
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

  
  void beginEpisode(CRAlgorithmPtr crAlgorithm)
  {
    stepNumber = 0;
    episodeReward = 0.0;
    if (verbosity >= 2)
      ostr << "Episode begin: " << crAlgorithm->getName() << std::endl;
  }
  
  void finishEpisode()
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
