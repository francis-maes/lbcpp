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
  : public EpisodicDecoratorPolicy<ComputeStatisticsPolicy<DecoratedType> , DecoratedType>
{
  typedef EpisodicDecoratorPolicy<ComputeStatisticsPolicy, DecoratedType> BaseClass;
  
  ComputeStatisticsPolicy(const DecoratedType& decorated)
    : BaseClass(decorated),
      rewardPerChoose(new ScalarRandomVariableStatistics("rewardPerChoose")),
      choicesPerChoose(new ScalarRandomVariableStatistics("choicesPerChoose")),
      rewardPerEpisode(new ScalarRandomVariableStatistics("rewardPerEpisode")),
      choosesPerEpisode(new ScalarRandomVariableStatistics("choosesPerEpisode")),
      choicesPerEpisode(new ScalarRandomVariableStatistics("choicesPerEpisode"))
  {
  }

  VariablePtr policyStart(ChoosePtr choose)
  {
    episodeNumChooses = 0;
    episodeNumChoices = 0;
    episodeRewardSum = 0.0;
    processChoose(choose);
    return BaseClass::policyStart(choose);
  }
  
  VariablePtr policyStep(double reward, ChoosePtr choose)
  {
    processReward(reward);
    processChoose(choose);
    return BaseClass::policyStep(reward, choose);
  }
    
  void policyEnd(double reward)
  {
    processReward(reward);
    if (episodeNumChooses)
    {
      rewardPerEpisode->push(episodeRewardSum);
      choosesPerEpisode->push((double)episodeNumChooses);
      choicesPerEpisode->push((double)episodeNumChoices);
    }
    BaseClass::policyEnd(reward);
  }
  
  size_t getNumResults() const
    {return 5 + BaseClass::getNumResults();}
    
  ObjectPtr getResult(size_t i) const
  {
    switch (i)
    {
    case 0: return rewardPerChoose;
    case 1: return choicesPerChoose;
    case 2: return rewardPerEpisode;
    case 3: return choosesPerEpisode;
    case 4: return choicesPerEpisode;
    default: return BaseClass::getResult(i - 5);
    };
  }

private:
  void processChoose(ChoosePtr choose)
  {
    size_t numChoices = choose->getNumChoices();
    ++episodeNumChooses;
    episodeNumChoices += numChoices;
    choicesPerChoose->push((double)numChoices);
  }
  
  void processReward(double reward)
  {
    episodeRewardSum += reward;
    rewardPerChoose->push(reward);
  }

  ScalarRandomVariableStatisticsPtr rewardPerChoose, choicesPerChoose;
  ScalarRandomVariableStatisticsPtr rewardPerEpisode, choosesPerEpisode, choicesPerEpisode;

  size_t episodeNumChooses;
  size_t episodeNumChoices;
  double episodeRewardSum;
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_COMPUTE_STATISTICS_H_
