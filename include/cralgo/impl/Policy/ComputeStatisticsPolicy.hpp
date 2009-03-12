/*-----------------------------------------.---------------------------------.
| Filename: ComputeStatisticsPolicy.hpp    | A policy that computes general  |
| Author  : Francis Maes                   |   statistics about episodes     |
| Started : 11/03/2009 20:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_COMPUTE_STATISTICS_H_
# define CRALGO_IMPL_POLICY_COMPUTE_STATISTICS_H_

# include "PolicyStatic.hpp"
# include "../../RandomVariable.h"

namespace cralgo {
namespace impl {

template<class DecoratedType>
struct ComputeStatisticsPolicy
  : public DecoratorPolicy<ComputeStatisticsPolicy<DecoratedType> , DecoratedType>
{
  typedef DecoratorPolicy<ComputeStatisticsPolicy, DecoratedType> BaseClass;
  
  ComputeStatisticsPolicy(const DecoratedType& decorated)
    : BaseClass(decorated),
      rewardPerChoose(new ScalarRandomVariableStatistics("rewardPerChoose")),
      choicesPerChoose(new ScalarRandomVariableStatistics("choicesPerChoose")),
      rewardPerEpisode(new ScalarRandomVariableStatistics("rewardPerEpisode")),
      choosesPerEpisode(new ScalarRandomVariableStatistics("choosesPerEpisode")),
      choicesPerEpisode(new ScalarRandomVariableStatistics("choicesPerEpisode"))
  {
  }
  
  void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    // FIXME: check flat
    episodeNumChooses = 0;
    episodeNumChoices = 0;
    episodeRewardSum = 0.0;
    BaseClass::policyEnter(crAlgorithm);
  }

  VariablePtr policyChoose(ChoosePtr choose)
  {
    ++episodeNumChooses;
    episodeNumChoices += choose->getNumChoices();
    return BaseClass::policyChoose(choose);
  }

  void policyReward(double reward)
  {
    episodeRewardSum += reward;
    BaseClass::policyReward(reward);
  }
  
  void policyLeave()
  {
    if (episodeNumChooses)
    {
      rewardPerChoose->push(episodeRewardSum / episodeNumChooses);
      choicesPerChoose->push(episodeNumChoices / (double)episodeNumChooses);
      rewardPerEpisode->push(episodeRewardSum);
      choosesPerEpisode->push((double)episodeNumChooses);
      choicesPerEpisode->push((double)episodeNumChoices);
    }
    BaseClass::policyLeave();    
  }
  
  size_t getNumResults() const
    {return 5 + BaseClass::getNumResults();}
    
  ObjectPtr getResult(size_t i) const
  {
    if (i >= 5)
      return BaseClass::getResult(i - 5);
    switch (i)
    {
    case 0: return rewardPerChoose;
    case 1: return choicesPerChoose;
    case 2: return rewardPerEpisode;
    case 3: return choosesPerEpisode;
    case 4: return choicesPerEpisode;
    };
    assert(false);
    return ObjectPtr();
  }

private:
  ScalarRandomVariableStatisticsPtr rewardPerChoose, choicesPerChoose;
  ScalarRandomVariableStatisticsPtr rewardPerEpisode, choosesPerEpisode, choicesPerEpisode;

  size_t episodeNumChooses;
  size_t episodeNumChoices;
  double episodeRewardSum;
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_COMPUTE_STATISTICS_H_
