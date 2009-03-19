/*-----------------------------------------.---------------------------------.
| Filename: AllPairsLossFunction.hpp       | All-pairs ranking loss          |
| Author  : Francis Maes                   |                                 |
| Started : 17/03/2009 19:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_ALL_PAIRS_LOSS_H_
# define CRALGO_IMPL_FUNCTION_ALL_PAIRS_LOSS_H_

# include "LossFunctions.hpp"

namespace cralgo {
namespace impl {

template<class DiscriminantLoss>
struct AllPairsLossFunction
  : public AdditiveRankingLossFunction< AllPairsLossFunction<DiscriminantLoss>, DiscriminantLoss >
{
  typedef AdditiveRankingLossFunction< AllPairsLossFunction<DiscriminantLoss>, DiscriminantLoss > BaseClass;
  
  AllPairsLossFunction(const DiscriminantLoss& discriminantLoss)
    : BaseClass(discriminantLoss) {}
  AllPairsLossFunction() {}
  
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
    
    computeAllPairsAnyLoss(scores->getValues(), costs, output, gdir, gradient ? &g : NULL);

    if (gradient)
      gradient->set(DenseVectorPtr(new DenseVector(g)));
  }
  
protected:
  void computeAllPairsAnyLoss(const std::vector<double>& scores, const std::vector<double>& costs,
                              double* output, const std::vector<double>* gradientDirection, std::vector<double>* gradient) const
  {
    size_t n = scores.size();
    if (output)
      *output = 0.0;
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
inline AllPairsLossFunction<DiscriminantLoss> allPairsLossFunction(const ScalarFunction<DiscriminantLoss>& discriminantFunction)
  {return AllPairsLossFunction<DiscriminantLoss>(static_cast<const DiscriminantLoss& >(discriminantFunction));}

STATIC_RANKING_LOSS_FUNCTION(allPairsLoss, AllPairsLoss, AllPairsLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_ALL_PAIRS_LOSS_H_
