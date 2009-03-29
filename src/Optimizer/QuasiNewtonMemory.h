/*-----------------------------------------.---------------------------------.
| Filename: QuasiNewtonMemory.h            | Quasi Newton Memory             |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 19:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_OPTIMIZER_QUASI_NEWTON_MEMORY_H_
# define CRALGO_OPTIMIZER_QUASI_NEWTON_MEMORY_H_

# include <cralgo/FeatureGenerator.h>
# include <deque>

namespace cralgo
{

class QuasiNewtonMemory
{
public:
  QuasiNewtonMemory(size_t memorySize = 10)
    : memorySize(memorySize) {}
    
  void mapDirectionByInverseHessian(DenseVectorPtr dir)
  {
    int count = (int)parameterDeltas.size();
    if (!count)
      return;
     
    std::vector<double> alphas(count);
    for (int i = count - 1; i >= 0; i--)
    {
      alphas[i] = -(parameterDeltas[i]->dotProduct(dir)) / dotProducts[i];
      dir->addWeighted(gradientDeltas[i], alphas[i]);
    }
    
    const FeatureGeneratorPtr lastGradientDelta = gradientDeltas.back();
    double lastGradientDeltaNorm = lastGradientDelta->l2norm();
    dir->multiplyByScalar(dotProducts[count - 1] / (lastGradientDeltaNorm * lastGradientDeltaNorm));

    for (int i = 0; i < count; i++)
    {
      double beta = gradientDeltas[i]->dotProduct(dir) / dotProducts[i];
      dir->addWeighted(parameterDeltas[i], -alphas[i] - beta);
    }
  }
  
  void shift(FeatureGeneratorPtr& currentParameters, FeatureGeneratorPtr& currentGradient,
             const FeatureGeneratorPtr newParameters, const FeatureGeneratorPtr newGradient)
  {
    if (parameterDeltas.size() >= memorySize)
    {
      parameterDeltas.pop_front();
      gradientDeltas.pop_front();
      dotProducts.pop_front();
    }
    FeatureGeneratorPtr parametersDelta = FeatureGenerator::difference(newParameters, currentParameters, true);
    FeatureGeneratorPtr gradientDelta = FeatureGenerator::difference(newGradient, currentGradient, true);
    parameterDeltas.push_back(parametersDelta);
    gradientDeltas.push_back(gradientDelta);
    dotProducts.push_back(parametersDelta->dotProduct(gradientDelta));
    currentParameters = newParameters;
    currentGradient = newGradient;    
  }
  
private:
  size_t memorySize;
  std::deque<FeatureGeneratorPtr> parameterDeltas, gradientDeltas;
  std::deque<double> dotProducts;  
};

}; /* namespace cralgo */

#endif // !CRALGO_OPTIMIZER_QUASI_NEWTON_MEMORY_H_
