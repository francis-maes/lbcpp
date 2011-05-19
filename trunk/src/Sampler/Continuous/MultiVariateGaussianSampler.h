/*-----------------------------------------.---------------------------------.
| Filename: MultiVariateGaussianSampler.h  | MultiVariateGaussianSampler     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 15 mai 2011  08:57:38          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_MULTIVARIATE_GAUSSIAN_SAMPLER_H_
# define LBCPP_SAMPLER_MULTIVARIATE_GAUSSIAN_SAMPLER_H_

# include "lbcpp/Sampler/Sampler.h"

namespace lbcpp
{

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

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
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
    double invDenominator;
    invDenominator = 1.0 / (std::pow(2 * M_PI, (double)data->getElement(0).getObjectAndCast<
        DoubleMatrix> ()->getNumRows() / 2.0) * std::pow(covariances->determinant(), 0.5));
    DoubleMatrixPtr inverseCovariance = covariances->getInverse();
    DoubleMatrixPtr tempSub;
    DoubleMatrixPtr tempSubT;
    DoubleMatrixPtr product;
    for (size_t i = 0; i < data->getNumElements(); i++)
    {
      tempSub
          = (data->getElement(i).getObjectAndCast<DoubleMatrix> ())->cloneAndCast<DoubleMatrix> ();
      tempSub->subtract(means);
      tempSubT = tempSub->transpose();
      product = inverseCovariance->multiplyBy(tempSub);
      product = tempSubT->multiplyBy(product);
      double numerator = std::exp(-0.5 * product->getValue(0, 0));

      probabilities->setValue(i, numColumnToFill, numerator * invDenominator);
    }
  }

  virtual void updateParameters(const ContainerPtr& data,
      const DoubleMatrixPtr& probabilitiesForAllModels, size_t numColumn)
  {
    // means
    DoubleMatrixPtr tempSum = new DoubleMatrix(means->getNumRows(), means->getNumColumns(), 0);
    DoubleMatrixPtr factor;
    double nj = 0;
    for (size_t i = 0; i < data->getNumElements(); i++)
    {
      factor
          = (data->getElement(i).getObjectAndCast<DoubleMatrix> ())->cloneAndCast<DoubleMatrix> ();
      factor->multiplyByScalar(probabilitiesForAllModels->getValue(i, numColumn));
      tempSum->add(factor);
      nj += probabilitiesForAllModels->getValue(i, numColumn);
    }
    tempSum->multiplyByScalar(1.0 / nj);
    means = tempSum;

    // covariances
    DoubleMatrixPtr tempSub;
    DoubleMatrixPtr tempSubT;
    DoubleMatrixPtr product;
    DoubleMatrixPtr tempCov = new DoubleMatrix(covariances->getNumRows(),
            covariances->getNumColumns(), 0);
    for (size_t i = 0; i < data->getNumElements(); i++)
    {
      tempSub = (data->getElement(i).getObjectAndCast<DoubleMatrix>())->cloneAndCast<DoubleMatrix>();
      tempSub->subtract(means);
      tempSubT = tempSub->transpose();
      product = tempSub->multiplyBy(tempSubT);
      product->multiplyByScalar(probabilitiesForAllModels->getValue(i, numColumn));
      tempCov->add(product);
    }
    tempCov->multiplyByScalar(1.0 / nj);
    covariances = tempCov;
  }

protected:
  friend class MultiVariateGaussianSamplerClass;

  size_t numVariables;
  DoubleMatrixPtr means;
  DoubleMatrixPtr covariances;
};

typedef ReferenceCountedObjectPtr<MultiVariateGaussianSampler> MultiVariateGaussianSamplerPtr;

}; /* namespace lbcpp */


#endif //! LBCPP_SAMPLER_MULTIVARIATE_GAUSSIAN_SAMPLER_H_
