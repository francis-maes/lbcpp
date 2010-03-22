/*-----------------------------------------.---------------------------------.
| Filename: RankingExamplesCreatorPolicy.h | A policy that creates           |
| Author  : Francis Maes                   |   ranking examples              |
| Started : 13/03/2009 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_RANKING_EXAMPLES_CREATOR_H_
# define LBCPP_POLICY_RANKING_EXAMPLES_CREATOR_H_

# include <lbcpp/Policy.h>
# include <lbcpp/LearningMachine.h>

namespace lbcpp
{

class RankingExamplesCreatorPolicy : public DecoratorPolicy
{
public:
  RankingExamplesCreatorPolicy(PolicyPtr explorationPolicy, RankerPtr ranker, ActionValueFunctionPtr supervisor = ActionValueFunctionPtr())
    : DecoratorPolicy(explorationPolicy), ranker(ranker), supervisor(supervisor), inclusionLevel(0), isTraining(false) {}
  
  RankingExamplesCreatorPolicy() {}
    
  /*
  ** Object
  */
  virtual std::string toString() const
  {
    std::string res = "rankingExamplesCreatorPolicy(" + decorated->toString() + ", " + ranker->toString();
    if (supervisor)
      res += ", " + supervisor->toString();
    return res + ")";
  }
  
  virtual void save(std::ostream& ostr) const
    {DecoratorPolicy::save(ostr); write(ostr, ranker); write(ostr, supervisor);}
    
  virtual bool load(std::istream& istr)
    {return DecoratorPolicy::load(istr) && read(istr, ranker) && read(istr, supervisor);}
  
  /*
  ** Policy
  */
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    if (inclusionLevel == 0)
      isTraining = true;
    DecoratorPolicy::policyEnter(crAlgorithm);
    ++inclusionLevel;
  }
    
  virtual void policyLeave()
  {
    DecoratorPolicy::policyLeave();
    --inclusionLevel;
    if (inclusionLevel == 0 && isTraining)
    {
      ranker->trainStochasticEnd();
      isTraining = false;
    }
  }

  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    FeatureGeneratorPtr alternatives = choose->computeActionsFeatures(true);

    std::vector<double> costs;
    choose->computeActionValues(costs, supervisor);
    if (!costs.size())
      return VariablePtr();
    
    // transform action values into (regret) costs
    double bestValue = -DBL_MAX;
    for (size_t i = 0; i < costs.size(); ++i)
      if (costs[i] > bestValue)
        bestValue = costs[i];
    for (size_t i = 0; i < costs.size(); ++i)
      costs[i] = bestValue - costs[i];
      
    //std::cout << "COSTS => " << lbcpp::toString(costs) << std::endl;
    
    //std::cout << "ALTERNATIVES ===> " << std::endl << alternatives->toString() << std::endl;
    if (!isTraining)
    {
      ranker->trainStochasticBegin(alternatives->getDictionary());
      isTraining = true;
    }
    ranker->trainStochasticExample(new RankingExample(alternatives, costs));
    return DecoratorPolicy::policyChoose(choose);
  }
  
private:
  RankerPtr ranker;
  ActionValueFunctionPtr supervisor;
  size_t inclusionLevel;
  bool isTraining;
};

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_RANKING_EXAMPLES_CREATOR_H_
