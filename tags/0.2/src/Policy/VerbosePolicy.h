/*-----------------------------------------.---------------------------------.
| Filename: VerbosePolicy.h                | A policy that displays          |
| Author  : Francis Maes                   | information on an output stream |
| Started : 13/03/2009 13:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_VERBOSE_H_
# define LBCPP_POLICY_VERBOSE_H_

# include <lbcpp/CRAlgorithm/Policy.h>

namespace lbcpp
{

/*
** Verbosity
*/
// 0: nothing
// 1: one '.' per episode
// 2: one '.' per action
// 3: chosen action descriptions
// 4: state descriptions
// 5: ActionSet descriptions

class VerbosePolicy : public DecoratorPolicy
{
public:
  VerbosePolicy(PolicyPtr decorated, size_t verbosity, std::ostream& ostr)
    : DecoratorPolicy(decorated), ostr(ostr), verbosity(verbosity), inclusionLevel(0), stepNumber(0)
    {}
    
  VerbosePolicy() : ostr(std::cout), verbosity(0), inclusionLevel(0), stepNumber(0) {}
  
  /*
  ** Object
  */
  virtual String toString() const
    {return decorated->toString() + "->verbose(" + lbcpp::toString(verbosity) + ")";}

  virtual bool load(InputStream& istr)
    {return DecoratorPolicy::load(istr) && read(istr, verbosity);}
    
  virtual void save(OutputStream& ostr) const
    {DecoratorPolicy::save(ostr); write(ostr, verbosity);}
  
  /*
  ** Policy
  */
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    if (verbosity >= 3)
      ostr << "policyEnter(" << crAlgorithm->getName() << ")" << std::endl;
    DecoratorPolicy::policyEnter(crAlgorithm);
    stepNumber = 0;
    episodeReward = 0.0;
    ++inclusionLevel;
  }
  
  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    VariablePtr res = DecoratorPolicy::policyChoose(choose);
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

  virtual void policyReward(double reward)
  {
    if (verbosity >= 3)
      ostr << "  => Reward: " << reward << std::endl;
    episodeReward += reward;
    DecoratorPolicy::policyReward(reward);
  }
    
  virtual void policyLeave()
  {
    if (verbosity >= 3)
      ostr << "policyLeave()" << std::endl;
    DecoratorPolicy::policyLeave();
    --inclusionLevel;
    if (inclusionLevel == 0)
    {
      if (verbosity == 1)
        ostr << "." << std::flush;
      else if (verbosity == 2)
        ostr << " -> " << episodeReward << std::endl;
      else if (verbosity >= 3)
      {
        if (verbosity > 3)
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

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_VERBOSE_H_
