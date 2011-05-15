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

class MultiVariateGaussianSampler : public ContinuousSampler
{
public:
  MultiVariateGaussianSampler(const DoubleMatrixPtr& initialMean, const DoubleMatrixPtr& initialStdDev)
    : numVariables(initialMean->getNumRows()), means(initialMean), covariances(initialStdDev)
  {
  }

  MultiVariateGaussianSampler() : numVariables(0) {}

  /*
   * The variable returned is a n by 1 matrix containing the n values sampled.
   */
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    DoubleMatrixPtr normalSeed = new DoubleMatrix(numVariables, 1, 0.0);
    for (size_t i = 0; i < numVariables; i++)
      normalSeed->setElement(i, 0, Variable(random->sampleDoubleFromGaussian(0, 1)));

    DoubleMatrixPtr chol = choleskyDecomposition(covariances);
    DoubleMatrixPtr result = chol->multiplyBy(normalSeed);

    result->add(means);

    return result;
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    means = new DoubleMatrix(numVariables, 1, 0.0);
    for (size_t i = 0; i < dataset.size(); i++)
      means->add(dataset[i].getObjectAndCast<DoubleMatrix> ());
    double factor = 1.0 / (double)dataset.size();
    means->multiplyByScalar(factor);

    covariances = new DoubleMatrix(numVariables, numVariables, 0.0);
    for (size_t i = 0; i < dataset.size(); i++)
    {
      DoubleMatrixPtr sub = dataset[i].getObjectAndCast<DoubleMatrix>();
      sub->subtract(means);
      sub->transpose();
      covariances->add(sub->multiplyBy(sub));
    }
    covariances->multiplyByScalar(1.0 / (dataset.size() - 1));
  }

  /**
   * Matrices
   */

  /**
   * Cholesky, works only with DoubleSymmetricMatrix. Returns the lower part.
   */
  MatrixPtr choleskyDecomposition(const MatrixPtr& a) const
  {
    MatrixPtr temp = new DoubleMatrix(a->getNumRows(), a->getNumColumns(), 0.0);
    for (size_t i = 0; i < a->getNumRows(); i++)
      for (size_t j = 0; j < a->getNumColumns(); j++)
        temp->setElement(i, j, Variable(a->getElement(i, j).getDouble()));

    // Decomposition
    temp->setElement(0, 0, Variable(std::sqrt(std::abs(temp->getElement(0, 0).getDouble()))));
    size_t N = temp->getNumColumns();
    for (size_t i = 1; i < N; i++)
      temp->setElement(i, 0, Variable(temp->getElement(i, 0).getDouble()
          / temp->getElement(0, 0).getDouble()));

    for (size_t k = 1; k < (N - 1); k++)
    {
      double sum1 = 0;

      for (size_t j = 0; j < k; j++)
        sum1 += std::pow(temp->getElement(k, j).getDouble(), 2.0);
      temp->setElement(k, k, Variable(
          std::sqrt(std::abs(temp->getElement(k, k).getDouble() - sum1))));

      for (size_t i = k + 1; i < N; i++)
      {
        double sum3 = 0;
        for (size_t j = 0; j < k; j++)
          sum3 += temp->getElement(i, j).getDouble() * temp->getElement(k, j).getDouble();
        temp->setElement(i, k, Variable((1.0 / temp->getElement(k, k).getDouble())
            * (temp->getElement(i, k).getDouble() - sum3)));
      }
    }

    double sum2 = 0;
    for (size_t j = 0; j < (N - 1); j++)
      sum2 += std::pow(temp->getElement(N - 1, j).getDouble(), 2.0);
    temp->setElement(N - 1, N - 1, Variable(std::sqrt(std::abs(
        temp->getElement(N - 1, N - 1).getDouble() - sum2))));

    // Set the upper part to 0
    for (size_t i = 0; i < a->getNumRows(); i++)
      for (size_t j = i + 1; j < a->getNumColumns(); j++)
        temp->setElement(i, j, Variable(0.0));

    return temp;
  }


  /**
   * Works only with 2 by 2 matrices.
   */
  MatrixPtr inverseMatrix(MatrixPtr a) const
  {
    DoubleMatrixPtr temp = new DoubleMatrix(a->getNumRows(), a->getNumColumns(), 0.0);
    double determinant = a->getElement(0, 0).getDouble() * a->getElement(1, 1).getDouble()
        - a->getElement(0, 1).getDouble() * a->getElement(1, 0).getDouble();
    temp->setElement(0, 0, a->getElement(1, 1));
    temp->setElement(0, 1, Variable(a->getElement(0, 1).getDouble() * (-1.0)));
    temp->setElement(1, 0, Variable(a->getElement(1, 0).getDouble() * (-1.0)));
    temp->setElement(1, 1, a->getElement(0, 0));

    temp->multiplyByScalar(1.0 / determinant);
    return temp;
  }

  double normOneMatrix(MatrixPtr a) const
  {
    double max = -1.0;
    for (size_t j = 0; j < a->getNumColumns(); j++)
    {
      double sum = 0;
      for (size_t i = 0; i < a->getNumRows(); i++)
      {
        sum += std::abs(a->getElement(i, j).getDouble());
      }
      if (sum > max)
        max = sum;
    }
    return max;
  }

  /**
   * Works only for 2 by 2 matrices.
   */
  double determinantMatrix(MatrixPtr a) const
  {
    double determinant = a->getElement(0, 0).getDouble() * a->getElement(1, 1).getDouble()
        - a->getElement(0, 1).getDouble() * a->getElement(1, 0).getDouble();
    return determinant;
  }

protected:
  friend class MultiVariateGaussianSamplerClass;

  size_t numVariables;
  DoubleMatrixPtr means;
  DoubleMatrixPtr covariances;
};

}; /* namespace lbcpp */


#endif //! LBCPP_SAMPLER_MULTIVARIATE_GAUSSIAN_SAMPLER_H_
