/*-----------------------------------------.---------------------------------.
| Filename: MostViolatedPairRankingLoss...h| Most-violated-pair ranking loss |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_PAIR_RANKING_H_
# define LBCPP_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_PAIR_RANKING_H_

# include "AdditiveRankingLossFunction.h"

namespace lbcpp
{

class MostViolatedPairRankingLossFunction : public AdditiveRankingLossFunction
{
public:
  MostViolatedPairRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss = DiscriminativeLossFunctionPtr())
    : AdditiveRankingLossFunction(baseLoss) {}

  virtual bool isDerivable() const
    {return false;}

  virtual void computeRankingLoss(const std::vector<double>& scores, const std::vector<double>& costs, double* output, std::vector<double>* gradient) const
  {
    std::pair<size_t, size_t> mostViolatedPair;
    bool hasViolation;    

    bool zeroIsPositive;
    if (areCostsBipartite(costs, zeroIsPositive))
      hasViolation = findMaxViolationBipartite(scores, costs, zeroIsPositive, mostViolatedPair);
    else
    {    
      std::map<double, std::pair<size_t, size_t> > scoreRangePerCost;
      getScoreRangePerCost(scores, costs, scoreRangePerCost);
      if (hasFewDifferentCosts(scores.size(), scoreRangePerCost.size()))
        hasViolation = findMaxViolationFewDifferentCosts(scores, costs, scoreRangePerCost, mostViolatedPair);
      else
        hasViolation = findMaxViolationAllPairs(scores, costs, mostViolatedPair);        
    }
    
    if (hasViolation)
      addRankingPair(costs[mostViolatedPair.second] - costs[mostViolatedPair.first], 
                                scores[mostViolatedPair.first] - scores[mostViolatedPair.second],
                                mostViolatedPair.first, mostViolatedPair.second, output, gradient);
  }
  
private:
  double computeViolation(double deltaScore, double deltaCost) const
  {
    jassert(deltaCost > 0);
    double violation;
    baseLoss->computeDiscriminativeLoss(deltaScore, &violation, NULL);
    return violation * deltaCost;
  }
  
  bool findMaxViolationBipartite(const std::vector<double>& scores, const std::vector<double>& costs, bool zeroIsPositive, std::pair<size_t, size_t>& mostViolatedPair) const
  {
    size_t n = scores.size();
    size_t maxNegative = (size_t)-1;
    size_t minPositive = (size_t)-1;
    double maxNegativeScore = -DBL_MAX;
    double minPositiveScore = DBL_MAX;

    for (size_t i = 0; i < n; ++i)
    {
      double score = scores[i];
      if (isPositiveCost(costs[i], zeroIsPositive))
      {
        if (score < minPositiveScore)
          minPositiveScore = score, minPositive = i;
      }
      else
      {
        if (score > maxNegativeScore)
          maxNegativeScore = score, maxNegative = i;
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

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_PAIR_RANKING_H_
