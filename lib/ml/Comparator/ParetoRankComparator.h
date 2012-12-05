/*-----------------------------------------.---------------------------------.
| Filename: ParetoRankComparator.h         | Objective Comparator            |
| Author  : Francis Maes                   |                                 |
| Started : 1/09/2012 16:08                |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_COMPARATOR_PARETO_RANK_H_
# define LBCPP_ML_COMPARATOR_PARETO_RANK_H_

# include <ml/SolutionComparator.h>

namespace lbcpp
{

class ParetoRankAndCrowdingDistanceComparator : public SolutionComparator
{
public:
  virtual void initialize(const SolutionContainerPtr& solutions)
  {
    subFronts = solutions.staticCast<SolutionVector>()->nonDominatedSort(&mapping);
    crowdingDistances.resize(subFronts.size());
    for (size_t i = 0; i < crowdingDistances.size(); ++i)
      subFronts[i]->computeCrowdingDistances(crowdingDistances[i]);
  }

  virtual int compare(size_t index1, size_t index2)
  {
    size_t rank1 = mapping[index1].first;
    size_t rank2 = mapping[index2].first;
    if (rank1 != rank2)
      return rank1 < rank2 ? -1 : 1;
    else
    {
      size_t rank = rank1;
      double distance1 = crowdingDistances[rank][mapping[index1].second];
      double distance2 = crowdingDistances[rank][mapping[index2].second];
      if (distance1 != distance2)
        return distance1 > distance2 ? -1 : 1;
      else
        return 0;
    }
  }

private:
  std::vector< std::pair<size_t, size_t> > mapping;
  std::vector<ParetoFrontPtr> subFronts;
  std::vector<std::vector<double> > crowdingDistances;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_COMPARATOR_PARETO_RANK_H_
