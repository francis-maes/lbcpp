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

    if (trainingWeights)
    {
      // means
      DoubleMatrixPtr tempSum = new DoubleMatrix(means->getNumRows(), means->getNumColumns(), 0);
      DoubleMatrixPtr factor;
      double nj = 0;
      for (size_t i = 0; i < n; i++)
      {
        factor = (trainingSamples->getElement(i).getObjectAndCast<DoubleMatrix> ())->cloneAndCast<DoubleMatrix> ();
        factor->multiplyByScalar(trainingWeights->getValue(i));
        tempSum->add(factor);
        nj += trainingWeights->getValue(i);
      }
      tempSum->multiplyByScalar(1.0 / nj);
      means = tempSum;

      // covariances
      DoubleMatrixPtr tempSub;
      DoubleMatrixPtr tempSubT;
      DoubleMatrixPtr product;
      DoubleMatrixPtr tempCov = new DoubleMatrix(covariances->getNumRows(), covariances->getNumColumns(), 0);
      for (size_t i = 0; i < n; i++)
      {
        tempSub = (trainingSamples->getElement(i).getObjectAndCast<DoubleMatrix> ())->cloneAndCast<DoubleMatrix> ();
        tempSub->subtract(means);
        tempSubT = tempSub->transpose();
        product = tempSub->multiplyBy(tempSubT);
        product->multiplyByScalar(trainingWeights->getValue(i));
        tempCov->add(product);
      }
      tempCov->multiplyByScalar(1.0 / nj);
      covariances = tempCov;
    }
    else
    {
      means = new DoubleMatrix(numVariables, 1, 0.0);
      for (size_t i = 0; i < n; i++)
        means->add(trainingSamples->getElement(i).getObjectAndCast<DoubleMatrix> ());
      double factor = 1.0 / (double)n;
      means->multiplyByScalar(factor);

      covariances = new DoubleMatrix(numVariables, numVariables, 0.0);
      for (size_t i = 0; i < n; i++)
      {
        DoubleMatrixPtr sub = trainingSamples->getElement(i).getObjectAndCast<DoubleMatrix> ();
        sub->subtract(means);
        DoubleMatrixPtr subT = sub->transpose();
        DoubleMatrixPtr product = sub->multiplyBy(subT);
        covariances->add(product);
      }
      covariances->multiplyByScalar(1.0 / (n - 1));
    }
  }

  virtual DenseDoubleVectorPtr computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(samples->getNumElements(), 0);
    double invDenominator;
    invDenominator = 1.0 / (std::pow(2 * M_PI, (double)numVariables / 2.0) * std::sqrt(covariances->determinant()));
    DoubleMatrixPtr inverseCovariance = covariances->getInverse();
    DoubleMatrixPtr tempSub;
    DoubleMatrixPtr tempSubT;
    DoubleMatrixPtr product;
    for (size_t i = 0; i < samples->getNumElements(); i++)
    {
      tempSub = (samples->getElement(i).getObjectAndCast<DoubleMatrix> ())->cloneAndCast<
          DoubleMatrix> ();
      tempSub->subtract(means);
      tempSubT = tempSub->transpose();
      product = inverseCovariance->multiplyBy(tempSub);
      product = tempSubT->multiplyBy(product);
      double numerator = std::exp(-0.5 * product->getValue(0, 0));

      result->setValue(i, numerator * invDenominator);
    }
    return result;
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
