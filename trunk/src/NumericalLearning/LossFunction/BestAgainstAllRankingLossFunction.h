/*-----------------------------------------.---------------------------------.
| Filename: BestAgainstAllRankingLossFun..h| Best-against-all ranking loss   |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2009 21:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_BEST_AGAINST_ALL_RANKING_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_BEST_AGAINST_ALL_RANKING_H_

# include <lbcpp/NumericalLearning/LossFunctions.h>

namespace lbcpp
{

class BestAgainstAllRankingLossFunction : public AdditiveRankingLossFunction
{
public:
  BestAgainstAllRankingLossFunction(BinaryClassificationLossFunctionPtr baseLoss = BinaryClassificationLossFunctionPtr())
    : AdditiveRankingLossFunction(baseLoss) {}
  
  virtual bool isDerivable() const
    {return false;}

  virtual void computeRankingLoss(const std::vector<double>& scores, const std::vector<double>& costs, double* output, std::vector<double>* gradient) const
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
        addRankingPair(deltaCost, topRankScore - scores[i], topRankIndex, i, output, gradient);
        ++numPairs;
      }
      else if (deltaCost < 0) // i is better than topRank
      {
        addRankingPair(-deltaCost, scores[i] - topRankScore, i, topRankIndex, output, gradient);
        ++numPairs;
      }
    }
    
    if (numPairs)
      multiplyOutputAndGradient(output, gradient, 1.0 / numPairs);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_BEST_AGAINST_ALL_RANKING_H_
