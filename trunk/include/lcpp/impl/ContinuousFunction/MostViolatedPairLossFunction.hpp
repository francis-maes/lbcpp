/*-----------------------------------------.---------------------------------.
| Filename: MostViolatedPairLossFunction.hpp| Most-violated-pair ranking loss|
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_FUNCTION_MOST_VIOLATED_PAIR_LOSS_H_
# define LCPP_CORE_IMPL_FUNCTION_MOST_VIOLATED_PAIR_LOSS_H_

# include "LossFunctions.hpp"

namespace lcpp {
namespace impl {

template<class DiscriminantLoss>
struct MostViolatedPairLossFunction
  : public AdditiveRankingLossFunction< MostViolatedPairLossFunction<DiscriminantLoss>, DiscriminantLoss >
{
  typedef AdditiveRankingLossFunction< MostViolatedPairLossFunction<DiscriminantLoss>, DiscriminantLoss > BaseClass;
  
  MostViolatedPairLossFunction(const DiscriminantLoss& discriminantLoss)
    : BaseClass(discriminantLoss) {}
  MostViolatedPairLossFunction() {}
  
  enum {isDerivable = false};

  void computeRankingLoss(const std::vector<double>& scores, const std::vector<double>& costs,
               double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const
  {
    std::pair<size_t, size_t> mostViolatedPair;
    bool hasViolation;    

    if (BaseClass::areCostsBipartite(costs))
      hasViolation = findMaxViolationBipartite(scores, costs, mostViolatedPair);
    else
    {    
      std::map<double, std::pair<size_t, size_t> > scoreRangePerCost;
      BaseClass::getScoreRangePerCost(scores, costs, scoreRangePerCost);
      if (BaseClass::hasFewDifferentCosts(scores.size(), scoreRangePerCost.size()))
        hasViolation = findMaxViolationAllPairs(scores, costs, mostViolatedPair);
      else
        hasViolation = findMaxViolationFewDifferentCosts(scores, costs, scoreRangePerCost, mostViolatedPair);
    }
    
    if (hasViolation)
      BaseClass::addRankingPair(costs[mostViolatedPair.second] - costs[mostViolatedPair.first], 
                                scores[mostViolatedPair.first] - scores[mostViolatedPair.second],
                                mostViolatedPair.first, mostViolatedPair.second, output, gradientDirection, gradient);
  }
  
private:
  double computeViolation(double deltaScore, double deltaCost) const
  {
    assert(deltaCost > 0);
    double violation;
    BaseClass::discriminantLoss.compute(deltaScore, &violation, NULL, NULL);
    return violation * deltaCost;
  }
  
  bool findMaxViolationBipartite(const std::vector<double>& scores, const std::vector<double>& costs, std::pair<size_t, size_t>& mostViolatedPair) const
  {
    size_t n = scores.size();
    size_t maxNegative = (size_t)-1;
    size_t minPositive = (size_t)-1;
    double maxNegativeScore = -DBL_MAX;
    double minPositiveScore = DBL_MAX;

    for (size_t i = 0; i < n; ++i)
    {
      double score = scores[i];
      if (costs[i])
      {
        if (score > maxNegativeScore)
          maxNegativeScore = score, maxNegative = i;
      }
      else
      {
        if (score < minPositiveScore)
          minPositiveScore = score, minPositive = i;
      }
    }
    
    mostViolatedPair = std::make_pair(minPositive, maxNegative);
    return maxNegative != (size_t)-1 && minPositive != (size_t)-1;
  }
  
  bool findMaxViolationAllPairs(const std::vector<double>& scores, const std::vector<double>& costs, std::pair<size_t, size_t>& mostViolatedPair) const
  {
    double maxViolation = 0.0;
    size_t n = scores.size();
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j)
      {
        double deltaCost = costs[j] - costs[i];
        if (deltaCost > 0) // i is better than j
        {
          double violation = computeViolation(scores[i] - scores[j], deltaCost);
          if (violation > maxViolation)
          {
            maxViolation = violation;
            mostViolatedPair = std::make_pair(i, j);
          }
        }
      }
    return maxViolation > 0;  
  }
  
  bool findMaxViolationFewDifferentCosts(const std::vector<double>& scores, const std::vector<double>& costs,
                                         const std::map<double, std::pair<size_t, size_t> >& scoreRangePerCost, std::pair<size_t, size_t>& mostViolatedPair) const
  {
    double maxViolation = 0.0;
    for (std::map<double, std::pair<size_t, size_t> >::const_iterator it = scoreRangePerCost.begin();
          it != scoreRangePerCost.end(); ++it)
    {
      double currentMinimum = scores[it->second.first];
      std::map<double, std::pair<size_t, size_t> >::const_iterator it2 = it;
      for (++it2; it2 != scoreRangePerCost.end(); ++it2)
      {
        double currentMaximum = scores[it2->second.second];
        double violation = computeViolation(currentMaximum - currentMinimum, it2->first - it->first);
        if (violation > maxViolation)
        {
          maxViolation = violation;
          mostViolatedPair = std::make_pair(it->second.first, it2->second.second);
        }
      }
    }
    return maxViolation > 0;
  }
};

template<class DiscriminantLoss>
inline MostViolatedPairLossFunction<DiscriminantLoss> mostViolatedPairLossFunction(const ScalarFunction<DiscriminantLoss>& discriminantFunction)
  {return MostViolatedPairLossFunction<DiscriminantLoss>(static_cast<const DiscriminantLoss& >(discriminantFunction));}

STATIC_RANKING_LOSS_FUNCTION(mostViolatedPairLoss, MostViolatedPairLoss, MostViolatedPairLossFunction);

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_CORE_IMPL_FUNCTION_MOST_VIOLATED_PAIR_LOSS_H_
