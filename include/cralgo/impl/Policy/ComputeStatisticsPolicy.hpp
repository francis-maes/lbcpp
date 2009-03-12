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

namespace cralgo {
namespace impl {

template<class DecoratedType>
struct ComputeStatisticsPolicy
  : public DecoratorPolicy<ComputeStatisticsPolicy<DecoratedType> , DecoratedType>
{
  typedef DecoratorPolicy<ComputeStatisticsPolicy, DecoratedType> BaseClass;
  
  ComputeStatisticsPolicy(const DecoratedType& decorated)
    : BaseClass(decorated), numChooses(0), numChoices(0), rewardSum(0.0) {}
    
  std::string toString() const
  {
    std::string res = cralgo::toString(numChooses) + " chooses, " +
        cralgo::toString(numChoices) + " choices, " +
        cralgo::toString(rewardSum) + " reward";
    if (numChooses)
    {
      res += ", " + cralgo::toString(numChoices / (double)numChooses) + " choices/choose, "
        + cralgo::toString(rewardSum / numChooses) + " reward/choose";
    }
    return res;
  }
      
  VariablePtr policyChoose(ChoosePtr choose)
  {
    ++numChooses;
    numChoices += choose->getNumChoices();
    return BaseClass::policyChoose(choose);
  }

  void policyReward(double reward)
  {
    rewardSum += reward;
    BaseClass::policyReward(reward);
  }

private:
  size_t numChooses;
  size_t numChoices; // todo: incremental statistics
  double rewardSum;
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_COMPUTE_STATISTICS_H_
