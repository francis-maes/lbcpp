/*-----------------------------------------.---------------------------------.
| Filename: GaussianMultivariateSampler.h  | GaussianMultivariateSampler     |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 11 mai 2011  11:55:19          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_GAUSSIAN_MULTIVARIATE_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_GAUSSIAN_MULTIVARIATE_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"

namespace lbcpp
{

class GaussianMultivariateSampler;
typedef ReferenceCountedObjectPtr<GaussianMultivariateSampler> GaussianMultivariateSamplerPtr;

class GaussianMultivariateSampler : public ContinuousSampler
{
public:
  GaussianMultivariateSampler()
    : maxIt(100), tolerance(0.01), numVariables(0), numClusters(0)
  {
  }

  GaussianMultivariateSampler(size_t maxIt, double tolerance, MatrixPtr& initialProbabilities,
      std::vector<MatrixPtr>& initialMean, std::vector<MatrixPtr>& initialStd)
    : maxIt(maxIt), tolerance(tolerance),
      numVariables(initialMean[0]->getNumRows()), numClusters(initialProbabilities->getNumRows())
  {
    probabilities = new DoubleMatrix(numClusters, 1, 1.0 / numClusters);
    double norm = 0;
    for (size_t j = 0; j < numClusters; j++)
    {
      probabilities->setElement(j, 0, initialProbabilities->getElement(j, 0));
      norm += probabilities->getElement(j, 0).getDouble();
    }
    // ensures that variables are normalized
    double factor = 1.0 / norm;
    for (size_t j = 0; j < numClusters; j++)
      probabilities->setElement(j, 0, probabilities->getElement(j, 0).getDouble() * factor);

    means = std::vector<MatrixPtr>(numClusters);
    covarianceMatrix = std::vector<MatrixPtr>(numClusters);
    for (size_t j = 0; j < numClusters; j++)
    {
      means[j] = new DoubleMatrix(numVariables, 1, 0.0);
      for (size_t k = 0; k < numVariables; k++)
        means[j]->setElement(k, 0, initialMean[j]->getElement(k, 0));

      covarianceMatrix[j] = new DoubleMatrix(numVariables, numVariables, 0.0);
      for (size_t k = 0; k < covarianceMatrix[j]->getNumRows(); k++)
        for (size_t l = 0; l < covarianceMatrix[j]->getNumColumns(); l++)
          covarianceMatrix[j]->setElement(k, l, initialStd[j]->getElement(k, l));
    }
  }
/*
  GaussianMultivariateSampler(const GaussianMultivariateSampler& copy)
    : ContinuousSampler(copy.mean, copy.std), maxIt(copy.maxIt), tolerance(copy.tolerance),
        numVariables(copy.numVariables), numClusters(copy.numClusters)
  {
    probabilities = new DoubleMatrix(copy.probabilities->getNumRows(), 1);
    for (size_t j = 0; j < copy.probabilities->getNumRows(); j++)
      probabilities->setElement(j, 0, copy.probabilities->getElement(j, 0));

    means = std::vector<MatrixPtr>(copy.numClusters);
    covarianceMatrix = std::vector<MatrixPtr>(copy.numClusters);

    for (size_t j = 0; j < copy.numClusters; j++)
    {
      means[j] = new DoubleMatrix(copy.means[j]->getNumRows(), 1);
      for (size_t k = 0; k < copy.means[j]->getNumRows(); k++)
        means[j]->setElement(k, 0, copy.means[j]->getElement(k, 0));

      covarianceMatrix[j] = new DoubleMatrix(copy.covarianceMatrix[0]->getNumRows(),
          copy.covarianceMatrix[0]->getNumColumns(), 0);
      for (size_t k = 0; k < copy.covarianceMatrix[j]->getNumRows(); k++)
        for (size_t l = 0; l < copy.covarianceMatrix[j]->getNumColumns(); l++)
          covarianceMatrix[j]->setElement(k, l, copy.covarianceMatrix[j]->getElement(k, l));
    }
  }*/

  /*
   * The variable returned is a n by 1 matrix containing the n values sampled.
   */
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    DenseDoubleVectorPtr proba = new DenseDoubleVector(actionClass, numClusters, 0.0);
    for (size_t i = 0; i < numClusters; i++)
      proba->setValue(i, probabilities->getElement(i, 0).getDouble());
    size_t distrib = random->sampleWithProbabilities(proba->getValues());

    MatrixPtr normalSeed = new DoubleMatrix(numVariables, 1, 0.0);
    for (size_t i = 0; i < numVariables; i++)
      normalSeed->setElement(i, 0, Variable(random->sampleDoubleFromGaussian(0, 1)));

    MatrixPtr chol = choleskyDecomposition(covarianceMatrix[distrib]);
    MatrixPtr result = matrixProduct(chol, normalSeed);

    for (size_t i = 0; i < result->getNumRows(); i++)
      result->setElement(i, 0, Variable(result->getElement(i, 0).getDouble()
          + means[distrib]->getElement(i, 0).getDouble()));

    return Variable(result);
  }

  virtual void learn(ExecutionContext& context, const std::vector<std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() < 2 * numClusters)
    {
      context.warningCallback(T("Not enough values in dataset to ensure correct EM. Nothing done."));
      return;
    }

    std::vector<MatrixPtr> vals(dataset.size());
    for (size_t i = 0; i < dataset.size(); i++)
      vals[i] = dataset[i].first.getObjectAndCast<Matrix> ();

    double oldLL = computeLogLikelihood(vals, probabilities, means, covarianceMatrix);
    double newLL = oldLL + 2 * oldLL * tolerance;

    for (size_t i = 0; ((std::abs((oldLL - newLL) / oldLL) > tolerance) && (i < maxIt)); i++)
    {
      MatrixPtr gammas;
      MatrixPtr newProbabilities;
      estimationStep(vals, probabilities, means, covarianceMatrix, gammas, newProbabilities);
      maximizationStep(vals, gammas, newProbabilities, probabilities, means, covarianceMatrix);

      oldLL = newLL;
      newLL = computeLogLikelihood(vals, probabilities, means, covarianceMatrix);
    }
  }

  /**
   * Fills probabilitiesToUpdate, meansToUpdate and covariancesToUpdate
   * with the values computed.
   */
  void maximizationStep(std::vector<MatrixPtr>& dataset, MatrixPtr& gammas,
      MatrixPtr& newProbabilities, MatrixPtr& probabilitiesToUpdate,
      std::vector<MatrixPtr>& meansToUpdate, std::vector<MatrixPtr>& covariancesToUpdate)
  {
    // probabilities
    probabilitiesToUpdate = new DoubleMatrix(newProbabilities->getNumRows(),
        newProbabilities->getNumColumns());
    for (size_t j = 0; j < newProbabilities->getNumRows(); j++)
      probabilitiesToUpdate->setElement(j, 0, Variable(
          newProbabilities->getElement(j, 0).getDouble() / (double)dataset.size()));

    // means
    meansToUpdate = std::vector<MatrixPtr>(newProbabilities->getNumRows());
    for (size_t j = 0; j < meansToUpdate.size(); j++)
    {
      MatrixPtr tempSum = new DoubleMatrix(dataset[0]->getNumRows(), 1);

      for (size_t i = 0; i < dataset.size(); i++)
        tempSum = addMatrix(tempSum, scalarMultiplyMatrix(dataset[i],
            gammas->getElement(i, j).getDouble()));
      meansToUpdate[j] = scalarMultiplyMatrix(tempSum, 1.0
          / newProbabilities->getElement(j, 0).getDouble());
    }

    // covariances
    covariancesToUpdate = std::vector<MatrixPtr>(newProbabilities->getNumRows());
    for (size_t j = 0; j < covariancesToUpdate.size(); j++)
    {
      MatrixPtr tempSub = new DoubleMatrix(dataset[0]->getNumRows(), 1);
      MatrixPtr acc = new DoubleMatrix(dataset[0]->getNumRows(), dataset[0]->getNumRows());
      for (size_t i = 0; i < dataset.size(); i++)
      {
        tempSub = subtractMatrix(dataset[i], meansToUpdate[j]);
        acc = addMatrix(acc, scalarMultiplyMatrix(matrixProduct(tempSub, transposeMatrix(tempSub)),
            gammas->getElement(i, j).getDouble()));
      }
      covariancesToUpdate[j] = scalarMultiplyMatrix(acc, 1.0
          / newProbabilities->getElement(j, 0).getDouble());
    }
  }

  /**
   * Fills gammas and newProbabilities with the values computed.
   */
  void estimationStep(std::vector<MatrixPtr>& dataset, MatrixPtr& probabilities, std::vector<
      MatrixPtr>& means, std::vector<MatrixPtr>& covariances, MatrixPtr& gammas,
      MatrixPtr& newProbabilities)
  {
    gammas = new DoubleMatrix(dataset.size(), means.size());
    std::vector<MatrixPtr> invCov(covariances.size());
    for (size_t i = 0; i < covariances.size(); i++)
      invCov[i] = inverseMatrix(covariances[i]);

    for (size_t i = 0; i < gammas->getNumRows(); i++)
    {
      double norm = 0;
      for (size_t j = 0; j < gammas->getNumColumns(); j++)
      {
        double increment = probabilities->getElement(j, 0).getDouble() * computeProbability(
            dataset[i], means[j], covariances[j], invCov[j]);
        gammas->setElement(i, j, Variable(increment));
        norm += increment;
      }

      double invNorm = 1.0 / norm;
      for (size_t j = 0; j < gammas->getNumColumns(); j++)
        gammas->setElement(i, j, Variable(gammas->getElement(i, j).getDouble() * invNorm));
    }

    newProbabilities = new DoubleMatrix(probabilities->getNumRows(), 1);
    for (size_t j = 0; j < gammas->getNumColumns(); j++)
    {
      double tempSum = 0;
      for (size_t i = 0; i < gammas->getNumRows(); i++)
        tempSum += gammas->getElement(i, j).getDouble();
      newProbabilities->setElement(j, 0, Variable(tempSum));
    }
  }

  /**
   * dataset vector containing m matrices of size n by 1, where n is the dimension
   * and m the number of samples.
   */
  double computeLogLikelihood(std::vector<MatrixPtr>& dataset, MatrixPtr& probabilities,
      std::vector<MatrixPtr>& means, std::vector<MatrixPtr>& covariances)
  {
    std::vector<MatrixPtr> invCov(covariances.size());
    for (size_t i = 0; i < covariances.size(); i++)
      invCov[i] = inverseMatrix(covariances[i]);

    double ll = 0;

    for (size_t i = 0; i < dataset.size(); i++)
    {
      double tempSum = 0;
      for (size_t j = 0; j < probabilities->getNumRows(); j++)
        tempSum += probabilities->getElement(j, 0).getDouble() * computeProbability(dataset[i],
            means[j], covariances[j], invCov[j]);

      ll += std::log(tempSum);
    }

    return ll / (double)dataset.size();
  }

  /**
   * observed and mean are n by 1 matrices, where n is the dimension of the space.
   * normalizationFactor is the factor by which the density has to be multiplied
   * to be normalized.
   */
  double computeProbability(MatrixPtr& observed, MatrixPtr& mean, MatrixPtr& covariance,
      MatrixPtr& inverseCovariance, double normalizationFactor = -1)
  {
    double denominator;
    if (normalizationFactor <= 0)
      denominator = 1.0 / (std::pow(2 * M_PI, (double)observed->getNumRows() / 2.0) * std::pow(
          determinantMatrix(covariance), 0.5));
    else
      denominator = normalizationFactor;

    MatrixPtr temp = subtractMatrix(observed, mean);
    MatrixPtr temp2 = matrixProduct(inverseCovariance, temp);
    MatrixPtr temp3 = transposeMatrix(temp);
    MatrixPtr result = matrixProduct(temp3, temp2);
    double numerator = std::exp(-0.5 * result->getElement(0, 0).getDouble());

    return numerator * denominator;
  }

  double computeNormalizationFactor(size_t dimension, MatrixPtr& covariance)
  {
    return 1.0 / (std::pow(2 * M_PI, (double)dimension / 2.0) * std::pow(determinantMatrix(
        covariance), 0.5));
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

  MatrixPtr matrixProduct(MatrixPtr a, MatrixPtr b) const
  {
    MatrixPtr temp = new DoubleMatrix(a->getNumRows(), b->getNumColumns(), 0.0);

    for (size_t i = 0; i < a->getNumRows(); i++)
      for (size_t j = 0; j < b->getNumColumns(); j++)
        for (size_t k = 0; k < a->getNumColumns(); k++)
        {
          double value = temp->getElement(i, j).getDouble();
          value += a->getElement(i, k).getDouble() * b->getElement(k, j).getDouble();
          temp->setElement(i, j, Variable(value));
        }

    return temp;
  }

  void printMatrix(MatrixPtr m) const
  {
    for (size_t i = 0; i < m->getNumRows(); i++)
    {
      for (size_t j = 0; j < m->getNumColumns(); j++)
        std::cout << m->getElement(i, j).getDouble() << " : ";
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  MatrixPtr transposeMatrix(MatrixPtr a) const
  {
    MatrixPtr temp = new DoubleMatrix(a->getNumColumns(), a->getNumRows(), 0.0);
    for (size_t i = 0; i < a->getNumRows(); i++)
      for (size_t j = 0; j < a->getNumColumns(); j++)
        temp->setElement(j, i, a->getElement(i, j));

    return temp;
  }

  MatrixPtr addMatrix(MatrixPtr a, MatrixPtr b)
  {
    MatrixPtr temp = new DoubleMatrix(a->getNumRows(), a->getNumColumns(), 0.0);
    for (size_t i = 0; i < a->getNumRows(); i++)
      for (size_t j = 0; j < a->getNumColumns(); j++)
        temp->setElement(i, j, Variable(a->getElement(i, j).getDouble()
            + b->getElement(i, j).getDouble()));

    return temp;
  }

  MatrixPtr subtractMatrix(MatrixPtr a, MatrixPtr b)
  {
    MatrixPtr temp = new DoubleMatrix(a->getNumRows(), a->getNumColumns(), 0.0);
    for (size_t i = 0; i < a->getNumRows(); i++)
      for (size_t j = 0; j < a->getNumColumns(); j++)
        temp->setElement(i, j, Variable(a->getElement(i, j).getDouble()
            - b->getElement(i, j).getDouble()));

    return temp;
  }

  MatrixPtr scalarMultiplyMatrix(MatrixPtr a, double b)
  {
    MatrixPtr temp = new DoubleMatrix(a->getNumRows(), a->getNumColumns(), 0.0);
    for (size_t i = 0; i < a->getNumRows(); i++)
      for (size_t j = 0; j < a->getNumColumns(); j++)
        temp->setElement(i, j, Variable(b * a->getElement(i, j).getDouble()));

    return temp;
  }

  /**
   * Works only with 2 by 2 matrices.
   */
  MatrixPtr inverseMatrix(MatrixPtr a)
  {
    MatrixPtr temp = new DoubleMatrix(a->getNumRows(), a->getNumColumns(), 0.0);
    double determinant = a->getElement(0, 0).getDouble() * a->getElement(1, 1).getDouble()
        - a->getElement(0, 1).getDouble() * a->getElement(1, 0).getDouble();
    temp->setElement(0, 0, a->getElement(1, 1));
    temp->setElement(0, 1, Variable(a->getElement(0, 1).getDouble() * (-1.0)));
    temp->setElement(1, 0, Variable(a->getElement(1, 0).getDouble() * (-1.0)));
    temp->setElement(1, 1, a->getElement(0, 0));

    temp = scalarMultiplyMatrix(temp, 1.0 / determinant);
    return temp;
  }

  double normOneMatrix(MatrixPtr a)
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
  double determinantMatrix(MatrixPtr a)
  {
    double determinant = a->getElement(0, 0).getDouble() * a->getElement(1, 1).getDouble()
        - a->getElement(0, 1).getDouble() * a->getElement(1, 0).getDouble();
    return determinant;
  }

protected:
  friend class GaussianMultivariateSamplerClass;
  double tolerance;
  size_t maxIt;
  size_t numVariables;
  size_t numClusters;
  MatrixPtr probabilities;
  std::vector<MatrixPtr> means;
  std::vector<MatrixPtr> covarianceMatrix;
};

}; /* namespace lbcpp */


#endif //! LBCPP_PROTEINS_ROSETTA_GAUSSIAN_MULTIVARIATE_SAMPLER_H_
