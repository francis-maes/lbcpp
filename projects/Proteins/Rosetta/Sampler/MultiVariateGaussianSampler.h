/*-----------------------------------------.---------------------------------.
| Filename: MultiVariateGaussianSampler.h  | MultiVariateGaussianSampler     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 15 mai 2011  08:57:38          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_MULTIVARIATE_GAUSSIAN_SAMPLER_H_
# define LBCPP_SAMPLER_MULTIVARIATE_GAUSSIAN_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"

namespace lbcpp
{

class MultiVariateGaussianSampler;
typedef ReferenceCountedObjectPtr<MultiVariateGaussianSampler> MultiVariateGaussianSamplerPtr;

class MultiVariateGaussianSampler : public ScalarContinuousSampler
{
public:
  MultiVariateGaussianSampler(const DoubleMatrixPtr& initialMean, const DoubleMatrixPtr& initialStdDev)
    : numVariables(initialMean->getNumRows()), means(initialMean), covariances(initialStdDev)
  {}

  MultiVariateGaussianSampler() : numVariables(0) {}

  /*
   * The variable returned is a n by 1 matrix containing the n values sampled.
   */
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    DoubleMatrixPtr normalSeed = new DoubleMatrix(numVariables, 1, 0.0);
    for (size_t i = 0; i < numVariables; i++)
      normalSeed->setValue(i, 0, random->sampleDoubleFromGaussian(0, 1));

    DoubleMatrixPtr chol = covariances->choleskyDecomposition();
    DoubleMatrixPtr result = chol->multiplyBy(normalSeed);

    result->add(means);

    return result;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, 
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples)
  {
    size_t n = trainingSamples->getNumElements();
    jassert(n);

    means = new DoubleMatrix(numVariables, 1, 0.0);
    for (size_t i = 0; i < n; i++)
      means->add(trainingSamples->getElement(i).getObjectAndCast<DoubleMatrix> ());
    double factor = 1.0 / (double)n;
    means->multiplyByScalar(factor);

    covariances = new DoubleMatrix(numVariables, numVariables, 0.0);
    for (size_t i = 0; i < n; i++)
    {
      DoubleMatrixPtr sub = trainingSamples->getElement(i).getObjectAndCast<DoubleMatrix>();
      sub->subtract(means);
      DoubleMatrixPtr subT = sub->transpose();
      DoubleMatrixPtr product = sub->multiplyBy(subT);
      covariances->add(product);
    }
    covariances->multiplyByScalar(1.0 / (n - 1));
  }

  virtual void computeProbabilities(const ContainerPtr& data, DoubleMatrixPtr& probabilities,
      size_t numColumnToFill) const
  {
    // TODO
    for (size_t i = 0; i < data->getNumElements(); i++)
    {
//      double denominator;
//      denominator = 1.0 / (std::pow(2 * M_PI, (double)observed->getNumRows() / 2.0) * std::pow(
//          covariances->determinant(), 0.5));
//
//      DoubleMatrixPtr inverseCovariance = covariances->getInverse();
//
//      MatrixPtr temp = subtractMatrix(observed, mean);
//      MatrixPtr temp2 = matrixProduct(inverseCovariance, temp);
//      MatrixPtr temp3 = transposeMatrix(temp);
//      MatrixPtr result = matrixProduct(temp3, temp2);
//      double numerator = std::exp(-0.5 * result->getElement(0, 0).getDouble());
//
//      return numerator * denominator;
    }
  }

  virtual void updateParameters(const ContainerPtr& data, const DoubleMatrixPtr& probabilitiesForAllModels, size_t numColumn)
  {

  }

protected:
  friend class MultiVariateGaussianSamplerClass;

  size_t numVariables;
  DoubleMatrixPtr means;
  DoubleMatrixPtr covariances;
};

}; /* namespace lbcpp */


#endif //! LBCPP_SAMPLER_MULTIVARIATE_GAUSSIAN_SAMPLER_H_
