/*-----------------------------------------.---------------------------------.
| Filename: DualResidueSampler.h           | DualResidueSampler              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12 mai 2011  14:56:29          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DUAL_RESIDUE_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DUAL_RESIDUE_SAMPLER_H_

# define MAX_INTERVAL_VALUE_DUAL 100

# include "precompiled.h"
# include "../Sampler.h"
# include "GaussianMultivariateSampler.h"

namespace lbcpp
{

class DualResidueSampler;
typedef ReferenceCountedObjectPtr<DualResidueSampler> DualResidueSamplerPtr;

class DualResidueSampler : public CompositeSampler
{
public:
  DualResidueSampler()
    : CompositeSampler(), numResidues(1), residuesDeviation(0)
  {
  }

  DualResidueSampler(size_t numResidues, size_t residuesDeviation = 0)
    : CompositeSampler(1), numResidues(numResidues), residuesDeviation(residuesDeviation)
  {
    // only 2 clusters for GMM
    MatrixPtr probabilities = new DoubleMatrix(2, 1, 0.0);

    // initialize probabilities for each cluster
    probabilities->setElement(0, 0, Variable(0.5));
    probabilities->setElement(1, 0, Variable(0.5));

    // initialize means
    std::vector<MatrixPtr> means(2);
    means[0] = new DoubleMatrix(2, 1, 0.0);
    means[1] = new DoubleMatrix(2, 1, 0.0);
    means[0]->setElement(0, 0, Variable(0.45 * MAX_INTERVAL_VALUE_DUAL));
    means[0]->setElement(1, 0, Variable(0.45 * MAX_INTERVAL_VALUE_DUAL));
    means[1]->setElement(0, 0, Variable(0.55 * MAX_INTERVAL_VALUE_DUAL));
    means[1]->setElement(1, 0, Variable(0.55 * MAX_INTERVAL_VALUE_DUAL));

    // initialize covarianceMatrix
    std::vector<MatrixPtr> covarianceMatrix(2);
    covarianceMatrix[0] = new DoubleMatrix(2, 2, 0.0);
    double var = std::pow(0.25 * MAX_INTERVAL_VALUE_DUAL, 2.0);
    double covar = std::pow(0.01 * MAX_INTERVAL_VALUE_DUAL, 2.0);
    covarianceMatrix[0]->setElement(0, 0, Variable(var));
    covarianceMatrix[0]->setElement(0, 1, Variable(covar));
    covarianceMatrix[0]->setElement(1, 0, Variable(covar));
    covarianceMatrix[0]->setElement(1, 1, Variable(var));
    covarianceMatrix[1] = new DoubleMatrix(2, 2, 0.0);
    covarianceMatrix[1]->setElement(0, 0, Variable(var));
    covarianceMatrix[1]->setElement(0, 1, Variable(covar));
    covarianceMatrix[1]->setElement(1, 0, Variable(covar));
    covarianceMatrix[1]->setElement(1, 1, Variable(var));
    GaussianMultivariateSamplerPtr p = new GaussianMultivariateSampler(1000, 0.001, probabilities,
        means, covarianceMatrix);

    sons[0] = p;
  }

  DualResidueSampler(const DualResidueSampler& sampler)
    : CompositeSampler(1), numResidues(sampler.numResidues),
      residuesDeviation(sampler.residuesDeviation)
  {
    GaussianMultivariateSamplerPtr temp = new GaussianMultivariateSampler(
        (*(sampler.sons[0].getObjectAndCast<GaussianMultivariateSampler> ())));
    sons[0] = temp;
  }

  ~DualResidueSampler()
  {
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    MatrixPtr temp =
        (sons[0].getObjectAndCast<Sampler> ())->sample(context, random, NULL).getObjectAndCast<
            Matrix> ();
    double rand1 = std::abs(temp->getElement(0, 0).getDouble());
    double rand2 = std::abs(temp->getElement(1, 0).getDouble());
    rand1 = rand1 > (1 * MAX_INTERVAL_VALUE_DUAL) ? std::abs((2 * MAX_INTERVAL_VALUE_DUAL) - rand1)
        : rand1;
    rand2 = rand2 > (1 * MAX_INTERVAL_VALUE_DUAL) ? std::abs((2 * MAX_INTERVAL_VALUE_DUAL) - rand2)
        : rand2;

    size_t firstResidue = (size_t)std::floor(rand1 * numResidues / (double)MAX_INTERVAL_VALUE_DUAL);
    size_t secondResidue = (size_t)std::floor(rand2 * numResidues / (double)MAX_INTERVAL_VALUE_DUAL);

    if (firstResidue == numResidues)
      firstResidue--;
    if (secondResidue == numResidues)
      secondResidue--;

    if (std::abs((int)(firstResidue - secondResidue)) <= 1)
    {
      if (firstResidue < secondResidue)
      {
        firstResidue = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)firstResidue - 1);
        secondResidue = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)secondResidue + 1);
      }
      else if (firstResidue > secondResidue)
      {
        firstResidue = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)firstResidue + 1);
        secondResidue = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)secondResidue - 1);
      }
      else
      {
        firstResidue = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)firstResidue - 2);
        secondResidue = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)secondResidue + 2);
      }
    }

    MatrixPtr result = new DoubleMatrix(2, 1);
    result->setElement(0, 0, Variable((double)firstResidue));
    result->setElement(1, 0, Variable((double)secondResidue));
    return Variable(result);
  }

  /**
   * dataset = first : a Variable of DoubleMatrix type containing the residues observed
   * expressed in terms of their integer value represented in double.
   *           second : not yet used.
   */
  virtual void learn(ExecutionContext& context, const std::vector<std::pair<Variable, Variable> >& dataset)
  {
    if ((dataset.size() < 2) || (numResidues <= 0))
      return;

    std::vector<std::pair<Variable, Variable> > data;
    double scaleFactor = (double)MAX_INTERVAL_VALUE_DUAL / (double)numResidues;
    double varianceIncrement = (double)residuesDeviation * scaleFactor;
    
    RandomGeneratorPtr random = new RandomGenerator(); // francis: I do not understand why random is needed here ..
    for (size_t i = 0; i < dataset.size(); i++)
    {
      MatrixPtr residuePair = dataset[i].first.getObjectAndCast<Matrix> ();
      size_t res1 = (size_t)(residuePair->getElement(0, 0).getDouble());
      size_t res2 = (size_t)(residuePair->getElement(1, 0).getDouble());
      double value1 = (double)res1 * scaleFactor;
      double value2 = (double)res2 * scaleFactor;
      value1 = std::abs(value1 + varianceIncrement * random->sampleDoubleFromGaussian(0, 1));
      value1 = value1 > (1 * MAX_INTERVAL_VALUE_DUAL) ? std::abs((2 * MAX_INTERVAL_VALUE_DUAL)
          - value1) : value1;
      value2 = std::abs(value2 + varianceIncrement * random->sampleDoubleFromGaussian(0, 1));
      value2 = value2 > (1 * MAX_INTERVAL_VALUE_DUAL) ? std::abs((2 * MAX_INTERVAL_VALUE_DUAL)
          - value2) : value2;
      residuePair = new DoubleMatrix(2, 1);
      residuePair->setElement(0, 0, Variable(value1));
      residuePair->setElement(1, 0, Variable(value2));
      data.push_back(std::pair<Variable, Variable>(Variable(residuePair), Variable()));
    }

    sons[0].getObjectAndCast<Sampler> ()->learn(context, data);
  }

protected:
  friend class DualResidueSamplerClass;
  size_t numResidues;
  size_t residuesDeviation;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DUAL_RESIDUE_SAMPLER_H_
