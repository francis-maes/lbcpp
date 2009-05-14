/*-----------------------------------------.---------------------------------.
| Filename: AllPairsLossFunction.hpp       | All-pairs ranking loss          |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2009 19:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_ALL_PAIRS_LOSS_H_
# define LBCPP_CORE_IMPL_FUNCTION_ALL_PAIRS_LOSS_H_

# include "LossFunctions.hpp"

namespace lbcpp {
namespace impl {

template<class DiscriminantLoss>
struct IsLargeMarginLoss { enum {margin = -1}; };
template<>
struct IsLargeMarginLoss<PerceptronLossFunction> { enum {margin = 0}; };
template<>
struct IsLargeMarginLoss<HingeLossFunction> { enum {margin = 1}; };

template<class DiscriminantLoss>
struct AllPairsLossFunction
  : public AdditiveRankingLossFunction< AllPairsLossFunction<DiscriminantLoss>, DiscriminantLoss >
{
  typedef AdditiveRankingLossFunction< AllPairsLossFunction<DiscriminantLoss>, DiscriminantLoss > BaseClass;
  
  AllPairsLossFunction(const DiscriminantLoss& discriminantLoss)
    : BaseClass(discriminantLoss) {}
  AllPairsLossFunction() {}

  void computeRankingLoss(const std::vector<double>& scores, const std::vector<double>& costs,
               double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const

  {
   // testLoss(scores, costs);

    double discriminantLossMargin = IsLargeMarginLoss<DiscriminantLoss>::margin;
    if (discriminantLossMargin >= 0 && !output)
    {
      // todo: implement output computation in specialized versions
      if (BaseClass::areCostsBipartite(costs))
        computeAllPairsBipartiteLargeMargin(scores, costs, discriminantLossMargin, output, gradientDirection, gradient);
      else
      {
        std::vector<size_t> order;
        BaseClass::sortScores(scores, order);
        std::map<double, std::vector<size_t> > alternativesPerCost;
        for (size_t i = 0; i < scores.size(); ++i)
          alternativesPerCost[costs[order[i]]].push_back(order[i]);
        if (BaseClass::hasFewDifferentCosts(scores.size(), alternativesPerCost.size()))
          computeAllPairsFewDifferentCostsLargeMargin(scores, costs, discriminantLossMargin, order, alternativesPerCost, output, gradientDirection, gradient);
        else
          computeAllPairsDefault(scores, costs, output, gradientDirection, gradient);          
      }
    }
    else
      computeAllPairsDefault(scores, costs, output, gradientDirection, gradient);
  }
  
  void testLoss(const std::vector<double>& scores, const std::vector<double>& costs) const
  {
    double discriminantLossMargin = IsLargeMarginLoss<DiscriminantLoss>::margin;

    if (discriminantLossMargin >= 0)
    {
      size_t n = scores.size();
      
      double refOutput;
      std::vector<double> refGradient(n, 0.0);
      computeAllPairsDefault(scores, costs, &refOutput, NULL, &refGradient);
      
      std::vector<size_t> order;
      BaseClass::sortScores(scores, order);
      std::map<double, std::vector<size_t> > alternativesPerCost;
      for (size_t i = 0; i < scores.size(); ++i)
        alternativesPerCost[costs[order[i]]].push_back(order[i]);
      
      // test "few-different-costs"
      {
        std::vector<double> gradient(n, 0.0);
        computeAllPairsFewDifferentCostsLargeMargin(scores, costs, discriminantLossMargin, order, alternativesPerCost, NULL, NULL, &gradient);
        assert(gradient == refGradient);
      }
        
      // test "bipartite"
      if (BaseClass::areCostsBipartite(costs))
      {
        std::vector<double> gradient(n, 0.0);
        computeAllPairsBipartiteLargeMargin(scores, costs, discriminantLossMargin, NULL, NULL, &gradient);
        assert(gradient == refGradient);
      }
    }
  }
  
protected:
  void computeAllPairsDefault(const std::vector<double>& scores, const std::vector<double>& costs,
                              double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const
  {
    size_t n = scores.size();
    size_t numPairs = 0;
    for (size_t i = 0; i < n; ++i)
      for (size_t j = 0; j < n; ++j)
      {
        double deltaCost = costs[j] - costs[i];
        if (deltaCost > 0) // i is better than j
        {
          ++numPairs;
          
          double deltaScore = scores[i] - scores[j]; // deltaScore should be positive
          //std::cout << "Pair (" << i << ", " << j << ") => " << deltaValue << std::endl;
          BaseClass::addRankingPair(deltaCost, deltaScore, i, j, output, gradientDirection, gradient);
        }
      }
      
    if (numPairs)
      BaseClass::multiplyOutputAndGradient(output, gradient, 1.0 / numPairs);
  }
  
  void computeAllPairsBipartiteLargeMargin(const std::vector<double>& scores, const std::vector<double>& costs, double margin, 
                                double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const
  {
    assert(!output); // value computation not implemented yet

    std::vector<size_t> order;
    BaseClass::sortScores(scores, order);
    size_t n = scores.size();
    
    if (gradient)
    {
      // for each negative example
      int forwardIndex = n - 1;
      double forwardCosts = 0;
      for (int i = (int)n - 1; i >= 0; --i)
        if (costs[order[i]]) // negative
        {
          double scorePlusMargin = scores[order[i]] + margin;
          while (forwardIndex >= 0 && scores[order[forwardIndex]] <= scorePlusMargin)
          {
            if (costs[order[forwardIndex]] == 0)
              forwardCosts++;
            --forwardIndex;
          }
          (*gradient)[order[i]] += forwardCosts;
        }
        
      // for each positive example
      size_t backwardIndex = 0;
      double backwardCosts = 0;
      for (size_t i = 0; i < n; ++i)
        if (costs[order[i]] == 0) // positive
        {
          double scoreMinusMargin = scores[order[i]] - margin;
          while (backwardIndex < n && scores[order[backwardIndex]] >= scoreMinusMargin)
          {
            if (costs[order[backwardIndex]])
              backwardCosts++;
            ++backwardIndex;
          }
          (*gradient)[order[i]] -= backwardCosts;
        }
    }
    
    size_t numPositives = 0;
    size_t numNegatives = 0;
    double cost = 1.0;
    for (size_t i = 0; i < n; ++i)
      if (costs[i])
        ++numNegatives, cost = costs[i];
      else
        ++numPositives;
    size_t numPairs = numPositives * numNegatives;
    if (numPairs)
      BaseClass::multiplyOutputAndGradient(output, gradient, cost / numPairs);
  }

  void computeAllPairsFewDifferentCostsLargeMargin(const std::vector<double>& scores, const std::vector<double>& costs,
                                double margin, const std::vector<size_t>& order, const std::map<double, std::vector<size_t> >& alternativesPerCost,
                                double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const
  {
    assert(!output); // value computation not implemented yet
    size_t n = scores.size();
    
    // for each possible cost
    std::map<double, std::vector<size_t> >::const_iterator it, it2;
    for (it = alternativesPerCost.begin(); it != alternativesPerCost.end(); ++it)
    {
      double referenceCost = it->first;
      const std::vector<size_t>& alternativesOfCost = it->second;
      
      std::vector<double> losses(alternativesOfCost.size(), 0.0);
      if (it != alternativesPerCost.begin()) // if is not the lowest cost
      {
        int forwardIndex = n - 1;
        double forwardCosts = 0;
        for (int i = alternativesOfCost.size() - 1; i >= 0; --i)
        {
          double scorePlusMargin = scores[alternativesOfCost[i]] + margin;
          while (forwardIndex >= 0 && scores[order[forwardIndex]] <= scorePlusMargin)
          {
            double cost = costs[order[forwardIndex]];
            if (cost < referenceCost)
              forwardCosts += referenceCost - cost;
            --forwardIndex;
          }
          losses[i] += forwardCosts;
        }
      }
      
      if (it->first != alternativesPerCost.rbegin()->first) // if is not the highest cost
      {
        size_t backwardIndex = 0;
        double backwardCosts = 0;
        for (size_t i = 0; i < alternativesOfCost.size(); ++i)
        {
          double scoreMinusMargin = scores[alternativesOfCost[i]] - margin;
          while (backwardIndex < n && scores[order[backwardIndex]] >= scoreMinusMargin)
          {
            double cost = costs[order[backwardIndex]];
            if (cost > referenceCost)
              backwardCosts += referenceCost - cost;
            ++backwardIndex;
          }
          losses[i] += backwardCosts;
        }
      }
      
      if (gradient)
        for (size_t i = 0; i < losses.size(); ++i)
          (*gradient)[alternativesOfCost[i]] += losses[i];
    }
    
    size_t numPairs = 0;
    for (it = alternativesPerCost.begin(); it != alternativesPerCost.end(); ++it)
    {
      size_t size1 = it->second.size(); 
      size_t size2 = 0;
      it2 = it;
      for (++it2; it2 != alternativesPerCost.end(); ++it2)
        size2 += it2->second.size();
      numPairs += size1 * size2;
    }
    if (numPairs)
      BaseClass::multiplyOutputAndGradient(output, gradient, 1.0 / numPairs);
  }
};

template<class DiscriminantLoss>
inline AllPairsLossFunction<DiscriminantLoss> allPairsLossFunction(const ScalarFunction<DiscriminantLoss>& discriminantFunction)
  {return AllPairsLossFunction<DiscriminantLoss>(static_cast<const DiscriminantLoss& >(discriminantFunction));}

STATIC_RANKING_LOSS_FUNCTION(allPairsLoss, AllPairsLoss, AllPairsLossFunction);

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_ALL_PAIRS_LOSS_H_
