/*-----------------------------------------.---------------------------------.
| Filename: BestAgainstAllLossFunction.hpp | Best-against-all ranking loss   |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2009 21:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_BEST_AGAINST_ALL_LOSS_H_
# define CRALGO_IMPL_FUNCTION_BEST_AGAINST_ALL_LOSS_H_

# include "LossFunctions.hpp"

namespace cralgo {
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

  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
  {
    const std::vector<double>& costs = BaseClass::getCosts();
    if (!costs.size())
      {if (output) *output = 0; return;}

    DenseVectorPtr scores = input->toDenseVector();
    assert(scores && scores->getNumValues() == costs.size());
    
    std::vector<double> g;
    DenseVectorPtr gradientDirectionDense;
    const std::vector<double>* gdir = NULL;
    if (gradient)
      g.resize(costs.size(), 0.0);
    if (gradientDirection)
    {
      gradientDirectionDense = gradientDirection->toDenseVector();
      gdir = &gradientDirectionDense->getValues();
    }
    
    computeAnyLoss(scores->getValues(), costs, output, gdir, gradient ? &g : NULL);

    if (gradient)
      gradient->set(DenseVectorPtr(new DenseVector(g)));
  }
  
protected:
  void computeAnyLoss(const std::vector<double>& scores, const std::vector<double>& costs,
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
    assert(topRankIndex != (size_t)-1);
    double topRankCost = costs[topRankIndex];
    
    if (output)
      *output = 0.0;
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
    
    if (!numPairs)
      return;
    if (output)
      *output /= numPairs;
    if (gradient)
      for (size_t i = 0; i < gradient->size(); ++i)
        (*gradient)[i] /= numPairs;    
  }
};

template<class DiscriminantLoss>
inline BestAgainstAllLossFunction<DiscriminantLoss> bestAgainstAllLossFunction(const ScalarFunction<DiscriminantLoss>& discriminantFunction)
  {return BestAgainstAllLossFunction<DiscriminantLoss>(static_cast<const DiscriminantLoss& >(discriminantFunction));}

STATIC_RANKING_LOSS_FUNCTION(bestAgainstAllLoss, BestAgainstAllLoss, BestAgainstAllLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_BEST_AGAINST_ALL_LOSS_H_
