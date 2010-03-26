/*-----------------------------------------.---------------------------------.
| Filename: BestAgainstAllLossFunction.hpp | Best-against-all ranking loss   |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2009 21:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_BEST_AGAINST_ALL_LOSS_H_
# define LBCPP_CORE_IMPL_FUNCTION_BEST_AGAINST_ALL_LOSS_H_

# include "LossFunctions.hpp"

namespace lbcpp {
namespace impl {

template<class DiscriminantLoss>
struct BestAgainstAllLossFunction
  : public AdditiveRankingLossFunction< BestAgainstAllLossFunction<DiscriminantLoss>, DiscriminantLoss >
{
  typedef AdditiveRankingLossFunction< BestAgainstAllLossFunction<DiscriminantLoss>, DiscriminantLoss > BaseClass;
  
  BestAgainstAllLossFunction(const DiscriminantLoss& discriminantLoss)
    : BaseClass(discriminantLoss) {}
  BestAgainstAllLossFunction() {}
  
  enum {isDerivable = false};

  void computeRankingLoss(const std::vector<double>& scores, const std::vector<double>& costs,
               double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const
  {
    size_t n = scores.size();
    double topRankScore = -DBL_MAX;
    size_t topRankIndex = (size_t)-1;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      double score = scores[i];
      if (score > topRankScore)
        topRankScore = score, topRankIndex = i;
    }
    jassert(topRankIndex != (size_t)-1);
    double topRankCost = costs[topRankIndex];
    
    size_t numPairs = 0;
    for (size_t i = 0; i < n; ++i)
    {
      double deltaCost = costs[i] - topRankCost;
      if (deltaCost > 0) // topRank is better than i
      {
        BaseClass::addRankingPair(deltaCost, topRankScore - scores[i], topRankIndex, i,
          output, gradientDirection, gradient);
        ++numPairs;
      }
      else if (deltaCost < 0) // i is better than topRank
      {
        BaseClass::addRankingPair(-deltaCost, scores[i] - topRankScore, i, topRankIndex,
          output, gradientDirection, gradient);
        ++numPairs;
      }
    }
    
    if (numPairs)
      BaseClass::multiplyOutputAndGradient(output, gradient, 1.0 / numPairs);
  }
};

template<class DiscriminantLoss>
inline BestAgainstAllLossFunction<DiscriminantLoss> bestAgainstAllLossFunction(const ScalarFunction<DiscriminantLoss>& discriminantFunction)
  {return BestAgainstAllLossFunction<DiscriminantLoss>(static_cast<const DiscriminantLoss& >(discriminantFunction));}

STATIC_RANKING_LOSS_FUNCTION(bestAgainstAllLoss, BestAgainstAllLoss, BestAgainstAllLossFunction);

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_BEST_AGAINST_ALL_LOSS_H_
