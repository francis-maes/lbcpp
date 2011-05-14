/*-----------------------------------------.---------------------------------.
| Filename: DualResidueSampler.h           | DualResidueSampler              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 12 mai 2011  14:56:29          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DUAL_RESIDUE_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DUAL_RESIDUE_SAMPLER_H_

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
    : CompositeSampler(), numResidues(1)
  {
  }

  DualResidueSampler(size_t numResidues)
    : CompositeSampler(1), numResidues(numResidues)
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
    means[0]->setElement(0, 0, Variable(0.45 * numResidues));
    means[0]->setElement(1, 0, Variable(0.45 * numResidues));
    means[1]->setElement(0, 0, Variable(0.55 * numResidues));
    means[1]->setElement(1, 0, Variable(0.55 * numResidues));

    // initialize covarianceMatrix
    std::vector<MatrixPtr> covarianceMatrix(2);
    covarianceMatrix[0] = new DoubleMatrix(2, 2, 0.0);
    double var = std::pow(0.25 * numResidues, 2.0);
    double covar = std::pow(0.01 * numResidues, 2.0);
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

    samplers[0] = p;
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    MatrixPtr temp = samplers[0]->sample(context, random, inputs).getObjectAndCast<Matrix>();
    double rand1 = std::abs(temp->getElement(0, 0).getDouble());
    double rand2 = std::abs(temp->getElement(1, 0).getDouble());

    size_t firstResidue = juce::jlimit((size_t)0, (size_t)(numResidues - 1), (size_t)std::floor(rand1));
    size_t secondResidue = juce::jlimit((size_t)0, (size_t)(numResidues - 1), (size_t)std::floor(rand2));

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

    PairPtr residuePair = new Pair(firstResidue, secondResidue);
    return Variable(residuePair);
  }

  /**
   * dataset = first : a Variable of Pair type containing the residues observed
   * expressed in size_t variables.
   */
  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    if ((dataset.size() < 2) || (numResidues <= 0))
      return;

    std::vector<Variable> data;
    
    for (size_t i = 0; i < dataset.size(); i++)
    {
      PairPtr residuePair = dataset[i].getObjectAndCast<Pair> ();
      size_t res1 = (size_t)residuePair->getFirst().getInteger();
      size_t res2 = (size_t)residuePair->getSecond().getInteger();

      DoubleMatrixPtr residuePair2 = new DoubleMatrix(2, 1);
      residuePair2->setValue(0, 0, (double)res1);
      residuePair2->setValue(1, 0, (double)res2);
      data.push_back(residuePair2);
    }

    samplers[0]->learn(context, data);
  }

protected:
  friend class DualResidueSamplerClass;
  size_t numResidues;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DUAL_RESIDUE_SAMPLER_H_
