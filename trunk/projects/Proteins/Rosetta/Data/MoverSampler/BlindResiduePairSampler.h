/*-----------------------------------------.---------------------------------.
| Filename: BlindResiduePairSampler.h      | Blind Residue Pair Sampler      |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 8, 2012  4:46:54 PM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDRESIDUEPAIRSAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDRESIDUEPAIRSAMPLER_H_

# include "precompiled.h"
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class BlindResiduePairSampler : public CompositeSampler
{
public:
  BlindResiduePairSampler() : numResidues(0) {}
  BlindResiduePairSampler(size_t numResidues)
    : numResidues(numResidues)
  {
    DenseDoubleVectorPtr probas = new DenseDoubleVector(numResidues, 1.0 / numResidues);
    samplers.push_back(enumerationSampler(probas));
    DenseDoubleVectorPtr probas2 = new DenseDoubleVector(numResidues, 1.0 / numResidues);
    samplers.push_back(enumerationSampler(probas2));
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    size_t indexResidueOne = (size_t)samplers[0]->sample(context, random).getInteger();
    size_t indexResidueTwo = (size_t)samplers[1]->sample(context, random).getInteger();

    if (std::abs((int)indexResidueOne - (int)indexResidueTwo) < 2)
    {
      indexResidueOne = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)indexResidueOne - 2);
      indexResidueTwo = (size_t)juce::jlimit(0, (int)numResidues - 1, (int)indexResidueTwo + 2);
    }
    PairPtr residues = new Pair(indexResidueOne, indexResidueTwo);
    return residues;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& supervisionWeights = DenseDoubleVectorPtr())
    {}

  virtual DenseDoubleVectorPtr computeLogProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
  {
    double jointProbability = std::log(pow(1.0 / numResidues, 2.0));
    DenseDoubleVectorPtr probabilities = new DenseDoubleVector(samples->getNumElements(), jointProbability);
    return probabilities;
  }

protected:
  friend class BlindResiduePairSamplerClass;

  size_t numResidues;
};

typedef ReferenceCountedObjectPtr<BlindResiduePairSampler> BlindResiduePairSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_MOVERSAMPLER_BLINDRESIDUEPAIRSAMPLER_H_
