/*-----------------------------------------.---------------------------------.
| Filename: ResiduePairSampler.h           | ResiduePairSampler              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 15 mai 2011  17:16:11          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_RESIDUE_PAIR_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_RESIDUE_PAIR_SAMPLER_H_

# include "../Sampler.h"

namespace lbcpp
{

class ResiduePairSampler;
typedef ReferenceCountedObjectPtr<ResiduePairSampler> ResiduePairSamplerPtr;

class ResiduePairSampler : public CompositeSampler
{
public:
  ResiduePairSampler(size_t numResidues)
    : CompositeSampler(1), numResidues(numResidues)
  {
    // only 2 clusters for GMM
    MatrixPtr probabilities = new DoubleMatrix(2, 1, 0.5);

    // initialize means
    std::vector<MatrixPtr> means(2);
    means[0] = new DoubleMatrix(2, 1, 0.45 * numResidues);
    means[1] = new DoubleMatrix(2, 1, 0.55 * numResidues);

    // initialize covarianceMatrix
    std::vector<MatrixPtr> covarianceMatrix(2);
    double var = std::pow(0.25 * numResidues, 2.0);
    double covar = std::pow(0.01 * numResidues, 2.0);

    covarianceMatrix[0] = new DoubleMatrix(2, 2, covar);
    covarianceMatrix[0]->setElement(0, 0, Variable(var));
    covarianceMatrix[0]->setElement(1, 1, Variable(var));

    covarianceMatrix[1] = new DoubleMatrix(2, 2, covar);
    covarianceMatrix[1]->setElement(0, 0, Variable(var));
    covarianceMatrix[1]->setElement(1, 1, Variable(var));

    ContinuousSamplerPtr gauss0 = multiVariateGaussianSampler(means[0], covarianceMatrix[0]);
    ContinuousSamplerPtr gauss1 = multiVariateGaussianSampler(means[1], covarianceMatrix[1]);
    DenseDoubleVectorPtr probas = new DenseDoubleVector(2, 0.5);
    std::vector<SamplerPtr> mixtsamp;
    mixtsamp.push_back(gauss0);
    mixtsamp.push_back(gauss1);
    CompositeSamplerPtr p = mixtureSampler(probas, mixtsamp);

    samplers[0] = p;
  }

  ResiduePairSampler() : CompositeSampler(1), numResidues(1) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    DoubleMatrixPtr temp = samplers[0]->sample(context, random, inputs).getObjectAndCast<DoubleMatrix>();
    double rand1 = std::abs(temp->getValue(0, 0));
    double rand2 = std::abs(temp->getValue(1, 0));

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
  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                  const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    size_t n = trainingSamples->getNumElements();
    jassert((n > 0) && (numResidues > 0));

    ObjectVectorPtr residuePairs = new ObjectVector(doubleMatrixClass(), n);
    std::vector<Variable> data;

    for (size_t i = 0; i < n; i++)
    {
      PairPtr residuePair = trainingSamples->getElement(i).getObjectAndCast<Pair> ();
      size_t res1 = (size_t)residuePair->getFirst().getInteger();
      size_t res2 = (size_t)residuePair->getSecond().getInteger();

      DoubleMatrixPtr residuePair2 = new DoubleMatrix(2, 1);
      residuePair2->setValue(0, 0, (double)res1);
      residuePair2->setValue(1, 0, (double)res2);
      residuePairs->set(i, residuePair2);
    }

    if (n)
      samplers[0]->learn(context, trainingInputs, residuePairs);
    // validation data not supported
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
    {jassert(false);}

protected:
  friend class ResiduePairSamplerClass;

  size_t numResidues;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RESIDUE_PAIR_SAMPLER_H_
