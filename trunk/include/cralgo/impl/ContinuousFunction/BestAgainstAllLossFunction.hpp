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
  : public RankingLossFunction< BestAgainstAllLossFunction<DiscriminantLoss> >
{
  typedef RankingLossFunction< BestAgainstAllLossFunction<DiscriminantLoss> > BaseClass;
  
  BestAgainstAllLossFunction(const DiscriminantLoss& discriminantLoss)
    : discriminantLoss(discriminantLoss) {}
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
    if (gradient)
      g.resize(costs.size(), 0.0);
    
    computeAnyLoss(scores->getValues(), costs, output, gradientDirection, gradient ? &g : NULL);

    if (gradient)
      gradient->set(DenseVectorPtr(new DenseVector(g)));
  }
  
protected:
  void computeAnyLoss(const std::vector<double>& scores, const std::vector<double>& costs,
                              double* output, const FeatureGeneratorPtr gradientDirection, std::vector<double>* gradient) const
  {
    size_t n = scores.size();
    double topRankScore = -DBL_MAX;
    double topRankCost = 0;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      double score = scores[i];
      if (score > topRankScore)
        topRankScore = score, topRankCost = costs[i];
    }
    assert(topRankScore > -DBL_MAX);
    
    if (output)
      *output = 0.0;
    size_t numPairs = 0;
    for (size_t i = 1; i < n; ++i)
    {
      double deltaCost = costs[i] - topRankCost;
      if (deltaCost == 0)
        continue;
      ++numPairs;
      
      double deltaScore = scores[i] - topRankScore;
      if (deltaCost > 0)
        deltaScore = -deltaScore;

      double baseLossValue, baseLossDerivative;
      // FIXME: baseLossDerivativeDirection
      discriminantLoss.compute(deltaScore, output ? &baseLossValue : NULL, NULL, gradient ? &baseLossDerivative : NULL);
      if (gradient)
      {
        (*gradient)[i] -= baseLossDerivative * deltaCost;
        (*gradient)[0] += baseLossDerivative * deltaCost;
      }
      if (output)
        *output += baseLossValue * fabs(deltaCost);
    }
    
    if (!numPairs)
      return;
    if (output)
      *output /= numPairs;
    if (gradient)
      for (size_t i = 0; i < gradient->size(); ++i)
        (*gradient)[i] /= numPairs;    
  }

private:
  DiscriminantLoss discriminantLoss;
};

template<class DiscriminantLoss>
inline BestAgainstAllLossFunction<DiscriminantLoss> bestAgainstAllLossFunction(const ScalarFunction<DiscriminantLoss>& discriminantFunction)
  {return BestAgainstAllLossFunction<DiscriminantLoss>(static_cast<const DiscriminantLoss& >(discriminantFunction));}

STATIC_RANKING_LOSS_FUNCTION(bestAgainstAllLoss, BestAgainstAllLoss, BestAgainstAllLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_BEST_AGAINST_ALL_LOSS_H_
