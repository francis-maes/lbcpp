/*-----------------------------------------.---------------------------------.
| Filename: ComputeStatisticsPolicy.h      | A Policy Decorator that computes|
| Author  : Francis Maes                   |  various statistics related     |
| Started : 16/06/2009 16:15               |   to choose and rewards.        |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_COMPUTE_STATISTICS_H_
# define LBCPP_POLICY_COMPUTE_STATISTICS_H_

# include <lbcpp/CRAlgorithm/Policy.h>

namespace lbcpp
{

class ComputeStatisticsPolicy : public DecoratorPolicy
{
public:
  ComputeStatisticsPolicy(PolicyPtr decorated, PolicyStatisticsPtr statistics)
    : DecoratorPolicy(decorated), statistics(statistics), inclusionLevel(0)
    {}
  ComputeStatisticsPolicy() {}

  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    jassert(inclusionLevel == 0);
    ++inclusionLevel;
    episodeNumChooses = 0;
    episodeNumChoices = 0;
    episodeRewardSum = 0.0;
    rewardSinceLastChoose = 0.0;
    DecoratorPolicy::policyEnter(crAlgorithm);
  }

  virtual VariablePtr policyChoose(ChoosePtr choose)
  {
    size_t numChoices = choose->getNumChoices();
    if (episodeNumChooses)
      statistics->getRewardPerChoose()->push(rewardSinceLastChoose);
    rewardSinceLastChoose = 0.0;
    ++episodeNumChooses;
    episodeNumChoices += numChoices;
    statistics->getChoicesPerChoose()->push((double)numChoices);
    return DecoratorPolicy::policyChoose(choose);
  }

  virtual void policyReward(double reward)
  {
    episodeRewardSum += reward;
    rewardSinceLastChoose += reward;
    DecoratorPolicy::policyReward(reward);
  }

  virtual void policyLeave()
  {
    if (episodeNumChooses)
    {
      statistics->getRewardPerChoose()->push(rewardSinceLastChoose);
      
      statistics->getRewardPerEpisode()->push(episodeRewardSum);
      statistics->getChoosesPerEpisode()->push((double)episodeNumChooses);
      statistics->getChoicesPerEpisode()->push((double)episodeNumChoices);
    }
    --inclusionLevel;
    jassert(inclusionLevel == 0);
    DecoratorPolicy::policyLeave();
  }
  
  virtual String toString() const
    {return "computeStatisticsPolicy(" + decorated->toString() + ")";}

private:
  PolicyStatisticsPtr statistics;
  size_t inclusionLevel;
  size_t episodeNumChooses;
  size_t episodeNumChoices;
  double episodeRewardSum;
  double rewardSinceLastChoose;
};

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_COMPUTE_STATISTICS_H_
