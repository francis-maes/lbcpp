/*-----------------------------------------.---------------------------------.
| Filename: QuasiNewtonMemory.h            | Quasi Newton Memory             |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 19:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_QUASI_NEWTON_MEMORY_H_
# define LBCPP_OPTIMIZER_QUASI_NEWTON_MEMORY_H_

# include <lbcpp/FeatureGenerator.h>
# include <deque>

namespace lbcpp
{

class QuasiNewtonMemory
{
public:
  QuasiNewtonMemory(size_t memorySize = 10)
    : memorySize(memorySize) {}
    
  void reset()
  {
    parametersDeltas.clear();
    gradientDeltas.clear();
    dotProducts.clear();
  }
    
  void mapDirectionByInverseHessian(DenseVectorPtr dir)
  {
    int count = (int)parametersDeltas.size();
    if (!count)
      return;
     
    std::vector<double> alphas(count);
    for (int i = count - 1; i >= 0; i--)
    {
      alphas[i] = -(parametersDeltas[i]->dotProduct(dir)) / dotProducts[i];
      dir->addWeighted(gradientDeltas[i], alphas[i]);
    }
    
    const FeatureGeneratorPtr lastGradientDelta = gradientDeltas.back();
    double lastGradientDeltaNorm = lastGradientDelta->l2norm();
    dir->multiplyByScalar(dotProducts[count - 1] / (lastGradientDeltaNorm * lastGradientDeltaNorm));

    for (int i = 0; i < count; i++)
    {
      double beta = gradientDeltas[i]->dotProduct(dir) / dotProducts[i];
      dir->addWeighted(parametersDeltas[i], -alphas[i] - beta);
    }
  }
  
  void shift(FeatureGeneratorPtr& currentParameters, FeatureGeneratorPtr& currentGradient,
             const FeatureGeneratorPtr newParameters, const FeatureGeneratorPtr newGradient)
  {
    if (parametersDeltas.size() >= memorySize)
    {
      parametersDeltas.pop_front();
      gradientDeltas.pop_front();
      dotProducts.pop_front();
    }
    FeatureGeneratorPtr parametersDelta = difference(newParameters, currentParameters, true);
    FeatureGeneratorPtr gradientDelta = difference(newGradient, currentGradient, true);
    parametersDeltas.push_back(parametersDelta);
    gradientDeltas.push_back(gradientDelta);
    dotProducts.push_back(parametersDelta->dotProduct(gradientDelta));
    currentParameters = newParameters;
    currentGradient = newGradient;    
  }
  
private:
  size_t memorySize;
  std::deque<FeatureGeneratorPtr> parametersDeltas, gradientDeltas;
  std::deque<double> dotProducts;  
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_QUASI_NEWTON_MEMORY_H_
